#include "davefilehandlerbase.h"

#include <QDirIterator>
#include <QMessageBox>
#include <QtZlib/zlib.h>
#include <boost/multiprecision/cpp_int.hpp>

#include "../datutils.h"
#include "../datefileentry.h"

using namespace DATUtils;

DaveFileHandlerBase::DaveFileHandlerBase(const QString &filePath)
    : IDATFileHandler(filePath) {
    if (!file_.isOpen())
        return;

    entriesBlockSize_ = toLittleEndian(file_.read(4));
    nameBlockSize_ = toLittleEndian(file_.read(4));
}

bool DaveFileHandlerBase::readFile() {
    QString filePath;
    FileEntry newEntry;
    emit setProgressBarMax(entryCount_);
    for (quint32 i = 0; i < entryCount_; i++) {
        file_.seek(0x800 + i * 0x10);
        newEntry.nameOffset = toLittleEndian(file_.read(4)) + entriesBlockSize_ + 0x800;
        newEntry.fileOffset = toLittleEndian(file_.read(4));
        newEntry.sizeFull = toLittleEndian(file_.read(4));
        newEntry.sizeCompressed = toLittleEndian(file_.read(4));
        file_.seek(newEntry.nameOffset);

        filePath = readEntryPath(filePath);
        if (filePath.endsWith("/"))
            continue;

        newEntry.setFile(filePath);

        addVirtualPath(newEntry);
        updateProgressBar(i + 1);
    }

    file_.close();
    emit readFinished(true);
    return true;
}

bool DaveFileHandlerBase::unpackAndExport(const QString &exportDirectory) {
    if (!file_.open(QIODeviceBase::ReadOnly)) {
        messageFileNotFound(file_.fileName(), file_.errorString());
        return false;
    }

    emit setProgressBarMax(files_.length());

    quint32 fileNumber = 1;
    for (const DATFileEntry *file : files_) {
        file_.seek(file->fileOffset());
        QByteArray fileData = file_.read(file->sizeCompressed());
        if (file->sizeFull() != file->sizeCompressed()) {
            fileData = decompressFile(fileData, file->sizeFull());
        }
        QString filePath = exportDirectory + "/" + baseName() + "/";
        QFile newFile(filePath + file->filePath());

        QDir directory;
        if (directory.mkpath(filePath + file->path()) && newFile.open(QIODevice::WriteOnly)) {
            newFile.write(fileData);
            newFile.close();
        }

        emit updateProgressBar(fileNumber);
        fileNumber++;
    }

    file_.close();
    emit exportFinished();
    return true;
}

bool DaveFileHandlerBase::packAndExport(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) {
    QDir sourceDirectory(sourceDirectoryPath);
    QString exportFilePath = exportDirectoryPath + "/" + baseName() + ".DAT";

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
    emit setProgressBarMax(fileEntries.size());
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
        emit updateProgressBar(i + 1);
    }

    // Pad last file
    exportFile.seek(fileOffset - 1);
    exportFile.write("\0", 1);

    // Calculate name offsets

    QList<quint32> nameOffsets = calculateNameOffsets(fileNames);

    // Write header
    exportFile.seek(0);
    exportFile.write(fileMagicBytes_.toUtf8(), 4);
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

    QMessageBox::information(nullptr, "Success",
        QString("Archive built successfully at:\n%1\n\nFrom source folder:\n%2")
        .arg(exportFilePath, sourceDirectory.path()));

    emit exportFinished();
    emit packingFinished();
    initItemModel(true);
    return true;
}

QByteArray DaveFileHandlerBase::decompressFile(const QByteArray &fileFata, const quint32 decompressedSize) const {
    QByteArray outBytes;
    outBytes.resize(decompressedSize);

    z_stream zStream = {};
    zStream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(fileFata.data()));
    zStream.avail_in = fileFata.size();
    zStream.next_out = reinterpret_cast<Bytef*>(outBytes.data());
    zStream.avail_out = outBytes.size();

    // -15 = raw deflate
    if (inflateInit2(&zStream, -15) != Z_OK)
        return {};

    if (inflate(&zStream, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&zStream);
        return {};
    }

    outBytes.resize(zStream.total_out);
    inflateEnd(&zStream);
    return outBytes;
}

QByteArray DaveFileHandlerBase::compressFile(const QByteArray &fileFata) const {
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

quint32 DaveFileHandlerBase::calculateAlign(quint32 size, quint32 align) const {
    if (align == 0)
        return size;
    return ((size / align) + 1) * align;
}
