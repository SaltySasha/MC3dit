#pragma once
#include <QStandardItemModel>
#include <QFuture>
#include <QVector>

#include "../dat/datutils.h"

class DATFolderEntry;
class DATFileEntry;

struct ParsedFileEntry {
    QString path;
    QString fileName;
    FileEntry entry;
    QStringList pathComponents;
};

class IFileHandler : public QObject {
    Q_OBJECT

public:
    virtual bool parseFile() = 0;
    virtual bool populateModel(QStandardItem* rootItem);
    virtual void setFileInfo(const QFileInfo &fileInfo) {fileInfo_ = fileInfo;}

protected:
    const QString signature_ = "SaltyWasHere:)";
    QFileInfo fileInfo_;
    // QVector<EntryInfo> entryList_;
    QList<DATFileEntry*> files_;
    QHash<QString, DATFolderEntry*> pathCache_;
    QVector<ParsedFileEntry> parsedEntries_;

    void addVirtualPath(QStandardItem* rootItem, const FileEntry &fileEntry);
};