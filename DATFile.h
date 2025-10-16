#ifndef MC3DIT_DATFile_H
#define MC3DIT_DATFile_H

#include <QFileInfo>
#include <QStandardItemModel>
#include <QTreeView>

class DATFile : public QTreeView {
    Q_OBJECT

public:
    explicit DATFile(const QString &InFilePath);

    void ReadDaveFile();
    QString GetFileName() const {return FileInfo.fileName();}
    QString GetFileType() const {return FileType;}
    QString GetFilePath() const {return FileInfo.absoluteFilePath();}
    QString GetFileDirectory() const {return FileInfo.absolutePath();}
    QString MakeFileDirectory() const {return FileInfo.absoluteFilePath().left(FileInfo.absoluteFilePath().length() - 4);}

signals:
    void SetProgressBarMax(quint32 NewMax);
    void UpdateProgressBar(quint32 NewValue);

private:
    const QStringList DATHeaderList = {"Dave", "DAVE"}; // TODO, "Hash"}; - Add Hash file type support
    const char DaveUsableChars[69] = "\x00 #$()-./?0123456789_abcdefghijklmnopqrstuvwxyz~\x7F";

    QFileInfo FileInfo;
    QStandardItemModel *ItemModel;

    QString FileType;
    quint32 Entries;
    quint32 FileSize;
    quint32 FileNameSize;

    QString ReadString(QDataStream &InStream);
    QList<qint32> ReadBits(QDataStream &InStream);
    void AddVirtualPath(const QString& VirtualPath) const;

};


#endif //MC3DIT_DATFile_H