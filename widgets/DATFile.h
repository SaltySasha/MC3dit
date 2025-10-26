#pragma once

#include "dcfile.h"
#include <QFileInfo>
#include <QStandardItemModel>
#include <QTreeView>

class DATFile : public QTreeView {
    Q_OBJECT

public:
    explicit DATFile(const QString &InFilePath);

    bool ReadDaveFile();
    void UnpackFiles(QString InFolderPath);
    void PackFiles(QString InFolderPath);
    void SetUnpackDirectory(QString InFolderPath) {UnpackDirectory = InFolderPath;}
    void SetPackPath(QString InFilePath) {PackPath = InFilePath;}
    QString GetFileName(bool IsFullFileName = true) const {return IsFullFileName ? FileInfo.fileName() : FileInfo.fileName().left(FileInfo.fileName().length() - 4);}
    QString GetFileType() const {return FileType;}
    QString GetFilePath() const {return FileInfo.absoluteFilePath();}
    QString GetFileDirectory() const {return FileInfo.absolutePath();}
    QString MakeFileDirectory() const {return FileInfo.absoluteFilePath().left(FileInfo.absoluteFilePath().length() - 4);}
    QString GetUnpackDirectory() const {return UnpackDirectory;}
    QString GetPackPath() const {return PackPath;}

signals:
    void SetProgressBarMax(quint32 NewMax);
    void UpdateProgressBar(quint32 NewValue);
    void ExportFinished();

private:
    const QStringList DATHeaderList = {"Dave", "DAVE"}; // TODO, "Hash"}; - Add Hash file type support
    const char DaveUsableChars[69] = "\x00 #$()-./?0123456789_abcdefghijklmnopqrstuvwxyz~\x7F";

    QFileInfo FileInfo;
    QStandardItemModel *ItemModel;
    QList<DCFile*> Files;
    QString UnpackDirectory;
    QString PackPath;

    QString FileType;
    quint32 Entries;
    quint32 FileSize;
    quint32 FileNameSize;

    QString ReadString(QDataStream &InStream);
    QList<quint32> ReadBits(QDataStream &InStream);
    void AddVirtualPath(const QString& InVirtualPath, quint32 InNameOffset, quint32 InFileOffset, quint32 InFileSizeFull, quint32 InFileSizeCompressed);

};
