// #include "job.h"
// #include <cstdlib>
// #include <cstring>

// #include "fiber/fiber.h"
// #include "Utils/os.h"

// typedef struct sx__job
// {
//     int job_index;
//     int done;
//     uint32_t owner_tid;
//     uint32_t tags;
//     fiber_stack stack_mem;
//     fiber_t fiber;
//     fiber_t selector_fiber;
//     sx_job_t counter;
//     sx_job_t wait_counter;
//     sx_job_context *ctx;
//     sx_job_cb *callback;
//     void *user;
//     int range_start;
//     int range_end;
//     sx_job_priority priority;
//     struct sx__job *next;
//     struct sx__job *prev;
// } sx__job;

// typedef struct sx__job_thread_data
// {
//     sx__job *cur_job;
//     fiber_stack selector_stack;
//     fiber_t selector_fiber;
//     int thread_index;
//     uint32_t tid;
//     uint32_t tags;
//     bool main_thrd;
// } sx__job_thread_data;

// typedef struct sx__job_select_result
// {
//     sx__job *job;
//     bool waiting_list_alive;
// } sx__job_select_result;

// static sx__job_thread_data *sx__job_create_tdata(uint32_t tid, int index,
//                                                  bool main_thrd)
// {
//     sx__job_thread_data *tdata =
//         (sx__job_thread_data *)malloc(sizeof(sx__job_thread_data));
//     memset(tdata, 0x0, sizeof(sx__job_thread_data));

//     tdata->thread_index = index;
//     tdata->tid = tid;
//     tdata->tags = 0xffffffff;
//     tdata->main_thrd = main_thrd;

//     bool r = fiber_stack_init(&tdata->selector_stack, (int)sx_os_minstacksz());
//     return tdata;
// }

// // Threads run this function to pick a job from the list and execute the job fiber
// static void sx__job_selector_fn(sx_fiber_transfer transfer)
// {
//     sx_job_context *ctx = (sx_job_context *)transfer.user;
//     sx__job_thread_data *tdata = (sx__job_thread_data *)sx_tls_get(ctx->thread_tls);
//     sx_assert(tdata);

//     while (!ctx->quit)
//     {
//         sx_semaphore_wait(&ctx->sem, -1); // Wait for a job

//         // Select the best job in the waiting list
//         sx__job_select_result r = sx__job_select(ctx, tdata->tid, tdata->tags);

//         //
//         if (r.job)
//         {
//             // Job is a slave (in wait mode), get back to it and remove slave mode
//             if (r.job->owner_tid > 0)
//             {
//                 sx_assert(tdata->cur_job == NULL);
//                 r.job->owner_tid = 0;
//             }

//             // Run the job from beginning, or continue after 'wait'
//             tdata->selector_fiber = r.job->selector_fiber;
//             tdata->cur_job = r.job;
//             r.job->fiber = sx_fiber_switch(r.job->fiber, r.job).from;

//             // Delete the job and decrement job counter if it's done
//             if (r.job->done)
//             {
//                 tdata->cur_job = NULL;
//                 sx_atomic_fetch_sub32(r.job->counter, 1);
//                 sx__del_job(ctx, r.job);
//             }
//         }
//         else if (r.waiting_list_alive)
//         {
//             // If we have a pending job, continue this loop one more time
//             sx_semaphore_post(&ctx->sem, 1);
//             sx_relax_cpu();
//         }
//     }

//     // Back to caller thread
//     sx_fiber_switch(transfer.from, transfer.user);
// }

// static int sx__job_thread_fn(void *user1, void *user2)
// {
//     sx_job_context *ctx = (sx_job_context *)user1;
//     int index = (int)(intptr_t)user2;

//     uint32_t thread_id = sx_thread_tid();

//     // Create thread data
//     // note: thread index #0 is reserved for main thread
//     sx__job_thread_data *tdata = sx__job_create_tdata(ctx->alloc, thread_id, index + 1, false);
//     if (!tdata)
//     {
//         return -1;
//     }
//     sx_tls_set(ctx->thread_tls, tdata);

//     if (ctx->thread_init_cb)
//         ctx->thread_init_cb(ctx, index, thread_id, ctx->thread_user);

//     // Get first stack and run selector loop
//     fiber_t fiber = fiber_create(tdata->selector_stack, sx__job_selector_fn);
//     fiber_switch(fiber, ctx);

//     sx_tls_set(ctx->thread_tls, NULL);
//     sx__job_destroy_tdata(tdata, ctx->alloc);
//     if (ctx->thread_shutdown_cb)
//         ctx->thread_shutdown_cb(ctx, index, thread_id, ctx->thread_user);

//     return 0;
// }

// static sx__job_select_result sx__job_select(sx_job_context *ctx, uint32_t tid, uint32_t tags)
// {
//     sx__job_select_result r = {0};

