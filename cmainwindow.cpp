#include "cmainwindow.h"
#include "./ui_cmainwindow.h"

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

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CMainWindow) {
    ui->setupUi(this);

    showConsole();
    setupTextures();
    updateProcessesCombo();
    updateCurrentProcessLabel();
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
    auto selectedProcess = std::make_shared<CProcess>(m_ProcessList->data()[index]);
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

void CMainWindow::on_moduleInfoHexadecimalButton_clicked() {
    updateModuleInfoLines();
}

void CMainWindow::on_moduleInfoDecimalButton_clicked() {
    updateModuleInfoLines();
}

