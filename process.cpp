#include "process.h"
#include "settings.h"
#include "ntapi.h"

#include <TlHelp32.h>
#include <Psapi.h>

CProcessMemento::CProcessMemento(const std::uint32_t id, const std::string& name)
    : m_Id{ id }
    , m_Name{ name } { }

CProcessMemento::CProcessMemento(CProcessMemento &&mv) noexcept {
    *this = std::move(mv);
};

CProcessMemento& CProcessMemento::operator=(CProcessMemento&& mv) noexcept {
    this->m_Id = mv.m_Id;
    this->m_Name = std::move(mv.m_Name);
    this->m_Description = std::move(mv.m_Description);
    return *this;
}

int operator<=>(const CProcessMemento& a1, const CProcessMemento& a2) {
    return a1.m_Id - a2.m_Id;
}

std::string CProcessMemento::format() const {
    //char buffer[512]{ };
    //sprintf_s(buffer, "[%d] %s - %s", m_Id, m_Name.c_str(), m_Description.c_str());

    char buffer[256]{ };
    sprintf_s(buffer, "[%d] %s", m_Id, m_Name.c_str());
    return std::string(buffer);
}

std::uint32_t CProcessMemento::id() const {
    return m_Id;
}

const std::string& CProcessMemento::name() const {
    return m_Name;
}

const std::string& CProcessMemento::description() const {
    return m_Description;
}

CProcessList::CProcessList() {
    refresh();
}

void CProcessList::refresh() {
    cleanup();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(!Utilities::isHandleValid(snapshot))
        return;

    PROCESSENTRY32 entry{ };
    entry.dwSize = sizeof(PROCESSENTRY32);

    if(Process32First(snapshot, &entry)) {
        do {
            std::wstring wName{ entry.szExeFile };
            m_Processes.emplace_back(static_cast<std::uint32_t>(entry.th32ProcessID), std::string(wName.begin(), wName.end()));
        } while(Process32Next(snapshot, &entry));
    }

    std::unique_ptr<ISortStrategy<CProcessMemento>> sortStrategy{ std::make_unique<CNoSort<CProcessMemento>>() };
    switch(CSettingsManager::settings()->processListSortType()) {
        case CSettings::TSort::None: break;
        case CSettings::TSort::ID:
            sortStrategy = std::make_unique<CSortProcessesByID>();
            break;
        case CSettings::TSort::Name:
            sortStrategy = std::make_unique<CSortProcessesByName>();
            break;
        default:
            throw std::out_of_range("CProcessList::refresh -> sortType is out of range");
    }
    sortStrategy->sort(m_Processes);
}

const std::vector<CProcessMemento>& CProcessList::data() const {
    return m_Processes;
}

void CProcessList::cleanup() {
    m_Processes.clear();
}

void CSortProcessesByID::sort(std::vector<CProcessMemento>& v) {
    std::sort(v.begin(), v.end());
}

void CSortProcessesByName::sort(std::vector<CProcessMemento>& v) {
    std::sort(v.begin(), v.end(), [](const CProcessMemento& p1, const CProcessMemento& p2) -> bool {
        return p1.name() < p2.name();
    });
}

IProcessIO::IProcessIO(std::uint32_t id)
    : IProcessIO(CProcessMemento(id, "")) { }

IProcessIO::IProcessIO(const CProcessMemento& process)
    : m_Memento{ process } {
}

const CProcessMemento& IProcessIO::memento() const {
    return m_Memento;
}

bool IProcessIO::readPages(std::uint64_t startAddress, std::uint32_t size, std::uint8_t* buffer, CBytesProtectionMask* mask) {
    std::uint32_t remainingSize{ size }, offset{ 0 };
    int p{ }; // protect against deadloop
    while(remainingSize && ++p < 100) {
        std::uint64_t currentAddress = startAddress + offset;
        MBIEx mbi{ query(currentAddress) };
        if(!mbi.BaseAddress && !mbi.AllocationBase) {
            if(mask) mask->setAllProtection(PAGE_NOACCESS);
            return false;
        }

        std::uint64_t pageEndAddress = reinterpret_cast<std::uint64_t>(mbi.BaseAddress) + mbi.RegionSize;
        std::uint32_t toReadSize = pageEndAddress - currentAddress;
        if(toReadSize > remainingSize)
            toReadSize = remainingSize;

        for(std::size_t i = offset; i < offset + toReadSize && mask; ++i) {
            mask->setProtection(i, mbi.Protect);
        }

        bool canRead = !(mbi.Protect & PAGE_GUARD && mbi.Protect & PAGE_NOACCESS);
        if(!canRead) {
            remainingSize -= toReadSize;
            offset += toReadSize;
            continue;
        }

        readToBuffer(currentAddress, toReadSize, buffer + offset);

        remainingSize -= toReadSize;
        offset += toReadSize;
    }
    return true;
}

