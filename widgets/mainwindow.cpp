#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../misc/utils.h"
#include "DATFile.h"

#include <QDragLeaveEvent>
#include <QMimeData>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), MainUI(new Ui::MainWindow) {
    MainUI->setupUi(this);
    MainUI->ProgressBar->hide();
    MainUI->PackSection->hide();
    MainUI->UnpackSection->hide();

    connect(MainUI->UnpackButton, &QPushButton::clicked, this, &MainWindow::OnUnpackButtonClicked);
    connect(MainUI->PackButton, &QPushButton::clicked, this, &MainWindow::OnPackButtonClicked);
    connect(MainUI->BrowsePackButton, &QPushButton::clicked, this, &MainWindow::OnPackBrowseButtonClicked);
    connect(MainUI->BrowseUnpackButton, &QPushButton::clicked, this, &MainWindow::OnUnpackBrowseButtonClicked);


    connect(MainUI->TabWidget, &QTabWidget::currentChanged, [this](int Index) {
        if (auto *Widget = dynamic_cast<DATFile*>(MainUI->TabWidget->widget(Index))) {
            Widget->GetFileType() == "Dave" ? MainUI->PackSection->show() : MainUI->PackSection->hide();
            MainUI->UnpackSection->show();

            MainUI->UnpackLineEdit->setText(!Widget->GetUnpackDirectory().isEmpty() ? Widget->GetUnpackDirectory() : Widget->GetFileDirectory());
            MainUI->PackLineEdit->setText(!Widget->GetPackPath().isEmpty() ? Widget->GetPackPath() : Widget->GetFilePath());
        }
        else {
            MainUI->UnpackLineEdit->clear();
            MainUI->PackLineEdit->clear();
            MainUI->PackSection->hide();
            MainUI->UnpackSection->hide();
        }
    });
    connect(MainUI->TabWidget, &QTabWidget::tabCloseRequested, [this](const int Index) {
        auto TreeView = dynamic_cast<QTreeView*>(MainUI->TabWidget->widget(Index));

        if (TreeView) {
            static_cast<QStandardItemModel*>(TreeView->model())->clear();
            TreeView->model()->deleteLater();
            TreeView->setModel(nullptr);
            TreeView->deleteLater();
        }
    });
}

MainWindow::~MainWindow() {
    delete MainUI;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *Event) {
    QMainWindow::dragEnterEvent(Event);

    if (Event->mimeData()->hasUrls())
        Event->accept();
    else
        Event->ignore();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *Event) {
    QMainWindow::dragMoveEvent(Event);
    if (Event->mimeData()->hasUrls()) {
        Event->setDropAction(Qt::CopyAction);
        Event->accept();
    }
    else
        Event->ignore();
}

void MainWindow::dropEvent(QDropEvent *Event) {
    QMainWindow::dropEvent(Event);

    if (Event->mimeData()->hasUrls()) {
        Event->setDropAction(Qt::CopyAction);
        Event->accept();

        for (const auto& Url : Event->mimeData()->urls()) {
            if (Url.isLocalFile()) {
                QString File = Url.toLocalFile();
                for (qint32 TabIndex = 0; TabIndex < MainUI->TabWidget->count(); TabIndex++) {
                    auto* Tab = dynamic_cast<DATFile*>(MainUI->TabWidget->widget(TabIndex));
                    if (Tab->GetFilePath() == File) {
                        MainUI->TabWidget->setCurrentWidget(Tab);
                        return;
                    }
                }

                if (File.endsWith(".dat", Qt::CaseInsensitive)) {
                    AssetPath = Url.toLocalFile();

                    auto *NewDatFile = new DATFile(AssetPath);
                    connect(NewDatFile, &DATFile::SetProgressBarMax, this, [this](qint32 NewMax) {
                        MainUI->ProgressBar->show();
                        MainUI->ProgressBar->setMaximum(NewMax);
                    });
                    connect(NewDatFile, &DATFile::UpdateProgressBar, this, [this](qint32 NewProgressAmount) {
                        MainUI->ProgressBar->setValue(NewProgressAmount);
                        if (NewProgressAmount >= MainUI->ProgressBar->maximum())
                            MainUI->ProgressBar->hide();
                    });
                    connect(NewDatFile, &DATFile::ExportFinished, this, [this]() {
                        ResetButton(MainUI->UnpackButton, "Unpack");
                    });
                    if (NewDatFile->ReadDaveFile()) {
                        MainUI->TabWidget->addTab(NewDatFile, NewDatFile->GetFileName());
                        MainUI->TabWidget->setCurrentWidget(NewDatFile);
                        MainUI->TabWidget->tabBar()->setTabToolTip(MainUI->TabWidget->currentIndex(), NewDatFile->GetFilePath());
                    }
                    else
                        NewDatFile->deleteLater();
                }
                else
                    QMessageBox::warning(this, "Invalid File", QString("%1 is not a DAT file!").arg(QFileInfo(File).fileName()));
            }
        }
    }
    else
        Event->ignore();
}

