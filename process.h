#pragma once
#include "utilities.h"

#include <Windows.h>
#include <vector>

class CProcessMemento final : public IFormattable {
public:
    CProcessMemento() = delete;
    CProcessMemento(const std::uint32_t id, const std::string& name)
        : m_Id{ id }
        , m_Name{ name }
    { }
    virtual ~CProcessMemento() = default;

    CProcessMemento(const CProcessMemento&) = default;
    CProcessMemento& operator=(const CProcessMemento&) = default;

    CProcessMemento(CProcessMemento&& mv);
    CProcessMemento& operator=(CProcessMemento&& mv);

    friend int operator<=>(const CProcessMemento& a1, const CProcessMemento& a2);
public:

    virtual std::string format() const override {
        //char buffer[512]{ };
        //sprintf_s(buffer, "[%d] %s - %s", m_Id, m_Name.c_str(), m_Description.c_str());

        char buffer[256]{ };
        sprintf_s(buffer, "[%d] %s", m_Id, m_Name.c_str());
        return std::string(buffer);
    }
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
