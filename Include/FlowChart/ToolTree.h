#pragma once
#include <QtWidgets/QWidget>
#include <map>
#include <vector>
#include <list>
//#include"../Control/datavar.h"
#include "flowchar_graphics_item_base.h"

#ifndef _NODE_EXPORT_
#define _NODE_API_ _declspec(dllexport)
#else
#define _NODE_API_ _declspec(dllimport)
#endif // !_PORT_EXPORT_

#include"NodeManager.h"
using namespace std;

typedef FlowchartGraphicsItem *(*CreateTool)(QString strTool);

class _NODE_API_ ToolTree
{
public:
	typedef std::pair<QString, std::list<QString> > ToolsPair;
public:
	ToolTree();
	static ToolTree& Instance();
public:
	int Init();

	void RegistCreateToolCallBack(CreateTool call) { m_ToolCall = call; };
public:
	int Register(QString strTools, QString strTool);
	FlowchartGraphicsItem *CreateToolNode(QString strTool);

public:
	std::vector<ToolsPair>	m_Tools;

private:
	CreateTool m_ToolCall;

};

