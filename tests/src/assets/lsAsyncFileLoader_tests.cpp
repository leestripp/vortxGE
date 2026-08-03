// Tests for vortxGE/src/assets/lsAsyncFileLoader.{h,cpp} (Plan.md Phase 7 M1)
// Pure CPU — no GL context needed.

#include "doctest/doctest.h"

#include "assets/lsAsyncFileLoader.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using doctest::Approx;

namespace {

// RAII temp file for tests
struct TempFile
{
    std::filesystem::path path;
    TempFile( const std::string& name, const std::string& contents )
    {
        path = std::filesystem::temp_directory_path() / name;
        std::ofstream f( path, std::ios::binary );
        f << contents;
    }
    ~TempFile() { std::filesystem::remove( path ); }
};

std::string as_string( const std::vector<std::byte>& data )
{
    return std::string( reinterpret_cast<const char*>( data.data() ), data.size() );
}

} // namespace

TEST_CASE( "lsAsyncFileLoader: LoadFile (sync) success and failure" )
{
    SUBCASE( "existing file returns its bytes" )
    {
        TempFile tf( "vortxge_test_sync.bin", "hello vortxGE" );
        auto result = lsAsyncFileLoader::LoadFile( tf.path.string() );
        REQUIRE( result.has_value() );
        CHECK( as_string( *result ) == "hello vortxGE" );
    }

    SUBCASE( "missing file returns an error, not a crash" )
    {
        auto result = lsAsyncFileLoader::LoadFile( "/nonexistent/path/nope.bin" );
        CHECK_FALSE( result.has_value() );
        CHECK( result.error().find( "nope.bin" ) != std::string::npos );
    }

    SUBCASE( "empty file succeeds with zero bytes" )
    {
        TempFile tf( "vortxge_test_empty.bin", "" );
        auto result = lsAsyncFileLoader::LoadFile( tf.path.string() );
        REQUIRE( result.has_value() );
        CHECK( result->empty() );
    }
}

TEST_CASE( "lsAsyncFileLoader: async Load delivers via future" )
{
    TempFile tf( "vortxge_test_async.bin", "async-content-123" );
    lsAsyncFileLoader loader( 1 );

    auto fut = loader.Load( tf.path.string() );
    auto result = fut.get();
    REQUIRE( result.has_value() );
    CHECK( as_string( *result ) == "async-content-123" );
}

TEST_CASE( "lsAsyncFileLoader: many concurrent loads on a small pool" )
{
    constexpr int N = 8;
    std::vector<std::unique_ptr<TempFile>> files;
    std::vector<std::future<lsAsyncFileLoader::LoadResult>> futs;

    for( int i = 0; i < N; ++i )
        files.push_back( std::make_unique<TempFile>(
            "vortxge_test_pool_" + std::to_string( i ) + ".bin",
            "content-" + std::to_string( i ) ) );

    {
        lsAsyncFileLoader loader( 2 );   // fewer threads than tasks
        for( int i = 0; i < N; ++i )
            futs.push_back( loader.Load( files[i]->path.string() ) );

        for( int i = 0; i < N; ++i )
        {
            auto result = futs[i].get();
            REQUIRE( result.has_value() );
            CHECK( as_string( *result ) == "content-" + std::to_string( i ) );
        }
    }   // loader destroyed here — must have drained cleanly
    CHECK( true );                      // reached here without hang
}

TEST_CASE( "lsAsyncFileLoader: missing file error propagates through future" )
{
    lsAsyncFileLoader loader( 1 );
    auto fut = loader.Load( "/nonexistent/async-nope.bin" );
    auto result = fut.get();
    CHECK_FALSE( result.has_value() );
    CHECK( result.error().find( "async-nope.bin" ) != std::string::npos );
}
