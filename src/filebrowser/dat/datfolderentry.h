#pragma once
#include <QDir>
#include <QFileInfo>
#include <QStandardItem>


class DATFolderEntry : public QStandardItem{
public:
    explicit DATFolderEntry(const QString &virtualFilePath);
    ~DATFolderEntry() override;

private:
    QDir virtualDirectory_;
};
