#include "QtQQ_Server.h"
#include <QMovie>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QSqlRecord> /*��¼*/
#include <QSqlQuery>
#include <QFileDialog>

/*Tcp�˿ں�*/
const int gTcpPort = 8888;
/*UDP�˿ں�*/
const int gUdpPort = 6666;

QtQQ_Server::QtQQ_Server(QWidget *parent)
    : QDialog(parent), m_pixPath("")
{
	
	if (!connectMySql())/*�������ݿ�ʧ��*/
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"),QString::fromLocal8Bit("���ݿ�����ʧ��!") );
		close();
	}
	m_queryInfoModel.setQuery("SELECT * FROM qtqq.tab_employees");

	ui.setupUi(this);

	/*����ǩ�Ӹ���ͼ*/ 
	QMovie* movie = new QMovie(QString::fromLocal8Bit("1.gif"));
	QSize a = QSize(ui.label->width(), ui.label->height());
	movie->setScaledSize(a);
	ui.label->setMovie(movie);
	movie->start();/*���Ը���Ҽ�����˱�ǩ��ʾһЩ�������Ϣ*/

	/*���ò�������ӳ��*/setDepNameMap();
	/*��������ӳ��*/setOnlineMap();
	/*����״̬ӳ��*/setStatusMap();
	/*��ʼ����Ͽ������*/initComboBoxData();

	/*��ʼ����ѯ��˾Ⱥ����Ա����Ϣ*/
	m_employeeID = 0;
	m_depID = getCompDepID();
	m_compDepID = m_depID;

	/*���±������*/updataTableData();

	/*��ʱˢ�±������*/
	m_timer = new QTimer(this);
	m_timer->setInterval(200);
	m_timer->start();
	connect(m_timer, &QTimer::timeout, this, &QtQQ_Server::onRefresh);

    initTcpSocket();
	initUdpSocket();
}

/*����UDP�㲥*/
void QtQQ_Server::onUDPbroadMsg(QByteArray& btData) 
{
	/*quint16=short*/
	for(quint16 port= gUdpPort; port < gUdpPort +200; port++)
	{
		/*д���ݱ�(����,���ݴ�С,�㲥��ַ,�˿�)*/
		m_udpSender->writeDatagram(btData, btData.size(), QHostAddress::Broadcast, port);
	}
}

void QtQQ_Server::onRefresh()
{
	updataTableData(m_depID, m_employeeID);
}

/*queryDepartmentBtn��ť�ĵ���ź���ۺ����Զ�����*/
void QtQQ_Server::on_queryDepartmentBtn_clicked()
{
	m_employeeID = 0;
	m_depID = ui.debartmentBox->currentData().toInt();
	updataTableData(m_depID);
}

/*queryIDBtn��ť�ĵ���ź���ۺ����Զ�����*/
void QtQQ_Server::on_queryIDBtn_clicked()
{
	/*��debartmentBox��Ͽ�����Ϊ��˾Ⱥ,��Ȼѡ��������²��Ͳ�̫�ÿ�,����û��ô����*/
	ui.debartmentBox->setCurrentIndex(0);
	m_depID = m_compDepID;
	/*���Ա��QQ���Ƿ�����*/
	if (!ui.queryIDLineEdit->text().length())
	{
		QMessageBox::information(this,QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("������Ա��QQ��!"));
		ui.queryIDLineEdit->setFocus();
		return;
	}

	/*��ȡ�û������QQ��*/
	int employeeID = ui.queryIDLineEdit->text().toInt();

	QSqlQuery queryInfo(QString("SELECT * FROM qtqq.tab_employees where employeeID = %1").arg(employeeID));
	queryInfo.exec();

	if(!queryInfo.next())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("��������ȷ��Ա��QQ��!"));
		ui.queryIDLineEdit->setFocus();
		return;
	}
	else
	{
		m_employeeID = employeeID;
		ui.queryIDLineEdit->clear();
	}
}

