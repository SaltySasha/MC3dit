#pragma once
#include "davefilehandlerbase.h"


class DaveUpperFileHandler : public DaveFileHandlerBase {
public:
    explicit DaveUpperFileHandler(const QString& filePath) : DaveFileHandlerBase(filePath) {}

protected:
    QString readEntryPath(const QString &prevFileName = QString()) override;
    bool validateChars(const QString &filePath) const override;
    bool sortFiles( QList<FileEntry> &fileList) const override;
    bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const override;
    bool shouldCompressFile(const QString &filePath, const QByteArray &fileData) const override;
    QList<quint32> calculateNameOffsets(QList<QByteArray> &namesList) override;
};
