#include "hashfilehandler.h"

#include <QDir>
#include <QDirIterator>
#include <QMessageBox>

#include "../datefileentry.h"

using namespace DATUtils;

bool HashFileHandler::readFile() {
    quint32 signatureOffset = entryCount_ * 12 + file_.pos();
    file_.seek(signatureOffset);
    QString signatureInFile = file_.read(16).data();

    QMap<quint32, QString>  fileNames;
    if (signatureInFile == signature_) {
        for (quint32 i = 0; i < entryCount_; i++) {
            QString fileName = readString();
            quint32 fileHash = toLittleEndian(file_.read(4));
            fileNames.insert(fileHash, fileName);
        }
    }
    else {
        QFile nameList(":/external/MC3_PS2_Streams.lst");
        if (!nameList.open(QIODevice::ReadOnly)) {
            messageFileNotFound(file_.fileName(), file_.errorString());
            return false;
        }

        QTextStream stream(&nameList);

        stream.readLine();

        QString currentLine;
        while (!stream.atEnd()) {
            currentLine = stream.readLine();
            fileNames.insert(calculateHash(currentLine), currentLine);
        }
    }

    if (fileNames.isEmpty())
        return false;

    file_.seek(8);

    FileEntry newEntry;
    emit setProgressBarMax(entryCount_);
    for (quint32 i = 0; i < entryCount_; i++) {
        newEntry.hash = toLittleEndian(file_.read(4));
        newEntry.fileOffset = toLittleEndian(file_.read(4));
        newEntry.sizeFull = toLittleEndian(file_.read(4));
        if (fileNames.contains(newEntry.hash))
            newEntry.setFile(fileNames[newEntry.hash]);
        else
            newEntry.setFile("UknownFiles/" + QString::number(newEntry.hash) + ".rsm");

        addVirtualPath(newEntry);
        updateProgressBar(i + 1);
        //qDebug() << newEntry.fileName() << newEntry.hash << newEntry.fileOffset << newEntry.sizeFull;
    }

    file_.close();
    emit readFinished(true);

    // std::ranges::sort(files_, [](const DATFileEntry* a, const DATFileEntry* b) {
    //     return a->fileOffset() < b->fileOffset();
    // });
    //
    // for (auto file : files_)
    //     qDebug() << file->filePath() << file->fileOffset();
    return true;
}

bool HashFileHandler::unpackAndExport(const QString &exportDirectory) {
    if (!file_.open(QIODeviceBase::ReadOnly)) {
        messageFileNotFound(file_.fileName(), file_.errorString());
        return false;
    }

    emit setProgressBarMax(files_.length());

    quint32 fileNumber = 1;
    for (const DATFileEntry *file : files_) {
        file_.seek(file->fileOffset());
        QByteArray fileData = file_.read(file->sizeFull());
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

bool HashFileHandler::packAndExport(const QString &exportDirectoryPath, const QString &sourceDirectoryPath) {
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
    FileEntry newEntry;
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

        newEntry.setFile(filePath);
        newEntry.hash = calculateHash(relativePath);
        newEntry.sizeFull = fileInfo.size();
        newEntry.relativePath = relativePath;
        fileEntries.append(newEntry);
    }

    for (auto &entry : fileEntries)
        qDebug() << entry.relativePath;
    sortFiles(fileEntries);

    QList<QByteArray> fileNames;
    prepareFileBlock(fileEntries, fileNames);

    QByteArray allNameBytes;
    for (const QByteArray& name : fileNames)
        allNameBytes.append(name);
    const quint32 nameBlockSize = 8 + fileEntries.size() * 16 + allNameBytes.size() + signature_.length() + 2;

    // Create output directory if needed
    QFileInfo exportFileInfo(exportFile);
    QDir().mkpath(exportFileInfo.absolutePath());

    // Open output file
    if (!exportFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error",
                                QString("Could not create output file: %1").arg(exportFile.errorString()));
        return false;
    }

    exportFile.write(fileMagicBytes_.toUtf8());
    exportFile.write(getIntAsBytes(fileEntries.size(), 4));

    quint32 fileOffset = (nameBlockSize + 6144 - 1) / 6144 * 6144;
    quint32 customNameOffset = fileEntries.size() * 12 + 8;
    exportFile.seek(customNameOffset);
    exportFile.write(signature_.toUtf8());
    customNameOffset += signature_.length() + 2;
    emit setProgressBarMax(fileEntries.size());
    for (quint32 i = 0; i < fileEntries.size(); i++) {
        FileEntry &entry = fileEntries[i];
        exportFile.seek(i * 12 + 8);
        exportFile.write(getIntAsBytes(entry.hash, 4));
        exportFile.write(getIntAsBytes(fileOffset, 4));
        exportFile.write(getIntAsBytes(entry.sizeFull, 4));

        exportFile.seek(customNameOffset);
        exportFile.write(fileNames[i]);
        exportFile.write(getIntAsBytes(entry.hash, 4));
        customNameOffset = exportFile.pos();


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

        exportFile.seek((fileOffset + 6144 - 1) / 6144 * 6144);
        exportFile.write(entryFileData);
        fileOffset += entry.sizeFull;
        emit updateProgressBar(i + 1);
    }

    exportFile.close();
    return true;
}

bool HashFileHandler::validateChars(const QString &filePath) const {
    for (const QChar& c : filePath) {
        if (c.unicode() > 127) {
            QMessageBox::critical(nullptr, "Error",
                QString("Non-ASCII character '%1' in filename: %2").arg(c, filePath));
            return false;
        }
    }

    return true;
}

bool HashFileHandler::sortFiles(QList<FileEntry> &fileList) const {
    std::ranges::sort(fileList, [](const FileEntry& a, const FileEntry& b) {
        return a.hash < b.hash;
    });

    return true;
}

bool HashFileHandler::prepareFileBlock(const QList<FileEntry> &fileEntries, QList<QByteArray> &fileBytesList) const {
    for (const FileEntry& entry : fileEntries)
        fileBytesList.append(entry.relativePath.toUtf8() + '\0');

    return true;
}

quint32 HashFileHandler::calculateHash(const QString &fileData) const {
    quint32 outHash = 0;
    QString cleanedString = fileData.toUpper().toLatin1().replace('\\', '/');
    for (quint32 i = 0; i < cleanedString.size(); ++i) {
        outHash = (outHash << 1 | outHash >> 31) + cleanedString[i].toLatin1() * (i + 1);
    }
    //qDebug() << cleanedString << outHash;
    return outHash;
}

QByteArray HashFileHandler::readString() {
    QByteArray outBytes;
    char currentChar = file_.read(1)[0];
    while (currentChar != '\0') {
        outBytes.append(currentChar);
        currentChar = file_.read(1)[0];
    }

    //qDebug() << outBytes;
    return outBytes;
}