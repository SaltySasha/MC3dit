#include "dcfile.h"

#include <QAbstractFileIconProvider>

DCFile::DCFile(const QString &InString) : QStandardItem(QFileInfo(InString).fileName()) {
    setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::File));
    //qDebug() << "Creating file " << InString;
    FileInfo.setFile(InString);
}
