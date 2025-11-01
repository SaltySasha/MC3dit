#pragma once

#include <QProcess>

namespace DATFileUtils {
    void runDaveScript(QStringList args, const std::function<void(int, QProcess::ExitStatus)>& onFinishedCallback = nullptr);
}