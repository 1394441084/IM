#include "TcpSocket.h"

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

void TcpSocket::run()
{
	//����������
	m_descriptor = this->socketDescriptor();
	//*׼���Ķ�,�����������*/
	connect(this, SIGNAL(readyRead()), this, SLOT(onReceiveData()));
	//*�Ͽ�����,����ͻ��˶Ͽ�����*/
	connect(this, SIGNAL(disconnected()), this, SLOT(onClientDisconnect()));
}

//*����readyRead�źŶ�ȡ������*/
void TcpSocket::onReceiveData()
{
	//*��ȡ����*/
	QByteArray buffer = this->readAll(/*�Ķ�����*/);
	if (!buffer.isEmpty())
	{
		QString strData = QString::fromLocal8Bit(buffer);
		//*������յ��ͻ������ݵ��źš�����TcpServer*/
		emit signalGetDataFromClient(buffer, m_descriptor);
	}
}

//*����ͻ��˶Ͽ�����*/
void TcpSocket::onClientDisconnect()
{
	//*����Server�пͻ��˶Ͽ�����*/
	emit signalCilentDisconnect(m_descriptor);
}
