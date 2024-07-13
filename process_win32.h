#pragma once
#include "process.h"

class CProcessWinIO : public IProcessIO {
public:
    CProcessWinIO(const CProcessMemento& process);
    CProcessWinIO(std::uint32_t id);
    virtual ~CProcessWinIO();

    bool isAttached();
    HANDLE handle() const;

    virtual bool readToBuffer(std::uint64_t address, std::uint32_t size, void* buffer) override;
    virtual bool writeFromBuffer(std::uint64_t address, std::uint32_t size, void* buffer) override;
    virtual MBIEx query(std::uint64_t address) override;
    virtual std::tuple<bool, std::uint32_t> protect(std::uint64_t address, std::uint32_t size, std::uint32_t flags) override;

    std::uint32_t exitCode() const;
private:
    bool tryAttach();
    void detach();

    HANDLE m_Handle{ INVALID_HANDLE_VALUE };
    std::uint32_t m_ExitCode{ UINT_MAX };

    //std::uint64_t allocate(std::uint32_t size, std::uint32_t flags, std::uint32_t flags2); // Wrappers around VirtualAllocEx, VirtualFreeEx
    //bool free(std::uint64_t address, std::uint32_t flags);
};
