#include "filehandlerfactory.h"

#include <QDebug>
#include <QFileInfo>
#include <QTabWidget>
#include <QString>

#include "ifilehandler.h"

IFileHandler* FileHandleFactory::createHandler(const QString &filePath, QObject *parent) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file:" << filePath << "with error:" << file.errorString();

        return nullptr;
    }
    QString magic = file.read(4);
    file.close();

    auto iterator = handlerMap_.find(magic);
    if (iterator != handlerMap_.end()) {
        std::function<IFileHandler*()> factory = iterator.value();
        IFileHandler* handler = factory();
        handler->setFileInfo(QFileInfo(filePath));
        handler->setParent(parent);
        return handler;
    }

    return nullptr;
}

bool FileHandleFactory::registerHandler(const QString& magic, const HandlerFactory& factory) {;
    if (handlerMap_.contains(magic)) {
        qWarning() << "Handler already registered for magic bytes:" << magic;
        return false;
    }
    handlerMap_[magic] = factory;
    return true;
}
