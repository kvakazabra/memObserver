#include "module_list.h"
#include "ui_module_list.h"
#include "cmainwindow.h"
#include "dumper.h"

CModuleListWindow::CModuleListWindow(QWidget *parent, CSettingsWindow* settings, CProcessSelectorWindow* processSelector)
    : QDialog(parent)
    , ui(new Ui::CModuleListWindow)
    , m_Settings{ settings }
    , m_ProcessSelector{ processSelector } {
    ui->setupUi(this);

    if(!qobject_cast<CMainWindow*>(this->parent())) // im not sure how qobject_cast works (if it works like dynamic_cast then it's ok)
        throw std::runtime_error("CMainWindow must be a parent of CModuleListWindow");

    connectSignals();
}

CModuleListWindow::~CModuleListWindow() {
    delete ui;
}

void CModuleListWindow::connectSignals() {
    QObject::connect(m_Settings, &CSettingsWindow::moduleInfoFormatChanged, this, &CModuleListWindow::onModuleInfoFormatChanged);

    QObject::connect(m_Settings, &CSettingsWindow::moduleListRetrieveMethodChanged, this, &CModuleListWindow::on_modulesRefreshButton_clicked);
    QObject::connect(m_Settings, &CSettingsWindow::moduleListSortTypeChanged, this, &CModuleListWindow::on_modulesRefreshButton_clicked);

    QObject::connect(m_ProcessSelector, &CProcessSelectorWindow::processAttached, this, &CModuleListWindow::onProcessAttach);
    QObject::connect(m_ProcessSelector, &CProcessSelectorWindow::processDetached, this, &CModuleListWindow::onProcessDetach);
}

void CModuleListWindow::updateModuleList() {
    ui->moduleList->clear();

    for(auto& module : m_ProcessSelector->selectedProcess()->moduleList().lock()->data()) {
        ui->moduleList->addItem(QString(module.memento().format().c_str()));
    }
}

