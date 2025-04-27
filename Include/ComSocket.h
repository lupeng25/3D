#pragma once

#include <winsock.h>
#include <QTcpSocket>
#include <QTcpServer>
#include <QSerialPort>
#include "CnComm.h"
#include <QMutex>

class QMutex;
class CTcpClient;

#ifndef _COMSOCKET_EXPORT_
#define _COMSOCKET_API_ _declspec(dllexport)
#else
#define _COMSOCKET_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _COMSOCKET_API_ ComSocket  : public QTcpSocket
{
	Q_OBJECT

public:
	ComSocket(QObject *parent);
	~ComSocket();
public slots:


private slots:
	void ConnectSocket();
	void DisConnectSocket();
	void Write(QByteArray ba);//写入数据槽函数
public:
Q_SIGNALS:
	void sig_ConnectedSocket(QString strkey,QTcpSocket* socket);
	void sig_DisConnectedSocket(QString strkey,QTcpSocket* socket);
public:


public:
	QString	m_strKey;
	bool m_bIsConnected;
};

class _COMSOCKET_API_ SocketInformation
{
public:
	ComSocket* mysocket;//socket指针
	QString strIP;//ip端口字符串
	QString strPort;//ip端口字符串
	int threadIndex;//所在线程ID
};

class _COMSOCKET_API_ MyServer : public QTcpServer
{
	Q_OBJECT

public:
	/*explicit*/ MyServer(QObject *parent = nullptr);
	~MyServer();
public:
	void ResetConnect();
public:
	QList<SocketInformation> list_information;//socket信息列表

public slots:

private slots:
	void serviceClientConnected();

public:
Q_SIGNALS:
	void sig_ConnectedSocket(QTcpSocket* socket);
	void sig_DisConnectedSocket(QTcpSocket* socket);
	void sig_ReadyReadConnectedSocket(QTcpSocket* socket);
};

class _COMSOCKET_API_ MyQSerialPort : public QSerialPort
{
	Q_OBJECT

public:
	explicit MyQSerialPort(QObject *parent = nullptr);
	~MyQSerialPort();
public slots:
	void Write(QByteArray ba);//写入数据槽函数
public:
	QString	m_strKey;
};

class RecieveData
{
public:
	virtual	int	RecieveTCP(CTcpClient* sock,QString& str) = 0;
	virtual	int	DisConnect(CTcpClient* sock) = 0;

};
enum EnumTCpType
{
	TCpType_Server,
	TCpType_Client
};


// socket通信的客户端类
class _COMSOCKET_API_ CTcpClient {
private:
	bool m_btimeout;  // 调用Recv和Send方法时，失败的原因是否是超时：true-超时，false-未超时。
	int  m_buflen;    // 调用Recv方法后，接收到的报文的大小，单位：字节。
	QMutex	m_mutex;
public:
	char	m_ip[21];    // 服务端的ip地址。
	int		m_port;      // 与服务端通信的端口。
	int		m_sockfd;    // 客户端的socket.
	bool	m_bConnected;
	void*	hThread{ nullptr };

	std::vector<RecieveData*> m_vecPtrRecieveData;
	RecieveData* m_PtrData = nullptr;

	void RegisterRecieveData(RecieveData* ptr);// { m_PtrData = ptr; };
	void RemoveRecieveData(RecieveData* ptr);// { m_PtrData = ptr; };
	void ClearRecieveData();

	//void SetRecieveMsg(QString strMsg);

	QString			m_strRecieveMsg;

	void*			m_hTrrigerHand;
public:
	void SetEvent();
	void RstEvent();
	int WaitEvent(int iTimeOut = 5000);
public:
	CTcpClient();  // 构造函数。

	bool isOpen() { return	m_bConnected; };
	// 向服务端发起连接请求。
	// ip：服务端的ip地址。
	// port：服务端监听的端口。
	// 返回值：true-成功；false-失败。
	bool ConnectToServer(const char *ip, const int port);

