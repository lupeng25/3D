#include "ScriptEditTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmScriptEditTool.h"
#include "databaseVar.h"
#include "qdebug.h"
#include "gvariable.h"
#include <QDateTime>
#include "LUA/lstate.h"

ScriptEditTool::ScriptEditTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
	Init();
}

ScriptEditTool::~ScriptEditTool()
{
	lua_close(m_LuaState);
}

void ScriptEditTool::Init()
{
	m_LuaState				= luaL_newstate();
	m_LuaState->user_param	= this;
	luaL_openlibs(m_LuaState);  /* open libraries */
	lua_register(m_LuaState,"getdatetime",	ScriptEditTool::getDateTime);
	lua_register(m_LuaState,"setglobal",	ScriptEditTool::setGlobalValue);
	lua_register(m_LuaState,"getglobal",	ScriptEditTool::getGlobalValue);
	lua_register(m_LuaState,"setlocal",		ScriptEditTool::setLocalValue);
	lua_register(m_LuaState,"getlocal",		ScriptEditTool::getLocalValue);

	lua_register(m_LuaState,"SendData",		ScriptEditTool::sendData);
	lua_register(m_LuaState,"RecieveData",	ScriptEditTool::RecieveData);
	
	lua_register(m_LuaState,"RunCall",		ScriptEditTool::RunCall);

	lua_register(m_LuaState,"print" ,		ScriptEditTool::print);
	lua_register(m_LuaState,"split" ,		ScriptEditTool::split);

	lua_register(m_LuaState,"loginfo",		ScriptEditTool::LogInfo);
	lua_register(m_LuaState,"logwarn",		ScriptEditTool::LogWarn);
	lua_register(m_LuaState,"logerror",		ScriptEditTool::LogError);

	lua_sethook(m_LuaState, NULL, NULL, 1);			//清空触发
	m_PtrParam = nullptr;

}

EnumNodeResult ScriptEditTool::ExcuteString(QString strData, QString & strError)
{
	if (luaL_dostring(m_LuaState, (strData.toLocal8Bit())) != 0)
	{
		const char * strchar = lua_tostring(m_LuaState, -1);
		strError.append(strchar);
		return	NodeResult_Error;
	}
	return NodeResult_OK;
}

int ScriptEditTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_object = strData.find("ConFig").value().toArray();
	if (!array_object.empty())
	{
		m_strScriptData = array_object.at(0).toString();

		//m_iScriptEditType	= array_object.at(0).toString().toInt();
		//m_iScriptEditTime	= array_object.at(1).toString().toInt();
	}
	return 0;
}

int ScriptEditTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	QJsonArray ConFig = {
		m_strScriptData,
		//QString::number(m_iScriptEditType),
		//QString::number(m_iScriptEditTime),
	};
	strData.insert("ConFig", ConFig);
	return 0;
}

void ScriptEditTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	//frmScriptEditTool * frm = new frmScriptEditTool(databaseVar::Instance().m_pWindow);
	frmScriptEditTool * frm = new frmScriptEditTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	//frm->Save();
	frm->m_pTool = nullptr;
	delete frm;
	lua_sethook(m_LuaState, NULL, NULL, 1);			//清空触发
}

EnumNodeResult ScriptEditTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	if (m_strScriptData.isEmpty()) {
		SetError(QString("Script %1 is Empty").arg(m_strScriptData));
		return	NodeResult_Error; }
	m_PtrParam = nullptr;
	gVariable::Instance().ClearRecieve();
	return NodeResult_OK;
}

EnumNodeResult ScriptEditTool::PrExecute(MiddleParam & param, QString& strError)
{
	m_PtrParam = nullptr;
	return NodeResult_OK;
}

EnumNodeResult ScriptEditTool::Execute(MiddleParam& param, QString& strError)
{
	m_PtrParam = &param;
	EnumNodeResult retn = ExcuteString(m_strScriptData, strError);
	if (retn != NodeResult_OK)	{
		SetError(QString("Script Error %1").arg(strError));
		qCritical() << strError;
	}
	return retn;
}

QPair<QString, QString> ScriptEditTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> ScriptEditTool::GetToolName()
{
	return 	QPair<QString, QString>("Script", tr("Script"));
}

