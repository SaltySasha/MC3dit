#include "entryitem.h"

#include <QAbstractFileIconProvider>

entryItem::entryItem(const EntryInfo& entryInfo, QObject* parent)
    : QStandardItem(entryInfo.name()), QObject(parent), entryInfo_(entryInfo){
    bool isFolder = entryInfo_.fileInfo.isDir();
    QStandardItem::setData(isFolder, Qt::UserRole + 1);
    setIcon(QAbstractFileIconProvider().icon(isFolder ? QAbstractFileIconProvider::Folder : QAbstractFileIconProvider::File));
    setEditable(false);
}
