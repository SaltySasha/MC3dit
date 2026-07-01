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

    [[nodiscard]] bool tabExists(const QString &filePath, bool setCurrent = false) const;

protected:
    Ui::FileBrowser *ui;

    QMap<quint32, QTimer*> loadingTimers_;
    QList<QString> spinnerChars_ = {
        "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
    };
    QList<QIcon> spinnerIcons_;

    void toggleTabLoadingIndicator(quint32 tabIndex, bool enabled);
    QIcon createTextIcon(const QString& text);
    void toggleTabCloseButton(quint32 tabIndex, bool enabled);
    void setLineEditTexts();
    bool canPack() const;
};