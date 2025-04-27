#include "basemainwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QFile>
#include "mytitlebar.h"

#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

BaseWindow::BaseWindow(QWidget*parent)
	: QDialog(parent)
{
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//setWindowFlags(Qt::WindowMinimizeButtonHint);
	//setWindowFlags(windowFlags() & Qt::FramelessWindowHint);
	//setWindowFlags(windowFlags() & Qt::WindowMinimizeButtonHint);
	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);
	//初始化标题栏
	initTitleBar();
	ChangeLanguage();
}

BaseWindow::~BaseWindow()
{
	if(m_titleBar != nullptr) delete m_titleBar; m_titleBar = nullptr;
}

void BaseWindow::initTitleBar()
{
	m_titleBar = new MyTitleBar(this);
	m_titleBar->move(0, 0);	
	connect(m_titleBar, SIGNAL(signalButtonMinClicked()),		this, SLOT(onButtonMinClicked()));
	connect(m_titleBar, SIGNAL(signalButtonRestoreClicked()),	this, SLOT(onButtonRestoreClicked()));
	connect(m_titleBar, SIGNAL(signalButtonMaxClicked()),		this, SLOT(onButtonMaxClicked()));
	//connect(m_titleBar, SIGNAL(signalButtonCloseClicked()),		this, SLOT(onButtonCloseClicked()));
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()),		this, SLOT(animationClose()));

}

void BaseWindow::loadStyleSheet(const QString &sheetName)
{
	QFile file(":/Resource Files/" + sheetName + ".css");
	file.open(QFile::ReadOnly);
	if (file.isOpen())
	{
		QString styleSheet = this->styleSheet();
		styleSheet += QLatin1String(file.readAll());
		this->setStyleSheet(styleSheet);
	}
}

void BaseWindow::changeEvent(QEvent * ev)
{
	switch (ev->type())
	{
	case QEvent::LanguageChange:{
		ChangeLanguage();
	}	break;
	default:
		break;
	}
	QDialog::changeEvent(ev);
}

void BaseWindow::showEvent(QShowEvent *event)
{
	QDesktopWidget* desktopWidget = QApplication::desktop();
	QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
	animation->setDuration(200);

	int w = desktopWidget->availableGeometry().width();
	int h = desktopWidget->availableGeometry().height();
	if (parentWidget() != nullptr) {
		w = parentWidget()->width();
		h = parentWidget()->height();
	}
	animation->setStartValue(QRect(w * 0.5, h*0.5, 0, 0));
	animation->setEndValue(QRect(w / 2 - width() / 2,
		h / 2 - height() / 2,
		width(), height()));
	animation->start();

	QWidget::showEvent(event);
}

void BaseWindow::closeEvent(QCloseEvent *event)
{
	QWidget::closeEvent(event);
}

void BaseWindow::animationClose()
{
	QRect noraml = geometry();
	QRect closeRect = QRect(noraml.x(), noraml.y() + noraml.height() / 2, noraml.width(), 0);
	QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
	connect(animation, SIGNAL(finished()), this, SLOT(close()));
	animation->setDuration(500);
	animation->setStartValue(noraml);
	animation->setEndValue(closeRect);
	animation->start();
}


void BaseWindow::ChangeLanguage()
{
}

void BaseWindow::onButtonMinClicked()
{
	if (Qt::Tool == (windowFlags() & Qt::Tool))	{
		hide();   
	}
	else	{
		showMinimized();
	}
}

void BaseWindow::onButtonRestoreClicked()
{
	QPoint windowPos;
	QSize windowSize;
	if(m_titleBar != nullptr) m_titleBar->getRestoreInfo(windowPos, windowSize);
	this->setGeometry(QRect(windowPos, windowSize));
	//m_titleBar->move(0, 0);
}

void BaseWindow::onButtonMaxClicked()
{
	if (m_titleBar != nullptr) m_titleBar->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
	QRect desktopRect = QApplication::desktop()->availableGeometry();
	QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
	setGeometry(FactRect);
	//m_titleBar->move(0, 0);
}

void BaseWindow::onButtonCloseClicked()
{
	close();
}
