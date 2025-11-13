#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dat/datfile.h"

#include <QDragLeaveEvent>
#include <QMimeData>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>

#include "dat/idatfilehandler.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->progressBar->hide();
    ui->packSection->hide();
    ui->unpackSection->hide();
    setButtonsEnabled(false);

    connect(ui->unpackButton, &QPushButton::clicked, this, &MainWindow::onUnpackButtonClicked);
    connect(ui->packButton, &QPushButton::clicked, this, &MainWindow::onPackButtonClicked);
    connect(ui->browseUnpackButton, &QPushButton::clicked, this, &MainWindow::onUnpackBrowseButtonClicked);
    connect(ui->browsePackButton, &QPushButton::clicked, this, &MainWindow::onPackBrowseButtonClicked);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    //testMethod();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    QMainWindow::dragEnterEvent(event);
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    event->accept();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    QMainWindow::dragMoveEvent(event);
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QMainWindow::dropEvent(event);
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();

    // Begin opening files
    for (const QUrl& url : event->mimeData()->urls()) {
        if (!url.isLocalFile()) {
            // QMessageBox::warning(this, "Invalid File", QString("Not a local file!"));
            continue;
        }

        QString file = url.toLocalFile();

        // Only open DAT files for now
        if (!file.endsWith(".dat", Qt::CaseInsensitive)) {
            // QMessageBox::warning(this, "Invalid File", QString("%1 is not a DAT file!").arg(QFileInfo(file).fileName()));
            continue;
        }

        bool isFileOpened = false;
        // Switch to file tab if file is already opened
        for (qint32 tabIndex = 0; tabIndex < ui->tabWidget->count(); tabIndex++) {
            auto* tab = dynamic_cast<DATFile*>(ui->tabWidget->widget(tabIndex));
            if (tab->fileHandler()->fileInfo().filePath() == file) {
                ui->tabWidget->setCurrentWidget(tab);
                isFileOpened =  true;
                break;
            }
        }
        if (isFileOpened)
            continue;

        auto *newDatFile = DATFile::create(url.toLocalFile());
        if (!newDatFile)
            continue;

        connect(newDatFile->fileHandler(), &IDATFileHandler::setProgressBarMax, this, [this](qint32 newMax) {
            ui->progressBar->show();
            ui->progressBar->setMaximum(newMax);
            lockUi(true);
        });
        connect(newDatFile->fileHandler(), &IDATFileHandler::updateProgressBar, this, [this](qint32 newProgressAmount) {
            ui->progressBar->setValue(newProgressAmount);
            if (newProgressAmount >= ui->progressBar->maximum())
                ui->progressBar->hide();
        });
        connect(newDatFile->fileHandler(), &IDATFileHandler::exportFinished, this, [this]() {
            refreshButtons();
            lockUi(false);
        });
        connect(newDatFile->fileHandler(), &IDATFileHandler::readFinished, this, [this]() {
            lockUi(false);
        });

        if (!newDatFile->fileHandler()->readFile()) {
            newDatFile->deleteLater();
            continue;
        }

        ui->unpackSection->show();
        ui->packSection->show();
        ui->tabWidget->addTab(newDatFile, newDatFile->fileHandler()->baseName());
        ui->tabWidget->setCurrentWidget(newDatFile);
        ui->tabWidget->tabBar()->setTabToolTip(ui->tabWidget->currentIndex(), newDatFile->fileHandler()->fileInfo().filePath());
        setUnpackDirectory(newDatFile->fileHandler()->fileInfo().path());
        setPackDirectory(newDatFile->fileHandler()->fileInfo().path() + "/" + newDatFile->fileHandler()->baseName());
    }
}

void MainWindow::setButtonsEnabled(const bool enabled) const {
    ui->packButton->setEnabled(enabled);
    ui->unpackButton->setEnabled(enabled);
}

