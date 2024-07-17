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

    void on_actionOpen_Program_Data_Folder_triggered();
    void on_actionSettings_triggered();
    void on_actionProcess_Selector_triggered();
    void on_actionModule_List_triggered();
    void on_actionExit_triggered();

    void updateMemoryDataEdit();
    void onMemoryAddressFormatChanged();
signals:
    void updateMemorySignal();
private:
    void connectSignals();
    void setupTextures();
    void startMemoryUpdateThread();

    void onProcessAttach();
    void onProcessDetach();
private:
    static constexpr std::size_t c_MemoryBytesInRow{ 8 }; // must be divisible by 4
    static constexpr std::size_t c_MemoryRows{ 22 };
    static constexpr std::size_t c_MemoryBufferSize{ c_MemoryBytesInRow * c_MemoryRows };

    std::uint64_t m_MemoryStartAddress{ };
    std::int32_t m_MemoryOffset{ };

    Ui::CMainWindow *ui;
    CSettingsWindow* m_Settings;
    CProcessSelectorWindow* m_ProcessSelector;
    CModuleListWindow* m_ModuleList;
};


/*
 *
 * TODO:
 * add buttons to change page protection
 * add different types in memory
 *  maybe break down all UI and functional classes into two, like CSettings where settings would be accessed, and CSettingsWindow which can be shown and derived from CSettings
*/
