#include "ifilehandler.h"

#include <QTimer>

#include "../dat/datefileentry.h"
#include "../dat/datfolderentry.h"

void IFileHandler::addVirtualPath(QStandardItem *rootItem, const EntryInfo &entryInfo) {
    const QString virtualPath = entryInfo.fileInfo.filePath();
    QStringList pathParts = virtualPath.split("/", Qt::SkipEmptyParts);
    QStandardItem* parent = rootItem;
    QString currentPath;

    for (int i = 0; i < pathParts.size() - 1; ++i) {
        const QString& part = pathParts[i];
        currentPath += part + "/";

        EntryItem* folderEntry = nullptr;

        if (pathCache_.contains(currentPath)) {
            folderEntry = pathCache_[currentPath];
        } else {
            EntryInfo newFolderEntry;
            newFolderEntry.fileInfo = QFileInfo(currentPath);
            folderEntry = new EntryItem(newFolderEntry);
            parent->appendRow(folderEntry);

            pathCache_[currentPath] = folderEntry;
        }

        parent = folderEntry;
    }

    auto *newFileEntry = new EntryItem(entryInfo);
    parent->appendRow(newFileEntry);
}

void IFileHandler::populateModel(QStandardItem* rootItem) {
    const int batchSize = 100;
    int currentIndex = 0;

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, currentIndex, rootItem, timer]() mutable {
        int endIndex = qMin(currentIndex + batchSize, entryInfoList_.size());

        for (int i = currentIndex; i < endIndex; ++i) {
            addVirtualPath(rootItem, entryInfoList_[i]);
        }

        currentIndex = endIndex;

        if (currentIndex >= entryInfoList_.size()) {
            timer->stop();
            timer->deleteLater();
            emit populationFinished();
        }
    });

    timer->start(0);
}
