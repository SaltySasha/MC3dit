#include <QApplication>
#include <QFileDialog>
#include <QDir>

#include "widgets/mainwindow.h"

int main(int argc, char *argv[]) {
    // This line is needed for CLion to pick up qDebug output
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");

    QApplication App(argc, argv);
    App.setWindowIcon(QIcon("icon.png"));
    MainWindow MainWindow;
    MainWindow.show();
    return QApplication::exec();
}
