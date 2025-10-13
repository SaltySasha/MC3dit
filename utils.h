#ifndef MC3DIT_UTILS_H
#define MC3DIT_UTILS_H

#include <QProcess>

void RunDaveScript(QStringList InArguments, bool ShouldPlaySound, std::function<void(int, QProcess::ExitStatus)> OnFinishedCallback = nullptr);

#endif //MC3DIT_UTILS_H