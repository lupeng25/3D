#include "ComSocket.h"
#include <process.h>

#include "iostream"
#pragma comment(lib,"ws2_32.lib")
#include <Windows.h>

#pragma warning(disable : 4996)
#pragma warning(disable : 4995)
#define Const_BufLength 1024
#define MAX_THREAD_LEGTH 64
#define Const_Port 12345

ComSocket::ComSocket(QObject *parent)
	: QTcpSocket(parent)
{
	m_bIsConnected = false;
	connect(this, SIGNAL(connected()), this, SLOT(ConnectSocket()));
	connect(this, SIGNAL(disconnected()), this, SLOT(DisConnectSocket()));

}

ComSocket::~ComSocket()
{
	disconnected();
	close();
	m_bIsConnected = false;
}

void ComSocket::ConnectSocket()
{
	m_bIsConnected = true;
	emit sig_ConnectedSocket(m_strKey,this);
}

void ComSocket::DisConnectSocket()
{
	m_bIsConnected = false;
	emit sig_DisConnectedSocket(m_strKey,this);
}

void ComSocket::Write(QByteArray ba)
{
	this->write(ba);
}

MyServer::MyServer(QObject *parent)
	: QTcpServer(parent)
{
	ResetConnect();
}

MyServer::~MyServer()
{    
	//释放所有socket
	while (list_information.count() > 0)
	{
		list_information[0].mysocket->disconnect(this);
		list_information[0].mysocket->disconnected();
		list_information[0].mysocket->close();
		emit list_information[0].mysocket->deleteLater();
		list_information.removeAt(0);
	}
}

void MyServer::ResetConnect()
{
	connect(this, SIGNAL(newConnection()), this, SLOT(serviceClientConnected()));
	list_information.clear();
}

void MyServer::serviceClientConnected()
{
	ComSocket *socket = (ComSocket*)nextPendingConnection();
	socket->disconnect(this);
	SocketInformation item;
	item.mysocket	= socket;
	item.strIP		= socket->peerAddress().toString();
	item.strPort	= socket->peerPort();
	list_information.push_back(item);
	connect(socket, &QTcpSocket::connected, this, [=]() {	socket->readAll();	});
	connect(socket, &QTcpSocket::readyRead, this, [=]() {	emit sig_ReadyReadConnectedSocket(socket);	});
	connect(socket, &QTcpSocket::disconnected,	this, [=]() {
		emit sig_DisConnectedSocket(socket);
		for (size_t i = 0; i < list_information.size(); i++)
		{
			if (socket == list_information[i].mysocket)
			{
				list_information.removeAt(i);
				return;		
			}	
		}	});
	emit sig_ConnectedSocket(socket);
}


MyQSerialPort::MyQSerialPort(QObject *parent)
	: QSerialPort(parent)
{
}

MyQSerialPort::~MyQSerialPort()
{
}

void MyQSerialPort::Write(QByteArray ba)
{
	this->write(ba);
}

#pragma region 通用
//从已经准备好的socket中读取数据。
//sockfd：已经准备好的socket连接。
//buffer：接收数据缓冲区的地址。
//n：本次接收数据的字节数。
//返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
bool recvn(const int sockfd, char *buffer, const size_t n) {
	int remain, len, idx;
	remain = n;
	idx = 0;
	while (remain > 0) {
		if ((len = ::recv(sockfd, buffer + idx, remain, 0)) <= 0)
			return false;
		idx += len;
		remain -= len;
	}
	return true;
}

//向已经准备好的socket中写入数据。
//sockfd：已经准备好的socket连接。
//buffer：待发送数据缓冲区的地址。
//n：待发送数据的字节数，缺省值是0时默认为buffer的大小。
//返回值：成功发送完n字节的数据后返回true，socket连接不可用返回false。
bool sendn(const int sockfd, const char *buffer, const size_t n = 0) {
	int remain, len, idx;
	remain = (n == 0) ? strlen(buffer) : n;
	idx = 0;
	while (remain > 0) {
		if ((len = send(sockfd, buffer + idx, remain, 0)) <= 0)
			return false;
		idx += len;
		remain -= len;
	}
	return true;
}

