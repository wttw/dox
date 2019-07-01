#include <QApplication>

#include "mainwindow.h"
#include "network_manager.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    app.setApplicationName("dox");
    app.setOrganizationName("Word to the Wise");
    app.setOrganizationDomain("wordtothewise.com");

    initNetworkManager(&app);

    auto window = new MainWindow;
    window->show();

    return app.exec();
}
