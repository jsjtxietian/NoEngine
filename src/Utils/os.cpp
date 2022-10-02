#include "os.h"

size_t os_pagesz(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (size_t)si.dwPageSize;
}

size_t os_align_pagesz(size_t size)
{
    size_t page_sz = os_pagesz();
    size_t page_cnt = (size + page_sz - 1) / page_sz;
    return page_cnt * page_sz;
}

int sx_os_numcores(void)
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}


