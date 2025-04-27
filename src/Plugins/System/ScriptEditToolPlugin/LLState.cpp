#include "LLState.h"
#include "lua/lstate.h"
//#include "lj_obj.h"
//bool LLState::m_bNeedBreak = false;

// fill stack frame variable with info from Lua debug struct
static void fill_frame(const lua_Debug& dbg, QsciStackFrame& frame)
{
	frame.Clear();

	frame.source = dbg.source ? dbg.source : dbg.short_src;

	if (dbg.currentline > 0)
		frame.current_line = dbg.currentline;

	if (dbg.what)
	{
		if (strcmp(dbg.what, "C") == 0)
			frame.type = QsciStackFrame::CFun;
		else if (strcmp(dbg.what, "Lua") == 0)
			frame.type = QsciStackFrame::LuaFun;
		else if (strcmp(dbg.what, "main") == 0)
			frame.type = QsciStackFrame::MainChunk;
		else if (strcmp(dbg.what, "tail") == 0)
			frame.type = QsciStackFrame::TailCall;
		else
			frame.type = QsciStackFrame::Err;
	}
	if (dbg.namewhat != 0 && *dbg.namewhat != '\0')	// is there a name?
		frame.name_what = dbg.name;

	frame.last_line_defined = dbg.lastlinedefined;
	frame.line_defined = dbg.linedefined;
}

LLState::LLState()
{
	step_event_.Create(true, false);
	start_event_.Create(true, false);
	L					= NULL;
	thread_				= 0;
	break_flag_			= abort_flag_ = false;
	func_call_level_	= 0;
	stop_at_level_		= 0;
	run_mode_			= StepLine;
	is_running_			= false;
	status_ready_		= false;
	m_bIsAutoRun		= true;
	m_PtrUser			= nullptr;
	m_bIsTaskFinished	= true;
	m_Mapbreakpoints_.clear();
}

LLState::~LLState()
{
	Stop();
	if (thread_)
	{
		abort_flag_ = true;
		step_event_.SetEvent();
		start_event_.SetEvent();
		::WaitForSingleObject(thread_, 100);
		CloseHandle(thread_);
	}
}

void LLState::Init()
{
	L = luaL_newstate();
	luaL_openlibs(L);  /* open libraries */
}
std::string LLState::GetCurrentFunCallStack() const
{
	if (is_data_available())
		return std::string("");

	std::string funcallstack;

	int level = 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof dbg);
	bool _bCurrentFirst = true;

	while (lua_getstack(L, level++, &dbg))
	{
		if (lua_getinfo(L, "Snl", &dbg) == 0)
		{
			funcallstack = "";
			break;
		}

		if (dbg.what)
		{
			if (dbg.namewhat != 0 && *dbg.namewhat != '\0' && _bCurrentFirst)	// is there a name?
			{
				funcallstack.assign(dbg.name);
				_bCurrentFirst = false;
			}
		}
	}
	return funcallstack.c_str();
}


int LLState::create_thread()
{
	if (!is_execution_finished())  return -1;	//当前线程没有执行完成
	thread_ = ::CreateThread(NULL, 0, LLState::exec_thread, this, CREATE_SUSPENDED, NULL);
	::ResumeThread(thread_);
	return 0;
}

void LLState::Start()
{
	break_flag_ = abort_flag_ = false;
	//m_bNeedBreak = false;
}

void LLState::Stop()
{
	abort_flag_ = true;
	step_event_.SetEvent();
	start_event_.SetEvent();
}
static bool m_isThread = true;
void LLState::go(RunMode mode)
{
	if (is_execution_finished()) return;
	run_mode_		= mode;
	break_flag_		= false;
	//m_bNeedBreak	= false;
	stop_at_level_	= func_call_level_;
	switch (mode)
	{
	case StepLine:
		step_event_.SetEvent();
		start_event_.SetEvent();
		break;
	case StepInto:
		step_event_.SetEvent();
		start_event_.SetEvent();
		break;
	case StepOut:
		step_event_.SetEvent();
		start_event_.SetEvent();
		break;
	case DebugRun:
		start_event_.SetEvent();
		step_event_.SetEvent();
		break;
	case AutoRun:							//当前为自动运行
		lua_sethook(L, NULL, NULL, 1);			//清空触发
		start_event_.SetEvent();
		step_event_.SetEvent();
		break;
	case NoThreadRun:
		run();
		m_isThread = false;
	default:
		break;
	}
}

void LLState::gobreak()
{
	break_flag_ = true;
}

void LLState::RegisterFunction(std::string name,lua_CFunction fun)
{
	lua_register(L,name.c_str(), fun);
}

