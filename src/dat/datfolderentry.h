#pragma once
#include <QDir>
#include <QFileInfo>
#include <QStandardItem>


class DATFolderEntry : public QStandardItem, QObject{
public:
    explicit DATFolderEntry(const QString &virtualFilePath, QObject *parent = nullptr);

private:
    QDir virtualDirectory_;
};
