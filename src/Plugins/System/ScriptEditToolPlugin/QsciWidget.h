#pragma once
#include <Qsci\qsciscintilla.h>
#include <Qsci\qscilexercustom.h>
#include <Qsci/QsciAPIs.h>
#include <Qsci/qscilexerlua.h>
#include <functional>
#include <qvector.h>
#include "LLState.h"

enum QsciValType { Val_None = -1, Val_Nil, Val_Bool, Val_LightUserData, Val_Number, Val_String, Val_Table, Val_Function, Val_UserData, Val_Thread };
struct QsciValue	// value on a virtual stack or elsewhere
{
	QsciValue() : type(Val_None), type_name(0)
	{}
	QsciValType type;
	const char* type_name;
	std::string value;	// simplified string representation of value
};

struct QsciVar
{
	std::string name;	// variable's identifier
	QsciValue v;
};
typedef std::vector<QsciValue> QsciValueStack;

struct QsciField	// table entry
{
	QsciValue key;
	QsciValue val;
};
typedef std::vector<QsciField> QsciTableInfo;

class pop_stack_elements
{
public:
	pop_stack_elements(lua_State* L, int num) : L_(L), num_(num)
	{}
	void dec()
	{
		--num_;
	}
	~pop_stack_elements()
	{
		try
		{
			if (num_ > 0)
				lua_pop(L_, num_);
		}
		catch (...)
		{
		}
	}
private:
	lua_State* L_;
	int num_;
};

void capture_value(lua_State* L, QsciValue& v, int index, int recursive = 0, size_t table_size_limit = 10);
static bool list_table(lua_State* L, int idx, QsciTableInfo& out, int recursive = 0)
{
	out.clear();

	if (lua_type(L, idx) != LUA_TTABLE)
		return false;

	int size = lua_rawlen(L, idx);

	out.reserve(size);

	//table to traverse
	lua_pushvalue(L, idx);

	//push a key
	lua_pushnil(L);

	pop_stack_elements pop(L, 2);	// remove key & table off the stack at the end of this fn
									//pop_stack_elements pop(L, 1);	// remove table off the stack at the end of this fn

	int table = lua_gettop(L) - 1;

	//traverse a table
	while (lua_next(L, table))
	{
		pop_stack_elements pop(L, 1);
		QsciField field;
		capture_value(L, field.key, -2);
		capture_value(L, field.val, -1, recursive);

		out.push_back(field);
	}

	pop.dec();	// final lua_next call removed key

	return true;
}

static std::string table_as_string(const QsciTableInfo& table, size_t limit = 10)
{
	std::string ost;
	ost.append("{ ");
	const size_t count = table.size();
	for (size_t i = 0; i < count; ++i)
	{
		const QsciField& f = table[i];
		if (i > 0)
			ost.append(", ");
		ost.append(f.key.value).append(" = ").append(f.val.value);
		if (i + 1 == limit)
		{
			ost.append(", ... ");
			break;
		}
	}
	if (count > 0)
		ost.append(" ");
	ost.append("}");
	return ost.c_str();
}
static char* to_pointer(char* buffer, const void* ptr)
{
	sprintf(buffer, "0x%p", ptr);
	return buffer;
}
//store information about Lua value present at the 'index' inside 'v' struct
static bool list_virtual_stack(lua_State* L, QsciValueStack& stack, size_t table_size_limit)
{
	int size = lua_gettop(L);

	stack.clear();
	stack.reserve(size);
	for (int idx = size - 1; idx > 0; --idx)
	{
		QsciValue v;
		capture_value(L, v, idx, 1, table_size_limit);
		stack.push_back(v);
	}
	return true;
}
struct Funcs
{
	int iFuncLine;
	int	iCol;
	QString strFuncName;
	Funcs()
	{
		iFuncLine	= -1;
		iCol		= -1;
		strFuncName = NoName;
	}
};

struct ErrorItem
{
	int iErrorCode;
	int iFuncLine;
	int	iCol;
	QString strError;
	ErrorItem()
	{
		iErrorCode = 0;
		iFuncLine = -1;
		iCol = -1;
		strError = "";
	}
};

struct TabItem
{
	int iLine;
	int	iCol;
	QString strTabName;
	TabItem()
	{
		iLine = -1;
		iCol = -1;
		strTabName = NoName;
	}
};

enum EnumError
{
	Error_HaveFun = - 2000,

};

