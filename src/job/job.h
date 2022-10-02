// #include <cstdint>

// #define DEFAULT_FIBER_STACK_SIZE 1048576    // 1MB
// #define COUNTER_POOL_SIZE 256
// #define DEFAULT_MAX_FIBERS 64

// typedef struct sx_alloc sx_alloc;
// typedef struct sx_job_context sx_job_context;
// typedef uint32_t *sx_job_t;
// typedef void* sx_tls;

// typedef struct sx_job_context
// {
//     sx_thread **threads;
//     int num_threads;
//     int stack_sz;
//     sx_pool *job_pool;     // sx__job: not-growable !
//     sx_pool *counter_pool; // int: growable
//     sx__job *waiting_list[SX_JOB_PRIORITY_COUNT];
//     sx__job *waiting_list_last[SX_JOB_PRIORITY_COUNT];
//     uint32_t *tags; // count = num_threads + 1
//     sx_lock_t job_lk;
//     sx_lock_t counter_lk;
//     sx_tls thread_tls;
//     sx_atomic_uint32 dummy_counter;
//     sx_sem sem;
//     int quit;
//     sx_job_thread_init_cb *thread_init_cb;
//     sx_job_thread_shutdown_cb *thread_shutdown_cb;
//     void *thread_user;
//     sx__job_pending *pending;
// } sx_job_context;

// typedef void(sx_job_cb)(int range_start, int range_end, int thread_index, void *userData);
// typedef void(sx_job_thread_init_cb)(sx_job_context *ctx, int thread_index, unsigned int thread_id,
//                                     void *user);
// typedef void(sx_job_thread_shutdown_cb)(sx_job_context *ctx, int thread_index,
//                                         unsigned int thread_id, void *user);

// // Job priority, higher priority jobs will run sooner
// typedef enum sx_job_priority
// {
//     SX_JOB_PRIORITY_HIGH = 0,
//     SX_JOB_PRIORITY_NORMAL,
//     SX_JOB_PRIORITY_LOW,
//     SX_JOB_PRIORITY_COUNT
// } sx_job_priority;

// typedef struct sx_job_context_desc
// {
//     int num_threads;                               // number of worker threads to spawn,exclude main (default: num_cpu_cores-1)
//     int max_fibers;                                // maximum fibers that are can be running at the same time (default: 64)
//     int fiber_stack_sz;                            // fiber stack size (default: 1mb)
//     sx_job_thread_init_cb *thread_init_cb;         // callback function that will be called on
//                                                    // initiaslization of each worker thread
//     sx_job_thread_shutdown_cb *thread_shutdown_cb; // callback functions that will be called on
//                                                    // the shutdown of each worker thread
//     void *thread_user_data;                        // user-data to be passed to callback functions above
// } sx_job_context_desc;

// sx_job_context *sx_job_create_context(const sx_alloc *alloc,
//                                       const sx_job_context_desc *desc);
// void sx_job_destroy_context(sx_job_context *ctx, const sx_alloc *alloc);

// sx_job_t sx_job_dispatch(sx_job_context *ctx, int count, sx_job_cb *callback, void *user,
//                          sx_job_priority priority = SX_JOB_PRIORITY_NORMAL,
//                          unsigned int tags = 0);
// void sx_job_wait_and_del(sx_job_context *ctx, sx_job_t job);
// bool sx_job_test_and_del(sx_job_context *ctx, sx_job_t job);
// int sx_job_num_worker_threads(sx_job_context *ctx);
// void sx_job_set_current_thread_tags(sx_job_context *ctx, unsigned int tags);

// int sx_job_thread_index(sx_job_context *ctx);
// unsigned int sx_job_thread_id(sx_job_context *ctx);