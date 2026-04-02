#pragma once
#include "ifilehandler.h"


class HashFileHandler : public IFileHandler {
public:
     bool parseFile() override;
     bool exportFiles(const QString& exportDirectory) override;
     // bool packAndExport(const QString &exportDirectoryPath, const QString &sourceDirectoryPath);

protected:
     bool sortFiles( QList<FileEntry> &fileList) const;
     bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const;
     QByteArray readString(QFile &file) const;
     [[nodiscard]] bool validateChars(const QString &filePath) const;
     [[nodiscard]] quint32 calculateHash(const QString &fileData) const;
};
