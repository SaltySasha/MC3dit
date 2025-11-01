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
    ui->ProgressBar->hide();
    ui->PackSection->hide();
    ui->UnpackSection->hide();

    connect(ui->UnpackButton, &QPushButton::clicked, this, &MainWindow::onUnpackButtonClicked);
    connect(ui->PackButton, &QPushButton::clicked, this, &MainWindow::onPackButtonClicked);
    connect(ui->BrowsePackButton, &QPushButton::clicked, this, &MainWindow::onPackBrowseButtonClicked);
    connect(ui->BrowseUnpackButton, &QPushButton::clicked, this, &MainWindow::onUnpackBrowseButtonClicked);


    connect(ui->TabWidget, &QTabWidget::currentChanged, [this](int Index) {
        if (auto *Widget = dynamic_cast<DATFile*>(ui->TabWidget->widget(Index))) {
            Widget->fileType() == "Dave" ? ui->PackSection->show() : ui->PackSection->hide();
            ui->UnpackSection->show();

            ui->UnpackLineEdit->setText(!Widget->unpackDirectory().isEmpty() ? Widget->unpackDirectory() : Widget->fileDirectory());
            ui->PackLineEdit->setText(!Widget->packPath().isEmpty() ? Widget->packPath() : Widget->filePath());
        }
        else {
            ui->UnpackLineEdit->clear();
            ui->PackLineEdit->clear();
            ui->PackSection->hide();
            ui->UnpackSection->hide();
        }
    });
    connect(ui->TabWidget, &QTabWidget::tabCloseRequested, [this](const int Index) {
        auto TreeView = dynamic_cast<QTreeView*>(ui->TabWidget->widget(Index));

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
                for (qint32 TabIndex = 0; TabIndex < ui->TabWidget->count(); TabIndex++) {
                    auto* Tab = dynamic_cast<DATFile*>(ui->TabWidget->widget(TabIndex));
                    if (Tab->filePath() == File) {
                        ui->TabWidget->setCurrentWidget(Tab);
                        return;
                    }
                }

                if (File.endsWith(".dat", Qt::CaseInsensitive)) {
                    assetPath_ = Url.toLocalFile();

                    auto *NewDatFile = new DATFile(assetPath_);
                    connect(NewDatFile, &DATFile::setProgressBarMax, this, [this](qint32 NewMax) {
                        ui->ProgressBar->show();
                        ui->ProgressBar->setMaximum(NewMax);
                    });
                    connect(NewDatFile, &DATFile::updateProgressBar, this, [this](qint32 NewProgressAmount) {
                        ui->ProgressBar->setValue(NewProgressAmount);
                        if (NewProgressAmount >= ui->ProgressBar->maximum())
                            ui->ProgressBar->hide();
                    });
                    connect(NewDatFile, &DATFile::exportFinished, this, [this]() {
                        resetButton(ui->UnpackButton, "Unpack");
                    });
                    if (NewDatFile->readDaveFile()) {
                        ui->TabWidget->addTab(NewDatFile, NewDatFile->GetFileName());
                        ui->TabWidget->setCurrentWidget(NewDatFile);
                        ui->TabWidget->tabBar()->setTabToolTip(ui->TabWidget->currentIndex(), NewDatFile->filePath());
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
    ui->PackButton->setEnabled(locked);
    ui->UnpackButton->setEnabled(locked);
}

void MainWindow::onUnpackButtonClicked() {
    auto *Widget = dynamic_cast<DATFile*>(ui->TabWidget->currentWidget());
    if (QFileInfo(Widget->filePath()).isFile()) {
        ui->TabWidget->tabBar()->tabButton(ui->TabWidget->currentIndex(), QTabBar::RightSide)->hide();
        playButtonAnimation(ui->UnpackButton, "Unpacking");
        Widget->unpackFiles(ui->UnpackLineEdit->text());
    }
    else {
        QMessageBox::critical(this, "No File", QString("Could not find %1.").arg(Widget->GetFileName()));
    }
}

void MainWindow::onPackButtonClicked() {
    auto *Widget = dynamic_cast<DATFile*>(ui->TabWidget->currentWidget());
    if (QFileInfo(Widget->filePath()).isFile() && QDir(Widget->makeFileDirectory()).exists()) {
        playButtonAnimation(ui->PackButton, "Packing");
        ui->TabWidget->tabBar()->tabButton(ui->TabWidget->currentIndex(), QTabBar::RightSide)->hide();
        QStringList Arguments = {"B", "-ca", "-cn", "-cf", "-fc", "1", Widget->makeFileDirectory(), ui->PackLineEdit->text()};
        DATFileUtils::runDaveScript(Arguments, [&](int, QProcess::ExitStatus) {
            resetButton(ui->PackButton, "Pack");
            ui->TabWidget->tabBar()->tabButton(ui->TabWidget->currentIndex(), QTabBar::RightSide)->show();
        });
    }
    else
        QMessageBox::critical(this, "Couldn't Pack File", QString("Could not find folder:\n %1").arg(Widget->makeFileDirectory()));
}

void MainWindow::onPackBrowseButtonClicked() {
    QString FilePath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", ui->UnpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!FilePath.isEmpty()) {
        auto *Widget = dynamic_cast<DATFile*>(ui->TabWidget->currentWidget());
        Widget->setPackPath(FilePath + "/" + Widget->GetFileName());
        ui->PackLineEdit->setText(Widget->packPath());
    }
}

void MainWindow::onUnpackBrowseButtonClicked() {
    QString FolderPath = QFileDialog::getExistingDirectory(this, "Choose Unpack Destination", ui->UnpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!FolderPath.isEmpty()) {
        auto *Widget = dynamic_cast<DATFile*>(ui->TabWidget->currentWidget());
        Widget->setUnpackDir(FolderPath);
        ui->UnpackLineEdit->setText(Widget->unpackDirectory());
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
