#pragma once
#include "utilities.h"
#include "process.h"

#include <Windows.h>
#include <vector>

class CSection {
public:
    CSection(std::uint64_t baseAddress, std::uint32_t size, char* tag, std::uint32_t protection);


    const char* tag() const;
private:
    std::uint64_t m_BaseAddress{ };
    std::uint32_t m_Size{ };
    char m_Tag[9]{ };
    std::uint32_t m_Protection;
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
    void parseSections();

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
    void sortByName(); // could use Strategy here?

    std::weak_ptr<CProcess> m_Process{ };
    std::vector<CModuleMemento> m_Modules{ };
};
