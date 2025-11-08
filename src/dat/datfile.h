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


signals:
    // void setProgressBarMax(quint32 newMax);
    // void updateProgressBar(quint32 newValue);
    // void exportFinished();

private:
    explicit DATFile(const QString &filePath);
    IDATFileHandler* fileHandler_;

    void openContextMenu();
    void exportSingleFile();
};