std::weak_ptr<CModuleList> IProcessIO::moduleList() const {
    return m_ModuleList;
}

CModuleList::CModuleList(IProcessIO* process)
    : m_ThisProcess{ process } {
    if(!m_ThisProcess)
        throw std::runtime_error("m_ThisProcess can not be nullptr");

    refresh();
}

std::vector<CModule> CRetrieveModuleListSnapshot::retrieve() const {
    printf("[%s] Retrieving via CreateToolhelp32Snapshot\n", __FUNCTION__);
    if(!m_ThisProcess)
        return { };

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_ThisProcess->memento().id());
    if(!Utilities::isHandleValid(snapshot))
        return { };

    std::vector<CModule> modules{ };

    MODULEENTRY32 entry{ };
    entry.dwSize = sizeof(MODULEENTRY32);

    if(Module32First(snapshot, &entry)) {
        do {
            std::wstring wName = std::wstring(entry.szModule);
            modules.emplace_back(CModuleMemento(
                reinterpret_cast<std::uint64_t>(entry.modBaseAddr),
                static_cast<std::uint32_t>(entry.modBaseSize),
                std::string(wName.begin(), wName.end())),
                m_ThisProcess
            );
        } while(Module32Next(snapshot, &entry));
    }

    return modules;
}

std::vector<CModule> CRetrieveModuleListEnumerate::retrieve() const {
    return { };
    // std::unique_ptr<HMODULE[]> rawModules{ std::make_unique<HMODULE[]>(1) };
    // DWORD neededSize{ }, size{ 8 };
    // int i{ };
    // while(++i < 100) {
    //     if(!EnumProcessModules(m_hProcess, rawModules.get(), size, &neededSize)) {
    //         printf("EnumProcessModules failed (%d)\n", GetLastError());
    //         return { };
    //     }

    //     if(size != neededSize) {
    //         rawModules = std::make_unique<HMODULE[]>(neededSize / 8);
    //         size = neededSize;
    //     }
    //     else break;
    // }
    // if(i > 95) {
    //     printf("i exceeded the limit in %s\n", __FUNCTION__);
    //     return { };
    // }

    // std::vector<CModule> modules{ };
    // for(std::size_t i = 0; i < static_cast<std::size_t>(size / 8); ++i) {
    //     HMODULE currentModule = rawModules[i];
    //     if(!currentModule)
    //         continue;

    //     MODULEINFO moduleInfo{ };
    //     if(!GetModuleInformation(m_hProcess, currentModule, &moduleInfo, sizeof(MODULEINFO)))
    //         moduleInfo.SizeOfImage = 0x1000; // write a dummy value if handle was stripped

    //     char moduleName[MAX_PATH]{ };
    //     GetModuleBaseNameA(m_hProcess, currentModule, moduleName, MAX_PATH);

    //     modules.emplace_back(CModuleMemento(
    //         reinterpret_cast<std::uint64_t>(currentModule),
    //         static_cast<std::uint32_t>(moduleInfo.SizeOfImage),
    //         std::string(moduleName)),
    //         m_ThisProcess
    //     );
    // }

    // return modules;
}

