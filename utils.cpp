#include "utils.h"

#include <windows.h>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QAbstractFileIconProvider>

void RunDaveScript(QStringList InArguments, std::function<void(int, QProcess::ExitStatus)> OnFinishedCallback) {
    QString Program = "python";
    QString DaveScript = QDir::currentPath() + "/scripts/dave.py";
    InArguments.insert(0, DaveScript);
    QProcess* Process = new QProcess();
    Process->setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(Process, &QProcess::readyReadStandardOutput, [Process]() {
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

    // QObject::connect(Process, &QProcess::finished, [ShouldPlaySound](int exitCode, QProcess::ExitStatus status) {
    //     if (status == QProcess::NormalExit)
    //         qDebug() << "Process finished normally with code" << exitCode;
    //     else
    //         qDebug() << "Process was killed or crashed" << exitCode << status;
    // });

    if (OnFinishedCallback)
        QObject::connect(Process, &QProcess::finished, OnFinishedCallback);

    Process->start(Program, InArguments);
}