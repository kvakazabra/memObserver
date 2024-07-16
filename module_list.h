#pragma once
#include <QDialog>
#include <QListWidgetItem>
#include "settings.h"
#include "module.h"
#include "process_selector.h"

namespace Ui {
class CModuleListWindow;
}

class CModuleListWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CModuleListWindow(QWidget *parent, CSettingsWindow* settings, CProcessSelectorWindow* processSelector);
    ~CModuleListWindow();
private slots:
    void on_modulesRefreshButton_clicked();
    void on_moduleList_currentRowChanged(int currentRow);
    void on_moduleList_itemDoubleClicked(QListWidgetItem *item);

    void on_sectionList_currentRowChanged(int currentRow);
    void on_sectionListGoToButton_clicked();
    void on_sectionList_itemDoubleClicked(QListWidgetItem *item);

    void on_dumpSectionButton_clicked();
    void on_dumpModuleButton_clicked();

    void on_closeButton_clicked();

    void onProcessAttach();
    void onProcessDetach();
    void onModuleInfoFormatChanged();
private:
    void connectSignals();

    void selectModule(int idx = -1);
    const CModule& getSelectedModule();
    void goToSelectedModule();
    void updateModuleInfoLines();
    void updateModuleDumpLastLabel(const QString& message = "");

    void selectSection(int idx = -1);
    const CSection& getSelectedSection();
    void goToSelectedSection();
    void updateSectionInfoLines();
    void updateSectionDumpLastLabel(const QString& message = "");

    void updateMainWindowStatusBar(const QString& message = "");
    void goToMemoryAddress(std::uint64_t address);
private:
    int m_SelectedModule{ -1 }, m_SelectedSection{ -1 };

    Ui::CModuleListWindow *ui;
    CSettingsWindow* m_Settings;
    CProcessSelectorWindow* m_ProcessSelector;
};
