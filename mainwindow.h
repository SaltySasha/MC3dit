#ifndef MC3DIT_MAINWINDOW_H
#define MC3DIT_MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>


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

    QString AssetPath;

private:
    Ui::MainWindow *ui;
    QProcess* Process = new QProcess(this);

    void OnUnpackButtonClicked();
    void OnPackButtonClicked();
};


#endif //MC3DIT_MAINWINDOW_H