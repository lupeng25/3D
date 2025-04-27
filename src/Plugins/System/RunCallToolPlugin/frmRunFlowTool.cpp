#include "frmRunFlowTool.h"
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


frmRunFlowTool::frmRunFlowTool(QWidget* parent)
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

}

frmRunFlowTool::~frmRunFlowTool()
{
	if (btnGroupRadio != nullptr)				delete btnGroupRadio;				btnGroupRadio	= nullptr;
	if (m_titleBar != nullptr)					delete m_titleBar;					m_titleBar		= nullptr;
	this->deleteLater();
}

void frmRunFlowTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Script.png");
}

int frmRunFlowTool::Execute(const QString toolname)
{

	return 0;
}

EnumNodeResult frmRunFlowTool::Execute(MiddleParam & param, QString & strError)
{
	return EnumNodeResult(NodeResult_Idle);
}

int frmRunFlowTool::Load()
{
	m_titleBar->setTitleContent(m_pTool->GetItemId());

	ui.cbx_ImgInPut->clear();
	for (auto iter : databaseVar::Instance().m_vecPtr)	//图像
	{
		if (!iter->m_bEnable)
		{
			ui.cbx_ImgInPut->addItem(iter->m_strName.second);
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((RunFlowTool*)m_pTool)->m_strFlow);

	return 0;
}

int frmRunFlowTool::Save()
{
	((RunFlowTool*)m_pTool)->m_strFlow = ui.cbx_ImgInPut->currentText();
	return 0;
}
