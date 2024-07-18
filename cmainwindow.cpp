#include "cmainwindow.h"
#include "./ui_cmainwindow.h"

#include <QPixmap>
#include <QFontDatabase>
#include <thread>

void showConsole() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("Console allocated\n");
}

void CMainWindow::startMemoryUpdateThread() {
    static auto autoUpdateMemoryViewer =
        [](CMainWindow* window, CSettingsWindow* settings) -> void {
        while(true) {
            std::this_thread::sleep_for( std::chrono::milliseconds(settings->memoryViewAutoUpdateInterval()) );
            if(!settings->memoryViewIsAutoUpdateEnabled())
                continue;

            emit window->updateMemorySignal();
        }
    };

    static bool once{ };
    if(!once) {
        std::thread(autoUpdateMemoryViewer, this, m_Settings).detach();
        once = true;
    }
}

void CMainWindow::setupTextures() {
    QPixmap refreshIcon(":/resources/images/refreshIcon.png");
    QPixmap mainIcon(":/resources/images/mainIcon.png");

    if(refreshIcon.isNull() || mainIcon.isNull())
        throw std::runtime_error("Failed to load resources");

    QFontDatabase::addApplicationFont(":/resources/fonts/UbuntuMono-Regular.ttf");
    QFont font = QFont("Ubuntu Mono", 9, 1);
}

void CMainWindow::connectSignals() {
    QObject::connect(m_Settings, &CSettingsWindow::memoryViewFormatChanged, this, &CMainWindow::onMemoryAddressFormatChanged);

    QObject::connect(this, &CMainWindow::updateMemorySignal, this, &CMainWindow::updateMemoryDataEdit);

    QObject::connect(m_ProcessSelector, &CProcessSelectorWindow::processAttached, this, &CMainWindow::onProcessAttach);
    QObject::connect(m_ProcessSelector, &CProcessSelectorWindow::processDetached, this, &CMainWindow::onProcessDetach);
}

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CMainWindow)
    , m_Settings{ new CSettingsWindow(this) }
    , m_ProcessSelector{ new CProcessSelectorWindow(this, m_Settings) }
    , m_ModuleList{ new CModuleListWindow(this, m_Settings, m_ProcessSelector) } {
    ui->setupUi(this);

#ifndef NDEBUG
    showConsole();
#endif
    setupTextures();
    connectSignals();

    updateMemoryDataEdit();
    startMemoryUpdateThread();

    m_ProcessSelector->show();
}

CMainWindow::~CMainWindow() {
    delete ui;
    delete m_ModuleList;
    delete m_ProcessSelector;
    delete m_Settings;
}

void CMainWindow::onProcessAttach() {
    //
}

void CMainWindow::onProcessDetach() {
    m_MemoryStartAddress = { };
    m_MemoryOffset = { };
}

void CMainWindow::goToMemoryAddress(std::uint64_t address) {
    if(!m_ProcessSelector->selectedProcess())
        return;

    ui->memoryStartAddress->setText(QString::number(address, 16));
    m_MemoryStartAddress = address;
    m_MemoryOffset = { };
    updateMemoryDataEdit();
}

