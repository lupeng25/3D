#include "frmIFConditionTool.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QThread>
#include <QDateTime>
#include <QElapsedTimer>
#include <QTextCursor>
#include "databaseVar.h"
#include "gVariable.h"
#include "frmAllVariableTool.h"

frmIFConditionTool::frmIFConditionTool(QWidget* parent)
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
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/IF.png"));

	//初始化标题栏
	initTitleBar();	
	initConnect();
}

frmIFConditionTool::~frmIFConditionTool()
{
	if (m_titleBar != nullptr)				delete m_titleBar;			m_titleBar		= nullptr;
	this->deleteLater();
}

void frmIFConditionTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/IF.png");
}

void frmIFConditionTool::initConnect()
{
	connect(ui.btnLinkContent,					SIGNAL(pressed()),	this, SLOT(on_btnLinkContent_clicked()));
	connect(ui.btnDelLinkContent,				SIGNAL(pressed()),	this, SLOT(on_btnDelLinkContent_clicked()));

	connect(ui.btnLinkSecondContent,			SIGNAL(pressed()),	this, SLOT(on_btnLinkSecondContent_clicked()));
	connect(ui.btnDelLinkSecondContent,			SIGNAL(pressed()),	this, SLOT(on_btnDelLinkSecondContent_clicked()));
}

void frmIFConditionTool::on_btnLinkContent_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();
	frm->SetCurrentLinkContent( ui.txtLinkContent->text() );
	int _Type = EnumLimitType_Bool | EnumLimitType_Int | EnumLimitType_Dou | EnumLimitType_Str ;
	frm->SetLimitModel("", _Type);
	frm->Load();
	databaseVar::Instance().CenterMainWindow(this);
	frm->exec();
	frm->Save();
	if (frm->m_iRetn == 2)			{		//关联变量
		QString strModelIndex		= "";
		QString strTypeVariable		= "";
		QString strValueName		= "";
		frm->GetCurrentVariable(strModelIndex, strTypeVariable, strValueName);
		ui.txtLinkContent->setText(QString("%1:%2:%3").arg(strModelIndex).arg(strTypeVariable).arg(strValueName));
	}
	delete frm;
	UpdateOperator();
}

void frmIFConditionTool::on_btnDelLinkContent_clicked()
{
	ui.txtLinkContent->clear();
}

void frmIFConditionTool::on_btnLinkSecondContent_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();
	frm->SetCurrentLinkContent(ui.txtLinkSecondContent->text());
	QString strModelIndex		= "";
	QString strTypeVariable		= "";
	QString	strValue			= "";
	int _iLinkRetn				= IFTool::FindParamInLinkContent(ui.txtLinkContent->text(),			strModelIndex,	strTypeVariable, strValue);
	if (	_iLinkRetn		>=		0	)	{
		int _Type = EnumLimitType_NotAll;
		if (strTypeVariable == "Bool")					{	_Type = EnumLimitType_Bool | EnumLimitType_Int;						}
		else if (strTypeVariable == "Int")				{	_Type = EnumLimitType_Bool | EnumLimitType_Int | EnumLimitType_Dou;	}
		else if (strTypeVariable == "Double")			{	_Type = EnumLimitType_Bool | EnumLimitType_Int | EnumLimitType_Dou;	}
		else if (strTypeVariable == "QString")			{	_Type = _Type | EnumLimitType_Str;									}
		else if (strTypeVariable == "QPoint")			{	_Type = _Type | EnumLimitType_Point;								}
		else if (strTypeVariable == "QPointF")			{	_Type = _Type | EnumLimitType_PointF;								}
		else if (strTypeVariable == "sResultCross")		{	_Type = _Type | EnumLimitType_ResultPointF;							}
		else if (strTypeVariable == "Metrix")			{	_Type = _Type | EnumLimitType_Metrix;								}
		frm->SetLimitModel("", _Type);
	}

	frm->Load();
	databaseVar::Instance().CenterMainWindow(this);
	frm->exec();	frm->Save();
	if (frm->m_iRetn == 2)			{	//关联变量
		QString strModelIndex	= "";
		QString strTypeVariable = "";
		QString strValueName	= "";
		frm->GetCurrentVariable(strModelIndex, strTypeVariable, strValueName);
		ui.txtLinkSecondContent->setText(QString("%1:%2:%3").arg(strModelIndex).arg(strTypeVariable).arg(strValueName));
	}
	delete frm;
}

