#include "TcpServer.h"
#include <QTcpSocket>
#include "TcpSocket.h"

TcpServer::TcpServer(int port):m_prot(port)
{
	
}

TcpServer::~TcpServer()
{
}

bool TcpServer::run()
{
	/*listen服务端监听(监听的地址范围[默认Any所有],端口号)*/
	//if (this->listen(QHostAddress::Any,m_prot))
	if (this->listen(QHostAddress::AnyIPv4, m_prot))
	{
		qDebug() << QString::fromLocal8Bit("服务端监听端口 %1 成功!").arg(m_prot);
		return true;
	}
	else
	{
		qDebug() << QString::fromLocal8Bit("服务端监听端口 %1 失败!").arg(m_prot);
		return false;
	}
}

/***客户端有新的连接时(描述符) //重写******************
/***void incomingConnection(int socketDescriptor);//旧*/
void TcpServer::incomingConnection(qintptr socketDescriptor)//新
{
	qDebug() << QString::fromLocal8Bit("新的连接:") << socketDescriptor << "\n";

	TcpSocket* tcpsocket = new TcpSocket();
	/*设置TcpSocket描述符*/
	tcpsocket->setSocketDescriptor(socketDescriptor);
	tcpsocket->run();/*初始化*/
	/*TcpSocket客户端数据来了,TcpServer处理数据*/
	connect(tcpsocket,SIGNAL(signalGetDataFromClient(QByteArray&,int)),this,SLOT(onSocketDataProcessing(QByteArray&, int)));
	/*TcpSocket客户端断开连接,TcpServer断开连接处理*/
	connect(tcpsocket, SIGNAL(signalCilentDisconnect(int)), this, SLOT(onSocketDisconnected(int)));
	/*添加客户端连接*/
	m_tcpSocketConnectList.append(tcpsocket);
}

/*处理数据(发送的信息,描述符)*/
void TcpServer::onSocketDataProcessing(QByteArray& SendData, int descriptor) 
{
	for(int i = 0;i< m_tcpSocketConnectList.count();i++)
	{
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		if (item->socketDescriptor(/*Tcp接口描述符*/) == descriptor) 
		{
			qDebug() << QString::fromLocal8Bit("来自IP:") << item->peerAddress(/*对等地址*/).toString() 
						<< QString::fromLocal8Bit("发来的数据:") << QString(SendData);

			emit signalTcpMsgComes(SendData);/*tcp消息来了*/
		}
	}
}

/*断开连接处理(描述符[谁断开连接])*/
void TcpServer::onSocketDisconnected(int descriptor) 
{
	for (int i = 0; i < m_tcpSocketConnectList.count(); i++)
	{
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		int itemDescriptor = item->socketDescriptor();/*Tcp接口描述符*/

		if (itemDescriptor == descriptor || itemDescriptor == -1)
		{
			m_tcpSocketConnectList.removeAt(i);/*移除·断开的socket从链表移除*/
			item->deleteLater();/*资源回收*/
			qDebug() << QString::fromLocal8Bit("TcpSocket断开连接:") << itemDescriptor << "\n";
			return;
		}
	}
}