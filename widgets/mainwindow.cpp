#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../misc/utils.h"
#include "DATFile.h"

#include <QDragLeaveEvent>
#include <QMimeData>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>

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
    for (const auto& url : event->mimeData()->urls()) {
        if (!url.isLocalFile()) {
            QMessageBox::warning(this, "Invalid File", QString("Not a local file!"));
            continue;
        }

        QString file = url.toLocalFile();

        bool fileOpened = false;
        // Switch to file tab if file is already opened
        for (qint32 tabIndex = 0; tabIndex < ui->tabWidget->count(); tabIndex++) {
            auto* tab = dynamic_cast<DATFile*>(ui->tabWidget->widget(tabIndex));
            if (tab->filePath() == file) {
                ui->tabWidget->setCurrentWidget(tab);
                fileOpened =  true;
                break;
            }
        }
        if (fileOpened)
            continue;

        // Only open DAT files for now
        if (!file.endsWith(".dat", Qt::CaseInsensitive)) {
            QMessageBox::warning(this, "Invalid File", QString("%1 is not a DAT file!").arg(QFileInfo(file).fileName()));
            continue;
        }

        auto *newDatFile = new DATFile(url.toLocalFile());
        connect(newDatFile, &DATFile::setProgressBarMax, this, [this](qint32 newMax) {
            ui->progressBar->show();
            ui->progressBar->setMaximum(newMax);
        });
        connect(newDatFile, &DATFile::updateProgressBar, this, [this](qint32 newProgressAmount) {
            ui->progressBar->setValue(newProgressAmount);
            if (newProgressAmount >= ui->progressBar->maximum())
                ui->progressBar->hide();
        });
        connect(newDatFile, &DATFile::exportFinished, this, [this]() {
            resetButton(ui->unpackButton, "Unpack");
        });

        // Try reading file as a Dave file
        if (!newDatFile->readDaveFile())
            newDatFile->deleteLater();

        ui->tabWidget->addTab(newDatFile, newDatFile->GetFileName());
        ui->tabWidget->setCurrentWidget(newDatFile);
        ui->tabWidget->tabBar()->setTabToolTip(ui->tabWidget->currentIndex(), newDatFile->filePath());
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

    widget->fileType() ==   "Dave"
                            ? ui->packSection->show()
                            : ui->packSection->hide();
    ui->unpackSection->show();

    ui->unpackLineEdit->setText(widget->unpackDirectory().isEmpty()
                                ? widget->fileDirectory()
                                : widget->unpackDirectory());

    ui->packLineEdit->setText(  widget->packPath().isEmpty()
                                ? widget->filePath()
                                : widget->packPath());
}

void MainWindow::onTabCloseRequested(int index) {
    auto treeView = dynamic_cast<QTreeView*>(ui->tabWidget->widget(index));
    if (!treeView)
        return;

    static_cast<QStandardItemModel*>(treeView->model())->clear();
    treeView->model()->deleteLater();
    treeView->setModel(nullptr);
    treeView->deleteLater();
}

void MainWindow::onUnpackButtonClicked() {
    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (!widget)
        return;

    if (QFileInfo(widget->filePath()).isFile()) {
        QMessageBox::critical(this, "No File", QString("Could not find %1.").arg(widget->GetFileName()));
        return;
    }

    ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->hide();
    playButtonAnimation(ui->unpackButton, "Unpacking");
    widget->unpackFiles(ui->unpackLineEdit->text());
}

void MainWindow::onPackButtonClicked() {
    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (QFileInfo(widget->filePath()).isFile() && QDir(widget->makeFileDirectory()).exists()) {
        QMessageBox::critical(this, "Couldn't Pack File", QString("Could not find folder:\n %1").arg(widget->makeFileDirectory()));
        return;
    }

    playButtonAnimation(ui->packButton, "Packing");
    ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->hide();
    QStringList args = {"B", "-ca", "-cn", "-cf", "-fc", "1", widget->makeFileDirectory(), ui->packLineEdit->text()};
    DATFileUtils::runDaveScript(args, [&](int, QProcess::ExitStatus) {
        resetButton(ui->packButton, "Pack");
        ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->show();
    });
}

void MainWindow::onPackBrowseButtonClicked() {
    QString filePath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (filePath.isEmpty())
        return;

    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    widget->setPackPath(filePath + "/" + widget->GetFileName());
    ui->packLineEdit->setText(widget->packPath());
}

void MainWindow::onUnpackBrowseButtonClicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "Choose Unpack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty())
        return;

    auto *widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    widget->setUnpackDir(folderPath);
    ui->unpackLineEdit->setText(widget->unpackDirectory());
}

void MainWindow::playButtonAnimation(QPushButton* button, const QString& buttonText) {
    setButtonLock(false);
    QStringList animationText = {"", ".", "..", "..."};
    quint32 index = 0;
    connect(buttonAnimationTimer, &QTimer::timeout, this, [=]() mutable {
                button->setText(buttonText + animationText[index]);
                index = (index + 1) % animationText.size();
            });
    buttonAnimationTimer->setInterval(0);
    buttonAnimationTimer->start(300);
}

void MainWindow::resetButton(QPushButton* button, const QString &buttonText) {
    setButtonLock(true);
    button->setText(buttonText);
    buttonAnimationTimer->stop();
    buttonAnimationTimer->disconnect();
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
