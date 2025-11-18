#pragma once
#include <qtypes.h>

#include "../idatfilehandler.h"


class DaveFileHandlerBase : public IDATFileHandler {
public:
    explicit DaveFileHandlerBase(const QString& filePath);

    bool readFile() override;
    bool unpackAndExport(const QString &exportDirectory) override;
    bool packAndExport(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) override;

protected:
    virtual QString readEntryPath(const QString &previousFileName = QString()) = 0;
    virtual bool shouldCompressFile(const QString &filePath, const QByteArray &fileData) const = 0;
    virtual QList<quint32> calculateNameOffsets(QList<QByteArray> &namesList) = 0;
    virtual quint32 handleCompactAlignment(const QList<FileEntry> &entryList, quint32 byteAlignment,
                                            quint32 entryFileSize) const {return -1;}

    [[nodiscard]] QByteArray decompressFile(const QByteArray &fileFata, quint32 decompressedSize) const;
    [[nodiscard]] QByteArray compressFile(const QByteArray &fileFata) const;

    quint32 calculateAlign(quint32 size, quint32 align) const;
    quint32 entriesBlockSize_ = 0;
    quint32 nameBlockSize_ = 0;
};
