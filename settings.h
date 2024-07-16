#pragma once

#include <QDialog>

namespace Ui {
class CSettings;
}

class CSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CSettingsWindow(QWidget *parent = nullptr);
    ~CSettingsWindow();

    bool moduleInfoIsHexadecimalFormat() const;
    bool memoryViewIsOffsetRelative() const;
    bool memoryViewIsAutoUpdateEnabled() const;
    int memoryViewAutoUpdateInterval() const;
private slots:
    void on_settingsCloseButton_clicked();
    void on_memoryRealTimeUpdateCheckbox_stateChanged(int arg1);
    void on_memoryUpdateIntervalSlider_valueChanged(int value);
    void on_memoryOffsetAbsoluteButton_clicked();
    void on_memoryOffsetRelativeButton_clicked();
    void on_moduleInfoDecimalButton_clicked();
    void on_moduleInfoHexadecimalButton_clicked();
signals:
    void moduleInfoFormatChanged();
    void memoryViewFormatChanged();
private:
    Ui::CSettings *ui;

    void connectButtons();

    class ModuleInfo {
    public:
        inline static bool m_IsHexadecimal{ true };
    };
    class MemoryView {
    public:
        inline static bool m_IsOffsetRelative{ true };
        inline static std::atomic<bool> m_AutoUpdateEnabled{ true };
        inline static std::atomic<int> m_AutoUpdateInterval{ 500 };
    };
};
