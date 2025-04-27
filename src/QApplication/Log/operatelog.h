#ifndef OPERATELOG_H
#define OPERATELOG_H

#include <QDateTime>
#include <QMutex>
#include <QObject>

enum EnumLogType
{
	LogType_Trace,
	LogType_Debug,
	LogType_Info,
	LogType_Warn,
	LogType_Critical,
	LogType_Fatal,

};
//Log
typedef struct Operate_Log
{
    QString sTime;
    QString sUser;
    QString sType;
    QString sContent;

}OPERATE_LOG_T;

class OperateLog : public QObject
{
    Q_OBJECT
public:
    OperateLog();
    ~OperateLog();

public:
    static OperateLog& getInstance();

public:
    //添加日志
    void appendLog(OPERATE_LOG_T stLog);
    //清理日志
    void clearLog();
	//加载日志
	void LoadLog(QDateTime StartTime, QDateTime EndTime);
    //导出日志
    int ExportLog();
public:
	//获取日志总数
	int getLogTotal();
    //设置每页日志行数
    void setEachPageRows(int nRows);
    //获取每页日志行数
    int getEachPageRows();
    //获取总页数
    int getTotalPages();

    //获取当前页码
    int getCurPageNo();

	//获取最后一页的行数
	int getLastPageNo();

	//设置过滤的类型
	int SetFilterType(QString type);
public:
	//通过页码获取日志
	QList<OPERATE_LOG_T> getAllLogListByPage(int nPage, QString sType);
    //通过页码获取日志
    QList<OPERATE_LOG_T> getLogListByPage(int nPage);
    //通过时间和类型获取日志
    QList<OPERATE_LOG_T> getLogListByTimeAndType(QDateTime StartTime, QDateTime EndTime, QString sType);
    //通过内容获取日志
    QList<OPERATE_LOG_T> getLogListByContent(QString sContent);

signals:
    //更新总页数和当前页码
    void sig_updatePageInfo(int nPages, int nPageNo);

public:
	QList<OPERATE_LOG_T> m_logList;			//所有日志
	QList<OPERATE_LOG_T> m_logListFilter;	//筛选后的日志
private:
    int m_nEachPageRows;	//每页行数
    int m_nPages;			//总页数
    int m_nCurPageNo;		//当前页面
	int m_iAllRowCount;		//当前总行数
	QDateTime m_lastFindStartDate;
	QDateTime m_lastFindEndDate;
	QString m_strFilterType;

	QMutex m_lock;
};

#endif // OPERATELOG_H
