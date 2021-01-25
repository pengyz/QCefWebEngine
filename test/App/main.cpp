#include <QApplication>

#include "mainwindow.h"
#include "include/QCefCoreManager.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    QCefCoreManager::get()->init(false, true);
    MainWindow mainWindow;
    mainWindow.show();
    int ret = app.exec();
    QCefCoreManager::get()->deInit();
    return ret;
}