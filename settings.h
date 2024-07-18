#pragma once
#include <QDialog>



namespace Ui {
class CSettings;
}

class CSettings {
public:
    enum class TRetrieveMethod {
        None,
        Snapshot,
        PEB,
    };
    enum class TSort { // ik its not the best idea to put it here
        None,
        ID, // not an ID for modules
        Name,
    };

    TSort processListSortType() const;
    TSort moduleListSortType() const;
    bool moduleInfoIsHexadecimalFormat() const;
    TRetrieveMethod moduleListRetrieveMethod() const;
    bool memoryViewIsOffsetRelative() const;
    bool memoryViewIsAutoUpdateEnabled() const;
    int memoryViewAutoUpdateInterval() const;
protected:
    class ProcessList {
    public:
        inline static TSort m_SortType{ TSort::Name };
    };
    class ModuleList {
    public:
        inline static TSort m_SortType{ TSort::Name };
        inline static TRetrieveMethod m_RetrieveMethod{ TRetrieveMethod::Snapshot };
    };
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

// This class is made for places in code where you can not directly access actual CSettings instance like in process.cpp
class CSettingsManager {
public:
    static CSettings* settings(CSettings* inst = nullptr) {
        static CSettings* instance = inst;
        if(!instance)
            throw std::runtime_error("Before calling CSettingsManager::settings you must set an instance of CSettings");
        return instance;
    }
};

class CSettingsWindow : public QDialog, public CSettings {
    Q_OBJECT
public:
    explicit CSettingsWindow(QWidget *parent = nullptr);
    ~CSettingsWindow();
private slots:
    void changeProcessListSortType();
    void changeModuleListSortType();
    void changeModuleRetrieveMethod();

    void on_memoryRealTimeUpdateCheckbox_stateChanged(int arg1);
    void on_memoryUpdateIntervalSlider_valueChanged(int value);
    void on_memoryOffsetAbsoluteButton_clicked();
    void on_memoryOffsetRelativeButton_clicked();

    void on_moduleInfoDecimalButton_clicked();
    void on_moduleInfoHexadecimalButton_clicked();

    void on_settingsCloseButton_clicked();
signals:
    void moduleInfoFormatChanged();
    void memoryViewFormatChanged();
    void processListSortTypeChanged();
    void moduleListSortTypeChanged();
    void moduleListRetrieveMethodChanged();
private:
    void connectSignals();
private:
    Ui::CSettings *ui;
};
