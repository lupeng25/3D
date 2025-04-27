#pragma once
#ifndef SCRIPTEDITTOOL_H
#define SCRIPTEDITTOOL_H


#include "FlowchartTool.h"
#include "LUA/lua.hpp"
class ScriptEditTool :public FlowchartTool
{
	Q_OBJECT
public:
	ScriptEditTool();
	~ScriptEditTool();

public:
	void Init();
public:
	EnumNodeResult ExcuteString(QString strData, QString& strError);

public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);

public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:	//运行参数
	lua_State*		m_LuaState;
	QString			m_strScriptData;
	MiddleParam*	m_PtrParam;

public:
	static int print(lua_State *L);
	static int getDateTime(lua_State *L);	//获取当前时间
	static int getValue(lua_State *L);
	static int setValue(lua_State *L);
	static int setGlobalValue(lua_State *L);
	static int getGlobalValue(lua_State *L);
	static int setLocalValue(lua_State *L);
	static int getLocalValue(lua_State *L);
	static int split(lua_State *L);

	static int LogInfo(lua_State *L);
	static int LogWarn(lua_State *L);
	static int LogError(lua_State *L);

	static int sendData(lua_State *L);
	static int RecieveData(lua_State *L);

	static int RunCall(lua_State *L);

};
#endif // ScriptEditTool