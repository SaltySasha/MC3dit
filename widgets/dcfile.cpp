#include "dcfile.h"

#include <QAbstractFileIconProvider>

DCFile::DCFile(const QString &filePath) : QStandardItem(QFileInfo(filePath).fileName()) {
    setIcon(QAbstractFileIconProvider().icon(QAbstractFileIconProvider::File));
    //qDebug() << "Creating file " << InString;
    fileInfo_.setFile(filePath);
}