	QString	localAddress();
	int	localPort();
	// 接收服务端发送过来的数据。
	// buffer：接收数据缓冲区的地址，数据的长度存放在m_buflen成员变量中。
	// itimeout：等待数据的超时时间，单位：秒，缺省值是0-无限等待。
	// 返回值：true-成功；false-失败，失败有两种情况：1）等待超时，成员变量m_btimeout的值被设置为true；2）socket连接已不可用。
	bool Recv(char *buffer, const int itimeout = 0);

	int write(QString& str);

	int write(std::string& str);

	bool WaitMsg(QString strSend, QString& strMsg, int itimeout = 5000);

	int WaitMsg(QString& strMsg, int itimeout = 2000);

	void ClearRecieve();

	//接受数据
	virtual	int	Recieve(CTcpClient* socket, QString& str);
	virtual	int	sig_Connected(CTcpClient* socket);
	virtual	int	sig_DisConnected(CTcpClient* socket);
	// 向服务端发送数据。
	// buffer：待发送数据缓冲区的地址。
	// ibuflen：待发送数据的大小，单位：字节，缺省值为0，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
	//itimeout：接收等待超时的时间，单位：秒，值是0-无限等待。
	// 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
	bool Send(const char *buffer, const int ibuflen = 0, const int itimeout = 5);

	// 断开与服务端的连接
	void Close();

	//关闭服务器
	void disconnect();

	~CTcpClient();  // 析构函数自动关闭socket，释放资源。
public:
	int	RecvProcThread();

};
class CTcpServer;
struct _COMSOCKET_API_ MyTcpServer
{
	CTcpServer* m_Server;
	CTcpClient* m_Client;
};

typedef struct in_addr_ {
	union {
		struct { unsigned char s_b1, s_b2, s_b3, s_b4; } S_un_b;
		struct { unsigned short s_w1, s_w2; } S_un_w;
		unsigned long S_addr;
	} S_un;
#define s_addr_  S_un.S_addr /* can be used for most tcp & ip code */
#define s_host_  S_un.S_un_b.s_b2    // host on imp
#define s_net_   S_un.S_un_b.s_b1    // network
#define s_imp_   S_un.S_un_w.s_w2    // imp
#define s_impno_ S_un.S_un_b.s_b4    // imp #
#define s_lh_    S_un.S_un_b.s_b3    // logical host
} IN_ADDR_, *PIN_ADDR_, FAR *LPIN_ADDR_;

struct sockaddr_ {
	short   sin_family;
	unsigned short sin_port;
	struct  in_addr_ sin_addr;
	char    sin_zero[8];
};

// socket通信的服务端类
class _COMSOCKET_API_ CTcpServer {
private:
	int m_socklen;           // 结构体struct sockaddr_in的大小。
	sockaddr_ m_clientaddr;  // 客户端的地址信息。
	sockaddr_ m_servaddr;    // 服务端的地址信息。

	int		m_listenfd;   // 服务端用于监听的socket。
	int		m_connfd;     // 客户端连接上来的socket。
	bool	m_btimeout;   // 调用Recv和Send方法时，失败的原因是否是超时：true-超时，false-未超时。
	int		m_buflen;     // 调用Recv方法后，接收到的报文的大小，单位：字节。
	void*	hThread{ nullptr };
protected:
	QMutex	m_mutex;
	void*	m_hTrrigerHand;
public:
	QString	m_strRecieveMsg;
	//void SetRecieveMsg(QString strMsg);
	void SetEvent();
	void RstEvent();
	int WaitEvent(int iTimeOut = 5000);
public:
	std::vector<MyTcpServer*> m_vecCTcpClient;
	bool m_bConnected;
	std::vector<RecieveData*> m_vecPtrRecieveData;

	RecieveData* m_PtrData = nullptr;
public:
	void RegisterRecieveData(RecieveData* ptr);// { m_PtrData = ptr; };
	void RemoveRecieveData(RecieveData* ptr);// { m_PtrData = ptr; };
	void ClearRecieveData();

	CTcpServer();  // 构造函数。

	~CTcpServer();  // 析构函数自动关闭socket，释放资源。

	// 服务端初始化。
	// port：指定服务端用于监听的端口。
	// 返回值：true-成功；false-失败，一般情况下，只要port设置正确，没有被占用，初始化都会成功。
	bool InitServer(char *ip,const unsigned int port);

	QString	localAddress();
	int	localPort();