//接收socket的对端发送过来的数据。
//sockfd：可用的socket连接。
//buffer：接收数据缓冲区的地址。
//ibuflen：本次成功接收数据的字节数。解决TCP网络传输「粘包」
//itimeout：接收等待超时的时间，单位：秒，缺省值是0-无限等待。
//返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
bool TcpRecv(const int sockfd, char *buffer, int *ibuflen, const int itimeout = 0) {
	if (sockfd == -1) return false;
	if (itimeout > 0) {  //延时操作 
		fd_set tmpfd;

		FD_ZERO(&tmpfd);
		FD_SET(sockfd, &tmpfd);

		struct timeval timeout;
		timeout.tv_sec = itimeout;
		timeout.tv_usec = 0;

		//如果在itimeout时间内没有可用资源的文件描述符的话就退出 
		if (select(sockfd + 1, &tmpfd, 0, 0, &timeout) <= 0) return false;
	}

	//数据包 = 数据大小 + 数据
	(*ibuflen) = 0;

	if (!recvn(sockfd, (char*)ibuflen, 4)) return false;

	(*ibuflen) = ntohl(*ibuflen);  //把网络字节序转换为主机字节序。

	return recvn(sockfd, buffer, *ibuflen);
}

//向socket的对端发送数据。
//sockfd：可用的socket连接。
//buffer：待发送数据缓冲区的地址。
//ibuflen：待发送数据的字节数，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
//itimeout：接收等待超时的时间，单位：秒，值是0-无限等待。
//返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
bool TcpSend(const int sockfd, const char *buffer, const int ibuflen = 0, const int itimeout = 5) {
	if (sockfd == -1) return false;
	if (itimeout > 0) {  //延时操作 
		fd_set tmpfd;

		FD_ZERO(&tmpfd);
		FD_SET(sockfd, &tmpfd);

		struct timeval timeout;
		timeout.tv_sec = itimeout;
		timeout.tv_usec = 0;

		//如果在itimeout时间内没有可用资源的文件描述符的话就退出 
		if (select(sockfd + 1, &tmpfd, 0, 0, &timeout) <= 0) return false;
	}
	//如果长度为0，就采用字符串的长度
	int ilen = (ibuflen == 0) ? strlen(buffer) : ibuflen;
	int ilenn = htonl(ilen);  //转换为网络字节序
	int __iLenth = ilen + 4;
	//数据包 = 数据大小 + 数据
	char* strTBuffer = new char[__iLenth];  //前面保存长度
	memset(strTBuffer, 0, sizeof(strTBuffer));
	memcpy(strTBuffer, &ilenn, 4);
	memcpy(strTBuffer + 4, buffer, ilen);
	bool _bRetn = sendn(sockfd, strTBuffer, ilen + 4);
	delete[] strTBuffer;
	return _bRetn;
}

// TCP客户端连服务端的函数，serverip-服务端ip，port通信端口
// 返回值：成功返回已连接socket，失败返回-1。
int connecttoserver(const char *serverip, const int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 创建客户端的socket

	struct hostent* h; // ip地址信息的数据结构
	if ((h = gethostbyname(serverip)) == 0) {
		perror("gethostbyname");
		closesocket(sockfd);
		//close(sockfd);
		return -1;
	}

	// 把服务器的地址和端口转换为数据结构
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

	// 向服务器发起连接请求
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
		perror("connect");
		closesocket(sockfd);
		return -1;
	}

	return sockfd;
}

//// 初始化服务端的socket，port为通信端口
//// 返回值：成功返回初始化的socket，失败返回-1。
//int initserver(int port) {
//	int listenfd = socket(AF_INET, SOCK_STREAM, 0);  // 创建服务端的socket
//
//	//设置SO_REUSEADDR选项，解决关闭程序端口还在占用，再运行socket bind不成功的问题
//	int opt = 1;
//	unsigned int len = sizeof(opt);
//	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
//
//	// 把服务端用于通信的地址和端口绑定到socket上
//	struct sockaddr_in servaddr;    // 服务端地址信息的数据结构
//	memset(&servaddr, 0, sizeof(servaddr));
//	servaddr.sin_family = AF_INET;  // 协议族，在socket编程中只能是AF_INET
//	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 本主机的任意ip地址
//	servaddr.sin_port = htons(port);  // 绑定通信端口
//	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
//		perror("bind");
//		closesocket(listenfd);
//		return -1;
//	}
//	// 把socket设置为监听模式
//	if (listen(listenfd, 5) != 0) {
//		perror("listen");
//		closesocket(listenfd);
//		return -1;
//	}
//
//	return listenfd;
//}
#pragma endregion