//     sx_lock(ctx->job_lk)
//     {
//         for (int pr = 0; pr < SX_JOB_PRIORITY_COUNT; pr++)
//         {
//             sx__job *node = ctx->waiting_list[pr];
//             while (node)
//             {
//                 r.waiting_list_alive = true;
//                 if (*node->wait_counter == 0)
//                 { // job must not be waiting/depend on any jobs
//                     if ((node->owner_tid == 0 || node->owner_tid == tid) &&
//                         (node->tags == 0 || (node->tags & tags)))
//                     {
//                         r.job = node;
//                         sx__job_remove_list(&ctx->waiting_list[pr], &ctx->waiting_list_last[pr], node);
//                         pr = SX_JOB_PRIORITY_COUNT; // break out of the outer loop
//                         break;
//                     }
//                 }
//                 node = node->next;
//             } // while(iterate nodes)
//         }     // foreach(priority)
//     }         // lock

//     return r;
// }

// static void sx__job_selector_main_thrd(fiber_transfer transfer)
// {
//     sx_job_context *ctx = (sx_job_context *)transfer.userData;
//     sx__job_thread_data *tdata = (sx__job_thread_data *)TlsGetValue((DWORD)(uintptr_t)(ctx->thread_tls));

//     // Select the best job in the waiting list
//     sx__job_select_result r =
//         sx__job_select(ctx, tdata->tid, ctx->num_threads > 0 ? tdata->tags : 0xffffffff);

//     if (r.job)
//     {
//         // Job is a slave (in wait mode), get back to it and remove slave mode
//         if (r.job->owner_tid > 0)
//         {
//             r.job->owner_tid = 0;
//         }

//         // Run the job from beginning, or continue after 'wait'
//         tdata->selector_fiber = r.job->selector_fiber;
//         tdata->cur_job = r.job;
//         r.job->fiber = fiber_switch(r.job->fiber, r.job).from;

//         // Delete the job and decrement job counter if it's done
//         if (r.job->done)
//         {
//             tdata->cur_job = NULL;
//             sx_atomic_fetch_sub32(r.job->counter, 1);
//             sx__del_job(ctx, r.job);
//         }
//     }

//     // before returning, set selector to NULL, so we know that we have to recreate the fiber
//     tdata->selector_fiber = NULL;
//     fiber_switch(transfer.from, transfer.userData);
// }

// sx_job_context *sx_job_create_context(const sx_job_context_desc *desc)
// {
//     sx_job_context *ctx = (sx_job_context *)malloc(sizeof(sx_job_context));
//     memset(ctx, 0x0, sizeof(sx_job_context));

//     ctx->num_threads = desc->num_threads > 0 ? desc->num_threads : (sx_os_numcores() - 1);
//     ctx->thread_tls = (sx_tls)(uintptr_t)TlsAlloc();
//     ctx->stack_sz = desc->fiber_stack_sz > 0 ? desc->fiber_stack_sz : DEFAULT_FIBER_STACK_SIZE;
//     ctx->thread_init_cb = desc->thread_init_cb;
//     ctx->thread_shutdown_cb = desc->thread_shutdown_cb;
//     ctx->thread_user = desc->thread_user_data;
//     int max_fibers = desc->max_fibers > 0 ? desc->max_fibers : DEFAULT_MAX_FIBERS;

//     ctx->sem.handle = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);

//     sx__job_thread_data *main_tdata = sx__job_create_tdata(GetCurrentThreadId(), 0, true);

//     TlsSetValue((DWORD)(uintptr_t)ctx->thread_tls, main_tdata);

//     main_tdata->selector_fiber =
//         fiber_create(main_tdata->selector_stack, sx__job_selector_main_thrd);

//     // pools
//     ctx->job_pool = sx_pool_create(alloc, sizeof(sx__job), max_fibers);
//     ctx->counter_pool = sx_pool_create(alloc, sizeof(int), COUNTER_POOL_SIZE);
//     if (!ctx->job_pool || !ctx->counter_pool)
//         return NULL;
//     sx_memset(ctx->job_pool->pages->buff, 0x0, sizeof(sx__job) * max_fibers);

//     // keep tags in an array for evaluating num_jobs
//     ctx->tags = sx_malloc(alloc, sizeof(uint32_t) * ((size_t)ctx->num_threads + 1));
//     sx_memset(ctx->tags, 0xff, sizeof(uint32_t) * ((size_t)ctx->num_threads + 1));

//     // Worker threads
//     if (ctx->num_threads > 0)
//     {
//         ctx->threads = (sx_thread **)sx_malloc(alloc, sizeof(sx_thread *) * ctx->num_threads);
//         sx_assert(ctx->threads);

//         for (int i = 0; i < ctx->num_threads; i++)
//         {
//             char name[32];
//             sx_snprintf(name, sizeof(name), "sx_job_thread(%d)", i + 1);
//             ctx->threads[i] = sx_thread_create(alloc, sx__job_thread_fn, ctx, (int)sx_os_minstacksz(),
//                                                name, (void *)(intptr_t)i);
//         }
//     }

//     return ctx;
// }
