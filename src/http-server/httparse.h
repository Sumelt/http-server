#ifndef HTTPARSE_H
#define HTTPARSE_H

#include "httpserver.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dirent.h>

class httparse
{
private:
    // 发送响应头
    //套接字 状态码 原因短语 文件类型 数据长度
    void send_respond_head(int cfd, int no, const char* desp, const char* type, long len);
    // 发送文件
    void send_file(int cfd, const char* filename);
    // 发送目录内容
    void send_dir(int cfd, const char* dirname);  
public:
    httparse();
    ~httparse();

    //解析
    void http_parse_request( int fd, const char* line );
};


const char *get_file_type(const char *name);//获取文件类型
void encode_str(char* to, int tosize, const char* from);//字符编码
void decode_str(char *to, char *from);//字符解码
int get_line(int sock, char *buf, int size);//解析一行

#endif // HTTPARSE_H