/*ע��Ա��QQ��*/
void QtQQ_Server::on_logoutBtn_clicked()
{
	/*��debartmentBox��Ͽ�����Ϊ��˾Ⱥ,��Ȼѡ��������²��Ͳ�̫�ÿ�,����û��ô����*/
	ui.debartmentBox->setCurrentIndex(0);
	/*���Ա��QQ���Ƿ�����*/
	if (!ui.logoutIDLineEdit->text().length())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("������Ա��QQ��!"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}

	/*��ȡ�û������QQ��*/
	int employeeID = ui.logoutIDLineEdit->text().toInt();

	//QSqlQuery queryInfo(QString("SELECT employee_name FROM qtqq.tab_employees where employeeID = %1").arg(employeeID));
	QSqlQuery queryInfo(QString("SELECT employee_name FROM qtqq.tab_employees where employeeID = %1 and status != 0").arg(employeeID));
	queryInfo.exec();

	if (!queryInfo.next())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("��������ȷ��Ա��QQ��!"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}
	else
	{
		/*ע������,�������ݿ�,��Ա����״̬status����Ϊ0*/
		QSqlQuery sqlUpData(QString("UPDATE qtqq.tab_employees SET status = 0 WHERE employeeID = %1").arg(employeeID));
		sqlUpData.exec();

		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("Ա�� %1 �� QQ: %2 �ѱ�ע��!")
			.arg(queryInfo.value(0).toString()).arg(employeeID));

		ui.logoutIDLineEdit->clear();
	}
}

/*ѡ��Ա������*/
void QtQQ_Server::on_selectPictureBtn_clicked()
{
	/*��ȡ�û�ѡ���ͷ��·��*/
	m_pixPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("ѡ��ͷ��"), ".",
		"*.png;*.jpg;*.gif;;*.png;;*.jpg;;*.gif;;");

	if (!m_pixPath.length())/*�û�ȡ����ѡ��*/ return;

	/*��ȡ�ļ���׺*/
	QString file_suffix;
	QFileInfo fileinfo(m_pixPath);/*QFileInfo�ļ���Ϣ*/
	file_suffix = fileinfo.suffix();

	/*��ȡͼƬ���ǩ����*/ /*����������ͼƬ,�������첻�ÿ�*/
	QPixmap pixmap(m_pixPath);
	qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
	qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();
	QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);

	if (file_suffix == "gif")
	{	/*���GIFͼƬ*/
		QMovie* movie = new QMovie(m_pixPath);
		movie->setScaledSize(size);/*�����ŵ�֡��С����Ϊsize*/
		ui.headLabel->setMovie(movie);
		movie->start();
	}
	else 
	{
		ui.headLabel->setPixmap(pixmap.scaled(size));
	}
}

/*����Ա��*/ /*�����Ż��������Ż�ΪQQע�������������*/
void QtQQ_Server::on_addBtn_clicked()
{
	/*Ա������*/
	QString strName = ui.nameLineEdit->text();

	/*���Ա������������*/
	if (!strName.length())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("������Ա������!"));
		ui.nameLineEdit->setFocus();
		return;
	}

	/*���Ա���Ƿ�ѡ����ͷ��*/
	if (!m_pixPath.length()) 
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("��ѡ��Ա��ͷ���·��!"));
		return;
	}

	/*���ݿ�����µ�Ա������*/

	/*��ȡԱ��QQ��*/
	QSqlQuery maxEmployeeID("SELECT MAX(employeeID) FROM qtqq.tab_employees");
	maxEmployeeID.exec();
	maxEmployeeID.next();

	/*Ա��QQ��*/
	int employeeID = maxEmployeeID.value(0).toInt()+1;
	/*Ա������QQ��*/
	int depID = ui.employeeDepBox->currentData().toInt();
	/*ͼƬ·��ͳһΪ :/Resources/imagesss/M.jpg ������ D:/C++_source_code_Qt_VS/IT_QT/VS_QT_QQ/QtQQ_Server/1.gif */
	//m_pixPath.replace("/","\\\\\");/*  '/' �滻Ϊ '\'  */
	/*�����Ż���ͼƬ·��ͳһ,������*/
	QSqlQuery insertSql(QString("insert into qtqq.tab_employees(departmentID,employeeID,employee_name,picture)\
		values(%1,%2,'%3','%4')").arg(depID).arg(employeeID).arg(strName).arg(m_pixPath));/*������ǿ��ַ�����ִ��*/
	insertSql.exec();

	QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����Ա���ɹ�!"));
	/*������Ա���Ĺ����ؼ��ָ�Ĭ��ֵ*/
	m_pixPath = "";
	ui.headLabel->setText(QStringLiteral(" Ա������ "));
	ui.employeeDepBox->setCurrentIndex(0);
	ui.nameLineEdit->clear();
}

