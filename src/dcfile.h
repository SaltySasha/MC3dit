#pragma once

#include <QFileInfo>
#include <QStandardItem>

// DAT Child File class
class DCFile : public QStandardItem {

public:
    explicit DCFile(const QString &filePath);

    void setNameOffset(const quint32 newNameOffset){nameOffset_ = newNameOffset;}
    void setFileOffset(const quint32 newFileOffset){fileOffset_ = newFileOffset;}
    void setFileSizeFull(const quint32 newFileSizeFull){fileSizeFull_ = newFileSizeFull;}
    void setFileSizeCompressed(const quint32 newFileSizeCompressed){fileSizeCompressed_ = newFileSizeCompressed;}

    [[nodiscard]] quint32 nameOffset() const{return nameOffset_;}
    [[nodiscard]] quint32 fileOffset() const{return fileOffset_;}
    [[nodiscard]] quint32 fileSizeFull() const{return fileSizeFull_;}
    [[nodiscard]] quint32 fileSizeCompressed() const{return fileSizeCompressed_;}
    [[nodiscard]] QString fileName() const {return fileInfo_.fileName();}
    [[nodiscard]] QString filePath() const {return fileInfo_.filePath();}
    [[nodiscard]] QString path() const {return fileInfo_.path();}

private:
    QFileInfo fileInfo_;
    quint32 nameOffset_ = 0;
    quint32 fileOffset_ = 0;
    quint32 fileSizeFull_ = 0;
    quint32 fileSizeCompressed_ = 0;
};