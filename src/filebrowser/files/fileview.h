#pragma once

#include <QFileInfo>
#include <QStandardItemModel>
#include <QTreeView>


class IFileHandler;

class FileView : public QTreeView {
    Q_OBJECT

public:
    explicit FileView(const QString &filePath, QWidget *parent = nullptr);
    void loadFile();
    void exportFiles();
    void packFiles();
    [[nodiscard]] bool isValid() const {return fileHandler_ != nullptr;}
    [[nodiscard]] QString exportDirectory() const {return exportDirectory_;}
    [[nodiscard]] QString packDirectory() const {return packDirectory_;}

    // static FileView* create(const QString &filePath);

    // IFileHandler* fileHandler() const {return fileHandler_;}

    // void setUnpackDirectory(const QString &unpackDirectory) {unpackDirectory_ = unpackDirectory;}
    // void setPackDirectory(const QString &packDirectory) {packDirectory_ = packDirectory;}


signals:
    void fileLoaded(bool success);
    void filesExported(bool success);
    void filesPacked(bool success);
    // void setProgressBarMax(quint32 newMax);
    // void updateProgressBar(quint32 newValue);
    // void exportFinished();

protected:
    IFileHandler* fileHandler_ = nullptr;
    QStandardItemModel* model_ = nullptr;
    QString exportDirectory_;
    QString packDirectory_;

    // void openContextMenu();
    // void exportSingleFile();
};
