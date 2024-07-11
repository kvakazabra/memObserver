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

    void on_modulesRefreshButton_clicked();
    void on_modulesList_currentRowChanged(int currentRow);

    void on_memoryVScrollBar_valueChanged(int value);
    void on_memoryStartAddress_textChanged(const QString &arg1);
    void on_memoryResetOffsetButton_clicked();

    void onModuleInfoFormatChanged();
    void onMemoryAddressFormatChanged();
private:
    Ui::CMainWindow *ui;
    void connectButtons();
    void setupTextures();

    std::shared_ptr<CProcessList> m_ProcessList{ std::make_shared<CProcessList>() };
    std::shared_ptr<CProcess> m_SelectedProcess{ };
    std::shared_ptr<CModuleList> m_ModulesList{ };
    int m_SelectedModule{ -1 };
    void selectModule(int idx = -1);

    void updateProcessesCombo();
    void updateProcessLastLabel(const QString& message);
    void updateCurrentProcessLabel(const CProcessMemento& process = CProcessMemento(0, "none"));
    void updateModuleInfoLines();

    void onProcessAttach();
    void onProcessDetach();

    std::uint64_t m_MemoryStartAddress{ };
    std::int32_t m_MemoryOffset{ };

    static constexpr std::size_t c_MemoryBytesInRow{ 8 }; // must be divisible by 4
    static constexpr std::size_t c_MemoryRows{ 10 };
    static constexpr std::size_t c_MemoryBufferSize{ c_MemoryBytesInRow * c_MemoryRows };

    void updateMemoryDataEdit();
};

/*
 * process list -> refresh, combobox, (maybe) button to attach
 * modules list -> refresh, list widget, on right click on item context menu (copy address, copy dll name), on select maybe dump sections info
 * address dumper -> line edit with address, auto refresh, text edit with vertical scroll
*/
