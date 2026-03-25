#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE

namespace Ui {
    class FileBrowser;
}

QT_END_NAMESPACE

class FileBrowser : public QWidget {
    Q_OBJECT

public:
    explicit FileBrowser(QWidget *parent = nullptr);
    ~FileBrowser() override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void tabCloseRequested(int index);

    bool tabExists(const QString &filePath, bool setCurrent = false) const;

private:
    Ui::FileBrowser *ui;

    QMap<int, QTimer*> loadingTimers_;
    QList<QString> spinnerChars_ = {
        "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
    };

    void startTabLoadingIndicator(int tabIndex);
    void stopTabLoadingIndicator(int tabIndex);
    QIcon createTextIcon(const QString& text);
    void toggleTabCloseButton(int tabIndex, bool enabled);
};