#include "QsciWidget.h"
#include "lua/lua.hpp"
#include "lua/lstate.h"
#include <QKeyEvent>
//#include "lj_obj.h"

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

QsciWidget::QsciWidget(QWidget* parent):QsciScintilla(parent)
{
	InitDefaultQsci();
	m_ptrLLState = new LLState();
	m_ptrLLState->callback_ = NotifyCallBack;
	m_ptrLLState->m_PtrUser = this;
}

QsciWidget::~QsciWidget()
{
	if (m_ptrLLState != nullptr)
		delete[]m_ptrLLState;
	m_ptrLLState = nullptr;
}

void QsciWidget::InitDefaultQsci()
{
	setUtf8(true);
	this->setAutoCompletionSource(QsciScintilla::AcsAll);		//设置源，自动补全所有地方出现的
	this->setAutoCompletionCaseSensitivity(true);				//设置自动补全大小写敏感
	this->setAutoCompletionReplaceWord(true);					//设置自动补全大小写敏感
	this->setAutoCompletionThreshold(2);						//设置每输入2个字符就会出现自动补全的提示
	//设置自动缩进
	this->setAutoIndent(true);
	this->setIndentationGuides(true);
	//selection color
	this->setSelectionBackgroundColor(QColor("#55aaff"));		//选中文本背景色
	this->setSelectionForegroundColor(Qt::transparent);			//选中文本前景色
	//括号匹配
	this->setBraceMatching(QsciScintilla::StrictBraceMatch);
	this->setUnmatchedBraceForegroundColor(Qt::transparent);
	this->setUnmatchedBraceBackgroundColor(Qt::white);
	this->setMatchedBraceForegroundColor(Qt::red);
	this->setMatchedBraceBackgroundColor(Qt::white);
	//光标
	this->setCaretLineVisible(false);
	this->setCaretWidth(1);										//光标宽度，0表示不显示光标
	this->setCaretForegroundColor(Qt::black);					//蓝色光标颜色
	this->setCaretLineBackgroundColor(Qt::white);
	//断点区
	QFont margin_font;
	margin_font.setFamily("SimSun");
	margin_font.setPointSize(9);								//边栏字体设置px我这里显示不出行号，不知道是怎么回事
	this->setMarginsFont(margin_font);							//设置页边字体
	//行号显示区域
	this->setMarginType(0, QsciScintilla::NumberMargin);
	this->setMarginLineNumbers(0, true);
	this->setMarginSensitivity(0, true);						//设置是否可以显示断点
	this->setMarginWidth(0, 25);
	// 设置Tab键为4个空格
	this->setTabWidth(4);
	this->setMarginType(1, QsciScintilla::SymbolMargin);
	this->setMarginWidth(1, 10);
	this->setMarginSensitivity(1, true);
	this->markerDefine(QsciScintilla::Circle, SymbolHandler::POINT1);
	this->setMarkerBackgroundColor(QColor(Qt::red), SymbolHandler::POINT1);
	this->markerDefine(QsciScintilla::Bookmark, SymbolHandler::LABEL);
	this->setMarkerBackgroundColor(QColor(Qt::transparent), SymbolHandler::LABEL);
	this->markerDefine(QsciScintilla::RightArrow, SymbolHandler::ARROW);
	this->setMarkerBackgroundColor(QColor(Qt::green), SymbolHandler::ARROW);
	this->markerDefine(QsciScintilla::RightArrow, SymbolHandler::LAARROW);
	this->setMarkerBackgroundColor(QColor(Qt::lightGray), SymbolHandler::LAARROW);

	this->markerDefine(QsciScintilla::Circle, SymbolComPile::Error);
	this->setMarkerBackgroundColor(QColor(Qt::red), SymbolComPile::Error);
	this->markerDefine(QsciScintilla::Circle, SymbolComPile::Alter);
	this->setMarkerBackgroundColor(QColor(Qt::yellow), SymbolComPile::Alter);
	this->markerDefine(QsciScintilla::Circle, SymbolComPile::Complete);
	this->setMarkerBackgroundColor(QColor(Qt::darkGreen), SymbolComPile::Complete);
	this->markerDefine(QsciScintilla::Circle, SymbolComPile::Alarm);
	this->setMarkerBackgroundColor(QColor(Qt::darkYellow), SymbolComPile::Alarm);
	// 设置符号区掩码 全部都可以显示
	this->setMarginMarkerMask(1, S_BREAK | S_LABEL | S_ARROW | S_LAARROW | S_Error | S_Alter | S_Complete | S_Alarm);

	this->setMarginType(2, QsciScintilla::SymbolMargin);
	this->setMarginLineNumbers(2, false);
	this->setMarginWidth(2, 10);
	this->setMarginSensitivity(2, true);
	this->setFolding(QsciScintilla::CircledTreeFoldStyle, 2);

	this->setFoldMarginColors(Qt::white, Qt::transparent);//折叠栏颜色
	this->setCaretLineVisible(false);
	//禁用自动补全提示功能|所有可用的资源|当前文档中出现的名称都自动补全提示|使用QsciAPIs类加入的名称都自动补全提示
	this->setCallTipsHighlightColor(QColor(0x20, 0x30, 0xff, 0xff));
	this->setAutoCompletionSource(QsciScintilla::AcsAll);	//自动补全。对于所有Ascii字符
	this->setAutoCompletionThreshold(1);					//设置每输入一个字符就会出现自动补全的提示
	this->setAutoCompletionReplaceWord(true);				//是否用补全的字符串替代光标右边的字符串
	//this->setCallTipsVisible(10);							//设置调用相关的显示

	this->setFocusPolicy(Qt::NoFocus);
	resize(QSize(400,300));
	//connect(this, SIGNAL(SendNotifyCallUISignal(Event, int, char*)), this, SLOT(SetNotifyCall(Event, int, char*)), Qt::BlockingQueuedConnection);
}

