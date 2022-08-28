#pragma once

typedef struct fiber_transfer
{
    void* from;
    void *user;
} fiber_transfer;

typedef struct fiber_stack
{
    void * sptr;
    unsigned int ssize;
} fiber_stack;

using fiber_cb = void (fiber_transfer);

// Low-level functions
bool fiber_stack_init(fiber_stack *fstack, unsigned int size = 0);
void fiber_stack_init_ptr(fiber_stack *fstack, void *ptr, unsigned int size);
void fiber_stack_release(fiber_stack *fstack);

void * fiber_create(const fiber_stack stack, fiber_cb *fiber_cb);
fiber_transfer fiber_switch(const void * to, void *user);
