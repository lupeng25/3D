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
	StepLine,	//����
	StepInto,	//�����
	StepOut,	//����
	DebugRun,	//��������
	AutoRun,	//�Զ�����
	NoThreadRun, //���߳�����
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
	std::map<int, Defs::Breakpoint>				breakpoints;			//�ϵ�����
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
	HANDLE													thread_;				//�߳̾��
	LEvent													step_event_;			//�����¼�
	LEvent													start_event_;			//��ʼ�¼�
	bool													abort_flag_;
	bool													break_flag_;
	std::function<void(void*,Event, int,char*)>				callback_;
	void*													m_PtrUser;
	std::function<void(void*,void*)>						m_BreakFuncall;
	void*													m_BreakUser;
	std::string												status_msg_;
	bool													status_ready_;

	//���÷���Tab�ĺ���
	std::function<int(void*,std::string,std::string&)>		m_TabFunCall;
	void*													m_TabUser;

	std::map<std::string,std::map<int, Defs::Breakpoint>>	m_Mapbreakpoints_;		//��Tab�Ķϵ�
	//std::map<int,Defs::Breakpoint>							breakpoints_;			//�ϵ�����
	std::mutex												breakpoints_lock_;		//�ϵ���
	int														func_call_level_;		//��ǰ�ĵ��õĺ������
	int														stop_at_level_;			//��ǰֹͣ�ĺ������
	bool													is_running_;			//��������
	bool													m_bIsAutoRun;			//��ǰ�����Զ�����
	bool													m_bIsTaskFinished;		//״̬��������
	bool													m_bIsTaskFinishedByWait;		//״̬��������
	std::string												m_CurrentTab;

	std::vector<std::string>								m_vecFunCaBack;			//��ǰ������������
private:
	void run();
	RunMode run_mode_;
};