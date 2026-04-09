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

    QFileInfo fileInfo;
    quint32 hash = 0;
    quint32 nameOffset = 0;
    quint32 fileOffset = 0;
    quint32 sizeFull = 0;
    quint32 sizeCompressed = 0;
};

class EntryItem : public QStandardItem {

public:
    explicit EntryItem(const EntryInfo& entryInfo);
    [[nodiscard]] bool isDir() const {return entryInfo_.isDir();}

protected:
    EntryInfo entryInfo_;
};