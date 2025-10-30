#include "datfile.h"
#include "../misc/utils.h"
#include <QAbstractFileIconProvider>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>

DATFile::DATFile(const QString &InFilePath) {
    FileInfo.setFile(InFilePath);
    ItemModel = new QStandardItemModel(this);
    QTreeView::setModel(ItemModel);
    ItemModel->setHorizontalHeaderLabels({"Name"}); //TODO:, "Size"});
    ItemModel->setSortRole(Qt::UserRole + 1);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, &DATFile::OpenContextMenu);
}

bool DATFile::ReadDaveFile() {
    if (QFile File(FileInfo.filePath()); File.exists()) {
        if (!File.open(QIODeviceBase::ReadOnly)) {
            QMessageBox::warning(this, "No File", QString("Could not find %1 with reason: %2").arg(FileInfo.fileName(), File.errorString()));
        }

        QDataStream Stream(&File);
        Stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray FileData(4, 0);
        Stream.readRawData(FileData.data(), FileData.size());
        FileType = FileData;
        if (DATHeaderList.contains(FileType)) {
            Stream >> Entries;
            Files.reserve(Entries);
            Stream >> FileSize;
            Stream >> FileNameSize;

            emit SetProgressBarMax(Entries);


            QString FileName;
            quint32 NameOffset;
            quint32 FileOffset;
            quint32 FileSizeFull;
            quint32 FileSizeCompressed;
            for (quint32 i = 0; i < Entries; i++) {
                Stream.device()->seek(0x800 + i * 0x10);

                Stream  >> NameOffset;
                NameOffset += FileSize + 0x800;
                Stream >> FileOffset;
                Stream >> FileSizeFull;
                Stream >> FileSizeCompressed;

                Stream.device()->seek(NameOffset);
                if (FileType == "DAVE")
                    FileName = ReadString(Stream);
                else if (FileType == "Dave"){
                    // Convert to its own function.
                    QList<quint32> NameBits;
                    NameBits = ReadBits(Stream);
                    QString PreviousFileName = FileName;
                    FileName.clear();
                    if (NameBits[0] >= 0x38) {
                        quint32 DeduplicatedSize = (NameBits.takeAt(1) - 0x20) * 8 + NameBits.takeAt(0) - 0x38;
                        FileName = PreviousFileName.left(DeduplicatedSize);
                    }
                    while (!NameBits.isEmpty() && NameBits.first() != 0) {
                        FileName += DaveUsableChars[NameBits.takeAt(0)];
                        if (NameBits.isEmpty())
                            NameBits = ReadBits(Stream);
                    }
                }

                //qDebug() << i << FileName << NameOffset << FileOffset << FileSizeFull << FileSizeCompressed;
                AddVirtualPath(FileName, NameOffset, FileOffset, FileSizeFull, FileSizeCompressed);
                emit UpdateProgressBar(i + 1);
            }
        }
        else {
            QMessageBox::warning(this, "Non Dave File Detected", QString("%1 is not a Dave file.").arg(FileInfo.fileName()));
            return false;
        }

        File.close();
        ItemModel->sort(0, Qt::AscendingOrder);
        return true;
    }

    return false;
}

QString DATFile::ReadString(QDataStream &InStream) const {
    QString Result;
    char CurrentChar;

    while (!InStream.atEnd()) {
        if (InStream.readRawData(&CurrentChar, 1) != 1)
            break;
        if (CurrentChar == '\0')
            break;
        Result.append(CurrentChar);
    }

    return Result;
}

QList<quint32> DATFile::ReadBits(QDataStream &InStream) const {
    QList<quint32> Result;
    Result.reserve(4);

    quint8 Buffer[3];
    InStream.readRawData(reinterpret_cast<char*>(Buffer), 3);
    quint32 CompressedData = Buffer[0]
                            | Buffer[1] << 8
                            | Buffer[2] << 16;
    for (quint32 i = 0; i < 4; i++)
        Result.append(CompressedData >> (i * 6) & 0x3f);

    return Result;
}