void CModuleListWindow::onProcessAttach() {
    if(m_ProcessSelector->selectedProcess()->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    updateModuleList();
}

void CModuleListWindow::onProcessDetach() {
    ui->sectionList->clear();
    selectSection(-1);

    updateSectionDumpLastLabel();
    updateModuleDumpLastLabel();

    ui->moduleList->clear();
    selectModule(-1);
}

void CModuleListWindow::selectModule(int idx) {
    m_SelectedModule = idx;
    updateModuleInfoLines();
}

void CModuleListWindow::selectSection(int idx) {
    m_SelectedSection = idx;
    updateSectionInfoLines();
}

const CModule& CModuleListWindow::getSelectedModule() {
    if(m_SelectedModule == -1)
        throw std::out_of_range("Check m_SelectedModule for -1 before calling getSelectedModule");

    if(m_ProcessSelector->selectedProcess()->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    return m_ProcessSelector->selectedProcess()->moduleList().lock()->data()[m_SelectedModule];
}

const CSection& CModuleListWindow::getSelectedSection() {
    if(m_SelectedSection == -1)
        throw std::out_of_range("Check m_SelectedSection for -1 before calling getSelectedSection");

    const auto& selectedModule = getSelectedModule();
    if(m_SelectedSection >= selectedModule.sections().size())
        throw std::out_of_range("Error: m_SelectedSection >= sections.size()");

    return selectedModule.sections()[m_SelectedSection];
}

void CModuleListWindow::goToSelectedModule() {
    if(m_SelectedModule == -1)
        return;

    const auto baseAddress = std::get<0>(getSelectedModule().memento().info());
    goToMemoryAddress(baseAddress);
}

void CModuleListWindow::goToSelectedSection() {
    if(m_SelectedSection == -1)
        return;

    const auto baseAddress = std::get<0>(getSelectedSection().info());
    goToMemoryAddress(baseAddress);
}

void CModuleListWindow::on_modulesRefreshButton_clicked() {
    if(!m_ProcessSelector->selectedProcess())
        return;

    if(m_ProcessSelector->selectedProcess()->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    m_ProcessSelector->selectedProcess()->moduleList().lock()->refresh();
    updateModuleList();
    selectModule(-1);
}

void CModuleListWindow::updateSectionInfoLines() {
    if(m_SelectedSection == -1) {
        ui->sectionInfoBaseAddressLine->setText("");
        ui->sectionInfoSizeLine->setText("");
        return;
    }

    const auto [baseAddress, size] = getSelectedSection().info();
    ui->sectionInfoBaseAddressLine->setText(QString::number(baseAddress, 16));
    ui->sectionInfoSizeLine->setText(QString::number(size, 16));
}

void CModuleListWindow::updateSectionDumpLastLabel(const QString& message) {
    ui->dumpSectionLastMessageLabel->setText(message);
}

void CModuleListWindow::updateModuleDumpLastLabel(const QString& message) {
    ui->dumpModuleLastMessageLabel->setText(message);
}

void CModuleListWindow::updateModuleInfoLines() {
    ui->sectionList->clear();

    if(m_SelectedModule == -1) {
        ui->moduleInfoNameLine->setText("");
        ui->moduleInfoBaseAddressLine->setText("");
        ui->moduleInfoSizeLine->setText("");
        return;
    }

    if(m_ProcessSelector->selectedProcess()->moduleList().expired())
        throw std::runtime_error("Expired std::weak_ptr<CModuleList>, this should not happen");

    const auto modulesData = m_ProcessSelector->selectedProcess()->moduleList().lock()->data();
    if(m_SelectedModule >= modulesData.size())
        throw std::out_of_range("Error: m_SelectedModule >= modulesData.size()");

    const auto& selectedModule = modulesData[m_SelectedModule];
    const auto [baseAddress, size] = selectedModule.memento().info();

    const int base = m_Settings->moduleInfoIsHexadecimalFormat() ? 16 : 10;

    ui->moduleInfoNameLine->setText(QString(selectedModule.memento().name().c_str()));
    ui->moduleInfoBaseAddressLine->setText(QString::number(baseAddress, base));
    ui->moduleInfoSizeLine->setText(QString::number(size, base));

    for(auto& section : selectedModule.sections()) {
        ui->sectionList->addItem(QString(section.tag()));
    }
}

void CModuleListWindow::on_moduleList_currentRowChanged(int currentRow) {
    selectModule(currentRow);
}

void CModuleListWindow::on_sectionList_currentRowChanged(int currentRow) {
    selectSection(currentRow);
}

void CModuleListWindow::onModuleInfoFormatChanged() {
    updateModuleInfoLines();
}

void CModuleListWindow::on_sectionListGoToButton_clicked() {
    goToSelectedSection();
}

void CModuleListWindow::on_sectionList_itemDoubleClicked(QListWidgetItem *item) {
    selectSection(item->listWidget()->row(item));
    goToSelectedSection();
}

void CModuleListWindow::on_dumpSectionButton_clicked() {
    if(!m_ProcessSelector->selectedProcess() || m_SelectedSection == -1) {
        updateSectionDumpLastLabel("You must select a process and a section you want to dump");
        return;
    }

    const auto& selectedSection = getSelectedSection();
    auto [baseAddress, size] = selectedSection.info();
    if(!baseAddress || !size) {
        updateSectionDumpLastLabel("Base address or size is null, can not perform an operation");
        return;
    }

    const auto dumpBuffer = CSectionDumper(m_ProcessSelector->selectedProcess(), baseAddress, size).dump();
    const auto dumpPath =
        Utilities::generatePathForDump(
            m_ProcessSelector->selectedProcess()->memento().name(),
            m_ProcessSelector->selectedProcess()->moduleList().lock()->data()[m_SelectedModule].memento().name(),
            selectedSection.tag()
            );

    if(std::filesystem::exists(dumpPath)) {
        updateSectionDumpLastLabel("File with the name of the dump already exist, delete it manually to proceed");
        return;
    }

    std::ofstream outFile(dumpPath, std::ios::trunc | std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(dumpBuffer.data()), dumpBuffer.size());
    updateSectionDumpLastLabel("Dumped successfully");
    updateMainWindowStatusBar("Saved to " + QString(dumpPath.c_str()));
}

void CModuleListWindow::on_dumpModuleButton_clicked() {
    if(!m_ProcessSelector->selectedProcess() || m_SelectedModule == -1) {
        updateModuleDumpLastLabel("You must select a process and a module you want to dump");
        return;
    }

    const auto& module = getSelectedModule();
    auto [baseAddress, size] = module.memento().info();
    if(!baseAddress || !size) {
        updateModuleDumpLastLabel("Base address or size is null, can not perform an operation");
        return;
    }

    const auto dumpBuffer = CModuleDumper(m_ProcessSelector->selectedProcess(), baseAddress).dump();
    const auto dumpPath =
        Utilities::generatePathForDump(
            m_ProcessSelector->selectedProcess()->memento().name(),
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
    updateMainWindowStatusBar("Saved to " + QString(dumpPath.c_str()));
}

void CModuleListWindow::on_moduleList_itemDoubleClicked(QListWidgetItem *item) {
    selectModule(item->listWidget()->row(item));
    goToSelectedModule();
}

void CModuleListWindow::updateMainWindowStatusBar(const QString& message) {
    qobject_cast<CMainWindow*>(this->parent())->updateStatusBar(message);
}

void CModuleListWindow::goToMemoryAddress(std::uint64_t address) {
    qobject_cast<CMainWindow*>(this->parent())->goToMemoryAddress(address);
}

void CModuleListWindow::on_closeButton_clicked() {
    hide();
}

