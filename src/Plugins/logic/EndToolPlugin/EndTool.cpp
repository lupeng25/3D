#include "EndTool.h"
#include "frmEndTool.h"
#include "databaseVar.h"

EndTool::EndTool(QPointF pos):FlowchartGraphicsRectItem()
{
	nodeDir = HasCombineDirect;
	AddNodePort(new Port(0, "", Port::OutStream, Port::Stream));	//是
	AddNodePort(new Port(0, "", Port::InStream, Port::Stream));	//是
	AddNodePort(new Port(1, "", Port::InStream, Port::Stream));	//是
	GetPort(0, Port::InStream)->portDir		= DIRECTION::NORTH;
	GetPort(1, Port::InStream)->portDir		= DIRECTION::EAST;
	GetPort(0, Port::OutStream)->portDir	= DIRECTION::SOUTH;
}

EndTool::~EndTool()
{
	Clearport();
}

//QList<Port *> EndTool::OutputStreamLogicExecution()
//{
//    QList<Port*>outList;
//    // 控制输出端口有多个，默认全部添加，如果需要规定那几个输出端口激活，则需要重载这个函数  EndTool，Then节点之类的
//	outList.append(GetPort(0, Port::OutStream));
//    return outList;
//}

int EndTool::SetData(QJsonObject & strData)
{
	QJsonArray array_object = strData.find("ConFig").value().toArray();
	if (!array_object.empty())
	{
		m_iNumber = array_object.at(0).toString().toInt();
	}
	return 0;
}

int EndTool::GetData(QJsonObject & strData)
{
	QJsonArray ConFig = {
		QString::number(m_iNumber),
	};
	strData.insert("ConFig", ConFig);
	return 0;
}

QList<PortInfo> EndTool::CheckScenePortPoint(const QPointF & p, const Port::PortType& type)
{
	QList<PortInfo>	_portLst;
	DIRECTION		_dir;
	auto _pos = mapFromScene(p);
	PointInItem(_pos, _dir);

	bool _bIsOut = false;
	if (type == Port::PortType::Output || type == Port::PortType::OutStream) { _bIsOut = true; }
	else if (type == Port::PortType::Input || type == Port::PortType::InStream) { _bIsOut = false; }

	if (_bIsOut) { if (_dir != DIRECTION::NONE) { _portLst.append(PortInfo(this, GetPort(0, Port::OutStream))); } }
	else {
		if (_dir != DIRECTION::NONE) {
			int _iIndex = -1;
			for (int i = 0; i < portInList.size(); i++)
			{
				if (portInList[i]->IsConnected)
				{

				}
				else
				{
					_portLst.append(PortInfo(this, GetPort(i, Port::InStream)));
					//portInList.append();
				}

			}

			if (GetPort(0, Port::InStream)->IsConnected) {
				_portLst.append(PortInfo(this, GetPort(1, Port::InStream)));
			}
			else {
				_portLst.append(PortInfo(this, GetPort(0, Port::InStream)));
			}
		}
	}
	return _portLst;
}

void EndTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmEndTool * frm = new frmEndTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool			= this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->Save();
	frm->m_pTool			= nullptr;

	if (m_iNumber > portInList.size())
	{
		for (int i = portInList.size() ; i < m_iNumber; i++)
		{
			AddNodePort(new Port(i, "", Port::InStream, Port::Stream));
		}
	}
	else
	{
		for (int i = m_iNumber ; i < portInList.size(); i++)
		{
			DeleteNodePort(i, Port::InStream);
		}
	}
	delete frm;
}

void EndTool::StopExecute()
{
}

EnumNodeResult EndTool::InitBeforeRun(MiddleParam & param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult EndTool::PrExecute(MiddleParam & param, QString& strError)
{
	m_iForNumber = 0;
	return NodeResult_OK;
}

EnumNodeResult EndTool::Execute(MiddleParam & param, QString& strError)
{
	if (m_iForNumber > 100) {	//循环不能超过100次
		SetError(QString("EndTool Repeat %1 Times").arg(QString::number(m_iForNumber)));
		return NodeResult_ParamError;
	}
	m_iForNumber++;
	return NodeResult_OK;
}

QPair<QString, QString> EndTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> EndTool::GetToolName()
{
	return 	QPair<QString, QString>("End", tr("End"));
}
