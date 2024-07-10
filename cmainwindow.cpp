#include "cmainwindow.h"
#include "./ui_cmainwindow.h"

#include <QPixmap>

void CMainWindow::setupTextures() {
    QPixmap refreshIcon(":/resources/images/refreshIcon.png");
    if(refreshIcon.isNull())
        throw std::runtime_error("Failed to load resources");
}

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CMainWindow)
{
    ui->setupUi(this);

    setupTextures();
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::on_processRefreshButton_clicked()
{
    m_ProcessList->refresh();
    updateProcessesCombo();
}

void CMainWindow::updateProcessesCombo() {
    const auto processes = m_ProcessList->data();
    ui->processComboBox->clear();

    for(const auto& process : processes) {
        ui->processComboBox->addItem(QString(process.format().c_str()));
    }
}
