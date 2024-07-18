#pragma once
#include "utilities.h"
#include "module.h"
#include <Windows.h>
#include <vector>
#include <QObject>

class CProcessMemento final : public IFormattable {
public:
    CProcessMemento(const std::uint32_t id, const std::string& name);
    virtual ~CProcessMemento() = default;

    CProcessMemento(const CProcessMemento&) = default;
    CProcessMemento& operator=(const CProcessMemento&) = default;

    CProcessMemento(CProcessMemento&& mv) noexcept;
    CProcessMemento& operator=(CProcessMemento&& mv) noexcept;

    friend int operator<=>(const CProcessMemento& a1, const CProcessMemento& a2);
public:
    virtual std::string format() const override;

    std::uint32_t id() const;
    const std::string& name() const;
    const std::string& description() const;
private:
    std::uint32_t m_Id{ };
    std::string m_Name{ }, m_Description{ };
};

class CSortProcessesByName : public ISortStrategy<CProcessMemento> {
public:
    void sort(std::vector<CProcessMemento>& v) override;
};

class CSortProcessesByID : public ISortStrategy<CProcessMemento> {
public:
    void sort(std::vector<CProcessMemento>& v) override;
};

class CProcessList final {
public:
    CProcessList();
    ~CProcessList() = default;

    // copy/move later
public:
    void refresh();
    const std::vector<CProcessMemento>& data() const;
    void cleanup();
private:
    std::vector<CProcessMemento> m_Processes{ };
};

class CModuleList;

class IProcessIO : public QObject {
    Q_OBJECT
public:
    IProcessIO(const CProcessMemento& process);
    IProcessIO(std::uint32_t id);
    virtual ~IProcessIO() = default;

    const CProcessMemento& memento() const;

    virtual bool readToBuffer(std::uint64_t address, std::uint32_t size, void* buffer) = 0;
    virtual bool writeFromBuffer(std::uint64_t address, std::uint32_t size, void* buffer) = 0;
    virtual MBIEx query(std::uint64_t address) = 0;
    // @return Returns true and oldProtect on success, false and 0 otherwise
    virtual std::tuple<bool, std::uint32_t> protect(std::uint64_t address, std::uint32_t size, std::uint32_t flags) = 0;

    // invalidMask: 0 - regular byte, 1 - invalid (page protection or something else), 2 - guarded byte
    bool readPages(std::uint64_t startAddress, std::uint32_t size, std::uint8_t* buffer, CBytesProtectionMask* mask = std::nullptr_t());

    template<typename R>
    inline R read(std::uint64_t address) {
        R buf{ };
        if(!readToBuffer(address, sizeof(R), &buf))
            return { };
        return buf;
    }
    template<typename W>
    inline bool write(std::uint64_t address, W value) {
        return writeFromBuffer(address, sizeof(W), &value);
    }

    std::weak_ptr<CModuleList> moduleList() const;
private:
    CProcessMemento m_Memento;
protected:
    std::shared_ptr<CModuleList> m_ModuleList;
signals:
    void invalidProcessSignal();
};

class CSortModulesByName : public ISortStrategy<CModule> {
public:
    void sort(std::vector<CModule>& v) override;
};

class CSortModulesByAddress : public ISortStrategy<CModule> {
public:
    void sort(std::vector<CModule>& v) override;
};

class IRetrieveModuleListStrategy {
public:
    IRetrieveModuleListStrategy(IProcessIO* thisProcess)
        : m_ThisProcess{ thisProcess } { }
    virtual ~IRetrieveModuleListStrategy() = default;

    virtual std::vector<CModule> retrieve() const = 0;
protected:
    IProcessIO* m_ThisProcess{ std::nullptr_t() }; // required for CModule
};

class CRetrieveModuleListEnumerate : public IRetrieveModuleListStrategy {
public:
    CRetrieveModuleListEnumerate(IProcessIO* thisProcess, HANDLE hProcess)
        : IRetrieveModuleListStrategy(thisProcess), m_hProcess{ hProcess } { }

    virtual std::vector<CModule> retrieve() const override;
protected:
    HANDLE m_hProcess{ INVALID_HANDLE_VALUE }; // EnumProcessModules requires the minimum access privilege (PROCESS_QUERY_LIMITED_INFORMATION)
};

class CRetrieveModuleListSnapshot : public IRetrieveModuleListStrategy {
public:
    CRetrieveModuleListSnapshot(IProcessIO* thisProcess)
        : IRetrieveModuleListStrategy(thisProcess) { }

    virtual std::vector<CModule> retrieve() const override;
};

class CRetrieveModuleListPEB : public IRetrieveModuleListStrategy {
public:
    CRetrieveModuleListPEB(IProcessIO* thisProcess)
        : IRetrieveModuleListStrategy(thisProcess) { }

    virtual std::vector<CModule> retrieve() const override;
};

// must be instantiated in IProcessIO context
class CModuleList final {
public:
    CModuleList(IProcessIO* process);
    ~CModuleList() = default;
public:
    void refresh();
    const std::vector<CModule>& data() const;
    void cleanup();
private:
    void swapMainModule();

    IProcessIO* m_ThisProcess{ };
    std::vector<CModule> m_Modules{ };
};
