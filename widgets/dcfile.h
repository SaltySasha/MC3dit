#pragma once

#include <QFileInfo>
#include <QStandardItem>

// DAT Child File class
class DCFile : public QStandardItem {

public:
    explicit DCFile(const QString &InString);

    void SetNameOffset(quint32 NewNameOffset){NameOffset = NewNameOffset;}
    void SetFileOffset(quint32 NewFileOffset){FileOffset = NewFileOffset;}
    void SetFileSizeFull(quint32 NewFileSizeFull){FileSizeFull = NewFileSizeFull;}
    void SetFileSizeCompressed(const quint32 NewFileSizeCompressed){FileSizeCompressed = NewFileSizeCompressed;}

    [[nodiscard]] quint32 GetNameOffset() const{return NameOffset;}
    [[nodiscard]] quint32 GetFileOffset() const{return FileOffset;}
    [[nodiscard]] quint32 GetFileSizeFull() const{return FileSizeFull;}
    [[nodiscard]] quint32 GetFileSizeCompressed() const{return FileSizeCompressed;}
    [[nodiscard]] QString GetFileName() const {return FileInfo.fileName();}
    [[nodiscard]] QString GetFilePath() const {return FileInfo.filePath();}
    [[nodiscard]] QString GetPath() const {return FileInfo.path();}

private:
    QFileInfo FileInfo;
    quint32 NameOffset = 0;
    quint32 FileOffset = 0;
    quint32 FileSizeFull = 0;
    quint32 FileSizeCompressed = 0;
};