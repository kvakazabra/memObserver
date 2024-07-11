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
    void sortByID(); // can use Strategy here?
    void sortByName();

    std::vector<CProcessMemento> m_Processes{ };
};

class CProcess {
public:
    CProcess(const CProcessMemento& process);
    CProcess(std::uint32_t id);
    virtual ~CProcess();

    bool isAttached() const;
    HANDLE handle() const;

    const CProcessMemento& memento() const;
private:
    bool tryAttach();
    void detach();

    CProcessMemento m_Memento;

    HANDLE m_Handle{ INVALID_HANDLE_VALUE };
};

class IProcessIO : public CProcess {
public:
    IProcessIO(const CProcessMemento& process) : CProcess{ process } { }
    IProcessIO(std::uint32_t id) : CProcess{ id } { }
    virtual ~IProcessIO() = default;

    virtual bool readToBuffer(std::uint64_t address, std::uint32_t size, void* buffer) const = 0;
    virtual bool writeFromBuffer(std::uint64_t address, std::uint32_t size, void* buffer) const = 0;
    virtual MBIEx query(std::uint64_t address) const = 0;
    // @return Returns true and oldProtect on success, false and 0 otherwise
    virtual std::tuple<bool, std::uint32_t> protect(std::uint64_t address, std::uint32_t size, std::uint32_t flags) const = 0;

    // invalidMask: 0 - regular byte, 1 - invalid (page protection or something else), 2 - guarded byte
    bool readPages(std::uint64_t startAddress, std::uint32_t size, std::uint8_t* buffer, std::uint8_t* invalidMask = std::nullptr_t());

    template<typename R>
    inline R read(std::uint64_t address) const {
        R buf{ };
        if(!readToBuffer(address, &buf, sizeof(R)))
            return { };
        return buf;
    }
    template<typename W>
    inline bool write(std::uint64_t address, W value) const {
        return writeFromBuffer(address, &value, sizeof(W));
    }
};
