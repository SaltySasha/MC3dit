#include "utils.h"

#include <QDir>
#include <QMessageBox>

namespace DATFileUtils {
    void runDaveScript(QStringList args, const std::function<void(int, QProcess::ExitStatus)>& onFinishedCallback) {
        QString program = "python";
        QString daveScript = QDir::currentPath() + "/scripts/dave.py";
        args.insert(0, daveScript);
        auto* process = new QProcess();
        process->setProcessChannelMode(QProcess::MergedChannels);

        QObject::connect(process, &QProcess::readyReadStandardOutput, [process]() {
            const QString output = process->readAllStandardOutput();
            if (output.contains("Overwrite")) {
                process->write(QString("Y").toLatin1());
                process->waitForBytesWritten();
                process->closeWriteChannel();
            }
            else {
                process->terminate();
            }
        });

        // QObject::connect(Process, &QProcess::finished, [ShouldPlaySound](int exitCode, QProcess::ExitStatus status) {
        //     if (status == QProcess::NormalExit)
        //         qDebug() << "Process finished normally with code" << exitCode;
        //     else
        //         qDebug() << "Process was killed or crashed" << exitCode << status;
        // });

        if (onFinishedCallback)
            QObject::connect(process, &QProcess::finished, onFinishedCallback);

        process->start(program, args);
    }
}