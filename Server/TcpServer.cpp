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
	/*listen����˼���(�����ĵ�ַ��Χ[Ĭ��Any����],�˿ں�)*/
	//if (this->listen(QHostAddress::Any,m_prot))
	if (this->listen(QHostAddress::AnyIPv4, m_prot))
	{
		qDebug() << QString::fromLocal8Bit("����˼����˿� %1 �ɹ�!").arg(m_prot);
		return true;
	}
	else
	{
		qDebug() << QString::fromLocal8Bit("����˼����˿� %1 ʧ��!").arg(m_prot);
		return false;
	}
}

/***�ͻ������µ�����ʱ(������) //��д******************
/***void incomingConnection(int socketDescriptor);//��*/
void TcpServer::incomingConnection(qintptr socketDescriptor)//��
{
	qDebug() << QString::fromLocal8Bit("�µ�����:") << socketDescriptor << "\n";

	TcpSocket* tcpsocket = new TcpSocket();
	/*����TcpSocket������*/
	tcpsocket->setSocketDescriptor(socketDescriptor);
	tcpsocket->run();/*��ʼ��*/
	/*TcpSocket�ͻ�����������,TcpServer��������*/
	connect(tcpsocket,SIGNAL(signalGetDataFromClient(QByteArray&,int)),this,SLOT(onSocketDataProcessing(QByteArray&, int)));
	/*TcpSocket�ͻ��˶Ͽ�����,TcpServer�Ͽ����Ӵ���*/
	connect(tcpsocket, SIGNAL(signalCilentDisconnect(int)), this, SLOT(onSocketDisconnected(int)));
	/*��ӿͻ�������*/
	m_tcpSocketConnectList.append(tcpsocket);
}

/*��������(���͵���Ϣ,������)*/
void TcpServer::onSocketDataProcessing(QByteArray& SendData, int descriptor) 
{
	for(int i = 0;i< m_tcpSocketConnectList.count();i++)
	{
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		if (item->socketDescriptor(/*Tcp�ӿ�������*/) == descriptor) 
		{
			qDebug() << QString::fromLocal8Bit("����IP:") << item->peerAddress(/*�Եȵ�ַ*/).toString() 
						<< QString::fromLocal8Bit("����������:") << QString(SendData);

			emit signalTcpMsgComes(SendData);/*tcp��Ϣ����*/
		}
	}
}

/*�Ͽ����Ӵ���(������[˭�Ͽ�����])*/
void TcpServer::onSocketDisconnected(int descriptor) 
{
	for (int i = 0; i < m_tcpSocketConnectList.count(); i++)
	{
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		int itemDescriptor = item->socketDescriptor();/*Tcp�ӿ�������*/

		if (itemDescriptor == descriptor || itemDescriptor == -1)
		{
			m_tcpSocketConnectList.removeAt(i);/*�Ƴ����Ͽ���socket�������Ƴ�*/
			item->deleteLater();/*��Դ����*/
			qDebug() << QString::fromLocal8Bit("TcpSocket�Ͽ�����:") << itemDescriptor << "\n";
			return;
		}
	}
}