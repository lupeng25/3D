#include "operatelog.h"
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include "Data/dataVar.h"
#include "DataBase/QWorld.h"

OperateLog::OperateLog()
{
    m_nEachPageRows = 10;
    m_nCurPageNo = 1;
	m_iAllRowCount = 1000;
	m_strFilterType = "All";
}

OperateLog::~OperateLog()
{
}

OperateLog &OperateLog::getInstance()
{
    static OperateLog s_obj;
    return s_obj;
}

/*************************
 * 添加日志
 * ***********************/
void OperateLog::appendLog(OPERATE_LOG_T stLog)
{
	m_lock.lock();
    m_logList.append(stLog);
	if (m_strFilterType == stLog.sType || m_strFilterType == "All")
		m_logListFilter.append(stLog);
	if (m_logList.size() > m_iAllRowCount)
	{
		m_logList.removeLast();
	}
	m_lock.unlock();
}

/*************************
 * 清理日志
 * ***********************/
void OperateLog::clearLog()
{
    m_logList.clear();
    m_nPages = 0;
    m_nCurPageNo = 0;

    emit sig_updatePageInfo(m_nPages, m_nCurPageNo);
}

void OperateLog::LoadLog(QDateTime StartTime, QDateTime EndTime)
{
	if (StartTime == m_lastFindStartDate && m_lastFindEndDate == EndTime) return;
	try
	{
		QDir dir(dataVar::Instance().strLogPath);
		if (dir.exists() == false)
			dir.mkpath(dataVar::Instance().strLogPath);
		m_logList.clear();

		QString strDays = StartTime.toString("yyyy-MM-dd");
		strDays = EndTime.toString("yyyy-MM-dd");

		int _days = StartTime.daysTo(EndTime);
		for (long i = 0; i < (_days + 1); i++)
		{
			QDateTime newDate = StartTime.addDays(i); // 添加5天
			QString _strfiles = "log_" + newDate.toString("yyyy-MM-dd") + ".log";
			QFile file(dataVar::Instance().strLogPath + _strfiles);
			if (!file.exists()) continue;
			if (file.open(QIODevice::ReadOnly))
			{
				file.seek(0);
				while (!file.atEnd())
				{
					QString temp = QString::fromUtf8(file.readLine());
					QStringList _strLst = temp.split(']');
					OPERATE_LOG_T _log;
					for (long isize = 0; isize < _strLst.size(); isize++)
					{
						QString& _tempstr = _strLst[isize];
						if (_tempstr.contains('['))
						{
							QStringList _strSecLst = _tempstr.split('[');
							switch (isize)
							{
							case 0: {
								if (_strSecLst.size() > 1)	_log.sTime = _strSecLst[1];
							}	break;
							case 1: {
								if (_strSecLst.size() > 1)	_log.sType = _strSecLst[1];
							}	break;
							case 3: {
								if (_strSecLst.size() > 1)	_log.sUser = _strSecLst[1];
							}	break;
							default:
								break;
							}
						}
						else
						{
							_log.sContent = _strLst[isize];
						}
					}
					appendLog(_log);
				}
				file.close();
			}
		}
	}
	catch (...) {	}
	m_lastFindStartDate = StartTime;
	m_lastFindEndDate = EndTime;
}

/*************************
 * 获取日志总数
 * ***********************/
int OperateLog::getLogTotal()
{
    return m_logList.count();
}

/*************************
 * 导出日志
 * ***********************/
int OperateLog::ExportLog()
{
    QString sPath = QCoreApplication::applicationDirPath() + "/csv/";
    QDir dir(sPath);
    if(dir.exists() == false)
    {
        dir.mkpath(sPath);
    }

    QString sFileName = sPath + "log_" +
            QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".log";
    QStringList sLogContent;
    int nId = 1;

    if(m_logListFilter.count() == 0 && m_logList.count() > 0)
    {
        m_logListFilter = m_logList;
    }

    QList<OPERATE_LOG_T>::iterator it;
    if(m_logListFilter.count() > 0)
    {
        sLogContent << "ID,Time,User,Type,Content\r\n";
        for(it = m_logListFilter.begin(); it != m_logListFilter.end(); it++)
        {
            QString log = QString("%1,%2,%3,%4,%5\r\n").arg(nId).arg(it->sTime).arg(it->sUser).arg(it->sType).arg(it->sContent);
            sLogContent << log;
            nId++;
        }
		return 0;
        //if(CtCsvFile::getInstance().saveFile(sFileName, sLogContent))
        //{
        //    return 0;
        //}
        //else
        //{
        //    return -1;
        //}
    }
    return -1;
}

/*************************
 * 设置每页日志行数
 * ***********************/
void OperateLog::setEachPageRows(int nRows)
{
    m_nEachPageRows = nRows;
}

/*************************
 * 获取每页日志行数
 * ***********************/
