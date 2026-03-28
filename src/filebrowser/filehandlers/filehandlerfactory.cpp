#include "filehandlerfactory.h"

#include <QDebug>
#include <QFileInfo>
#include <QTabWidget>
#include <QString>

#include "ifilehandler.h"

std::unique_ptr<IFileHandler> FileHandleFactory::createHandler(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file:" << filePath << "with error:" << file.errorString();

        return nullptr;
    }
    QString magic = file.read(4);
    file.close();

    auto iterator = handlerMap_.find(magic);
    if (iterator != handlerMap_.end()) {
        std::function<std::unique_ptr<IFileHandler>()> factory = iterator.value();
        std::unique_ptr<IFileHandler> handler = factory();
        handler->setFileInfo(QFileInfo(filePath));
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
