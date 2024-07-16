#include "settings.h"
#include "ui_settings.h"

CSettingsWindow::CSettingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CSettings) {
    ui->setupUi(this);
    connectButtons();
}

CSettingsWindow::~CSettingsWindow() {
    delete ui;
}

void CSettingsWindow::connectButtons() {
    // nothing here...
}


bool CSettingsWindow::moduleInfoIsHexadecimalFormat() const {
    return ModuleInfo::m_IsHexadecimal;
}

bool CSettingsWindow::memoryViewIsOffsetRelative() const {
    return MemoryView::m_IsOffsetRelative;
}

bool CSettingsWindow::memoryViewIsAutoUpdateEnabled() const {
    return MemoryView::m_AutoUpdateEnabled;
}

int CSettingsWindow::memoryViewAutoUpdateInterval() const {
    return MemoryView::m_AutoUpdateInterval;
}

void CSettingsWindow::on_settingsCloseButton_clicked() {
    hide();
}

void CSettingsWindow::on_memoryRealTimeUpdateCheckbox_stateChanged(int arg1) {
    bool isEnabled = arg1 == 2;
    MemoryView::m_AutoUpdateEnabled= isEnabled;
}

void CSettingsWindow::on_memoryUpdateIntervalSlider_valueChanged(int value) {
    MemoryView::m_AutoUpdateInterval = value;
}

void CSettingsWindow::on_memoryOffsetAbsoluteButton_clicked() {
    MemoryView::m_IsOffsetRelative = false;
    emit memoryViewFormatChanged();
}

void CSettingsWindow::on_memoryOffsetRelativeButton_clicked() {
    MemoryView::m_IsOffsetRelative = true;
    emit memoryViewFormatChanged();
}

void CSettingsWindow::on_moduleInfoDecimalButton_clicked() {
    ModuleInfo::m_IsHexadecimal = false;
    emit moduleInfoFormatChanged();
}

void CSettingsWindow::on_moduleInfoHexadecimalButton_clicked() {
    ModuleInfo::m_IsHexadecimal = true;
    emit moduleInfoFormatChanged();
}