int ScriptEditTool::print(lua_State * L)
{
	int n = lua_gettop(L);		/* number of arguments */
	int i;
	QString strMsg;
	for (i = 1; i <= n; i++)
	{
		size_t l;
		int itype = lua_type(L, i);
		switch (itype)
		{
		case LUA_TSTRING:
		{
			const char * strchar = lua_tolstring(L, i, &l);
			strMsg.append(QString::fromLocal8Bit(strchar));
		}
		break;
		case LUA_TBOOLEAN:
		{
			strMsg.append(lua_toboolean(L, i) ? "true" : "false");
		}
		break;
		case LUA_TNUMBER:
		{
			strMsg.append(QString::number(lua_tonumber(L, i)));
		}
		break;
		case LUA_TTABLE:	//列表
		{
			QsciTableInfo t;
			list_table(L, i, t, 1 - 1);
			strMsg.append(QString::fromLocal8Bit(table_as_string(t, 10).c_str()));
		}
		break;
		default:
			break;
		}
	}
	qDebug() << strMsg;

	return 0;
}

int ScriptEditTool::getDateTime(lua_State * L)
{
	int n = lua_gettop(L);
	if (n <= 0)	return 0;
	size_t l;	const char *s = luaL_tolstring(L, 1, &l);	/* convert it to string */
	std::string value = QDateTime::currentDateTime().toString(s).toStdString();
	if(value.empty())	luaL_error(L, "QDateTime Format Error!");
	lua_pushstring(L, value.data());
	return 1;
}

int ScriptEditTool::getValue(lua_State * L)
{
	return 0;
}

int ScriptEditTool::setValue(lua_State * L)
{
	return 0;
}

int ScriptEditTool::setGlobalValue(lua_State * L)
{
	int n				= lua_gettop(L);
	if (n <= 0)			return 0;
	size_t l;
	const char *s		= luaL_tolstring(L, 1, &l);
	QString strKey		= QString::fromLocal8Bit(s);
	const char *s1		= luaL_tolstring(L, 2, &l);
	QString strValue	= QString::fromLocal8Bit(s1);

	//将参数转化为GlobalVar
	if ((*MiddleParam::Instance()).MapInt.count(strKey) > 0)
	{
		(*MiddleParam::Instance()).MapInt[strKey] = strValue.toInt();
	}
	else if ((*MiddleParam::Instance()).MapDou.count(strKey) > 0)
	{
		(*MiddleParam::Instance()).MapDou[strKey] = strValue.toDouble();
	}
	else if ((*MiddleParam::Instance()).MapStr.count(strKey) > 0)
	{
		(*MiddleParam::Instance()).MapStr[strKey] = strValue;
	}
	else if ((*MiddleParam::Instance()).MapBool.count(strKey) > 0)
	{
		(*MiddleParam::Instance()).MapBool[strKey] = strValue.toInt();
	}
	else if ((*MiddleParam::Instance()).MapPoint.count(strKey) > 0)
	{
		QStringList strLst = strValue.split(",");
		QPoint pt;
		if (strLst.size() > 0)	pt.setX(strLst[0].toInt());
		if (strLst.size() > 1)	pt.setY(strLst[1].toInt());
		else luaL_error(L, "MapPoint split Error");
		(*MiddleParam::Instance()).MapPoint[strKey] = pt;
	}
	else if ((*MiddleParam::Instance()).MapPointF.count(strKey) > 0)
	{
		QStringList strLst = strValue.split(",");
		QPointF pt;
		if (strLst.size() > 0)	pt.setX(strLst[0].toDouble());
		if (strLst.size() > 1)	pt.setY(strLst[1].toDouble());
		else luaL_error(L, "MapPointF split Error");
		(*MiddleParam::Instance()).MapPointF[strKey] = pt;
	}
	else
	{
		luaL_error(L, (QString("setGlobalValue No Such Key %1").arg(strKey)).toLocal8Bit().data());
	}

	return 0;
}

