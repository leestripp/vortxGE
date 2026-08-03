/**
 * @file lsAsyncFileLoader.h
 * @brief Worker-thread async file loader (Plan.md Phase 7, M1)
 *
 * First building block of the dynamic streaming Asset Manager:
 * file I/O happens on worker threads ("Stage A" of the two-stage
 * pipeline — CPU-side load/parse), while GPU upload stays on the
 * main thread ("Stage B", M2+).
 *
 * Results are returned as std::expected (C++23) — the error-handling
 * model for all new asset code (Phase 6 step 5).
 *
 * Destruction DRAINS the queue: all queued tasks complete before
 * the loader joins its threads.
 *
 * @see Plan.md Phase 7 M1
 */
#ifndef LSASYNCFILELOADER_H
#define LSASYNCFILELOADER_H

#include <cstddef>
#include <expected>
#include <future>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <type_traits>

class lsAsyncFileLoader
{
public:
    /// File contents on success, error message on failure.
    using LoadResult = std::expected<std::vector<std::byte>, std::string>;

    /** Create a loader with `num_threads` worker threads (>= 1). */
    lsAsyncFileLoader( unsigned int num_threads = 1 );
    /** Drain queued tasks, stop workers, join threads. */
    ~lsAsyncFileLoader();

    lsAsyncFileLoader( const lsAsyncFileLoader& ) = delete;
    lsAsyncFileLoader& operator=( const lsAsyncFileLoader& ) = delete;

    /** Queue an async load; the future yields the LoadResult. */
    std::future<LoadResult> Load( const std::string& path );

    /** Synchronous load (same code path the workers use). */
    static LoadResult LoadFile( const std::string& path );

    /** Queue an arbitrary task on the worker pool (e.g. load + parse);
     *  the future yields the task's return value. */
    template<typename F>
    auto Queue( F&& f ) -> std::future<std::invoke_result_t<F>>
    {
        using R = std::invoke_result_t<F>;
        auto task = std::make_shared<std::packaged_task<R()>>( std::forward<F>( f ) );
        std::future<R> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            m_tasks.push( [task]() { (*task)(); } );
        }
        m_cv.notify_one();
        return fut;
    }

private:
    void WorkerLoop();

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stopping = false;
};

#endif // LSASYNCFILELOADER_H
