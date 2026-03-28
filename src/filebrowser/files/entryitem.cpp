#include "entryitem.h"

#include <QAbstractFileIconProvider>

EntryItem::EntryItem(const EntryInfo& entryInfo)
    : QStandardItem(entryInfo.name()), entryInfo_(entryInfo) {
    QStandardItem::setData(isDir(), Qt::UserRole + 1);
    setIcon(QAbstractFileIconProvider().icon(isDir() ? QAbstractFileIconProvider::Folder : QAbstractFileIconProvider::File));
    setEditable(false);
}