int ScriptEditTool::getGlobalValue(lua_State * L)
{
	int n = lua_gettop(L);
	if (n <= 0)			return 0;
	size_t l;
	const char *s		= luaL_tolstring(L, 1, &l);	/* convert it to string */
	QString strKey		= QString::fromLocal8Bit(s);
	QString strValue;

	//将参数转化为GlobalVar
	if ((*MiddleParam::Instance()).MapInt.count(strKey) > 0)
	{
		strValue = QString::number((*MiddleParam::Instance()).MapInt[strKey]);
	}
	else if ((*MiddleParam::Instance()).MapDou.count(strKey) > 0)
	{
		strValue = QString::number((*MiddleParam::Instance()).MapDou[strKey]);
	}
	else if ((*MiddleParam::Instance()).MapStr.count(strKey) > 0)
	{
		strValue = (*MiddleParam::Instance()).MapStr[strKey];
	}
	else if ((*MiddleParam::Instance()).MapBool.count(strKey) > 0)
	{
		strValue = QString::number((*MiddleParam::Instance()).MapBool[strKey]);
	}
	else if ((*MiddleParam::Instance()).MapPoint.count(strKey) > 0)
	{
		QPoint& pt = (*MiddleParam::Instance()).MapPoint[strKey];
		strValue = QString("%1,%2").arg(QString::number(pt.x())).arg(QString::number(pt.y()));
	}
	else if ((*MiddleParam::Instance()).MapPointF.count(strKey) > 0)
	{
		QPointF& pt = (*MiddleParam::Instance()).MapPointF[strKey];
		strValue = QString("%1,%2").arg(QString::number(pt.x())).arg(QString::number(pt.y()));
	}
	else
	{
		luaL_error(L, (QString("getGlobalValue No Such Key %1").arg(strKey)).toLocal8Bit().data());

	}
	if (strValue.isEmpty())	return 0;
	lua_pushstring(L, strValue.toLocal8Bit().data());

	return 1;
}

int ScriptEditTool::setLocalValue(lua_State * L)
{
	int n					= lua_gettop(L);
	if (n <= 0)				return 0;
	ScriptEditTool* _PUser	= (ScriptEditTool*)L->user_param;

	size_t l;
	const char *s			= luaL_tolstring(L, 1, &l);	/* convert it to string */
	QString strKey			= QString(s);

	const char *s1			= luaL_tolstring(L, 2, &l);
	QString strValue		= QString::fromLocal8Bit(s1);

	if (_PUser->m_PtrParam == nullptr)
	{
		luaL_error(L, (QString("setLocalValue MiddleParam Empty %1").arg(strKey)).toLocal8Bit().data());
		return 0;
	}

	//将参数转化为GlobalVar
	if (_PUser->m_PtrParam->MapInt.count(strKey) > 0)	{
		_PUser->m_PtrParam->MapInt[strKey] = strValue.toInt();
	}
	else if (_PUser->m_PtrParam->MapDou.count(strKey) > 0)	{
		_PUser->m_PtrParam->MapDou[strKey] = strValue.toDouble();
	}
	else if (_PUser->m_PtrParam->MapStr.count(strKey) > 0)	{
		_PUser->m_PtrParam->MapStr[strKey] = strValue;
	}
	else if (_PUser->m_PtrParam->MapBool.count(strKey) > 0)	{
		_PUser->m_PtrParam->MapBool[strKey] = strValue.toInt();
	}
	else if (_PUser->m_PtrParam->MapPoint.count(strKey) > 0)	{
		QStringList strLst = strValue.split(",");
		QPoint pt;
		if (strLst.size() > 0)	pt.setX(strLst[0].toInt());
		if (strLst.size() > 1)	pt.setY(strLst[1].toInt());
		else luaL_error(L, "MapPoint split Error");
		_PUser->m_PtrParam->MapPoint[strKey] = pt;
	}
	else if (_PUser->m_PtrParam->MapPointF.count(strKey) > 0)	{
		QStringList strLst = strValue.split(",");
		QPointF pt;
		if (strLst.size() > 0)	pt.setX(strLst[0].toDouble());
		if (strLst.size() > 1)	pt.setY(strLst[1].toDouble());
		else luaL_error(L, "MapPointF split Error");
		_PUser->m_PtrParam->MapPointF[strKey] = pt;
	}
	else	{
		luaL_error(L, (QString("setLocalValue No Such Key %1").arg(strKey)).toLocal8Bit().data());
	}
	return 0;
}

