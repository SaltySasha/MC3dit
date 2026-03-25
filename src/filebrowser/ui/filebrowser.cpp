#include "filebrowser.h"

#include <QPainter>
#include <QMessageBox>
#include <QtEvents>
#include <QMimeData>
#include <QTimer>

#include "ui_filebrowser.h"
#include "../files/fileview.h"

FileBrowser::FileBrowser(QWidget *parent)
    : QWidget(parent), ui(new Ui::FileBrowser) {
    ui->setupUi(this);

    connect(ui->fileTabWidget, &QTabWidget::tabCloseRequested, this, &FileBrowser::tabCloseRequested);
}

FileBrowser::~FileBrowser() {
    delete ui;
}

void FileBrowser::dragEnterEvent(QDragEnterEvent *event) {
    QWidget::dragEnterEvent(event);
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    event->accept();
}

void FileBrowser::dragMoveEvent(QDragMoveEvent *event) {
    QWidget::dragMoveEvent(event);
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void FileBrowser::dropEvent(QDropEvent *event) {
    QWidget::dropEvent(event);

    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();

    for (const QUrl& url : event->mimeData()->urls()) {
        if (!url.isLocalFile()) {
            QMessageBox::warning(this, "Non Local File", QString("Not a local file:\n%1").arg(url.url()));
            continue;
        }

        // TODO: Ability to open folders
        // if (QFileInfo(url.toLocalFile()).isDir()) {
        //     QMessageBox::question(this, "Open Folder", QString("Do you want to proceed with opening all files from\n%1").arg(url.toLocalFile()));
        // }

        if (tabExists(url.toLocalFile(), true))
            continue;

        auto* newFileView = new FileView(ui->fileTabWidget, url.toLocalFile());
        if (!newFileView->isValid())
            continue;

        QString fileName = QFileInfo(url.toLocalFile()).fileName();
        int newTabIndex = ui->fileTabWidget->addTab(newFileView, fileName);
        ui->fileTabWidget->setCurrentIndex(newTabIndex);

        startTabLoadingIndicator(newTabIndex);
        toggleTabCloseButton(newTabIndex, false);

        connect(newFileView, &FileView::fileLoaded, this, [this, newFileView, url, newTabIndex](bool success) {
            stopTabLoadingIndicator(newTabIndex);
            toggleTabCloseButton(newTabIndex, true);

            if (success) {
                ui->fileTabWidget->tabBar()->setTabToolTip(newTabIndex, url.toLocalFile());
            } else {
                QMessageBox::warning(this, "Couldn't Load File", QString("Unable to load file:\n%1").arg(url.toLocalFile()));
                newFileView->deleteLater();
                ui->fileTabWidget->removeTab(newTabIndex);
            }
        });
    }
}

void FileBrowser::tabCloseRequested(int index) {
    ui->fileTabWidget->widget(index)->deleteLater();
    ui->fileTabWidget->removeTab(index);
}

bool FileBrowser::tabExists(const QString &filePath, bool setCurrent) const {
    for (qint32 tabIndex = 0; tabIndex < ui->fileTabWidget->count(); tabIndex++) {
        if (ui->fileTabWidget->tabBar()->tabToolTip(tabIndex) == filePath) {
            if (setCurrent)
                ui->fileTabWidget->setCurrentIndex(tabIndex);
            return true;
        }
    }

    return false;
}

void FileBrowser::startTabLoadingIndicator(int tabIndex) {
    auto* timer = new QTimer(this);
    loadingTimers_[tabIndex] = timer;

    int frame = 0;
    connect(timer, &QTimer::timeout, this, [this, tabIndex, frame]() mutable {
        if (tabIndex >= ui->fileTabWidget->count()) {
            stopTabLoadingIndicator(tabIndex);
            return;
        }

        QString spinnerChar = spinnerChars_[frame % spinnerChars_.size()];
        ui->fileTabWidget->setTabIcon(tabIndex, createTextIcon(spinnerChar));
        frame++;
    });

    timer->start(60);
}

void FileBrowser::stopTabLoadingIndicator(int tabIndex) {
    if (loadingTimers_.contains(tabIndex)) {
        loadingTimers_[tabIndex]->stop();
        loadingTimers_[tabIndex]->deleteLater();
        loadingTimers_.remove(tabIndex);

        ui->fileTabWidget->setTabIcon(tabIndex, QIcon());
    }
}

QIcon FileBrowser::createTextIcon(const QString &text) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = painter.font();
    font.setPointSize(16);  // Adjust size as needed
    painter.setFont(font);

    painter.setPen(palette().color(QPalette::Accent));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

    return QIcon(pixmap);
}

void FileBrowser::toggleTabCloseButton(int tabIndex, bool enabled) {
    QTabBar* bar = ui->fileTabWidget->tabBar();
    QWidget* closeBtn = bar->tabButton(tabIndex, QTabBar::RightSide);
    if (closeBtn)
        enabled ? closeBtn->show() : closeBtn->hide();
}
