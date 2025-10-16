#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include "DATFile.h"

#include <QDragLeaveEvent>
#include <QMimeData>
#include <QProcess>
#include <QAbstractFileIconProvider>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), MainUI(new Ui::MainWindow) {
    MainUI->setupUi(this);
    MainUI->ProgressBar->hide();
    MainUI->PackSection->hide();
    MainUI->UnpackSection->hide();

    connect(MainUI->UnpackButton, &QPushButton::clicked, this, &MainWindow::OnUnpackButtonClicked);
    connect(MainUI->PackButton, &QPushButton::clicked, this, &MainWindow::OnPackButtonClicked);


    connect(MainUI->TabWidget, &QTabWidget::currentChanged, [this](int Index) {
        DATFile *Widget = static_cast<DATFile*>(MainUI->TabWidget->widget(Index));

        if (Widget) {
            Widget->GetFileType() == "Dave" ? MainUI->PackSection->show() : MainUI->PackSection->hide();
            MainUI->UnpackSection->show();

            MainUI->UnpackLineEdit->setText(Widget->MakeFileDirectory());
            MainUI->PackLineEdit->setText(Widget->GetFilePath());
        }
        else {
            MainUI->UnpackLineEdit->clear();
            MainUI->PackLineEdit->clear();
            MainUI->PackSection->hide();
            MainUI->UnpackSection->hide();
        }
    });
    connect(MainUI->TabWidget, &QTabWidget::tabCloseRequested, [this](int Index) {
        QWidget* Widget = MainUI->TabWidget->widget(Index);

        QStandardItemModel* Model = TabToModelMap.value(Widget, nullptr);
        QTreeView* TreeView = TabToTreeViewMap.value(Widget, nullptr);

        TabToModelMap.remove(Widget);
        TabToTreeViewMap.remove(Widget);

        if (TreeView) {
            TreeView->setModel(nullptr);
        }

        if (Model) {
            Model->clear();
            delete Model;
        }

        MainUI->TabWidget->removeTab(Index);
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

        for (auto Url : Event->mimeData()->urls()) {
            if (Url.isLocalFile()) {
                QString File = Url.toLocalFile();
                for (quint32 TabIndex = 0; TabIndex < MainUI->TabWidget->count(); TabIndex++) {
                    DATFile* Tab = static_cast<DATFile*>(MainUI->TabWidget->widget(TabIndex));
                    if (Tab->GetFilePath() == File) {
                        MainUI->TabWidget->setCurrentWidget(Tab);
                        return;
                    }
                }

                if (File.endsWith(".dat", Qt::CaseInsensitive)) {
                    AssetPath = Url.toLocalFile();
                    QFileInfo FileInfo(AssetPath);

                    DATFile *NewDatFile = new DATFile(AssetPath);
                    connect(NewDatFile, &DATFile::SetProgressBarMax, this, [this](quint32 NewMax) {
                        MainUI->ProgressBar->show();
                        MainUI->ProgressBar->setMaximum(NewMax);
                    });
                    connect(NewDatFile, &DATFile::UpdateProgressBar, this, [this](quint32 NewProgressAmount) {
                        MainUI->ProgressBar->setValue(NewProgressAmount);
                        if (NewProgressAmount >= MainUI->ProgressBar->maximum())
                            MainUI->ProgressBar->hide();
                    });
                    NewDatFile->ReadDaveFile();
                    MainUI->TabWidget->addTab(NewDatFile, NewDatFile->GetFileName());
                    MainUI->TabWidget->setCurrentWidget(NewDatFile);
                    MainUI->TabWidget->tabBar()->setTabToolTip(MainUI->TabWidget->currentIndex(), NewDatFile->GetFilePath());
                }
                else
                    QMessageBox::warning(this, "Invalid File", QString("%1 is not a DAT file!").arg(QFileInfo(File).fileName()));
            }
        }
    }
    else
        Event->ignore();
}

void MainWindow::SetButtonLock(bool AreButtonsLocked) {
    MainUI->PackButton->setEnabled(AreButtonsLocked);
    MainUI->UnpackButton->setEnabled(AreButtonsLocked);
}

void MainWindow::OnUnpackButtonClicked() {
    DATFile *Widget = static_cast<DATFile*>(MainUI->TabWidget->currentWidget());
    MainUI->TabWidget->tabBar()->tabButton(MainUI->TabWidget->currentIndex(), QTabBar::RightSide)->hide();
    if (QFileInfo(Widget->GetFilePath()).isFile()) {
        PlayButtonAnimation(MainUI->UnpackButton, "Unpacking");
        QStringList Arguments = {"X", Widget->GetFilePath()};
        RunDaveScript(Arguments, [&](int, QProcess::ExitStatus) {
            ResetButton(MainUI->UnpackButton, "Unpack");
        });
    }
    // else {
    //     QMessageBox::critical(this, "Invalid Directory", QString("%1 is not a valid directory, please select another one.").arg(Widget->GetFilePath()));
    // }
}

void MainWindow::OnPackButtonClicked() {
    DATFile *Widget = static_cast<DATFile*>(MainUI->TabWidget->currentWidget());
    if (QFileInfo(Widget->GetFilePath()).isFile()) {
        PlayButtonAnimation(MainUI->PackButton, "Packing");
        MainUI->TabWidget->tabBar()->tabButton(MainUI->TabWidget->currentIndex(), QTabBar::RightSide)->hide();
        QStringList Arguments = {"B", "-ca", "-cn", "-cf", "-fc", "1", Widget->MakeFileDirectory(), Widget->GetFilePath()};
        RunDaveScript(Arguments, [&](int, QProcess::ExitStatus) {
            ResetButton(MainUI->PackButton, "Pack");
            MainUI->TabWidget->tabBar()->tabButton(MainUI->TabWidget->currentIndex(), QTabBar::RightSide)->show();
        });
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
