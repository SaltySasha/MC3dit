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
    virtual bool packAndExport(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) = 0;

    QStandardItemModel* itemModel() const {return itemModel_;}
    QString baseName() const {return QFileInfo(file_).baseName();}
    QFileInfo fileInfo() const {return QFileInfo(file_);}

signals:
    void readFinished(bool wasSuccessful);
    void setProgressBarMax(quint32 newMax);
    void updateProgressBar(quint32 newValue);
    void exportFinished();
    void packingFinished();

protected:
    virtual void initItemModel(bool startRead = false);
    virtual QString readEntryPath(const QString &previousFileName = QString()) = 0;
    virtual void addVirtualPath(const FileEntry &fileEntry);
    [[nodiscard]] virtual QByteArray decompressFile(const QByteArray &fileData, quint32 decompressedSize) const = 0;
    [[nodiscard]] virtual QByteArray compressFile(const QByteArray &fileFata) const = 0;

    const QString signature_ = "SaltyWasHere:)";
    QFile file_;
    QString fileMagicBytes_;
    quint32 entryCount_ = 0;
    QList<DATFileEntry*> files_;
    QStandardItemModel *itemModel_ = nullptr;
    QHash<QString, DATFolderEntry*> pathCache_;
};