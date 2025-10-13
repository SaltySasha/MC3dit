#include <QApplication>
#include <QPushButton>
#include <QProcess>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QMainWindow>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");

    QApplication a(argc, argv);
    MainWindow MainWindow;
    MainWindow.show();
    return QApplication::exec();
}
