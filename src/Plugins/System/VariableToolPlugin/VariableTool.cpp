#include "VariableTool.h"
#include <qdebug.h>
#include "frmVariableTool.h"
#include "databaseVar.h"

VariableTool::VariableTool() :FlowchartTool()
{
	// GetItemId() = GetToolName();
	//this->setToolTip( GetItemId());
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));

	for (size_t i = 0; i < m_Variable.size(); i++)	m_Variable.removeFirst();

}

VariableTool::~VariableTool()
{
	Clearport();
}

int VariableTool::SetData(QJsonObject & strData)
{
	m_CurMiddleParam.ClearAll();
	m_Variable = strData.find("ConFig").value().toArray();
	frmVariableTool::SetData(m_Variable, m_CurMiddleParam);

	return 0;
}

int VariableTool::GetData(QJsonObject & strData)
{
	//scale
	strData.insert("ConFig", m_Variable);
	return 0;
}

void VariableTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmVariableTool * frm = new frmVariableTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->Save();
	m_CurMiddleParam.ClearAll();
	frmVariableTool::SetData(m_Variable, m_CurMiddleParam);
	delete frm;
}

EnumNodeResult VariableTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult VariableTool::PrExecute(MiddleParam & param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult VariableTool::Execute(MiddleParam& param, QString& strError)
{	
	//Ω¯––œ‘ æ
	m_CurMiddleParam.CloneTo(param);

	return NodeResult_OK;
}

QPair<QString, QString> VariableTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> VariableTool::GetToolName()
{
	return 	QPair<QString, QString>("Variable", tr("Variable"));
}
