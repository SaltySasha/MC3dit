#pragma once
#include <QStandardItemModel>
#include <QFuture>
#include <QVector>

#include "../dat/datutils.h"
#include "../files/entryitem.h"

class DATFolderEntry;
class DATFileEntry;
class EntryItem;

struct ParsedFileEntry {
    QString path;
    QString fileName;
    FileEntry entry;
    QStringList pathComponents;
};

class IFileHandler : public QObject {
    Q_OBJECT

public:
    virtual void setFileInfo(const QFileInfo &fileInfo) {fileInfo_ = fileInfo;}
    virtual bool parseFile() = 0;
    virtual void populateModel(QStandardItem* rootItem);

signals:
    void parseFinished(bool success);
    void populationFinished();

protected:
    const QString signature_ = "SaltyWasHere:)";
    QFileInfo fileInfo_;
    QHash<QString, EntryItem*> pathCache_;
    QList<EntryItem*> entryList_;
    QList<EntryInfo> entryInfoList_;
    QList<DATFileEntry*> files_;
    QHash<QString, DATFolderEntry*> pathCacheDELETE_;
    QList<ParsedFileEntry> parsedEntriesDELETE_;

    void addVirtualPath(QStandardItem* rootItem, const EntryInfo &entryInfo);
};