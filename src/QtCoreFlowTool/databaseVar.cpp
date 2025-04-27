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

// ��ȡ��������
void get_decrypt_content(std::string _param_path)
{
	std::fstream BeforeFile;
	// �򿪴�����				
	BeforeFile.open(_param_path, std::ios::in | std::ios::binary);//��|�ֽ���
	// ��ȡ�������ļ���С	
	BeforeFile.seekg(0, std::ios::end);//3.1��λ�ļ�����ָ�뵽ĩβ
	std::streamoff size = BeforeFile.tellg();	//3.2��ȡ��С
	BeforeFile.seekg(0, std::ios::beg);//3.3 ��λ�ļ�����ָ�뵽�ļ�ͷ

	std::vector<char> model;
	for (std::streamoff i = 0; i < size; i++)
	{
		model.push_back(char((BeforeFile.get() ^ 0x88)));//���ܻ���ܱ���  0x88 
	}
	std::string str_model_temp(model.begin(), model.end());
}

// ����
void encrypte_file(std::string needEncryptFilePath, std::string saveFilePath)
{
	// �������ļ���������
	std::fstream BeforeFile, AfterFile;

	// �򿪴�����				
	BeforeFile.open(needEncryptFilePath, std::ios::in | std::ios::binary);//��|�ֽ���
	// �򿪴����ܺ��ļ�
	AfterFile.open(saveFilePath, std::ios::out | std::ios::binary);

	// ��ȡ�������ļ���С	
	BeforeFile.seekg(0, std::ios::end);//3.1��λ�ļ�����ָ�뵽ĩβ
	std::streamoff  size = BeforeFile.tellg();	//3.2��ȡ��С
	BeforeFile.seekg(0, std::ios::beg);//3.3 ��λ�ļ�����ָ�뵽�ļ�ͷ

	for (std::streamoff i = 0; i < size; i++)
	{
		//��BeforeFile�����ݴ�������AfterFile��
		AfterFile.put(BeforeFile.get() ^ 0x88);//���ܻ���ܱ���  0x88 
	}
	// �ر������ļ�
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

	//������ת��ΪQString
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

	//������ת��ΪQString
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
 * ���� : �ݹ�����ļ��У��ҵ����а����������ļ�
 *----------------------------
 * ���� : find
 * ���� : public
 *
 * ���� : lpPath [in]      ��������ļ���Ŀ¼
 * ���� : fileList [in]    ���ļ����Ƶ���ʽ�洢��������ļ�
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
	// TODO: �ڴ˴����� return ���
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
	// ��ȡ��Ļ�Ŀ�Ⱥ͸߶�
	int screenWidth = QApplication::desktop()->width();
	int screenHeight = QApplication::desktop()->height();

	// ��ȡ�Ӵ��ڵĿ�Ⱥ͸߶�
	int windowWidth = window->width();
	int windowHeight = window->height();

	// �����Ӵ��ڵ����Ͻ�����
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// ���Ӵ����ƶ�����Ļ����
	window->move(x, y);
}

void databaseVar::CenterMainWindow(QWidget * window)
{	
	if (databaseVar::m_pWindow == nullptr)
		return;
	// ��ȡ��Ļ�Ŀ�Ⱥ͸߶�
	int screenWidth = databaseVar::m_pWindow->width();
	int screenHeight = databaseVar::m_pWindow->height();

	// ��ȡ�Ӵ��ڵĿ�Ⱥ͸߶�
	int windowWidth = window->width();
	int windowHeight = window->height();

	// �����Ӵ��ڵ����Ͻ�����
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// ���Ӵ����ƶ�����Ļ����
	window->move(x, y);
}

void databaseVar::CenterWindow(QWidget * window, QWidget * subwindow)
{
	if (window == nullptr)	return;

	// ��ȡ��Ļ�Ŀ�Ⱥ͸߶�
	int screenWidth		= window->width();
	int screenHeight	= window->height();

	// ��ȡ�Ӵ��ڵĿ�Ⱥ͸߶�
	int windowWidth		= subwindow->width();
	int windowHeight	= subwindow->height();

	// �����Ӵ��ڵ����Ͻ�����
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// ���Ӵ����ƶ�����Ļ����
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