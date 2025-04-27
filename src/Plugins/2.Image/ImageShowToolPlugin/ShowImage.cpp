#include "ShowImage.h"
#include <qdebug.h>
#include "frmImageShow.h"
#include "ImgGraphicsViews.h"
#include "databaseVar.h"

ShowImage::ShowImage() :FlowchartTool()
{
	// GetItemId() = GetToolName();
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
	m_vecCamParam.clear();
}

ShowImage::~ShowImage()
{
	Clearport();
}

int ShowImage::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_object = strData.find("ShowImageConFig").value().toArray();
	if (!array_object.empty()){
		m_iShowIndex	= array_object.at(0).toString().toInt();
	}
	m_vecCamParam.clear();
	QJsonArray object_arrayvalue = strData.find("ShowImageCamParam").value().toArray();
	for (size_t i = 0; i < object_arrayvalue.count(); i++){
		m_vecCamParam.push_back(object_arrayvalue.at(i).toString());
	}
	return 0;
}

int ShowImage::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	//scale
	QJsonArray ConFig = {
		QString::number(m_iShowIndex)
	};
	strData.insert("ShowImageConFig", ConFig);
	QJsonArray ConParamFig;
	for (auto iter : m_vecCamParam){
		ConParamFig.append(iter);
	}
	strData.insert("ShowImageCamParam", ConParamFig);
	return 0;
}

void ShowImage::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmImageShow * frm = new frmImageShow(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	//frm->Save();
	frm->m_pTool = nullptr;
	delete frm;
}

EnumNodeResult ShowImage::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult ShowImage::PrExecute(MiddleParam & param, QString& strError)
{
	if (m_iShowIndex < databaseVar::Instance().VideoWidgets.size() && IsDisabled)
	{
		((ImgGraphicsViews*)databaseVar::Instance().VideoWidgets[m_iShowIndex])->ClearShowAllImage();
	}
	return NodeResult_OK;
}

EnumNodeResult ShowImage::Execute(MiddleParam& param, QString& strError)
{	
	if ( m_vecCamParam.size() > 0 ){
		param.VecShowPImg.clear();
	}
	for (size_t i = 0; i < m_vecCamParam.size(); i++){
		param.VecShowPImg.push_back(m_vecCamParam[i]);
	}

	//½øÐÐÏÔÊ¾
	if (m_iShowIndex < databaseVar::Instance().VideoWidgets.size())	{
		((ImgGraphicsViews*)databaseVar::Instance().VideoWidgets[m_iShowIndex])->CloneParams(param);
	}
	else	{
		SetError(QString("Show Index %1 didn't Exist").arg(m_iShowIndex));
		return NodeResult_ParamError;
	}
	return NodeResult_OK;
}

QPair<QString, QString> ShowImage::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> ShowImage::GetToolName()
{
	return QPair<QString, QString>("Show", tr("Show"));
}