void QsciWidget::SetTabName(QString strName)
{
	m_strTab					= strName;
	m_ptrLLState->m_CurrentTab	= strName.toStdString();
}

void QsciWidget::NotifyCallBack(void * user,Event ev,int line,char * Error)
{
	QsciWidget*_User = (QsciWidget*)user;
	emit _User->SetNotifyCall(ev, line, Error);
}

void QsciWidget::SetNotifyCall(Event ev, int line, char * Error)
{
	if (m_Getcallback)
		m_Getcallback(m_PtrUser,m_strTab, ev, line, Error);
	if (m_FunCall != nullptr)
		m_FunCall(ev, line, Error);
}

void QsciWidget::SymbolLineSignal(Event ev,int line,char * Error)
{
	//SendSymbolLineSignal(m_strTab, ev, line, Error);
}

int QsciWidget::Start(QMap<QString, QVector<int>>& mapMarks)
{
	if (!m_ptrLLState->is_execution_finished())
		return -1;
	////设置断点
	//m_ptrLLState->clearbreakpoint();
	//QVector<int> vecBreak1D = GetBreakPoints();
	//for (size_t i = 0; i < vecBreak1D.size(); i++)
	//	m_ptrLLState->toggle_breakpoint(vecBreak1D[i] + 1);

	//多个函数
	std::map<std::string, std::map<int, Defs::Breakpoint>> _BreakPoint;
	for (QMap<QString, QVector<int>>::iterator iter = mapMarks.begin(); iter != mapMarks.end();iter++)
	{
		std::map<int, Defs::Breakpoint> _breaks;
		for (size_t i = 0; i < iter.value().size(); i++)
			_breaks[iter.value()[i] + 1] = Defs::BPT_EXECUTE;
		_BreakPoint[iter.key().toStdString()] = _breaks;
	}
	m_ptrLLState->toggle_SetMapbreakpoint(_BreakPoint);

	//开始
	m_ptrLLState->Start();
	return m_ptrLLState->create_thread();
}

void QsciWidget::Stop()
{
	m_ptrLLState->Stop();
}

void QsciWidget::Gobreak()
{
	m_ptrLLState->gobreak();
}

void QsciWidget::StepLine()
{
	m_ptrLLState->go(RunMode::StepLine);
}

void QsciWidget::StepInto()
{
	m_ptrLLState->go(RunMode::StepInto);
}

