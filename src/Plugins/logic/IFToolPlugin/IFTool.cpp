#include "IFTool.h"
#include "frmIFConditionTool.h"
#include "databaseVar.h"

IFTool::IFTool(QPointF pos)
	: FlowchartGraphicsConditionItem()
	, iLogicOperator(1)
	, m_strSendContent("")
	, m_strSecondContent("")
{
	nodeDir = HasDirection;
	AddNodePort(new Port(0, "", Port::InStream, Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream, Port::Stream));	//是
	AddNodePort(new Port(1, "", Port::OutStream, Port::Stream));	//否
	GetPort(0, Port::InStream)->portDir		= DIRECTION::NORTH;
	GetPort(0, Port::OutStream)->portDir	= DIRECTION::SOUTH;
	GetPort(1, Port::OutStream)->portDir	= DIRECTION::EAST;
}

IFTool::~IFTool()
{
	//DeleteNodePort(0, Port::InStream);
	//DeleteNodePort(0, Port::OutStream);
	//DeleteNodePort(1, Port::OutStream);
	//DeleteNodePort(1, Port::Input);
}

QList<Port *> IFTool::OutputStreamLogicExecution()
{
    QList<Port*>outList;

    // 控制输出端口有多个，默认全部添加，如果需要规定那几个输出端口激活，则需要重载这个函数  IFTool，Then节点之类的
    //输入值是真把控制输出端口0添加，否则添加端口1
    if(m_iLogicValue)
    {
        outList.append(GetPort(0,Port::OutStream));
    }
	else
    {
        outList.append(GetPort(1,Port::OutStream));
    }

    return outList;
}

int IFTool::SetData(QJsonObject & strData)
{
	QJsonArray array_object = strData.find("ConFig").value().toArray();
	if (!array_object.empty())
	{
		m_strSendContent	= array_object.at(0).toString();
		m_strSecondContent	= array_object.at(1).toString();
		iLogicOperator		= array_object.at(2).toString().toInt();

	}
	return 0;
}

int IFTool::GetData(QJsonObject & strData)
{
	QJsonArray ConFig = {
		m_strSendContent,
		m_strSecondContent,
		QString::number(iLogicOperator),
	};
	strData.insert("ConFig", ConFig);
	return 0;
}

int IFTool::FindParamInLinkContent(QString strLinkContent, QString & strModel, QString & TypeVariable, QString & strValue)
{
	MiddleParam* _Params = nullptr;
	int	_DataType = -1;
	bool _bFind = false;
	QStringList _Lst = strLinkContent.split(":");
	if (_Lst.size() > 0)								{
		if (!_Lst[0].isEmpty())							{
			if (_Lst[0] == "GlobalVar")					{
				_DataType = 0;
				_Params = MiddleParam::Instance();
				strModel = "GlobalVar";
			}
			else if (_Lst.size() > 2)					{
				_DataType = _Lst[0].toInt() + 1;
				if (MiddleParam::MapMiddle().size() >= (_DataType - 1)) {
					_Params = &MiddleParam::MapMiddle()[_DataType - 1];
					strModel = QString::number(_DataType - 1);
				}
			}
		}
	}
	if (_Lst.size() > 2 && _Params != nullptr)			{
		if (_Lst[1] == "Bool")							{
			TypeVariable	= "Bool";
			if (_Params->MapBool.count(_Lst[2]) > 0)	{
				_bFind		= true;
				strValue	= QString::number(_Params->MapBool[_Lst[2]]);
			}
		}
		else if (_Lst[1] == "Int")						{
			TypeVariable	= "Int";
			if (_Params->MapInt.count(_Lst[2]) > 0)		{
				_bFind		= true;
				strValue	= QString::number(_Params->MapInt[_Lst[2]]);
			}
		}
		else if (_Lst[1] == "Double")					{
			TypeVariable	= "Double";
			if (_Params->MapDou.count(_Lst[2]) > 0)		{
				_bFind		= true;
				strValue	= QString::number(_Params->MapDou[_Lst[2]]);
			}
		}
		else if (_Lst[1] == "QString")					{
			TypeVariable	= "QString";
			if (_Params->MapStr.count(_Lst[2]) > 0)		{
				_bFind		= true;
				strValue	= QString(_Params->MapStr[_Lst[2]]);
			}
		}
		else if (_Lst[1] == "QPoint")					{
			TypeVariable	= "QPoint";
			if (_Params->MapPoint.count(_Lst[2]) > 0)	{
				_bFind		= true;
				auto& pt	= _Params->MapPoint[_Lst[2]];
				strValue	= QString("(%1,%2)").arg(QString::number(pt.x())).arg(QString::number(pt.y()));// _Lst[2];
			}
		}
		else if (_Lst[1] == "QPointF")					{
			TypeVariable	= "QPointF";
			if (_Params->MapPointF.count(_Lst[2]) > 0)	{
				_bFind		= true;
				auto& pt	= _Params->MapPointF[_Lst[2]];
				strValue	= QString("(%1,%2)").arg(QString::number(pt.x())).arg(QString::number(pt.y()));// _Lst[2];
			}
		}
	}
	if (_bFind )	{

	}
	else
	{
		strValue = strLinkContent;
		return -1;
	}

	return 0;
}