	//等待客户端连接
	int WaitConnected();

	bool isOpen() ;

	bool isListening();
	//接受数据
	virtual	int	Recieve(CTcpClient* socket,QString& str);
	virtual	int	sig_Connected(CTcpClient* socket);
	virtual	int	sig_DisConnected(CTcpClient* socket);
	// 阻塞等待客户端的连接请求。
	// 返回值：true-有新的客户端已连接上来，false-失败，Accept被中断，如果Accept失败，可以重新Accept。
	bool Accept();

	// 获取客户端的ip地址。
	// 返回值：客户端的ip地址，如"192.168.1.100"。
	char *GetIP();

	// 接收客户端发送过来的数据。
	// buffer：接收数据缓冲区的地址，数据的长度存放在m_buflen成员变量中。
	// itimeout：等待数据的超时时间，单位：秒，缺省值是0-无限等待。
	// 返回值：true-成功；false-失败，失败有两种情况：1）等待超时，成员变量m_btimeout的值被设置为true；2）socket连接已不可用。
	bool Recv(char *buffer, const int itimeout = 0);

	// 向客户端发送数据。
	// buffer：待发送数据缓冲区的地址。
	// ibuflen：待发送数据的大小，单位：字节，缺省值为0，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
	//itimeout：接收等待超时的时间，单位：秒，值是0-无限等待。
	// 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
	bool Send(const char *buffer, const int ibuflen = 0, const int itimeout = 5);

	// 向客户端发送数据。
	bool SendAllMsg(QString strMsg);

	// 向客户端发送数据。
	bool WaitMsg(QString strSend, QString& strMsg, int itimeout = 5000);

	int WaitMsg(QString& strMsg, int itimeout = 2000);

	void ClearRecieve();

	// 关闭监听的socket，即m_listenfd，常用于多进程服务程序的子进程代码中。
	void CloseListen();

	// 关闭客户端的socket，即m_connfd，常用于多进程服务程序的父进程代码中。
	void CloseClient();

	//关闭服务器
	void disconnect();

	// 断开与服务端的连接
	void Close();
};

class _COMSOCKET_API_ CTcpSocket :public QObject, public CTcpClient
{
	Q_OBJECT
public:
	explicit CTcpSocket(QObject *parent = nullptr);
	~CTcpSocket();

public:
	QString m_strKey;
public:
	//接受数据
	virtual	int	Recieve(CTcpClient* socket, QString& str);
	virtual	int	sig_Connected(CTcpClient* socket);
	virtual	int	sig_DisConnected(CTcpClient* socket);

private slots:
	void Write(QString& strValue);//写入数据槽函数

public:
Q_SIGNALS:
	void sig_ConnectedSocket(QString, CTcpSocket* socket);
	void sig_DisConnectedSocket(QString, CTcpSocket* socket);
	void sig_ReadyRead(CTcpSocket* socket,QString& str);

};

class _COMSOCKET_API_ CTcpServerSocket :public QObject, public CTcpServer
{
	Q_OBJECT
public:
	explicit CTcpServerSocket(QObject *parent = nullptr);
	~CTcpServerSocket();
public:
	QString m_strKey;
public:
	//接受数据
	virtual	int	Recieve(CTcpClient* socket, QString& str);
	virtual	int	sig_Connected(CTcpClient* socket);
	virtual	int	sig_DisConnected(CTcpClient* socket);
private slots:
	//void Write(QString& strValue);//写入数据槽函数
public:
Q_SIGNALS:
	void sig_ConnectedSocket(QString, CTcpSocket* socket);
	void sig_DisConnectedSocket(QString, CTcpSocket* socket);
	void sig_ReadyRead(CTcpSocket* socket, QString& str);
};

class _COMSOCKET_API_ MyCnCommort :public QObject, public CnComm
{
	Q_OBJECT
public:
	explicit MyCnCommort(QObject *parent = nullptr);
	~MyCnCommort();

public:
	virtual	int	Recieve(CnComm* socket, QString& str);
	public slots:
	void Write(QString& strValue);//写入数据槽函数

public:
	Q_SIGNALS :
			  void sig_readyRead(MyCnCommort* socket, QString& str);
public:
	QString	m_strKey;
};