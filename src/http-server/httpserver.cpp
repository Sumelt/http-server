#include "httpserver.h"
#include "httparse.h"
//设置不阻塞
int setnoblock( int &fd ) {
    int oldOpt = fcntl( fd, F_GETFL );
    fcntl( fd, F_SETFL, oldOpt | O_NONBLOCK );
    return oldOpt;
}

inline void cot( const char* str, bool opt ) {
    opt ? ( cout << str << endl ) :( cout << str << flush );
}

httpserver::httpserver( uint16_t port ) {
    onlineSum = 0;
    memset( &servaddr, 0, sizeof ( servaddr ) );
    servfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( servfd == -1 )
        handle_error_exit( "http_server socket faile!" );
    cot( "http_server socket OK!" );
    
    int opt = 1;
    setsockopt( servfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) );//端口复用
    http_bind( port );
    
    pasre = new httparse ();//创建解析对象
    cot( "The server is ready to wait for the client to connect......." );
}

httpserver::~httpserver() {
    close( servfd );
}

void httpserver::http_bind( uint16_t port ) {
    int ret;
    servlen = sizeof ( servaddr );
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( port );
    servaddr.sin_addr.s_addr = htonl( INADDR_ANY );//绑定本机地址

    ret = bind( servfd, reinterpret_cast<sockaddr*>( &servaddr ), servlen );
    if( ret == -1 )
        handle_error_exit( "http_bind faile!" );
    cot( "http_server bind OK!" );
    
    http_listen();
}

void httpserver::http_listen() {
    int ret = listen( this->servfd, 10 );
    if( ret ) 
        handle_error_exit( "http_listen faile!" );
    cot( "http_server listen OK!" );
}

void httpserver::http_disconnect( int fd ) {
    close( fd );
    setEpollEvent( fd, EPOLL_CTL_DEL, EPOLLIN );
    cot( "http_server Notice: a client left!" );
}

void httpserver::http_accept(void *) {
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof( cliaddr );
    
    int connfd = accept( servfd, reinterpret_cast<sockaddr*>( &cliaddr ), &clilen );
    if( connfd == -1 )
        handle_error_exit( "http_server Notice: accpet a new client faile! " );
    
    ++onlineSum;
    string IP = string( inet_ntoa( cliaddr.sin_addr ) );
    cout << "http_server accpet new Client: " << IP << endl;
    
    setnoblock( connfd ); //该套接字设置非阻塞
    setEpollEvent( connfd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET ); //开启ET模式
    
}

void httpserver::setEpollEvent(int fd, int op, uint32_t status) {
    ep_ctl.data.fd = fd;
    ep_ctl.events = status;
    epoll_ctl( ep_fd, op, fd, &this->ep_ctl );
}

void httpserver::do_read( int fd ) {
    char line[ 1024 ] = { 0 };
    bool keeplive = false;
    int len = get_line( fd, line, sizeof ( line ) );//解析请求行
    if( len == 0 ) {
        http_disconnect( fd );
        //exit( EXIT_FAILURE );
    }
    //截取请求头
    else {
        cot( "============= Request line: =============", true );
        cot( line );
        
        cot( "============= Request head: =============", true );
        char buff[ 4096 ] = { 0 };
        while ( len > 0 ) {
            len = get_line( fd, buff, sizeof ( buff ) );
            if( strncmp( "Connection: keep-alive", buff, 22 ) == 0 )//检测长连接
                keeplive = true;
            cot( buff, false );
            memset( buff, 0, sizeof ( buff ) );
        }
        cot( "============= The End ===================" );
    }

    //解析请求头
    if( strncasecmp( "GET", line, 3 ) == 0 ) {
        this->pasre->http_parse_request( fd, line );//组合设计模式
        if( !keeplive ) //保持长连接
            http_disconnect( fd );
    }
}

void httpserver::http_epoll() {
    ep_fd =  epoll_create( MAXUSERS );
    struct epoll_event ep_wait[ MAXUSERS + 1 ];//就绪队列
    //服务器事件加入
    setEpollEvent( servfd, EPOLL_CTL_ADD, EPOLLIN );
    
    while ( true ) {
        int cnt = epoll_wait( ep_fd, ep_wait, MAXUSERS + 1, -1 ); //堵塞
        if( cnt < 0 && errno != EINTR ) {
            perror( "epoll Faile" );
            break;
        }        
        for ( int i = 0; i < cnt ; ++i ) {
            //只监听新连接
            if( this->servfd == ep_wait[ i ].data.fd && ep_wait[ i ].events == EPOLLIN )
                http_accept( nullptr );
            //解析HTTP报文
            else if ( ep_wait[ i ].events == EPOLLIN ) {
                do_read( ep_wait[ i ].data.fd );
            }
            else {
                continue;
            }
        }
    }     
}

//用户调用启动函数
void httpserver::http_run() {
    http_epoll();
}
