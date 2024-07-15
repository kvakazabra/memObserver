#pragma once

#include <QDialog>
#include "process.h"

namespace Ui {
class CProcessSelector;
}

class CProcessSelector : public QDialog
{
    Q_OBJECT

public:
    explicit CProcessSelector(QWidget *parent = nullptr);
    ~CProcessSelector();

    std::weak_ptr<IProcessIO> selectedProcess() const;
    const std::vector<CProcessMemento>& processes() const;
signals:
    void processAttached();
    void processDetached();
    void refreshProcesses();
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

    void onProcessAttach();
    void onProcessDetach();
    void updateMainWindowStatusBar(const QString& message = "");

    std::shared_ptr<CProcessList> m_ProcessList{ std::make_shared<CProcessList>() }; // todo: to unique ptr i guess
    std::shared_ptr<IProcessIO> m_SelectedProcess{ };
};
