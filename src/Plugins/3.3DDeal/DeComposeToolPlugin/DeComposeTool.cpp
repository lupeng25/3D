#include "DeComposeTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmDeComposeTool.h"
#include "databaseVar.h"

DeComposeTool::DeComposeTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
	m_dstImage.GenEmptyObj();
}

DeComposeTool::~DeComposeTool()
{
}

int DeComposeTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	return 0;
}

int DeComposeTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	return 0;
}

//双击后弹出控件的QT窗口
void DeComposeTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmDeComposeTool * frm = new frmDeComposeTool(/*databaseVar::Instance().m_pWindow*/);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->m_pTool = nullptr;
	delete frm;
}

void DeComposeTool::StopExecute()
{

}

EnumNodeResult DeComposeTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult DeComposeTool::PrExecute(MiddleParam & param, QString & strError)
{
	return NodeResult_OK;
}

EnumNodeResult DeComposeTool::Execute(MiddleParam& param, QString& strError)
{
	if (param.MapPImgVoid.count(m_strInImg) > 0)
	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strInImg));
		if (!hImg.IsInitialized())
		{
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		HTuple	hChannal;
		CountChannels(hImg,&hChannal);
		switch (hChannal.I())
		{
		case 1:
		{	
			if (m_vecOutImg.size() <= 0)
				m_vecOutImg.push_back(HObject());
			m_vecOutImg[0] = hImg.Clone();
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_0", &m_vecOutImg[0]));
		}	break;
		case 2:
		{
			if (m_vecOutImg.size() <= 0)
				m_vecOutImg.push_back(HObject());
			if (m_vecOutImg.size() <= 1)
				m_vecOutImg.push_back(HObject());
			Decompose2(hImg, &m_vecOutImg[0], &m_vecOutImg[1]);
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_0", &m_vecOutImg[0]));
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_1", &m_vecOutImg[1]));
		}break;
		case 3:
		{		
			if (m_vecOutImg.size() <= 0)
				m_vecOutImg.push_back(HObject());
			if (m_vecOutImg.size() <= 1)
				m_vecOutImg.push_back(HObject());
			if (m_vecOutImg.size() <= 2)
				m_vecOutImg.push_back(HObject());
			HObject hImg1, hImg2, hImg3;
			Decompose3(hImg, &m_vecOutImg[0], &m_vecOutImg[1], &m_vecOutImg[2]);
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_0", &m_vecOutImg[0]));
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_1", &m_vecOutImg[1]));
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_2", &m_vecOutImg[2]));
		}break;
		case 4:
		{
			if (m_vecOutImg.size() <= 0)
				m_vecOutImg.push_back(HObject());
			if (m_vecOutImg.size() <= 1)
				m_vecOutImg.push_back(HObject());
			if (m_vecOutImg.size() <= 2)
				m_vecOutImg.push_back(HObject());
			if (m_vecOutImg.size() <= 3)
				m_vecOutImg.push_back(HObject());
			Decompose4(hImg, &m_vecOutImg[0], &m_vecOutImg[1], &m_vecOutImg[2], &m_vecOutImg[3]);
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_0", &m_vecOutImg[0]));
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_1", &m_vecOutImg[1]));
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_2", &m_vecOutImg[2]));
			param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + "_3", &m_vecOutImg[3]));
		}break;
		default:
		{
		}	break;
		}
	}
	else
	{
		SetError(QString("Image %1 is Empty").arg(m_strInImg));
		return NodeResult_ParamError;
	}
	return NodeResult_OK;
}

QPair<QString, QString> DeComposeTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> DeComposeTool::GetToolName()
{
	return 	QPair<QString, QString>("DeCompose", tr("DeCompose"));
}