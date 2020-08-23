#include "BasicWindow.h"
#include "NotifyManager.h"

#include <QFile>
#include <QStyleOption>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <qdesktopwidget.h>
#include <QSqlQuery>
#include "CommonUtils.h"



// �ⲿȫ�ֱ�������½��ID
extern QString gLoginEmployeeID;

BasicWindow::BasicWindow(QWidget *parent)
	: QDialog(parent)
{
	// ��ȡĬ�ϵ���ɫֵ
	m_colorBackGround = CommonUtils::getDefaultSkinColor();

	// �����ô��ڷ��
	setWindowFlags(Qt::FramelessWindowHint);							// �ޱ߿�
	setAttribute(Qt::WA_TranslucentBackground,true);				// ͸��Ч��

	// �������Ƥ���źţ��޸�Ƥ��
	connect(NotifyManager::getInstance(), 
					SIGNAL(signalSkinChanged(const QColor&)),
					this, 
					SLOT(onSignalSkinChange(const QColor&)));
}

BasicWindow::~BasicWindow()
{
}

void BasicWindow::loadStyleSheet(const QString& sheetName)
{
	m_styleName = sheetName;
	QFile file(":/Resources/QSS/" + sheetName + ".css");
	file.open(QFile::ReadOnly);

	if (file.isOpen())
	{
		setStyleSheet("");
		QString qsstyleSheet = QLatin1String(file.readAll());			// ��������ȫ����ȡ����, ���浽 qstyleSheet�ַ�����

		// ��ȡ��ǰ�û���Ƥ��RGBֵ
		QString r = QString::number(m_colorBackGround.red());
		QString g = QString::number(m_colorBackGround.green());
		QString b = QString::number(m_colorBackGround.blue());

		// titleskin ����Ƥ������Ϊ true
		// �ײ���Ƥ�� bottomskin Ϊ true
		// rgba �� a ,��͸����
		qsstyleSheet += QString("QWidget[titleskin=true]\
												{background-color:rgb(%1,%2,%3);\
												border-top-left-radius:4px;}\
												QWidget[bottomskin=true]\
												{border-top:1px solid rgba(%1,%2,%3,100);\
												background-color:rgba(%1,%2,%3,50);\
												border-bottom-left-radius:4px;\
												border-bottom-right-radius:4px;}")
												.arg(r).arg(g).arg(b);
		// ������ʽ��
		setStyleSheet(qsstyleSheet);
	}

	file.close(); 

}

// src , ��Դ  ,   mask , ����, һ���յ�Բͷ��  ,   maskSize, �ߴ��С
QPixmap BasicWindow::getRoundImage(const QPixmap& src, QPixmap& mask, QSize maskSize)
{
	if (maskSize == QSize(0,0))
	{
		maskSize = mask.size();
	}
	else
	{	
		// Qt::KeepAspectRatio , ����ʱ, �������Դ�ľ�������
		// Qt::SmoothTransformation , ƽ����ʽ
		mask = mask.scaled(maskSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	// ����ת�����ͷ��
	// QImage::Format_ARGB32_Premultiplied ��ʽ, ��ȡ��ͷ���Ƚ�����
	QImage resultImage(maskSize, QImage::Format_ARGB32_Premultiplied);

	// CompositionMode , ͼƬ�ĵ���ģʽ
	QPainter painter(&resultImage);				// ����һ������, �� resultImage
	painter.setCompositionMode(QPainter::CompositionMode_Source);			// ����ͼƬ����ģʽ, ��Դ�ļ��� ���ƽ��в��� �� Source ��ԭͼƬ��ֱ�ӿ�������
	painter.fillRect(resultImage.rect(), Qt::transparent);										// ������, ���εĴ�С, ����ͼƬ�Ĵ�С , ͬʱ��͸���� transparent
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);	// ��SourceOver��ʽ, ���е���
	painter.drawPixmap(0, 0, mask);																		// �Կհ׵�Բ��ͼƬ , ���е���
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);		// ��SourceIn��ʽ, ���е���
	painter.drawPixmap(0, 0, src.scaled(maskSize,Qt::KeepAspectRatio, Qt::SmoothTransformation));		// �ڶ�Ŀ���QQͷ��, ���б������� 
	painter.end();

	return QPixmap::fromImage(resultImage);
}

