#ifndef HTTPSERVER_H
#define HTTPSERVER_H
//----- CPP headfile
#include <iostream>

//----- C headfile
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

const int MAXUSERS = 10; //最大数量的用户
class httparse;
using namespace std;

//宏函数--打印错误消息

#define handle_error_exit( msg ) \
    do { \
        perror(msg); exit(EXIT_FAILURE); \
    } while (0)

#define handle_error_en( msg ) \
    do { \
        perror(msg); \
    } while (0)

class httpserver
{
private:
    struct sockaddr_in servaddr;//服务器信息
    struct epoll_event ep_ctl;//epoll event结构体
    socklen_t servlen;
    int servfd, ep_fd;
    int onlineSum;//在线人数

    void http_accept( void* );    
    void http_bind( uint16_t port );
    void http_listen();
    void http_epoll();
    void http_disconnect( int fd );
    
    void setEpollEvent( int fd, int op, uint32_t status );//epoll事件
    void do_read( int fd );//读事件
    
public:
    httparse *pasre;
    
    httpserver( uint16_t port );
    ~httpserver();
    void http_run();
};


int setnoblock( int &fd );
inline void cot( const char* str, bool opt = true );

#endif // HTTPSERVER_H
