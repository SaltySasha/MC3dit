#include "utils.h"

#include <windows.h>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QAbstractFileIconProvider>
#include <QtZlib/zlib.h>

void RunDaveScript(QStringList InArguments, const std::function<void(int, QProcess::ExitStatus)>& OnFinishedCallback) {
    QString Program = "python";
    QString DaveScript = QDir::currentPath() + "/scripts/dave.py";
    InArguments.insert(0, DaveScript);
    auto* Process = new QProcess();
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

QByteArray Decompress(const QByteArray &InData, quint32 InDecompressedSize) {
    QByteArray OutBytes;
    OutBytes.resize(InDecompressedSize);

    z_stream ZStream = {};
    ZStream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(InData.data()));
    ZStream.avail_in = InData.size();
    ZStream.next_out = reinterpret_cast<Bytef*>(OutBytes.data());
    ZStream.avail_out = OutBytes.size();

    // -15 = raw deflate
    if (inflateInit2(&ZStream, -15) != Z_OK)
        return {};

    if (inflate(&ZStream, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&ZStream);
        return {};
    }

    OutBytes.resize(ZStream.total_out);
    inflateEnd(&ZStream);
    return OutBytes;
}
