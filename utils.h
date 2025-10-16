#ifndef MC3DIT_UTILS_H
#define MC3DIT_UTILS_H

#include <QProcess>
#include <QFile>
#include <QStandardItem>
#include <QProgressBar>

inline QStringList DaveHeaderList = {"Dave", "DAVE", "Hash"};
inline const char DaveUsableChars[] = "\x00 #$()-./?0123456789_abcdefghijklmnopqrstuvwxyz~\x7F";

void RunDaveScript(QStringList InArguments, std::function<void(int, QProcess::ExitStatus)> OnFinishedCallback = nullptr);

#endif //MC3DIT_UTILS_H