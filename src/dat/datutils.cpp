#include "datutils.h"

#include <qendian.h>
#include <QMessageBox>

#include "handlers/davelowerfilehandler.h"
#include "handlers/daveupperfilehandler.h"
#include "handlers/hashfilehandler.h"

namespace DATUtils {
    quint32 toLittleEndian(const QByteArray &byteArray) {
        return qFromLittleEndian<quint32>(byteArray.constData());
    }

    QList<quint32> unpackSixBitValues(const QByteArray &byteArray) {
        QList<quint32> result;
        result.reserve(4);

        const quint32 compressedData = toLittleEndian(byteArray);
        for (quint32 i = 0; i < 4; i++)
            result.append(compressedData >> (i * 6) & 0x3f);

        return result;
    }

    QByteArray getIntAsBytes(const boost::multiprecision::cpp_int &value, int bytes) {
        QByteArray result;

        export_bits(value, std::front_inserter(result), 8, true);

        if (result.size() < bytes) {
            int paddingNeeded = bytes - result.size();
            result.append(QByteArray(paddingNeeded, '\x00'));
        }

        return result;
    }

    void messageFileNotFound(const QString &fileName, const QString &errorString) {
        QMessageBox::warning(nullptr, "Unable To Open File", QString("Could not open %1 with reason:\n %2").arg(fileName, errorString));
    }
}

namespace DATFileFactory {
    IDATFileHandler * createHandler(const QString &filePath) {
        QFile file(filePath);
        if (!file.exists()) {
            QMessageBox::warning(nullptr, "No File", QString("Could not find %1 with reason:\n %2").arg(file.fileName(), file.errorString()));
            return nullptr;
        }

        if (!file.open(QIODeviceBase::ReadOnly)) {
            QMessageBox::warning(nullptr, "Unable To Open File", QString("Could not open %1 with reason:\n %2").arg(file.fileName(), file.errorString()));
            return nullptr;
        }

        QString fileSignature = file.read(4);
        file.close();

        if (!allowedSignatureList_.contains(fileSignature)) {
            QMessageBox::warning(nullptr, "Unsupported File Signature", QString("The file signature for %1 is not supported.").arg(file.fileName()));
            return nullptr;
        }

        // Create handler based on signature
        if (fileSignature == "Dave")
            return new DaveLowerFileHandler(filePath);

        if (fileSignature == "DAVE")
            return new DaveUpperFileHandler(filePath);

        if (fileSignature == "Hash")
            return new HashFileHandler(filePath);

        return nullptr;
    }
}
