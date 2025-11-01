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


    connect(ui->tabWidget, &QTabWidget::currentChanged, [this](int Index) {
        if (auto *Widget = dynamic_cast<DATFile*>(ui->tabWidget->widget(Index))) {
            Widget->fileType() == "Dave" ? ui->packSection->show() : ui->packSection->hide();
            ui->unpackSection->show();

            ui->unpackLineEdit->setText(!Widget->unpackDirectory().isEmpty() ? Widget->unpackDirectory() : Widget->fileDirectory());
            ui->packLineEdit->setText(!Widget->packPath().isEmpty() ? Widget->packPath() : Widget->filePath());
        }
        else {
            ui->unpackLineEdit->clear();
            ui->packLineEdit->clear();
            ui->packSection->hide();
            ui->unpackSection->hide();
        }
    });
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, [this](const int Index) {
        auto TreeView = dynamic_cast<QTreeView*>(ui->tabWidget->widget(Index));

        if (TreeView) {
            static_cast<QStandardItemModel*>(TreeView->model())->clear();
            TreeView->model()->deleteLater();
            TreeView->setModel(nullptr);
            TreeView->deleteLater();
        }
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    QMainWindow::dragEnterEvent(event);

    if (event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    QMainWindow::dragMoveEvent(event);
    if (event->mimeData()->hasUrls()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
        event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QMainWindow::dropEvent(event);

    if (event->mimeData()->hasUrls()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();

        for (const auto& Url : event->mimeData()->urls()) {
            if (Url.isLocalFile()) {
                QString File = Url.toLocalFile();
                for (qint32 TabIndex = 0; TabIndex < ui->tabWidget->count(); TabIndex++) {
                    auto* Tab = dynamic_cast<DATFile*>(ui->tabWidget->widget(TabIndex));
                    if (Tab->filePath() == File) {
                        ui->tabWidget->setCurrentWidget(Tab);
                        return;
                    }
                }

                if (File.endsWith(".dat", Qt::CaseInsensitive)) {
                    assetPath_ = Url.toLocalFile();

                    auto *NewDatFile = new DATFile(assetPath_);
                    connect(NewDatFile, &DATFile::setProgressBarMax, this, [this](qint32 NewMax) {
                        ui->progressBar->show();
                        ui->progressBar->setMaximum(NewMax);
                    });
                    connect(NewDatFile, &DATFile::updateProgressBar, this, [this](qint32 NewProgressAmount) {
                        ui->progressBar->setValue(NewProgressAmount);
                        if (NewProgressAmount >= ui->progressBar->maximum())
                            ui->progressBar->hide();
                    });
                    connect(NewDatFile, &DATFile::exportFinished, this, [this]() {
                        resetButton(ui->unpackButton, "Unpack");
                    });
                    if (NewDatFile->readDaveFile()) {
                        ui->tabWidget->addTab(NewDatFile, NewDatFile->GetFileName());
                        ui->tabWidget->setCurrentWidget(NewDatFile);
                        ui->tabWidget->tabBar()->setTabToolTip(ui->tabWidget->currentIndex(), NewDatFile->filePath());
                    }
                    else
                        NewDatFile->deleteLater();
                }
                else
                    QMessageBox::warning(this, "Invalid File", QString("%1 is not a DAT file!").arg(QFileInfo(File).fileName()));
            }
        }
    }
    else
        event->ignore();
}

void MainWindow::setButtonLock(const bool locked) const {
    ui->packButton->setEnabled(locked);
    ui->unpackButton->setEnabled(locked);
}

void MainWindow::onUnpackButtonClicked() {
    auto *Widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (QFileInfo(Widget->filePath()).isFile()) {
        ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->hide();
        playButtonAnimation(ui->unpackButton, "Unpacking");
        Widget->unpackFiles(ui->unpackLineEdit->text());
    }
    else {
        QMessageBox::critical(this, "No File", QString("Could not find %1.").arg(Widget->GetFileName()));
    }
}

void MainWindow::onPackButtonClicked() {
    auto *Widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
    if (QFileInfo(Widget->filePath()).isFile() && QDir(Widget->makeFileDirectory()).exists()) {
        playButtonAnimation(ui->packButton, "Packing");
        ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->hide();
        QStringList Arguments = {"B", "-ca", "-cn", "-cf", "-fc", "1", Widget->makeFileDirectory(), ui->packLineEdit->text()};
        DATFileUtils::runDaveScript(Arguments, [&](int, QProcess::ExitStatus) {
            resetButton(ui->packButton, "Pack");
            ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->show();
        });
    }
    else
        QMessageBox::critical(this, "Couldn't Pack File", QString("Could not find folder:\n %1").arg(Widget->makeFileDirectory()));
}

void MainWindow::onPackBrowseButtonClicked() {
    QString FilePath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!FilePath.isEmpty()) {
        auto *Widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
        Widget->setPackPath(FilePath + "/" + Widget->GetFileName());
        ui->packLineEdit->setText(Widget->packPath());
    }
}

void MainWindow::onUnpackBrowseButtonClicked() {
    QString FolderPath = QFileDialog::getExistingDirectory(this, "Choose Unpack Destination", ui->unpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!FolderPath.isEmpty()) {
        auto *Widget = dynamic_cast<DATFile*>(ui->tabWidget->currentWidget());
        Widget->setUnpackDir(FolderPath);
        ui->unpackLineEdit->setText(Widget->unpackDirectory());
    }
}

void MainWindow::playButtonAnimation(QPushButton* button, const QString& buttonText) {
    setButtonLock(false);
    QStringList AnimationText = {"", ".", "..", "..."};
    quint32 Index = 0;
    connect(buttonAnimationTimer, &QTimer::timeout, this, [=]() mutable {
                button->setText(buttonText + AnimationText[Index]);
                Index = (Index + 1) % AnimationText.size();
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

}
