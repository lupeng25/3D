#include "RunFlowTool.h"
#include <qdebug.h>
#include "frmRunFlowTool.h"
#include "databaseVar.h"

RunFlowTool::RunFlowTool() :FlowchartTool()
{
	// GetItemId() = GetToolName();
	//this->setToolTip( GetItemId());
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
}

RunFlowTool::~RunFlowTool()
{
	Clearport();
}

int RunFlowTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_object = strData.find("ConFig").value().toArray();
	if (!array_object.empty())
	{
		m_strFlow = array_object.at(0).toString();
	}
	return 0;
}

int RunFlowTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	QJsonArray ConFig = {
		m_strFlow,
	};
	strData.insert("ConFig", ConFig);
	return 0;
}

void RunFlowTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmRunFlowTool * frm = new frmRunFlowTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->Save();
	delete frm;
}

EnumNodeResult RunFlowTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	int _iFindIndex = -1;
	for (size_t i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)
	{
		if (databaseVar::Instance().m_vecPtr[i]->m_strName.second == m_strFlow)
		{
			_iFindIndex = i;
		}
	}
	if (_iFindIndex != -1)
	{
		return	databaseVar::Instance().m_vecPtr[_iFindIndex]->m_pNodeManager->InitBeforeRun(param, strError);
	}
	else
	{
		SetError(QString("Flow %1 Didn't Exist").arg(m_strFlow)); return NodeResult_ParamError;
	}

	return NodeResult_OK;
}

EnumNodeResult RunFlowTool::PrExecute(MiddleParam & param, QString& strError)
{
	int _iFindIndex = -1;
	for (size_t i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)
	{
		if (databaseVar::Instance().m_vecPtr[i]->m_strName.second == m_strFlow)
		{
			_iFindIndex = i;
		}
	}
	if (_iFindIndex != -1)
	{
		return	databaseVar::Instance().m_vecPtr[_iFindIndex]->m_pNodeManager->PrExecute(param, strError);
	}
	else
	{
		SetError(QString("Flow %1 Didn't Exist").arg(m_strFlow)); return NodeResult_ParamError;
	}
	return NodeResult_OK;
}

EnumNodeResult RunFlowTool::Execute(MiddleParam& param, QString& strError)
{	
	int _iFindIndex = -1;
	for (size_t i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)
	{
		if (databaseVar::Instance().m_vecPtr[i]->m_strName.second == m_strFlow)
		{
			_iFindIndex = i;
		}
	}
	if (_iFindIndex != -1)	//执行流程不能重复
	{
		if (databaseVar::Instance().m_vecPtr[_iFindIndex]->m_bEnable)
		{
			SetError(QString("Flow %1 Is Enable").arg(m_strFlow)); return NodeResult_ParamError;
		}
		else
		{
			return	databaseVar::Instance().m_vecPtr[_iFindIndex]->m_pNodeManager->Execute(param, strError);
		}
	}
	else
	{
		SetError(QString("Flow %1 Didn't Exist").arg(m_strFlow)); return NodeResult_ParamError;
	}

	return NodeResult_OK;
}

QPair<QString, QString> RunFlowTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> RunFlowTool::GetToolName()
{
	return QPair<QString, QString>("Flow", tr("Flow"));
}