std::vector<CModule> CRetrieveModuleListPEB::retrieve() const {
    printf("[%s] Retrieving via PEB\n", __FUNCTION__);

    using NtQueryInformationProcessFn = NTSTATUS(*)(HANDLE, int, PVOID, ULONG, PULONG);

    static auto getNtQueryInformationProcess = []() -> NtQueryInformationProcessFn {
        HMODULE ntDll = GetModuleHandleA("ntdll.dll");
        if(!ntDll) {
            ntDll = LoadLibraryA("ntdll.dll");
            if(!ntDll) throw std::runtime_error("Failed loading ntdll");
        }

        return reinterpret_cast<NtQueryInformationProcessFn>(GetProcAddress(ntDll, "NtQueryInformationProcess"));
    };

    static NtQueryInformationProcessFn NtQueryInformationProcess{ getNtQueryInformationProcess() };
    if(!NtQueryInformationProcess)
        throw std::runtime_error("Failed initializing NtQueryInformationProcess");

    std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)>
        hProcess{ OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, m_ThisProcess->memento().id()), [](HANDLE handle) { CloseHandle(handle); } };

    PROCESS_BASIC_INFORMATION basicInfo{ };
    ULONG returnSize{ };
    NTSTATUS status = NtQueryInformationProcess(hProcess.get(), ProcessBasicInformation, &basicInfo, sizeof(PROCESS_BASIC_INFORMATION), &returnSize);
    if(status != STATUS_SUCCESS) {
        printf("NtQueryInformationProcess failed with %x code\n", status);
        return { };
    }

    hProcess.reset(); // close the handle
    if(!basicInfo.PebBaseAddress)
        return { };

    PEB64 peb{ m_ThisProcess->read<PEB64>(reinterpret_cast<std::uint64_t>(basicInfo.PebBaseAddress)) };
    if(!peb.Ldr)
        return { };

    std::vector<CModule> modules{ };
    modules.emplace_back(CModuleMemento(
        static_cast<std::uint64_t>(peb.ImageBaseAddress),
        static_cast<std::uint32_t>(0x1000), // dummy size, it won't hurt: when dumping for example it retrieves image size from headers
        m_ThisProcess->memento().name()),
        m_ThisProcess
    );

    PEB_LDR_DATA pebLdrData{ m_ThisProcess->read<PEB_LDR_DATA>(peb.Ldr) };
    if(!pebLdrData.InLoadOrderModuleList.Flink)
        return { };

    std::uint64_t firstLoadedModule{ reinterpret_cast<std::uint64_t>(pebLdrData.InLoadOrderModuleList.Flink) }, loadedModule{ firstLoadedModule };
    for(std::size_t i = 0; i < 1000; ++i) {
        loadedModule = m_ThisProcess->read<std::uint64_t>(loadedModule); // Flink [0x0]
        if(loadedModule == firstLoadedModule || !loadedModule)
            break;

        LDR_DATA_TABLE_ENTRY entry{ m_ThisProcess->read<LDR_DATA_TABLE_ENTRY>(loadedModule) };
        std::wstring dllNameW(MAX_PATH, '\00');
        if(!m_ThisProcess->readToBuffer(reinterpret_cast<std::uint64_t>(entry.BaseDllName.Buffer), 256, dllNameW.data()))
            continue;

        modules.emplace_back(CModuleMemento(
            reinterpret_cast<std::uint64_t>(entry.DllBase),
            static_cast<std::uint32_t>(entry.SizeOfImage),
            std::string(dllNameW.begin(), dllNameW.end())),
            m_ThisProcess
        );

        //printf("%llx %x %ws\n", entry.DllBase, entry.SizeOfImage, dllNameW.c_str());
    }

    return modules;
}

void CModuleList::refresh() {
    cleanup();

    std::unique_ptr<IRetrieveModuleListStrategy> retrieveStrategy{ std::make_unique<CRetrieveModuleListSnapshot>(m_ThisProcess) };
    switch(CSettingsManager::settings()->moduleListRetrieveMethod()) {
    case CSettings::TRetrieveMethod::None:
    case CSettings::TRetrieveMethod::Snapshot: break;
    case CSettings::TRetrieveMethod::PEB:
        retrieveStrategy = std::make_unique<CRetrieveModuleListPEB>(m_ThisProcess);
        break;
    default:
        throw std::out_of_range("CModuleList::refresh -> retrieveMethod is out of range");
    }

    m_Modules = retrieveStrategy->retrieve();

    std::unique_ptr<ISortStrategy<CModule>> sortStrategy{ std::make_unique<CNoSort<CModule>>() };
    switch(CSettingsManager::settings()->moduleListSortType()) {
        case CSettings::TSort::None: break;
        case CSettings::TSort::ID:
            sortStrategy = std::make_unique<CSortModulesByAddress>();
            break;
        case CSettings::TSort::Name:
            sortStrategy = std::make_unique<CSortModulesByName>();
            break;
        default:
            throw std::out_of_range("CModuleList::refresh -> sortType is out of range");
    }
    sortStrategy->sort(m_Modules);

    swapMainModule();
}

const std::vector<CModule>& CModuleList::data() const {
    return m_Modules;
}

void CModuleList::cleanup() {
    m_Modules.clear();
}

void CModuleList::swapMainModule() {
    const auto& processName = m_ThisProcess->memento().name();
    if(processName.empty())
        return;

    const auto mainModuleIterator = std::find_if(m_Modules.begin(), m_Modules.end(), [&processName](const CModule& module) -> bool {
        if(module.memento().name() == processName)
            return true;
        return false;
    });
    if(mainModuleIterator == m_Modules.end())
        return;

    std::swap(*m_Modules.begin(), *mainModuleIterator);
}

void CSortModulesByAddress::sort(std::vector<CModule>& v) {
    std::sort(v.begin(), v.end(), [](const CModule& m1, const CModule& m2) -> bool {
        return std::get<0>(m1.memento().info()) < std::get<0>(m2.memento().info());
    });
}

void CSortModulesByName::sort(std::vector<CModule>& v) {
    std::sort(v.begin(), v.end(), [](const CModule& m1, const CModule& m2) -> bool {
        return m1.memento().name() < m2.memento().name();
    });
}
