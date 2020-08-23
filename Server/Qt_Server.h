#pragma once

#include <QtWidgets/QDialog>
#include "ui_QtQQ_Server.h"
#include "TcpServer.h"  //TCPһ��һ
#include <QSqlQueryModel>
#include <QTimer>
#include <QUdpSocket> //UDP�㲥

//QtQQ�ķ�������
class QtQQ_Server : public QDialog
{
    Q_OBJECT

public:
    QtQQ_Server(QWidget *parent = Q_NULLPTR);


public slots:
    /*����UDP�㲥*/
    void onUDPbroadMsg(QByteArray& btData);
    /*���±������*/
    void onRefresh();
    /*����Ⱥ����Ա��*/
    void on_queryDepartmentBtn_clicked();/*queryDepartmentBtn��ť�ĵ���ź���ۺ����Զ�����*/
    /*����Ա��QQ�Ų�ѯ*/
    void on_queryIDBtn_clicked();
    /*ע��Ա��QQ��*/
    void on_logoutBtn_clicked();
    /*ѡ��Ա������*/
    void on_selectPictureBtn_clicked();
    /*����Ա��*/
    void on_addBtn_clicked();
private:
    /*��ʼ��Tcp�����*/
    void initTcpSocket();
    /*�������ݿ�*/
    bool connectMySql();
    /*���±������*/
    void updataTableData(int depID=0,int employeeID= 0);
    /*��ȡ��˾ȺQQ��*/
    int getCompDepID();
    /*���ò�������ӳ��*/
    void setDepNameMap();
    /*��������ӳ��*/
    void setOnlineMap();
    /*����״̬ӳ��*/
    void setStatusMap();
    /*��ʼ����Ͽ������*/
    void initComboBoxData();
    /*��ʼ��UDP�����*/
    void initUdpSocket();
private:
    Ui::QtQQ_ServerClass ui;
    /*TcpServer�����*/
    TcpServer* m_tcpServer;
    /*��ѯ����Ա������Ϣģ��*/
    QSqlQueryModel m_queryInfoModel;
    /*��˾ȺQQ��*/
    int m_compDepID;
    /*״̬ӳ��*/
    QMap<QString, QString> m_statusMap;
    /*��������ӳ��*/
    QMap<QString, QString> m_depNameMap;
    /*����ӳ��*/
    QMap<QString, QString> m_onlineMap;
    /*��ʱˢ�±������*/
    QTimer* m_timer;
    /*����QQ��*/
    int m_depID;
    /*Ա��QQ��*/
    int m_employeeID;
    /*ͷ��·��*/
    QString m_pixPath;
    /*UDP�����*/
    QUdpSocket* m_udpSender;//UDP�㲥
};
