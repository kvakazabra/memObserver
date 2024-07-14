#pragma once
#include "process.h"

class IDumper {
public:
    IDumper(std::weak_ptr<IProcessIO> targetProcess);
    virtual ~IDumper() = default;

    const std::vector<std::uint8_t>& cachedDump() const;
    virtual const std::vector<std::uint8_t>& dump() const = 0;
protected:
    mutable std::vector<std::uint8_t> m_Data{ };
    std::weak_ptr<IProcessIO> m_TargetProcess;
};

class CSectionDumper : public IDumper {
public:
    CSectionDumper(std::weak_ptr<IProcessIO> targetProcess, std::uint64_t address, std::uint32_t size);
    ~CSectionDumper() = default;

    virtual const std::vector<std::uint8_t>& dump() const override;
private:
    std::uint64_t m_Address{ };
    std::uint32_t m_Size{ };
};