void QsciWidget::DebugRun()
{
	m_ptrLLState->go(RunMode::DebugRun);
}

void QsciWidget::AutoRun(bool useThre)
{
	if(useThre)
		m_ptrLLState->go(RunMode::AutoRun);
	else
		m_ptrLLState->go(RunMode::NoThreadRun);
}

void QsciWidget::StepOut()
{
	m_ptrLLState->go(RunMode::StepOut);
}

bool QsciWidget::IsRunning()
{
	if (m_ptrLLState->is_execution_finished())
		return false;
	return m_ptrLLState->is_running_;
}

bool QsciWidget::IsFinished(int type)
{
	if (type == 1)
	{
		return m_ptrLLState->m_bIsTaskFinishedByWait;
	}
	return m_ptrLLState->is_execution_finished();
}

bool QsciWidget::IsStopped()
{
	return m_ptrLLState->is_data_available();
}

bool QsciWidget::IsTaskFinished()
{
	return m_ptrLLState->m_bIsTaskFinished;
}

//bool QsciWidget::ToggleBreakpoint(int line)
//{
//	return m_ptrLLState->toggle_breakpoint(line);
//}

void QsciWidget::ClearSymbolLine(int iline,SymbolHandler Symbol)
{
	markerDeleteAllSignal(Symbol);
}

void QsciWidget::MarkGoSymbolLine(int iline,SymbolHandler Symbol)
{
	GotoLineSignal(iline);
	markerAddSignal(iline,Symbol); // 添加箭头
}

void QsciWidget::GotoSymbolLine(int iline,SymbolHandler Symbol)
{
	if (iline > 0)
		GotoLineSignal(iline);
	markerDeleteAllSignal(Symbol);
	markerAddSignal(iline,Symbol); // 添加箭头
}

void QsciWidget::GetCurLinePosition(int& iline, int& iCol)
{
	this->getCursorPosition(&iline, &iCol);
}

void QsciWidget::MarkDeleteSymbolLine(int iline, SymbolHandler Symbol)
{
	this->markerDeleteSignal(iline,Symbol);
}

void QsciWidget::ToggleBreakLine(int iline, SymbolHandler Symbol)
{		
	//得到光标位置
	int line, col;
	this->getCursorPosition(&line, &col);
	unsigned mask = this->markersAtLine(line);
	if (isMarker(mask, SymbolHandler::POINT1))
	{
		this->markerDelete(line, SymbolHandler::POINT1);
		m_ptrLLState->toggle_OperatorLineMapbreakpoint(m_strTab.toStdString(),iline , false);
	}
	else
	{
		this->markerAdd(line, SymbolHandler::POINT1); // 添加断点
		m_ptrLLState->toggle_OperatorLineMapbreakpoint(m_strTab.toStdString(),iline , true);
	}
}

void QsciWidget::ToggleMapBreakLine(QString strTab, int iline, SymbolHandler Symbol)
{
	//得到光标位置
	if (strTab == m_strTab)
	{
		int line, col;
		this->getCursorPosition(&line, &col);
		unsigned mask = this->markersAtLine(line);
		if (isMarker(mask, SymbolHandler::POINT1))
		{
			this->markerDelete(line, SymbolHandler::POINT1);
			m_ptrLLState->toggle_OperatorLineMapbreakpoint(m_strTab.toStdString(), iline , false);
		}
		else
		{
			this->markerAdd(line, SymbolHandler::POINT1); // 添加断点
			m_ptrLLState->toggle_OperatorLineMapbreakpoint(m_strTab.toStdString(), iline , true);
		}
	}
	else
	{
		m_ptrLLState->toggle_OperatorMapbreakpoint(strTab.toStdString(), iline );
	}
}

char GetNextChar(QString& txt, long& iIndex)
{
	iIndex++;
	return txt.at(iIndex).toLatin1();
}

bool inRange(QChar ch,ushort b,ushort c)
{
	return (ch.unicode() >= b && ch.unicode() <= c);
}