// TODO: Potentially rework to speed up opening of file
void DATFile::AddVirtualPath(const QString &InVirtualPath, quint32 InNameOffset, quint32 InFileOffset, quint32 InFileSizeFull, quint32 InFileSizeCompressed) {
    QStringList Parts = InVirtualPath.split("/", Qt::SkipEmptyParts);
    QStandardItem* Parent = ItemModel->invisibleRootItem();
    QString Part;
    QStandardItem* Item;
    for (int i = 0; i < Parts.size(); ++i) {
        Part.clear();
        Part = Parts[i];
        const bool IsFile = (i == Parts.size() - 1);

        Item = nullptr;
        for (int j = 0; j < Parent->rowCount(); ++j) {
            QStandardItem* Child = Parent->child(j, 0);
            if (Child && Child->text() == Part) {
                Item = Child;
                break;
            }
        }

        if (!Item) {
            if (IsFile) {
                auto *NewFile = new DCFile(InVirtualPath);
                NewFile->SetNameOffset(InNameOffset);
                NewFile->SetFileOffset(InFileOffset);
                NewFile->SetFileSizeFull(InFileSizeFull);
                NewFile->SetFileSizeCompressed(InFileSizeCompressed);
                Files.append(NewFile);
                Item = NewFile;
            }
            else {
                Item = new QStandardItem(Part);
                Item->setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::Folder));
            }

            Item->setData(IsFile ? 1 : 0);
            Item->setEditable(false);
            Parent->appendRow(Item);
        }

        Parent = Item;
    }
}

void DATFile::OpenContextMenu() {
    auto File = dynamic_cast<DCFile*>(ItemModel->itemFromIndex(currentIndex()));
    if (File) {
        auto* NewMenu = new QMenu(this);
        QAction* Open = NewMenu->addAction("Export");
        connect(Open, &QAction::triggered, this, &DATFile::ExportSingleFile);
        NewMenu->exec(QCursor::pos());
        NewMenu->deleteLater();
    }
}

void DATFile::ExportSingleFile() {
    if (QFile DATFile(FileInfo.filePath()); DATFile.exists()) {
        if (!DATFile.open(QIODeviceBase::ReadOnly)) {
            QMessageBox::critical(this, "Couldn't Open", QString("Could not open %1 with reason: %2").arg(FileInfo.fileName(), DATFile.errorString()));
            return;
        }

        QString FolderPath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", QDir::homePath(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!FolderPath.isEmpty()) {
            auto File = dynamic_cast<DCFile*>(ItemModel->itemFromIndex(currentIndex()));
            DATFile.seek(File->GetFileOffset());
            QByteArray Data = DATFile.read(File->GetFileSizeCompressed());
            if (File->GetFileSizeFull() != File->GetFileSizeCompressed()) {
                Data = Decompress(Data, File->GetFileSizeFull());
            }

            QString FilePath = FolderPath + "/";
            QFile NewFile(FilePath + File->GetFileName());
            QDir Directory;
            if (Directory.mkpath(FilePath) && NewFile.open(QIODevice::WriteOnly)) {
                NewFile.write(Data);
                NewFile.close();
            }
        }

        DATFile.close();
    }
    else
        QMessageBox::critical(this, "No File", QString("Could not find %1 with reason: %2").arg(FileInfo.fileName(), DATFile.errorString()));
}


// TODO: Export on separate thread, maybe even more than one (I am speed)
void DATFile::UnpackFiles(const QString& InFolderPath) {
    if (QFile DATFile(FileInfo.filePath()); DATFile.exists()) {
        if (!DATFile.open(QIODeviceBase::ReadOnly)) {
            QMessageBox::critical(this, "Couldn't Open", QString("Could not open %1 with reason: %2").arg(FileInfo.fileName(), DATFile.errorString()));
            return;
        }

        emit SetProgressBarMax(Entries);

        quint32 FileNumber = 1;
        for (auto File : Files) {
            DATFile.seek(File->GetFileOffset());
            QByteArray Data = DATFile.read(File->GetFileSizeCompressed());
            if (File->GetFileSizeFull() != File->GetFileSizeCompressed()) {
                Data = Decompress(Data, File->GetFileSizeFull());
            }
            QString FilePath = InFolderPath + "/" + GetFileName(false) + "/";
            QFile NewFile(FilePath + File->GetFilePath());

            QDir Directory;
            if (Directory.mkpath(FilePath + File->GetPath()) && NewFile.open(QIODevice::WriteOnly)) {
                NewFile.write(Data);
                NewFile.close();
            }

            emit UpdateProgressBar(FileNumber);
            FileNumber++;
        }

        DATFile.close();
        emit ExportFinished();
    }
    else
        QMessageBox::critical(this, "No File", QString("Could not find %1 with reason: %2").arg(FileInfo.fileName(), DATFile.errorString()));
}

void DATFile::PackFiles(const QString& InFolderPath) const {

}