#pragma region 客户端类
unsigned  __stdcall RecvProc(LPVOID lpParameter)
{
	CTcpClient* _Client = (CTcpClient*)lpParameter;
	if(_Client != nullptr)
		return _Client->RecvProcThread();
	else return 0;
}

void CTcpClient::RegisterRecieveData(RecieveData * ptr)
{
	m_mutex.lock();
	for (int i = 0; i < m_vecPtrRecieveData.size(); i++){
		if (m_vecPtrRecieveData[i] == ptr)	{
			m_mutex.unlock();
			return;
		}
	}
	m_vecPtrRecieveData.push_back(ptr);
	m_mutex.unlock();
}

void CTcpClient::RemoveRecieveData(RecieveData * ptr)
{
	m_mutex.lock();
	for (auto iter = m_vecPtrRecieveData.begin(); iter != m_vecPtrRecieveData.end(); iter++)	{
		if (ptr == *iter)	{
			m_vecPtrRecieveData.erase(iter);
			break;
		}
	}
	m_mutex.unlock();
}

//
CTcpClient::CTcpClient() {
	m_sockfd		= -1;
	memset(m_ip,	0, sizeof(m_ip));
	m_port			= 0;
	m_btimeout		= false;
	hThread			= 0;
	m_bConnected	= false;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_vecPtrRecieveData.clear();
	//m_PtrData		= nullptr;
	m_hTrrigerHand = ::CreateEventA(NULL, true, false, NULL);

	RstEvent();
}

