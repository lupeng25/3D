#include "frmflow.h"
#include <QDateTime>
#include "qmutex.h"
#include <qdebug.h>
#include <QTimer>
#include "Port.h"
#include "NodeManager.h"
#include "databaseVar.h"

frmflow::frmflow(QWidget* parent) :
	QWidget(parent)
{
	ui.setupUi(this);
	ClearAllTab();
}

frmflow::~frmflow()
{
	slot_BtnStop();
}

int frmflow::GetData(QJsonObject & strData)
{
	QJsonObject object_value;
	for (int i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)	{
		QJsonObject json;
		if (databaseVar::Instance().m_vecPtr[i]->GetGraphData(json) == 0)		{
			json.insert("Name", databaseVar::Instance().m_vecPtr[i]->m_strName.first);
			json.insert("ShowName", databaseVar::Instance().m_vecPtr[i]->m_strName.second);
			json.insert("Enable", QString::number(databaseVar::Instance().m_vecPtr[i]->m_bEnable));
			object_value.insert(QString::number(i), json);
		}
	}
	object_value.insert("Current", ui.stackedWidget->currentIndex());
	strData.insert("flow", object_value);
	return 0;
}

int frmflow::SetData(QJsonObject & strData)
{
	for (auto iter : MiddleParam::MapMiddle())	iter.ClearAll();
	MiddleParam::MapMiddle().clear();
	ClearAllTab();
	// 迭代器输出对象中的数据
	int	_iCount = 0;
	QJsonObject object_value = strData.find("flow").value().toObject();
	for (int i = 0; i < object_value.size(); i++)	{
		QJsonObject json = object_value.find(QString::number(i)).value().toObject();
		if (json.isEmpty())	continue;
		QString name = json.find("Name").value().toString();
		QString ShowName = json.find("ShowName").value().toString();
		int _Enable = json.find("Enable").value().toString().toInt();
		if (_iCount >= databaseVar::Instance().m_vecPtr.size())	{
			slot_AddFlow(name, ShowName);
		}
		databaseVar::Instance().m_vecPtr[_iCount]->m_bEnable = _Enable;
		databaseVar::Instance().m_vecPtr[_iCount]->SetGraphData(json);
		ui.stackedWidget->insertWidget(_iCount, (QWidget *)databaseVar::Instance().m_vecPtr[_iCount]);
		_iCount++;
	}
	ui.stackedWidget->setCurrentIndex(object_value.find("Current").value().toString().toInt());
	return 0;
}

int frmflow::NewProject()
{
	ClearAllTab();
	return 0;
}

void frmflow::ClearAllTab()
{
	for (int i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)	{
		if (databaseVar::Instance().m_vecPtr[i] != nullptr) {
			databaseVar::Instance().m_vecPtr[i]->m_pNodeManager->StopExecute();
			if(databaseVar::Instance().m_vecPtr[i]->m_pNodeManager)
				databaseVar::Instance().m_vecPtr[i]->m_pNodeManager->terminate();
		}
		delete databaseVar::Instance().m_vecPtr[i];
		databaseVar::Instance().m_vecPtr[i] = nullptr;
	}
	databaseVar::Instance().m_vecPtr.clear();
	for (int i = 0; i < ui.stackedWidget->count(); i++) {
		ui.stackedWidget->removeWidget(ui.stackedWidget->widget(0));
	}
}

void frmflow::slot_AddFlow(QString strKey, QString strShow)
{
	FlowGraphicsViews*	_ptrgraph = new FlowGraphicsViews(this);
	_ptrgraph->m_strName.first = strKey;
	_ptrgraph->m_strName.second = strShow;
	_ptrgraph->SetText(strShow);
	databaseVar::Instance().m_vecPtr.push_back(_ptrgraph);
	ui.stackedWidget->addWidget(_ptrgraph);
	connect(_ptrgraph, SIGNAL(sigAutoRunFinished()), this, SLOT(Status_Stop()));
	MiddleParam::MapMiddle().push_back(MiddleParam());
	UpDateTab();
}

void frmflow::slot_ChangeFlow(QString strKey)
{
	UpDateTab();
	for (int i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)	{
		if (databaseVar::Instance().m_vecPtr[i]->m_strName.first == strKey)		{
			ui.stackedWidget->setCurrentIndex(i);		break;
		}
	}
}

void frmflow::slot_ReNameFlow(QString strKey,QString strOld, QString strNew)
{
	UpDateTab();
	for (int i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)	{
		if (databaseVar::Instance().m_vecPtr[i]->m_strName.first == strKey)		{
			databaseVar::Instance().m_vecPtr[i]->SetText(strNew);
			databaseVar::Instance().m_vecPtr[i]->m_strName.second = strNew;	
			break;
		}
	}
}

void frmflow::slot_DeleteFlow(QString strKey)
{
	UpDateTab();
	int	_iIndex = 0;
	for (auto iter = databaseVar::Instance().m_vecPtr.begin(); iter != databaseVar::Instance().m_vecPtr.end(); )	{
		if (MiddleParam::MapMiddle().size() > _iIndex)		{
			MiddleParam::MapMiddle()[_iIndex].ClearAll();
			MiddleParam::MapMiddle().erase(MiddleParam::MapMiddle().begin() + _iIndex);
		}
		_iIndex++;
		if ((*iter)->m_strName.first == strKey)		{
			ui.stackedWidget->removeWidget((QWidget *)(iter));
			delete (*iter);
			(*iter) = nullptr;
			databaseVar::Instance().m_vecPtr.erase(iter);
			break;
		}
		else { iter++; }
	}
}

