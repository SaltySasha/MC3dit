#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QDir>
#include <QDragLeaveEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setFixedSize(width(), height());
    // ui->statusbar->setStyleSheet("color: blue");
    // ui->statusbar->showMessage("Test!!");

    connect(ui->UnpackButton, &QPushButton::clicked, this, &MainWindow::OnUnpackButtonClicked);
    connect(ui->PackButton, &QPushButton::clicked, this, &MainWindow::OnPackButtonClicked);
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

        if (event->mimeData()->urls()[0].isLocalFile()) {
            QString File = event->mimeData()->urls()[0].toLocalFile();

            if (File.endsWith(".dat", Qt::CaseInsensitive)) {
                AssetPath = event->mimeData()->urls()[0].toLocalFile();
                QFileInfo FileInfo(AssetPath);
                ui->DropLabel->setText(FileInfo.fileName());
            }
            else
                ui->DropLabel->setText("Not a .DAT file!");
        }
    }
    else
        event->ignore();
}

void MainWindow::SetButtonLock(bool AreButtonsLocked) {
    ui->PackButton->setEnabled(AreButtonsLocked);
    ui->UnpackButton->setEnabled(AreButtonsLocked);
}

void MainWindow::OnUnpackButtonClicked() {
    if (QFileInfo(AssetPath).isFile()) {
        PlayButtonAnimation(ui->UnpackButton, "Unpacking");
        QStringList Arguments = {"X", AssetPath};
        bool ShouldPlaySound = ui->SoundCheckBox->isChecked();
        RunDaveScript(Arguments, ShouldPlaySound, [&](int, QProcess::ExitStatus) {
            ResetButton(ui->UnpackButton, "Unpack");
        });
        qDebug() << ui->SoundCheckBox->isChecked();
    }
}

void MainWindow::OnPackButtonClicked() {
    if (QFileInfo(AssetPath).isFile()) {
        PlayButtonAnimation(ui->PackButton, "Packing");
        QStringList Arguments = {"B", "-ca", "-cn", "-cf", "-fc", "1", AssetPath.left(AssetPath.length()-4), AssetPath};
        bool ShouldPlaySound = ui->SoundCheckBox->isChecked();
        RunDaveScript(Arguments, ShouldPlaySound, [&](int, QProcess::ExitStatus) {
            ResetButton(ui->PackButton, "Pack");
        });
    }
}

void MainWindow::PlayButtonAnimation(QPushButton* InButton, const QString& InButtonText) {
    SetButtonLock(false);
    QStringList AnimationText = {"", ".", "..", "..."};
    int Index = 0;
    connect(ButtonAnimationTimer, &QTimer::timeout, this, [=]() mutable {
                InButton->setText(InButtonText + AnimationText[Index]);
                Index = (Index + 1) % AnimationText.size();
            });
    ButtonAnimationTimer->setInterval(0);
    ButtonAnimationTimer->start(300);
}

void MainWindow::ResetButton(QPushButton* InButton, const QString &InButtonText) {
    SetButtonLock(true);
    InButton->setText(InButtonText);
    ButtonAnimationTimer->stop();
    ButtonAnimationTimer->disconnect();
}