//提前进行编译
int QsciWidget::PreComPile()
{
	int iRetn = 0;
	markerDeleteAllSignal(SymbolComPile::Error);
	markerDeleteAllSignal(SymbolComPile::Alarm);
	//m_vecErrorItem.clear();
	//QString& txt = text();
	//char ch = ' ';
	//m_vecFuncs.clear();
	//clearMarginText();
	//bool _bIsFunc = false;
	//int _iCurrentLine = 0;
	//int _iCurrentCol = 0;
	//char _Words[1024];
	//long _iWordIndex = 0;
	//long _ilineIndex = 0;
	//long _lCurrentFuncLevel = 0;
	//memset(_Words,0,1024);
	//for (long i = 0; i < txt.length(); i++)
	//{
	//	bool bMark = inRange(txt[i],0x100,0xFFFFFF);
	//	ch = txt.at(i).toLatin1();
	//	memset(_Words, 0, 1024);
	//	_iWordIndex		= 0;
	//	if ((ch > 'a' && ch< 'z') || (ch > 'A' && ch < 'Z') || ch == '_')
	//	{
	//		while ((ch >= 'a' && ch<= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')
	//		{
	//			_Words[_iWordIndex++] = ch;
	//			if (i >= (txt.length() - 1))	break;
	//			ch = GetNextChar(txt,i);
	//		}
	//	}
	//	if (_bIsFunc && ch != ' ')
	//	{
	//		_bIsFunc = false;
	//		ch = txt.at(i).toLatin1();
	//		Funcs _funs;
	//		_funs.iFuncLine = _iCurrentLine;
	//		_funs.iCol		= i - _ilineIndex;
	//		if (ch == '(')
	//		{
	//			_funs.strFuncName = _Words;
	//		}
	//		else
	//		{
	//			_funs.strFuncName = NoName;
	//		}
	//		if (AlreadHaveFuncs(_funs))
	//		{
	//			markerAddSignal(_iCurrentLine - 1,SymbolComPile::Error);
	//			ErrorItem item;
	//			item.iErrorCode = Error_HaveFun;
	//			item.iFuncLine	= _iCurrentLine;
	//			item.iCol		= _funs.iCol;
	//			item.strError.append("Error Code ").append(QString::number(item.iErrorCode)).
	//				append(" Line ").append(QString::number(_iCurrentLine)).
	//				append(" Alread Have Function ").append(_funs.strFuncName);
	//			m_vecErrorItem.push_back(item);
	//			iRetn = Error_HaveFun;
	//		}
	//		m_vecFuncs.push_back(_funs);
	//	}
	//	if ((strcmp(_Words, "if") == 0) || (strcmp(_Words, "do") == 0) || (strcmp(_Words, "function") == 0) ||
	//		(strcmp(_Words, "repeat") == 0))
	//	{
	//		_lCurrentFuncLevel++;
	//		if (strcmp(_Words, "function") == 0)
	//		{
	//			_bIsFunc = true;
	//		}
	//	}
	//	if ((strcmp(_Words, "end") == 0) || (strcmp(_Words, "elseif") == 0) || (strcmp(_Words, "until") == 0))
	//	{
	//		_lCurrentFuncLevel--;
	//	}
	//	if (ch == '\r')
	//	{
	//		long _iIndex = i;
	//		if (i >= (txt.length() - 1))	break;
	//		ch = GetNextChar(txt, _iIndex);
	//		if (ch == '\n')
	//		{
	//			_iCurrentLine++;
	//		}
	//		_ilineIndex  = _iIndex;
	//		_iCurrentCol = 0;
	//	}
	//}
	////设置提示符号
	//for (long i = 0; i < m_vecFuncs.size(); i++)
	//{
	//	int iLine = m_vecFuncs[i].iFuncLine;
	//	QString strtext = m_vecFuncs[i].strFuncName;
	//	setMarginText(iLine, strtext,0);
	//}
	return iRetn;
}

