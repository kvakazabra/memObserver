#include "utilities.h"

bool Utilities::isHandleValid(HANDLE h) {
    return h != 0 && h != INVALID_HANDLE_VALUE;
}


MBIEx::MBIEx(const MEMORY_BASIC_INFORMATION& a1)
    : MEMORY_BASIC_INFORMATION{ a1 } {

}

std::string MBIEx::format() const {
    static std::unordered_map<int, const std::string> protectionTranslations{
        { PAGE_NOACCESS, "Restricted" },
        { PAGE_EXECUTE, "X" },
        { PAGE_EXECUTE_READ, "RX" },
        { PAGE_EXECUTE_READWRITE, "RWX" },
        { PAGE_EXECUTE_WRITECOPY, "RWX Copy" },
        { PAGE_READONLY, "Read" },
        { PAGE_READWRITE, "RW" },
        { PAGE_WRITECOPY, "RW Copy" },
        { PAGE_TARGETS_INVALID, "PAGE_TARGETS_INVALID" },
        { PAGE_TARGETS_NO_UPDATE, "PAGE_TARGETS_NO_UPDATE" },
        { PAGE_GUARD, "Guard" },
        { PAGE_NOCACHE, "No Cache" },
        { PAGE_WRITECOMBINE, "Write Combine" },
        { PAGE_ENCLAVE_DECOMMIT, "PAGE_ENCLAVE_DECOMMIT" },
        { PAGE_ENCLAVE_THREAD_CONTROL, "PAGE_ENCLAVE_THREAD_CONTROL" },
        { PAGE_ENCLAVE_UNVALIDATED, "PAGE_ENCLAVE_UNVALIDATED" },
    };

    static const std::array<DWORD, 16> protections{
        PAGE_NOACCESS, PAGE_EXECUTE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE,
        PAGE_EXECUTE_WRITECOPY, PAGE_READONLY, PAGE_READWRITE, PAGE_WRITECOPY,
        PAGE_TARGETS_INVALID, PAGE_TARGETS_NO_UPDATE, PAGE_GUARD, PAGE_NOCACHE,
        PAGE_WRITECOMBINE, PAGE_ENCLAVE_DECOMMIT, PAGE_ENCLAVE_THREAD_CONTROL, PAGE_ENCLAVE_UNVALIDATED,
    }; // there's more protections such as PAGE_GRAPHICS_... but in past they didn't work for me, limited use

    std::string protectionsString{ };
    for(auto& p : protections) {
        if(!(this->Protect & p))
            continue;

        protectionsString += protectionTranslations[p];
        protectionsString += " ";
    }

    return protectionsString;
}
