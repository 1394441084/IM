#pragma once

#include <QTcpSocket>

/*Tcp套接字*/
class TcpSocket :  public QTcpSocket
{
	Q_OBJECT
public:
	TcpSocket();
	~TcpSocket();

	void run();

signals:
	/*从客户端收到数据后发射信号告诉server有数据要处理(数据,描述符)*/
	void signalGetDataFromClient(QByteArray&, int);
	/*有客户端断开连接(描述符)·告诉Server*/
	void signalCilentDisconnect(int);

private slots:
	/*处理readyRead信号读取的数据*/
	void onReceiveData();
	/*处理客户端断开连接*/
	void onClientDisconnect();
private:
	/*描述符·用于唯一标识*/
	int m_descriptor;
};
