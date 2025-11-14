#include <QApplication>
#include <QFileDialog>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>

#include "src/mainwindow.h"

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
    if (sendMessageToRunningInstance(args))
        return 0;

    //app.setWindowIcon(QIcon("icon.png"));
    MainWindow mainWindow;
    mainWindow.show();
    mainWindow.setWindowIcon(QIcon("icon.png"));

    // Otherwise, start listening for future launches
    QLocalServer server;
    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *client = server.nextPendingConnection();
        if (client->waitForReadyRead(500)) {
            QString message = QString::fromUtf8(client->readAll());
            QStringList paths = message.split('\n', Qt::SkipEmptyParts);

            for (QString& path : paths)
                mainWindow.tryOpenFile(path);
        }
        client->deleteLater();
    });

    // Remove stale socket if needed
    QLocalServer::removeServer(APP_SERVER_NAME);
    if (!server.listen(APP_SERVER_NAME)) {
        qWarning() << "Failed to listen on server:" << server.errorString();
    }


    for (QString& arg : app.arguments())
        mainWindow.tryOpenFile(arg);

    return QApplication::exec();
}
