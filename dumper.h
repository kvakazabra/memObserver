#pragma once
#include "process.h"

class IDumper {
public:
    IDumper(std::weak_ptr<IProcessIO> targetProcess);
    virtual ~IDumper() = default;

    const std::vector<std::uint8_t>& cachedDump();
    virtual const std::vector<std::uint8_t>& dump() = 0;
protected:
    mutable std::vector<std::uint8_t> m_Data{ };
    std::weak_ptr<IProcessIO> m_TargetProcess;
};

class CSectionDumper : public IDumper {
public:
    CSectionDumper(std::weak_ptr<IProcessIO> targetProcess, std::uint64_t address, std::uint32_t size);
    ~CSectionDumper() = default;

    virtual const std::vector<std::uint8_t>& dump() override;
private:
    std::uint64_t m_Address{ };
    std::uint32_t m_Size{ };
};

class CModuleDumper : public IDumper {
public:
    CModuleDumper(std::weak_ptr<IProcessIO> targetProcess, std::uint64_t address);
    ~CModuleDumper() = default;

    virtual const std::vector<std::uint8_t>& dump() override;
private:
    void fixSections();

    std::unique_ptr<CModule> m_Module{ };
    std::uint64_t m_Address{ };
    std::uint32_t m_Size{ };
};
