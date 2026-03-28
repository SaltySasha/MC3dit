#pragma once
#include <QFileInfo>
#include <QStandardItem>
#include <QString>

struct EntryInfo {
    [[nodiscard]] QString name() const {
        return fileInfo.isDir() ? fileInfo.path().split('/', Qt::SkipEmptyParts).last() : fileInfo.fileName();
    }
    [[nodiscard]] QString filePath() const {return fileInfo.filePath();}
    [[nodiscard]] QString path() const {return fileInfo.path();}

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

class EntryItem : public QStandardItem, QObject {

public:
    explicit EntryItem(const EntryInfo& entryInfo, QObject* parent = nullptr);
    [[nodiscard]] bool isDir() const {return entryInfo_.fileInfo.isDir();}

protected:
    EntryInfo entryInfo_;
};