#include "QtQQ_Server.h"
#include <QMovie>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QSqlRecord> /*记录*/
#include <QSqlQuery>
#include <QFileDialog>

/*Tcp端口号*/
const int gTcpPort = 8888;
/*UDP端口号*/
const int gUdpPort = 6666;

QtQQ_Server::QtQQ_Server(QWidget *parent)
    : QDialog(parent), m_pixPath("")
{
	
	if (!connectMySql())/*连接数据库失败*/
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("数据库连接失败!") );
		close();
	}
	m_queryInfoModel.setQuery("SELECT * FROM qtqq.tab_employees");

	ui.setupUi(this);

	/*给标签加个动图*/ 
	QMovie* movie = new QMovie(QString::fromLocal8Bit("1.gif"));
	QSize a = QSize(ui.label->width(), ui.label->height());
	movie->setScaledSize(a);
	ui.label->setMovie(movie);
	movie->start();/*可以搞个右键点击此标签显示一些好玩的信息*/

	/*设置部门名称映射*/setDepNameMap();
	/*设置在线映射*/setOnlineMap();
	/*设置状态映射*/setStatusMap();
	/*初始化组合框的数据*/initComboBoxData();

	/*初始化查询公司群所有员工信息*/
	m_employeeID = 0;
	m_depID = getCompDepID();
	m_compDepID = m_depID;

	/*更新表格数据*/updataTableData();

	/*定时刷新表格数据*/
	m_timer = new QTimer(this);
	m_timer->setInterval(200);
	m_timer->start();
	connect(m_timer, &QTimer::timeout, this, &QtQQ_Server::onRefresh);

    initTcpSocket();
	initUdpSocket();
}

/*进行UDP广播*/
void QtQQ_Server::onUDPbroadMsg(QByteArray& btData) 
{
	/*quint16=short*/
	for(quint16 port= gUdpPort; port < gUdpPort +200; port++)
	{
		/*写数据报(数据,数据大小,广播地址,端口)*/
		m_udpSender->writeDatagram(btData, btData.size(), QHostAddress::Broadcast, port);
	}
}

void QtQQ_Server::onRefresh()
{
	updataTableData(m_depID, m_employeeID);
}

/*queryDepartmentBtn按钮的点击信号与槽函数自动连接*/
void QtQQ_Server::on_queryDepartmentBtn_clicked()
{
	m_employeeID = 0;
	m_depID = ui.debartmentBox->currentData().toInt();
	updataTableData(m_depID);
}

/*queryIDBtn按钮的点击信号与槽函数自动连接*/
void QtQQ_Server::on_queryIDBtn_clicked()
{
	/*将debartmentBox组合框设置为公司群,不然选择的是人事部就不太好看,而且没那么方便*/
	ui.debartmentBox->setCurrentIndex(0);
	m_depID = m_compDepID;
	/*检测员工QQ号是否输入*/
	if (!ui.queryIDLineEdit->text().length())
	{
		QMessageBox::information(this,QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("请输入员工QQ号!"));
		ui.queryIDLineEdit->setFocus();
		return;
	}

	/*获取用户输入的QQ号*/
	int employeeID = ui.queryIDLineEdit->text().toInt();

	QSqlQuery queryInfo(QString("SELECT * FROM qtqq.tab_employees where employeeID = %1").arg(employeeID));
	queryInfo.exec();

	if(!queryInfo.next())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("请输入正确的员工QQ号!"));
		ui.queryIDLineEdit->setFocus();
		return;
	}
	else
	{
		m_employeeID = employeeID;
		ui.queryIDLineEdit->clear();
	}
}

/*注销员工QQ号*/
void QtQQ_Server::on_logoutBtn_clicked()
{
	/*将debartmentBox组合框设置为公司群,不然选择的是人事部就不太好看,而且没那么方便*/
	ui.debartmentBox->setCurrentIndex(0);
	/*检测员工QQ号是否输入*/
	if (!ui.logoutIDLineEdit->text().length())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("请输入员工QQ号!"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}

	/*获取用户输入的QQ号*/
	int employeeID = ui.logoutIDLineEdit->text().toInt();

	//QSqlQuery queryInfo(QString("SELECT employee_name FROM qtqq.tab_employees where employeeID = %1").arg(employeeID));
	QSqlQuery queryInfo(QString("SELECT employee_name FROM qtqq.tab_employees where employeeID = %1 and status != 0").arg(employeeID));
	queryInfo.exec();

	if (!queryInfo.next())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("请输入正确的员工QQ号!"));
		ui.logoutIDLineEdit->setFocus();
		return;
	}
	else
	{
		/*注销操作,更新数据库,将员工的状态status设置为0*/
		QSqlQuery sqlUpData(QString("UPDATE qtqq.tab_employees SET status = 0 WHERE employeeID = %1").arg(employeeID));
		sqlUpData.exec();

		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("员工 %1 的 QQ: %2 已被注销!")
			.arg(queryInfo.value(0).toString()).arg(employeeID));

		ui.logoutIDLineEdit->clear();
	}
}

