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
    void UnpackFiles(const QString& InFolderPath);
    void PackFiles(const QString& InFolderPath) const;
    void SetUnpackDirectory(const QString &InFolderPath) {UnpackDirectory = InFolderPath;}
    void SetPackPath(const QString &InFilePath) {PackPath = InFilePath;}

    [[nodiscard]] QString GetFileName(bool IsFullFileName = true) const {return IsFullFileName ? FileInfo.fileName() : FileInfo.fileName().left(FileInfo.fileName().length() - 4);}
    [[nodiscard]] QString GetFileType() const {return FileType;}
    [[nodiscard]] QString GetFilePath() const {return FileInfo.absoluteFilePath();}
    [[nodiscard]] QString GetFileDirectory() const {return FileInfo.absolutePath();}
    [[nodiscard]] QString MakeFileDirectory() const {return FileInfo.absoluteFilePath().left(FileInfo.absoluteFilePath().length() - 4);}
    [[nodiscard]] QString GetUnpackDirectory() const {return UnpackDirectory;}
    [[nodiscard]] QString GetPackPath() const {return PackPath;}

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
    quint32 Entries{};
    quint32 FileSize{};
    quint32 FileNameSize{};

    QString ReadString(QDataStream &InStream) const;
    QList<quint32> ReadBits(QDataStream &InStream) const;
    void AddVirtualPath(const QString& InVirtualPath, quint32 InNameOffset, quint32 InFileOffset, quint32 InFileSizeFull, quint32 InFileSizeCompressed);
    void OpenContextMenu();
    void ExportSingleFile();

};