int OperateLog::getEachPageRows()
{
    return m_nEachPageRows;
}

/*************************
 * 获取日志总页数
 * ***********************/
int OperateLog::getTotalPages()
{
    return m_nPages;
}

/*************************
 * 获取当前页码
 * ***********************/
int OperateLog::getCurPageNo()
{
    return m_nCurPageNo;
}

 int OperateLog::getLastPageNo()
 {
	 return m_logList.size() % m_nEachPageRows;
 }

 int OperateLog::SetFilterType(QString type)
 {
	 m_strFilterType = type;
	 return 0;
 }

 QList<OPERATE_LOG_T> OperateLog::getAllLogListByPage(int nPage, QString sType)
 {    
	 //MY_DEBUG << "nPage:" << nPage;
	 QList<OPERATE_LOG_T> logList;
	 if (nPage > 0 && m_logList.count() > 0)
	 {
		 m_nCurPageNo = nPage;
		 int nRowStart = (nPage - 1)*m_nEachPageRows;
		 int nRowEnd = nPage * m_nEachPageRows - 1;

		 for (int i = nRowStart; i < m_logList.count(); i++)
		 {
			 if (i > nRowEnd)
				 break;

			 OPERATE_LOG_T log = m_logList.at(i);
			 logList.append(log);
		 }
	 }

	 emit sig_updatePageInfo(m_nPages, m_nCurPageNo);

	 return logList;
 }

 /*************************
 * 通过页码获取日志
 * ***********************/
QList<OPERATE_LOG_T> OperateLog::getLogListByPage(int nPage)
{
    //MY_DEBUG << "nPage:" << nPage;
    QList<OPERATE_LOG_T> logList;
    if(nPage > 0 && m_logListFilter.count() > 0)
    {
        m_nCurPageNo = nPage;
        int nRowStart = (nPage - 1)*m_nEachPageRows;
        int nRowEnd = nPage * m_nEachPageRows - 1;

        for(int i = nRowStart; i < m_logListFilter.count(); i++)
        {
            if(i > nRowEnd)
                break;

            OPERATE_LOG_T log = m_logListFilter.at(i);
            logList.append(log);
        }
    }

    emit sig_updatePageInfo(m_nPages, m_nCurPageNo);

    return logList;
}

/*************************
 * 通过时间和类型获取日志
 * ***********************/
QList<OPERATE_LOG_T> OperateLog::getLogListByTimeAndType(QDateTime StartTime, QDateTime EndTime, QString sType)
{
    m_logListFilter.clear();
    QList<OPERATE_LOG_T>::iterator it;
    for(it = m_logList.begin(); it != m_logList.end(); it++)
    {
        QDateTime sLogTime = QDateTime::fromString(it->sTime, "yyyy-MM-dd hh:mm:ss.zzz");
		QString strDays = sLogTime.toString("yyyy-MM-dd hh:mm:ss");
        if(sLogTime.toTime_t() >= StartTime.toTime_t() && sLogTime.toTime_t() <= EndTime.toTime_t())
        {
            if(sType.compare("All") == 0 || it->sType.compare(sType) == 0)
            {
                OPERATE_LOG_T stLog;
                stLog.sTime = it->sTime;
                stLog.sType = it->sType;
                stLog.sUser = it->sUser;
                stLog.sContent = it->sContent;

                m_logListFilter.append(stLog);
            }
        }
    }

    int nTotalRows = m_logListFilter.count();
    int nOffset = (nTotalRows % m_nEachPageRows) > 0 ? 1 : 0;
    m_nPages = nTotalRows / m_nEachPageRows + nOffset;

    QList<OPERATE_LOG_T> logList = getLogListByPage(1);

    emit sig_updatePageInfo(m_nPages, m_nCurPageNo);

    return logList;
}

/*************************
 * 通过内容获取日志
 * ***********************/
QList<OPERATE_LOG_T> OperateLog::getLogListByContent(QString sContent)
{
    m_logListFilter.clear();
    QList<OPERATE_LOG_T>::iterator it;
    for(it = m_logList.begin(); it != m_logList.end(); it++)
    {
        if(it->sContent.contains(sContent, Qt::CaseInsensitive) == true)
        {
            OPERATE_LOG_T stLog;
            stLog.sTime = it->sTime;
            stLog.sType = it->sType;
            stLog.sUser = it->sUser;
            stLog.sContent = it->sContent;
            m_logListFilter.append(stLog);
        }
    }

    int nTotalRows = m_logListFilter.count();
    int nOffset = (nTotalRows % m_nEachPageRows) > 0 ? 1 : 0;
    m_nPages = nTotalRows / m_nEachPageRows + nOffset;

    QList<OPERATE_LOG_T> logList = getLogListByPage(1);

    emit sig_updatePageInfo(m_nPages, m_nCurPageNo);

    return logList;
}
