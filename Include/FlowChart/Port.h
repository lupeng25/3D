#pragma once
#ifndef PORT_H
#define PORT_H

#include <QMap>
#include <QPainter>
#include <QGraphicsItem>
#include <QString>
#include <QGraphicsView>
//#include "flowchart_global.h"
#include "Comm.h"

#ifndef _PORT_EXPORT_
#define _PORT_API_ _declspec(dllexport)
#else
#define _PORT_API_ _declspec(dllimport)
#endif // !_PORT_EXPORT_

class _PORT_API_ Port
{
public:
	//端口类型
	enum PortType { None, InStream, OutStream, Input, Output };
	//端口数据类型
	enum PortDataType
	{
		Stream,		//控制流
		Int,		//整数
		Bool,		//布尔值
		String,		//字符串
		Double,		//小数值
		Bit,		//位
		Vector2,	//二维向量
		Vector3,	//三维向量
		Color,		//颜色
		Image,		//图像
	};

	bool IsValid();
	Port();
	Port(uint id, QString Name = "Port", PortType type = Input, PortDataType portDataType = Int, QVariant Data = 0);
	// 端口类型检测
	bool PortTypeCheck(PortType traporttype);
	//端口存放的数据
	QVariant Data;
	//端口类型
	PortType portType;
	//端口数据类型
	PortDataType portDataType;
	//方向
	DIRECTION portDir;
	////端口范围
	//QRectF portRect;
	//端口ID
	uint ID = 0;
	//端口名称
	QString Name{ "Port" };
	//是否连接
	bool IsConnected = false;
public:
	//端口数据类型名字表
	static QMap<Port::PortDataType, QString> PortDataTypeNameMap;

	//端口数据类型转换表
	static QMap<Port::PortDataType, Port::PortDataType> PortDataConvertionMap;
};

class _PORT_API_ StreamPortinfo
{
public:
	enum StreamState { NotStream, InStreamAndOutStream, OnlyInStream, OnlyOutStream };
	Port *InStreamPort{ nullptr };
	//节点会有多个程序控制输出端口
	QList<Port*>OutStreamPortList;
	StreamState streamState;
	StreamPortinfo(Port *InStreamPort, QList<Port*> OutStreamPortList) :InStreamPort(InStreamPort), OutStreamPortList(OutStreamPortList)
	{
		if (InStreamPort == nullptr&&OutStreamPortList.count() == 0)
			streamState = NotStream;
		if (InStreamPort != nullptr&&OutStreamPortList.count() != 0)
			streamState = InStreamAndOutStream;
		if (InStreamPort != nullptr&&OutStreamPortList.count() == 0)
			streamState = OnlyInStream;
		if (InStreamPort == nullptr&&OutStreamPortList.count() != 0)
			streamState = OnlyOutStream;
	}
};

class _PORT_API_ MiddleParam
{
public:
	MiddleParam();
	~MiddleParam();
public:
	static MiddleParam* Instance();

	static std::vector<MiddleParam>& MapMiddle();	//图像中间参数
public:
	MiddleParam& Clone();
	void CloneTo(MiddleParam& param);
	void CloneShow(MiddleParam& param);

public:
	std::vector<MiddleParam> m_vecParam;	//图像中间参数
public:
	int Init();
	int Exit();

	int Clear();
public:
	int	m_iIndex;	//当前的图像索引
	QString m_strError;
	QString	m_strName;

public:	//中间数据
	std::map<QString, bool>			MapBool;
	std::map<QString, int>			MapInt;
	std::map<QString, double>		MapDou;
	std::map<QString, QString>		MapStr;
	std::map<QString, QPoint>		MapPoint;
	std::map<QString, QPointF>		MapPointF;
	std::map<QString, sRadPoint>	MapRadPointF;
	std::map<QString, sResultCross>	MapCrossPointF;

	std::map<QString, void*>		MapPImgVoid;
	std::map<QString, void*>		MapPtrVoid;
	std::map<QString, void*>		MapPtrMetrix;
public:

public:
	void ClearAll();
public:
	std::map<QString, sDrawCross>	MapDrawCross;
	std::vector<QString>			VecShowPImg;	//
	std::map<QString, QPointF>		MapShowPointF;
	std::map<QString, sDrawText>	MapShowMsg;
	std::map<QString, sColorCircle>	MapShowColorCircle;
	std::vector<QPointF>			VecShowQPointFs;	//
	std::vector<sColorPoint>		VecShowColorPoints;	//
	std::vector<sColorLine>			VecShowColorLine;	//
	std::vector<sColorRRectangle>	VecShowColorRRect;	//
	std::vector<sColorRRectangle>	VecShowColorEllipse;	//
	std::vector<sColorLineRect>		VecShowColorLineRect;	//
	std::vector<sColorCircle>		VecShowColorCircle;	//
public:
	std::vector<sDrawText>			VecDrawText;


};

#endif // PORT_H