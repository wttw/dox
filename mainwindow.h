#ifndef DOX_MAINWINDOW_H
#define DOX_MAINWINDOW_H

#include <QMainWindow>

class Query;
class Ping;
class Survey;

class QTabWidget;
class Resolvers;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
public slots:
    void addResolver();
    void deleteResolver(int idx);
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    Resolvers *resolvers;
    QTabWidget *tabs;
    Query *queryTab;
    Ping *pingTab;
    Survey *surveyTab;
};

#endif
