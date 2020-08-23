#pragma once

#include <QTcpServer> //Tcp服务
#include <QList>

/*********Tcp服务器:************
*****对连接的客户端进行维护*****/
class TcpServer : public QTcpServer
{
	Q_OBJECT

public:
	//port端口号
	TcpServer(int port);
	~TcpServer();

	//监听端口有没连接 //有true
	bool run();

protected:
	/*客户端有新的连接时 //重写*************************
	void incomingConnection(int socketDescriptor);//旧*/
	void incomingConnection(qintptr socketDescriptor);//新

signals:
	void signalTcpMsgComes(QByteArray&);
private slots:
	/*处理数据(发送的信息,描述符)*/
	void onSocketDataProcessing(QByteArray& SendData,int descriptor);
	/*断开连接处理(描述符[谁断开连接])*/
	void onSocketDisconnected(int descriptor);
private:
	int m_prot;//端口号
	/*tcp接口连接链表*****************************  
	**客户端连接成功就添加到链表,下线了就进行移除*/
	QList<QTcpSocket*> m_tcpSocketConnectList;
};
