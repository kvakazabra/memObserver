#pragma once
#include "utilities.h"

#include <Windows.h>
#include <vector>

class CSection {
public:
    CSection(std::uint64_t baseAddress, std::uint32_t size, char* tag);

    std::tuple<std::uint64_t, std::uint32_t> info() const;
    const char* tag() const;
private:
    std::uint64_t m_BaseAddress{ };
    std::uint32_t m_Size{ };
    char m_Tag[9]{ };
};

class CModuleMemento final : public IFormattable {
public:
    CModuleMemento(const std::uint64_t baseAddress, const std::uint32_t size, const std::string& name);
    virtual ~CModuleMemento() = default;

    CModuleMemento(const CModuleMemento&) = default;
    CModuleMemento& operator=(const CModuleMemento&) = default;

    CModuleMemento(CModuleMemento&& mv) noexcept;
    CModuleMemento& operator=(CModuleMemento&& mv) noexcept;

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

class IProcessIO;

class CModule {
public:
    // it should be created within a context of a IProcessIO
    CModule(const CModuleMemento& module, IProcessIO* process);
    ~CModule() = default;
public:
    const CModuleMemento& memento() const;
    const std::vector<CSection>& sections() const;
private:
    void parseSections();

    CModuleMemento m_Memento;
    IProcessIO* m_ThisProcess{ };
    std::vector<CSection> m_Sections{ };
};
