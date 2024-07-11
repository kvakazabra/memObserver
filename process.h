#pragma once
#include "utilities.h"

#include <Windows.h>
#include <vector>

class CProcessMemento final : public IFormattable {
public:
    CProcessMemento(const std::uint32_t id, const std::string& name);
    virtual ~CProcessMemento() = default;

    CProcessMemento(const CProcessMemento&) = default;
    CProcessMemento& operator=(const CProcessMemento&) = default;

    CProcessMemento(CProcessMemento&& mv);
    CProcessMemento& operator=(CProcessMemento&& mv);

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

class CProcessList final {
public:
    CProcessList();
    ~CProcessList();

    // copy/move later
public:
    void refresh();
    const std::vector<CProcessMemento>& data() const;
    void cleanup();
private:
    void sortByID();

    std::vector<CProcessMemento> m_Processes{ };
};

class CProcess final {
public:
    CProcess(const CProcessMemento& process);
    CProcess(std::uint32_t id);
    ~CProcess();

    bool isAttached() const;
    HANDLE handle() const;

    const CProcessMemento& memento() const;
private:
    bool tryAttach();
    void detach();

    CProcessMemento m_Memento;

    HANDLE m_Handle{ INVALID_HANDLE_VALUE };
};

class CModuleMemento final : public IFormattable {
public:
    CModuleMemento(const std::uint64_t baseAddress, const std::uint32_t size, const std::string& name);
    virtual ~CModuleMemento() = default;

    CModuleMemento(const CModuleMemento&) = default;
    CModuleMemento& operator=(const CModuleMemento&) = default;

    CModuleMemento(CModuleMemento&& mv);
    CModuleMemento& operator=(CModuleMemento&& mv);

    friend int operator<=>(const CModuleMemento& a1, const CModuleMemento& a2);
public:
    virtual std::string format() const override;

    std::tuple<std::uint64_t, std::uint32_t> info() const;
    const std::string& name() const;
private:
    std::uint64_t m_BaseAddress{ };
    std::uint32_t m_Size{ };
    std::string m_Name{ };
};

class CModuleList final {
public:
    CModuleList(std::weak_ptr<CProcess> process);
    ~CModuleList();

    // copy/move later
public:
    void refresh();
    const std::vector<CModuleMemento>& data() const;
    void cleanup();
private:
    void sortByAddress();

    std::weak_ptr<CProcess> m_Process{ };
    std::vector<CModuleMemento> m_Modules{ };
};
