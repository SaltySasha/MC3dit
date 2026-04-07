#pragma once
#include "ifilehandler.h"


class DaveLowerFileHandler : public IFileHandler {
public:
    bool parseFile() override;
    bool exportFiles(const QString& exportDirectory) override;
    bool packFiles(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) override;

protected:
    QString readEntryPath(QFile& file, const QString &prevFileName) const;
    QByteArray decompressFile(const QByteArray &fileFata, const quint32 decompressedSize) const;
    bool validateChars(const QString &filePath) const;
    bool sortFiles( QList<FileEntry> &fileList) const;
    bool prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const;
    bool shouldCompressFile(const QString &filePath, const QByteArray &fileData) const;
    QList<quint32> calculateNameOffsets(QList<QByteArray> &namesList);
    quint32 handleCompactAlignment(const QList<FileEntry> &entryList, quint32 byteAlignment,
                                             quint32 entryFileSize) const;

private:
    const QByteArray usableChars_ = QByteArray("\x00 #$()-./?0123456789_abcdefghijklmnopqrstuvwxyz~\x7F", 49);
    const QStringList blockedExtensions_ = {".pck", ".ppf", ".psppck",".pspppf", ".xbck", ".xbpf"};
    const QList<QString> blockedDirectories_ = {"flash/", "resources/vehicle/"};
    QByteArray compressName(const QString& name, const QString& prevName, int& dedupIdx) const;
    quint32 calculateAlign(quint32 size, quint32 align) const;
    [[nodiscard]] QByteArray compressFile(const QByteArray &fileFata) const;
};
