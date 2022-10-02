#include "fiber.h"

#include <cstdint>
#include <iostream>

#include "Utils/os.h"

#define DEFAULT_STACK_SIZE 131072 // 128kb

// Fwd declare ASM functions
extern "C" fiber_transfer jump_fcontext(const void *, void *);
extern "C" void *make_fcontext(void *, size_t, fiber_cb *);

bool fiber_stack_init(fiber_stack *fstack, unsigned int size)
{
    if (size == 0)
        size = DEFAULT_STACK_SIZE;
    size = (uint32_t)os_align_pagesz(size);
    void *ptr;

    ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    DWORD old_opts;
    VirtualProtect(ptr, os_pagesz(), PAGE_READWRITE | PAGE_GUARD, &old_opts);

    fstack->sptr = (uint8_t *)ptr + size; // Move to end of the memory block for stack pointer
    fstack->ssize = size;
    return true;
}

void fiber_stack_init_ptr(fiber_stack *fstack, void *ptr, unsigned int size)
{
    // buffer size must be dividable to OS page size
    fstack->sptr = ptr;
    fstack->ssize = size;
}

void fiber_stack_release(fiber_stack *fstack)
{
    void *ptr = (uint8_t *)fstack->sptr - fstack->ssize;
    VirtualFree(ptr, 0, MEM_RELEASE);
}

fiber_t fiber_create(const fiber_stack stack, fiber_cb *fiber_cb)
{
    return make_fcontext(stack.sptr, stack.ssize, fiber_cb);
}

fiber_transfer fiber_switch(const fiber_t to, void *userData)
{
    return jump_fcontext(to, userData);
}