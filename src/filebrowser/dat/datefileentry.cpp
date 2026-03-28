#include "datefileentry.h"

#include <QAbstractFileIconProvider>

DATFileEntry::DATFileEntry(const FileEntry &fileEntry, QObject *parent)
    : QStandardItem(fileEntry.fileInfo.fileName()), QObject(parent) {
    fileEntry_ = fileEntry;
    QStandardItem::setData(0, Qt::UserRole + 1);  // 0 = file
    setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::File));
    setEditable(false);
}

DATFileEntry::~DATFileEntry() {
    //qDebug() << QString("Deleting:: %1").arg(filePath());
}
