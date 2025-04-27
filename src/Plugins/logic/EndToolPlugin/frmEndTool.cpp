#include "frmEndTool.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QThread>
#include <QDateTime>
#include <QElapsedTimer>
#include <QTextCursor>
#include "databaseVar.h"
//#include "gVariable.h"
#include "frmAllVariableTool.h"


frmEndTool::frmEndTool(QWidget* parent)
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
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/dark/end.png"));

	//初始化标题栏
	initTitleBar();	

	//选项按钮
	btnGroupRadio = new QButtonGroup(this);


}

frmEndTool::~frmEndTool()
{
	if (btnGroupRadio != nullptr)				delete btnGroupRadio;				btnGroupRadio	= nullptr;
	if (m_titleBar != nullptr)					delete m_titleBar;					m_titleBar		= nullptr;
	this->deleteLater();
}

void frmEndTool::initTitleBar()
{
	m_titleBar->move(0, 0);
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));

	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/end.png");
	//m_titleBar->setTitleContent(toolTitleName);
	//m_titleBar->setButtonType(ONLY_CLOSE_BUTTON);
	m_titleBar->setTitleWidth(this->width());
}

int frmEndTool::Execute(const QString toolname)
{
	return 0;
}

EnumNodeResult frmEndTool::Execute(MiddleParam & param, QString & strError)
{
	return EnumNodeResult();
}

int frmEndTool::Load()
{
	ui.spinBox_Recieve->setValue(((EndTool*)m_pTool)->m_iNumber);
	return 0;
}

int frmEndTool::Save()
{

	((EndTool*)m_pTool)->m_iNumber = ui.spinBox_Recieve->value();
	return 0;
}
