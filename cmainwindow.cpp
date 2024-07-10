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

    updateProcessLastMessage(QString("Total Processes: ") + QString::number(processes.size()));
}

void CMainWindow::updateProcessLastMessage(const QString& message) {
    ui->processLastMessage->setText(message);
}

void CMainWindow::updateCurrentProcessLabel(const CProcessMemento& process) {
    ui->processCurrentLabel->setText(QString("Current Process: ") +
                                     QString(process.name().c_str()) + QString(" [") + QString::number(process.id()) + QString("]"));
}

void CMainWindow::on_processComboBox_activated(int index) {
    if(index > m_ProcessList->data().size())
        throw std::runtime_error("Out of bounds: m_ProcessList");

    onProcessDetach();
    auto selectedProcess = std::make_shared<CProcess>(m_ProcessList->data()[index]);
    if(!selectedProcess->isAttached()) {
        updateProcessLastMessage(QString("Failed to attach"));
        return;
    }

    m_SelectedProcess = std::move(selectedProcess);
    onProcessAttach();
}

void CMainWindow::onProcessAttach() {
    updateProcessLastMessage(QString("Attached to ") + QString(m_SelectedProcess->name().c_str()) + QString(" successfully"));
    updateCurrentProcessLabel(CProcessMemento(m_SelectedProcess->id(), m_SelectedProcess->name())); // TODO (seriously): compose a fucking memento into a fucking process, this is the third time I needed it

    m_ModulesList = std::make_shared<CModuleList>(m_SelectedProcess);
    for(auto& module : m_ModulesList->data()) {
        ui->modulesList->addItem(QString(module.format().c_str()));
    }
}

void CMainWindow::onProcessDetach() {
    ui->modulesList->clear();
    m_ModulesList.reset();

    m_SelectedProcess.reset();

    updateCurrentProcessLabel(); // clean current process info
}

void CMainWindow::on_modulesRefreshButton_clicked() {
    if(!m_ModulesList)
        return;

    m_ModulesList->refresh();
}

