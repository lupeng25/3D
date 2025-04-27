#pragma once
#include <vector>
#include <string>
#include <map>
#include "Defs.h"
#include "lua/lua.hpp"
#include <mutex>
#include <assert.h>
#include <exception>
#include <Windows.h>

#define NoName "NoName"

typedef std::vector<unsigned char> ProgBuf;
enum Event
{
	Start,
	Running,
	NewLine,
	Finished
};
enum RunMode 
{ 
	StepLine,	//逐行
	StepInto,	//逐语句
	StepOut,	//跳出
	DebugRun,	//调试运行
	AutoRun,	//自动运行
	NoThreadRun, //无线程运行
};
class lua_exception : public std::exception
{
public:
	lua_exception(const char* msg):exception(msg)
	{}
};
class LEvent
{
public:
	void Create(BOOL bManualReset, BOOL bInitialState)
	{
		h_Event = CreateEventW(NULL, bManualReset, bInitialState, NULL);
	}
	void Realse()
	{
		CloseHandle(h_Event);
	}
	void SetEvent()
	{
		::SetEvent(h_Event);
	}
	void ResetEvent()
	{
		::ResetEvent(h_Event);
	}
	LEvent()
	{
		h_Event = NULL;
	}
	~LEvent()
	{
		CloseHandle(h_Event);
	}
public:
	HANDLE h_Event;
};

struct STabBreakPoint
{
	std::string strTab;
	std::map<int, Defs::Breakpoint>				breakpoints;			//断点问题
	STabBreakPoint()
	{
		strTab = NoName;
		breakpoints.clear();
	}
};

struct QsciStackFrame
{
	QsciStackFrame();
	void Clear();
	const char* SourcePath() const;
	enum Entry { Err, LuaFun, MainChunk, CFun, TailCall } type;
	std::string source;
	std::string name_what;
	int current_line;	// 1..N or 0 if not available
						// where it is defined (Lua fn)
	int line_defined;
	int last_line_defined;
};
// 
typedef std::vector<QsciStackFrame> QsciCallStack;

//static void fill_frame(const lua_Debug& dbg, QsciStackFrame& frame);

class LLState
{
public:
	LLState();
	virtual ~LLState();
	void Init();
	std::string GetCurrentFunCallStack() const;
public:
	void Start();
	void Stop();
	void go(RunMode mode);
	void gobreak();
	int create_thread();
	void RegisterFunction(std::string name,lua_CFunction fun);
	static DWORD WINAPI exec_thread(LPVOID param);
	static void exec_hook_function(lua_State* L, lua_Debug* ar);
	//void exec_hook(lua_State* L, lua_Debug* dbg);
	void line_hook(lua_State* L,	lua_Debug* dbg);
	void count_hook(lua_State* L,	lua_Debug* dbg);
	void call_hook(lua_State* L,	lua_Debug* dbg);
	void ret_hook(lua_State* L,		lua_Debug* dbg);
	void suspend_exec(lua_Debug* dbg, bool forced);
	void notify(Event ev, lua_Debug* dbg,std::string str = "");
	bool is_execution_finished() const { return ::WaitForSingleObject(thread_, 0) != WAIT_TIMEOUT; }
	bool is_data_available() const;
	//bool breakpoint_at_line(int line);
	//bool toggle_breakpoint(int line);
	//bool toggle_Operatorbreakpoint(int line,bool bIsAdd = false);
	//void clearbreakpoint();
public:
	void toggle_ClearMapbreakpoint();

	void toggle_OperatorMapbreakpoint(std::string,int line);
	void toggle_OperatorLineMapbreakpoint(std::string,int line,bool bIsAdd = false);

	void toggle_SetMapbreakpoint(std::map<std::string, std::map<int, Defs::Breakpoint>>& BreakPoint);
public:
	lua_State*												L;
	HANDLE													thread_;				//线程句柄
	LEvent													step_event_;			//单步事件
	LEvent													start_event_;			//开始事件
	bool													abort_flag_;
	bool													break_flag_;
	std::function<void(void*,Event, int,char*)>				callback_;
	void*													m_PtrUser;
	std::function<void(void*,void*)>						m_BreakFuncall;
	void*													m_BreakUser;
	std::string												status_msg_;
	bool													status_ready_;

	//设置访问Tab的函数
	std::function<int(void*,std::string,std::string&)>		m_TabFunCall;
	void*													m_TabUser;

	std::map<std::string,std::map<int, Defs::Breakpoint>>	m_Mapbreakpoints_;		//带Tab的断点
	//std::map<int,Defs::Breakpoint>							breakpoints_;			//断点问题
	std::mutex												breakpoints_lock_;		//断点锁
	int														func_call_level_;		//当前的调用的函数层次
	int														stop_at_level_;			//当前停止的函数层次
	bool													is_running_;			//正在运行
	bool													m_bIsAutoRun;			//当前正在自动运行
	bool													m_bIsTaskFinished;		//状态正在运行
	bool													m_bIsTaskFinishedByWait;		//状态正在运行
	std::string												m_CurrentTab;

	std::vector<std::string>								m_vecFunCaBack;			//当前函数堆载名称
private:
	void run();
	RunMode run_mode_;
};