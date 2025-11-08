#pragma once

#include <QFileInfo>
#include <qtypes.h>
#include <boost/multiprecision/cpp_int.hpp>

class IDATFileHandler;

struct FileEntry {
    void setFile(const QString &filePath){fileInfo.setFile(filePath);}

    QString fileName() const {return fileInfo.fileName();}
    QString filePath() const {return fileInfo.filePath();}
    QString path() const {return fileInfo.path();}

    QFileInfo fileInfo;
    QString relativePath;
    quint32 nameOffset = 0;
    quint32 fileOffset = 0;
    quint32 sizeFull = 0;
    quint32 sizeCompressed = 0;
};

namespace DATUtils {
    quint32 toLittleEndian(const QByteArray &byteArray);
    QList<quint32> unpackSixBitValues(const QByteArray &byteArray);
    QByteArray getIntAsBytes(const boost::multiprecision::cpp_int &value, int bytes);
    void messageFileNotFound(const QString &fileName, const QString &errorString);
};

// Factory to create the appropriate handler
namespace DATFileFactory {
    IDATFileHandler* createHandler(const QString &filePath);
    inline const QStringList allowedSignatureList_ = {"Dave", "DAVE"}; //TODO:, "Hash"};
};