#include "mainwindow.h"

#include "query.h"
#include "ping.h"
#include "resolvers.h"
#include "survey.h"

#include <QTabWidget>
#include <QSettings>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    resolvers = new Resolvers(this);
    tabs = new QTabWidget;
    setCentralWidget(tabs);
    queryTab = new Query(resolvers);
    tabs->addTab(queryTab, tr("Query"));
    pingTab = new Ping(resolvers);
    tabs->addTab(pingTab, tr("Performance"));
    surveyTab = new Survey;
    tabs->addTab(surveyTab, tr("Survey"));

    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("Load Resolvers..."), [=](){
        QString filename = QFileDialog::getOpenFileName(this, tr("Load Resolvers"), QString(), tr("JSON Files (*.json)"));
        if (!filename.isEmpty()) {
            QString err = resolvers->loadFile(filename);
            if (!err.isEmpty()) {
                QMessageBox::warning(this, tr("Failed to load resolvers"), err);
                return;
            }
        }
    });
    fileMenu->addAction(tr("Save Resolvers..."), [=](){
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Resolvers"), QString("resolvers.json"));
        if (!filename.isEmpty()) {
            QString err = resolvers->saveFile(filename);
            if (!err.isEmpty()) {
                QMessageBox::warning(this, tr("Failed to load resolvers"), err);
            }
        }
    });

    fileMenu->addAction(tr("Load Survey..."), surveyTab, &Survey::load);
    fileMenu->addAction(tr("Save Resolvers..."), surveyTab, &Survey::save);


    QAction *exitAct = fileMenu->addAction(tr("E&xit"), qApp, &QApplication::closeAllWindows);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);

    QSettings s;

    s.beginGroup("MainWindow");
    resize(s.value("size", QSize(400, 400)).toSize());
    move(s.value("pos", QPoint(200, 200)).toPoint());
    tabs->setCurrentIndex(s.value("currentTab", 0).toInt());
    s.endGroup();
}

void MainWindow::closeEvent(QCloseEvent */*event*/) {
    QSettings s;
    s.beginGroup("MainWindow");
    s.setValue("size", size());
    s.setValue("pos", pos());
    s.setValue("currentTab", tabs->currentIndex());
    s.endGroup();
}

void MainWindow::addResolver() {

}

void MainWindow::deleteResolver(int idx) {

}
