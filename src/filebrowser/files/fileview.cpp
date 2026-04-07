#include "fileview.h"
#include <QAbstractFileIconProvider>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrent>

#include "../filehandlers/filehandlerfactory.h"
#include "../filehandlers/ifilehandler.h"

FileView::FileView(const QString &filePath, QWidget *parent) : QTreeView(parent) {
    fileHandler_ = FileHandleFactory::instance().createHandler(filePath, this);
}

void FileView::loadFile() {
    QString directory = fileHandler_->getFileInfo().path() + "/" + fileHandler_->getFileInfo().baseName();
    exportDirectory_ = directory;
    packDirectory_ = directory;

    setSortingEnabled(true);

    QFuture<bool> parseFuture = QtConcurrent::run([this]() {
        return fileHandler_->parseFile();
    });

    auto* parseWatcher = new QFutureWatcher<bool>(this);
    connect(parseWatcher, &QFutureWatcher<bool>::finished, this, [this, parseWatcher]() {
        if (parseWatcher->result()) {
            model_ = new QStandardItemModel(this);
            model_->setHorizontalHeaderLabels({"Name"}); // TODO: Size
            model_->setSortRole(Qt::UserRole + 1);

            connect(fileHandler_, &IFileHandler::populationFinished, this, [this]() {
                setModel(model_);
                emit fileLoaded(model_->rowCount() > 0);
            });

            fileHandler_->populateModel(model_->invisibleRootItem());
        }
        else
            emit fileLoaded(false);

        parseWatcher->deleteLater();
    });

    parseWatcher->setFuture(parseFuture);
}

void FileView::exportFiles() {
    QFuture<bool> exportFuture = QtConcurrent::run([this]() {
       return fileHandler_->exportFiles(exportDirectory_);
    });

    auto* exportWatcher = new QFutureWatcher<bool>(this);
    connect(exportWatcher, &QFutureWatcher<bool>::finished, this, [this, exportWatcher]() {
        emit filesExported(exportWatcher->result());
        exportWatcher->deleteLater();
        if (!exportWatcher->result())
            QMessageBox::warning(this, "Export Failed", "Export failed. Check for missing DAT file.");
    });
    exportWatcher->setFuture(exportFuture);
}

void FileView::packFiles() {
    QFuture<bool> packFuture = QtConcurrent::run([this]() {
        return fileHandler_->packFiles(packDirectory_, packDirectory_);
    });

    auto* packWatcher = new QFutureWatcher<bool>(this);
    connect(packWatcher, &QFutureWatcher<bool>::finished, this, [this, packWatcher]() {
        emit filesPacked(packWatcher->result());
        packWatcher->deleteLater();
        });
    packWatcher->setFuture(packFuture);
}

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

