#ifndef MC3DIT_UTILS_H
#define MC3DIT_UTILS_H

#include <QProcess>
#include <QFile>

inline QStringList DaveHeaderList = {"Dave", "DAVE", "Hash"};
inline const char DaveUsableChars[] = "\x00 #$()-./?0123456789_abcdefghijklmnopqrstuvwxyz~\x7F";

void RunDaveScript(QStringList InArguments, bool ShouldPlaySound, std::function<void(int, QProcess::ExitStatus)> OnFinishedCallback = nullptr);
void ReadDaveFile(QWidget *InParent, const QString &InFilePath);
QString ReadString(QDataStream &InStream);
QList<quint8> ReadBits(QDataStream &InStream);

#endif //MC3DIT_UTILS_H