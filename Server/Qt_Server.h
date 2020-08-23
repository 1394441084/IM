#pragma once

#include <QtWidgets/QDialog>
#include "ui_QtQQ_Server.h"
#include "TcpServer.h"  //TCP一对一
#include <QSqlQueryModel>
#include <QTimer>
#include <QUdpSocket> //UDP广播

//QtQQ的服务器端
class QtQQ_Server : public QDialog
{
    Q_OBJECT

public:
    QtQQ_Server(QWidget *parent = Q_NULLPTR);


public slots:
    /*进行UDP广播*/
    void onUDPbroadMsg(QByteArray& btData);
    /*更新表格数据*/
    void onRefresh();
    /*根据群查找员工*/
    void on_queryDepartmentBtn_clicked();/*queryDepartmentBtn按钮的点击信号与槽函数自动连接*/
    /*根据员工QQ号查询*/
    void on_queryIDBtn_clicked();
    /*注销员工QQ号*/
    void on_logoutBtn_clicked();
    /*选择员工寸照*/
    void on_selectPictureBtn_clicked();
    /*新增员工*/
    void on_addBtn_clicked();
private:
    /*初始化Tcp服务端*/
    void initTcpSocket();
    /*连接数据库*/
    bool connectMySql();
    /*更新表格数据*/
    void updataTableData(int depID=0,int employeeID= 0);
    /*获取公司群QQ号*/
    int getCompDepID();
    /*设置部门名称映射*/
    void setDepNameMap();
    /*设置在线映射*/
    void setOnlineMap();
    /*设置状态映射*/
    void setStatusMap();
    /*初始化组合框的数据*/
    void initComboBoxData();
    /*初始化UDP服务端*/
    void initUdpSocket();
private:
    Ui::QtQQ_ServerClass ui;
    /*TcpServer服务端*/
    TcpServer* m_tcpServer;
    /*查询所有员工的信息模型*/
    QSqlQueryModel m_queryInfoModel;
    /*公司群QQ号*/
    int m_compDepID;
    /*状态映射*/
    QMap<QString, QString> m_statusMap;
    /*部门名称映射*/
    QMap<QString, QString> m_depNameMap;
    /*在线映射*/
    QMap<QString, QString> m_onlineMap;
    /*定时刷新表格数据*/
    QTimer* m_timer;
    /*部门QQ号*/
    int m_depID;
    /*员工QQ号*/
    int m_employeeID;
    /*头像路径*/
    QString m_pixPath;
    /*UDP服务端*/
    QUdpSocket* m_udpSender;//UDP广播
};
