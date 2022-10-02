// os related helper functions

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

size_t os_pagesz(void);
size_t os_align_pagesz(size_t size);
int sx_os_numcores(void);

inline size_t sx_os_minstacksz(void)
{
    return 32768;    // 32kb
}