/*选择员工寸照*/
void QtQQ_Server::on_selectPictureBtn_clicked()
{
	/*获取用户选择的头像路径*/
	m_pixPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择头像"), ".",
		"*.png;*.jpg;*.gif;;*.png;;*.jpg;;*.gif;;");

	if (!m_pixPath.length())/*用户取消了选择*/ return;

	/*获取文件后缀*/
	QString file_suffix;
	QFileInfo fileinfo(m_pixPath);/*QFileInfo文件信息*/
	file_suffix = fileinfo.suffix();

	/*获取图片与标签比例*/ /*按比例放置图片,以免拉伸不好看*/
	QPixmap pixmap(m_pixPath);
	qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
	qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();
	QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);

	if (file_suffix == "gif")
	{	/*添加GIF图片*/
		QMovie* movie = new QMovie(m_pixPath);
		movie->setScaledSize(size);/*将缩放的帧大小设置为size*/
		ui.headLabel->setMovie(movie);
		movie->start();
	}
	else 
	{
		ui.headLabel->setPixmap(pixmap.scaled(size));
	}
}

/*新增员工*/ /*升级优化·可以优化为QQ注册那样的随机数*/
void QtQQ_Server::on_addBtn_clicked()
{
	/*员工姓名*/
	QString strName = ui.nameLineEdit->text();

	/*检测员工姓名的输入*/
	if (!strName.length())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("请输入员工姓名!"));
		ui.nameLineEdit->setFocus();
		return;
	}

	/*检测员工是否选择了头像*/
	if (!m_pixPath.length()) 
	{
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("请选择员工头像的路径!"));
		return;
	}

	/*数据库插入新的员工数据*/

	/*获取员工QQ号*/
	QSqlQuery maxEmployeeID("SELECT MAX(employeeID) FROM qtqq.tab_employees");
	maxEmployeeID.exec();
	maxEmployeeID.next();

	/*员工QQ号*/
	int employeeID = maxEmployeeID.value(0).toInt()+1;
	/*员工部门QQ号*/
	int depID = ui.employeeDepBox->currentData().toInt();
	/*图片路径统一为 :/Resources/imagesss/M.jpg 而不是 D:/C++_source_code_Qt_VS/IT_QT/VS_QT_QQ/QtQQ_Server/1.gif */
	//m_pixPath.replace("/","\\\\\");/*  '/' 替换为 '\'  */
	/*升级优化·图片路径统一,放着先*/
	QSqlQuery insertSql(QString("insert into qtqq.tab_employees(departmentID,employeeID,employee_name,picture)\
		values(%1,%2,'%3','%4')").arg(depID).arg(employeeID).arg(strName).arg(m_pixPath));/*如果不是空字符串将执行*/
	insertSql.exec();

	QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("新增员工成功!"));
	/*将新增员工的关联控件恢复默认值*/
	m_pixPath = "";
	ui.headLabel->setText(QStringLiteral(" 员工寸照 "));
	ui.employeeDepBox->setCurrentIndex(0);
	ui.nameLineEdit->clear();
}

//初始化Tcp套接字
void QtQQ_Server::initTcpSocket()
{
    m_tcpServer = new TcpServer(gTcpPort);
    m_tcpServer->run();/*监听端口*/

    //*收到tcp客户端发来的信息,服务器端进行UDP广播*/
    connect(m_tcpServer, &TcpServer::signalTcpMsgComes, this, &QtQQ_Server::onUDPbroadMsg);
}

/*连接数据库*/
bool QtQQ_Server::connectMySql()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");//myysql数据库
	db.setHostName("127.0.0.1");//设置主机名
	//db.setHostName("localhost");//设置主机名
	db.setPort(3306);//设置端口
	db.setUserName("root");//设置登录名
	db.setPassword("zhou1zhou2");//设置密码
	db.setDatabaseName("qtqq");//设置数据库名

	if (db.open()) {
		return true;//数据打开成功
	}
	else {
		return false;//数据库打开失败
	}
}

