#pragma once

#include <QMainWindow>
#include "process.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CMainWindow;
}
QT_END_NAMESPACE

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow();

private slots:
    void on_processRefreshButton_clicked();

private:
    Ui::CMainWindow *ui;

    void setupTextures();

    std::unique_ptr<CProcessList> m_ProcessList{ std::make_unique<CProcessList>() };
    void updateProcessesCombo();
};
