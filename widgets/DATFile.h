#pragma once

#include "dcfile.h"
#include <QFileInfo>
#include <QProcess>
#include <QStandardItemModel>
#include <QTreeView>

class DATFile : public QTreeView {
    Q_OBJECT

public:
    explicit DATFile(const QString &filePath);

    bool readDaveFile();
    void unpackFiles(const QString& folderPath);
    void packFiles(const QString& folderPath) const;

    void setUnpackDir(const QString &folderPath) {unpackDirectory_ = folderPath;}
    void setPackPath(const QString &filePath) {packPath_ = filePath;}

    [[nodiscard]] QString GetFileName(bool IsFullFileName = true) const {return IsFullFileName ? fileInfo_.fileName() : fileInfo_.fileName().left(fileInfo_.fileName().length() - 4);}
    [[nodiscard]] QString fileType() const {return fileType_;}
    [[nodiscard]] QString filePath() const {return fileInfo_.absoluteFilePath();}
    [[nodiscard]] QString fileDirectory() const {return fileInfo_.absolutePath();}
    [[nodiscard]] QString makeFileDirectory() const {return fileInfo_.absoluteFilePath().left(fileInfo_.absoluteFilePath().length() - 4);}
    [[nodiscard]] QString unpackDirectory() const {return unpackDirectory_;}
    [[nodiscard]] QString packPath() const {return packPath_;}

signals:
    void setProgressBarMax(quint32 newMax);
    void updateProgressBar(quint32 newValue);
    void exportFinished();

private:
    const QStringList headerList_ = {"Dave", "DAVE"}; // TODO, "Hash"}; - Add Hash file type support
    const char usableChars_[69] = "\x00 #$()-./?0123456789_abcdefghijklmnopqrstuvwxyz~\x7F";

    QFileInfo fileInfo_;
    QStandardItemModel *itemModel_;
    QList<QSharedPointer<DCFile>> files_;
    QString unpackDirectory_;
    QString packPath_;

    QString fileType_;
    quint32 entries_{};
    quint32 fileSize_{};
    quint32 fileNameSize_{};

    void openContextMenu();
    void exportSingleFile();

    void addVirtualPath(const QString& virtualPath, quint32 nameOffset, quint32 fileOffset, quint32 fileSizeFull, quint32 fileSizeCompressed);

    QByteArray decompress(const QByteArray &data, quint32 decompressedSize) const;
    quint32 toLittleEndian(const QByteArray &byteArray) const;
    QString readName(QFile &file, const QString &fileType, const QString &fileName) const;
    QList<quint32> unpackSixBitValues(QFile &file) const;
};