int QsciWidget::Compile(QString& strError)
{
	int iRetn = 0;
	//if ((iRetn = PreComPile()) != 0)
	//{
	//	for (size_t i = 0; i < m_vecErrorItem.size(); i++)
	//	{
	//		strError.append(m_vecErrorItem[i].strError);
	//		strError.append("\r\n");
	//	}
	//	iRetn = -1;
	//	return iRetn;
	//}
	std::string strInMsg = text().toLocal8Bit().toStdString();

	if (luaL_loadstring(m_ptrLLState->L, strInMsg.c_str()) != 0)
	{
		const char * strchar = lua_tostring(m_ptrLLState->L, -1);
		strError.append(QString::fromLocal8Bit(strchar));
		iRetn = -1;
	}
	else
	{
		strError.append("Success !");
	}
	strError.append("\r\n");
	return iRetn;
}

void QsciWidget::ClearBreakpoint()
{
	m_ptrLLState->toggle_ClearMapbreakpoint();
}

//获取选中行的文本
QString QsciWidget::GetSelectedLine()
{
	QString _str;
	int line, col;
	this->getCursorPosition(&line, &col);
	return _str;
}

//获取选中的文本
QString QsciWidget::GetSelectedContent()
{
	QString _str;
	//获取选择注释的开始位置
	int selectionStart = SendScintilla(QsciScintillaBase::SCI_GETSELECTIONSTART);
	//获取选择注释的结束位置
	int selectionEnd = SendScintilla(QsciScintillaBase::SCI_GETSELECTIONEND);
	//获取选择注释的当前位置

	int iSelectedLenth = selectionEnd - selectionStart;
	//当前的光标位置 是否在选择内
	//获取要注释的行号
	int selStartLine = SendScintilla(QsciScintillaBase::SCI_LINEFROMPOSITION, selectionStart);
	int selEndLine = SendScintilla(QsciScintillaBase::SCI_LINEFROMPOSITION, selectionEnd);
	int lines = selEndLine - selStartLine;
	//处理结束行
	if ((lines  > 0) && (selectionEnd == SendScintilla(QsciScintillaBase::SCI_POSITIONFROMLINE, selEndLine)))	selEndLine--;
	//遍历行
	for (int i = selStartLine; i <= selEndLine; ++i)
	{
		//获取注释的行信息
		int lineStart	= SendScintilla(QsciScintillaBase::SCI_POSITIONFROMLINE,		i);
		int lineIndent	= SendScintilla(QsciScintillaBase::SCI_GETLINEINDENTPOSITION,	i);
		int lineEnd		= SendScintilla(QsciScintillaBase::SCI_GETLINEENDPOSITION,		i);
		// 要注释的行前面的空格处理     
		int lineBufferSize = 0;
		QString strMsg;
		if (selEndLine == selStartLine && (lineIndent <= lineEnd  && lineIndent >= lineStart))
		{
			lineBufferSize = iSelectedLenth;
			char *buf = new char[lineBufferSize];
			//获取注释范围
			SendScintilla(QsciScintillaBase::SCI_GETTEXTRANGE, selectionStart, selectionEnd, buf);
			strMsg = QString(buf);
		}
		else
		{
			lineIndent = lineStart;
			lineBufferSize = lineEnd - lineIndent + 1;
			char *buf = new char[lineBufferSize];
			//获取注释范围
			SendScintilla(QsciScintillaBase::SCI_GETTEXTRANGE, lineIndent, lineEnd, buf);
			strMsg = QString(buf);
		}
		_str = strMsg;
		break;
	}
	return _str;
}

QVector<int> QsciWidget::GetBreakPoints()
{
	QVector<int> _Markers;
	GetAllMarkers(SymbolHandler::POINT1, _Markers);
	return _Markers;
}

void QsciWidget::keyPressEvent(QKeyEvent * ev)
{
	QsciScintilla::keyPressEvent(ev);
}

void QsciWidget::GotoNextLine()
{
	int iRow, iCol;
	GetCurLinePosition(iRow, iCol);
	GotoLineSignal(iRow + 1);
	//int selectionEnd = SendScintilla(QsciScintillaBase::SCI_SETSELECTIONEND);

}

