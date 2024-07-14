#include "settings.h"
#include "ui_settings.h"

CSettings::CSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CSettings) {
    ui->setupUi(this);
    connectButtons();
}

CSettings::~CSettings() {
    delete ui;
}

void CSettings::connectButtons() {
    // nothing here...
}


bool CSettings::moduleInfoIsHexadecimalFormat() const {
    return ModuleInfo::m_IsHexadecimal;
}

bool CSettings::memoryViewIsOffsetRelative() const {
    return MemoryView::m_IsOffsetRelative;
}

bool CSettings::memoryViewIsAutoUpdateEnabled() const {
    return MemoryView::m_AutoUpdateEnabled;
}

int CSettings::memoryViewAutoUpdateInterval() const {
    return MemoryView::m_AutoUpdateInterval;
}

void CSettings::on_settingsCloseButton_clicked() {
    hide();
}

void CSettings::on_memoryRealTimeUpdateCheckbox_stateChanged(int arg1) {
    bool isEnabled = arg1 == 2;
    MemoryView::m_AutoUpdateEnabled= isEnabled;
}

void CSettings::on_memoryUpdateIntervalSlider_valueChanged(int value) {
    MemoryView::m_AutoUpdateInterval = value;
}

void CSettings::on_memoryOffsetAbsoluteButton_clicked() {
    MemoryView::m_IsOffsetRelative = false;
    emit memoryViewFormatChanged();
}

void CSettings::on_memoryOffsetRelativeButton_clicked() {
    MemoryView::m_IsOffsetRelative = true;
    emit memoryViewFormatChanged();
}

void CSettings::on_moduleInfoDecimalButton_clicked() {
    ModuleInfo::m_IsHexadecimal = false;
    emit moduleInfoFormatChanged();
}

void CSettings::on_moduleInfoHexadecimalButton_clicked() {
    ModuleInfo::m_IsHexadecimal = true;
    emit moduleInfoFormatChanged();
}