void BasicWindow::initBackGroundColor()
{
	QStyleOption opt;
	opt.init(this);

	QPainter p(this);						// ��this�� "��ǰ"
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p , this);				// ͼԴ, ���
}

// ���໯����ʱ, ��Ҫ��д��ͼ�¼� ���ñ���ͼ
void BasicWindow::paintEvent(QPaintEvent* event)
{
	initBackGroundColor();
	QDialog::paintEvent(event);
}

void BasicWindow::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		// pos() , �¼�����ʱ, ����ڴ������Ͻǵ� ƫ������
		m_mousePressed = true;
		m_mousePoint = event->globalPos() - pos();		// ��ȥ ����ֵ
		event->accept();			// ����
	}
}

void BasicWindow::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mousePressed && (event->buttons() && Qt::LeftButton))
	{
		// globalPos() , ��ȡ��ǰ�¼�����ʱ, ���ȫ�ֵ�����ֵ
		move(event->globalPos() - m_mousePoint);
		event->accept();
	}
}

void BasicWindow::mouseReleaseEvent(QMouseEvent* event)
{
	m_mousePressed = false;
}


void BasicWindow::onShowClose(bool)
{
	close();
}

void BasicWindow::onShowMin(bool)
{
	showMinimized();
}

void BasicWindow::onShowHide(bool)
{
	hide();
}

void BasicWindow::onShowNormal(bool)
{
	show();									// ֱ����ʾ
	activateWindow();				// ����Ϊ��Ĵ���
}

void BasicWindow::onShowQuit(bool)
{
	// ���µ�½״̬Ϊ"����"
	QString strSqlStatus = QString("UPDATE tab_employees SET online = 1 WHERE employeeID = %1").arg(gLoginEmployeeID);
	QSqlQuery queryStatus(strSqlStatus);
	queryStatus.exec();

	QApplication::quit();
}

void BasicWindow::onSignalSkinChange(const QColor& color)
{
	// �޸���ɫ , ������ʽ��, ��������
	m_colorBackGround = color;
	loadStyleSheet(m_styleName);
}

void BasicWindow::onButtonMinClicked()
{
	// �жϵ�ǰ����,�ǲ��ǹ��� , 
	// ����ǵĻ� , �ͱ�ʾ ��ǰ���� �й��߷�� , �ͽ��� ����
	// ������� ,�� ������С��
	if (Qt::Tool == (windowFlags() & Qt::Tool))
	{
		hide();
	}
	else
	{
		showMinimized();
	}
}

void BasicWindow::onButtonRestoreClicked()
{
	QPoint windowPos;
	QSize windowSize;

	// ��ȡ�����λ�� , ��С��Ϣ
	m_titleBar->getRestoreInfo(windowPos, windowSize);
	setGeometry(QRect(windowPos, windowSize));
}

void BasicWindow::onButtonMaxClicked()
{
	// �ȱ��洰��֮ǰ�� λ�� , ��С�߶�,����
	m_titleBar->saveRestoreInfo(pos(), QSize(width(),height()));

	// desktopRect , �������
	QRect desktopRect = QApplication::desktop()->availableGeometry();			// ��ȡ������Ϣ

	// factRect , ʵ�ʾ���
	QRect factRect = QRect( desktopRect.x() - 3, desktopRect.y() - 3 , 
											desktopRect.width() + 6, desktopRect.height() + 6);
	// ���þ���
	setGeometry(factRect);
}

void BasicWindow::onButtonCloseClicked()
{
	close();
}

void BasicWindow::initTitleBar(ButtonType buttonType)
{
	m_titleBar = new TitleBar(this);
	m_titleBar->setButtonType(buttonType);
	m_titleBar->move(0, 0);

	connect(m_titleBar, SIGNAL(signalButtonMinClicked()), this, SLOT(onButtonMinClicked()));
	connect(m_titleBar, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonMinClicked()));
	connect(m_titleBar, SIGNAL(signalButtonMaxClicked()), this, SLOT(onButtonMinClicked()));
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonMinClicked()));
}

void BasicWindow::setTitleBarTitle(const QString& title, const QString& icon)
{
	m_titleBar->setTitleIcon(icon);
	m_titleBar->setTitleContent(title);
}