void frmIFConditionTool::on_btnDelLinkSecondContent_clicked()
{
	ui.txtLinkSecondContent->clear();
}

void frmIFConditionTool::UpdateOperator()
{	
	QString strModelIndex			= "";
	QString strTypeVariable			= "";
	QString	strValue				= "";
	int _iLinkRetn = IFTool::FindParamInLinkContent(ui.txtLinkContent->text(), strModelIndex, strTypeVariable, strValue);
	ui.cbx_Operator->clear();
	if (strTypeVariable == "Bool") {
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
	else if (strTypeVariable == "Int") {
		ui.cbx_Operator->addItem(tr("Greater-than"));
		ui.cbx_Operator->addItem(tr("less-than"));
		ui.cbx_Operator->addItem(tr("Greater-than Equal"));
		ui.cbx_Operator->addItem(tr("less-than Equal"));
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
	else if (strTypeVariable == "Double") {
		ui.cbx_Operator->addItem(tr("Greater-than"));
		ui.cbx_Operator->addItem(tr("less-than"));
		ui.cbx_Operator->addItem(tr("Greater-than Equal"));
		ui.cbx_Operator->addItem(tr("less-than Equal"));
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
	else if (strTypeVariable == "QString") {
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
	else if (strTypeVariable == "QPoint") {
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
	else if (strTypeVariable == "QPointF") {
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
	else if (strTypeVariable == "sResultCross") {
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
	else if (strTypeVariable == "Metrix") {
		ui.cbx_Operator->addItem(tr("Equal"));
		ui.cbx_Operator->addItem(tr("Not Equal"));
	}
}

int frmIFConditionTool::Execute(const QString toolname)
{
	QString strModelIndex			= "";
	QString strTypeVariable			= "";
	QString	strValue				= "";
	QString strSecondModelIndex		= "";
	QString strSecondTypeVariable	= "";
	QString	strSecondValue			= "";
	int iValue = 0;

	IFTool::ParamInExecute(ui.txtLinkContent->text(), ui.txtLinkSecondContent->text(),ui.cbx_Operator->currentIndex(), iValue );
	if (iValue == 1)
	{

	}
	return 0;
}

EnumNodeResult frmIFConditionTool::Execute(MiddleParam & param, QString & strError)
{
	QString strModelIndex = "";
	QString strTypeVariable = "";
	QString	strValue = "";
	QString strSecondModelIndex = "";
	QString strSecondTypeVariable = "";
	QString	strSecondValue = "";
	int iValue = 0;

	IFTool::ParamInExecute(ui.txtLinkContent->text(), ui.txtLinkSecondContent->text(), ui.cbx_Operator->currentIndex(), iValue);
	if (iValue == 1)
	{

	}

	return NodeResult_OK;
}

int frmIFConditionTool::Load()
{
	ui.txtLinkContent->setText(((IFTool*)m_pTool)->m_strSendContent);
	ui.txtLinkSecondContent->setText(((IFTool*)m_pTool)->m_strSecondContent);
	UpdateOperator();
	ui.cbx_Operator->setCurrentIndex(((IFTool*)m_pTool)->iLogicOperator);
	return 0;
}

int frmIFConditionTool::Save()
{
	((IFTool*)m_pTool)->m_strSendContent		= ui.txtLinkContent->text();
	((IFTool*)m_pTool)->m_strSecondContent		= ui.txtLinkSecondContent->text();
	((IFTool*)m_pTool)->iLogicOperator			= ui.cbx_Operator->currentIndex();

	return 0;
}
