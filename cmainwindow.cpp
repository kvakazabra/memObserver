#include "cmainwindow.h"
#include "./ui_cmainwindow.h"
#include "process_win32.h"
#include <QPixmap>

void showConsole() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("Console allocated\n");
}

void CMainWindow::setupTextures() {
    QPixmap refreshIcon(":/resources/images/refreshIcon.png");
    if(refreshIcon.isNull())
        throw std::runtime_error("Failed to load resources");
}

void CMainWindow::connectButtons() {
    QObject::connect(ui->moduleInfoDecimalButton, SIGNAL(clicked()), this, SLOT(onModuleInfoFormatChanged()));
    QObject::connect(ui->moduleInfoHexadecimalButton, SIGNAL(clicked()), this, SLOT(onModuleInfoFormatChanged()));

    QObject::connect(ui->memoryOffsetAbsoluteButton, SIGNAL(clicked()), this, SLOT(onMemoryAddressFormatChanged()));
    QObject::connect(ui->memoryOffsetRelativeButton, SIGNAL(clicked()), this, SLOT(onMemoryAddressFormatChanged()));
}

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CMainWindow) {
    ui->setupUi(this);

    showConsole();
    setupTextures();
    connectButtons();

    updateProcessesCombo();
    updateCurrentProcessLabel();
    updateMemoryDataEdit();
}

CMainWindow::~CMainWindow() {
    delete ui;
    m_SelectedProcess.reset();
    m_ProcessList.reset();
}

void CMainWindow::on_processRefreshButton_clicked() {
    m_ProcessList->refresh();
    updateProcessesCombo();
}

void CMainWindow::updateProcessesCombo() {
    const auto processes = m_ProcessList->data();
    ui->processComboBox->clear();

    for(const auto& process : processes) {
        ui->processComboBox->addItem(QString(process.format().c_str()));
    }

    updateProcessLastLabel(QString("Total Processes: ") + QString::number(processes.size()));
}

void CMainWindow::updateProcessLastLabel(const QString& message) {
    ui->processLastMessage->setText(message);
}

void CMainWindow::updateCurrentProcessLabel(const CProcessMemento& process) {
    ui->processCurrentLabel->setText(QString("Current Process: ") +
                                     QString(process.name().c_str()) + QString(" [") + QString::number(process.id()) + QString("]"));
}

void CMainWindow::on_processComboBox_activated(int index) {
    if(index >= m_ProcessList->data().size())
        throw std::runtime_error("Out of bounds: m_ProcessList");

    onProcessDetach();
    auto selectedProcess = std::make_shared<CProcessWinIO>(m_ProcessList->data()[index]);
    if(!selectedProcess->isAttached()) {
        updateProcessLastLabel(QString("Failed to attach"));
        return;
    }

    m_SelectedProcess = std::move(selectedProcess);
    onProcessAttach();
}

void CMainWindow::onProcessAttach() {
    updateProcessLastLabel(QString("Attached to ") + QString(m_SelectedProcess->memento().name().c_str()) + QString(" successfully"));
    updateCurrentProcessLabel(m_SelectedProcess->memento());

    m_ModulesList = std::make_shared<CModuleList>(m_SelectedProcess);
    for(auto& module : m_ModulesList->data()) {
        ui->modulesList->addItem(QString(module.format().c_str()));
    }
}

void CMainWindow::onProcessDetach() {
    m_MemoryStartAddress = { };
    m_MemoryOffset = { };

    ui->modulesList->clear();
    m_ModulesList.reset();

    selectModule(-1);

    m_SelectedProcess.reset();
    updateCurrentProcessLabel();
}

void CMainWindow::selectModule(int idx) {
    m_SelectedModule = idx;
    updateModuleInfoLines();
}

void CMainWindow::on_modulesRefreshButton_clicked() {
    if(!m_ModulesList)
        return;

    selectModule(-1);
    m_ModulesList->refresh();
}

void CMainWindow::updateModuleInfoLines() {
    if(m_SelectedModule == -1) {
        ui->moduleInfoNameLine->setText("");
        ui->moduleInfoBaseAddressLine->setText("");
        ui->moduleInfoSizeLine->setText("");
        return;
    }

    const auto& selectedModule = m_ModulesList->data()[m_SelectedModule];
    const auto [baseAddress, size] = selectedModule.info();

    const int base = ui->moduleInfoDecimalButton->isChecked() ? 10 : 16;

    ui->moduleInfoNameLine->setText(QString(selectedModule.name().c_str()));
    ui->moduleInfoBaseAddressLine->setText(QString::number(baseAddress, base));
    ui->moduleInfoSizeLine->setText(QString::number(size, base));
}

void CMainWindow::on_modulesList_currentRowChanged(int currentRow) {
    if(currentRow >= m_ModulesList->data().size())
        throw std::runtime_error("Out of bounds: m_ModulesList");

    selectModule(currentRow);
}

void CMainWindow::onModuleInfoFormatChanged() {
    updateModuleInfoLines();
}

void CMainWindow::updateMemoryDataEdit() {
    ui->memoryDataEdit->clear();
    ui->memoryPageInfoLabel->clear();
    if(!m_SelectedProcess)
        return;

    std::uint8_t* buffer = new std::uint8_t[c_MemoryBufferSize]{ }, *invalidMask = new std::uint8_t[c_MemoryBufferSize]{ };
    std::uint64_t currentAddress = m_MemoryStartAddress + m_MemoryOffset;

    m_SelectedProcess->readPages(currentAddress, c_MemoryBufferSize, buffer, invalidMask);

    for(std::size_t i = 0; i < c_MemoryRows; ++i) {
        QString bytesRow{ };
        for(std::size_t j = 0; j < c_MemoryBytesInRow; ++j) {
            static char invalidByte[4]{"?? "}, guardedByte[4]{"xx "};
            switch(invalidMask[i * c_MemoryBytesInRow + j]) {
            case 1: bytesRow += invalidByte; continue;
            case 2: bytesRow += guardedByte; continue;
            default: break;
            }

            char b[4]{ };
            sprintf_s(b, "%02x ", buffer[i * c_MemoryBytesInRow + j]);
            bytesRow += b;
        }

        char locationBuffer[32]{ };
        auto formatLocation = [&]() -> void {
            bool isRelative = ui->memoryOffsetRelativeButton->isChecked();
            if(isRelative) {
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
        ui->memoryDataEdit->appendPlainText(QString(locationBuffer) + bytesRow);
    }

    ui->memoryDataEdit->appendPlainText(QString("--------------------"));

    MBIEx mbi{ m_SelectedProcess->query(currentAddress) };
    ui->memoryDataEdit->appendPlainText(QString("Page Base and Size: ") +
                                        QString::number(reinterpret_cast<std::uint64_t>(mbi.BaseAddress), 16) +
                                        QString(" - ") +
                                        QString::number(mbi.RegionSize, 16));
    ui->memoryDataEdit->appendPlainText(QString("Page Protection: ") +
                                        QString(mbi.format().c_str()));
    // ui->memoryDataEdit->appendPlainText(QString("Allocation Base and Size: ") +
    //                                     QString::number(reinterpret_cast<std::uint64_t>(mbi.AllocationBase), 16) +
    //                                     QString(" - ") +
    //                                     QString::number(mbi.RegionSize, 16));
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
    m_MemoryOffset = { };
    updateMemoryDataEdit();
}

