#pragma once
#include <QFileInfo>
#include <QStandardItem>
#include <QString>

struct EntryInfo {
    QString name() const {
        return fileInfo.isDir() ? fileInfo.path().split('/', Qt::SkipEmptyParts).last() : fileInfo.fileName();
    }
    QString filePath() const {return fileInfo.filePath();}
    QString path() const {return fileInfo.path();}
    void setMetadata(const QString& key, quint32 value) {metadata.insert(key, value);}
    bool getMetadata(const QString& key, quint32& value) const {
        if (!metadata.contains(key))
            return false;
        value = metadata.value(key);
        return true;
    }

    QFileInfo fileInfo;
    QHash<QString, quint32> metadata;
};

class entryItem : public QStandardItem, QObject {

public:
    explicit entryItem(const EntryInfo& entryInfo, QObject* parent = nullptr);

protected:
    EntryInfo entryInfo_;
};