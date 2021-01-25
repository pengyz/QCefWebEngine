#include "mainwindow.h"
#include "include/QCefCoreManager.h"
#include <QHBoxLayout>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget* parent/* = nullptr*/)
    :QDialog(parent)
{
    setupUi(this);
    this->setMinimumSize(800, 600);
    connect(QCefCoreManager::get(), &QCefCoreManager::sig_allBrowserClosed, this, [this]() {
        m_allClosed = true;
        close();
    });
}

void MainWindow::setupUi(QWidget* parent)
{
    m_webView = new QCefView("https://www.baidu.com", "MAINVIEW", this);
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_webView);
}

void MainWindow::closeEvent(QCloseEvent* evt)
{
    if (!m_allClosed) {
        QCefCoreManager::get()->closeAllBrowsers(true);
        evt->accept();
    } else {
        qApp->quit();
    }
}
