#include <exception>
#include <iostream>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace
{
struct DevNode
{
    DevNode() = delete;

    DevNode( const char * path )
    {
        if ( ! path )
        {
            throw std::invalid_argument { "invalid device node path (null)" };
        }

        fd_ = open( path, O_RDONLY );
        if ( fd_ == -1 )
        {
            throw std::runtime_error {
                    std::string( "could not open device at " ) + path +
                    std::string( ", error: " ) + strerror( errno )
            };
        }
    }

    ~DevNode()
    {
        std::cout << "---closing \n";
        ::close( fd_ );
    }

    operator int()
    {
        return fd_;
    }

private:
    int fd_;
};
} // namespace

int main( int argc, char **argv )
{
    using namespace std;

    try
    {

        if ( argc <= 1 )
        {
            std::cout << "usage: " << argv[0] << " path_to_input_event_device_node" << std::endl;
            return 1;
        }

        const char *dev = argv[1];

        std::cout << "---opening " << dev << "\n";

        DevNode fd( dev );

        std::cout << "---waiting for keypress\n";
        while ( true )
        {
            struct input_event ev;

            auto n = read( fd, &ev, sizeof( ev ) );
            (void) n;

            if ( ev.type == EV_KEY )
            {
                printf( "event timestamp: %ld.%06ld code: %d, value: %d \n",
                        ev.time.tv_sec,
                        ev.time.tv_usec,
                        ev.code,
                        ev.value );

                if ( KEY_ESC == ev.code )
                {
                    break;
                }
            }
            else if ( EV_SYN == ev.type )
            {
                printf( "event timestamp: %ld.%06ld, SYN \n",
                        ev.time.tv_sec,
                        ev.time.tv_usec );
            }
            else
            {
                printf( "event timestamp: %ld.%06ld, OTHER: %d \n",
                        ev.time.tv_sec,
                        ev.time.tv_usec,
                        ev.type );
            }
        }
        fflush( stdout );

        return 0;
    }
    catch ( exception& e )
    {
        cerr << "error: " << e.what() << endl;
    }
    catch ( ... )
    {
        cerr << "unknown exception caught" << endl;
    }

    return 1;
}
