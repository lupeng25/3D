#include "DelayTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmDelayTool.h"
#include "databaseVar.h"

DelayTool::DelayTool() :FlowchartTool()
{
	// GetItemId() = GetToolName();
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
}

DelayTool::~DelayTool()
{
	//DeleteNodePort(0, Port::InStream);
	//DeleteNodePort(0, Port::OutStream);
}

int DelayTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_object = strData.find("ConFig").value().toArray();
	if (!array_object.empty())
	{
		m_iDelayType	= array_object.at(0).toString().toInt();
		m_iDelayTime	= array_object.at(1).toString().toInt();
	}
	return 0;
}

int DelayTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	QJsonArray ConFig = {
		QString::number(m_iDelayType),
		QString::number(m_iDelayTime),
	};
	strData.insert("ConFig", ConFig);
	return 0;
}

void DelayTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmDelayTool * frm = new frmDelayTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->Save();
	frm->m_pTool = nullptr;
	delete frm;
}

EnumNodeResult DelayTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult DelayTool::Execute(MiddleParam& param, QString& strError)
{
	switch (m_iDelayType)	{
	case 0: {	//œﬂ≥Ã
		QThread::msleep(m_iDelayTime);
	}	break;
	default: {	//—” ±
		QElapsedTimer t;
		t.start();
		while (t.elapsed() < m_iDelayTime);
	}	break;
	}
	return NodeResult_OK;
}

QPair<QString, QString> DelayTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> DelayTool::GetToolName()
{
	return QPair<QString, QString>("Delay", tr("Delay"));
}