void QsciWidget::updateSymbol(int margin, int line, Qt::KeyboardModifiers state)
{
	bool bIsAdd = false;
	QString strMsg = "";
	unsigned mask = this->markersAtLine(line);
	switch (state) 
	{
	case Qt::ControlModifier: // 按下Ctrl键
		strMsg.append(SymbolHandler::ARROW).append(",");
		if (isMarker(mask, SymbolHandler::ARROW))
		{
			bIsAdd = false;
			this->markerDelete(line, SymbolHandler::ARROW);
		}
		else
		{
			bIsAdd = true;
			this->markerAdd(line, SymbolHandler::ARROW); // 添加箭头
		}
		strMsg.append(line).append(",");
		strMsg.append(bIsAdd).append(",");
		break;
	case Qt::AltModifier: // 按下Alt键
		strMsg.append(SymbolHandler::LABEL).append(",");
		if (isMarker(mask, SymbolHandler::LABEL))
		{
			bIsAdd = false;
			this->markerDelete(line, SymbolHandler::LABEL);
		}
		else
		{
			this->markerAdd(line, SymbolHandler::LABEL); // 添加标签
			bIsAdd = true;
		}
		strMsg.append(line).append(",");
		strMsg.append(bIsAdd).append(",");
		break;
	default:
		strMsg.append(SymbolHandler::POINT1).append(",");
		if (isMarker(mask, SymbolHandler::POINT1))
		{
			bIsAdd = false;
			this->markerDelete(line, SymbolHandler::POINT1);
			m_ptrLLState->toggle_OperatorLineMapbreakpoint(m_strTab.toStdString(),line + 1, false);
		}
		else
		{
			//判定当前是否空白能添加断点
			//获取当前是否有空行
			this->markerAdd(line, SymbolHandler::POINT1); // 添加断点
			m_ptrLLState->toggle_OperatorLineMapbreakpoint(m_strTab.toStdString(),line + 1, true);
			bIsAdd = true;
		}
		strMsg.append(line).append(",");
		strMsg.append(bIsAdd).append(",");
		break;
	}
	if (m_NotifyTabEvent)
		m_NotifyTabEvent(m_PtrUser,m_strTab, strMsg,4);
}

std::string QsciWidget::GetCallStack() const
{
	if (!m_ptrLLState->is_data_available())
		return std::string();

	std::string callstack;

	// local info= debug.getinfo(1)
	//LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);

	int level = 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof dbg);

	while (lua_getstack(m_ptrLLState->L, level++, &dbg))
	{
		if (lua_getinfo(m_ptrLLState->L, "Snl", &dbg) == 0)
		{
			callstack.append("-- error at level ").append("\r\n");
			break;
		}
		callstack.append(dbg.short_src);
		//callstack << dbg.short_src;
		if (dbg.currentline > 0)
			callstack.append(":").append(std::to_string(dbg.currentline));;
		if (*dbg.namewhat != '\0')  /* is there a name? */
			callstack.append(" in function ").append(dbg.name);
		else
		{
			if (*dbg.what == 'm')  /* main? */
				callstack.append(" in main chunk");
			else if (*dbg.what == 'C' || *dbg.what == 't')
				callstack.append(" ?");
			else
				callstack.append(" in file <").append(dbg.short_src).append(":").append(std::to_string(dbg.linedefined)).append(">");
		}
		callstack.append("\r\n");
	}
	return callstack.c_str();
}
static std::string to_table(lua_State* L, int index)
{
	//	lua_gettable
}

void capture_value(lua_State* L, QsciValue& v, int index, int recursive, size_t table_size_limit)
{
	int i = index;
	char buf[100];	// temp output for fast number/pointer formatting
	int t = lua_type(L, i);
	switch (t)
	{
	case LUA_TSTRING:
		v.type = QsciValType::Val_String;
		v.value = lua_tostring(L, i);
		break;
	case LUA_TBOOLEAN:
		v.type = QsciValType::Val_Bool;
		v.value = lua_toboolean(L, i) ? "true" : "false";
		break;
	case LUA_TNUMBER:
		v.type = QsciValType::Val_Number;
		sprintf(buf, "%g", static_cast<double>(lua_tonumber(L, i)));
		v.value = buf;
		break;
	case LUA_TLIGHTUSERDATA:
		v.type = QsciValType::Val_LightUserData;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;
	case LUA_TUSERDATA:
		v.type = QsciValType::Val_UserData;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;
	case LUA_TTABLE:
		v.type = QsciValType::Val_Table;
		if (recursive > 0)
		{
			QsciTableInfo t;
			list_table(L, i, t, recursive - 1);
			v.value = table_as_string(t, table_size_limit);
		}
		else
			v.value = to_pointer(buf, lua_topointer(L, i));
		break;
	case LUA_TFUNCTION:
		v.type = QsciValType::Val_Function;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;
	case LUA_TTHREAD:
		v.type = QsciValType::Val_Thread;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;
	case LUA_TNIL:
		v.type = QsciValType::Val_Nil;
		v.value.clear();
		break;
	default:
		v.type = QsciValType::Val_None;
		v.value.clear();
		break;
	}
	v.type_name = lua_typename(L, t);
}