DWORD WINAPI LLState::exec_thread(LPVOID param)
{
	LLState* state = static_cast<LLState*>(param);
	try
	{
		WaitForSingleObject(state->start_event_.h_Event, INFINITE); //无限时间等待
		if (!state->abort_flag_)
			state->run();
	}
	catch (...) 
	{
		if (state->abort_flag_)
		{
			state->m_bIsTaskFinished = true;
			state->notify(Event::Finished, 0);
		}
		assert(false);	
		return 1; 
	}
	if (state->abort_flag_)
	{
		state->m_bIsTaskFinished = true;
		state->notify(Event::Finished, 0);
	}
	return 0;
}

void LLState::run()
{
	is_running_ = true;
	//lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
	//luaL_openlibs(L);  /* open libraries */
	if(!m_isThread)
		notify(Event::Start, 0);
	m_bIsTaskFinished = false;
	m_bIsTaskFinishedByWait = false;
	//lua_gc(L, LUA_GCCOLLECT, 0);

	// correct the level: we are entering function (main chunk) now
	stop_at_level_ = func_call_level_ + 1;
	if (run_mode_ == AutoRun || run_mode_ == NoThreadRun)
		lua_sethook(L, NULL, NULL, 1);			//清空触发
	else
		lua_sethook(L, &LLState::exec_hook_function, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);

	int narg = 0;
	//int base= lua_gettop(L) - narg;  // function index
	int err_fn = 0;
	int status = lua_pcall(L, narg, LUA_MULTRET, err_fn);
	if (abort_flag_)
		status_msg_ = "Aborted";
	else if (status)
		status_msg_ = lua_tostring(L, -1);
	else
		status_msg_ = "Finished";
	status_ready_ = true;	// it's ok to read 'status_msg_' from different thread now
	m_bIsTaskFinishedByWait = true;
	if (!abort_flag_)
		if (status != 0)
		{
			m_bIsTaskFinished = true;
			notify(Event::Finished, 0, status_msg_);
		}
		else
		{
			m_bIsTaskFinished = true;
			notify(Event::Finished, 0, status_msg_);
		}
}

bool LLState::is_data_available() const
{
	if (is_execution_finished())
		return false;
	return !is_running_;
}

void LLState::notify(Event ev, lua_Debug* dbg, std::string str)
{
	if (callback_)
	{
		try
		{
			callback_(m_PtrUser,ev, dbg != 0 ? dbg->currentline : -1,(char*)str.c_str());
		}
		catch (...) { }
	}
}

//bool LLState::breakpoint_at_line(int line)
//{
//	std::unique_lock<std::mutex>_lock(breakpoints_lock_, std::defer_lock);
//	std::map<int, Defs::Breakpoint>::const_iterator it = breakpoints_.find(line);
//	if (it == breakpoints_.end())
//		return false;
//	return (it->second & Defs::BPT_MASK) == Defs::BPT_EXECUTE;
//}

//bool LLState::toggle_breakpoint(int line)
//{
//	std::unique_lock<std::mutex>_lock(breakpoints_lock_, std::defer_lock);
//	if (breakpoint_at_line(line))
//	{
//		breakpoints_.erase(line);
//		return false;
//	}
//	else
//	{
//		breakpoints_[line] = Defs::BPT_EXECUTE;
//		return true;
//	}
//}

//bool LLState::toggle_Operatorbreakpoint(int line, bool bIsAdd)
//{
//	std::unique_lock<std::mutex>_lock(breakpoints_lock_,std::defer_lock);
//	if (breakpoint_at_line(line))
//	{
//		if (bIsAdd)
//			return true;
//		breakpoints_.erase(line);
//		return false;
//	}
//	else
//	{
//		if (!bIsAdd)
//		{
//			breakpoints_.erase(line);
//			return true;
//		}
//		breakpoints_[line] = Defs::BPT_EXECUTE;
//		return true;
//	}
//}

//void LLState::clearbreakpoint()
//{
//	std::unique_lock<std::mutex>_lock(breakpoints_lock_, std::defer_lock);
//	breakpoints_.clear();
//}

void LLState::toggle_ClearMapbreakpoint()
{
	std::unique_lock<std::mutex>_lock(breakpoints_lock_, std::defer_lock);
	m_Mapbreakpoints_.clear();
}

void LLState::toggle_OperatorMapbreakpoint(std::string tab, int line)
{
	std::unique_lock<std::mutex>_lock(breakpoints_lock_, std::defer_lock);
	//if (m_Mapbreakpoints_.count(tab))
	{
		if (m_Mapbreakpoints_[tab].count(line + 1))
		{
			m_Mapbreakpoints_[tab].erase(line + 1);
		}
		else
		{
			m_Mapbreakpoints_[tab][line + 1] = Defs::BPT_EXECUTE;
		}
	//}
	//else
	//{
		//m_Mapbreakpoints_[tab];
		//if (m_Mapbreakpoints_[tab].count(line + 1))
		//{
		//	m_Mapbreakpoints_[tab].erase(line + 1);
		//}
		//else
		//{
		//	m_Mapbreakpoints_[tab][line + 1] = Defs::BPT_EXECUTE;
		//}
	}
}

