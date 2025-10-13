#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setFixedSize(width(), height());
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

void MainWindow::OnUnpackButtonClicked() {
    if (QFileInfo(AssetPath).isFile()) {
        ui->UnpackButton->setText("Unpacking...");
        QApplication::processEvents();
        QString Program = "python";
        QString DaveScript = QDir::currentPath() + "/scripts/dave.py";
        QStringList Arguments = {DaveScript, "X", AssetPath};
        QProcess* Process = new QProcess(this);
        Process->setProcessChannelMode(QProcess::MergedChannels);
        connect(Process, &QProcess::readyReadStandardOutput, [Process]() {
            QString Output = Process->readAllStandardOutput();
            qDebug() << Output;
            if (Output.contains("Overwrite")) {
                Process->write(QString("Y").toLatin1());
                Process->waitForBytesWritten();
                Process->closeWriteChannel();
            }
            else {
                Process->terminate();
            }
        });
        connect(Process, &QProcess::finished, [&](int exitCode, QProcess::ExitStatus status) {
            if (status == QProcess::NormalExit)
                qDebug() << "Process finished normally with code" << exitCode;
            else
                qDebug() << "Process was killed or crashed" << exitCode << status;

            ui->UnpackButton->setText("Unpack");
        });
        Process->start(Program, Arguments);
        Process->waitForFinished(-1);
    }
}

void MainWindow::OnPackButtonClicked() {
    if (QFileInfo(AssetPath).isFile()) {
        ui->PackButton->setText("Packing...");
        QApplication::processEvents();
        QString Program = "python";
        QString DaveScript = QDir::currentPath() + "/scripts/dave.py";
        QStringList Arguments = {DaveScript, "B", "-ca", "-cn", "-cf", "-fc", "1", AssetPath.left(AssetPath.length()-4), AssetPath};
        qDebug() << Arguments;
        QProcess* Process = new QProcess(this);
        Process->setProcessChannelMode(QProcess::MergedChannels);
        connect(Process, &QProcess::readyReadStandardOutput, [Process]() {
            QString Output = Process->readAllStandardOutput();
            qDebug() << Output;
            if (Output.contains("Overwrite")) {
                Process->write(QString("Y").toLatin1());
                Process->waitForBytesWritten();
                Process->closeWriteChannel();
            }
            else {
                Process->terminate();
            }
        });
        connect(Process, &QProcess::finished, [&](int exitCode, QProcess::ExitStatus status) {
            if (status == QProcess::NormalExit)
                qDebug() << "Process finished normally with code" << exitCode;
            else
                qDebug() << "Process was killed or crashed" << exitCode << status;

            ui->PackButton->setText("Pack");
        });
        Process->start(Program, Arguments);
        Process->waitForFinished(-1);
    }
}