//bool QsciWidget::GetLocalVars(std::vector<QsciVar>& out, int level) const
//{
//	out.clear();
//
//	if (!m_ptrLLState->is_data_available())
//		return false;
//
//	lua_Debug dbg;
//	memset(&dbg, 0, sizeof(dbg));
//
//	if (!lua_getstack(m_ptrLLState->L, level, &dbg))
//		return false;
//
//	if (lua_getinfo(m_ptrLLState->L, "Snl", &dbg))
//	{
//		const int SAFETY_COUNTER = 10000;
//
//		for (int i = 1; i < SAFETY_COUNTER; ++i)
//		{
//			const char* name = lua_getlocal(m_ptrLLState->L, &dbg, i);
//			if (name == 0)
//				break;
//			pop_stack_elements pop(m_ptrLLState->L, 1);	// pop variable value eventually
//			QsciVar var;
//			var.name = name;
//			capture_value(m_ptrLLState->L, var.v, lua_gettop(m_ptrLLState->L), 1);
//
//			out.push_back(var);
//		}
//	}
//	return true;
//}
//bool QsciWidget::GetGlobalVars(QsciTableInfo& out, bool deep) const
//{
//	if (!m_ptrLLState->is_data_available())
//		return false;
//
//	return list_table(m_ptrLLState->L, -10002, out, deep ? 1 : 0);
//}
//bool QsciWidget::GetValueStack(QsciValueStack& stack) const
//{
//	if (!m_ptrLLState->is_data_available())
//		return false;
//
//	const size_t limit_table_elements_to = 10;
//	return list_virtual_stack(m_ptrLLState->L, stack, limit_table_elements_to);
//}

QsciStackFrame::QsciStackFrame()
{
	Clear();
}
void QsciStackFrame::Clear()
{
	current_line = 0;
	type = Err;
	line_defined = last_line_defined = 0;
}
const char* QsciStackFrame::SourcePath() const
{
	if (source.size() > 1 && source[0] == '@')
		return source.c_str() + 1;
	return 0;
}
bool QsciWidget::GetCallStack(QsciCallStack& stack) const
{
	if (!m_ptrLLState->is_data_available())
		return false;
	stack.clear();
	stack.reserve(8);
	int level = 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof(lua_Debug));
	while (lua_getstack(m_ptrLLState->L, level++, &dbg))
	{
		QsciStackFrame frame;
		if (lua_getinfo(m_ptrLLState->L, "Snl", &dbg) == 0)
		{
			stack.push_back(frame);	// error encountered
			break;
		}
		fill_frame(dbg, frame);
		stack.push_back(frame);
	}
	return true;
}
bool QsciWidget::GetCurrentSource(QsciStackFrame& top) const
{
	if (!m_ptrLLState->is_data_available())
		return false;

	int level = 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof dbg);

	if (!lua_getstack(m_ptrLLState->L, level, &dbg) || !lua_getinfo(m_ptrLLState->L, "Snl", &dbg))
		return false;

	fill_frame(dbg, top);

	return true;
}
std::string QsciWidget::Status() const
{
	if (m_ptrLLState->is_execution_finished() || m_ptrLLState->status_ready_)
		return m_ptrLLState->status_msg_;

	return m_ptrLLState->is_running_ ? "Running" : "Stopped";
}
void QsciWidget::txtChanged()
{
	PreComPile();
	if (m_NotifyTabEvent)
		m_NotifyTabEvent(m_PtrUser, m_strTab,"",Qt::NoModifier);
}