#include "daveupperfilehandler.h"

#include <QMessageBox>

using namespace DATUtils;

QString DaveUpperFileHandler::readEntryPath(const QString &prevFileName) {
    QString outName;
    QChar currentChar = file_.read(1)[0];
    while (!currentChar.isNull()) {
        outName.append(currentChar);
        currentChar = file_.read(1)[0];
    }

    return outName;
}

bool DaveUpperFileHandler::validateChars(const QString &filePath) const {
    for (const QChar& c : filePath) {
        if (c.unicode() > 127) {
            QMessageBox::critical(nullptr, "Error",
                QString("Non-ASCII character '%1' in filename: %2").arg(c, filePath));
            return false;
        }
    }

    return true;
}

bool DaveUpperFileHandler::sortFiles(QList<FileEntry> &fileList) const {
    std::ranges::sort(fileList, [](const FileEntry& a, const FileEntry& b) {
        return a.relativePath.toLower() < b.relativePath.toLower();
    });

    return true;
}

bool DaveUpperFileHandler::prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const {
    for (const FileEntry& entry : fileEntries)
        fileBytesList.append(entry.relativePath.toUtf8() + '\0');

    return true;
}

bool DaveUpperFileHandler::shouldCompressFile(const QString &filePath, const QByteArray &fileData) const {
    return false;
}

QList<quint32> DaveUpperFileHandler::calculateNameOffsets(QList<QByteArray> &namesList) {
    QList<quint32> outNameOffsets;
    outNameOffsets.append(0);
    for (int i = 0; i < namesList.size() - 1; i++) {
        outNameOffsets.append(outNameOffsets.last() + namesList[i].size());
    }

    return outNameOffsets;
}
