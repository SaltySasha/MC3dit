#pragma once
#include "ifilehandler.h"


class DaveUpperFileHandler : public IFileHandler {
public:
    // bool populateModel(QStandardItem* rootItem) override;

protected:
    QVector<ParsedFileEntry> parseFile() override;
    QString readEntryPath(QFile& file) const;
    // bool validateChars(const QString &filePath) const override;
    // bool sortFiles( QList<FileEntry> &fileList) const override;
    // bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const override;
    // bool shouldCompressFile(const QString &filePath, const QByteArray &fileData) const override;
    // QList<quint32> calculateNameOffsets(QList<QByteArray> &namesList) override;
};
