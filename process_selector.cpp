#include "process_selector.h"
#include "ui_process_selector.h"

#include "cmainwindow.h"
#include "process_win32.h"

CProcessSelector::CProcessSelector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CProcessSelector) {

    if(!qobject_cast<CMainWindow*>(this->parent())) // im not sure how qobject_cast works (if it works like dynamic_cast then it's ok)
        throw std::runtime_error("CMainWindow must be a parent of CProcessSelector");

    ui->setupUi(this);

    updateProcessesCombo();
    updateCurrentProcessLabel();
}

CProcessSelector::~CProcessSelector() {
    delete ui;

    m_SelectedProcess.reset();
    m_ProcessList.reset();
}

std::weak_ptr<IProcessIO> CProcessSelector::selectedProcess() const {
    return m_SelectedProcess;
}

const std::vector<CProcessMemento>& CProcessSelector::processes() const {
    return m_ProcessList->data();
}

void CProcessSelector::on_processRefreshButton_clicked() {
    m_ProcessList->refresh();
    updateProcessesCombo();
    emit refreshProcesses();
}

void CProcessSelector::updateProcessesCombo() {
    const auto processes = m_ProcessList->data();
    ui->processComboBox->clear();

    for(const auto& process : processes) {
        ui->processComboBox->addItem(QString(process.format().c_str()));
    }

    updateProcessLastLabel(QString("Total Processes: ") + QString::number(processes.size()));
}

void CProcessSelector::updateProcessLastLabel(const QString& message) {
    ui->processLastMessage->setText(message);
}

void CProcessSelector::updateCurrentProcessLabel(const CProcessMemento& process) {
    ui->processCurrentLabel->setText(QString("Current Process: ") +
                                     QString(process.name().c_str()) + QString(" [") + QString::number(process.id()) + QString("]"));
}

void CProcessSelector::on_processComboBox_activated(int index) {
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

void CProcessSelector::onProcessAttach() {
    updateMainWindowStatusBar(QString("Attached to ") + QString(m_SelectedProcess->memento().name().c_str()) + QString(" successfully"));

    updateProcessLastLabel(QString("Attached successfully"));
    updateCurrentProcessLabel(m_SelectedProcess->memento());

    QObject::connect(m_SelectedProcess.get(), &IProcessIO::invalidProcessSignal, this, &CProcessSelector::invalidProcessSlot);

    emit processAttached();
}

void CProcessSelector::onProcessDetach() {
    emit processDetached();

    m_SelectedProcess.reset();
    updateCurrentProcessLabel();
}

void CProcessSelector::invalidProcessSlot() {
    if(!m_SelectedProcess)
        return;

    CProcessWinIO* winProcess = dynamic_cast<CProcessWinIO*>(m_SelectedProcess.get());
    if(winProcess) {
        updateProcessLastLabel(
            QString("The process exited with code ") +
            QString::number(winProcess->exitCode(), 16)
            );

        updateMainWindowStatusBar(QString("Detached"));
        return;
    }

    updateProcessLastLabel(QString("The process crashed"));
    updateMainWindowStatusBar(QString("Crashed"));
}

void CProcessSelector::updateMainWindowStatusBar(const QString& message) {
    qobject_cast<CMainWindow*>(this->parent())->updateStatusBar(message);
}

void CProcessSelector::on_closeButton_clicked() {
    hide();
}

