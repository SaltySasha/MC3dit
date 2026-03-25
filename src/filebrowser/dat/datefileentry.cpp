#include "datefileentry.h"

#include <QAbstractFileIconProvider>

DATFileEntry::DATFileEntry(const FileEntry &fileEntry, QObject *parent)
    : QStandardItem(fileEntry.fileInfo.fileName()), QObject(parent) {
    fileEntry_ = fileEntry;
    QStandardItem::setData(1);  // 1 = file
    setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::File));
    setEditable(false);
}
