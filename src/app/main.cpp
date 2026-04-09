#include <QApplication>
#include <QFileDialog>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
// #include <QtMultimedia/QAudioSink>

#include "../filebrowser/ui/filebrowser.h"
#include "ui/mainwindow.h"

constexpr auto APP_SERVER_NAME = "MC3ditServer";

bool sendMessageToRunningInstance(const QStringList &args)
{
    QLocalSocket socket;
    socket.connectToServer(APP_SERVER_NAME, QIODevice::WriteOnly);
    if (!socket.waitForConnected(100)) {
        return false; // no other instance running
    }

    QByteArray data = args.join('\n').toUtf8();
    socket.write(data);
    socket.flush();
    socket.waitForBytesWritten(100);
    socket.disconnectFromServer();
    return true;
}

int main(int argc, char *argv[]) {
    // This line is needed for CLion to pick up qDebug output
#ifdef QT_DEBUG
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");
#endif

    QApplication app(argc, argv);

    QStringList args = app.arguments();
    // if (sendMessageToRunningInstance(args))
    //     return 0;

    // Remove stale socket if needed
    // QLocalServer::removeServer(APP_SERVER_NAME);
    // QLocalServer server;
    // if (!server.listen(APP_SERVER_NAME)) {
    //     qWarning() << "Failed to listen on server:" << server.errorString();
    // }

    app.setWindowIcon(QIcon(":/icon"));

    MainWindow mainWindow;
    mainWindow.show();

    // Otherwise, start listening for future launches
    // QObject::connect(&server, &QLocalServer::newConnection, [&]() {
    //     QLocalSocket *client = server.nextPendingConnection();
    //     if (client->waitForReadyRead(500)) {
    //         QString message = QString::fromUtf8(client->readAll());
    //         QStringList paths = message.split('\n', Qt::SkipEmptyParts);
    //
    //         for (QString& path : paths)
    //             continue;
    //             // mainWindow.tryOpenFile(path);
    //     }
    //     client->deleteLater();
    // });
    //
    // for (QString& arg : app.arguments())
        // continue;
        // mainWindow.tryOpenFile(arg);

    return QApplication::exec();
}