void LLState::toggle_OperatorLineMapbreakpoint(std::string tab, int line,bool bIsAdd)
{
	std::unique_lock<std::mutex>_lock(breakpoints_lock_, std::defer_lock);
	m_Mapbreakpoints_[tab];

	if (m_Mapbreakpoints_[tab].count(line + 1))
	{
		if (!bIsAdd)
		{
			m_Mapbreakpoints_[tab].erase(line + 1);
		}
	}
	else
	{
		if (bIsAdd)
			m_Mapbreakpoints_[tab][line + 1] = Defs::BPT_EXECUTE;
	}

}

void LLState::toggle_SetMapbreakpoint(std::map<std::string, std::map<int, Defs::Breakpoint>>& BreakPoint)
{
	std::unique_lock<std::mutex>_lock(breakpoints_lock_, std::defer_lock);
	m_Mapbreakpoints_.clear();
	m_Mapbreakpoints_ = BreakPoint;
}

void LLState::exec_hook_function(lua_State* L, lua_Debug* dbg)
{
	LLState* state = (LLState*)((L)->user_param);
	if (state->abort_flag_)
	{
		lua_error(state->L);		// abort now
		return;
	}
	switch (dbg->event)
	{
	case LUA_HOOKCOUNT:
		state->count_hook(L, dbg);
		break;
	case LUA_HOOKCALL:
		state->call_hook(L, dbg);
		break;
	case LUA_HOOKRET:
		state->ret_hook(L, dbg);
		break;
	case LUA_HOOKLINE:
		state->line_hook(L, dbg);
		break;
	}
}

void LLState::count_hook(lua_State* L, lua_Debug*)
{
	if (break_flag_)	//需要中断？
	{
		lua_Debug dbg;
		memset(&dbg, 0, sizeof(dbg));
		dbg.currentline = -1;
		if (lua_getstack(L, 0, &dbg))
		{
			// retrieve current line number, count hook doesn't provide it
			int stat = lua_getinfo(L, "l", &dbg);
			if (stat == 0)
				dbg.currentline = -1;
		}
		// break signaled; stop
		suspend_exec(&dbg, true);
	}
}

//调用函数堆载
void LLState::call_hook(lua_State* L, lua_Debug* dbg)
{
	func_call_level_++;
	m_vecFunCaBack.push_back(GetCurrentFunCallStack());
}

//返回当前函数堆载
void LLState::ret_hook(lua_State* L, lua_Debug* dbg)
{
	func_call_level_--;
	m_vecFunCaBack.pop_back();
}

void LLState::line_hook(lua_State* L, lua_Debug* dbg)
{
	if (break_flag_)														{
		suspend_exec(dbg, true);
	}
	else if (run_mode_ == StepInto)											{
		if (stop_at_level_ >= func_call_level_)		// 'step over' done?
			suspend_exec(dbg, true);				// stop now
	}
	else if (run_mode_ == StepOut)											{
		if (stop_at_level_ > func_call_level_)		// 'step out' done?
			suspend_exec(dbg, true);				// stop now
	}
	else if (run_mode_ == DebugRun)											{		// run without delay?
		// check breakpoints
		if (m_vecFunCaBack.size() > 0)										{
			//获取当前的函数的文件
			std::string strFunction = m_vecFunCaBack.back();
			std::string strTab;
			if (m_TabFunCall != nullptr)									{
				if (strFunction == "")										{
					if (m_Mapbreakpoints_[m_CurrentTab].count(dbg->currentline))	//当前的是否包含相关的断点
						suspend_exec(dbg, true);									// stop now; there's a breakpoint at the current line
				}
				else														{
					if (m_TabFunCall(m_TabUser, strFunction, strTab) == 0)	{
						if (m_Mapbreakpoints_[strTab].count(dbg->currentline))		//当前的是否包含相关的断点
							suspend_exec(dbg, true);								// stop now; there's a breakpoint at the current line
					}
				}
			}
		}
	}
	else
		suspend_exec(dbg, false);				// line-by-line execution, so stop
}

void LLState::suspend_exec(lua_Debug* dbg, bool forced)
{
	if (forced)
		step_event_.ResetEvent();
	// step by step execution
	//执行单步运行
	if (::WaitForSingleObject(step_event_.h_Event, 0) != WAIT_OBJECT_0)		{		// blocked?
		is_running_ = false;
		if (m_BreakFuncall != nullptr)
			m_BreakFuncall(m_BreakUser, m_PtrUser);
	}
	notify(NewLine, dbg);
	::WaitForSingleObject(step_event_.h_Event, INFINITE);
	if (abort_flag_)			{
		lua_error(L);		// abort now
		return;
	}
	is_running_ = true;
	step_event_.ResetEvent();
}