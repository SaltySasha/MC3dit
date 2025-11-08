#include "idatfilehandler.h"

#include <QAbstractFileIconProvider>
#include <QMessageBox>

#include "datefileentry.h"
#include "datfolderentry.h"

using namespace DATUtils;

IDATFileHandler::IDATFileHandler(const QString &filePath)
    : file_(filePath) {
    if (!file_.open(QIODeviceBase::ReadOnly)) {
        messageFileNotFound(file_.fileName(), file_.errorString());
        return;
    }

    fileMagicBytes_ = file_.read(4);
    entryCount_ = toLittleEndian( file_.read(4));
    itemModel_ = new QStandardItemModel(this);
    itemModel_->setHorizontalHeaderLabels({"Name"}); //TODO:, "Size"});
    itemModel_->setSortRole(Qt::UserRole + 1);
}

void IDATFileHandler::addVirtualPath(const FileEntry &fileEntry) {
    const QString virtualPath = fileEntry.filePath();
    QStringList parts = virtualPath.split("/", Qt::SkipEmptyParts);
    QStandardItem* parent = itemModel_->invisibleRootItem();
    QString currentPath;

    // Navigate/create all parent folders using cache
    for (int i = 0; i < parts.size() - 1; ++i) {
        const QString& part = parts[i];
        currentPath += part + "/";

        DATFolderEntry* item = nullptr;

        // Check cache first
        if (pathCache_.contains(currentPath)) {
            item = pathCache_[currentPath];
        } else {
            // Not in cache, create new folder
            item = new DATFolderEntry(currentPath, this);
            parent->appendRow(item);

            // Add to cache
            pathCache_[currentPath] = item;
        }

        parent = item;
    }

    // Create the file leaf node
    auto *newFile = new DATFileEntry(fileEntry, this);
    files_.append(newFile);
    parent->appendRow(newFile);
}
