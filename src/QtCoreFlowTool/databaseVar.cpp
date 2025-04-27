#include "databaseVar.h"
#include <QDateTime>
#include <QtWidgets/QApplication>
#include <QDesktopWidget>

#include <Windows.h>
#include <QSet>
#include <QFileInfo>
#include <qdebug.h>
#include <bits.h>
#include <QDir>

#include <fstream>
#include <vector>
#include <iostream>
#include <vector>
#include <cstring>
#include <direct.h>

// 获取解密内容
void get_decrypt_content(std::string _param_path)
{
	std::fstream BeforeFile;
	// 打开待加密				
	BeforeFile.open(_param_path, std::ios::in | std::ios::binary);//读|字节流
	// 获取待加密文件大小	
	BeforeFile.seekg(0, std::ios::end);//3.1定位文件内容指针到末尾
	std::streamoff size = BeforeFile.tellg();	//3.2获取大小
	BeforeFile.seekg(0, std::ios::beg);//3.3 定位文件内容指针到文件头

	std::vector<char> model;
	for (std::streamoff i = 0; i < size; i++)
	{
		model.push_back(char((BeforeFile.get() ^ 0x88)));//加密或解密编码  0x88 
	}
	std::string str_model_temp(model.begin(), model.end());
}

// 加密
void encrypte_file(std::string needEncryptFilePath, std::string saveFilePath)
{
	// 待加密文件的流对象
	std::fstream BeforeFile, AfterFile;

	// 打开待加密				
	BeforeFile.open(needEncryptFilePath, std::ios::in | std::ios::binary);//读|字节流
	// 打开待解密后文件
	AfterFile.open(saveFilePath, std::ios::out | std::ios::binary);

	// 获取待加密文件大小	
	BeforeFile.seekg(0, std::ios::end);//3.1定位文件内容指针到末尾
	std::streamoff  size = BeforeFile.tellg();	//3.2获取大小
	BeforeFile.seekg(0, std::ios::beg);//3.3 定位文件内容指针到文件头

	for (std::streamoff i = 0; i < size; i++)
	{
		//把BeforeFile的内容处理后放入AfterFile中
		AfterFile.put(BeforeFile.get() ^ 0x88);//加密或解密编码  0x88 
	}
	// 关闭两个文件
	BeforeFile.close();
	AfterFile.close();
}

void QDecrypte_file(QString needEncryptFilePath, QString saveFilePath)
{
	QString strData;
	{
		QFile file(needEncryptFilePath);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QByteArray t = file.readAll();
		strData = QString(t);
		file.close();
	}

	//将数据转化为QString
	QByteArray byteArray = strData.toLatin1();
	QString _str = QByteArray::fromBase64(byteArray);

	{
		QFile file(saveFilePath);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		file.write(_str.toUtf8());
		file.close();
	}

}

void Qencrypte_file(QString needEncryptFilePath, QString saveFilePath)
{
	QString strData;
	{
		QFile file(needEncryptFilePath);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QByteArray t = file.readAll();
		strData = QString(t);
		file.close();
	}

	//将数据转化为QString
	QByteArray byteArray = strData.toLatin1();
	QString _str = QString::fromLatin1(byteArray.toBase64());

	{
		QFile file(saveFilePath);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		file.write(_str.toUtf8());
		file.close();
	}

}

int FindAllFileInFolder(QString strFolder,QVector<QString>& strFiles)
{
	QDir dir(strFolder);
	QFileInfoList fileinfoLst = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	for each (auto var in fileinfoLst)
	{
		if (var.isFile())
		{
			strFiles.push_back(var.fileName());
		}
	}
	return 0;
}

/*----------------------------
 * 功能 : 递归遍历文件夹，找到其中包含的所有文件
 *----------------------------
 * 函数 : find
 * 访问 : public
 *
 * 参数 : lpPath [in]      需遍历的文件夹目录
 * 参数 : fileList [in]    以文件名称的形式存储遍历后的文件
 */
