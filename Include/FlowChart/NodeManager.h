#pragma once
#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include "flowchar_graphics_link.h"
#include "Port.h"
#include <unordered_set>
#include <QGraphicsView>
#include <QThread>
class FlowGraphicsViews;

#ifndef _COREFLOWTOOL_EXPORT_
#define _COREFLOWTOOL_API_ _declspec(dllexport)
#else
#define _COREFLOWTOOL_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

//参数和结果
class _COREFLOWTOOL_API_ NodeManager :	public QThread
{
	Q_OBJECT
public:
	NodeManager(QObject *parent = nullptr);
	~NodeManager();
public:
	void SetGraphicsView(FlowGraphicsViews *pView);
public:
	bool SafePortCheck(PortInfo& info1);

	// 判断箭头连线起始点类型
	DrawLineAlignment CompareDrawPoint(QPointF _start_point, QPointF _end_point);
public:
	//通过点拿到节点     即这个点是不是在节点上
	FlowchartGraphicsItem* GetNodeByPos(QPoint pos);

	//通过节点名称拿到节点     即这个点是不是在节点上
	FlowchartGraphicsItem* GetNodeByName(QString node);

	//通过端口拿到与该端口连接的所有端口信息信息
	QList<PortInfo> GetNodeAndPortListByPort(Port* port);

	//通过端口拿到与该端口连接的所有线信息
	QList<LineInfo> GetLineInfoListByPort(Port* port);

	//获取连接到节点上输入端口上的所有端口信息
	QList<PortInfo> GetConnectedInPortInfo(Port* port);

	//通过端口拿到与该端口连接的节点和端口信息列表  (只会拿到一个，比如输入口只能有一个输入节点)
	PortInfo GetNodeAndPortByPort(Port* port);

	//拿到节点程序控制输出端口节点信息表  控制输出端口连接的所有的节点
	virtual QList<PortInfo> GetOutStreamPortInfoByNode(FlowchartGraphicsItem*node);

	//拿到与该节点控制输入端口节点信息  因为输入只能有一个，所以只会存在一个节点与输入端口连接
	PortInfo GetInStreamPortInfoByNode(FlowchartGraphicsItem*node);
protected:
	void run();
public:
Q_SIGNALS:
	void sigAutoRunFinished();
public:
	//端口类型检测
	bool PortTypeCheck(PortInfo info1, PortInfo info2);

	//端口连接单调性检测
	bool PortMonotonicityCheck(PortInfo info1, PortInfo info2);

	//端口数据类型检测
	bool PortDataTypeCheck(PortInfo info1, PortInfo info2);

	//从一个端口出发看看能不能到达这个端口，如果可以到达那么就是回环，如果最后是空的端口那么就不是回环
	bool CycleCheck();

	//通过点更新节点线的位置
	void UpdateNode(QPoint pos);

	//更新节点线的位置
	void UpDateNode(FlowchartGraphicsItem*node);

	//更新选中的节点线的位置
	void UpDateSelectedNode();

	//添加一条连线关系
	void AddRelation(LineInfo info);
public:
	//执行
	void StopExecute();

	//执行
	EnumNodeResult InitBeforeRun();

	//执行
	EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);

	//执行之前
	EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	////执行
	EnumNodeResult Execute(MiddleParam& param, QString& strError);

	////节点执行 递归
	EnumNodeResult NodeRun(QList<PortInfo> portnodeinfolist,MiddleParam& param, QString& strError);

	//节点执行完成以后刷新
	void NodeReflush();
public:
	//添加节点
	void AddNode(FlowchartGraphicsItem*node);

	//连接两个端点
	void PortConnect(PortInfo port1, PortInfo port2);

	//重新连接两个端点
	void RePortConnect(PortInfo port1, PortInfo port2);

	//尝试连接两个类型不同的端点
	void PortConvertConnect(PortInfo port1, PortInfo port2);

	//检查数据是否可以转换
	bool PortIsConvertion(PortInfo port1, PortInfo port2);

public:
	//获取选中端口
	QList<FlowchartGraphicsItem*> GetSelectedPorts();

	//删除一个端点之间所有连接的线
	void DeletePortConnect(PortInfo portinfo1);

	//删除选中的节点和线
	void DeleteSelected();

	//当前是否选中
	bool IsSelected();

	//清除选中
	void ClearAllSelected();

	//清除所有组员
	void ClearAll();
public:
	//端口连线信息列表
	QList<LineInfo> PortLineInfoList;

	//节点表
	QList<FlowchartGraphicsItem*>NodeList;

	//场景
	FlowGraphicsViews *view ;

	int	m_iIndex = 0;
	bool m_bThreadRun = false;
	float m_fDurTime = 0;
};

#endif // NODEMANAGER_H