#pragma once

#include <QDialog>
#include "process.h"
#include "settings.h"

namespace Ui {
class CProcessSelector;
}

class CProcessSelector : public QDialog
{
    Q_OBJECT

public:
    CProcessSelector(QWidget *parent, CSettings* settings);
    ~CProcessSelector();

    std::weak_ptr<IProcessIO> selectedProcess() const;
    const std::vector<CProcessMemento>& processes() const;
signals:
    void processAttached();
    void processDetached();
private slots:
    void on_processRefreshButton_clicked();
    void on_processComboBox_activated(int index);
    void invalidProcessSlot();

    void on_closeButton_clicked();
private:
    void updateProcessesCombo();
    void updateProcessLastLabel(const QString& message);
    void updateCurrentProcessLabel(const CProcessMemento& process = CProcessMemento(0, "none"));
private:
    Ui::CProcessSelector *ui;
    CSettings* m_Settings;

    void onProcessAttach();
    void onProcessDetach();
    void updateMainWindowStatusBar(const QString& message = "");

    std::unique_ptr<CProcessList> m_ProcessList{ std::make_unique<CProcessList>() };
    std::shared_ptr<IProcessIO> m_SelectedProcess{ };
};
