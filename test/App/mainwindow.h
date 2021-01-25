#pragma once;
#include <QDialog>
#include <QCloseEvent>
#include "include/QCefView.h"

class MainWindow : public QDialog {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

protected:
    void setupUi(QWidget* parent);
    void closeEvent(QCloseEvent* evt) override;

private:
    QCefView* m_webView = nullptr;
    bool m_allClosed = false;
};