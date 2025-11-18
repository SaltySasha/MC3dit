#pragma once
#include "../idatfilehandler.h"


class HashFileHandler : public IDATFileHandler {
public:
    explicit HashFileHandler(const QString& filePath)
        : IDATFileHandler(filePath) {}

    bool readFile() override;
    bool unpackAndExport(const QString &exportDirectory) override;
    bool packAndExport(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) override;

protected:
    bool validateChars(const QString &filePath) const override;
    bool sortFiles( QList<FileEntry> &fileList) const override;
    bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const override;

private:
    quint32 calculateHash(const QString &fileData) const;

    QByteArray readString();
};
