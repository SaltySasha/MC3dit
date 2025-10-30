#pragma once

#include <QProcess>
#include <QProgressBar>

void RunDaveScript(QStringList InArguments, const std::function<void(int, QProcess::ExitStatus)>& OnFinishedCallback = nullptr);

QByteArray Decompress(const QByteArray &InData, quint32 InDecompressedSize);