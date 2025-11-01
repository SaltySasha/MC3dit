#include "datfile.h"
#include "../misc/utils.h"
#include <QAbstractFileIconProvider>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QtZlib/zlib.h>

using namespace DATFileUtils;

DATFile::DATFile(const QString &filePath) {
    fileInfo_.setFile(filePath);
    itemModel_ = new QStandardItemModel(this);
    QTreeView::setModel(itemModel_);
    itemModel_->setHorizontalHeaderLabels({"Name"}); //TODO:, "Size"});
    itemModel_->setSortRole(Qt::UserRole + 1);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, &DATFile::openContextMenu);
}

bool DATFile::readDaveFile() {
        if (QFile file(fileInfo_.filePath()); file.exists()) {
        if (!file.open(QIODeviceBase::ReadOnly)) {
            QMessageBox::warning(this, "No File", QString("Could not find %1 with reason: %2").arg(fileInfo_.fileName(), file.errorString()));
        }

        fileType_ = file.read(4);
        if (headerList_.contains(fileType_)) {
            entries_ = toLittleEndian(file.read(4));
            fileSize_ = toLittleEndian(file.read(4));
            fileNameSize_ = toLittleEndian(file.read(4));

            emit setProgressBarMax(entries_);

            QString fileName;
            quint32 nameOffset;
            quint32 fileOffset;
            quint32 fileSizeFull;
            quint32 fileSizeCompressed;
            for (quint32 i = 0; i < entries_; i++) {
                file.seek(0x800 + i * 0x10);
                nameOffset = toLittleEndian(file.read(4)) + fileSize_ + 0x800;
                fileOffset = toLittleEndian(file.read(4));
                fileSizeFull = toLittleEndian(file.read(4));
                fileSizeCompressed = toLittleEndian(file.read(4));

                file.seek(nameOffset);
                fileName = readName(file, fileType_, fileName);

                 //qDebug() << i << fileName << nameOffset << fileOffset << fileSizeFull << fileSizeCompressed;

                addVirtualPath(fileName, nameOffset, fileOffset, fileSizeFull, fileSizeCompressed);
                emit updateProgressBar(i + 1);
            }
        }
        else {
            QMessageBox::warning(this, "Non Dave File Detected", QString("%1 is not a Dave file.").arg(fileInfo_.fileName()));
            return false;
        }

        file.close();
        itemModel_->sort(0, Qt::AscendingOrder);
        return true;
    }

    return false;
}

// TODO: Export on separate thread, maybe even more than one (I am speed)
void DATFile::unpackFiles(const QString& folderPath) {
    if (QFile datFile(fileInfo_.filePath()); datFile.exists()) {
        if (!datFile.open(QIODeviceBase::ReadOnly)) {
            QMessageBox::critical(this, "Couldn't Open", QString("Could not open %1 with reason: %2").arg(fileInfo_.fileName(), datFile.errorString()));
            return;
        }

        emit setProgressBarMax(entries_);

        quint32 fileNumber = 1;
        for (const auto file : files_) {
            datFile.seek(file->fileOffset());
            QByteArray data = datFile.read(file->fileSizeCompressed());
            if (file->fileSizeFull() != file->fileSizeCompressed()) {
                data = decompress(data, file->fileSizeFull());
            }
            QString filePath = folderPath + "/" + GetFileName(false) + "/";
            QFile newFile(filePath + file->filePath());

            QDir dir;
            if (dir.mkpath(filePath + file->path()) && newFile.open(QIODevice::WriteOnly)) {
                newFile.write(data);
                newFile.close();
            }

            emit updateProgressBar(fileNumber);
            fileNumber++;
        }

        datFile.close();
        emit exportFinished();
    }
    else
        QMessageBox::critical(this, "No File", QString("Could not find %1 with reason: %2").arg(fileInfo_.fileName(), datFile.errorString()));
}

void DATFile::packFiles(const QString& folderPath) const {

}

void DATFile::openContextMenu() {
    auto file = dynamic_cast<DCFile*>(itemModel_->itemFromIndex(currentIndex()));
    if (file) {
        auto* newMenu = new QMenu(this);
        QAction* action = newMenu->addAction("Export");
        connect(action, &QAction::triggered, this, &DATFile::exportSingleFile);
        newMenu->exec(QCursor::pos());
        newMenu->deleteLater();
    }
}