void CMainWindow::updateMemoryDataEdit() {
    ui->memoryDataEdit->clear();
    if(!m_ProcessSelector->selectedProcess() || !m_MemoryStartAddress)
        return;

    std::uint64_t currentAddress = m_MemoryStartAddress + m_MemoryOffset;

    std::uint8_t* buffer = new std::uint8_t[c_MemoryBufferSize]{ };
    CBytesProtectionMaskFormattablePlain protectionMask(c_MemoryBufferSize);
    m_ProcessSelector->selectedProcess()->readPages(currentAddress, c_MemoryBufferSize, buffer, &protectionMask);

    for(std::size_t i = 0; i < c_MemoryRows; ++i) {
        QString bytesRow{ }, charsRow{ };
        for(std::size_t j = 0; j < c_MemoryBytesInRow; ++j) {
            std::size_t currentByteIndex = i * c_MemoryBytesInRow + j;
            bytesRow += (protectionMask.format(currentByteIndex, buffer[currentByteIndex]) + std::string(" ")).c_str();
            charsRow +=
                Utilities::isValidASCIIChar(static_cast<char>(buffer[currentByteIndex])) ?
                            static_cast<char>(buffer[currentByteIndex]) :
                            '.';
        }

        char locationBuffer[32]{ };
        auto formatLocation = [&]() -> void {
            if(m_Settings->memoryViewIsOffsetRelative()) {
                std::int32_t currentOffset = m_MemoryOffset + static_cast<std::int32_t>(i * c_MemoryBytesInRow);
                bool isNegative{ currentOffset < 0 };
                if(;isNegative) {
                    sprintf_s(locationBuffer, "-%04x:", std::abs(currentOffset));
                    return;
                }
                sprintf_s(locationBuffer, "%05x:", currentOffset);
            } else { // abs format
                sprintf_s(locationBuffer, "%llx: ", currentAddress + i * c_MemoryBytesInRow);
            }
        };

        formatLocation();
        ui->memoryDataEdit->append(QString(locationBuffer) + bytesRow + charsRow);
    }

    ui->memoryDataEdit->append(QString("--------------------"));

    MBIEx mbi{ m_ProcessSelector->selectedProcess()->query(currentAddress) };
    ui->memoryDataEdit->append(QString("Page Base and Size: ") +
                                        QString::number(reinterpret_cast<std::uint64_t>(mbi.BaseAddress), 16) +
                                        QString(" - ") +
                                        QString::number(mbi.RegionSize, 16));
    ui->memoryDataEdit->append(QString("Page Protection: ") +
                                        QString(mbi.format().c_str()));
    ui->memoryDataEdit->append(QString("Allocation Base and Size: ") +
                                        QString::number(reinterpret_cast<std::uint64_t>(mbi.AllocationBase), 16) +
                                        QString(" - ") +
                                        QString::number(mbi.RegionSize, 16));
}

void CMainWindow::on_memoryVScrollBar_valueChanged(int value) {
    static int middleValue =
        ui->memoryVScrollBar->minimum() +
        (ui->memoryVScrollBar->maximum() - ui->memoryVScrollBar->minimum()) / 2;

    if(value == middleValue) {
        return;
    }

    m_MemoryOffset += (value - middleValue) * c_MemoryBytesInRow;
    ui->memoryVScrollBar->setValue(middleValue);
    updateMemoryDataEdit();
}

void CMainWindow::on_memoryStartAddress_textChanged(const QString &arg1) {
    bool isOk{ };
    std::uint64_t address = arg1.toULongLong(&isOk, 16);
    if(!isOk)
        return;

    m_MemoryStartAddress = address;
    m_MemoryOffset = { };
    updateMemoryDataEdit();
}

void CMainWindow::onMemoryAddressFormatChanged() {
    // yeah, i could just use updateMemoryData but this would be easier to understand
    updateMemoryDataEdit();
}

void CMainWindow::on_memoryResetOffsetButton_clicked() {
    if(!m_MemoryStartAddress)
        return;

    m_MemoryOffset = { };
    updateMemoryDataEdit();
}

void CMainWindow::on_actionOpen_Program_Data_Folder_triggered() {
    char cmd[MAX_PATH + 20]{ };
    sprintf_s(cmd, "explorer %s", Utilities::programDataDirectory().c_str());
    system(cmd);
}

void CMainWindow::updateStatusBar(const QString& message) {
    ui->statusbar->showMessage(message);
}

void CMainWindow::on_actionSettings_triggered() {
    m_Settings->show();
}

void CMainWindow::on_actionProcess_Selector_triggered() {
    m_ProcessSelector->show();
}

void CMainWindow::on_actionModule_List_triggered() {
    m_ModuleList->show();
}

void CMainWindow::on_actionExit_triggered() {
    close();
}

