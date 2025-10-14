#include "utils.h"

#include <windows.h>
#include <QApplication>
#include <QDir>
#include <QMessageBox>

void RunDaveScript(QStringList InArguments, bool ShouldPlaySound, std::function<void(int, QProcess::ExitStatus)> OnFinishedCallback) {
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

    QObject::connect(Process, &QProcess::finished, [ShouldPlaySound](int exitCode, QProcess::ExitStatus status) {

        if (ShouldPlaySound) {
            PlaySound(TEXT("C:\\Windows\\Media\\tada.wav"), NULL, SND_FILENAME | SND_ASYNC);
        }

        if (status == QProcess::NormalExit)
            qDebug() << "Process finished normally with code" << exitCode;
        else
            qDebug() << "Process was killed or crashed" << exitCode << status;
    });

    if (OnFinishedCallback)
        QObject::connect(Process, &QProcess::finished, OnFinishedCallback);

    Process->start(Program, InArguments);
}

void ReadDaveFile(QWidget *InParent, const QString &InFilePath) {
    if (QFile File(InFilePath); File.exists()) {
        if (!File.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
            QMessageBox::warning(InParent, "File not found", InFilePath);
        }

        QDataStream Stream(&File);
        Stream.setByteOrder(QDataStream::LittleEndian);

        char Buffer[4];
        Stream.readRawData(Buffer, 4);
        QString FileType = QString::fromLatin1(Buffer, 4);
        qDebug() << FileType;
        if (DaveHeaderList.contains(FileType)) {
            quint32 Entries;
            Stream >> Entries;
            quint32 FileSize;
            Stream >> FileSize;
            quint32 FileNameSize;
            Stream >> FileNameSize;

            QString FileName;
            quint32 NameOffset;
            quint32 FileOffset;
            quint32 FileSizeFull;
            quint32 FileSizeCompressed;
            for (quint32 i = 0; i < Entries; i++) {
                Stream.device()->seek(0x800 + i * 0x10);

                Stream >> NameOffset;
                NameOffset += FileSize + 0x800;
                Stream >> FileOffset;
                Stream >> FileSizeFull;
                Stream >> FileSizeCompressed;

                Stream.device()->seek(NameOffset);
                if (FileType == "DAVE")
                    FileName = ReadString(Stream);
                else if (FileType == "Dave"){
                    // Convert to its own function.
                    QList<quint8> NameBits;
                    while (NameBits.isEmpty())
                        NameBits = ReadBits(Stream);
                    QString PreviousFileName = FileName;
                    FileName.clear();
                    if (NameBits[0] >= 0x38) {
                        quint32 DeduplicatedSize = (NameBits.takeAt(1) - 0x20) * 8 + NameBits.takeFirst() - 0x38;
                        FileName = PreviousFileName.left(DeduplicatedSize);
                    }
                    while (!NameBits.isEmpty() && NameBits.first() != 0) {
                        FileName += DaveUsableChars[NameBits.takeFirst()];
                        if (NameBits.isEmpty())
                            NameBits = ReadBits(Stream);
                    }
                }

                qDebug() << i << FileName << NameOffset << FileOffset << FileSizeFull << FileSizeCompressed;
            }

        }
        else {
            QMessageBox::warning(InParent, "Not a Dave file", InFilePath);
        }

        File.close();
    }
}

QString ReadString(QDataStream &InStream) {
    QString Result;
    char CurrentChar;

    while (!InStream.atEnd()) {
        if (InStream.readRawData(&CurrentChar, 1) != 1)
            break;
        if (CurrentChar == '\0')
            break;
        Result.append(CurrentChar);
    }

    return Result;
}

QList<quint8> ReadBits(QDataStream &InStream) {
    QList<quint8> Result;
    char Buffer[3];

    InStream.readRawData(Buffer, 3);
    quint32 CompressedData = static_cast<quint8>(Buffer[0])
                            | (static_cast<quint8>(Buffer[1]) << 8)
                            | (static_cast<quint8>(Buffer[2]) << 16);
    for (quint32 i = 0; i < 4; i++)
        Result.append((CompressedData >> (i * 6)) & 0x3f);

    return Result;
}