/*更新表格数据()*/
void QtQQ_Server::updataTableData(int depID, int employeeID)
{
	ui.tableWidget->clear();/*清理*/

	if (depID && depID != m_compDepID)
	{/*查询部门*/
		m_queryInfoModel.setQuery(QString("SELECT * FROM qtqq.tab_employees where departmentID = %1").arg(depID));
	}/*可优化为三目运算,只是可读性变差,但里面注释了就是最佳选择*/
	else if (employeeID)
	{/*精确查找*/
		m_queryInfoModel.setQuery(QString("SELECT * FROM qtqq.tab_employees where employeeID = %1").arg(employeeID));
	}
	else
	{/*查找所有*/
		m_queryInfoModel.setQuery(QString("SELECT * FROM qtqq.tab_employees"));
	}

	int rows = m_queryInfoModel.rowCount();/*数据库查询得出的总行数*/
	int columns = m_queryInfoModel.columnCount();/*数据库查询得出的总列数*/
	QModelIndex index;/*通过模型索引提取m_queryInfoModel里的数据*/

	/*设置表格的行数,列数*/
	ui.tableWidget->setRowCount(rows);
	ui.tableWidget->setColumnCount(columns);
	/*设置表格表头*/
	QStringList headers;
	headers << QStringLiteral("部门") << QStringLiteral("工号") << QStringLiteral("员工姓名") << QStringLiteral("员工签名")
			<< QStringLiteral("员工状态") << QStringLiteral("员工头像") << QStringLiteral("在线状态");
	ui.tableWidget->setHorizontalHeaderLabels(headers);
	/*设置表格列等宽*/
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	/*表格添加数据*/
	for (int i = 0; i < rows; i++) 
	{
		for(int j = 0;j<columns;j++)
		{
			/*提取数据库得到的数据*/
			index = m_queryInfoModel.index(i, j);
			QString strData = m_queryInfoModel.data(index).toString();

			/*获取字段名称*/ /*当前行的记录*/
			QSqlRecord record = m_queryInfoModel.record(i);
			/*获取记录名称*/ /*列*/
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
				
			/*界面表格设置表格部件项目*/
			ui.tableWidget->setItem(i, j, new QTableWidgetItem(strData));
		}
	}

}

int QtQQ_Server::getCompDepID()
{
	QSqlQuery queryCompDepID(QString("SELECT departmentID FROM qtqq.tab_department where department_name = '%1'")
		.arg(QString::fromLocal8Bit("公司群")));
	queryCompDepID.exec();
	queryCompDepID.next();
	int comDepid = queryCompDepID.value(0).toInt();
	return comDepid;
}

/*设置部门名称映射*/
void QtQQ_Server::setDepNameMap()
{/*可优化从数据库取或者是不需要这么做*/
	m_depNameMap.insert(QStringLiteral("2001"), QStringLiteral("人事群"));
	m_depNameMap.insert(QStringLiteral("2002"), QStringLiteral("研发群"));
	m_depNameMap.insert(QStringLiteral("2003"), QStringLiteral("市场群"));
}

/*设置在线映射*/
void QtQQ_Server::setOnlineMap()
{
	m_onlineMap.insert(QStringLiteral("1"), QStringLiteral("离线"));
	m_onlineMap.insert(QStringLiteral("2"), QStringLiteral("在线"));
	m_onlineMap.insert(QStringLiteral("3"), QStringLiteral("隐身"));
	/*可优化从数据库取或者是不需要这么做,m_onlineMap,m_statusMap,m_depNameMap更改为hash结构就QMap<QString, QStringList>这样
	部门名称,在线,状态key,数据就从数据库取*/
}

/*设置状态映射*/
void QtQQ_Server::setStatusMap()
{/*可优化从数据库取或者是不需要这么做*/
	m_statusMap.insert(QStringLiteral("1"), QStringLiteral("有效"));
	m_statusMap.insert(QStringLiteral("0"), QStringLiteral("已注销"));
}

/*初始化组合框的数据*/
void QtQQ_Server::initComboBoxData()
{
	/*组合框项·项目文本*/
	QString itemText;
	/*获取公司总部门数*/
	QSqlQueryModel queryDepModel;
	queryDepModel.setQuery("SELECT * FROM qtqq.tab_department");
	/*公司总部门数*/  /*查询到的部门包含了公司群,所有部门总数应减去公司群*/
	int depCounts = queryDepModel.rowCount()-1;

	/*设置ui.employeeDepBox->setItemData()*/
	for (int i = 0; i < depCounts; i++) 
	{
		/*获取员工所属部门组合框其中项的文本*/
		itemText = ui.employeeDepBox->itemText(i);
		/*获取部门QQ号*/
		QSqlQuery queryDepID(QString("SELECT departmentID FROM qtqq.tab_department where department_name = '%1'").arg(itemText));
		queryDepID.exec();
		queryDepID.next();
		/*设置当前员工所属部门组合框的数据为相应的部门QQ号*/
		ui.employeeDepBox->setItemData(i, queryDepID.value(0).toInt());
	}

	/*设置ui.debartmentBox->setItemData()*/
	for (int i = 0; i < depCounts+1; i++) 
	{
		/*获取群组组合框其中项的文本*/
		itemText= ui.debartmentBox->itemText(i);
		/*获取群组QQ号*/
		QSqlQuery queryDepID(QString("SELECT departmentID FROM qtqq.tab_department where department_name = '%1'").arg(itemText));
		queryDepID.exec();
		queryDepID.next();
		/*设置群组组合框的数据为相应的部门QQ号*/
		ui.debartmentBox->setItemData(i, queryDepID.value(0).toInt());
	}

}

/*初始化UDP服务端*/
void QtQQ_Server::initUdpSocket()
{
	m_udpSender = new QUdpSocket(this);

}
