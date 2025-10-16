#pragma once

#include <QFileInfo>
#include <QStandardItem>

// DAT Child File class
class DCFile : public QStandardItem {

public:
    DCFile(const QString &InString);

    void SetNameOffset(quint32 NewNameOffset){NameOffset = NewNameOffset;}
    void SetFileOffset(quint32 NewFileOffset){FileOffset = NewFileOffset;}
    void SetFileSizeFull(quint32 NewFileSizeFull){FileSizeFull = NewFileSizeFull;}
    void SetFileSizeCompressed(const quint32 NewFileSizeCompressed){FileSizeCompressed = NewFileSizeCompressed;}

    quint32 GetNameOffset(){return NameOffset;}
    quint32 GetFileOffset(){return FileOffset;}
    quint32 GetFileSizeFull(){return FileSizeFull;}
    quint32 GetFileSizeCompressed(){return FileSizeCompressed;}
    QString GetFileName(){return FileInfo.fileName();}
    QString GetFilePath(){return FileInfo.filePath();}
    QString GetPath(){return FileInfo.path();}

private:
    QFileInfo FileInfo;
    quint32 NameOffset = 0;
    quint32 FileOffset = 0;
    quint32 FileSizeFull = 0;
    quint32 FileSizeCompressed = 0;
};