class QsciWidget : public QsciScintilla
{
public:
	QsciWidget(QWidget* parent = nullptr);
	virtual ~QsciWidget();
	void InitDefaultQsci();
	void SetTabName(QString strName);
public:
	static void NotifyCallBack(void*,Event, int, char*);
signals:
	void SendNotifyCallUISignal(Event, int, char*);
	void SendChangeSignal(int type);
public slots:
	void SetNotifyCall(Event, int, char*);
	//执行LUA脚本
public:
	void SymbolLineSignal(Event, int, char*);

	//提前进行编译
	int PreComPile();

	////当前是否有以前的函数
	//bool AlreadHaveFuncs(Funcs& fun);

	// compile
	int	Compile(QString& strError);

public:
	// execute
	int Start(QMap<QString, QVector<int>>& mapMarks);
	//停止
	void Stop();
	//LUA执行过程中中断
	void Gobreak();
	// execute single line (current one) following calls, if any
	void StepLine();
	// execute current line, without entering any functions
	void StepInto();
	// start execution (it runs in a thread)
	void DebugRun();

	void AutoRun(bool useThre = true);
	// run till return from the current function
	void StepOut();
	bool IsRunning();	// is Lua program running now?
	bool IsFinished(int type = 0);	// has Lua program finished execution?
	bool IsStopped();	// if stopped, it can be resumed
	bool IsTaskFinished();	// is Lua program Finished now?
public://断点
	void ClearBreakpoint();
	// 标记SymbolLine
	void ClearSymbolLine(int iline,SymbolHandler Symbol = SymbolHandler::ARROW);
	void MarkGoSymbolLine(int iline, SymbolHandler Symbol = SymbolHandler::ARROW);
	void GotoSymbolLine(int iline, SymbolHandler Symbol = SymbolHandler::ARROW);
	void GetCurLinePosition(int& iline, int& iCol);
	void MarkDeleteSymbolLine(int iline, SymbolHandler Symbol = SymbolHandler::ARROW);

	void ToggleBreakLine(int iline, SymbolHandler Symbol = SymbolHandler::ARROW);

	void ToggleMapBreakLine(QString strTab,int iline,SymbolHandler Symbol = SymbolHandler::ARROW);
public:
	//获取选中行的文本
	QString GetSelectedLine();
	//获取选中的文本
	QString GetSelectedContent();
	//获取断点
	QVector<int> GetBreakPoints();
	void keyPressEvent(QKeyEvent *event);
	void GotoNextLine();
public slots:
	//刷新 Symbol
	void updateSymbol(int, int, Qt::KeyboardModifiers) override;
public:	//回调函数
	void SetUser(void* user)
	{
		m_ptrLLState->m_TabUser = user;
		m_ptrLLState->m_BreakUser = user;
		m_PtrUser = user;
	}
	//通知事件
	void SetNotifyTabEventCall(std::function<void(void*, QString, QString,int)> fun)
	{
		m_NotifyTabEvent = fun;
	};
	void SetGetFunCall(std::function<void(void*, QString, Event, int, char*)> fun)
	{
		m_Getcallback = fun;
	};	
	//设置函数Call
	void SetFunCall(std::function<void(Event, int, char*)> fun)
	{
		m_FunCall = fun;
	};
	void RegisterTabFunCall(std::function<int(void*, std::string, std::string&)> fun)
	{
		m_ptrLLState->m_TabFunCall = fun;
	}
	//设置暂停
	void SetFunBreakCall(std::function<void(void*, void*)> fun)
	{
		m_ptrLLState->m_BreakFuncall = fun;
	};
public:
	void txtChanged();

	//void*									m_PtrTabUser;
public:	//获取堆载
	std::string Status() const;

	// get current call stack
	std::string GetCallStack() const;

	//// get local vars of function at given 'level'
	//bool GetLocalVars(std::vector<QsciVar>& out, int level = 0) const;

	//// get global vars
	//bool GetGlobalVars(QsciTableInfo& out, bool deep) const;

	//// read all values off virtual value stack
	//bool GetValueStack(QsciValueStack& stack) const;

	// get function call stack
	bool GetCallStack(QsciCallStack& stack) const;

	// info about current function and source file (at the top of the stack)
	bool GetCurrentSource(QsciStackFrame& top) const;
public:
	std::function<void(Event, int, char*)>				m_FunCall;
	std::function<void(void*, QString, QString,int)>	m_NotifyTabEvent;
	std::function<void(void*,QString,Event,int,char*)>	m_Getcallback;
	void*												m_PtrUser;
	LLState *			m_ptrLLState;	//
	QString				m_strTab;
	//TabItem				m_LastSymbolLineItem;
	//QVector<Funcs>		m_vecFuncs;
	//QVector<ErrorItem>	m_vecErrorItem;
};