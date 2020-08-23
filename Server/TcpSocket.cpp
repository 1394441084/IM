#include "TcpSocket.h"

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

void TcpSocket::run()
{
	//设置描述符
	m_descriptor = this->socketDescriptor();
	//*准备阅读,处理接收数据*/
	connect(this, SIGNAL(readyRead()), this, SLOT(onReceiveData()));
	//*断开连接,处理客户端断开连接*/
	connect(this, SIGNAL(disconnected()), this, SLOT(onClientDisconnect()));
}

//*处理readyRead信号读取的数据*/
void TcpSocket::onReceiveData()
{
	//*获取数据*/
	QByteArray buffer = this->readAll(/*阅读所有*/);
	if (!buffer.isEmpty())
	{
		QString strData = QString::fromLocal8Bit(buffer);
		//*发射接收到客户端数据的信号·告诉TcpServer*/
		emit signalGetDataFromClient(buffer, m_descriptor);
	}
}

//*处理客户端断开连接*/
void TcpSocket::onClientDisconnect()
{
	//*告诉Server有客户端断开连接*/
	emit signalCilentDisconnect(m_descriptor);
}
