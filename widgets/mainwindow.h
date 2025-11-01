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

    void setButtonLock(bool locked) const;

private:
    Ui::MainWindow *ui;
    QProcess* process_ = new QProcess(this);

    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onUnpackButtonClicked();
    void onPackButtonClicked();
    void onPackBrowseButtonClicked();
    void onUnpackBrowseButtonClicked();

    QTimer* buttonAnimationTimer = new QTimer(this);
    void playButtonAnimation(QPushButton* button, const QString& buttonText);
    void resetButton(QPushButton* button, const QString& buttonText);

    void testMethod();
};