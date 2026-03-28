#pragma once
#include "ifilehandler.h"


class HashFileHandler : public IFileHandler {
public:
//     bool unpackAndExport(const QString &exportDirectory) override;
//     bool packAndExport(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) override;

protected:
    bool parseFile() override;
//     bool validateChars(const QString &filePath) const override;
//     bool sortFiles( QList<FileEntry> &fileList) const override;
//     bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const override;

private:
     quint32 calculateHash(const QString &fileData) const;
     QByteArray readString(QFile &file) const;
};
