#include "ForTool.h"
#include "frmForTool.h"
#include "databaseVar.h"

ForTool::ForTool():	FlowchartGraphicsConditionItem()
{
	nodeDir = HasDirection;
	AddNodePort(new Port(0, "", Port::InStream, Port::Stream));
	AddNodePort(new Port(1, "", Port::InStream, Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream, Port::Stream));	//是
	AddNodePort(new Port(1, "", Port::OutStream, Port::Stream));	//否
	GetPort(0, Port::InStream)->portDir		= DIRECTION::NORTH;
	GetPort(1, Port::InStream)->portDir		= DIRECTION::WEST;
	GetPort(0, Port::OutStream)->portDir	= DIRECTION::SOUTH;
	GetPort(1, Port::OutStream)->portDir	= DIRECTION::EAST;

}

ForTool::~ForTool()
{
	Clearport();
}

QList<Port *> ForTool::OutputStreamLogicExecution()
{
    QList<Port*>outList;
    // 控制输出端口有多个，默认全部添加，如果需要规定那几个输出端口激活，则需要重载这个函数  ForTool，Then节点之类的
    //输入值是真把控制输出端口0添加，否则添加端口
	if (m_bFlag)
	{
		outList.append(GetPort(0, Port::OutStream));
	}
	else
	{
		outList.append(GetPort(1, Port::OutStream));
	}
    return outList;
}

int ForTool::SetData(QJsonObject & strData)
{
	return 0;
}

int ForTool::GetData(QJsonObject & strData)
{
	return 0;
}

void ForTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmForTool * frm = new frmForTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->Save();
	frm->m_pTool = nullptr;
	delete frm;
}

void ForTool::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	FlowchartGraphicsConditionItem::paint(painter, option, widget);
	const auto left_center_rect = QRectF((left_point_ - QPointF(3, 3)), QSizeF(6, 6));
	if (this->isSelected() || m_bShowSelected) {
		painter->drawRect(left_center_rect);
	}
}

QList<PortInfo> ForTool::CheckScenePortPoint(const QPointF & p, const Port::PortType& type)
{
	QList<PortInfo>	_portLst;
	DIRECTION		_dir;
	auto _pos = mapFromScene(p);
	PointInItem(_pos, _dir);
	bool _bIsOut = false;
	if (type == Port::PortType::Output || type == Port::PortType::OutStream) { _bIsOut = true; }
	else if (type == Port::PortType::Input || type == Port::PortType::InStream) { _bIsOut = false; }
	switch (_dir) {
	case NONE: {		}	break;
	case NORTH: {	if (!_bIsOut) { _portLst.append(PortInfo(this, GetPort(0, Port::InStream))); }			}	break;
	case EAST: {	if (_bIsOut) { _portLst.append(PortInfo(this, GetPort(1, Port::OutStream))); }			}	break;
	case SOUTH: {	if (_bIsOut) { _portLst.append(PortInfo(this, GetPort(0, Port::OutStream))); }			}	break;
	case WEST: {	if (!_bIsOut) { _portLst.append(PortInfo(this, GetPort(1, Port::InStream))); }			}	break;
	case INRECT: {	if (!_bIsOut) {
		if (GetPort(0, Port::InStream)->IsConnected) {
			_portLst.append(PortInfo(this, GetPort(1, Port::InStream)));
		}
		else {
			_portLst.append(PortInfo(this, GetPort(0, Port::InStream)));
		}
	}
				 else {}
	}	break;
	default: {		}	break;
	}
	return _portLst;
}

int ForTool::SizeInPath(const QPointF & p, DIRECTION & b)
{
	const auto left_center_rect = QRectF((left_point_ - QPointF(3, 3)), QSizeF(6, 6));
	const auto right_center_rect = QRectF((right_point_ - QPointF(3, 3)), QSizeF(6, 6));
	const auto top_center_rect = QRectF((top_point_ - QPointF(3, 3)), QSizeF(6, 6));
	const auto bottom_center_rect = QRectF((bottom_point_ - QPointF(3, 3)), QSizeF(6, 6));
	if (left_center_rect.contains(p))	b = DIRECTION::WEST;
	if (right_center_rect.contains(p))	b = DIRECTION::EAST;
	if (top_center_rect.contains(p))	b = DIRECTION::NORTH;
	if (bottom_center_rect.contains(p))	b = DIRECTION::SOUTH;
	return 0;
}

int ForTool::PointInItem(const QPointF & p, DIRECTION & b)
{
	if (path().contains(p))	b = DIRECTION::INRECT;
	SizeInPath(p, b);
	return 0;
}

EnumNodeResult ForTool::InitBeforeRun(MiddleParam & param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult ForTool::PrExecute(MiddleParam & param, QString& strError)
{
	m_iForNumber = 0;
	return NodeResult_OK;
}

EnumNodeResult ForTool::Execute(MiddleParam & param, QString& strError)
{
	if (m_iForNumber > 100)//循环不能超过100次
	{
		SetError(QString("ForTool Repeat %1 Times").arg(QString::number(m_iForNumber)));
		return NodeResult_ParamError;
	}
	m_iForNumber++;
	return NodeResult_OK;
}

QPair<QString, QString> ForTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> ForTool::GetToolName()
{
	return 	QPair<QString, QString>("For",tr("For"));
}