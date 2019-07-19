#include <QApplication>
#include <QtDebug>
#include <QSslSocket>
#include <QIcon>

#include "mainwindow.h"
#include "network_manager.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    app.setApplicationName("dox");
    app.setOrganizationName("Word to the Wise");
    app.setOrganizationDomain("wordtothewise.com");
    app.setWindowIcon(QIcon(":/dox-icon.png"));

    initNetworkManager(&app);

    auto window = new MainWindow;
    window->show();

    qDebug() << "--- main start";
    qDebug() << "  compiled with Qt" << QT_VERSION_STR;
    qDebug() << "  running with Qt" << qVersion();
#ifdef  Q_PROCESSOR_X86_64
    qDebug() << "  Q_PROCESSOR_X86_64 defined";
#endif
#ifdef Q_OS_WINRT
    qDebug() << "  Q_OS_WINRT defined";
#endif
#ifdef Q_OS_WIN
    qDebug() << "  Q_OS_WIN defined";
#endif
#ifdef Q_OS_WIN32
    qDebug() << "  Q_OS_WIN32 defined";
#endif
#ifdef Q_OS_WIN64
    qDebug() << "  Q_OS_WIN64 defined";
#endif
    qDebug() << "  supportsSsl" << QSslSocket::supportsSsl();
    qDebug() << "  sslLibraryBuildVersion" << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "  sslLibraryVersion" << QSslSocket::sslLibraryVersionString();
    qDebug() << "--- main exec";

    return app.exec();
}
