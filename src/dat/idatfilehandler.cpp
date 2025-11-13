#include "idatfilehandler.h"

#include <QAbstractFileIconProvider>
#include <QMessageBox>

#include "datefileentry.h"
#include "datfolderentry.h"

using namespace DATUtils;

IDATFileHandler::IDATFileHandler(const QString &filePath)
    : file_(filePath) {
    initItemModel();
}

void IDATFileHandler::initItemModel(bool startRead) {
    if (!file_.open(QIODeviceBase::ReadOnly)) {
        messageFileNotFound(file_.fileName(), file_.errorString());
        return;
    }
    if (itemModel_) {
        itemModel_->deleteLater();
        pathCache_.clear();
        files_.clear();;
    }
    fileMagicBytes_ = file_.read(4);
    entryCount_ = toLittleEndian( file_.read(4));
    itemModel_ = new QStandardItemModel(this);
    itemModel_->setHorizontalHeaderLabels({"Name"}); //TODO:, "Size"});
    itemModel_->setSortRole(Qt::UserRole + 1);

    if (startRead)
        readFile();
}

void IDATFileHandler::addVirtualPath(const FileEntry &fileEntry) {
    const QString virtualPath = fileEntry.filePath();
    QStringList pathParts = virtualPath.split("/", Qt::SkipEmptyParts);
    QStandardItem* parent = itemModel_->invisibleRootItem();
    QString currentPath;

    // Navigate/create all parent folders using cache
    for (int i = 0; i < pathParts.size() - 1; ++i) {
        const QString& part = pathParts[i];
        currentPath += part + "/";

        DATFolderEntry* folderEntry = nullptr;

        // Check cache first
        if (pathCache_.contains(currentPath)) {
            folderEntry = pathCache_[currentPath];
        } else {
            // Not in cache, create new folder
            folderEntry = new DATFolderEntry(currentPath, this);
            parent->appendRow(folderEntry);

            // Add to cache
            pathCache_[currentPath] = folderEntry;
        }

        parent = folderEntry;
    }

    // Create the file leaf node
    auto *newFile = new DATFileEntry(fileEntry, this);
    files_.append(newFile);
    parent->appendRow(newFile);
}
