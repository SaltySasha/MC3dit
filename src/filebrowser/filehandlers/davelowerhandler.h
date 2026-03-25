#pragma once
#include "ifilehandler.h"


class DaveLowerFileHandler : public IFileHandler {
public:
    // bool populateModel(QStandardItem* rootItem) override;
    // explicit DaveLowerFileHandler(const QString& filePath) : DaveFileHandlerBase(filePath) {}

protected:
    QVector<ParsedFileEntry> parseFile() override;
    QString readEntryPath(QFile& file, const QString &prevFileName) const;
//     bool validateChars(const QString &filePath) const override;
//     bool sortFiles( QList<FileEntry> &fileList) const override;
//     bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const override;
//     bool shouldCompressFile(const QString &filePath, const QByteArray &fileData) const override;
//     QList<quint32> calculateNameOffsets(QList<QByteArray> &namesList) override;
//     quint32 handleCompactAlignment(const QList<FileEntry> &entryList, quint32 byteAlignment,
//                                             quint32 entryFileSize) const override;
//
// private:
     const QByteArray usableChars_ = QByteArray("\x00 #$()-./?0123456789_abcdefghijklmnopqrstuvwxyz~\x7F", 49);
//     const QStringList blockedExtensions_ = {".pck", ".ppf", ".psppck",".pspppf", ".xbck", ".xbpf"};
//     const QList<QString> blockedDirectories_ = {"flash/", "resources/vehicle/"};
//     QByteArray compressName(const QString& name, const QString& prevName, int& dedupIdx) const;
};