int ScriptEditTool::getLocalValue(lua_State * L)
{
	int n				= lua_gettop(L);
	if (n <= 0)			return 0;
	ScriptEditTool* _PUser = (ScriptEditTool*)L->user_param;

	size_t l;
	const char *s		= luaL_tolstring(L, 1, &l);	/* convert it to string */
	QString strKey		= QString::fromLocal8Bit(s);
	QString strValue;
	if (_PUser->m_PtrParam == nullptr)
	{
		luaL_error(L, (QString("getLocalValue MiddleParam Empty %1").arg(strKey)).toLocal8Bit().data());
		return 0;
	}

	//将参数转化为GlobalVar
	//将参数转化为GlobalVar
	if (_PUser->m_PtrParam->MapInt.count(strKey) > 0)
	{
		strValue = QString::number(_PUser->m_PtrParam->MapInt[strKey]);
	}
	else if (_PUser->m_PtrParam->MapDou.count(strKey) > 0)
	{
		strValue = QString::number(_PUser->m_PtrParam->MapDou[strKey]);
	}
	else if (_PUser->m_PtrParam->MapStr.count(strKey) > 0)
	{
		strValue = _PUser->m_PtrParam->MapStr[strKey];
	}
	else if (_PUser->m_PtrParam->MapBool.count(strKey) > 0)
	{
		strValue = QString::number(_PUser->m_PtrParam->MapBool[strKey]);
	}
	else if (_PUser->m_PtrParam->MapPoint.count(strKey) > 0)
	{
		QPoint& pt = _PUser->m_PtrParam->MapPoint[strKey];
		strValue = QString("%1,%2").arg(QString::number(pt.x())).arg(QString::number(pt.y()));
	}
	else if (_PUser->m_PtrParam->MapPointF.count(strKey) > 0)
	{
		QPointF& pt = _PUser->m_PtrParam->MapPointF[strKey];
		strValue = QString("%1,%2").arg(QString::number(pt.x())).arg(QString::number(pt.y()));
	}
	else
	{
		luaL_error(L, (QString("getLocalValue No Such Key %1").arg(strKey)).toLocal8Bit().data());
	}
	if (strValue.isEmpty())	return 0;
	lua_pushstring(L, strValue.toLocal8Bit().data());
	return 1;
}

int ScriptEditTool::split(lua_State * L)		//字符串结构
{
	size_t _iLenth;
	const char *strValue = luaL_checklstring(L, 1, &_iLenth);	//检查名称
	size_t ld;
	const char *delimite = luaL_checklstring(L, 2, &ld);
	char buf[1024] = { 0 };
	memset(buf, 0, sizeof(char) * 1024);
	lua_newtable(L);   //为lua传表 需要先创建个表
	const char* token = strtok((char*)strValue, delimite);
	int _index = 0;
	while (token != NULL)
	{
		lua_pushnumber(L, _index);  //往栈里面压入i
		lua_pushstring(L, token);		//value
		lua_settable(L, -3);//弹出key,value，并设置到table里面去
		token = strtok(NULL, delimite);
		_index++;
	}
	return 1;
}

int ScriptEditTool::LogInfo(lua_State * L)
{
	int n = lua_gettop(L);		/* number of arguments */
	int i;
	QString strMsg;
	for (i = 1; i <= n; i++)
	{
		size_t l;
		int itype = lua_type(L, i);
		switch (itype)
		{
		case LUA_TSTRING:
		{
			const char * strchar = lua_tolstring(L, i, &l);
			strMsg.append(QString::fromLocal8Bit(strchar));
		}
		break;
		case LUA_TBOOLEAN:
		{
			strMsg.append(lua_toboolean(L, i) ? "true" : "false");
		}
		break;
		case LUA_TNUMBER:
		{
			strMsg.append(QString::number(lua_tonumber(L, i)));
		}
		break;
		case LUA_TTABLE:	//列表
		{
			QsciTableInfo t;
			list_table(L, i, t, 1 - 1);
			strMsg.append(QString::fromLocal8Bit(table_as_string(t, 10).c_str()));
		}
		break;
		default:
			break;
		}
	}
	qInfo() << strMsg;
	return 0;
}

int ScriptEditTool::LogWarn(lua_State * L)
{
	int n = lua_gettop(L);		/* number of arguments */
	int i;
	QString strMsg;
	for (i = 1; i <= n; i++)
	{
		size_t l;
		int itype = lua_type(L, i);
		switch (itype)
		{
		case LUA_TSTRING:
		{
			const char * strchar = lua_tolstring(L, i, &l);
			strMsg.append(QString::fromLocal8Bit(strchar));
		}
		break;
		case LUA_TBOOLEAN:
		{
			strMsg.append(lua_toboolean(L, i) ? "true" : "false");
		}
		break;
		case LUA_TNUMBER:
		{
			strMsg.append(QString::number(lua_tonumber(L, i)));
		}
		break;
		case LUA_TTABLE:	//列表
		{
			QsciTableInfo t;
			list_table(L, i, t, 1 - 1);
			strMsg.append(QString::fromLocal8Bit(table_as_string(t, 10).c_str()));
		}
		break;
		default:
			break;
		}
	}
	qWarning() << strMsg;
	return 0;
}

