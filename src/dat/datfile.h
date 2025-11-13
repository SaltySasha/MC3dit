#pragma once

#include <QFileInfo>
#include <QTreeView>


class IDATFileHandler;

class DATFile : public QTreeView {
    Q_OBJECT

public:
    static DATFile* create(const QString &filePath);
    ~DATFile() override;
    IDATFileHandler* fileHandler() const {return fileHandler_;}
    QString unpackDirectory() const {return unpackDirectory_;}
    QString packDirectory() const {return packDirectory_;}
    void setUnpackDirectory(const QString &unpackDirectory) {unpackDirectory_ = unpackDirectory;}
    void setPackDirectory(const QString &packDirectory) {packDirectory_ = packDirectory;}


signals:
    // void setProgressBarMax(quint32 newMax);
    // void updateProgressBar(quint32 newValue);
    // void exportFinished();

private:
    explicit DATFile(const QString &filePath);
    IDATFileHandler* fileHandler_;
    QString unpackDirectory_;
    QString packDirectory_;

    void openContextMenu();
    void exportSingleFile();
};
