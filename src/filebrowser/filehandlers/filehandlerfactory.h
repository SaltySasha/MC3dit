#pragma once

#include "ifilehandler.h"

class QTabWidget;
class IFileHandler;

class FileHandleFactory {
public:
    static FileHandleFactory& instance() {
        static FileHandleFactory registry;
        return registry;
    }

    using HandlerFactory = std::function<std::unique_ptr<IFileHandler>()>;
    std::unique_ptr<IFileHandler> createHandler(const QString& filePath);
    bool registerHandler(const QString& magic, const HandlerFactory &factory);

private:
    QMap<QString, HandlerFactory> handlerMap_;
};

template<typename HandlerType>
class FileHandlerRegistrar {
public:
    explicit FileHandlerRegistrar(const QString& magic) {
        FileHandleFactory::instance().registerHandler(magic, []() {
            return std::make_unique<HandlerType>();
        });
    }
};

#define REGISTER_FILE_HANDLER(HandlerClass, Magic) \
    static FileHandlerRegistrar<HandlerClass> s_##HandlerClass##Registrar(Magic)
