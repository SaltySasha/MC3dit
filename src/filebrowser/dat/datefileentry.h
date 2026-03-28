#pragma once
#include <QStandardItem>

#include "datutils.h"


class DATFileEntry : public QStandardItem {
public:
    explicit DATFileEntry(const FileEntry &fileEntry);

    QString fileName() const {return fileEntry_.fileName();}
    quint32 fileOffset() const {return fileEntry_.fileOffset;}
    quint32 sizeFull() const {return fileEntry_.sizeFull;}
    quint32 sizeCompressed() const {return fileEntry_.sizeCompressed;}
    QString filePath() const {return fileEntry_.filePath();}
    QString path() const {return fileEntry_.path();}

    ~DATFileEntry() override;
private:
    FileEntry fileEntry_;
};