void frmflow::slot_SetFlowEnable(QString strFlow, bool Endble)
{
	UpDateTab();
	for (int i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)	{
		if (databaseVar::Instance().m_vecPtr[i]->m_strName.first == strFlow)		{
			databaseVar::Instance().m_vecPtr[i]->m_bEnable = Endble;	break;
		}
	}
}

void frmflow::Status_Stop()
{
	ui.btnRunOnce->setEnabled(true);
	ui.btnRunCycle->setEnabled(true);
	ui.btnStop->setEnabled(false);
	ui.stackedWidget->setEnabled(true);
	bool _bAllStop = true;

	for (auto iter : databaseVar::Instance().m_vecPtr)
		_bAllStop &= !iter->m_pNodeManager->m_bThreadRun;
	if (_bAllStop)	FlowchartGraphicsItem::m_bVisionThreadRun = false;

	QTimer::singleShot(100, this, SIGNAL(sig_AllCycleStop()));
}

void frmflow::on_btnStop_clicked()
{
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = true;
	QTimer::singleShot(100, this, SLOT(Status_Stop()));
	int _iIndex = ui.stackedWidget->currentIndex();
	if (_iIndex >= databaseVar::Instance().m_vecPtr.size())	return;
	FlowGraphicsViews*	_ptrgraph = databaseVar::Instance().m_vecPtr[_iIndex];
	if (_ptrgraph != nullptr) { _ptrgraph->Stop(); }
}

void frmflow::on_btnRunOnce_clicked()
{
	int _iIndex = ui.stackedWidget->currentIndex();
	if (_iIndex >= databaseVar::Instance().m_vecPtr.size() || _iIndex < 0)	return;
	ui.btnRunOnce->setEnabled(false);
	ui.btnRunCycle->setEnabled(false);
	ui.btnStop->setEnabled(true);
	ui.stackedWidget->setEnabled(false);

	FlowchartGraphicsItem::m_bVisionThreadRun = true;
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = false;
	FlowchartGraphicsItem::m_iVisionThreadRunTime = 1;

	FlowGraphicsViews*	_ptrgraph = databaseVar::Instance().m_vecPtr[_iIndex];
	if (_ptrgraph != nullptr) { _ptrgraph->Start(); return; }
	Status_Stop();
}

void frmflow::on_btnRunCycle_clicked()
{
	int _iIndex = ui.stackedWidget->currentIndex();
	if (_iIndex >= databaseVar::Instance().m_vecPtr.size() || _iIndex < 0)	return;
	ui.btnRunOnce->setEnabled(false);
	ui.btnRunCycle->setEnabled(false);
	ui.btnStop->setEnabled(true);
	ui.stackedWidget->setEnabled(false);

	FlowchartGraphicsItem::m_bVisionThreadRun = true;
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = false;
	FlowchartGraphicsItem::m_iVisionThreadRunTime = 0;

	FlowGraphicsViews*	_ptrgraph = databaseVar::Instance().m_vecPtr[_iIndex];
	if (_ptrgraph != nullptr) { _ptrgraph->Start(); return; }
	Status_Stop();
}

void frmflow::slot_BtnRunOnce()
{
	if (databaseVar::Instance().m_vecPtr.size() <= 0)	return;
	//运行一次
	ui.btnRunOnce->setEnabled(false);
	ui.btnRunCycle->setEnabled(false);
	ui.btnStop->setEnabled(false);
	ui.stackedWidget->setEnabled(false);

	FlowchartGraphicsItem::m_bVisionThreadRun = true;
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = false;
	FlowchartGraphicsItem::m_iVisionThreadRunTime = 1;

	bool _bAllStop = false;
	for (auto iter : databaseVar::Instance().m_vecPtr) { if (iter != nullptr) if (iter->m_bEnable) { iter->Start(); _bAllStop = true; } }
	if (!_bAllStop) { Status_Stop(); }
}

void frmflow::slot_BtnRunCycle()
{
	if (databaseVar::Instance().m_vecPtr.size() <= 0)	return;
	ui.btnRunOnce->setEnabled(false);
	ui.btnRunCycle->setEnabled(false);
	ui.btnStop->setEnabled(false);
	ui.stackedWidget->setEnabled(false);

	FlowchartGraphicsItem::m_bVisionThreadRun = true;
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = false;
	FlowchartGraphicsItem::m_iVisionThreadRunTime = 0;
	bool _bAllStop = false;
	for (auto iter : databaseVar::Instance().m_vecPtr) { if (iter != nullptr)  if (iter->m_bEnable) { iter->Start(); _bAllStop = true; } }
	if (!_bAllStop) { Status_Stop(); }
}

void frmflow::slot_BtnStop()
{
	FlowchartGraphicsItem::m_bVisionThreadRun = false;
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = true;
	for (auto iter : databaseVar::Instance().m_vecPtr) { if (iter != nullptr) { iter->Stop(); } }
	Status_Stop();
}

void frmflow::UpDateTab()
{
	for (size_t i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)	if (databaseVar::Instance().m_vecPtr[i] != nullptr) databaseVar::Instance().m_vecPtr[i]->UpdateIndex(i);
	for (size_t i = 0; i < MiddleParam::MapMiddle().size(); i++)	MiddleParam::MapMiddle()[i].m_iIndex = i;
}