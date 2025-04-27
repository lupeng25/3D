#pragma once

#include <QtWidgets/QWidget>
#include <sstream>
#include <QString>
#include <QDomDocument>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <vector>
using namespace std;

class SoftKey
{
public:
	//SoftKey
	QString GetKey(QString key);
};

class FolderOperation
{
public:
	//删除N天前的文件；
	static bool FindFileForDelete(QString key, QString path, int day = -30);

	//删除N天前的文件；
	static bool FindFileForDeleteLog(QString key, QString path, int day = -30);

	//删除N天前的文件夹；
	static	bool FindFolderForDelete(QString key, QString path, int day = 30);

	//查看文件夹里的所有文件
	static bool FindFilesInFolder( QString path,QVector<QString>& Files);
};

class SQLite
{
public:
	//创建并连接SQLite数据库；
	//sql_name为数据库文件路径，如：QDir::currentPath() + "/MyDataBase.db"；
	//database为数据库；
	bool CreateConnectDB(QString key, QString sql_name);

	//创建表格；
	//table_name为数据库的表格名称，如：LoginTable；
	bool CreateTable(QString key, QString table_name);

	//插入数据；
	//table_name为数据库的表格名称，如：LoginTable；user_name为用户名；password为密码；
	bool InsertData(QString key, QString table_name, QString user_name, QString password,QString level = "0");

	//修改数据(默认表格为LoginTable)；
	//user_name为用户名；password为修改的密码；
	bool ModifyData(QString key, QString user_name, QString password);
	bool ModifyUserData(QString key, QString user_name, QString password, int level);

	//查询数据(默认表格为LoginTable)；
	//user_name为用户名；password为查询的密码；
	bool QueryData(QString key, QString user_name, QString &password);

	//user_name为用户名；password为查询的密码；
	bool QueryUserData(QString key, QString user_name, QString &password, int &level);

	//删除数据(默认表格为LoginTable)；
	//user_name为用户名；
	bool DeleteData(QString key, QString user_name);

	//删除表格；
	//table_name为数据库的表格名称，如：LoginTable；
	bool DeleteTable(QString key, QString table_name);

	//关闭数据库；
	//database为数据库；
	bool CloseDB(QString key);

public:
	QSqlDatabase sqldatabase;
};

class XML
{
public:
	//创建5个参数的Xml文件；
	//file_name为XML文件路径，如：QDir::currentPath() + "/Param.xml"；
	bool CreateXml5(QString key, QString file_name, QString value1, QString value2, QString value3, QString value4, QString value5);

	//创建9个参数的Xml文件；
	//file_name为XML文件路径，如：QDir::currentPath() + "/Param.xml"；
	bool CreateXml9(QString key, QString file_name, QString value1, QString value2, QString value3, QString value4, QString value5, QString value6, QString value7, QString value8, QString value9);

	bool SaveXmlL(QString key, QString file_name, vector<string> parameter, vector<string> value);

	//修改Xml文件的参数；
	bool ModifyXml(QString key, QString file_name, vector<string> parameter, vector<string> value);

	//读取Xml文件的参数；
	bool ReadXml(QString key, QString file_name, QString parameter, QString& value);
};
