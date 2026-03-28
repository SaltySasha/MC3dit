#include "datfolderentry.h"

#include <QAbstractFileIconProvider>
#include <QDir>

DATFolderEntry::DATFolderEntry(const QString &virtualFilePath)
    : QStandardItem(QDir(virtualFilePath).dirName()), virtualDirectory_(virtualFilePath){
    QStandardItem::setData(1, Qt::UserRole + 1); // 1 = folder
    setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::Folder));
    setEditable(false);
}

DATFolderEntry::~DATFolderEntry() {
    //qDebug() << QString("Deleting:: %1").arg(virtualDirectory_.path());
}