EnumNodeResult IFTool::ParamInExecute(QString strLinkContent, QString strLinkSecondContent,int iOperator, int& iValue)
{
	EnumNodeResult	m_PrEnumResult = NodeResult_OK;
	QString strModelIndex = "";
	QString strTypeVariable = "";
	QString	strValue = "";
	QString strSecondModelIndex = "";
	QString strSecondTypeVariable = "";
	QString	strSecondValue = "";
	int _iLinkRetn = FindParamInLinkContent(strLinkContent, strModelIndex, strTypeVariable, strValue);
	int _iLinkSecondRetn = FindParamInLinkContent(strLinkSecondContent, strSecondModelIndex, strSecondTypeVariable, strSecondValue);
	try
	{
		if (_iLinkRetn >= 0 && _iLinkSecondRetn >= 0) {
			if (strTypeVariable == "Bool") {
				if ((strSecondTypeVariable == "Bool") || (strSecondTypeVariable == "Int")) {}
				else { m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue.toInt() == strSecondValue.toInt()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue.toInt() != strSecondValue.toInt()) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "Int") {
				if ((strSecondTypeVariable == "Bool") || (strSecondTypeVariable == "Int") || (strSecondTypeVariable == "Double")) {}
				else { m_PrEnumResult = NodeResult_ParamError;return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "Double") {
				if ((strSecondTypeVariable == "Bool") || (strSecondTypeVariable == "Int") || (strSecondTypeVariable == "Double")) {}
				else { m_PrEnumResult = NodeResult_ParamError;  return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "QString") {
				if ((strSecondTypeVariable == "QString")) {}
				else { m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {			}	break;
				}
			}
			else if (strTypeVariable == "QPoint") {
				if ((strSecondTypeVariable == "QPoint")) {}
				else { m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "QPointF") {
				if ((strSecondTypeVariable == "QPointF")) {}
				else { m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: { m_PrEnumResult = NodeResult_ParamError;return m_PrEnumResult;	}	break;
				}
			}
			else if (strTypeVariable == "sResultCross") {
				if ((strSecondTypeVariable == "sResultCross")) {}
				else { m_PrEnumResult = NodeResult_ParamError;  return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	 m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult; }	break;
				}
			}
			else if (strTypeVariable == "Metrix") {
				if ((strSecondTypeVariable == "sResultCross")) {}
				else { m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult; };
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult;	}	break;
				}
			}
		}
		else if (_iLinkRetn >= 0 || _iLinkSecondRetn >= 0) {
			if (strTypeVariable == "Bool" || strSecondTypeVariable == "Bool") {
				switch (iOperator) {
				case 0: {	if (strValue.toInt() == strSecondValue.toInt()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue.toInt() != strSecondValue.toInt()) { iValue = 1; } else { iValue = 0; };	}	break;
				default: { m_PrEnumResult = NodeResult_ParamError; return m_PrEnumResult;	}	break;
				}
			}
			else if (strTypeVariable == "Int" || strSecondTypeVariable == "Bool") {
				switch (iOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "Double" || strSecondTypeVariable == "Double") {
				switch (iOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "QString" || strSecondTypeVariable == "QString") {
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "QPoint" || strSecondTypeVariable == "QPoint") {
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "QPointF" || strSecondTypeVariable == "QPointF") {
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "sResultCross" || strSecondTypeVariable == "sResultCross") {
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
			else if (strTypeVariable == "Metrix" || strSecondTypeVariable == "Metrix") {
				switch (iOperator) {
				case 0: {	if (strValue == strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { iValue = 1; } else { iValue = 0; };	}	break;
				default: {	}	break;
				}
			}
		}
		else {
			m_PrEnumResult = NodeResult_ParamError;
			return m_PrEnumResult;
		};
	}
	catch (const std::exception&)
	{
		m_PrEnumResult = NodeResult_ParamError;
		return m_PrEnumResult;
	}
	return m_PrEnumResult;
}

void IFTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmIFConditionTool * frm			= new frmIFConditionTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool						= this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->Save();
	frm->m_pTool						= nullptr;
	delete frm;
}

QList<PortInfo> IFTool::CheckScenePortPoint(const QPointF & p, const Port::PortType & type)
{	
	QList<PortInfo>	_portLst;
	DIRECTION		_dir;
	auto _pos		= mapFromScene(p);
	PointInItem(_pos, _dir);
	bool _bIsOut = false;
	if (type == Port::PortType::Output || type == Port::PortType::OutStream)	{	_bIsOut = true;	}
	else if (type == Port::PortType::Input || type == Port::PortType::InStream)	{	_bIsOut = false;}
	switch (_dir){
	case NONE:	 {		}	break;
	case NORTH:	 {	if (!_bIsOut) { _portLst.append(PortInfo(this,GetPort(0, Port::InStream)));		}	}	break;
	case EAST:	 {	if (_bIsOut) _portLst.append(PortInfo(this, GetPort(1, Port::OutStream)));	}	break;
	case SOUTH:	 {	if (_bIsOut) _portLst.append(PortInfo(this, GetPort(0, Port::OutStream)));	}	break;
	case WEST:	 {		}	break;
	case INRECT: {	if (!_bIsOut){ _portLst.append(PortInfo(this, GetPort(0, Port::InStream))); }	else {	}}	break;
	default:	 {		}	break;
	}
	return _portLst;
}

EnumNodeResult IFTool::InitBeforeRun(MiddleParam & param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult IFTool::PrExecute(MiddleParam & param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult IFTool::Execute(MiddleParam & param, QString& strError)
{
	m_PrEnumResult	= NodeResult_OK;
	QString strModelIndex			= "";
	QString strTypeVariable			= "";
	QString	strValue				= "";
	QString strSecondModelIndex		= "";
	QString strSecondTypeVariable	= "";
	QString	strSecondValue			= "";
	int _iLinkRetn					= FindParamInLinkContent(m_strSendContent,			strModelIndex,			strTypeVariable,		strValue);
	int _iLinkSecondRetn			= FindParamInLinkContent(m_strSecondContent,		strSecondModelIndex,	strSecondTypeVariable,	strSecondValue);
	try
	{
		if (_iLinkRetn >= 0 && _iLinkSecondRetn >= 0) {
			if (strTypeVariable == "Bool") {
				if ((strSecondTypeVariable == "Bool") || (strSecondTypeVariable == "Int")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool LinkParam Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue.toInt() == strSecondValue.toInt()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue.toInt() != strSecondValue.toInt()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;		}
			}
			else if (strTypeVariable == "Int") {
				if ((strSecondTypeVariable == "Bool") || (strSecondTypeVariable == "Int") || (strSecondTypeVariable == "Double")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;		}
			}
			else if (strTypeVariable == "Double") {
				if ((strSecondTypeVariable == "Bool") || (strSecondTypeVariable == "Int") || (strSecondTypeVariable == "Double")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;		}
			}
			else if (strTypeVariable == "QString") {
				if ((strSecondTypeVariable == "QString")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {			}	break;
				}
			}
			else if (strTypeVariable == "QPoint") {
				if ((strSecondTypeVariable == "QPoint")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;			}
			}
			else if (strTypeVariable == "QPointF") {
				if ((strSecondTypeVariable == "QPointF")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult;	}	break;		}
			}
			else if (strTypeVariable == "sResultCross") {
				if ((strSecondTypeVariable == "sResultCross")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	 m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; }	break;		}
			}
			else if (strTypeVariable == "Metrix") {
				if ((strSecondTypeVariable == "sResultCross")) {}
				else { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult; };
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult;	}	break;	}
			}
		}
		else if (_iLinkRetn >= 0 || _iLinkSecondRetn >= 0) {
			if (strTypeVariable == "Bool" || strSecondTypeVariable == "Bool") {
				switch (iLogicOperator) {
				case 0: {	if (strValue.toInt() == strSecondValue.toInt()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue.toInt() != strSecondValue.toInt()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: { m_PrEnumResult = NodeResult_ParamError; SetError(QString("IFTool Link Param Error")); return m_PrEnumResult;	}	break;
				}
			}
			else if (strTypeVariable == "Int" || strSecondTypeVariable == "Bool") {
				switch (iLogicOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;	}
			}
			else if (strTypeVariable == "Double" || strSecondTypeVariable == "Double") {
				switch (iLogicOperator) {
				case 0: {	if (strValue.toDouble() > strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue.toDouble() < strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 2: {	if (strValue.toDouble() >= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 3: {	if (strValue.toDouble() <= strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 4: {	if (strValue.toDouble() == strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 5: {	if (strValue.toDouble() != strSecondValue.toDouble()) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;	}
			}
			else if (strTypeVariable == "QString" || strSecondTypeVariable == "QString") {
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default:{	}	break;		}
			}
			else if (strTypeVariable == "QPoint" || strSecondTypeVariable == "QPoint") {
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;	}
			}
			else if (strTypeVariable == "QPointF" || strSecondTypeVariable == "QPointF") {
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;	}
			}
			else if (strTypeVariable == "sResultCross" || strSecondTypeVariable == "sResultCross") {
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;	}
			}
			else if (strTypeVariable == "Metrix" || strSecondTypeVariable == "Metrix") {
				switch (iLogicOperator) {
				case 0: {	if (strValue == strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				case 1: {	if (strValue != strSecondValue) { m_iLogicValue = 1; } else { m_iLogicValue = 0; };	}	break;
				default: {	}	break;	}
			}
		}
		else { 
			m_PrEnumResult	= NodeResult_ParamError; 
			SetError(QString(tr("IFTool Link Param Error")));
			return m_PrEnumResult; 
		};
	}
	catch (const std::exception&)
	{
		m_PrEnumResult = NodeResult_ParamError; 
		SetError(QString(tr("IFTool Link Param Error")));
		return m_PrEnumResult;
	}

	return m_PrEnumResult;
}

QPair<QString, QString> IFTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> IFTool::GetToolName()
{
	return QPair<QString, QString>("IF",tr("IF"));
}
