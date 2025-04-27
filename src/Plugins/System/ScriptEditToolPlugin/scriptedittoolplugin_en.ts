<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US">
<context>
    <name>CloudMesureToolPlugin</name>
    <message>
        <location filename="CloudMesureToolPlugin.cpp" line="18"/>
        <source>SystemSet</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>ScriptEditTool</name>
    <message>
        <location filename="ScriptEditTool.cpp" line="139"/>
        <source>Script</source>
        <translation>Script</translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="565"/>
        <location filename="ScriptEditTool.cpp" line="581"/>
        <location filename="ScriptEditTool.cpp" line="590"/>
        <location filename="ScriptEditTool.cpp" line="610"/>
        <location filename="ScriptEditTool.cpp" line="620"/>
        <source>No Such Net</source>
        <translation></translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="578"/>
        <location filename="ScriptEditTool.cpp" line="604"/>
        <location filename="ScriptEditTool.cpp" line="607"/>
        <source>Net</source>
        <translation></translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="578"/>
        <location filename="ScriptEditTool.cpp" line="604"/>
        <source> Isn&apos;t Connected</source>
        <translation></translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="607"/>
        <source> Time Out</source>
        <translation></translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="634"/>
        <location filename="ScriptEditTool.cpp" line="638"/>
        <location filename="ScriptEditTool.cpp" line="646"/>
        <source>Flow </source>
        <translation></translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="634"/>
        <source> Is Enable </source>
        <translation></translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="638"/>
        <source> Is Error </source>
        <translation></translation>
    </message>
    <message>
        <location filename="ScriptEditTool.cpp" line="646"/>
        <source> Didn&apos;t Exist </source>
        <translation></translation>
    </message>
</context>
<context>
    <name>frmScriptEditTool</name>
    <message>
        <location filename="frmScriptEditTool.cpp" line="188"/>
        <source>main</source>
        <translation>主窗口</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="190"/>
        <source>ToolDock</source>
        <translation>工具</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="189"/>
        <source>ModulDock</source>
        <oldsource>EditDock</oldsource>
        <translation>模块</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="191"/>
        <source>TextTips</source>
        <translation>提示</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="192"/>
        <source>LogDock</source>
        <translation>日志</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="193"/>
        <source>VariableDock</source>
        <translation>变量</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="325"/>
        <source>Variable</source>
        <translation>变量</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="333"/>
        <source>Flow</source>
        <translation>流程</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="338"/>
        <source>Char</source>
        <translation>字符操作</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="345"/>
        <source>Communicate</source>
        <translation>通讯</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="351"/>
        <source>Log</source>
        <oldsource>Log日志</oldsource>
        <translation>日志</translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="527"/>
        <location filename="frmScriptEditTool.cpp" line="554"/>
        <source>必须包含( )</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.cpp" line="614"/>
        <location filename="frmScriptEditTool.cpp" line="618"/>
        <source>GlobalVar</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>frmScriptEditToolClass</name>
    <message>
        <location filename="frmScriptEditTool.ui" line="35"/>
        <source>脚本编辑</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="228"/>
        <source>//示例1
//输出	  print(&apos;123123&apos;);
//获取当前时间  local time = getdatetime(&apos;yyyy-MM-dd hh:mm:ss.zzz&apos;);
年-月-日 时.分.秒.毫秒(&apos;2024-03-21 13:37:56.415&apos;)

//字符分割
local  strValue = &quot;V0.1.2.3&quot;;
local strLst = split(strValue,&apos;.&apos;);
print(strLst);		// &quot;{ 1 = 1, 2 = 2, 3 = 3, 0 = V0 }&quot;

//字符合并
print(strLst[0]..strLst[1]..strLst[2]..strLst[3]);

//设置GlobalVar
setglobal(&quot;Var4&quot;, &quot;2,1&quot;)	//设置点位坐标
getglobal(&quot;Var4&quot;)	//获取点位坐标
//设置零时变量
getlocal(&quot;Var4&quot;)	//获取点位坐标
setlocal(&quot;Var4&quot;, &quot;2,1&quot;)	//设置点位坐标

logerror(&apos;123&apos;)	//设置报警
loginfo(&apos;123&apos;);		//设置报警
logwarn(&apos;123&apos;);	//设置报警

//写文件
file = io.open(&quot;d:\\New1231231.txt&quot;,&quot;a&quot;);
io.output(file);
io.write(getglobal(&quot;gVar0&quot;)..&quot;\n&quot;);
io.close();

//读取
file = io.open(&quot;d:\\ReadNew.txt&quot;,&quot;r&quot;);
io.input(file);
loginfo(io.read());
io.close(file);
//单行读取
for lin in io.lines(&quot;d:\\ReadNew.txt&quot;) do
  loginfo(lin); 
end</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="379"/>
        <source>数据</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="384"/>
        <source>通讯</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="349"/>
        <source>模块</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="142"/>
        <source>编号</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="153"/>
        <source>变量名称</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="164"/>
        <source>变量值</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="175"/>
        <source>变量类型</source>
        <translation></translation>
    </message>
    <message>
        <location filename="frmScriptEditTool.ui" line="389"/>
        <source>流程</source>
        <translation></translation>
    </message>
</context>
</TS>
