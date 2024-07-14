#pragma once
#include <QMainWindow>
#include <QListWidgetItem>
#include "process.h"
#include "module.h"
#include "dumper.h"
#include "settings.h"

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
    void invalidProcessSlot();

    void on_modulesRefreshButton_clicked();
    void on_modulesList_currentRowChanged(int currentRow);
    void on_modulesList_itemDoubleClicked(QListWidgetItem *item);
    void on_sectionsList_currentRowChanged(int currentRow);
    void on_sectionsListGoToButton_clicked();
    void on_sectionsList_itemDoubleClicked(QListWidgetItem *item);
    void on_dumpSectionButton_clicked();
    void on_dumpModuleButton_clicked();

    void on_memoryVScrollBar_valueChanged(int value);
    void on_memoryStartAddress_textChanged(const QString &arg1);
    void on_memoryResetOffsetButton_clicked();

    void onModuleInfoFormatChanged();
    void onMemoryAddressFormatChanged();
    void on_actionOpen_Program_Data_Folder_triggered();
private:
    Ui::CMainWindow *ui;
    CSettings* m_Settings{ new CSettings(this) };

    void connectButtons();
    void setupTextures();
    void startMemoryUpdateThread();

    std::shared_ptr<CProcessList> m_ProcessList{ std::make_shared<CProcessList>() };
    std::shared_ptr<IProcessIO> m_SelectedProcess{ };
    int m_SelectedModule{ -1 }, m_SelectedSection{ -1 };
    void selectModule(int idx = -1);
    void selectSection(int idx = -1);
    const CModule& getSelectedModule();
    const CSection& getSelectedSection();
    void goToSelectedModule();
    void goToSelectedSection();
    void goToAddress(std::uint64_t address);

    void updateStatusBar(const QString& message = "");

    void updateProcessesCombo();
    void updateProcessLastLabel(const QString& message);
    void updateCurrentProcessLabel(const CProcessMemento& process = CProcessMemento(0, "none"));
    void updateModuleInfoLines();
    void updateSectionInfoLines();
    void updateSectionDumpLastLabel(const QString& message = "");
    void updateModuleDumpLastLabel(const QString& message = "");

    void onProcessAttach();
    void onProcessDetach();

    std::uint64_t m_MemoryStartAddress{ };
    std::int32_t m_MemoryOffset{ };
    bool m_MemoryAutoUpdateEnabled{ true };
    int m_MemoryAutoUpdateInterval{ 500 };

    static constexpr std::size_t c_MemoryBytesInRow{ 8 }; // must be divisible by 4
    static constexpr std::size_t c_MemoryRows{ 22 };
    static constexpr std::size_t c_MemoryBufferSize{ c_MemoryBytesInRow * c_MemoryRows };
private slots:
    void updateMemoryDataEdit();
    void on_actionSettings_triggered();
signals:
    void updateSignal();
};


/*
 *
 * TODO:
 * add buttons to change page protection
 * add different types in memory
 * [/] move process selector and module list to separate windows (will add when i finish working on everything else)
 * sort of processes and modules by names
 *
 * maybe:
 * async loading of all modules (idk why i thought about that, even on a potato pc itd still be fast enough), p.s. but practicing std::async would be nice
*/
