#pragma once

#include <QFileInfo>
#include <QStandardItemModel>
#include <QTreeView>


class IFileHandler;

class FileView : public QTreeView {
    Q_OBJECT

public:
    explicit FileView(QWidget *parent = nullptr, const QString& filePath = "");
    ~FileView() override;

    bool isValid() const {return fileHandler_ != nullptr;}
    // static FileView* create(const QString &filePath);

    // IFileHandler* fileHandler() const {return fileHandler_;}
    // QString unpackDirectory() const {return unpackDirectory_;}
    // QString packDirectory() const {return packDirectory_;}
    // void setUnpackDirectory(const QString &unpackDirectory) {unpackDirectory_ = unpackDirectory;}
    // void setPackDirectory(const QString &packDirectory) {packDirectory_ = packDirectory;}


signals:
    void fileLoaded(bool success);
    void progressChanged(int current, int total);
    // void setProgressBarMax(quint32 newMax);
    // void updateProgressBar(quint32 newValue);
    // void exportFinished();

protected:
    std::unique_ptr<IFileHandler> fileHandler_;
    QStandardItemModel* model_;

    // QString unpackDirectory_;
    // QString packDirectory_;
    //
    // void openContextMenu();
    // void exportSingleFile();
};
