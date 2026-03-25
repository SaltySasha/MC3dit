#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QFileSystemModel>


QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void tryOpenFile(const QString &filePath);
    void setButtonsEnabled(bool enabled) const;

private:
    Ui::MainWindow *ui;

    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onUnpackButtonClicked();
    void onPackButtonClicked();
    void onPackBrowseButtonClicked();
    void onUnpackBrowseButtonClicked();

    void refreshButtons();
    void lockUi(bool locked);
    void setUnpackDirectory(const QString &directory);
    void setPackDirectory(const QString &directory);

    void testMethod();
};