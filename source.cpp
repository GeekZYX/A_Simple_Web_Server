#include "winsock2.h"
#include "fstream"
#include "iostream"
#include "String"
#include "thread"
#define BUF_SIZE 1024
#define PROPERTY_SIZE 128
#define DEFULT_PORT 6156
//using namespace std;
void SimpleServer(SOCKET LisnSock,std::string path);
int main(int argc,char* argv[])
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2),&wsadata)!=0)
	{
		std::cout << "startup error!";
		exit(1);
	}
	SOCKET ServSock, ListenSock;
	ServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (ServSock==INVALID_SOCKET)
	{
		std::cout << "socket error";
		exit(1);
	}
	SOCKADDR_IN servAdr,listenAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(DEFULT_PORT);
	if (bind(ServSock,(SOCKADDR*) &servAdr,sizeof(servAdr))==SOCKET_ERROR)
	{
		std::cout << "bind error";
		exit(1);
	}
	if (listen(ServSock,15)==SOCKET_ERROR)
	{
		std::cout << "listen error";
		exit(1);
	}

	std::cout << "Listening...\n" << std::endl;

	while (1)
	{	
		int listenAdrSize = sizeof(listenAdr);
		ListenSock = accept(ServSock, (SOCKADDR*)&listenAdr, &listenAdrSize);
		if (ListenSock == INVALID_SOCKET)
		{
			std::cout << "accept error";
			break;
		}
		std::thread httpthread(SimpleServer,ListenSock,argv[0]);
		httpthread.detach();
	}
	closesocket(ServSock);
	WSACleanup();
	return 0;
}
void SimpleServer(SOCKET LisnSock, std::string path)
{
	char message[BUF_SIZE];
	std::string method;
	std::string url;
	std::string version;
	//std::cout << sizeof(message);
	memset(message, 0, sizeof(message));

	if (recv(LisnSock, message, sizeof(message), 0) == SOCKET_ERROR)
	{
		std::cout << WSAGetLastError();
		exit(1);
	}
	else
		std::cout << message;
	std::string temp;
	temp = message;
	//std::cout << temp.size();
	int finalidx[1024] = { 0 };
	int index[1024] = { 0 };
	int j = 0;
	for (int i = 0;;i++,j++)
	{
		i = temp.find(' ', i);
		if (i == std::string::npos)
			break;
		index[j] = i+1;
		//std::cout << i;
	}
	int half = j;
	for (int i = 0;; i++, j++)
	{
		i = temp.find("\r\n", i);
		if (i == std::string::npos)
			break;
		index[j] = i + 1;
		//std::cout << i;
	}
	int idxsize = j;
	//std::cout << index[half]<<" ";
	int i = 0, k = half, l = 0;
	do 
	{
		if (index[i]<index[k])
		{
			finalidx[l] = index[i];
			++i;
		}
		else
		{
			finalidx[l] = index[k];
			++k;
		}
		++l;
		if (i>=half||k>idxsize)
		{
			if (i>=half)
			{
				while (k<idxsize)
				{
					finalidx[l] = index[k];
					++l;
					++k;
				}
			}
			else
			{
				while (i<half)
				{
					finalidx[l] = index[i];
					++l;
					++i;
				}
			}
		}
	} while (l < idxsize);
	/*for (int e:finalidx)
	{
		std::cout << e << " ";
	}*/
	method = temp.substr(0,finalidx[0]-0-1);
	url = temp.substr(finalidx[0],finalidx[1]-finalidx[0]-1);
	version = temp.substr(finalidx[1], finalidx[2] - finalidx[1] - 1);
	std::cout << "Method:"<< method <<std::endl
		<<"Url:" << url <<std::endl
		<<"Http Version:"<< version<<std::endl;
	if (method=="GET")
	{
		path.erase(path.size()-14) += url.replace(0,1,"\\");
		std::fstream file(path,std::fstream::in|std::fstream::binary);
		if (file)
		{
			std::cout << "File Found!" << std::endl
				<< "Path:" << path << std::endl;
			file.seekg(0, std::fstream::end);
			int length = file.tellg();
			file.seekg(0, std::fstream::beg);
			std::cout << "Length:" << length << std::endl;
			std::string send_buf = "HTTP/1.1 200 OK\r\n\
							Connection: keep-alive\r\n\
							Content-Type: image/jpeg\r\n\
							\r\n";
			const char *buf = send_buf.c_str();
			send(LisnSock, buf, send_buf.size(), 0);
			char content[BUF_SIZE];
			while (!file.eof())
			{
				memset(content, 0, BUF_SIZE);
				file.read(content, BUF_SIZE);
				send(LisnSock, content, BUF_SIZE, 0);
			}
			if (file.eof())
			{
				std::cout << "Send Compelete!"<<std::endl;
			}
			else
			{
				std::cout << "Send Faild..." << std::endl;
			}
			file.close();
		}
		else
		{
			std::cout << "File Not Found!"<<std::endl;
			std::string send_buf = "HTTP/1.1 404 NOT FOUND\r\n\
				Connection: keep-alive\r\n\
				Content-Type: text/html\r\n\
				\r\n\
				<HTML><TITLE>Not Found</TITLE>\r\n\
				<BODY><h1 align='center'>404</h1><br/><h1 align='center'>File Not Found.</h1>\r\n\
				</BODY></HTML>\r\n";
			const char *buf = send_buf.c_str();
			send(LisnSock, buf, send_buf.size(), 0);
			//std::cout << send_buf.size();
			//std::cout << buf;
		}
		//std::cout << "Path:" << path << std::endl;
		
	}

	closesocket(LisnSock);
	std::cout << "Closed well.\n\n\n" << std::endl;
}
