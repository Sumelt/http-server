#include "httparse.h"
// 通过文件名获取文件的类型
const char *get_file_type(const char *name)
{
    const char* dot;

    dot = strrchr(name, '.');   // 自右向左查找‘.’字符, 如不存在返回NULL
    if (dot == nullptr)
        return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp( dot, ".wav" ) == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";//未知文件直接返回文本格式
}

// 16进制数转化为10进制
int hexit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

/*
 *  这里的内容是处理%20之类的东西！是"解码"过程。
 *  %20 URL编码中的‘ ’(space)
 *  %21 '!' %22 '"' %23 '#' %24 '$'
 *  %25 '%' %26 '&' %27 ''' %28 '('......
 *  相关知识html中的‘ ’(space)是&nbsp
 */
//编码--中文变为16进制数
//to存储的位置，tosize 存储位置大小 from
void encode_str(char* to, int tosize, const char* from)
{
    int tolen;

    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) //tolen + 4是因为最后的结束符也需要存储空间
    {
        if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) //数字和字母、以及一些特殊字符不需要编码
        {
            *to = *from;
            ++to;
            ++tolen;
        } 
        else 
        {
            sprintf(to, "%%%02x", (int) *from & 0xff);// %xx 字符先强转整型后再与操作则变为16进制数
            to += 3;
            tolen += 3;
        }

    }
    *to = '\0';
}

//解码操作
void decode_str(char *to, char *from)
{
    for ( ; *from != '\0'; ++to, ++from  ) 
    {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) //%xx 如果为% 或者为16进制数
        { 

            *to = hexit(from[1])*16 + hexit(from[2]);//转成十进制数，是ascii

            from += 2;//因为++from，所以这里移动了2位                      
        } 
        else
        {
            *to = *from;

        }

    }
    *to = '\0';

}


// 解析http请求消息的每一行内容
int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    ssize_t n = 0;
    while ((i < size - 1) && (c != '\n'))//读完指定的字节数或者读到回车跳出
    {
        n = recv(sock, &c, 1, 0);//读一个字符
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                {
                    recv(sock, &c, 1, 0);//完整读到一行
                }
                else
                {
                    c = '\n';//跳出
                }
            }
            buf[i] = c;//缓冲区写入每一个字符
            i++;
        }
        else
        {
            c = '\n';//跳出
        }
    }
    
    buf[i] = '\0';
    if( n == -1 )
        i = -1;
    return i; //返回最后写入的下标
}