bool CTcpClient::ConnectToServer(const char *ip, const int port) {

	if (m_sockfd != -1) {
		closesocket(m_sockfd);
		m_sockfd = -1;
	}

	strcpy(m_ip, ip);
	m_port = port;

	struct hostent* h;
	struct sockaddr_in servaddr;

	if ((m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return false;

	if (!(h = gethostbyname(m_ip))) {
		closesocket(m_sockfd);
		m_sockfd = -1;
		return false;
	}

	memset(&servaddr, 0,		sizeof(servaddr));
	servaddr.sin_family			= AF_INET;
	servaddr.sin_port			= htons(m_port);  // 指定服务端的通讯端口
	memcpy(&servaddr.sin_addr,	h->h_addr, h->h_length);

	if (connect(m_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
		closesocket(m_sockfd);
		m_sockfd = -1;
		return false;
	}
	m_bConnected = true;
	hThread = (HANDLE)_beginthreadex(NULL, 0, &RecvProc, (LPVOID)this, 0, 0);

	return true;
}

void CTcpClient::SetEvent()
{
	::SetEvent(m_hTrrigerHand);
}

void CTcpClient::RstEvent()
{
	::ResetEvent(m_hTrrigerHand);
}

int CTcpClient::WaitEvent(int timeout)
{
	DWORD dwRetn = WAIT_OBJECT_0;
	if (timeout < 0)
		dwRetn = WaitForSingleObject(m_hTrrigerHand, INFINITE);
	else
	{
		dwRetn = WaitForSingleObject(m_hTrrigerHand, timeout);
		if (dwRetn == WAIT_TIMEOUT)
			return -2;
		else
		{
			return dwRetn - WAIT_OBJECT_0;
		}
	}
	return dwRetn;
}

QString CTcpClient::localAddress()
{
	return QString(m_ip);
}

int CTcpClient::localPort()
{
	return m_port;
}

bool CTcpClient::Recv(char *buffer, const int itimeout) {
	if (m_sockfd == -1) return false;

	if (itimeout > 0) {
		fd_set tmpfd;

		FD_ZERO(&tmpfd);
		FD_SET(m_sockfd, &tmpfd);

		struct timeval timeout;
		timeout.tv_sec = itimeout;
		timeout.tv_usec = 0;

		m_btimeout = false;

		int i;
		if ((i = select(m_sockfd + 1, &tmpfd, 0, 0, &timeout)) <= 0) {
			if (i == 0) m_btimeout = true;
			return false;
		}
	}

	m_buflen = 0;
	return (TcpRecv(m_sockfd, buffer, &m_buflen));
}

int CTcpClient::write(QString & str)
{
	return send(m_sockfd, str.toStdString().c_str(), str.length(), 0);
	//return(TcpSend(m_sockfd, str.toStdString().c_str(), str.length()));
}

int CTcpClient::write(std::string& str)
{
	return send(m_sockfd, str.c_str(), str.length(), 0);
}

bool CTcpClient::WaitMsg(QString strSend, QString & strMsg, int itimeout)
{
	RstEvent();
	write(strSend);
	if (WaitEvent(itimeout) < 0)
		return false;
	strMsg = m_strRecieveMsg;
	return true;
}

int CTcpClient::WaitMsg(QString & strMsg, int itimeout)
{
	int _iRetn = WaitEvent(itimeout);
	RstEvent();
	if (_iRetn  < 0) {
		return -1;
	}
	strMsg = m_strRecieveMsg;
	m_strRecieveMsg.clear();
	return 1;
}

void CTcpClient::ClearRecieve()
{
	m_vecPtrRecieveData.clear();
	RstEvent();
}

int CTcpClient::Recieve(CTcpClient* socket, QString & str)
{
	m_mutex.lock();
	m_strRecieveMsg = str;
	for (auto iter : m_vecPtrRecieveData)	iter->RecieveTCP(socket, str);
	m_mutex.unlock();

	return 0;
}

int CTcpClient::sig_Connected(CTcpClient* socket)
{
	return 0;
}

int CTcpClient::sig_DisConnected(CTcpClient* socket)
{
	m_mutex.lock();
	for (auto iter : m_vecPtrRecieveData)	iter->DisConnect(socket);
	m_mutex.unlock();
	//if (m_PtrData != nullptr)	m_PtrData->DisConnect(socket);
	return 0;
}

bool CTcpClient::Send(const char *buffer, const int ibuflen, const int itimeout) {
	if (m_sockfd == -1) return false;

	if (itimeout > 0) {
		fd_set tmpfd;

		FD_ZERO(&tmpfd);
		FD_SET(m_sockfd, &tmpfd);

		struct timeval timeout;
		timeout.tv_sec = itimeout;
		timeout.tv_usec = 0;

		m_btimeout = false;

		int i;
		if ((i = select(m_sockfd + 1, &tmpfd, 0, 0, &timeout)) <= 0) {
			if (i == 0) m_btimeout = true;
			return false;
		}
	}

	int ilen = ibuflen;

	if (ibuflen == 0) ilen = strlen(buffer);

	return(TcpSend(m_sockfd, buffer, ilen));
}

void CTcpClient::Close() {
	if (m_sockfd > 0) closesocket(m_sockfd);
	m_bConnected = false;
	//sig_DisConnected(this);
	m_sockfd = -1;
	m_vecPtrRecieveData.clear();
	m_btimeout = false;
}

void CTcpClient::disconnect()
{
	Close();
}

CTcpClient::~CTcpClient() {

	Close();
	memset(m_ip, 0, sizeof(m_ip));
	m_port = 0;
	//m_PtrData = nullptr;
	m_vecPtrRecieveData.clear();
	WSACleanup();
	SetEvent();
	if (m_hTrrigerHand != nullptr)
		CloseHandle(m_hTrrigerHand);
	m_hTrrigerHand = nullptr;
}
int CTcpClient::RecvProcThread()
{
	int	_iRetn = 0;
	int iLength;
	char strRecvMsg[Const_BufLength];
	sig_Connected(this);
	while (m_bConnected)
	{
		RstEvent();
		memset(strRecvMsg, 0, sizeof(strRecvMsg));
		iLength = ::recv(m_sockfd, strRecvMsg, sizeof(strRecvMsg), 0);
		m_strRecieveMsg = strRecvMsg;
		SetEvent();
		if (iLength <= 0)
		{
			_iRetn = -1; break;
		}
		Recieve(this, m_strRecieveMsg);
	}
	m_bConnected = false;
	sig_DisConnected(this);
	return _iRetn;
}
#pragma endregion

#pragma region 服务器
unsigned  __stdcall ConnectedProc(LPVOID lpParameter)
{
	CTcpServer* _Client = (CTcpServer*)lpParameter;
	if (_Client != nullptr)
		return _Client->WaitConnected();
	else return 0;
}
unsigned  __stdcall RecvClientProc(LPVOID lpParameter)
{
	MyTcpServer* _Server = (MyTcpServer*)lpParameter;
	int	_iRetn = 0;

	if (_Server != nullptr)	{
		int	  _iRetn = 0;
		int	  iLength;
		char  strRecvMsg[Const_BufLength] = { 0 };
		_Server->m_Server->sig_Connected(_Server->m_Client);
		while (_Server->m_Client->m_bConnected)	{
			memset(strRecvMsg, 0, sizeof(char) * Const_BufLength);
			iLength = ::recv(_Server->m_Client->m_sockfd, strRecvMsg, sizeof(char) * Const_BufLength, 0);
			if (iLength <= 0)	{
				_iRetn = -1; break;
			}
			if (!_Server->m_Server->m_bConnected)	{
				break;
			}
			QString _str = QString(strRecvMsg);
			//_Server->m_Server->SetRecieveMsg(_str);
			_Server->m_Server->Recieve(_Server->m_Client, _str);
			_Server->m_Server->SetEvent();

		}
		_Server->m_Server->sig_DisConnected(_Server->m_Client);
		_Server->m_Client->Close();
		_Server->m_Client->m_bConnected = false;
	}
	_Server->m_Client->m_bConnected = false;

	for (size_t i = 0; i < _Server->m_Server->m_vecCTcpClient.size(); i++)	{
		if (_Server->m_Server->m_vecCTcpClient[i] == _Server)		{
			_Server->m_Server->m_vecCTcpClient.erase(_Server->m_Server->m_vecCTcpClient.begin() + i);
			break;
		}
	}
	if (_Server->m_Client != nullptr)	delete _Server->m_Client;
	if (_Server != nullptr)				delete _Server;
	return _iRetn;

}

void CTcpServer::SetEvent()
{
	::SetEvent(m_hTrrigerHand);
}

void CTcpServer::RstEvent()
{
	::ResetEvent(m_hTrrigerHand);
}

int CTcpServer::WaitEvent(int timeout)
{
	DWORD dwRetn = WAIT_OBJECT_0;
	if (timeout < 0)
		dwRetn = WaitForSingleObject(m_hTrrigerHand, INFINITE);
	else
	{
		dwRetn = WaitForSingleObject(m_hTrrigerHand, timeout);
		if (dwRetn == WAIT_TIMEOUT)
			return -2;
		else
		{
			return dwRetn - WAIT_OBJECT_0;
		}
	}
	return dwRetn;
}

void CTcpServer::RegisterRecieveData(RecieveData * ptr)
{
	m_mutex.lock();
	for (int i = 0; i < m_vecPtrRecieveData.size(); i++) {
		if (m_vecPtrRecieveData[i] == ptr) {
			m_mutex.unlock();
			return;
		}
	}
	m_vecPtrRecieveData.push_back(ptr);
	m_mutex.unlock();
}

void CTcpServer::RemoveRecieveData(RecieveData * ptr)
{
	m_mutex.lock();
	for (auto iter = m_vecPtrRecieveData.begin(); iter != m_vecPtrRecieveData.end(); iter++) {
		if (ptr == *iter) {
			m_vecPtrRecieveData.erase(iter);
			break;
		}
	}
	m_mutex.unlock();
}

void CTcpServer::ClearRecieveData()
{
	m_mutex.lock();
	//m_PtrData = nullptr;
	m_vecPtrRecieveData.clear();
	m_mutex.unlock();
}

CTcpServer::CTcpServer() {
	m_listenfd	= -1;
	m_connfd	= -1;
	m_socklen	= 0;
	m_btimeout	= false;
	WSADATA		wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_bConnected = false;
	//m_PtrData = nullptr;
	m_hTrrigerHand = ::CreateEventA(NULL, true, false, NULL);

	m_vecPtrRecieveData.clear();
}

bool CTcpServer::InitServer(char *ip, const unsigned int port) {
	CloseListen();

	if ((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) return false;

	//设置SO_REUSEADDR选项，解决关闭程序端口还在占用，再运行socket bind不成功的问题
	// WINDOWS平台如下
	char b_opt='1';
	setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&b_opt,sizeof(b_opt));

	//// Linux如下
	//int opt = 1;
	//unsigned int len = sizeof(opt);
	//::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);

	memset(&m_servaddr, 0, sizeof(m_servaddr));
	m_servaddr.sin_family = AF_INET;
	m_servaddr.sin_addr.S_un.S_addr = inet_addr(ip);
	//m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_servaddr.sin_port = htons(port);
	if (bind(m_listenfd, (struct sockaddr *)&m_servaddr, sizeof(m_servaddr)) != 0) {
		CloseListen();
		return false;
	}

	if (listen(m_listenfd, 5) != 0) {
		CloseListen();
		return false;
	}
	hThread = (HANDLE)_beginthreadex(NULL, 0, &ConnectedProc, (LPVOID)this, 0, 0);

	m_socklen = sizeof(struct sockaddr_in);

	return true;
}

QString CTcpServer::localAddress()
{
	in_addr _sin_addr;
	memcpy_s(&_sin_addr,sizeof(_sin_addr), &m_servaddr.sin_addr, sizeof(_sin_addr));

	return QString(inet_ntoa(_sin_addr));
}

int CTcpServer::localPort()
{
	return ntohs(m_servaddr.sin_port);
}

int CTcpServer::WaitConnected()
{
	int	_iRetn = 0;
	m_bConnected = true;
	while (m_bConnected)
	{
		if ((m_connfd = accept(m_listenfd, (struct sockaddr *)&m_clientaddr, (int*)&m_socklen)) < 0)
			return false;
		MyTcpServer* _PtrTCP			= new MyTcpServer();
		_PtrTCP->m_Client				= new CTcpClient();
		_PtrTCP->m_Server				= this;
		_PtrTCP->m_Client->m_bConnected	= true;
		u_long mode = 0;
		ioctlsocket(m_connfd,FIONBIO,&mode);
		_PtrTCP->m_Client->m_sockfd		= m_connfd;
		_PtrTCP->m_Client->m_port		= ntohs(m_clientaddr.sin_port);
		{
			in_addr _sin_addr;
			memcpy_s(&_sin_addr, sizeof(_sin_addr), &m_clientaddr.sin_addr, sizeof(m_clientaddr.sin_addr));
			strcpy(_PtrTCP->m_Client->m_ip, inet_ntoa(_sin_addr));
		}

		_PtrTCP->m_Client->hThread		= (HANDLE)_beginthreadex(NULL, 0, &RecvClientProc, (LPVOID)_PtrTCP, 0, 0);
		m_vecCTcpClient.push_back(_PtrTCP);

	}
	m_bConnected = false;
	return _iRetn;
}

bool CTcpServer::isOpen()
{
	return	m_bConnected;
}

bool CTcpServer::isListening()
{
	return	m_bConnected;
}

int CTcpServer::Recieve(CTcpClient* socket, QString & str)
{
	m_mutex.lock();
	m_strRecieveMsg = str;
	for (auto iter : m_vecPtrRecieveData)	iter->RecieveTCP(socket, str);
	m_mutex.unlock();
	//if (m_PtrData != nullptr)	m_PtrData->RecieveTCP(socket, str);

	return 0;
}

int CTcpServer::sig_Connected(CTcpClient* socket)
{
	return 0;
}

int CTcpServer::sig_DisConnected(CTcpClient* socket)
{
	m_mutex.lock();
	for (auto iter : m_vecPtrRecieveData)	iter->DisConnect(socket);
	m_mutex.unlock();
	//if (m_PtrData != nullptr)	m_PtrData->DisConnect(socket);

	return 0;
}

bool CTcpServer::Accept() {
	if (m_listenfd == -1) return false;

	if ((m_connfd = accept(m_listenfd, (struct sockaddr *)&m_clientaddr, (int*)&m_socklen)) < 0)
		return false;

	return true;
}

char *CTcpServer::GetIP() {
	in_addr _sin_addr;
	memcpy_s(&_sin_addr, sizeof(_sin_addr), &m_clientaddr.sin_addr, sizeof(m_clientaddr.sin_addr));
	return(inet_ntoa(_sin_addr));
}

bool CTcpServer::Recv(char *buffer, const int itimeout) {
	if (m_connfd == -1) return false;

	if (itimeout > 0) {
		fd_set tmpfd;

		FD_ZERO(&tmpfd);
		FD_SET(m_connfd, &tmpfd);

		struct timeval timeout;
		timeout.tv_sec = itimeout;
		timeout.tv_usec = 0;

		m_btimeout = false;

		int i;
		if ((i = select(m_connfd + 1, &tmpfd, 0, 0, &timeout)) <= 0) {
			if (i == 0) m_btimeout = true;
			return false;
		}
	}

	m_buflen = 0;
	return(TcpRecv(m_connfd, buffer, &m_buflen));
}

bool CTcpServer::Send(const char *buffer, const int ibuflen, const int itimeout) {
	if (m_connfd == -1) return false;

	if (itimeout > 0) {
		fd_set tmpfd;

		FD_ZERO(&tmpfd);
		FD_SET(m_connfd, &tmpfd);

		struct timeval timeout;
		timeout.tv_sec = itimeout;
		timeout.tv_usec = 0;

		m_btimeout = false;

		int i;
		if ((i = select(m_connfd + 1, &tmpfd, 0, 0, &timeout)) <= 0) {
			if (i == 0) m_btimeout = true;
			return false;
		}
	}

	int ilen = ibuflen;
	if (ilen == 0) ilen = strlen(buffer);

	return(TcpSend(m_connfd, buffer, ilen));
}

bool CTcpServer::SendAllMsg(QString strMsg) {

	for (auto iter : m_vecCTcpClient) {
		if (iter != nullptr)
			if (iter->m_Client != nullptr)
				if (iter->m_Client->m_bConnected)
					iter->m_Client->write(strMsg);
	}
	return true;
}

bool CTcpServer::WaitMsg(QString strSend, QString & strMsg, int itimeout)
{
	bool _bFlag = true;
	for (auto iter : m_vecCTcpClient) {
		if (iter != nullptr)
			if (iter->m_Client != nullptr)
				if (iter->m_Client->m_bConnected)
					iter->m_Client->RstEvent();
	}
	for (auto iter : m_vecCTcpClient) {
		if (iter != nullptr)
			if (iter->m_Client != nullptr)
				if (iter->m_Client->m_bConnected)
					iter->m_Client->write(strSend);
	}

	for (auto iter : m_vecCTcpClient) {
		if (iter != nullptr)
			if (iter->m_Client != nullptr)
				if (iter->m_Client->m_bConnected)
					if (iter->m_Client->WaitEvent(itimeout) < 0) {
						_bFlag = false;
					}
					else {
						strMsg = iter->m_Client->m_strRecieveMsg;
					}
	}

	return _bFlag;
}

int CTcpServer::WaitMsg(QString & strMsg, int itimeout)
{
	int _iRetn = WaitEvent(itimeout);
	RstEvent();
	if (_iRetn < 0) {
		return -1;
	}

	m_mutex.lock();
	strMsg = m_strRecieveMsg;
	m_strRecieveMsg.clear();
	m_mutex.unlock();
	return 1;
}

void CTcpServer::ClearRecieve()
{
	m_mutex.lock();
	m_vecPtrRecieveData.clear();
	m_mutex.unlock();
	RstEvent();
}

void CTcpServer::CloseListen() {
	if (m_listenfd > 0) {
		closesocket(m_listenfd);
		m_listenfd = -1;
	}
	m_vecPtrRecieveData.clear();
	m_bConnected = false;
}

void CTcpServer::CloseClient() {
	if (m_connfd > 0) {
		closesocket(m_connfd);
		m_connfd = -1;
	}
}

void CTcpServer::disconnect()
{
	// 关闭监听的socket，即m_listenfd，常用于多进程服务程序的子进程代码中。
	CloseListen();
}

void CTcpServer::Close()
{
	m_bConnected = false;
	CloseListen();
}

CTcpServer::~CTcpServer() {
	CloseListen();
	CloseClient();
	m_vecPtrRecieveData.clear();
	WSACleanup();
	if (m_hTrrigerHand != nullptr)
		CloseHandle(m_hTrrigerHand);
	m_hTrrigerHand = nullptr;
}
#pragma endregion
CTcpSocket::CTcpSocket(QObject *parent) 
	: QObject(parent)
{
	//m_PtrData = nullptr;
	m_vecPtrRecieveData.clear();
}

CTcpSocket::~CTcpSocket()
{
	//m_PtrData = nullptr;
	m_vecPtrRecieveData.clear();
}

int CTcpSocket::Recieve(CTcpClient* socket, QString & str)
{
	for (auto iter : m_vecPtrRecieveData)	iter->RecieveTCP(socket, str);
	//if (m_PtrData != nullptr)	m_PtrData->RecieveTCP(socket, str);
	emit sig_ReadyRead((CTcpSocket*)socket, str);
	return 0;
}

int CTcpSocket::sig_Connected(CTcpClient* socket)
{
	emit sig_ConnectedSocket("", (CTcpSocket*)socket);
	return 0;
}

int CTcpSocket::sig_DisConnected(CTcpClient* socket)
{
	for (auto iter : m_vecPtrRecieveData)	iter->DisConnect(socket);
	//if (m_PtrData != nullptr)	m_PtrData->DisConnect(socket);

	emit sig_DisConnectedSocket("", (CTcpSocket*)socket);
	return 0;
}

void CTcpSocket::Write(QString& strValue)
{
	CTcpClient::write(strValue);
}

CTcpServerSocket::CTcpServerSocket(QObject *parent)
	: QObject(parent)
{
	m_vecPtrRecieveData.clear();
}

CTcpServerSocket::~CTcpServerSocket()
{
	m_vecPtrRecieveData.clear();
}

//void CTcpServerSocket::Write(QString& strValue)
//{
//	//CTcpServer::write(strValue);
//}

int CTcpServerSocket::Recieve(CTcpClient* socket, QString & str)
{
	m_mutex.lock();
	m_strRecieveMsg = str;
	for (auto iter : m_vecPtrRecieveData)	iter->RecieveTCP(socket, str);
	m_mutex.unlock();

	//if (m_PtrData != nullptr)	m_PtrData->RecieveTCP(socket, str);
	emit sig_ReadyRead((CTcpSocket*)socket, str);
	return 0;
}

int CTcpServerSocket::sig_Connected(CTcpClient* socket)
{
	emit sig_ConnectedSocket("", (CTcpSocket*)socket);
	return 0;
}

int CTcpServerSocket::sig_DisConnected(CTcpClient* socket)
{
	m_mutex.lock();
	for (auto iter : m_vecPtrRecieveData)	iter->DisConnect(socket);
	m_mutex.unlock();

	//if (m_PtrData != nullptr)	m_PtrData->DisConnect(socket);
	emit sig_DisConnectedSocket("", (CTcpSocket*)socket);
	return 0;
}


MyCnCommort::MyCnCommort(QObject *parent)
	: QObject(parent)
{
	m_vecPtrRecieveData.clear();
	//m_PtrData = nullptr;
}

MyCnCommort::~MyCnCommort()
{
	m_vecPtrRecieveData.clear();
	//m_PtrData = nullptr;
}

int MyCnCommort::Recieve(CnComm * socket, QString & str)
{
	for (auto iter : m_vecPtrRecieveData)	iter->Recieve(socket, str);
	//if (m_PtrData != nullptr)	m_PtrData->Recieve(str);
	emit sig_readyRead(this, str);
	return 0;
}

void MyCnCommort::Write(QString& strValue)
{
	CnComm::Write(strValue.toStdString().c_str());
}