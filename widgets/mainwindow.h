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

    void dragEnterEvent(QDragEnterEvent *Event) override;
    void dragMoveEvent(QDragMoveEvent *Event) override;
    void dropEvent(QDropEvent *Event) override;

    void SetButtonLock(bool AreButtonsLocked);

    QString AssetPath;

private:
    Ui::MainWindow *MainUI;
    QProcess* Process = new QProcess(this);

    void OnUnpackButtonClicked();
    void OnPackButtonClicked();
    void OnPackBrowseButtonClicked();
    void OnUnpackBrowseButtonClicked();

    QTimer* ButtonAnimationTimer = new QTimer(this);
    void PlayButtonAnimation(QPushButton* InButton, const QString& InButtonText);
    void ResetButton(QPushButton* InButton, const QString& InButtonText);

    QMap<QWidget*, QTreeView*> TabToTreeViewMap;
    QMap<QWidget*, QStandardItemModel*> TabToModelMap;
};