void find(char* lpPath, std::vector<std::string> &fileList)
{
	char szFind[MAX_PATH];
	WIN32_FIND_DATAA FindFileData;

	strcpy(szFind, lpPath);
	strcat(szFind, "\\*.*");

	HANDLE hFind = ::FindFirstFileA(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)    return;

	while (true)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				char szFile[MAX_PATH];
				strcpy(szFile, lpPath);
				strcat(szFile, "\\");
				strcat(szFile, (char*)(FindFileData.cFileName));
				find(szFile, fileList);
			}
		}
		else
		{
			//std::cout << FindFileData.cFileName << std::endl;  
			fileList.push_back(FindFileData.cFileName);
		}
		if (!FindNextFileA(hFind, &FindFileData))    break;
	}
	FindClose(hFind);
}

void encrypteAllFile()
{

}

databaseVar & databaseVar::Instance()
{
	// TODO: 在此处插入 return 语句
	static databaseVar ins;
	return ins;
}

int databaseVar::Init()
{
	applicationPath			= QApplication::applicationDirPath();
	soft_key				= "59226f4700f1cc8a5de755d501af308d2d2";
	ConfigFolder			= applicationPath + "/config/";
	DefaultParamFolder		= applicationPath + "/default/";
	software_name			= "Mechine Vision";
	software_vision			= "0.0.0.0";
	iLanguage				= 0;
	m_pWindow				= nullptr;
	m_TitleBackColor		= QColor::fromRgb(255, 0, 0);
	m_BackColor				= QColor("#bcc2bc");// QColor::fromRgb(255, 0, 0);
	projectName				= QString();

	VideoWidgets.clear();
	m_iCheckMode			= 0;
	m_mapParam.clear();

	form_System_Precision	= 3;
	m_iCheckNGMotion		= 0;
	bAutoConnectedWhenStart = 1;

	return 0;
}

int databaseVar::Exit()
{
	return 0;
}

void databaseVar::CenterScreenWindow(QWidget * window)
{
	// 获取屏幕的宽度和高度
	int screenWidth = QApplication::desktop()->width();
	int screenHeight = QApplication::desktop()->height();

	// 获取子窗口的宽度和高度
	int windowWidth = window->width();
	int windowHeight = window->height();

	// 计算子窗口的左上角坐标
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// 将子窗口移动到屏幕中央
	window->move(x, y);
}

void databaseVar::CenterMainWindow(QWidget * window)
{	
	if (databaseVar::m_pWindow == nullptr)
		return;
	// 获取屏幕的宽度和高度
	int screenWidth = databaseVar::m_pWindow->width();
	int screenHeight = databaseVar::m_pWindow->height();

	// 获取子窗口的宽度和高度
	int windowWidth = window->width();
	int windowHeight = window->height();

	// 计算子窗口的左上角坐标
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// 将子窗口移动到屏幕中央
	window->move(x, y);
}

void databaseVar::CenterWindow(QWidget * window, QWidget * subwindow)
{
	if (window == nullptr)	return;

	// 获取屏幕的宽度和高度
	int screenWidth		= window->width();
	int screenHeight	= window->height();

	// 获取子窗口的宽度和高度
	int windowWidth		= subwindow->width();
	int windowHeight	= subwindow->height();

	// 计算子窗口的左上角坐标
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// 将子窗口移动到屏幕中央
	subwindow->move(x, y);
}

void databaseVar::ClearMenu(QMenu * menu)
{
	if (menu == nullptr)	return;
	QList<QAction*> actions = menu->actions();
	for (size_t i = 0; i < actions.size(); i++)
	{
		QMenu * _menu = actions[i]->menu();
		QString __text = actions[i]->text();
		if (_menu != nullptr)	ClearMenu(menu);
		if (actions[i]	!= nullptr)	delete actions[i];	actions[i]	= nullptr;
	}
	if (menu != nullptr)	delete menu;	menu = nullptr;
}