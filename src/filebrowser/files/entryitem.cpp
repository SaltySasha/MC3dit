#include "entryitem.h"

#include <QAbstractFileIconProvider>

EntryItem::EntryItem(const EntryInfo& entryInfo, QObject* parent)
    : QStandardItem(entryInfo.name()), QObject(parent), entryInfo_(entryInfo) {
    QStandardItem::setData(isDir(), Qt::UserRole + 1);
    setIcon(QAbstractFileIconProvider().icon(isDir() ? QAbstractFileIconProvider::Folder : QAbstractFileIconProvider::File));
    setEditable(false);
}
