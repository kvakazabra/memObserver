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
    void on_processComboBox_activated(int index);

private:
    Ui::CMainWindow *ui;

    void setupTextures();

    std::shared_ptr<CProcessList> m_ProcessList{ std::make_shared<CProcessList>() };
    std::shared_ptr<CProcess> m_SelectedProcess{ };
    std::shared_ptr<CModuleList> m_ModulesList{ };
    void updateProcessesCombo();
    void updateProcessLastMessage(const QString& message);
    void updateCurrentProcessLabel(const CProcessMemento& process = CProcessMemento(0, "none"));
};

/*
 * process list -> refresh, combobox, (maybe) button to attach
 * modules list -> refresh, list widget, on right click on item context menu (copy address, copy dll name), on select maybe dump sections info
 * address dumper -> line edit with address, auto refresh, text edit with vertical scroll
*/
