#pragma once
#include <QFileInfo>
#include <QStandardItem>
#include <QString>

struct EntryInfo {
    [[nodiscard]] QString name() const {
        return isDir() ? fileInfo.path().split('/', Qt::SkipEmptyParts).last() : fileInfo.fileName();
    }
    [[nodiscard]] QString filePath() const {return fileInfo.filePath();}
    [[nodiscard]] QString path() const {return fileInfo.path();}
    [[nodiscard]] bool isDir() const {return fileInfo.fileName().isEmpty();}

    void setMetadata(const QString& key, quint32 value) {metadata.insert(key, value);}
    quint32 getMetadata(const QString& key) const {
        if (metadata.contains(key))
            return metadata.value(key);

        return 0;
    }

    QFileInfo fileInfo;
    QHash<QString, quint32> metadata;
};

class EntryItem : public QStandardItem {

public:
    explicit EntryItem(const EntryInfo& entryInfo);
    [[nodiscard]] bool isDir() const {return entryInfo_.isDir();}

protected:
    EntryInfo entryInfo_;
};