void MainWindow::onTabChanged(int index) {
    auto *datFile = dynamic_cast<DATFile*>(ui->tabWidget->widget(index));
    if (!datFile) {
        ui->unpackLineEdit->clear();
        ui->packLineEdit->clear();
        ui->packSection->hide();
        ui->unpackSection->hide();
        return;
    }

    QString unpackDirectory = datFile->unpackDirectory().isEmpty() ? datFile->fileHandler()->fileInfo().path() : datFile->unpackDirectory();
    QString packDirectory = datFile->packDirectory().isEmpty() ? datFile->fileHandler()->fileInfo().path() + "/" + datFile->fileHandler()->baseName() : datFile->packDirectory();
    setUnpackDirectory(unpackDirectory);
    setPackDirectory(packDirectory);
}

void MainWindow::onTabCloseRequested(int index) {
    auto datFile = dynamic_cast<DATFile*>(ui->tabWidget->widget(index));
    if (!datFile)
        return;

    datFile->deleteLater();
}

void MainWindow::onUnpackButtonClicked() {
    refreshButtons();
    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (!widget && !QDir(ui->unpackLineEdit->text()).exists())
        return;

    if (!widget->fileHandler()->fileInfo().isFile()) {
        QMessageBox::critical(this, "No File", QString("Could not find %1.").arg(widget->fileHandler()->baseName()));
        return;
    }

    ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->hide();
    widget->fileHandler()->unpackAndExport(ui->unpackLineEdit->text());
}

void MainWindow::onPackButtonClicked() {
    refreshButtons();
    auto *datFile = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (datFile && QDir(ui->packLineEdit->text()).exists())
        datFile->fileHandler()->packAndExport(datFile->fileHandler()->fileInfo().path(), ui->packLineEdit->text());
}

void MainWindow::onPackBrowseButtonClicked() {
    QString filePath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (filePath.isEmpty())
        return;

    auto *datFile = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (datFile)
        datFile->setPackDirectory(filePath);
    ui->packLineEdit->setText(filePath);
    refreshButtons();
}

void MainWindow::onUnpackBrowseButtonClicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "Choose Unpack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty())
        return;

    auto *datFile = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (datFile)
        datFile->setUnpackDirectory(folderPath);
    ui->unpackLineEdit->setText(folderPath);
    refreshButtons();
}

void MainWindow::refreshButtons() {
    QDir dir;
    if (!ui->unpackLineEdit->text().isEmpty()) {
        dir.setPath(ui->unpackLineEdit->text());
        ui->unpackButton->setEnabled(dir.exists());
    }

    if (!ui->packLineEdit->text().isEmpty()) {
        dir.setPath(ui->packLineEdit->text());
        ui->packButton->setEnabled(dir.exists());
    }
}

void MainWindow::lockUi(bool locked) {
    ui->browsePackButton->setDisabled(locked);
    ui->browseUnpackButton->setDisabled(locked);
    setButtonsEnabled(!locked);

    QWidget* tab = ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide);
    if (tab)
        locked ? tab->hide() : tab->show();
}

void MainWindow::setUnpackDirectory(const QString &directory) {
    ui->unpackLineEdit->setText(directory);
    refreshButtons();
}

void MainWindow::setPackDirectory(const QString &directory) {
    ui->packLineEdit->setText(directory);
    refreshButtons();
}

void MainWindow::testMethod() {
    QString input = "assets/textures/blocks/stone.png.test";
    QFileInfo fileInfo(input);
    qDebug() << "input:             " << input << "\n";
    qDebug() << "baseName:          " << fileInfo.baseName();
    qDebug() << "completeBaseName:  " << fileInfo.completeBaseName();
    qDebug() << "completeSuffix:    " << fileInfo.completeSuffix();
    qDebug() << "fileName:          " << fileInfo.fileName();
    qDebug() << "filePath:          " << fileInfo.filePath();
    qDebug() << "path:              " << fileInfo.path();
    qDebug() << "absoluteFilePath:  " << fileInfo.absoluteFilePath();
    qDebug() << "absolutePath:      " << fileInfo.absolutePath();
    qDebug() << "dir:               " << fileInfo.dir();

}
