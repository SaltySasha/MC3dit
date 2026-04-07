#pragma once
#include <QFuture>
#include <QVector>

#include "../dat/datutils.h"
#include "../files/entryitem.h"


class IFileHandler : public QObject {
    Q_OBJECT

public:
    virtual bool parseFile() = 0;
    virtual bool exportFiles(const QString& exportDirectory) = 0;
    virtual bool packFiles(const QString& packDirectory, const QString& sourceDirectory) {return false;}; //TODO: FIX THIS
    virtual void populateModel(QStandardItem* rootItem);

    void setFileInfo(const QFileInfo &fileInfo) {fileInfo_ = fileInfo;}
    [[nodiscard]] QFileInfo getFileInfo() const {return fileInfo_;}

signals:
    void parseFinished(bool success);
    void populationFinished();

protected:
    const QString signature_ = "SaltyWasHere:)";
    QString fileMagic_ = "";
    QFileInfo fileInfo_;
    QHash<QString, EntryItem*> pathCache_;
    QList<EntryInfo> entryInfoList_;

    void addVirtualPath(QStandardItem* rootItem, const EntryInfo &entryInfo);
};