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
    const std::string& name() const; // todo maybe just compose CProcessMemento here?
    std::uint32_t id() const;
    // memento get
private:
    bool tryAttach();
    void detach();

    std::uint32_t m_Id{ };
    std::string m_Name{ };

    HANDLE m_Handle{ INVALID_HANDLE_VALUE };
};
