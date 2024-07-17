#include "settings.h"
#include "ui_settings.h"

CSettingsWindow::CSettingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CSettings) {
    ui->setupUi(this);
    connectSignals();

    CSettingsManager::settings(this); // initialize CSettingsManager
}

CSettingsWindow::~CSettingsWindow() {
    delete ui;
}

void CSettingsWindow::connectSignals() {
    QObject::connect(ui->processListSortNoneButton, &QAbstractButton::clicked, this, &CSettingsWindow::changeProcessListSortType);
    QObject::connect(ui->processListSortNameButton, &QAbstractButton::clicked, this, &CSettingsWindow::changeProcessListSortType);
    QObject::connect(ui->processListSortIDButton, &QAbstractButton::clicked, this, &CSettingsWindow::changeProcessListSortType);

    QObject::connect(ui->moduleListSortNoneButton, &QAbstractButton::clicked, this, &CSettingsWindow::changeModuleListSortType);
    QObject::connect(ui->moduleListSortNameButton, &QAbstractButton::clicked, this, &CSettingsWindow::changeModuleListSortType);
    QObject::connect(ui->moduleListSortAddressButton, &QAbstractButton::clicked, this, &CSettingsWindow::changeModuleListSortType);
}

void CSettingsWindow::changeProcessListSortType() {
    static std::unordered_map<QString, TSort> titleToSortTypeMap{
        { "None", TSort::None },
        { "Sort by ID", TSort::ID },
        { "Sort by Name", TSort::Name },
    };

    QRadioButton* v1 = qobject_cast<QRadioButton*>(sender());
    if(!v1)
        return;

    ProcessList::m_SortType = titleToSortTypeMap[v1->text()];
    emit processListSortTypeChanged();
}

void CSettingsWindow::changeModuleListSortType() {
    static std::unordered_map<QString, TSort> titleToSortTypeMap{
        { "None", TSort::None },
        { "Sort by Address", TSort::ID },
        { "Sort by Name", TSort::Name },
    };

    QRadioButton* v1 = qobject_cast<QRadioButton*>(sender());
    if(!v1)
        return;

    ModuleList::m_SortType = titleToSortTypeMap[v1->text()];
    emit moduleListSortTypeChanged();
}

TSort CSettings::processListSortType() const {
    return ProcessList::m_SortType;
}

TSort CSettings::moduleListSortType() const {
    return ModuleList::m_SortType;
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

