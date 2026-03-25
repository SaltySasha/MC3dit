#include "daveupperhandler.h"
#include "filehandlerfactory.h"

#include <QMessageBox>

REGISTER_FILE_HANDLER(DaveUpperFileHandler, QString("DAVE"));

using namespace DATUtils;

//
// bool DaveUpperFileHandler::validateChars(const QString &filePath) const {
//     for (const QChar& c : filePath) {
//         if (c.unicode() > 127) {
//             QMessageBox::critical(nullptr, "Error",
//                 QString("Non-ASCII character '%1' in filename: %2").arg(c, filePath));
//             return false;
//         }
//     }
//
//     return true;
// }
//
// bool DaveUpperFileHandler::sortFiles(QList<FileEntry> &fileList) const {
//     std::ranges::sort(fileList, [](const FileEntry& a, const FileEntry& b) {
//         return a.relativePath.toLower() < b.relativePath.toLower();
//     });
//
//     return true;
// }
//
// bool DaveUpperFileHandler::prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const {
//     for (const FileEntry& entry : fileEntries)
//         fileBytesList.append(entry.relativePath.toUtf8() + '\0');
//
//     return true;
// }
//
// bool DaveUpperFileHandler::shouldCompressFile(const QString &filePath, const QByteArray &fileData) const {
//     return false;
// }
//
// QList<quint32> DaveUpperFileHandler::calculateNameOffsets(QList<QByteArray> &namesList) {
//     QList<quint32> outNameOffsets;
//     outNameOffsets.append(0);
//     for (int i = 0; i < namesList.size() - 1; i++) {
//         outNameOffsets.append(outNameOffsets.last() + namesList[i].size());
//     }
//
//     return outNameOffsets;
// }

// bool DaveUpperFileHandler::populateModel(QStandardItem* rootItem) {
//     auto file = QFile(fileInfo_.absoluteFilePath());
//     if (!file.open(QIODeviceBase::ReadOnly)) {
//         messageFileNotFound(file.fileName(), file.errorString());
//         return false;
//     }
//
//     file.read(4);
//     quint32 entryCount = toLittleEndian(file.read(4));
//     quint32 entriesBlockSize = toLittleEndian(file.read(4));
//
//     QString filePath;
//     FileEntry newEntry;
//     // emit setProgressBarMax(entryCount_);
//     for (quint32 i = 0; i < entryCount; i++) {
//         file.seek(0x800 + i * 0x10);
//         newEntry.nameOffset = toLittleEndian(file.read(4)) + entriesBlockSize + 0x800;
//         newEntry.fileOffset = toLittleEndian(file.read(4));
//         newEntry.sizeFull = toLittleEndian(file.read(4));
//         newEntry.sizeCompressed = toLittleEndian(file.read(4));
//         file.seek(newEntry.nameOffset);
//
//         filePath = readEntryPath(file);
//         if (filePath.endsWith("/"))
//             continue;
//
//         newEntry.setFile(filePath);
//
//         addVirtualPath(rootItem, newEntry);
//         // updateProgressBar(i + 1);
//     }
//
//     file.close();
//     // emit readFinished(true);
//     return true;
// }

QVector<ParsedFileEntry> DaveUpperFileHandler::parseFile() {
auto file = QFile(fileInfo_.absoluteFilePath());
    if (!file.open(QIODeviceBase::ReadOnly)) {
        qWarning() << "Failed to open file:" << file.errorString();
        return parsedEntries_;
    }

    file.read(4);
    quint32 entryCount = toLittleEndian(file.read(4));
    quint32 entriesBlockSize = toLittleEndian(file.read(4));

    QString filePath;
    parsedEntries_.reserve(entryCount); // Pre-allocate for efficiency

    for (quint32 i = 0; i < entryCount; i++) {
        file.seek(0x800 + i * 0x10);
        FileEntry newEntry;
        newEntry.nameOffset = toLittleEndian(file.read(4)) + entriesBlockSize + 0x800;
        newEntry.fileOffset = toLittleEndian(file.read(4));
        newEntry.sizeFull = toLittleEndian(file.read(4));
        newEntry.sizeCompressed = toLittleEndian(file.read(4));
        file.seek(newEntry.nameOffset);

        filePath = readEntryPath(file);

        // Skip directories
        if (filePath.endsWith("/"))
            continue;

        newEntry.setFile(filePath);

        // Create thread-safe data structure
        ParsedFileEntry parsed;
        parsed.path = filePath;
        parsed.entry = newEntry;
        parsed.pathComponents = filePath.split('/', Qt::SkipEmptyParts);
        if (!parsed.pathComponents.isEmpty()) {
            parsed.fileName = parsed.pathComponents.last();
        }

        parsedEntries_.append(parsed);

        emit progressChanged(i + 1, entryCount);
    }

    file.close();
    return parsedEntries_;
}

QString DaveUpperFileHandler::readEntryPath(QFile& file) const {
    QString outName;
    QChar currentChar;
    constexpr int MAX_NAME_LENGTH = 512;  // Safety limit

    while (outName.length() < MAX_NAME_LENGTH) {
        if (file.atEnd()) {
            qWarning() << "Unexpected end of file while reading entry path";
            break;
        }

        QByteArray byte = file.read(1);
        if (byte.isEmpty()) {
            break;
        }

        currentChar = byte[0];
        if (currentChar.isNull()) {
            break;
        }

        outName.append(currentChar);
    }

    if (outName.length() >= MAX_NAME_LENGTH) {
        qWarning() << "Name length exceeded maximum, truncating";
    }

    return outName;
}
