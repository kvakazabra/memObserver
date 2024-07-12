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

    if(m_SelectedProcess->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    for(auto& module : m_SelectedProcess->moduleList().lock()->data()) {
        ui->modulesList->addItem(QString(module.memento().format().c_str()));
    }
}

void CMainWindow::onProcessDetach() {
    m_MemoryStartAddress = { };
    m_MemoryOffset = { };

    ui->sectionsList->clear();
    selectSection(-1);

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

const CSection& CMainWindow::getSelectedSection() {
    if(m_SelectedSection == -1)
        throw std::runtime_error("Check m_SelectedSection for -1 before calling getSelectedSection");

    if(m_SelectedProcess->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    const auto& selectedModule = m_SelectedProcess->moduleList().lock()->data()[m_SelectedModule];
    if(m_SelectedSection >= selectedModule.sections().size())
        throw std::runtime_error("Error: m_SelectedSection >= sections.size()");

    return selectedModule.sections()[m_SelectedSection];
}

void CMainWindow::goToSelectedSection() {
    if(m_SelectedSection == -1)
        return;

    const auto baseAddress = std::get<0>(getSelectedSection().info());

    ui->memoryStartAddress->setText(QString::number(baseAddress, 16));
    m_MemoryStartAddress = baseAddress;
    m_MemoryOffset = { };
    updateMemoryDataEdit();
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
        throw std::runtime_error("Error: m_SelectedModule >= modulesData.size()");

    const auto& selectedModule = modulesData[m_SelectedModule];
    const auto [baseAddress, size] = selectedModule.memento().info();

    const int base = ui->moduleInfoDecimalButton->isChecked() ? 10 : 16;

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
    if(!m_SelectedProcess)
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

    // if(GetAsyncKeyState(VK_RSHIFT) & 1) { // used for testing
    //     printf("BEGIN:\n%ws\n", ui->memoryDataEdit->toHtml().constData());
    // }
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

