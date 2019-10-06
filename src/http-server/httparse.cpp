/*
 * @Description: 解析请求类
 * @Author: melt
 * @Date: 2019-09-02 14:32:07
 * @LastEditTime: 2019-09-02 14:32:30
 */
#include "httparse.h"

httparse::httparse() {
    
}

void httparse::send_respond_head( int cfd, int no, const char *desp, const char *type, long len ) {
    char buff[ 2048 ] = { 0 };
    
    //发送状态行
        sprintf( buff, "http/1.1 %d %s\r\n", no, desp );
        send( cfd, buff, strlen( buff ), 0 );

    //发送响应头
        sprintf( buff, "Content-Type:%s\r\n", type );//必备 类型
        sprintf( buff + strlen( buff ), "Content-Length:%ld\r\n", len );
        send( cfd, buff, strlen( buff ), 0 );

    //发送空行
        send( cfd, "\r\n", strlen( "\r\n" ), 0 );
}

void httparse::send_file( int cfd, const char *filename ) {
    cot( filename );
    int filefd = open( filename, O_RDONLY );
    if( filefd == -1 ) {
        handle_error_exit( "http_parse open file faile!" );
    }
    
    struct stat fileAtrr;
    int ret = stat( filename, &fileAtrr );//检测文件大小
    if( ret == -1 ) {
        handle_error_exit( "http_server stat faile!" );
    }
    
    //零拷贝发送 
    while( sendfile( cfd, filefd, nullptr, static_cast<size_t>( fileAtrr.st_size ) ) );
  
}

//发送目录下的文件清单
void httparse::send_dir( int cfd, const char *dirname ) {
   
    //发送目录需要拼接HTML
    char buff[ 4096 ] = { 0 };
    
    sprintf( buff, "<html><head><title>目录：%s</title></head>", dirname );//HTML头部
    sprintf( buff + strlen( buff ), "<body><h1>当前目录：%s</h1><table>", dirname );
    

    char path[ 1024 ] = { 0 };
    char encodename[ 1024 ] = { 0 };//存储文件名编码后

	struct stat st;
    struct dirent** ptr;
    int num = scandir( dirname, &ptr, nullptr, alphasort );//递归搜索当前目录下文件API

    //搜索目录下所有文件(目录)，并加入HTML标记中
	for( int i = 0; i < num; ++i ) {
		char* name = ptr[ i ]->d_name;//获取文件名

		sprintf( path, "%s/%s", dirname, name );//目录/文件(目录)
		
		stat( path, &st );//检测类型
        //由于URL编码没有统一规范，同时避免浏览器参与编码带来麻烦
        //统一交由后台代码直接编码发送给前端浏览器
        //对中文的编码规则是UTF-8后使用百分号编码: %XX
        encode_str( encodename, sizeof( encodename ), name );//把中文文件名进行编码成%xx
        
        //拼接HTML
        //文件
        if( S_ISREG( st.st_mode ) ) {
            sprintf( buff + strlen( buff ), 
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", encodename, name, st.st_size);
            //href :超文本链接 encodename
            //name :文件名
            //size :文件大小

        }
        //目录
        else if ( S_ISDIR( st.st_mode ) ) {
            sprintf( buff + strlen( buff ), 
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", encodename, name, st.st_size);            
        }
        
        send( cfd, buff, strlen( buff ), 0 );
        memset( buff, 0, sizeof ( buff ) );
	}
    
    sprintf( buff, "</table></body></html>" );//HTML尾部
    send( cfd, buff, strlen( buff ), 0 );

}

//对HTTP请求资源路径进行解码
void httparse::http_parse_request( int fd, const char *line ) {
    //拆分请求方法 资源路径 协议版本
    char method[12], path[1024], protocol[12];//方法 资源路径 协议
    sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol);//匹配不是空格的部分
 
    
    //解码路径
    //对百分号编码进行解码，从而搜寻得到根目录下的文件
    decode_str( path, path );

    const char *file = "./"; //默认资源目录
    if( strcmp( path, "/" ) != 0 )//请求的资源不是根目录
        file = path + 1;
    
    //判断资源类型
    struct stat fileAttr;
    int ret = stat( file, &fileAttr );
    if( ret != 0 ) {
        //show 404 
        //handle_error_exit( "http_parse file type faile!" );
        send_respond_head( fd, 404, "file not found", ".html", -1 );//发送头部
        send_file( fd, "404.html" );//发送404网页
    }
        
    //普通文件
    if( S_ISREG( fileAttr.st_mode ) ) {
        send_respond_head( fd, 200, "OK", get_file_type( file ), fileAttr.st_size );//发送头部
        send_file( fd, file );//发送普通文件
    }
    //目录文件
    else if ( S_ISDIR( fileAttr.st_mode ) ) {
        send_respond_head( fd, 200, "OK", get_file_type( ".html" ), -1 );//发送头部
        //send_file( fd, "404.html" );
        send_dir( fd, file );//以HTML发送目录下的文件列表
        //cot( "oooook" );
    }
    
}
