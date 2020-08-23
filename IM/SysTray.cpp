#include "SysTray.h"
#include "CustomMenu.h"

SysTray::SysTray(QWidget *parent)
	: m_parent(parent)
	,QSystemTrayIcon(parent)
{
	initSystemTray();
	show();
}

SysTray::~SysTray()
{
}

void SysTray::initSystemTray()
{
	// 设置提示（鼠标放到托盘图标上，会出提示）
	setToolTip(QStringLiteral("我是托盘图标"));

	// 设置图标
	setIcon(QIcon(":/Resources/MainWindow/app/logo.ico"));

	// 发送，图标被激活信号，括号里是触发的原因
	// 收到信号，触发系统托盘图标
	connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(onIconActivateed(QSystemTrayIcon::ActivationReason)));



}

void SysTray::addSystrayMenu()
{
	// 指定 m_parent 为父窗体
	CustomMenu* customMenu = new CustomMenu(m_parent);

	// 设置菜单
	customMenu->addCustomMenu("onShow", ":/Resources/MainWindow/app/logo.ico",
		QStringLiteral("显示"));
	customMenu->addCustomMenu("onQuit", ":/Resources/MainWindow/app/page_close_btn_hover.png",
		QStringLiteral("退出"));

	// 发射信号
	connect(customMenu->getAction("onShow"), SIGNAL(triggered(bool)),
		m_parent, SLOT(onShowNormal(bool)));
	connect(customMenu->getAction("onQuit"), SIGNAL(triggered(bool)),
		m_parent, SLOT(onShowQuit(bool)));

	// 让菜单进入事件循环，接受鼠标操作
	// exec，让菜单进入事件循环，跑起来
	// QCursor::pos()，跑的位置，当前光栅在全屏坐标系中的位置
	customMenu->exec(QCursor::pos());
	delete customMenu;				// 事件结束后，进行释放
	customMenu = nullptr;
}

void SysTray::onIconActivateed(QSystemTrayIcon::ActivationReason reason)
{
	// 如果触发
	// Trigger 就是点击触发
	if (reason == QSystemTrayIcon::Trigger)
	{
		// 就进行显示
		m_parent->show();							// 触发就显示
		m_parent->activateWindow();		// 同时把父窗体，作为活动窗体
	}
	// 如果触发的是 Context 图标
	else if (reason == QSystemTrayIcon::Context)
	{
		addSystrayMenu();			// 添加系统菜单
	}
}