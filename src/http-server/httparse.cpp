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
    int ret = stat( filename, &fileAtrr );
    if( ret == -1 ) {
        handle_error_exit( "http_server stat faile!" );
    }
    
    //零拷贝发送 
    while( sendfile( cfd, filefd, nullptr, static_cast<size_t>( fileAtrr.st_size ) ) );
  
}

void httparse::send_dir( int cfd, const char *dirname ) {
   
    
    //发送目录需要拼接HTML
    char buff[ 4096 ] = { 0 };
    
    sprintf( buff, "<html><head><title>目录：%s</title></head>", dirname );
    sprintf( buff + strlen( buff ), "<body><h1>当前目录：%s</h1><table>", dirname );
    
    //搜索目录文件
    char path[ 1024 ] = { 0 };
    char encodename[ 1024 ] = { 0 };

	struct stat st;
    struct dirent** ptr;
	int num = scandir( dirname, &ptr, nullptr, alphasort );
    
	for( int i = 0; i < num; ++i ) {
		char* name = ptr[ i ]->d_name;//获取文件名

		sprintf( path, "%s/%s", dirname, name );//目录/文件(目录)
		
		stat( path, &st );//检测类型
		encode_str( encodename, sizeof( encodename ), name );//把文件名进行编码
        
        //拼接HTML
        //文件
        if( S_ISREG( st.st_mode ) ) {
            sprintf( buff + strlen( buff ), 
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", encodename, name, st.st_size);
        }
        //目录
        else if ( S_ISDIR( st.st_mode ) ) {
            sprintf( buff + strlen( buff ), 
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", encodename, name, st.st_size);            
        }
        
        send( cfd, buff, strlen( buff ), 0 );
        memset( buff, 0, sizeof ( buff ) );
	}
    
    sprintf( buff, "</table></body></html>" );
    send( cfd, buff, strlen( buff ), 0 );
    

}

void httparse::http_parse_request( int fd, const char *line ) {
    //拆分请求方法 资源路径 协议版本
    char method[12], path[1024], protocol[12];//方法 资源路径 协议
    sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol);//匹配不是空格的部分
 
    
    //解码路径
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
        send_respond_head( fd, 404, "file not found", ".html", -1 );
        send_file( fd, "404.html" );
    }
        
    //普通文件
    if( S_ISREG( fileAttr.st_mode ) ) {
        send_respond_head( fd, 200, "OK", get_file_type( file ), fileAttr.st_size );
        send_file( fd, file );
    }
    //目录文件
    else if ( S_ISDIR( fileAttr.st_mode ) ) {
        send_respond_head( fd, 200, "OK", get_file_type( ".html" ), -1 );
        //send_file( fd, "404.html" );
        send_dir( fd, file );
        //cot( "oooook" );
    }
    
}
