#pragma once
#include <QDialog>
#include "process.h"
#include "settings.h"

namespace Ui {
class CProcessSelector;
}

class CProcessSelectorWindow : public QDialog
{
    Q_OBJECT

public:
    CProcessSelectorWindow(QWidget *parent, CSettingsWindow* settings);
    ~CProcessSelectorWindow();

    std::shared_ptr<IProcessIO> selectedProcess() const;
    const std::vector<CProcessMemento>& processes() const;
private slots:
    void on_processRefreshButton_clicked();
    void on_processComboBox_activated(int index);

    void on_closeButton_clicked();

    void invalidProcessSlot();
signals:
    void processAttached();
    void processDetached();
private:
    void connectSignals();

    void onProcessAttach();
    void onProcessDetach();

    void updateProcessesCombo();
    void updateProcessLastLabel(const QString& message);
    void updateCurrentProcessLabel(const CProcessMemento& process = CProcessMemento(0, "none"));

    void updateMainWindowStatusBar(const QString& message = "");
private:
    Ui::CProcessSelector *ui;
    CSettingsWindow* m_Settings;

    std::unique_ptr<CProcessList> m_ProcessList{ std::make_unique<CProcessList>() };
    std::shared_ptr<IProcessIO> m_SelectedProcess{ };
};
