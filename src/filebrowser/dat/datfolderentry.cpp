#include "datfolderentry.h"

#include <QAbstractFileIconProvider>
#include <QDir>

DATFolderEntry::DATFolderEntry(const QString &virtualFilePath, QObject *parent)
    : QStandardItem(QDir(virtualFilePath).dirName()), QObject(parent), virtualDirectory_(virtualFilePath){
    QStandardItem::setData(1, Qt::UserRole + 1); // 1 = folder
    setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::Folder));
    setEditable(false);
}

DATFolderEntry::~DATFolderEntry() {
    //qDebug() << QString("Deleting:: %1").arg(virtualDirectory_.path());
}
