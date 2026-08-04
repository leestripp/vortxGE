// OmniNet integration smoke test.
//
// Proves the statically-linked OmniNet library (FetchContent:
// gitlab.com/leestripp/omninet) can serve a real HTTP request from
// inside the vortxGE test process — the foundation for engine-level
// remote inspection / MCP-over-HTTP debug hooks.

#include "doctest/doctest.h"

#include "OmniNet.h"
#include "http/HttpServer.h"

#include <chrono>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace OmniNet;
using namespace OmniNet::HTTP;

namespace
{
	// Minimal blocking HTTP/1.0 GET — keeps the test free of curl.
	std::string httpGet( uint16_t port, const std::string& path )
	{
		int fd = socket( AF_INET, SOCK_STREAM, 0 );
		if( fd < 0 ) return {};

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_port = htons( port );
		inet_pton( AF_INET, "127.0.0.1", &addr.sin_addr );
		if( connect( fd, (sockaddr*)&addr, sizeof(addr) ) != 0 )
		{
			close( fd );
			return {};
		}

		const std::string req =
			"GET " + path + " HTTP/1.0\r\nHost: 127.0.0.1\r\n\r\n";
		send( fd, req.data(), req.size(), 0 );

		std::string out;
		char buf[4096];
		ssize_t n;
		while( ( n = recv( fd, buf, sizeof(buf), 0 ) ) > 0 )
			out.append( buf, (size_t)n );
		close( fd );
		return out;
	}
}

TEST_CASE( "OmniNet: static link + HTTP round-trip" )
{
	OmniNet::init();

	Server::Config cfg;
	cfg.address = "127.0.0.1";
	cfg.port = 18090;
	cfg.workerCount = 1;

	Server server( cfg );
	server.router().get( "/api/vortxge",
						 []( const Request& req, Response& resp )
						 {
							 (void)req;
							 resp.setStatus( StatusCode::OK );
							 resp.setContentType( "application/json" );
							 resp.setBody(
								 R"({"engine":"vortxGE","net":"OmniNet"})" );
						 } );

	server.startAsync();

	// retry until the listener is up (max ~2 s)
	std::string body;
	for( int i = 0; i < 20 && body.empty(); i++ )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		body = httpGet( 18090, "/api/vortxge" );
	}

	server.stop();

	REQUIRE( !body.empty() );
	CHECK( body.find( "200" ) != std::string::npos );
	CHECK( body.find( "\"engine\":\"vortxGE\"" ) != std::string::npos );
}
