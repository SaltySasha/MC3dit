#include "daveupperhandler.h"

#include <QDir>
#include <QDirIterator>

#include "filehandlerfactory.h"

#include <QMessageBox>
#include <QtZlib/zconf.h>
#include <QtZlib/zlib.h>

REGISTER_FILE_HANDLER(DaveUpperFileHandler, QString("DAVE"));

using namespace DATUtils;


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

quint32 DaveUpperFileHandler::calculateAlign(quint32 size, quint32 align) const {
    if (align == 0)
        return size;
    return ((size / align) + 1) * align;
}

QByteArray DaveUpperFileHandler::compressFile(const QByteArray &fileFata) const {
    QByteArray outBytes;
    uLongf compressedSize = compressBound(fileFata.size());
    outBytes.resize(compressedSize);

    z_stream zStream = {};
    zStream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(fileFata.data()));
    zStream.avail_in = fileFata.size();
    zStream.next_out = reinterpret_cast<Bytef*>(outBytes.data());
    zStream.avail_out = outBytes.size();

    // -15 = raw deflate
    if (deflateInit2(&zStream, 9, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        return QByteArray();

    if (deflate(&zStream, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&zStream);
        return QByteArray();
    }

    outBytes.resize(zStream.total_out);
    deflateEnd(&zStream);
    return outBytes;
}

quint32 DaveUpperFileHandler::handleCompactAlignment(const QList<FileEntry> &entryList, quint32 byteAlignment,
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

bool DaveUpperFileHandler::exportFiles(const QString &exportDirectory) {
    auto file = QFile(fileInfo_.absoluteFilePath());
    if (!file.open(QIODeviceBase::ReadOnly)) {
        messageFileNotFound(file.fileName(), file.errorString());
        return false;
    }

    quint32 fileNumber = 1;
    for (const EntryInfo& entryInfo : entryInfoList_) {
        file.seek(entryInfo.getMetadata("fileOffset"));
        QByteArray fileData = file.read(entryInfo.getMetadata("sizeCompressed"));
        QString filePath = exportDirectory + "/";
        QFile newFile(filePath + entryInfo.filePath());

        QDir directory;
        if (directory.mkpath(filePath + entryInfo.path()) && newFile.open(QIODevice::WriteOnly)) {
            newFile.write(fileData);
            newFile.close();
        }

        fileNumber++;
    }

    file.close();
    return true;
}

bool DaveUpperFileHandler::packFiles(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) {
    QDir sourceDirectory(sourceDirectoryPath);
    QString exportFilePath = exportDirectoryPath + "/" + fileInfo_.baseName() + ".DAT";

    QFile exportFile(exportFilePath);
    if (exportFile.exists()) {
    auto reply = QMessageBox::question(nullptr, "Overwrite File",
                                   "Output file already exists. Overwrite?",
                                   QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
     return false;
    }

    if (!sourceDirectory.exists()) {
    QMessageBox::critical(nullptr, "Error", "Source folder does not exist");
    return false;
    }

    constexpr quint32 byteAlignment = 2048;

    // Collect all files
    QList<FileEntry> fileEntries;
    QDirIterator iterator(sourceDirectory.path(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden,
    QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
    QString filePath = iterator.next();
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir())
     continue;

    QString relativePath = sourceDirectory.relativeFilePath(filePath);
    relativePath.replace("\\", "/"); // Normalize to forward slashes

    // Validate filename length
    if (relativePath.length() >= 256) {
     QMessageBox::critical(nullptr, "Error",
                             QString("Filename too long: %1").arg(relativePath));
     return false;
    }

    // Validate characters in filename
    if (!validateChars(relativePath))
     return false;

    FileEntry newEntry;
    newEntry.setFile(filePath);
    newEntry.relativePath = relativePath;
    fileEntries.append(newEntry);
    }

    sortFiles(fileEntries);

    QList<QByteArray> fileNames;
    prepareFileBlock(fileEntries, fileNames);

    // Calculate sizes
    const quint32 entriesAmount = fileNames.size();
    const quint32 entreSize = calculateAlign(entriesAmount * 0x10, 0x800);

    QByteArray allNameBytes;
    for (const QByteArray& name : fileNames)
    allNameBytes.append(name);
    const quint32 nameBlockSize = calculateAlign(allNameBytes.size(), 0x800);

    // Create output directory if needed
    QFileInfo exportFileInfo(exportFile);
    QDir().mkpath(exportFileInfo.absolutePath());

    // Open output file
    if (!exportFile.open(QIODevice::WriteOnly)) {
    QMessageBox::critical(nullptr, "Error",
                         QString("Could not create output file: %1").arg(exportFile.errorString()));
    return false;
    }

    // Start writing file data
    quint32 fileOffset = 0x800 + entreSize + nameBlockSize;
    QList<FileEntry> outEntries;
    for (int i = 0; i < fileEntries.size(); i++) {
    FileEntry& entry = fileEntries[i];

    // Handle directories
    if (entry.relativePath.endsWith("/")) {
     entry.fileOffset = fileOffset;
     outEntries.append(entry);
     continue;
    }

    // Check file offset overflow
    if (fileOffset > 0xFFFFFFFF) {
     QMessageBox::critical(nullptr, "Archive Too Big",
         "Archive size exceeds maximum (4GB).");
     exportFile.close();
     QFile::remove(exportFilePath);
     return false;
    }

    // Read file data
    QFile entryFile(entry.filePath());
    if (!entryFile.open(QIODevice::ReadOnly)) {
     QMessageBox::critical(nullptr, "Unable To Open File",
         QString("Could not open %1 with reason:\n%2").arg(entryFile.fileName(), entryFile.errorString()));
     QFile::remove(exportFilePath);
     return false;
    }

    QByteArray entryFileData = entryFile.readAll();
    entryFile.close();

    entry.sizeFull = entryFileData.size();
    entry.sizeCompressed = entryFileData.size();

    // Compress if allowed
    if (shouldCompressFile(entry.relativePath, entryFileData)) {
     QByteArray compressedFileData = compressFile(entryFileData);
     if (!compressedFileData.isEmpty() && compressedFileData.size() < entryFileData.size()) {
         entryFileData = compressedFileData;
         entry.sizeCompressed = entryFileData.size();
     }
    }

    quint32 tempFileOffset = handleCompactAlignment(outEntries, byteAlignment, entryFileData.size());
    if (tempFileOffset > -1)
     fileOffset = tempFileOffset;

    exportFile.seek(fileOffset);
    entry.fileOffset = fileOffset;
    exportFile.write(entryFileData);

    fileOffset = calculateAlign(exportFile.pos(), byteAlignment);

    outEntries.append(entry);
    }

    // Pad last file
    exportFile.seek(fileOffset - 1);
    exportFile.write("\0", 1);

    // Calculate name offsets

    QList<quint32> nameOffsets = calculateNameOffsets(fileNames);

    // Write header
    exportFile.seek(0);
    exportFile.write(QString("DAVE").toUtf8(), 4);
    exportFile.write(getIntAsBytes(entriesAmount, 4));
    exportFile.write(getIntAsBytes(entreSize, 4));
    exportFile.write(getIntAsBytes(nameBlockSize, 4));

    // Cheeky signature addition
    exportFile.seek(0x800 - signature_.length());
    exportFile.write(signature_.toUtf8());

    // Write entry table
    exportFile.seek(0x800);
    for (int i = 0; i < outEntries.size(); i++) {
     const FileEntry& entry = outEntries[i];
     exportFile.write(getIntAsBytes(nameOffsets[i], 4));
     exportFile.write(getIntAsBytes(entry.fileOffset, 4));
     exportFile.write(getIntAsBytes(entry.sizeFull, 4));
     exportFile.write(getIntAsBytes(entry.sizeCompressed, 4));
    }

    // Write filename block
    exportFile.seek(0x800 + entreSize);
    for (const QByteArray& name : fileNames) {
     exportFile.write(name);
    }

    exportFile.close();
    return true;
}

bool DaveUpperFileHandler::parseFile() {
auto file = QFile(fileInfo_.absoluteFilePath());
    if (!file.open(QIODeviceBase::ReadOnly)) {
        qWarning() << "Failed to open file:" << file.errorString();
        return false;
    }

    file.read(4);
    quint32 entryCount = toLittleEndian(file.read(4));
    quint32 entriesBlockSize = toLittleEndian(file.read(4));

    QString filePath;
    if (!entryInfoList_.isEmpty())
        entryInfoList_.clear();
    entryInfoList_.reserve(entryCount);

    for (quint32 i = 0; i < entryCount; i++) {
        quint32 currentEntryOffset = i * 0x10 + 0x800;
        file.seek(currentEntryOffset);

        quint32 nameOffset = toLittleEndian(file.read(4)) + entriesBlockSize + 0x800;
        file.seek(nameOffset);
        filePath = readEntryPath(file);
        if (filePath.endsWith("/"))
            continue;

        file.seek(currentEntryOffset + 4);

        EntryInfo newEntryInfo;
        newEntryInfo.fileInfo = QFileInfo(filePath);
        newEntryInfo.setMetadata("nameOffset", nameOffset);
        newEntryInfo.setMetadata("fileOffset", toLittleEndian(file.read(4)));
        newEntryInfo.setMetadata("sizeFull", toLittleEndian(file.read(4)));
        newEntryInfo.setMetadata("sizeCompressed", toLittleEndian(file.read(4)));

        entryInfoList_.append(newEntryInfo);
    }

    file.close();
    return true;
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
