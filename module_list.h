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

    void onModuleInfoFormatChanged();
    void on_closeButton_clicked();
private:
    int m_SelectedModule{ -1 }, m_SelectedSection{ -1 };
    void selectModule(int idx = -1);
    void selectSection(int idx = -1);
    const CModule& getSelectedModule();
    const CSection& getSelectedSection();
    void goToSelectedModule();
    void goToSelectedSection();

    void updateModuleInfoLines();
    void updateSectionInfoLines();
    void updateSectionDumpLastLabel(const QString& message = "");
    void updateModuleDumpLastLabel(const QString& message = "");

    void updateMainWindowStatusBar(const QString& message = "");
private:
    Ui::CModuleListWindow *ui;
    CSettingsWindow* m_Settings;
    CProcessSelectorWindow* m_ProcessSelector;

    void connectSignals();

    void onProcessAttach();
    void onProcessDetach();

    void goToMemoryAddress(std::uint64_t address);
};
