#include <QApplication>
#include <QFileDialog>
#include <QDir>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");

    QApplication App(argc, argv);
    App.setWindowIcon(QIcon("Icon.png"));
    MainWindow MainWindow;
    MainWindow.show();
    return QApplication::exec();
}
