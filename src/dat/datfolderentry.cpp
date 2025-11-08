#include "datfolderentry.h"

#include <QAbstractFileIconProvider>
#include <QDir>

DATFolderEntry::DATFolderEntry(const QString &virtualFilePath, QObject *parent)
    : QStandardItem(QDir(virtualFilePath).dirName()), QObject(parent), virtualDirectory_(virtualFilePath){
    QStandardItem::setData(0); // 0 = folder
    setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::Folder));
    setEditable(false);
}
