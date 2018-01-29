#include<iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdio.h>
#include"WebSocket.h"
using namespace std;
unsigned short PORT=5000;
WebSocket wskt;
int main(int argc,char*argv[])
{
	int _socket_fd=socket(AF_INET,SOCK_STREAM,0);
	if(_socket_fd==-1)
	{
		printf("socket create error[%d]%s\n",errno,strerror(errno));
		return false;
	}
	int optval=1;
	if(-1==setsockopt(_socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(optval)))
	{
		printf("setsockopt error[%d]%s\n",errno,strerror(errno));
		return false;
	}
	struct sockaddr_in local_addr;
	local_addr.sin_family=AF_INET;
	local_addr.sin_addr.s_addr=inet_addr("0.0.0.0\n");
	local_addr.sin_port=htons(PORT);
	if(-1==::bind(_socket_fd,(struct sockaddr*)&local_addr,sizeof(struct sockaddr)))
	{
		printf("bind error[%d]%s,ip:%s,port:%hu\n",errno,strerror(errno),local_addr,PORT);
		return false;
	}
	if(-1==::listen(_socket_fd,1))
	{
		printf("listen error[%d]%s\n",errno,strerror(errno));
		return false;
	}

	while(true)
	{
		struct sockaddr_in client_addr;
		socklen_t addrlen=sizeof(client_addr);
		int new_fd=accept(_socket_fd,(struct sockaddr*)&client_addr,&addrlen);
		if(new_fd==-1)
		{
			printf("accept error[%d]%s\n",errno,strerror(errno));
			if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)
			{
				printf("fatal error accept error\n");
			}
			return false;
		}
		char buf[2048];
		int len=0;
		bool hc=false;
        struct WebSocketInfo wskt_info;
		while(true)
		{
			int s=::recv(new_fd,buf,sizeof(buf)-len,0);
			if(s==-1)
			{
				printf("recv error[%d]%s\n",errno,strerror(errno));
				if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)
				{
					close(new_fd);
					break;
				}
				continue;
			}
			if(s==0)
			{
				printf("recv close req from server[%d]%s\n",errno,strerror(errno));
				close(new_fd);
				break;
			}
			len+=s;
			if(!hc)
			{
				printf("recv_handshake_data:%s\n",buf);
                int output_len=0;
				int r=WebSocket::parseHandshake((unsigned char*)buf,len, output_len, wskt_info);
				if(r==OPENING_FRAME)
				{
                    printf("handshake, input_len: %d, output_len: %d\n", len, output_len);
					std::string re=WebSocket::answerHandshake(wskt_info);
					int sl=0;
					while(sl<re.size())
					{
						int s;
						if((s=::send(new_fd,re.c_str()+sl,re.size()-sl,MSG_NOSIGNAL))==-1)
						{
							printf("send error[%d]%s\n",errno,strerror(errno));
							if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)
							{
								printf("close connection\n");
								close(new_fd);
								break;
							}
						}
						sl+=s;
					}
					hc=true;

                    // buf 减掉，并偏移
                    // buf 减掉，并偏移
                    memmove(buf, buf+output_len, len - output_len);
                    len -= output_len;
				}
			}
			else
			{
                int output_len;
                int output_offset;
				int r=WebSocket::getFrame((unsigned char*)buf,len, output_offset, output_len);
				if(r==INCOMPLETE_FRAME)
				{
					printf("incomplete data\n");
					continue;
				}
				else if(r==ERROR_FRAME)
				{
                    // 当client.py调用close的时候，会收到这个报错
                    printf("recv error frame data[%x]\n",r);
                    close(new_fd);
                    new_fd = -1;
                    break;
				}

				printf("server recv normal data[%x]:, input_len: %d, output_offset: %d, output_len: %d\n",r, len, output_offset, output_len);

                char wrapper_send_buf[2048];

				int rl=WebSocket::makeFrame(BINARY_FRAME,(unsigned char*)(buf + output_offset), output_len,(unsigned char*)wrapper_send_buf,sizeof(wrapper_send_buf));
				
				//send back and make it an echo server
				int sl=0;
				while(sl<rl)
				{
					int s;
					if((s=::send(new_fd,wrapper_send_buf+sl,rl-sl,MSG_NOSIGNAL))==-1)
					{
						printf("send error[%d]%s\n",errno,strerror(errno));
						if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)
						{
							printf("close connection\n");
							close(new_fd);
							break;
						}
					}
					sl+=s;
				}

                int total_len = output_offset + output_len;
                // buf 减掉，并偏移
                memmove(buf, buf+total_len, len - total_len);
                len -= total_len;
			}
		}

	}
	
	cout<<"hello world"<<endl;
	return 0;
}
