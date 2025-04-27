/****************************************************************************
**
** Copyright (C) 2014 dragondjf
**
** QFramer is a frame based on Qt5.3, you will be more efficient with it. 
** As an Qter, Qt give us a nice coding experience. With user interactive experience(UE) 
** become more and more important in modern software, deveployers should consider business and UE.
** So, QFramer is born. QFramer's goal is to be a mature solution 
** which you only need to be focus on your business but UE for all Qters.
**
** Version	: 0.2.5.0
** Author	: dragondjf
** Website	: https://github.com/dragondjf
** Project	: https://github.com/dragondjf/QCFramer
** Blog		: http://my.oschina.net/dragondjf/home/?ft=atme
** Wiki		: https://github.com/dragondjf/QCFramer/wiki
** Lincence: LGPL V2
** QQ: 465398889
** Email: dragondjf@gmail.com, ding465398889@163.com, 465398889@qq.com
** 
****************************************************************************/

#include "uiManuDebug.h"
//#include "ftablewidget.h"
#include "QFramer/ftabwidget.h"
#include "QFramer/fnofocusdelegate.h"
#include "QFramer/futil.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QTreeWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QHeaderView>
#include <QProgressBar>
#include <QTimer>
#include <QGraphicsScene>

uiManuDebug::uiManuDebug(QWidget *parent) :
    FTabWidget(parent)
{
    initUI();
}

bool uiManuDebug::CheckPageAvalible()
{
	return true;
}

int uiManuDebug::CloseUI()
{
	//if (m_pQtVisionWidgets != nullptr)	m_pQtVisionWidgets->CloseUI();
	//if (m_pQtProductWidgets != nullptr)	m_pQtProductWidgets->CloseUI();
	if (m_pQFlowChart != nullptr)		m_pQFlowChart->CloseUI();
	//if (m_pQtMotionIOWidgets != nullptr)m_pQtMotionIOWidgets->CloseUI();
	//if (m_pQtSCrewWidgets != nullptr)	m_pQtSCrewWidgets->CloseUI();
	//if (m_pQtOtherWidgets != nullptr)	m_pQtOtherWidgets->CloseUI();
	//if (m_pQtMesWidgets != nullptr)		m_pQtMesWidgets->CloseUI();
	return 0;
}

int uiManuDebug::GetData(QJsonObject & strData)
{
	for (auto& iter : m_vecpWidget)
	{
		iter->GetData(strData);
	}
	return 0;
}

int uiManuDebug::SetData(QJsonObject & strData)
{
	for (auto& iter : m_vecpWidget)
	{
		iter->SetData(strData);
	}
	return 0;
}

int uiManuDebug::NewProject()
{
	for (auto& iter : m_vecpWidget)
	{
		iter->NewProject();
	}
	return 0;
}

int uiManuDebug::GetSystemData(QJsonObject & strData)
{
	for (auto& iter : m_vecpWidget)
	{
		iter->GetSystemData(strData);
	}
	return 0;
}

int uiManuDebug::SetSystemData(QJsonObject & strData)
{
	for (auto& iter : m_vecpWidget)
	{
		iter->SetSystemData(strData);
	}
	return 0;
}

int uiManuDebug::SCanTimerUI()
{
	for (auto& iter : m_vecpWidget)
	{
		iter->SCanTimerUI();
	}
	return 0;
}

void uiManuDebug::initUI()
{
	//m_pQtVisionWidgets		= new QtVisionWidgets(this);
	//m_pQtProductWidgets		= new QtProductWidgets(this);
	m_pQFlowChart			= new QFlowChart(this);
	//m_pQtMotionIOWidgets	= new QtMotionIOWidgets(this);
	//m_pQtSCrewWidgets		= new QtSCrewWidgets(this);
	//m_pQtMesWidgets			= new QtMesWidgets(this);
	//m_pQtOtherWidgets		= new QtOtherWidgets(this);
	//m_pQtSaveWidgets		= new QtSaveWidgets(this);
	//mainLayout->addWidget(m_pQtSaveWidgets);
	mainLayout->setStretch(0,15);
	mainLayout->setStretch(1, 1);
	addWidget(tr("FlowChart"), "FlowChart", m_pQFlowChart);
	//addWidget(tr("Vision"),		"Vision",	m_pQtVisionWidgets);
	//addWidget(tr("Product"),	"Product",	m_pQtProductWidgets);
	//addWidget(tr("MotionIO"),	"MotionIO", m_pQtMotionIOWidgets);
	//addWidget(tr("Mes"),		"Mes",		m_pQtMesWidgets);

    getButtons().at(0)->click();
}

int	uiManuDebug::initData(QString& strError)
{
	//if (m_pQtVisionWidgets != nullptr)	if (m_pQtVisionWidgets->initData(strError) != 0)	return -1;
	//if (m_pQtProductWidgets != nullptr)	if (m_pQtProductWidgets->initData(strError) != 0)	return -1;
	//if (m_pQtMotionIOWidgets != nullptr)if (m_pQtMotionIOWidgets->initData(strError) != 0)	return -1;
	if (m_pQFlowChart != nullptr)		if (m_pQFlowChart->initData(strError) != 0)			return -1;
	//if (m_pQtMesWidgets != nullptr)		if (m_pQtMesWidgets->initData(strError) != 0)		return -1;
	//if (m_pQtOtherWidgets != nullptr)	if (m_pQtOtherWidgets->initData(strError) != 0)		return -1;
	//if (m_pQtFeederWidgets != nullptr)	if (m_pQtFeederWidgets->initData(strError) != 0)	return -1;

	//if (m_pQtProductWidgets != nullptr)	m_pQtProductWidgets->initData();
	//if (m_pQtMotionIOWidgets != nullptr)	m_pQtMotionIOWidgets->initData();
	//if (m_pQtSCrewWidgets != nullptr)		m_pQtSCrewWidgets->initData();
	//if (m_pQtMesWidgets != nullptr)		m_pQtMesWidgets->initData();
	return 0;
}
