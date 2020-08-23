#pragma once

#include <QTcpServer> //Tcp����
#include <QList>

/*********Tcp������:************
*****�����ӵĿͻ��˽���ά��*****/
class TcpServer : public QTcpServer
{
	Q_OBJECT

public:
	//port�˿ں�
	TcpServer(int port);
	~TcpServer();

	//�����˿���û���� //��true
	bool run();

protected:
	/*�ͻ������µ�����ʱ //��д*************************
	void incomingConnection(int socketDescriptor);//��*/
	void incomingConnection(qintptr socketDescriptor);//��

signals:
	void signalTcpMsgComes(QByteArray&);
private slots:
	/*��������(���͵���Ϣ,������)*/
	void onSocketDataProcessing(QByteArray& SendData,int descriptor);
	/*�Ͽ����Ӵ���(������[˭�Ͽ�����])*/
	void onSocketDisconnected(int descriptor);
private:
	int m_prot;//�˿ں�
	/*tcp�ӿ���������*****************************  
	**�ͻ������ӳɹ�����ӵ�����,�����˾ͽ����Ƴ�*/
	QList<QTcpSocket*> m_tcpSocketConnectList;
};
