#include "mainwindow.h"

#include "../../filebrowser/ui/filebrowser.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    auto fileBrowser = new FileBrowser(this);
    ui->tabWidget->addTab(fileBrowser, "File Browser");
    ui->tabWidget->setTabToolTip(0, "Browse DAT file!");
    ui->tabWidget->addTab(new QWidget(), "Music Tool");
    ui->tabWidget->setTabToolTip(1, "Coming soon, check back later! ;)");
    ui->tabWidget->setTabEnabled(1, false);
}

MainWindow::~MainWindow() {
    delete ui;
}