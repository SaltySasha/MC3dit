#include "ifilehandler.h"

#include <QTimer>

#include "../dat/datefileentry.h"
#include "../dat/datfolderentry.h"
#include "../files/entryitem.h"

bool IFileHandler::populateModel(QStandardItem* rootItem) {
    if (parsedEntries_.isEmpty()) {
        return false;
    }

    for (const ParsedFileEntry& entry : parsedEntries_) {
        addVirtualPath(rootItem, entry.entry);
    }

    // TESTING CODE
    for (auto& entry : entryList_) {
        rootItem->appendRow(entry);
    }
    // END TESTING CODE

    return true;
}

void IFileHandler::addVirtualPath(QStandardItem* rootItem, const FileEntry &fileEntry) {
    const QString virtualPath = fileEntry.filePath();
    QStringList pathParts = virtualPath.split("/", Qt::SkipEmptyParts);
    QStandardItem* parent = rootItem;
    QString currentPath;

    for (int i = 0; i < pathParts.size() - 1; ++i) {
        const QString& part = pathParts[i];
        currentPath += part + "/";

        DATFolderEntry* folderEntry = nullptr;

        if (pathCache_.contains(currentPath)) {
            folderEntry = pathCache_[currentPath];
        } else {
            folderEntry = new DATFolderEntry(currentPath, this);
            parent->appendRow(folderEntry);

            pathCache_[currentPath] = folderEntry;
        }

        parent = folderEntry;
    }

    auto *newFile = new DATFileEntry(fileEntry, this);
    files_.append(newFile);
    parent->appendRow(newFile);
}

void IFileHandler::populateModelBatched(QStandardItem* rootItem) {
    // Process in batches of 100 items
    const int batchSize = 100;
    int currentIndex = 0;

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, currentIndex, rootItem, timer]() mutable {
        int endIndex = qMin(currentIndex + batchSize, parsedEntries_.size());

        for (int i = currentIndex; i < endIndex; ++i) {
            addVirtualPath(rootItem, parsedEntries_[i].entry);
        }

        currentIndex = endIndex;

        if (currentIndex >= parsedEntries_.size()) {
            timer->stop();
            timer->deleteLater();
            emit populationFinished();
        }
    });

    timer->start(0);
}