//��ʼ��Tcp�׽���
void QtQQ_Server::initTcpSocket()
{
    m_tcpServer = new TcpServer(gTcpPort);
    m_tcpServer->run();/*�����˿�*/

    //*�յ�tcp�ͻ��˷�������Ϣ,�������˽���UDP�㲥*/
    connect(m_tcpServer, &TcpServer::signalTcpMsgComes, this, &QtQQ_Server::onUDPbroadMsg);
}

/*�������ݿ�*/
bool QtQQ_Server::connectMySql()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");//myysql���ݿ�
	db.setHostName("127.0.0.1");//����������
	//db.setHostName("localhost");//����������
	db.setPort(3306);//���ö˿�
	db.setUserName("root");//���õ�¼��
	db.setPassword("zhou1zhou2");//��������
	db.setDatabaseName("qtqq");//�������ݿ���

	if (db.open()) {
		return true;//���ݴ򿪳ɹ�
	}
	else {
		return false;//���ݿ��ʧ��
	}
}

/*���±������()*/
void QtQQ_Server::updataTableData(int depID, int employeeID)
{
	ui.tableWidget->clear();/*����*/

	if (depID && depID != m_compDepID)
	{/*��ѯ����*/
		m_queryInfoModel.setQuery(QString("SELECT * FROM qtqq.tab_employees where departmentID = %1").arg(depID));
	}/*���Ż�Ϊ��Ŀ����,ֻ�ǿɶ��Ա��,������ע���˾������ѡ��*/
	else if (employeeID)
	{/*��ȷ����*/
		m_queryInfoModel.setQuery(QString("SELECT * FROM qtqq.tab_employees where employeeID = %1").arg(employeeID));
	}
	else
	{/*��������*/
		m_queryInfoModel.setQuery(QString("SELECT * FROM qtqq.tab_employees"));
	}

	int rows = m_queryInfoModel.rowCount();/*���ݿ��ѯ�ó���������*/
	int columns = m_queryInfoModel.columnCount();/*���ݿ��ѯ�ó���������*/
	QModelIndex index;/*ͨ��ģ��������ȡm_queryInfoModel�������*/

	/*���ñ�������,����*/
	ui.tableWidget->setRowCount(rows);
	ui.tableWidget->setColumnCount(columns);
	/*���ñ���ͷ*/
	QStringList headers;
	headers << QStringLiteral("����") << QStringLiteral("����") << QStringLiteral("Ա������") << QStringLiteral("Ա��ǩ��")
			<< QStringLiteral("Ա��״̬") << QStringLiteral("Ա��ͷ��") << QStringLiteral("����״̬");
	ui.tableWidget->setHorizontalHeaderLabels(headers);
	/*���ñ���еȿ�*/
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	/*����������*/
	for (int i = 0; i < rows; i++) 
	{
		for(int j = 0;j<columns;j++)
		{
			/*��ȡ���ݿ�õ�������*/
			index = m_queryInfoModel.index(i, j);
			QString strData = m_queryInfoModel.data(index).toString();

			/*��ȡ�ֶ�����*/ /*��ǰ�еļ�¼*/
			QSqlRecord record = m_queryInfoModel.record(i);
			/*��ȡ��¼����*/ /*��*/
			QString strRecordName = record.fieldName(j);
			if (strRecordName == QLatin1String("departmentID"))
			{
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_depNameMap.value(strData)));
				continue;
			}
			else if (strRecordName == QLatin1String("status"))
			{
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_statusMap.value(strData)));
				continue;
			}
			else if (strRecordName == QLatin1String("online"))
			{
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_onlineMap.value(strData)));
				continue;
			}
				
			/*���������ñ�񲿼���Ŀ*/
			ui.tableWidget->setItem(i, j, new QTableWidgetItem(strData));
		}
	}

}

