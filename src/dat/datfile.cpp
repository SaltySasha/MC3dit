#include "datfile.h"
#include <QAbstractFileIconProvider>
#include <QFileDialog>
#include <QMenu>

#include "datefileentry.h"
#include "idatfilehandler.h"


DATFile * DATFile::create(const QString &filePath) {
    auto* datFile = new DATFile(filePath);
    if (!datFile->fileHandler_) {
        datFile->deleteLater();
        return nullptr;
    }

    // datFile->setContextMenuPolicy(Qt::CustomContextMenu);
    // connect(datFile, &QTreeView::customContextMenuRequested, datFile, &DATFile::openContextMenu);
    return datFile;
}

DATFile::~DATFile() {
    if (fileHandler_)
        fileHandler_->deleteLater();
}

DATFile::DATFile(const QString &filePath) {
    fileHandler_ = DATFileFactory::createHandler(filePath);
    connect(fileHandler_, &IDATFileHandler::readFinished, [&] {
        setModel(fileHandler_->itemModel());
        model()->sort(0, Qt::AscendingOrder);
    });
}

void DATFile::openContextMenu() {
    auto file = dynamic_cast<DATFileEntry*>(fileHandler_->itemModel()->itemFromIndex(currentIndex()));
    if (!file)
        return;

    auto* newMenu = new QMenu(this);
    QAction* action = newMenu->addAction("Export");
    connect(action, &QAction::triggered, this, &DATFile::exportSingleFile);
    newMenu->exec(QCursor::pos());
    newMenu->deleteLater();
}

// TODO: Export through file handler
void DATFile::exportSingleFile() {
    QFile datFile("");
    if (!datFile.exists()) {
        //QMessageBox::critical(this, "No File", QString("Could not find %1 with reason: %2").arg(fileInfo_.fileName(), datFile.errorString()));
        return;
    }
    if (!datFile.open(QIODeviceBase::ReadOnly)) {
        //QMessageBox::warning(this, "Unable To Open File", QString("Could not open %1 with reason:\n %2").arg(fileInfo_.fileName(), datFile.errorString()));
        return;
    }

    QString folderPath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", QDir::homePath(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!folderPath.isEmpty()) {
        datFile.close();
        return;
    }

    const auto file = dynamic_cast<DATFileEntry*>(fileHandler_->itemModel()->itemFromIndex(currentIndex()));
    datFile.seek(file->fileOffset());
    QByteArray data = datFile.read(file->sizeCompressed());
    if (file->sizeFull() != file->sizeCompressed()) {
        data = QByteArray();// TODO: decompress(data, file->sizeFull());
    }

    QString filePath = folderPath + "/";
    QFile newFile(filePath + file->fileName());
    QDir dir;
    if (dir.mkpath(filePath) && newFile.open(QIODevice::WriteOnly)) {
        newFile.write(data);
        newFile.close();
    }

    datFile.close();
}
