#include <QApplication>

#include "mainwindow.h"
#include "include/QCefWebEngine.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    QCefWebEngine::get()->init(false, true);
    MainWindow mainWindow;
    mainWindow.show();
    int ret = app.exec();
    QCefWebEngine::get()->deInit();
    return ret;
}