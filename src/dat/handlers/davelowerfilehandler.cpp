#include "davelowerfilehandler.h"

#include <QMessageBox>

#include "../datutils.h"

using namespace DATUtils;

QString DaveLowerFileHandler::readEntryPath(const QString &prevFileName) {
    QString outName;
    QList<quint32> nameBits;

    nameBits = unpackSixBitValues(file_.read(3));
    if (nameBits[0] >= 0x38) {
        const quint32 deduplicatedSize = (nameBits.takeAt(1) - 0x20) * 8 + nameBits.takeAt(0) - 0x38;
        outName = prevFileName.left(deduplicatedSize);
    }
    int nextByte = 0;
    while (!nameBits.isEmpty() && nameBits.first() != 0) {
        nextByte = nameBits.takeAt(0);
        if (nextByte < 49)
            outName += usableChars_.at(nextByte);
        else
            outName += "\x00";
        if (nameBits.isEmpty())
            nameBits = unpackSixBitValues(file_.read(3));
    }

    return outName;
}

bool DaveLowerFileHandler::validateChars(const QString &filePath) const {
    for (const QChar& c : filePath.toLower()) {
        if (!usableChars_.contains(c.toLatin1())) {
            QMessageBox::critical(nullptr, "Error",
                QString("Invalid character '%1' in filename: %2").arg(c, filePath));
            return false;
        }
    }

    return true;
}

bool DaveLowerFileHandler::sortFiles(QList<FileEntry> &fileList) const {
    std::ranges::sort(fileList, [this](const FileEntry& a, const FileEntry& b) {
        QString aLower = a.relativePath.toLower();
        QString bLower = b.relativePath.toLower();
        for (int i = 0; i < qMin(aLower.length(), bLower.length()); i++) {
            int aIdx = usableChars_.indexOf(aLower[i].toLatin1());
            int bIdx = usableChars_.indexOf(bLower[i].toLatin1());
            if (aIdx != bIdx)
                return aIdx < bIdx;
        }

        return aLower.length() < bLower.length();
    });

    return true;
}

bool DaveLowerFileHandler::prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const {
    int dedupIdx = 0;
    QString prevFileName;

    for (const FileEntry& entry : fileEntries) {
        // Compressed names
        QString currentName = entry.relativePath.toLower();

        // Handle identical names (case-insensitive duplicates)
        if (currentName == prevFileName) {
            fileBytesList.append(QByteArray());
            continue;
        }

        QByteArray compressedName = compressName(currentName, prevFileName, dedupIdx);
        fileBytesList.append(compressedName);
        prevFileName = currentName;
    }

    return true;
}

bool DaveLowerFileHandler::shouldCompressFile(const QString &filePath, const QByteArray &fileData) const {
    // Don't compress nested Dave files
    if (fileData.startsWith("Dave") | fileData.startsWith("DAVE"))
        return false;

    QString lowerName = filePath.toLower();
    for (const QString& ext : blockedExtensions_) {
        if (lowerName.endsWith(ext)) {
            // Allow if not in root
            if (filePath.contains("/")) {
                for (const QString& dir : blockedDirectories_) {
                    if (lowerName.startsWith(dir))
                        return false;
                }
            }
        }
    }

    return true;
}

QList<quint32> DaveLowerFileHandler::calculateNameOffsets(QList<QByteArray> &namesList) {
    QList<quint32> outNameOffsets;
    outNameOffsets.append(0);

    // Compressed names are stored backwards
    for (int i = namesList.size() - 1; i >= 0; i--) {
        outNameOffsets.append(outNameOffsets.last() + namesList[i].size());
    }
    outNameOffsets.pop_back();
    std::reverse(outNameOffsets.begin(), outNameOffsets.end());
    std::reverse(namesList.begin(), namesList.end());

    return outNameOffsets;
}


quint32 DaveLowerFileHandler::handleCompactAlignment(const QList<FileEntry> &entryList, quint32 byteAlignment,
                                                     quint32 entryFileSize) const {
    if (!entryList.isEmpty()) {
        const FileEntry& previousEntry = entryList.last();
        quint32 previousAlign = previousEntry.fileOffset & (byteAlignment - 1);
        quint32 previousSize = calculateAlign(previousEntry.sizeCompressed, 0x20);

        if (previousEntry.sizeCompressed > 0 && previousEntry.sizeCompressed < byteAlignment &&
            previousAlign + previousSize + entryFileSize < byteAlignment) {
            return previousEntry.fileOffset + previousSize;
            }
    }

    return -1;
}

QByteArray DaveLowerFileHandler::compressName(const QString &name, const QString &prevName, int &dedupIdx) const {
    boost::multiprecision::cpp_int compName = 0;
    int dedupCount = 0;
    QString actualName = name;

    // Calculate deduplication
    if (!prevName.isEmpty() && dedupIdx > 0) {
        int idx = 0;

        for (int i = 0; i < qMin(prevName.length(), name.length()); ++i) {
            if (prevName[i] == name[i])
                idx  = i + 1;
            else
                break;
        }

        if (idx > 0) {
            dedupCount = idx;
            actualName = name.sliced(idx);
        } else
            dedupIdx = 0; // Reset on new name}
    }

    // Compress the name
    int nameSize = actualName.length() + 1;
    for (qsizetype i = actualName.size(); i--;) {
        compName <<= 6;
        qsizetype idx = usableChars_.indexOf(actualName.at(i).toLatin1());
        compName |= static_cast<boost::multiprecision::cpp_int>(idx);
    }

    // Add deduplication info if present
    if (dedupCount > 0) {
        int dedupDiv = dedupCount / 8;
        int dedupMod = dedupCount % 8;
        compName <<= 12;
        compName |= static_cast<boost::multiprecision::cpp_int>((dedupDiv + 0x20) << 6 | dedupMod + 0x38);
        nameSize += 2;
    }

    // Calculate byte size (6 bits per char = 0.75 bytes per char)
    int byteSize = qCeil(nameSize * 0.75);

    // Update dedup index (limit to 32)
    dedupIdx = dedupIdx + 1 & 0x1F;

    return getIntAsBytes(compName, byteSize);
}