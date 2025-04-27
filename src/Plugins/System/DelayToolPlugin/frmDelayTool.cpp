#include "frmDelayTool.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QThread>
#include <QElapsedTimer>
#include "databaseVar.h"

frmDelayTool::frmDelayTool(QWidget* parent)
	: frmBaseTool(parent)
{
	ui.setupUi(GetCenterWidget());
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop		= QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/Script.png"));
	//初始化标题栏
	initTitleBar();	
	//选项按钮
	btnGroupRadio = new QButtonGroup(this);
	btnGroupRadio->addButton(ui.radioThread, 0);
	btnGroupRadio->addButton(ui.radioTime, 1);
	ui.radioThread->click();

}

frmDelayTool::~frmDelayTool()
{
	if (btnGroupRadio != nullptr)				delete btnGroupRadio;				btnGroupRadio	= nullptr;
	if (m_titleBar != nullptr)					delete m_titleBar;					m_titleBar		= nullptr;
	this->deleteLater();
}

void frmDelayTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Script.png");
	//m_titleBar->setButtonType(ONLY_CLOSE_BUTTON);
}

int frmDelayTool::Execute(const QString toolname)
{
	int	_iDelayTime = ui.spinDelayTime->value();
	switch (btnGroupRadio->checkedId())	{
	case 0:		//线程
	{
		QThread::msleep(_iDelayTime);
	}	break;
	default:	//延时
	{
		QElapsedTimer t;
		t.start();
		while (t.elapsed() < _iDelayTime);
	}	break;
	}
	return 0;
}

EnumNodeResult frmDelayTool::Execute(MiddleParam & param, QString & strError)
{
	int	_iDelayTime = ui.spinDelayTime->value();
	switch (btnGroupRadio->checkedId())
	{
	case 0:		//线程
	{
		QThread::msleep(_iDelayTime);
	}	break;
	default:	//延时
	{
		QElapsedTimer t;
		t.start();
		while (t.elapsed() < _iDelayTime);
	}	break;
	}
	return NodeResult_OK;
}

int frmDelayTool::Load()
{
	switch (((DelayTool*)m_pTool)->m_iDelayType){
	case 0:	{
		ui.radioThread->click();
		ui.spinDelayTime->setValue(((DelayTool*)m_pTool)->m_iDelayTime);

	}	break;
	default:	{
		ui.radioTime->click();		
		ui.spinDelayTime->setValue(((DelayTool*)m_pTool)->m_iDelayTime);
	}	break;
	}

	return 0;
}

int frmDelayTool::Save()
{
	((DelayTool*)m_pTool)->m_iDelayType = btnGroupRadio->checkedId();
	if (btnGroupRadio->checkedId() == 0)
	{
		((DelayTool*)m_pTool)->m_iDelayTime = ui.spinDelayTime->value();
	}
	else
	{
		((DelayTool*)m_pTool)->m_iDelayTime = ui.spinDelayTime->value();
	}

	return 0;
}
