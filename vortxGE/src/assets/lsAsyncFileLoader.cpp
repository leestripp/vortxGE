/**
 * @file lsAsyncFileLoader.cpp
 * @brief Worker-thread async file loader (Plan.md Phase 7, M1)
 */
#include "lsAsyncFileLoader.h"

#include <fstream>

lsAsyncFileLoader::lsAsyncFileLoader( unsigned int num_threads )
{
    if( num_threads == 0 ) num_threads = 1;
    m_workers.reserve( num_threads );
    for( unsigned int i = 0; i < num_threads; ++i )
        m_workers.emplace_back( &lsAsyncFileLoader::WorkerLoop, this );
}

lsAsyncFileLoader::~lsAsyncFileLoader()
{
    // Drain semantics: workers keep going until the queue is empty
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        m_stopping = true;
    }
    m_cv.notify_all();
    for( auto& w : m_workers )
        w.join();
}

std::future<lsAsyncFileLoader::LoadResult> lsAsyncFileLoader::Load( const std::string& path )
{
    auto task = std::make_shared<std::packaged_task<LoadResult()>>(
        [path]() { return LoadFile( path ); } );
    std::future<LoadResult> fut = task->get_future();
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        m_tasks.emplace( [task]() { (*task)(); } );
    }
    m_cv.notify_one();
    return fut;
}

lsAsyncFileLoader::LoadResult lsAsyncFileLoader::LoadFile( const std::string& path )
{
    std::ifstream file( path, std::ios::binary | std::ios::ate );
    if( !file.is_open() )
        return std::unexpected( "lsAsyncFileLoader: cannot open '" + path + "'" );

    const std::streamsize size = file.tellg();
    if( size < 0 )
        return std::unexpected( "lsAsyncFileLoader: cannot determine size of '" + path + "'" );
    file.seekg( 0, std::ios::beg );

    std::vector<std::byte> data( static_cast<size_t>( size ) );
    if( size > 0 && !file.read( reinterpret_cast<char*>( data.data() ), size ) )
        return std::unexpected( "lsAsyncFileLoader: read failed for '" + path + "'" );

    return data;
}

void lsAsyncFileLoader::WorkerLoop()
{
    for( ;; )
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock( m_mutex );
            m_cv.wait( lock, [&] { return m_stopping || !m_tasks.empty(); } );
            if( m_stopping && m_tasks.empty() )
                return;                 // drained — exit
            task = std::move( m_tasks.front() );
            m_tasks.pop();
        }
        task();                         // result delivered via packaged_task future
    }
}
