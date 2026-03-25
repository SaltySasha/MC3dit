#pragma once
#include <QStandardItem>

#include "datutils.h"


class DATFileEntry : public QStandardItem, QObject {
public:
    explicit DATFileEntry(const FileEntry &fileEntry, QObject *parent = nullptr);

    QString fileName() const {return fileEntry_.fileName();}
    quint32 fileOffset() const {return fileEntry_.fileOffset;}
    quint32 sizeFull() const {return fileEntry_.sizeFull;}
    quint32 sizeCompressed() const {return fileEntry_.sizeCompressed;}
    QString filePath() const {return fileEntry_.filePath();}
    QString path() const {return fileEntry_.path();}

private:
    FileEntry fileEntry_;
};
