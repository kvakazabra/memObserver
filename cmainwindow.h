#pragma once
#include <QMainWindow>
#include <QListWidgetItem>
#include "process.h"
#include "settings.h"
#include "process_selector.h"
#include "module_list.h"

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

    void updateStatusBar(const QString& message = "");
    void goToMemoryAddress(std::uint64_t address);
private slots:

    void on_memoryVScrollBar_valueChanged(int value);
    void on_memoryStartAddress_textChanged(const QString &arg1);
    void on_memoryResetOffsetButton_clicked();

    void onMemoryAddressFormatChanged();

    void on_actionOpen_Program_Data_Folder_triggered();
    void on_actionSettings_triggered();
    void on_actionProcess_Selector_triggered();
    void on_actionModule_List_triggered();
private:
    Ui::CMainWindow *ui;
    CSettingsWindow* m_Settings;
    CProcessSelectorWindow* m_ProcessSelector;
    CModuleListWindow* m_ModuleList;

    void connectSignals();
    void setupTextures();
    void startMemoryUpdateThread();

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
signals:
    void updateMemorySignal();
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