// TODO: Make function for both single and all file export functions
void DATFile::exportSingleFile() {
    if (QFile datFile(fileInfo_.filePath()); datFile.exists()) {
        if (!datFile.open(QIODeviceBase::ReadOnly)) {
            QMessageBox::critical(this, "Couldn't Open", QString("Could not open %1 with reason: %2").arg(fileInfo_.fileName(), datFile.errorString()));
            return;
        }

        QString folderPath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", QDir::homePath(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!folderPath.isEmpty()) {
            const auto file = dynamic_cast<DCFile*>(itemModel_->itemFromIndex(currentIndex()));
            datFile.seek(file->fileOffset());
            QByteArray data = datFile.read(file->fileSizeCompressed());
            if (file->fileSizeFull() != file->fileSizeCompressed()) {
                data = decompress(data, file->fileSizeFull());
            }

            QString filePath = folderPath + "/";
            QFile newFile(filePath + file->fileName());
            QDir dir;
            if (dir.mkpath(filePath) && newFile.open(QIODevice::WriteOnly)) {
                newFile.write(data);
                newFile.close();
            }
        }

        datFile.close();
    }
    else
        QMessageBox::critical(this, "No File", QString("Could not find %1 with reason: %2").arg(fileInfo_.fileName(), datFile.errorString()));
}

// TODO: Potentially rework to speed up opening of file
void DATFile::addVirtualPath(const QString &virtualPath, quint32 nameOffset, quint32 fileOffset, quint32 fileSizeFull, quint32 fileSizeCompressed) {
    QStringList parts = virtualPath.split("/", Qt::SkipEmptyParts);
    QStandardItem* parent = itemModel_->invisibleRootItem();
    QString part;
    QStandardItem* item;
    for (int i = 0; i < parts.size(); ++i) {
        part.clear();
        part = parts[i];
        const bool isFile = (i == parts.size() - 1);

        item = nullptr;
        for (int j = 0; j < parent->rowCount(); ++j) {
            QStandardItem* child = parent->child(j, 0);
            if (child && child->text() == part) {
                item = child;
                break;
            }
        }

        if (!item) {
            if (isFile) {
                auto *newFile = new DCFile(virtualPath);
                newFile->setNameOffset(nameOffset);
                newFile->setFileOffset(fileOffset);
                newFile->setFileSizeFull(fileSizeFull);
                newFile->setFileSizeCompressed(fileSizeCompressed);
                files_.append(QSharedPointer<DCFile>(newFile));
                item = newFile;
            }
            else {
                item = new QStandardItem(part);
                item->setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::Folder));
            }

            item->setData(isFile ? 1 : 0);
            item->setEditable(false);
            parent->appendRow(item);
        }

        parent = item;
    }
}

QByteArray DATFile::decompress(const QByteArray &data, const quint32 decompressedSize) const {
    QByteArray outBytes;
    outBytes.resize(decompressedSize);

    z_stream zStream = {};
    zStream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
    zStream.avail_in = data.size();
    zStream.next_out = reinterpret_cast<Bytef*>(outBytes.data());
    zStream.avail_out = outBytes.size();

    // -15 = raw deflate
    if (inflateInit2(&zStream, -15) != Z_OK)
        return {};

    if (inflate(&zStream, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&zStream);
        return {};
    }

    outBytes.resize(zStream.total_out);
    inflateEnd(&zStream);
    return outBytes;
}

quint32 DATFile::toLittleEndian(const QByteArray &byteArray) const {
    return qFromLittleEndian<quint32>(byteArray.constData());
}

QString DATFile::readName(QFile &file, const QString &fileType, const QString &fileName) const {
    QString outName;

    if (fileType == "DAVE") {
        QChar currentChar = file.read(1)[0];
        while (!currentChar.isNull()) {
            outName.append(currentChar);
            currentChar = file.read(1)[0];
        }
    }
    else if (fileType == "Dave") {
        QList<quint32> nameBits;
        nameBits = unpackSixBitValues(file);
        if (nameBits[0] >= 0x38) {
            const quint32 deduplicatedSize = (nameBits.takeAt(1) - 0x20) * 8 + nameBits.takeAt(0) - 0x38;
            outName = fileName.left(deduplicatedSize);
        }
        while (!nameBits.isEmpty() && nameBits.first() != 0) {
            outName += usableChars_[nameBits.takeAt(0)];
            if (nameBits.isEmpty())
                nameBits = unpackSixBitValues(file);
        }
    }
    return outName;
}

QList<quint32> DATFile::unpackSixBitValues(QFile &file) const {
    QList<quint32> result;
    result.reserve(4);

    const quint32 compressedData = toLittleEndian(file.read(3));
    for (quint32 i = 0; i < 4; i++)
        result.append(compressedData >> (i * 6) & 0x3f);

    return result;
}
