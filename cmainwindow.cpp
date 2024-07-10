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
}

CMainWindow::~CMainWindow() {
    delete ui;
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

void CMainWindow::on_processComboBox_activated(int index) {
    if(index > m_ProcessList->data().size())
        throw std::runtime_error("Out of bounds: m_ProcessList");

    m_SelectedProcess.reset();
    m_SelectedProcess = std::make_unique<CProcess>(m_ProcessList->data()[index]);

    if(!m_SelectedProcess->isAttached()) {
        updateProcessLastMessage(QString("Failed to attach"));
        m_SelectedProcess.reset();
        return;
    }

    updateProcessLastMessage(QString("Attached to ") + QString(m_ProcessList->data()[index].name().c_str()) + QString(" successfully"));
}

