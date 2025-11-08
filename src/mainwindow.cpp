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

    connect(ui->unpackButton, &QPushButton::clicked, this, &MainWindow::onUnpackButtonClicked);
    connect(ui->packButton, &QPushButton::clicked, this, &MainWindow::onPackButtonClicked);
    connect(ui->browsePackButton, &QPushButton::clicked, this, &MainWindow::onPackBrowseButtonClicked);
    connect(ui->browseUnpackButton, &QPushButton::clicked, this, &MainWindow::onUnpackBrowseButtonClicked);

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
        });
        connect(newDatFile->fileHandler(), &IDATFileHandler::updateProgressBar, this, [this](qint32 newProgressAmount) {
            ui->progressBar->setValue(newProgressAmount);
            if (newProgressAmount >= ui->progressBar->maximum())
                ui->progressBar->hide();
        });
        connect(newDatFile->fileHandler(), &IDATFileHandler::exportFinished, this, [this]() {
            resetButton(ui->unpackButton, "Unpack");
        });

        ui->tabWidget->addTab(newDatFile, newDatFile->fileHandler()->baseName());
        ui->tabWidget->setCurrentWidget(newDatFile);
        ui->tabWidget->tabBar()->setTabToolTip(ui->tabWidget->currentIndex(), newDatFile->fileHandler()->fileInfo().filePath());
    }
}

void MainWindow::setButtonLock(const bool locked) const {
    ui->packButton->setEnabled(locked);
    ui->unpackButton->setEnabled(locked);
}

void MainWindow::onTabChanged(int index) {
    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->widget(index));
    if (!widget) {
        ui->unpackLineEdit->clear();
        ui->packLineEdit->clear();
        ui->packSection->hide();
        ui->unpackSection->hide();
        return;
    }

    ui->unpackSection->show();
    ui->packSection->show();

    // TODO
    // ui->unpackLineEdit->setText(widget->unpackDirectory().isEmpty()
                                // ? widget->fileDirectory()
                                // : widget->unpackDirectory());

    // ui->packLineEdit->setText(  widget->packPath().isEmpty()
                                // ? widget->filePath()
                                // : widget->packPath());
}

void MainWindow::onTabCloseRequested(int index) {
    auto datFile = dynamic_cast<DATFile*>(ui->tabWidget->widget(index));
    if (!datFile)
        return;

    datFile->deleteLater();
}

void MainWindow::onUnpackButtonClicked() {
    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (!widget)
        return;

    if (!widget->fileHandler()->fileInfo().isFile()) {
        QMessageBox::critical(this, "No File", QString("Could not find %1.").arg(widget->fileHandler()->baseName()));
        return;
    }

    //ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->hide();
    widget->fileHandler()->unpackAndExport(ui->unpackLineEdit->text());
}

void MainWindow::onPackButtonClicked() {
    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    widget->fileHandler()->packAndExport(QDir("D:/MC3dit/testing/packed"), QDir("D:/MC3dit/testing"));
}

void MainWindow::onPackBrowseButtonClicked() {
    QString filePath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (filePath.isEmpty())
        return;

    //TODO: Logic
}

void MainWindow::onUnpackBrowseButtonClicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "Choose Unpack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty())
        return;

    //TODO: Logic
}

void MainWindow::resetButton(QPushButton* button, const QString &buttonText) {
    setButtonLock(true);
    button->setText(buttonText);
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
