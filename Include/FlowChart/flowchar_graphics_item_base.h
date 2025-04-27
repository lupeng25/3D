#pragma once
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QUuid>
#include <QPen>
#include <QMap>
#include <QBrush>
#include <QFont>
#include <QElapsedTimer>
#include <QList>
#include <QRect>
#include <QPointF>
#include <QVariant>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "Port.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

//#include "Common.h"

#ifndef _COREFLOWTOOL_EXPORT_
#define _COREFLOWTOOL_API_ _declspec(dllexport)
#else
#define _COREFLOWTOOL_API_ _declspec(dllimport)
#endif // !_PORT_EXPORT_

class FlowcharGraphicsLink;
class FlowchartGraphicsItem;
//端口和这个端口所在的节点信息
class _COREFLOWTOOL_API_ PortInfo
{
public:
	FlowchartGraphicsItem *node{ nullptr };//端口所在的节点
	Port *port{ nullptr };//端口
	bool IsEmpty()//判空
	{
		return node == nullptr || port == nullptr;
	}
	PortInfo() {	}
	PortInfo(FlowchartGraphicsItem * _node, Port *_port) {
		node = _node;
		port = _port;
	}
};

class _COREFLOWTOOL_API_ LineInfo
{
public:
	PortInfo  PortInfo1;//端口信息1
	PortInfo  PortInfo2;//端口信息2
	FlowcharGraphicsLink *line{ nullptr };//线
	LineInfo() {}
	LineInfo(PortInfo info1, PortInfo info2, FlowcharGraphicsLink *line) :
		PortInfo1(info1), PortInfo2(info2), line(line)
	{
	}
	PortInfo GetOtherPortNodeInfoByPort(Port*port)
	{
		return PortInfo1.port != port ? PortInfo1 : PortInfo2;
	}
	PortInfo GetPortNodeInfoByPort(Port*port)
	{
		return PortInfo1.port == port ? PortInfo1 : PortInfo2;
	}
};
enum EnumNodeResult
{
	NodeResult_Idle,
	NodeResult_OK,
	NodeResult_NG,
	NodeResult_Error,
	NodeResult_ParamError,
	NodeResult_TimeOut,
	NodeResult_Return,
};
enum class DrawLineAlignment
{
	DrawLeft = 0,
	DrawRight,
	DrawTop,
	DrawBottom,
};

//*************************************图元基类***************************************
class _COREFLOWTOOL_API_ FlowchartGraphicsItem
{
public:	//节点类型
	enum NodeType
	{
		BaseNode,
		StartNode,
		FunctionNode,
		ProgrammeControlNode,
		DataNode
	};
	enum NodeDirection
	{
		HasDirection,
		HasCombineDirect,
		NotHasDirection,
	};
public:
	FlowchartGraphicsItem(FlowchartInforBase _infor, ItemType _type = ItemType::Null);
	~FlowchartGraphicsItem();
public:
	//是否禁用
	virtual	void SetDisable(bool bValue);

	//添加端口
	void AddNodePort(Port *port);

	//删除端口
	void DeleteNodePort(uint portID, Port::PortType type);

	//清除端口
	void Clearport();

	//清除端口
	void Clearport(Port::PortType type);

	//设置端口值
	void SetPortValue(uint portID, QVariant data, Port::PortType type);

	//获取端口值
	QVariant GetPortValue(uint portID, Port::PortType type);

	//获取端口
	Port *GetOutPort(uint portID);

	//获取端口
	Port *GetPort(uint portID, Port::PortType type);

	//设置端口数据类型
	void SetPortDataType(uint portId, Port::PortType porttype, Port::PortDataType datatype);

	//拿到控制端口信息
	StreamPortinfo GetStreamInfo();

	//控制输出端口有多个，默认全部添加，如果需要规定那几个输出端口激活，则需要重载这个函数  IF，Then节点之类的
	virtual QList<Port*> OutputStreamLogicExecution();
public:
	virtual QPair<QString, QString> GetNodeTypeName() = 0;		//节点执行逻辑

	virtual	int SetData(QJsonObject & strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void SetError(QString strError);

	virtual void StopExecute();

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//节点执行逻辑

	EnumNodeResult NodeRun(MiddleParam& param, QString& strError);						//节点运行

	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p,const Port::PortType& type) = 0;
public:
	//获取输入端口数量
	uint GetInputCount()const;

	//获取输出端口数量
	uint GetOutCount()const;

	QList<Port*> GetInport();			//获取输入端口
	QList<Port*> GetOutport();			//获取输入端口
	QList<Port*> GetConnectedInport();	//获取已经连接的输入端口
	QList<Port*> GetConnectedOutport();	//获取已经连接的输出端口	
public:
	// 获取图元id
	QString GetItemId();

	//	获取图元内容
	QString GetItemContent();

	// 获取图元类型
	ItemType GetItemType();
public:
	virtual void SetCenterPoint(QPointF pt)		{ center_point_ = pt; };
	virtual QPointF GetCenterPoint()			{ return center_point_; };
	virtual QPointF GetLeftPoint()				{ return left_point_;	};
	virtual QPointF GetRightPoint()				{ return right_point_;	};
	virtual QPointF GetTopPoint()				{ return top_point_;	};
	virtual QPointF GetBottomPoint()			{ return bottom_point_; };
	virtual QPointF GetNextPoint(DrawLineAlignment aligment, Port* port, DIRECTION& dir);

	virtual int SizeInPath(const QPointF &p, DIRECTION &b);
	virtual int PointInItem(const QPointF &p, DIRECTION &b);
	virtual FlowchartInforBase GetItemInformation() { return item_base_info_; };
	virtual void SetText(QString _content, QString _tooltip = "") = 0;
public:
	NodeDirection								nodeDir;
public:
	QList<Port*>								portOutList;				//输出端口列表
	QList<Port*>								portInList;				//输入端口列表
public:
	float										m_fExcuteTime = 0;					//执行耗时
	QElapsedTimer								timer;
public:
	NodeType									nodeType{ BaseNode };//节点类型
	bool										IsExecuted;
	int											iToolIndex;		
	bool										IsDisabled = false;				//	禁用
	QString										m_strError;
	EnumNodeResult								m_EnumResult;
	EnumNodeResult								m_PrEnumResult;
	int											m_iFlowIndex;			//参数索引
public:
	QMap<QString, QString>						m_mapValue;	//参数
public:
	bool										m_bShowSelected;	//是否显示选中
	bool										m_bSelected;		//是否选中
	bool										m_bPositionChanged;	//位置变更
	FlowchartInforBase							item_base_info_;	//基础参数
	QPointF										center_point_, left_point_, right_point_, top_point_, bottom_point_;
	static QColor								DisableColor;
	static QMap<EnumNodeResult, QColor>			ResultColorMap;
	static	bool								m_iVisionThreadFirstRun;

	static	bool								m_bVisionThreadRun;
	static	bool								m_bVisionThreadNeedStop;
	static	int									m_iVisionThreadRunTime;


};