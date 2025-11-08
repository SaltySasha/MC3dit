#pragma once
#include <QStandardItemModel>
#include <QThread>

#include "datutils.h"

class DATFolderEntry;
class DATFileEntry;

// Abstract interface for different DAT file handlers
class IDATFileHandler : public QObject {
    Q_OBJECT
public:
    explicit IDATFileHandler(const QString& filePath);

    virtual bool readFile() = 0;
    virtual bool unpackAndExport(const QString &exportDirectory) = 0;
    virtual bool packAndExport(const QDir &exportDirectory, const QDir &sourceDirectory) = 0;

    QStandardItemModel* itemModel() const {return itemModel_;}
    QString baseName() const {return QFileInfo(file_).baseName();}
    QFileInfo fileInfo() const {return QFileInfo(file_);}

signals:
    void setProgressBarMax(quint32 newMax);
    void updateProgressBar(quint32 newValue);
    void exportFinished();

protected:
    virtual QString readEntryPath(const QString &previousFileName = QString()) = 0;
    virtual void addVirtualPath(const FileEntry &fileEntry);
    virtual QByteArray decompressFile(const QByteArray &fileData, quint32 decompressedSize) const = 0;
    virtual QByteArray compressFile(const QByteArray &fileFata) const = 0;

    const QString signature_ = "SaltyWasHere:)";
    QFile file_;
    QString fileMagicBytes_;
    quint32 entryCount_ = 0;
    QList<DATFileEntry*> files_;
    QStandardItemModel *itemModel_;
    QHash<QString, DATFolderEntry*> pathCache_;
};