void MainWindow::SetButtonLock(const bool AreButtonsLocked) const {
    MainUI->PackButton->setEnabled(AreButtonsLocked);
    MainUI->UnpackButton->setEnabled(AreButtonsLocked);
}

void MainWindow::OnUnpackButtonClicked() {
    auto *Widget = dynamic_cast<DATFile*>(MainUI->TabWidget->currentWidget());
    if (QFileInfo(Widget->GetFilePath()).isFile()) {
        MainUI->TabWidget->tabBar()->tabButton(MainUI->TabWidget->currentIndex(), QTabBar::RightSide)->hide();
        PlayButtonAnimation(MainUI->UnpackButton, "Unpacking");
        Widget->UnpackFiles(MainUI->UnpackLineEdit->text());
    }
    else {
        QMessageBox::critical(this, "No File", QString("Could not find %1.").arg(Widget->GetFileName()));
    }
}

void MainWindow::OnPackButtonClicked() {
    auto *Widget = dynamic_cast<DATFile*>(MainUI->TabWidget->currentWidget());
    if (QFileInfo(Widget->GetFilePath()).isFile() && QDir(Widget->MakeFileDirectory()).exists()) {
        PlayButtonAnimation(MainUI->PackButton, "Packing");
        MainUI->TabWidget->tabBar()->tabButton(MainUI->TabWidget->currentIndex(), QTabBar::RightSide)->hide();
        QStringList Arguments = {"B", "-ca", "-cn", "-cf", "-fc", "1", Widget->MakeFileDirectory(), MainUI->PackLineEdit->text()};
        RunDaveScript(Arguments, [&](int, QProcess::ExitStatus) {
            ResetButton(MainUI->PackButton, "Pack");
            MainUI->TabWidget->tabBar()->tabButton(MainUI->TabWidget->currentIndex(), QTabBar::RightSide)->show();
        });
    }
    else
        QMessageBox::critical(this, "Couldn't Pack File", QString("Could not find folder:\n %1").arg(Widget->MakeFileDirectory()));
}

void MainWindow::OnPackBrowseButtonClicked() {
    QString FilePath = QFileDialog::getExistingDirectory(this, "Choose Pack Destination", MainUI->UnpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!FilePath.isEmpty()) {
        auto *Widget = dynamic_cast<DATFile*>(MainUI->TabWidget->currentWidget());
        Widget->SetPackPath(FilePath + "/" + Widget->GetFileName());
        MainUI->PackLineEdit->setText(Widget->GetPackPath());
    }
}

void MainWindow::OnUnpackBrowseButtonClicked() {
    QString FolderPath = QFileDialog::getExistingDirectory(this, "Choose Unpack Destination", MainUI->UnpackLineEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!FolderPath.isEmpty()) {
        auto *Widget = dynamic_cast<DATFile*>(MainUI->TabWidget->currentWidget());
        Widget->SetUnpackDirectory(FolderPath);
        MainUI->UnpackLineEdit->setText(Widget->GetUnpackDirectory());
    }
}

void MainWindow::PlayButtonAnimation(QPushButton* InButton, const QString& InButtonText) {
    SetButtonLock(false);
    QStringList AnimationText = {"", ".", "..", "..."};
    quint32 Index = 0;
    connect(ButtonAnimationTimer, &QTimer::timeout, this, [=]() mutable {
                InButton->setText(InButtonText + AnimationText[Index]);
                Index = (Index + 1) % AnimationText.size();
            });
    ButtonAnimationTimer->setInterval(0);
    ButtonAnimationTimer->start(300);
}

void MainWindow::ResetButton(QPushButton* InButton, const QString &InButtonText) {
    SetButtonLock(true);
    InButton->setText(InButtonText);
    ButtonAnimationTimer->stop();
    ButtonAnimationTimer->disconnect();
}
