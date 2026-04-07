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

    for (QString& spinnerChar : spinnerChars_)
        spinnerIcons_.append(createTextIcon(spinnerChar));

    connect(ui->fileTabWidget, &QTabWidget::tabCloseRequested, this, &FileBrowser::tabCloseRequested);
    connect(ui->fileTabWidget, &QTabWidget::currentChanged, this, [this] {
        setLineEditTexts();
    });
    connect(ui->packLineEdit, &QLineEdit::textChanged, this, [this] {
        ui->packButton->setEnabled(canPack());
    });
    connect(ui->exportButton, &QPushButton::clicked, this, [this] {
        auto* fileView = dynamic_cast<FileView*>(ui->fileTabWidget->currentWidget());
        if (fileView) {
            toggleTabLoadingIndicator(ui->fileTabWidget->currentIndex(), true);
            connect(fileView, &FileView::filesExported, this, [this] {
                toggleTabLoadingIndicator(ui->fileTabWidget->currentIndex(), false);
                ui->packButton->setEnabled(canPack());
            });
            fileView->exportFiles();
        }
    });
    connect(ui->packButton, &QPushButton::clicked, this, [this] {
        auto* fileView = dynamic_cast<FileView*>(ui->fileTabWidget->currentWidget());
        if (fileView) {
            toggleTabLoadingIndicator(ui->fileTabWidget->currentIndex(), true);
            connect(fileView, &FileView::filesPacked, this, [this] {
                toggleTabLoadingIndicator(ui->fileTabWidget->currentIndex(), false);
                ui->packButton->setEnabled(canPack());
            });
            fileView->packFiles();
        }
    });
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

        auto* newFileView = new FileView(url.toLocalFile(), ui->fileTabWidget);
        if (!newFileView->isValid()) {
            newFileView->deleteLater();
            continue;
        }

        QString fileName = QFileInfo(url.toLocalFile()).fileName();
        int newTabIndex = ui->fileTabWidget->addTab(newFileView, fileName);
        toggleTabLoadingIndicator(newTabIndex, true);
        ui->fileTabWidget->setCurrentIndex(newTabIndex);
        ui->fileTabWidget->tabBar()->setTabToolTip(newTabIndex, url.toLocalFile());

        connect(newFileView, &FileView::fileLoaded, this, [this, newFileView, url, newTabIndex](bool success) {
            toggleTabLoadingIndicator(newTabIndex, false);
            setLineEditTexts();

            if (!success) {
                QMessageBox::warning(this, "Couldn't Load File", QString("Unable to load file:\n%1").arg(url.toLocalFile()));
                newFileView->deleteLater();
                ui->fileTabWidget->removeTab(newTabIndex);
            }
        });

        newFileView->loadFile();
    }
}

void FileBrowser::tabCloseRequested(quint32 index) {
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

void FileBrowser::toggleTabLoadingIndicator(quint32 tabIndex, bool enabled) {
    if (enabled) {
        auto* timer = new QTimer(this);
        loadingTimers_[tabIndex] = timer;

        int frame = 0;
        connect(timer, &QTimer::timeout, this, [this, tabIndex, frame]() mutable {
            if (tabIndex >= ui->fileTabWidget->count()) {
                toggleTabLoadingIndicator(tabIndex, false);
                return;
            }

            ui->fileTabWidget->setTabIcon(tabIndex, spinnerIcons_[frame % spinnerIcons_.size()]);
            frame++;
        });

        timer->start(60);
        ui->exportButton->setEnabled(false);
    }
    else {
        if (loadingTimers_.contains(tabIndex)) {
            loadingTimers_[tabIndex]->stop();
            loadingTimers_[tabIndex]->deleteLater();
            loadingTimers_.remove(tabIndex);

            ui->fileTabWidget->setTabIcon(tabIndex, QIcon());
            ui->exportButton->setEnabled(true);
        }
    }

    toggleTabCloseButton(tabIndex, !enabled);
}

QIcon FileBrowser::createTextIcon(const QString &text) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = painter.font();
    font.setPointSize(16);
    painter.setFont(font);

    painter.setPen(palette().color(QPalette::Accent));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

    return QIcon(pixmap);
}

void FileBrowser::toggleTabCloseButton(quint32 tabIndex, bool enabled) {
    QTabBar* bar = ui->fileTabWidget->tabBar();
    QWidget* closeButton = bar->tabButton(tabIndex, QTabBar::RightSide);
    if (closeButton)
        enabled ? closeButton->show() : closeButton->hide();
}

void FileBrowser::setLineEditTexts() {
    auto* fileView = dynamic_cast<FileView*>(ui->fileTabWidget->currentWidget());
    if (fileView) {
        ui->exportLineEdit->setText(fileView->exportDirectory());
        ui->packLineEdit->setText(fileView->packDirectory());
    }
    else {
        ui->exportLineEdit->clear();
        ui->packLineEdit->clear();
    }

    bool enableExport = !ui->exportLineEdit->text().isEmpty() && !loadingTimers_.contains(ui->fileTabWidget->currentIndex());
    ui->exportButton->setEnabled(enableExport);
    ui->packButton->setEnabled(canPack());
}

bool FileBrowser::canPack() const {
    QString packText = ui->packLineEdit->text();
    return QFileInfo(packText).isDir() && !loadingTimers_.contains(ui->fileTabWidget->currentIndex());
}
