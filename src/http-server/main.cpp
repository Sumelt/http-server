#include "httpserver.h"

using namespace std;

int main( int argc, const char* argv[] )
{
    if( argc < 3 )
        cout << "./a.out port path, please input" << endl;
    else {
        int port = atoi( argv[ 1 ] );
		chdir( argv[2] );
        httpserver* http = new httpserver( static_cast<uint16_t>( port ) );
        http->http_run();
        
    }
    
    return 0;
}