#include "ScaleTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmScaleTool.h"
#include "databaseVar.h"

ScaleTool::ScaleTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
}

ScaleTool::~ScaleTool()
{
}

int ScaleTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	m_vecScale.clear();
	QJsonArray array_ScaleTool = strData.find("ScaleTool").value().toArray();
	for (size_t i = 0; i < array_ScaleTool.count(); i++)	{
		sScale _ScaleData;
		QJsonArray array_value = array_ScaleTool.at(i).toArray();
		_ScaleData.strImg	= array_value.at(0).toString();
		_ScaleData.fScale	= array_value.at(1).toString().toDouble();
		_ScaleData.fOffset	= array_value.at(2).toString().toDouble();
		m_vecScale.push_back(_ScaleData);
	}

	return 0;
}

int ScaleTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	QJsonArray array_ScaleTool;
	for (size_t i = 0; i < m_vecScale.size(); i++)
	{
		QJsonArray array_value = {
			m_vecScale[i].strImg,
			QString::number(m_vecScale[i].fScale),
			QString::number(m_vecScale[i].fOffset),
		};
		array_ScaleTool.append(array_value);
	}
	strData.insert("ScaleTool", array_ScaleTool);

	return 0;
}

//双击后弹出控件的QT窗口
void ScaleTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmScaleTool * frm = new frmScaleTool(/*databaseVar::Instance().m_pWindow*/);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	//frm->Save();
	frm->m_pTool = nullptr;
	delete frm;
}

void ScaleTool::StopExecute()
{

}

EnumNodeResult ScaleTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult ScaleTool::PrExecute(MiddleParam & param, QString & strError)
{
	return NodeResult_OK;
}

EnumNodeResult ScaleTool::Execute(MiddleParam& param, QString& strError)
{
	if (param.MapPImgVoid.count(m_strInImg) > 0)	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strInImg));
		if (!hImg.IsInitialized())		{
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		HTuple hchannel, hType;
		GetImageType(hImg,&hType);
		m_dstImage = hImg.Clone();
		if (hType.S() == HString(HString("byte")))		{
			ConvertImageType(m_dstImage, &m_dstImage, HTuple("real"));
		}
		else if (hType.S() == HString("real") || hType.S() == HString("uint2") || hType.S() == HString("int4"))		{
			ConvertImageType(m_dstImage, &m_dstImage, HTuple("real"));
		}
		for (int i = 0; i < m_vecScale.size(); i++)		{
			ScaleImage(m_dstImage, &m_dstImage, m_vecScale[i].fScale, m_vecScale[i].fOffset);
		}
		param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId(), &m_dstImage));
	}
	else
	{
		SetError(QString("Image %1 is Empty").arg(m_strInImg));
		sDrawText			_strText;
		_strText.bControl	= true;
		if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
		_strText.DrawColor	= QColor(255, 0, 0);
		_strText.strText	= GetItemContent() + tr(" Didn't Contain Image!");
		param.VecDrawText.push_back(_strText);

		if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
		else			return NodeResult_NG;
	}
	return NodeResult_OK;
}

QPair<QString, QString>  ScaleTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString>  ScaleTool::GetToolName()
{
	return 	QPair<QString, QString>("Scale", tr("Scale"));
}
