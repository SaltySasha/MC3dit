#pragma once
#include "ifilehandler.h"


class DaveUpperFileHandler : public IFileHandler {
public:
    bool parseFile() override;
    bool exportFiles(const QString &exportDirectory) override;

protected:
    QString readEntryPath(QFile& file) const;
    // bool validateChars(const QString &filePath) const override;
    // bool sortFiles( QList<FileEntry> &fileList) const override;
    // bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const override;
    // bool shouldCompressFile(const QString &filePath, const QByteArray &fileData) const override;
    // QList<quint32> calculateNameOffsets(QList<QByteArray> &namesList) override;
};
