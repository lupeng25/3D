#include "frmImageLayout.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QDesktopWidget>
#include <QPainter>
#include "Data/datavar.h"
#include "databaseVar.h"

frmImageLayout::frmImageLayout(QWidget* parent)
	: frmBase(parent)
{
	ui.setupUi(this);
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/appbar.layout.sidebar.png"));
	//初始化标题栏
	initTitleBar();		
	//选项按钮
	btnGroupRadio = new QButtonGroup(this);
	btnGroupRadio->addButton(ui.radioImage1, 0);		
	connect(ui.radioImage1, SIGNAL(toggled(bool)), this, SLOT(slot_Image(bool)));
	btnGroupRadio->addButton(ui.radioImage2, 1);	
	connect(ui.radioImage2, SIGNAL(toggled(bool)), this, SLOT(slot_Image(bool)));
	btnGroupRadio->addButton(ui.radioImage3, 2);	
	connect(ui.radioImage3, SIGNAL(toggled(bool)), this, SLOT(slot_Image(bool)));
	btnGroupRadio->addButton(ui.radioImage4, 3);	
	connect(ui.radioImage4, SIGNAL(toggled(bool)), this, SLOT(slot_Image(bool)));
	btnGroupRadio->addButton(ui.radioImage6A, 4);	
	connect(ui.radioImage6A, SIGNAL(toggled(bool)), this, SLOT(slot_Image(bool)));
	btnGroupRadio->addButton(ui.radioImage6B, 5);	
	connect(ui.radioImage6B, SIGNAL(toggled(bool)), this, SLOT(slot_Image(bool)));
	btnGroupRadio->addButton(ui.radioImage9, 6);	
	connect(ui.radioImage9, SIGNAL(toggled(bool)), this, SLOT(slot_Image(bool)));
	emit btnGroupRadio->buttonClicked(0);
	ui.radioImage1->setChecked(true);
}

frmImageLayout::~frmImageLayout()
{
	this->deleteLater();
}

int frmImageLayout::GetData(QJsonObject & strData)
{
	QJsonObject object_value;

	int	 _iIndex = btnGroupRadio->checkedId();
	object_value.insert("Name", QString::number(_iIndex));
	strData.insert("frmImageLayout", object_value);
	return 0;
}

int frmImageLayout::SetData(QJsonObject & strData)
{
	QJsonObject object_value	= strData.find("frmImageLayout").value().toObject();
	int _iIndex					= object_value.find("Name").value().toString().toInt();
	databaseVar::Instance().form_Layout_Number = _iIndex;
	switch (_iIndex)	{
	case 0:	ui.radioImage1->setChecked(true);  databaseVar::Instance().form_Layout_Number  = 1;	emit sig_ChangeVideoCount(1); break;
	case 1:	ui.radioImage2->setChecked(true);  databaseVar::Instance().form_Layout_Number  = 2;	emit sig_ChangeVideoCount(2); break;
	case 2:	ui.radioImage3->setChecked(true);  databaseVar::Instance().form_Layout_Number  = 3;	emit sig_ChangeVideoCount(3); break;
	case 3:	ui.radioImage4->setChecked(true);  databaseVar::Instance().form_Layout_Number  = 4;	emit sig_ChangeVideoCount(4); break;
	case 4:	ui.radioImage6A->setChecked(true); databaseVar::Instance().form_Layout_Number	= 5;	emit sig_ChangeVideoCount(5); break;
	case 5:	ui.radioImage6B->setChecked(true); databaseVar::Instance().form_Layout_Number	= 6;	emit sig_ChangeVideoCount(6); break;
	case 6:	ui.radioImage9->setChecked(true);  databaseVar::Instance().form_Layout_Number	= 9;	emit sig_ChangeVideoCount(9); break;
	default:
		break;
	}
	return 0;
}

int frmImageLayout::NewProject()
{
	ui.radioImage1->setChecked(true);
	slot_Image(true);
	return 0;
}

void frmImageLayout::initTitleBar()
{
	m_titleBar->move(0, 0);
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/appbar.layout.sidebar.png");
	m_titleBar->setTitleContent(tr("图像布局"));
	m_titleBar->setButtonType(MIN_MAX_BUTTON);
	m_titleBar->setTitleWidth(this->width());
}

void frmImageLayout::ChangeLanguage()
{
	frmBase::ChangeLanguage();
	m_titleBar->setTitleContent(tr("图像布局"));
	ui.retranslateUi(this);
}

void frmImageLayout::showEvent(QShowEvent * ev)
{
	frmBase::showEvent(ev);
	emit m_titleBar->getRestoreButton()->clicked();
}

void frmImageLayout::onButtonCloseClicked()
{
	this->close();
}

void frmImageLayout::slot_Image(bool state)
{		
	switch (btnGroupRadio->checkedId())
	{
	case 0:		
		if (state == true)
		{
			ui.radioImage1->setChecked(state); emit sig_ChangeVideoCount(1);
			databaseVar::Instance().Instance().form_Layout_Number		= 1;
		}		
		break;
	case 1:
		if (state == true)
		{
			ui.radioImage2->setChecked(state); emit sig_ChangeVideoCount(2);
			databaseVar::Instance().form_Layout_Number		= 2;
		}		
		break;
	case 2:
		if (state == true)
		{
			ui.radioImage3->setChecked(state); emit sig_ChangeVideoCount(3);
			databaseVar::Instance().form_Layout_Number		= 3;
		}	
		break;
	case 3:
		if (state == true)
		{
			ui.radioImage4->setChecked(state); emit sig_ChangeVideoCount(4);
			databaseVar::Instance().form_Layout_Number		= 4;
		}	
		break;
	case 4:
		if (state == true)
		{
			ui.radioImage6A->setChecked(state); emit sig_ChangeVideoCount(5);
			databaseVar::Instance().form_Layout_Number		= 5;
		}		
		break;
	case 5:
		if (state == true)
		{
			ui.radioImage6B->setChecked(state); emit sig_ChangeVideoCount(6);
			databaseVar::Instance().form_Layout_Number		= 6;
		}		
		break;
	case 6:
		if (state == true)
		{
			ui.radioImage9->setChecked(state); emit sig_ChangeVideoCount(9);
			databaseVar::Instance().form_Layout_Number		= 9;
		}		
		break;
	}
}
