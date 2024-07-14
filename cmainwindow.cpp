#include "cmainwindow.h"
#include "./ui_cmainwindow.h"
#include "process_win32.h"

#include <QPixmap>
#include <thread>

void showConsole() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("Console allocated\n");
}

void CMainWindow::startMemoryUpdateThread() {
    static auto autoUpdateMemoryViewer =
        [](CMainWindow* window, CSettings* settings) -> void {
        while(true) {
            std::this_thread::sleep_for( std::chrono::milliseconds(settings->memoryViewAutoUpdateInterval()) );
            if(!settings->memoryViewIsAutoUpdateEnabled())
                continue;

            emit window->updateSignal();
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
    if(refreshIcon.isNull())
        throw std::runtime_error("Failed to load resources");
}

void CMainWindow::connectButtons() {
    QObject::connect(m_Settings, &CSettings::moduleInfoFormatChanged, this, &CMainWindow::onModuleInfoFormatChanged);
    QObject::connect(m_Settings, &CSettings::memoryViewFormatChanged, this, &CMainWindow::onMemoryAddressFormatChanged);

    QObject::connect(this, &CMainWindow::updateSignal, this, &CMainWindow::updateMemoryDataEdit);
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
    startMemoryUpdateThread();
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

void CMainWindow::invalidProcessSlot() {
    if(!m_SelectedProcess)
        return;

    CProcessWinIO* winProcess = dynamic_cast<CProcessWinIO*>(m_SelectedProcess.get());
    if(winProcess) {
        updateProcessLastLabel(
            QString("The process exited with code ") +
            QString::number(winProcess->exitCode(), 16)
        );
        updateStatusBar(QString("Detached"));
        return;
    }

    updateProcessLastLabel(QString("The process crashed"));
    updateStatusBar(QString("Crashed"));
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
        throw std::out_of_range("Out of bounds: m_ProcessList");

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
    updateStatusBar(QString("Attached to ") + QString(m_SelectedProcess->memento().name().c_str()) + QString(" successfully"));
    updateProcessLastLabel(QString("Attached successfully"));
    updateCurrentProcessLabel(m_SelectedProcess->memento());

    if(m_SelectedProcess->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    for(auto& module : m_SelectedProcess->moduleList().lock()->data()) {
        ui->modulesList->addItem(QString(module.memento().format().c_str()));
    }

    QObject::connect(m_SelectedProcess.get(), &IProcessIO::invalidProcessSignal, this, &CMainWindow::invalidProcessSlot);
}

void CMainWindow::onProcessDetach() {
    updateStatusBar();

    m_MemoryStartAddress = { };
    m_MemoryOffset = { };

    ui->sectionsList->clear();
    selectSection(-1);

    updateSectionDumpLastLabel();
    updateModuleDumpLastLabel();

    ui->modulesList->clear();
    selectModule(-1);

    m_SelectedProcess.reset();
    updateCurrentProcessLabel();
}

void CMainWindow::selectModule(int idx) {
    m_SelectedModule = idx;
    updateModuleInfoLines();
}

void CMainWindow::selectSection(int idx) {
    m_SelectedSection = idx;
    updateSectionInfoLines();
}

const CModule& CMainWindow::getSelectedModule() {
    if(m_SelectedModule == -1)
        throw std::out_of_range("Check m_SelectedModule for -1 before calling getSelectedModule");

    if(m_SelectedProcess->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    return m_SelectedProcess->moduleList().lock()->data()[m_SelectedModule];
}

const CSection& CMainWindow::getSelectedSection() {
    if(m_SelectedSection == -1)
        throw std::out_of_range("Check m_SelectedSection for -1 before calling getSelectedSection");

    const auto& selectedModule = getSelectedModule();
    if(m_SelectedSection >= selectedModule.sections().size())
        throw std::out_of_range("Error: m_SelectedSection >= sections.size()");

    return selectedModule.sections()[m_SelectedSection];
}

void CMainWindow::goToAddress(std::uint64_t address) {
    if(!m_SelectedProcess)
        return;

    ui->memoryStartAddress->setText(QString::number(address, 16));
    m_MemoryStartAddress = address;
    m_MemoryOffset = { };
    updateMemoryDataEdit();
}

void CMainWindow::goToSelectedModule() {
    if(m_SelectedModule == -1)
        return;

    const auto baseAddress = std::get<0>(getSelectedModule().memento().info());
    goToAddress(baseAddress);
}

void CMainWindow::goToSelectedSection() {
    if(m_SelectedSection == -1)
        return;

    const auto baseAddress = std::get<0>(getSelectedSection().info());
    goToAddress(baseAddress);
}

void CMainWindow::on_modulesRefreshButton_clicked() {
    if(!m_SelectedProcess)
        return;

    if(m_SelectedProcess->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    selectModule(-1);
    m_SelectedProcess->moduleList().lock()->refresh();
}

void CMainWindow::updateSectionInfoLines() {
    if(m_SelectedSection == -1) {
        ui->sectionInfoBaseAddressLine->setText("");
        ui->sectionInfoSizeLine->setText("");
        return;
    }

    const auto [baseAddress, size] = getSelectedSection().info();
    ui->sectionInfoBaseAddressLine->setText(QString::number(baseAddress, 16));
    ui->sectionInfoSizeLine->setText(QString::number(size, 16));
}

void CMainWindow::updateSectionDumpLastLabel(const QString& message) {
    ui->dumpSectionLastMessageLabel->setText(message);
}

void CMainWindow::updateModuleDumpLastLabel(const QString& message) {
    ui->dumpModuleLastMessageLabel->setText(message);
}

void CMainWindow::updateModuleInfoLines() {
    ui->sectionsList->clear();

    if(m_SelectedModule == -1) {
        ui->moduleInfoNameLine->setText("");
        ui->moduleInfoBaseAddressLine->setText("");
        ui->moduleInfoSizeLine->setText("");
        return;
    }

    if(m_SelectedProcess->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    const auto modulesData = m_SelectedProcess->moduleList().lock()->data();
    if(m_SelectedModule >= modulesData.size())
        throw std::out_of_range("Error: m_SelectedModule >= modulesData.size()");

    const auto& selectedModule = modulesData[m_SelectedModule];
    const auto [baseAddress, size] = selectedModule.memento().info();

    const int base = m_Settings->moduleInfoIsHexadecimalFormat() ? 16 : 10;

    ui->moduleInfoNameLine->setText(QString(selectedModule.memento().name().c_str()));
    ui->moduleInfoBaseAddressLine->setText(QString::number(baseAddress, base));
    ui->moduleInfoSizeLine->setText(QString::number(size, base));

    for(auto& section : selectedModule.sections()) {
        ui->sectionsList->addItem(QString(section.tag()));
    }
}

void CMainWindow::on_modulesList_currentRowChanged(int currentRow) {
    selectModule(currentRow);
}

void CMainWindow::on_sectionsList_currentRowChanged(int currentRow) {
    selectSection(currentRow);
}

void CMainWindow::onModuleInfoFormatChanged() {
    updateModuleInfoLines();
}

void CMainWindow::updateMemoryDataEdit() {
    ui->memoryDataEdit->clear();
    if(!m_SelectedProcess || !m_MemoryStartAddress)
        return;

    std::uint64_t currentAddress = m_MemoryStartAddress + m_MemoryOffset;

    std::uint8_t* buffer = new std::uint8_t[c_MemoryBufferSize]{ };
    CBytesProtectionMaskFormattablePlain protectionMask(c_MemoryBufferSize);
    m_SelectedProcess->readPages(currentAddress, c_MemoryBufferSize, buffer, &protectionMask);

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

    MBIEx mbi{ m_SelectedProcess->query(currentAddress) };
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

void CMainWindow::on_sectionsListGoToButton_clicked() {
    goToSelectedSection();
}

void CMainWindow::on_sectionsList_itemDoubleClicked(QListWidgetItem *item) {
    selectSection(item->listWidget()->row(item));
    goToSelectedSection();
}

void CMainWindow::on_dumpSectionButton_clicked() {
    if(!m_SelectedProcess || m_SelectedSection == -1) {
        updateSectionDumpLastLabel("You must select a process and a section you want to dump");
        return;
    }

    const auto& selectedSection = getSelectedSection();
    auto [baseAddress, size] = selectedSection.info();
    if(!baseAddress || !size) {
        updateSectionDumpLastLabel("Base address or size is null, can not perform an operation");
        return;
    }

    const auto dumpBuffer = CSectionDumper(m_SelectedProcess, baseAddress, size).dump();
    const auto dumpPath =
        Utilities::generatePathForDump(
            m_SelectedProcess->memento().name(),
            m_SelectedProcess->moduleList().lock()->data()[m_SelectedModule].memento().name(),
            selectedSection.tag()
        );

    if(std::filesystem::exists(dumpPath)) {
        updateSectionDumpLastLabel("File with the name of the dump already exist, delete it manually to proceed");
        return;
    }

    std::ofstream outFile(dumpPath, std::ios::trunc | std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(dumpBuffer.data()), dumpBuffer.size());
    updateSectionDumpLastLabel("Dumped successfully");
    updateStatusBar("Saved to " + QString(dumpPath.c_str()));
}

void CMainWindow::on_actionOpen_Program_Data_Folder_triggered() {
    char cmd[MAX_PATH + 20]{ };
    sprintf_s(cmd, "explorer %s", Utilities::programDataDirectory().c_str());
    system(cmd);
}

void CMainWindow::on_dumpModuleButton_clicked() {
    if(!m_SelectedProcess || m_SelectedModule == -1) {
        updateModuleDumpLastLabel("You must select a process and a module you want to dump");
        return;
    }

    const auto& module = getSelectedModule();
    auto [baseAddress, size] = module.memento().info();
    if(!baseAddress || !size) {
        updateModuleDumpLastLabel("Base address or size is null, can not perform an operation");
        return;
    }

    const auto dumpBuffer = CModuleDumper(m_SelectedProcess, baseAddress).dump();
    const auto dumpPath =
        Utilities::generatePathForDump(
            m_SelectedProcess->memento().name(),
            module.memento().name(),
            ""
        );

    if(std::filesystem::exists(dumpPath)) {
        updateModuleDumpLastLabel("File with the name of the dump already exist, delete it manually to proceed");
        return;
    }

    std::ofstream outFile(dumpPath, std::ios::trunc | std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(dumpBuffer.data()), dumpBuffer.size());
    updateModuleDumpLastLabel("Dumped successfully");
    updateStatusBar("Saved to " + QString(dumpPath.c_str()));
}

void CMainWindow::updateStatusBar(const QString& message) {
    ui->statusbar->showMessage(message);
}

void CMainWindow::on_modulesList_itemDoubleClicked(QListWidgetItem *item) {
    selectModule(item->listWidget()->row(item));
    goToSelectedModule();
}

void CMainWindow::on_actionSettings_triggered() {
    m_Settings->show();
}

