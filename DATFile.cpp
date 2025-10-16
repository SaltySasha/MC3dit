#include "DATFile.h"

#include <QAbstractFileIconProvider>
#include <QMessageBox>

DATFile::DATFile(const QString &InFilePath) {
    FileInfo.setFile(InFilePath);
    ItemModel = new QStandardItemModel(this);
    QTreeView::setModel(ItemModel);
    ItemModel->setHorizontalHeaderLabels({"Name"}); //TODO:, "Size"});
    ItemModel->setSortRole(Qt::UserRole + 1);
}

void DATFile::ReadDaveFile() {
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
                //qDebug() << i << Stream.device()->pos();

                Stream >> NameOffset;
                //qDebug() << i << NameOffset;
                NameOffset += FileSize + 0x800;

                Stream >> FileOffset;
                Stream >> FileSizeFull;
                Stream >> FileSizeCompressed;

                Stream.device()->seek(NameOffset);
                if (FileType == "DAVE")
                    FileName = ReadString(Stream);
                else if (FileType == "Dave"){
                    // Convert to its own function.
                    QList<qint32> NameBits;
                    NameBits = ReadBits(Stream);
                    QString PreviousFileName = FileName;
                    FileName.clear();
                    if (NameBits[0] >= 0x38) {
                        qint32 DeduplicatedSize = (NameBits.takeAt(1) - 0x20) * 8 + NameBits.takeAt(0) - 0x38;
                        //qDebug() << i << DeduplicatedSize << NameBits;
                        FileName = PreviousFileName.left(DeduplicatedSize);
                    }
                    while (!NameBits.isEmpty() && NameBits.first() != 0) {
                        FileName += DaveUsableChars[NameBits.takeAt(0)];
                        if (NameBits.isEmpty())
                            NameBits = ReadBits(Stream);
                    }
                }

                //qDebug() << i << FileName << NameOffset << FileOffset << FileSizeFull << FileSizeCompressed;
                AddVirtualPath(FileName);
                emit UpdateProgressBar(i + 1);
            }

        }
        else {
            QMessageBox::warning(this, "Non Dave File Detected", QString("%1 is not a Dave file.").arg(FileInfo.fileName()));
        }

        File.close();
        ItemModel->sort(0, Qt::AscendingOrder);
        disconnect();
    }
}

QString DATFile::ReadString(QDataStream &InStream) {
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

QList<qint32> DATFile::ReadBits(QDataStream &InStream) {
    QList<qint32> Result;
    char Buffer[3];

    InStream.readRawData(Buffer, 3);
    quint32 CompressedData = static_cast<quint8>(Buffer[0])
                            | (static_cast<quint8>(Buffer[1]) << 8)
                            | (static_cast<quint8>(Buffer[2]) << 16);
    for (quint32 i = 0; i < 4; i++)
        Result.append(CompressedData >> (i * 6) & 0x3f);

    return Result;
}

// TODO: Potentially rework to speed up opening of file
void DATFile::AddVirtualPath(const QString &VirtualPath) const {
    QStringList Parts = VirtualPath.split("/", Qt::SkipEmptyParts);
    QStandardItem* Parent = ItemModel->invisibleRootItem();
    QString Part;
    QStandardItem* Item;
    for (int i = 0; i < Parts.size(); ++i) {
        Part.clear();
        Part = Parts[i];
        bool IsFile = (i == Parts.size() - 1);

        Item = nullptr;
        for (int j = 0; j < Parent->rowCount(); ++j) {
            QStandardItem* Child = Parent->child(j, 0);
            if (Child && Child->text() == Part) {
                Item = Child;
                break;
            }
        }

        if (!Item) {
            Item = new QStandardItem(Part);
            Item->setData(IsFile ? 1 : 0);
            Item->setEditable(false);
            if (IsFile)
                Item->setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::File));
            else
                Item->setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::Folder));

            Parent->appendRow(Item);
        }

        Parent = Item;
    }
}
