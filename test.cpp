#include <cstdio>
#include "src/fiber/fiber.h"

void fiber_test(fiber_transfer transfer)
{
    puts("Fiber1");
    fiber_switch(transfer.from, transfer.user); // Get back to where fiber1 is switched
    puts("Fiber1 - End");
    fiber_switch(transfer.from, transfer.user); // Always put this to return to caller at the end of the callback
}

int main()
{
    fiber_stack stack1;
    fiber_stack_init(&stack1, 80 * 1024);
    puts("Main");
    void *fiber = fiber_create(stack1, fiber_test);
    fiber_transfer t = fiber_switch(fiber, (void *)1);
    puts("Back to main");
    fiber_switch(t.from, t.user);
    puts("End");
    return 0;
}