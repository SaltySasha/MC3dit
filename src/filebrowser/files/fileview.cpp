#include "fileview.h"
#include <QAbstractFileIconProvider>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

#include "../dat/datefileentry.h"
#include "../filehandlers/filehandlerfactory.h"
#include "../filehandlers/ifilehandler.h"

FileView::FileView(QWidget *parent, const QString& filePath) : QTreeView(parent) {
    fileHandler_ = FileHandleFactory::instance().createHandler(filePath);
    if (!fileHandler_)
        return;

    connect(fileHandler_.get(), &IFileHandler::progressChanged, this, &FileView::progressChanged,
        Qt::QueuedConnection);

    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels({"Name"});
    model_->setSortRole(Qt::UserRole + 1);

    QFuture<QVector<ParsedFileEntry>> future = QtConcurrent::run([this]() {
        return fileHandler_->parseFile();
    });

    auto* watcher = new QFutureWatcher<QVector<ParsedFileEntry>>(this);
    connect(watcher, &QFutureWatcher<QVector<ParsedFileEntry>>::finished, this, [this, watcher]() {
        QVector<ParsedFileEntry> entries = watcher->result();

        if (entries.isEmpty()) {
            emit fileLoaded(false);
        } else {
            // Now safely populate the model on the main thread
            bool success = fileHandler_->populateModel(model_->invisibleRootItem());
            setModel(model_);
            emit fileLoaded(success);
        }

        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

FileView::~FileView() = default;

// FileView * FileView::create(const QString &filePath) {
//     auto* datFile = new FileView(filePath);
//     if (!datFile->fileHandler_) {
//         datFile->deleteLater();
//         return nullptr;
//     }
//
//     // datFile->setContextMenuPolicy(Qt::CustomContextMenu);
//     // connect(datFile, &QTreeView::customContextMenuRequested, datFile, &DATFile::openContextMenu);
//     return datFile;
// }
//
// FileView::~FileView() {
//     if (fileHandler_)
//         fileHandler_->deleteLater();
// }
//
// FileView::FileView(const QString &filePath) {
//     fileHandler_ = DATFileFactory::createHandler(filePath);
//     if (!fileHandler_)
//         return;
//
//     connect(fileHandler_, &IFileHandler::readFinished, [&] {
//         setModel(fileHandler_->itemModel());
//         model()->sort(0, Qt::AscendingOrder);
//     });
// }
//
// void FileView::openContextMenu() {
//     auto file = dynamic_cast<DATFileEntry*>(fileHandler_->itemModel()->itemFromIndex(currentIndex()));
//     if (!file)
//         return;
//
//     auto* newMenu = new QMenu(this);
//     QAction* action = newMenu->addAction("Export");
//     connect(action, &QAction::triggered, this, &FileView::exportSingleFile);
//     newMenu->exec(QCursor::pos());
//     newMenu->deleteLater();
// }
//
// // TODO: Export through file handler
// void FileView::exportSingleFile() {
//     QFile datFile("");
//     if (!datFile.exists()) {
//         //QMessageBox::critical(this, "No File", QString("Could not find %1 with reason: %2").arg(fileInfo_.fileName(), datFile.errorString()));
//         return;
//     }
//     if (!datFile.open(QIODeviceBase::ReadOnly)) {
//         //QMessageBox::warning(this, "Unable To Open File", QString("Could not open %1 with reason:\n %2").arg(fileInfo_.fileName(), datFile.errorString()));
//         return;
//     }
//
//     QString folderPath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", QDir::homePath(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
//     if (!folderPath.isEmpty()) {
//         datFile.close();
//         return;
//     }
//
//     const auto file = dynamic_cast<DATFileEntry*>(fileHandler_->itemModel()->itemFromIndex(currentIndex()));
//     datFile.seek(file->fileOffset());
//     QByteArray data = datFile.read(file->sizeCompressed());
//     if (file->sizeFull() != file->sizeCompressed()) {
//         data = QByteArray();// TODO: decompress(data, file->sizeFull());
//     }
//
//     QString filePath = folderPath + "/";
//     QFile newFile(filePath + file->fileName());
//     QDir dir;
//     if (dir.mkpath(filePath) && newFile.open(QIODevice::WriteOnly)) {
//         newFile.write(data);
//         newFile.close();
//     }
//
//     datFile.close();
// }

