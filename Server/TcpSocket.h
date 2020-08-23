#pragma once

#include <QTcpSocket>

/*Tcp�׽���*/
class TcpSocket :  public QTcpSocket
{
	Q_OBJECT
public:
	TcpSocket();
	~TcpSocket();

	void run();

signals:
	/*�ӿͻ����յ����ݺ����źŸ���server������Ҫ����(����,������)*/
	void signalGetDataFromClient(QByteArray&, int);
	/*�пͻ��˶Ͽ�����(������)������Server*/
	void signalCilentDisconnect(int);

private slots:
	/*����readyRead�źŶ�ȡ������*/
	void onReceiveData();
	/*����ͻ��˶Ͽ�����*/
	void onClientDisconnect();
private:
	/*������������Ψһ��ʶ*/
	int m_descriptor;
};