int QtQQ_Server::getCompDepID()
{
	QSqlQuery queryCompDepID(QString("SELECT departmentID FROM qtqq.tab_department where department_name = '%1'")
		.arg(QString::fromLocal8Bit("��˾Ⱥ")));
	queryCompDepID.exec();
	queryCompDepID.next();
	int comDepid = queryCompDepID.value(0).toInt();
	return comDepid;
}

/*���ò�������ӳ��*/
void QtQQ_Server::setDepNameMap()
{/*���Ż������ݿ�ȡ�����ǲ���Ҫ��ô��*/
	m_depNameMap.insert(QStringLiteral("2001"), QStringLiteral("����Ⱥ"));
	m_depNameMap.insert(QStringLiteral("2002"), QStringLiteral("�з�Ⱥ"));
	m_depNameMap.insert(QStringLiteral("2003"), QStringLiteral("�г�Ⱥ"));
}

/*��������ӳ��*/
void QtQQ_Server::setOnlineMap()
{
	m_onlineMap.insert(QStringLiteral("1"), QStringLiteral("����"));
	m_onlineMap.insert(QStringLiteral("2"), QStringLiteral("����"));
	m_onlineMap.insert(QStringLiteral("3"), QStringLiteral("����"));
	/*���Ż������ݿ�ȡ�����ǲ���Ҫ��ô��,m_onlineMap,m_statusMap,m_depNameMap����Ϊhash�ṹ��QMap<QString, QStringList>����
	��������,����,״̬key,���ݾʹ����ݿ�ȡ*/
}

/*����״̬ӳ��*/
void QtQQ_Server::setStatusMap()
{/*���Ż������ݿ�ȡ�����ǲ���Ҫ��ô��*/
	m_statusMap.insert(QStringLiteral("1"), QStringLiteral("��Ч"));
	m_statusMap.insert(QStringLiteral("0"), QStringLiteral("��ע��"));
}

/*��ʼ����Ͽ������*/
void QtQQ_Server::initComboBoxData()
{
	/*��Ͽ����Ŀ�ı�*/
	QString itemText;
	/*��ȡ��˾�ܲ�����*/
	QSqlQueryModel queryDepModel;
	queryDepModel.setQuery("SELECT * FROM qtqq.tab_department");
	/*��˾�ܲ�����*/  /*��ѯ���Ĳ��Ű����˹�˾Ⱥ,���в�������Ӧ��ȥ��˾Ⱥ*/
	int depCounts = queryDepModel.rowCount()-1;

	/*����ui.employeeDepBox->setItemData()*/
	for (int i = 0; i < depCounts; i++) 
	{
		/*��ȡԱ������������Ͽ���������ı�*/
		itemText = ui.employeeDepBox->itemText(i);
		/*��ȡ����QQ��*/
		QSqlQuery queryDepID(QString("SELECT departmentID FROM qtqq.tab_department where department_name = '%1'").arg(itemText));
		queryDepID.exec();
		queryDepID.next();
		/*���õ�ǰԱ������������Ͽ������Ϊ��Ӧ�Ĳ���QQ��*/
		ui.employeeDepBox->setItemData(i, queryDepID.value(0).toInt());
	}

	/*����ui.debartmentBox->setItemData()*/
	for (int i = 0; i < depCounts+1; i++) 
	{
		/*��ȡȺ����Ͽ���������ı�*/
		itemText= ui.debartmentBox->itemText(i);
		/*��ȡȺ��QQ��*/
		QSqlQuery queryDepID(QString("SELECT departmentID FROM qtqq.tab_department where department_name = '%1'").arg(itemText));
		queryDepID.exec();
		queryDepID.next();
		/*����Ⱥ����Ͽ������Ϊ��Ӧ�Ĳ���QQ��*/
		ui.debartmentBox->setItemData(i, queryDepID.value(0).toInt());
	}

}

/*��ʼ��UDP�����*/
void QtQQ_Server::initUdpSocket()
{
	m_udpSender = new QUdpSocket(this);

}
