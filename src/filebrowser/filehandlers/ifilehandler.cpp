#include "ifilehandler.h"

#include "../dat/datefileentry.h"
#include "../dat/datfolderentry.h"

bool IFileHandler::populateModel(QStandardItem* rootItem) {
    if (parsedEntries_.isEmpty()) {
        return false;
    }

    for (const ParsedFileEntry& entry : parsedEntries_) {
        addVirtualPath(rootItem, entry.entry);
    }

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
