#include "frmForTool.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QThread>
#include <QDateTime>
#include <QElapsedTimer>
#include <QTextCursor>
#include "databaseVar.h"
#include "frmAllVariableTool.h"

frmForTool::frmForTool(QWidget* parent)
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
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/For.png"));

	//初始化标题栏
	initTitleBar();	
	initConnect();
	//选项按钮
	btnGroupRadio = new QButtonGroup(this);
}

frmForTool::~frmForTool()
{
	if (btnGroupRadio != nullptr)				delete btnGroupRadio;				btnGroupRadio	= nullptr;
	if (m_titleBar != nullptr)					delete m_titleBar;					m_titleBar		= nullptr;
	this->deleteLater();
}

void frmForTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/For.png");
}

void frmForTool::initConnect()
{
	connect(ui.btnLinkContent,		&QPushButton::pressed, this, &frmForTool::on_btnLinkContent_clicked);
	connect(ui.btnDelLinkContent,	&QPushButton::pressed, this, &frmForTool::on_btnDelLinkContent_clicked);
}

void frmForTool::on_btnLinkContent_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();
	frm->m_strModelIndex		= m_strModelIndex;
	frm->m_strTypeVariable		= m_strTypeVariable;
	frm->m_strValueName			= m_strValueName;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(this);
	frm->exec();
	frm->Save();

	if (frm->m_iRetn == 2)	{
		//关联变量
		m_strModelIndex			= frm->m_strModelIndex;
		m_strTypeVariable		= frm->m_strTypeVariable;
		m_strValueName			= frm->m_strValueName;

		ui.txtLinkContent->setText(QString("%1:%2:%3")
			.arg(m_strModelIndex)
			.arg(m_strTypeVariable)
			.arg(m_strValueName));
	}
	delete frm;
}

void frmForTool::on_btnDelLinkContent_clicked()
{
	ui.txtLinkContent->clear();
	m_strModelIndex.clear();
	m_strTypeVariable.clear();
	m_strValueName.clear();
}

int frmForTool::Execute(const QString toolname)
{
	return 0;
}

EnumNodeResult frmForTool::Execute(MiddleParam & param, QString & strError)
{
	return EnumNodeResult();
}

int frmForTool::Load()
{
	ui.txtLinkContent->setText(((ForTool*)m_pTool)->m_strSendContent);
	return 0;
}

int frmForTool::Save()
{
	((ForTool*)m_pTool)->m_strSendContent		= ui.txtLinkContent->text();
	return 0;
}

//void frmForTool::on_btnExecute_clicked()
//{		
//	ui.btnExecute->setEnabled(false);
//	QApplication::processEvents();
//	Execute();
//	ui.btnExecute->setEnabled(true);
//}