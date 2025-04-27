#include "QWorld.h"
#include <QString>

#pragma region SQLite
//创建并连接SQLite数据库
bool SQLite::CreateConnectDB(QString key, QString sql_name)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		////建立并打开数据库
		if (QSqlDatabase::contains("MyDataBase"))
		{
			//QSqlDatabase::removeDatabase(QLatin1String(QSqlDatabase::defaultConnection));
			sqldatabase = QSqlDatabase::database("MyDataBase");
		}
		else
		{
			QSqlDatabase::removeDatabase(QLatin1String("MyDataBase"));
			sqldatabase = QSqlDatabase::addDatabase("QSQLITE", "MyDataBase");
		}

		//database = QSqlDatabase::addDatabase("QSQLITE");
		sqldatabase.setDatabaseName(sql_name);
		bool bol_sql_open = sqldatabase.open();
		if (bol_sql_open == false)
		{
			QMessageBox::critical(NULL, "Prompt", sqldatabase.lastError().text());
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}
//创建表格
bool SQLite::CreateTable(QString key, QString table_name)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		QSqlQuery sql_query(sqldatabase);
		QString sql = "create table " + table_name + "(USERNAME text, PASSWORD text, USERLIMIT INTEGER DEFAULT (0) NOT NULL)";
		if (!sql_query.exec(sql))
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}

//插入数据
bool SQLite::InsertData(QString key, QString table_name, QString user_name, QString password, QString level)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		QSqlQuery sql_query(sqldatabase);
		QString sql = "insert into " + table_name + " values (?, ?, ?)";
		sql_query.prepare(sql);
		sql_query.addBindValue(user_name);
		sql_query.addBindValue(password);
		sql_query.addBindValue(level);
		if (!sql_query.exec())
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}

//修改数据
bool SQLite::ModifyData(QString key, QString user_name, QString password)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		QSqlQuery sql_query(sqldatabase);
		QString sql = "update LoginTable set PASSWORD = :PASSWORD where USERNAME = :USERNAME";
		sql_query.prepare(sql);
		sql_query.bindValue(":USERNAME", user_name);
		sql_query.bindValue(":PASSWORD", password);
		if (!sql_query.exec())
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}
bool SQLite::ModifyUserData(QString key, QString user_name, QString password, int level)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		QSqlQuery sql_query(sqldatabase);
		QString sql = "update LoginTable set PASSWORD = :PASSWORD ,USERLIMIT = :USERLIMIT where USERNAME = :USERNAME";
		sql_query.prepare(sql);
		sql_query.bindValue(":USERNAME", user_name);
		sql_query.bindValue(":PASSWORD", password);
		sql_query.bindValue(":USERLIMIT", level);
		if (!sql_query.exec())
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}

//查询数据
bool SQLite::QueryData(QString key, QString user_name, QString &password)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		//eg: "select * from LoginTable where USERNAME == 'user0';"
		QSqlQuery sql_query(sqldatabase);
		if (!sql_query.exec("select * from LoginTable where USERNAME == '" + user_name + "';"))
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			while (sql_query.next())
			{
				password = sql_query.value(1).toString();
			}
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}
bool SQLite::QueryUserData(QString key, QString user_name, QString & password, int &level)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		//eg: "select * from LoginTable where USERNAME == 'user0';"
		QSqlQuery sql_query(sqldatabase);
		if (!sql_query.exec("select * from LoginTable where USERNAME == '" + user_name + "';"))
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			while (sql_query.next())
			{
				password = sql_query.value(1).toString();
				level = sql_query.value(2).toInt();
			}
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}

//删除数据
bool SQLite::DeleteData(QString key, QString user_name)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		QSqlQuery sql_query(sqldatabase);
		if (!sql_query.exec("delete from LoginTable where USERNAME == '" + user_name + "';"))
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}

//删除表格
bool SQLite::DeleteTable(QString key, QString table_name)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		QSqlQuery sql_query(sqldatabase);
		sql_query.exec("drop table " + table_name);
		if (sql_query.exec())
		{
			QMessageBox::critical(NULL, "Prompt", sql_query.lastError().text());
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}

//关闭数据库
bool SQLite::CloseDB(QString key)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{	
		sqldatabase.close();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

#pragma endregion