int ScriptEditTool::LogError(lua_State * L)
{
	int n = lua_gettop(L);		/* number of arguments */
	int i;
	QString strMsg;
	for (i = 1; i <= n; i++)
	{
		size_t l;
		int itype = lua_type(L, i);
		switch (itype)
		{
		case LUA_TSTRING:
		{
			const char * strchar = lua_tolstring(L, i, &l);
			strMsg.append(QString::fromLocal8Bit(strchar));
		}
		break;
		case LUA_TBOOLEAN:
		{
			strMsg.append(lua_toboolean(L, i) ? "true" : "false");
		}
		break;
		case LUA_TNUMBER:
		{
			strMsg.append(QString::number(lua_tonumber(L, i)));
		}
		break;
		case LUA_TTABLE:	//列表
		{
			QsciTableInfo t;
			list_table(L, i, t, 1 - 1);
			strMsg.append(QString::fromLocal8Bit(table_as_string(t, 10).c_str()));
		}
		break;
		default:
			break;
		}
	}
	qCritical() << strMsg;
	return 0;
}

int ScriptEditTool::sendData(lua_State * L)
{
	int n = lua_gettop(L);		/* number of arguments */
	if (n <= 1)	{
		luaL_error(L, tr("No Such Net").toStdString().c_str());
		return 0;
	}
	size_t l;
	const char * s = lua_tolstring(L, 1, &l);
	QString	strKey = QString::fromLocal8Bit(s);
	const char * s2 = lua_tolstring(L, 2, &l);
	QString	strValue = QString::fromLocal8Bit(s2);
	int iRetn = gVariable::Instance().SendMsg(strKey, strValue);
	if (iRetn > 0)	{

	}
	else if (iRetn == 0)	{
		luaL_error(L, (tr("Net") + QString(" %1").arg(strKey) + tr(" Isn't Connected")).toStdString().c_str());
	}
	else	{
		luaL_error(L, (tr("No Such Net") + QString(" %1").arg(strKey) ).toStdString().c_str());
	}
	return 0;
}

int ScriptEditTool::RecieveData(lua_State * L)
{
	int n = lua_gettop(L);		/* number of arguments */
	if (n <= 1) {
		luaL_error(L, tr("No Such Net").toStdString().c_str());
		return 0;
	}
	size_t l;
	const char * s = lua_tolstring(L, 1, &l);
	QString	strKey = QString::fromLocal8Bit(s);
	const char * s2 = lua_tolstring(L, 2, &l);
	gVariable::Instance().ClearRecieve();
	QString strMsg;
	int iRetn = gVariable::Instance().GetMsg(strKey, strMsg,QString::fromLocal8Bit(s2).toDouble());
	if (iRetn > 0) {

	}
	else if (iRetn == 0) {
		luaL_error(L, (tr("Net") + QString(" %1").arg(strKey) + tr(" Isn't Connected")).toStdString().c_str());
	}
	else if (iRetn == -1) {
		luaL_error(L, (tr("Net") + QString(" %1").arg(strKey) + tr(" Time Out")).toStdString().c_str());
	}
	else {
		luaL_error(L, (tr("No Such Net") + QString(" %1").arg(strKey)).toStdString().c_str());
	}
	lua_pushstring(L, strMsg.toLocal8Bit().data());
	return 1;
}

int ScriptEditTool::RunCall(lua_State * L)
{
	int n = lua_gettop(L);		/* number of arguments */
	if (n <= 1) {
		luaL_error(L, tr("No Such Net").toStdString().c_str());
		return 0;
	}
	size_t l;
	const char * s = lua_tolstring(L, 1, &l);
	QString	strFlowKey = QString::fromLocal8Bit(s);
	int _iFindIndex = -1;
	for (size_t i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++)	{
		if (databaseVar::Instance().m_vecPtr[i]->m_strName.second == strFlowKey)	{
			_iFindIndex = i;
		}
	}
	if (_iFindIndex != -1){
		if (databaseVar::Instance().m_vecPtr[_iFindIndex]->m_bEnable){
			luaL_error(L, (tr("Flow ") + QString("%1").arg(strFlowKey)+ tr(" Is Enable ")	).toLocal8Bit().data());
			return 0;
		}
		else if (((ScriptEditTool*)L->user_param)->m_iFlowIndex > MiddleParam::MapMiddle().size())	{
			luaL_error(L, (tr("Flow ") + QString("%1").arg(strFlowKey) + tr(" Is Error ")).toLocal8Bit().data());
		}
		else	{
			QString strError;
			return databaseVar::Instance().m_vecPtr[_iFindIndex]->m_pNodeManager->Execute(MiddleParam::MapMiddle()[	((ScriptEditTool*)L->user_param)->m_iFlowIndex	], strError);
		}
	}
	else {
		luaL_error(L, (tr("Flow ") + QString("%1").arg(strFlowKey) + tr(" Didn't Exist ")).toLocal8Bit().data());
	}

	return 0;
}
