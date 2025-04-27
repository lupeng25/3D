#include "QWorld.h"
#include "QDateTime"

//删除N天前的文件
bool FolderOperation::FindFileForDelete(QString key, QString path, int day)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{
		QDateTime now = QDateTime::currentDateTime();
		QDateTime dateTime1 = now;
		QDateTime dateTime2;
		QDir dir(path);
		QFileInfoList fileList = dir.entryInfoList();
		for (QFileInfo f : fileList)
		{
			if (f.baseName() == "")
			{
				continue;
			}
			QString str = f.fileName();
			QString _strCreated = f.created().toString("yyyy-MM-dd hh:mm:ss");
			dateTime2 = f.created().addDays(day);
		    QString _StrOld		= dateTime1.toString("yyyy-MM-dd hh:mm:ss");
			QString _StrOld2	= dateTime2.toString("yyyy-MM-dd hh:mm:ss");
			if (dateTime2 < dateTime1)
			{
				dir.remove(f.absoluteFilePath());
			}
		}
		return true;
	}
	catch (...)
	{
		return false;
	}
}
bool FolderOperation::FindFileForDeleteLog(QString key, QString path, int day)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2") {
		return false;
	}
	try
	{
		QDateTime now = QDateTime::currentDateTime();
		QDateTime dateTime1 = now;
		QDateTime dateTime2;
		QDir dir(path);
		QFileInfoList fileList = dir.entryInfoList();
		for (QFileInfo f : fileList) {
			if (f.baseName() == "")	{
				continue;
			}
			QString _strFileName	= f.baseName();
			QStringList _strFiles	= _strFileName.split("_");
			if (_strFiles.size() > 1)	{
				dateTime2 = QDateTime::fromString(_strFiles[1], "yyyy-MM-dd").addDays(day);
			} else {
				dateTime2 = f.created().addDays(day);
			}
			QString _StrOld = dateTime1.toString("yyyy-MM-dd hh:mm:ss");
			QString _StrOld2 = dateTime2.toString("yyyy-MM-dd hh:mm:ss");
			if (dateTime2 < dateTime1) {
				dir.remove(f.absoluteFilePath());
			}
		}
		return true;
	}
	catch (...)
	{
		return false;
	}
}

//删除N天前的文件夹
bool FolderOperation::FindFolderForDelete(QString key, QString path, int day)
{
	if (key != "59226f4700f1cc8a5de755d501af308d2d2")
	{
		return false;
	}
	try
	{		
		QList<QString> dirs;		
		dirs.reserve(1000);
		dirs.clear();
		QDir dir(path);
		dir.setFilter(QDir::Dirs);
		for (auto fullDir : dir.entryInfoList())
		{
			if (fullDir.fileName() == "." || fullDir.fileName() == "..")
				continue;
			dirs.push_back(fullDir.absoluteFilePath());
		}
		if (dirs.size() < day)
		{
			return false;
		}
		else
		{
			QList<quint64> times = QList<quint64>();
			times.reserve(1000);
			times.clear();
			QList<QString> folder = QList<QString>();
			folder.reserve(1000);
			folder.clear();
			for (auto dir : dirs)
			{
				QFileInfo fl(dir);
				QDateTime DT = fl.created();
				QDateTime dt0(QDate(1970, 1, 1));
				quint64 time = DT.toTime_t() - dt0.toTime_t();
				times.append(time);
				folder.append(dir);
			}
			//获取数组中最小值的索引
			int index = -1;
			QVector<quint64> max_and_min(2);
			if (times.size() != 0) {
				max_and_min[1] = times[0];
				index = 0;
				for (int i = 0; i < times.size(); i++) {
					if (max_and_min[1] > times[i]) {
						index = i;
						max_and_min[1] = times[i];
					}
				}
			}
			QDir dir_x;
			if (dir_x.exists(folder[index]))
			{
				dir_x.setPath(folder[index]);
				dir_x.removeRecursively();
			}
			return true;
		}		
	}
	catch (...)
	{
		return false;
	}
}
bool FolderOperation::FindFilesInFolder(QString path, QVector<QString>& Files)
{
	Files.clear();
	QDir dir(path); // 指定目录路径
	QFileInfoList fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

	// 遍历文件信息列表
	foreach(const QFileInfo& fileInfo, fileInfoList) {
		if (fileInfo.isFile()) {
			Files.push_back(fileInfo.fileName());
		}
		else if (fileInfo.isDir()) {

		}
	}

	return false;
}
