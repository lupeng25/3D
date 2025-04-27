#include "QtUserWidgets.h"
#include "Data/dataVar.h"

QtUserWidgets::QtUserWidgets(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);
	setWindowFlag(Qt::FramelessWindowHint); // 设置窗口无边框

	initForm();
	on_btnSelect_clicked();
	connect(this, &QtUserWidgets::sigExcuteSql, this, &QtUserWidgets::slotExcuteSql);
	connect(ui.tableView, &QTableView::pressed, this, &QtUserWidgets::slotActive);
}

QtUserWidgets::~QtUserWidgets()
{
}

bool QtUserWidgets::CheckPageAvalible()
{
	on_btnSelect_clicked();
	return true;
}

int QtUserWidgets::CloseUI()
{
	return 0;
}

int QtUserWidgets::initData(QString& strError)
{
	return 0;
}

//bool QtUserWidgets::CheckNextPageAvalible()
//{
//	on_btnSelect_clicked();
//	return true;
//}

void QtUserWidgets::initForm()
{
	columnNames.clear();
	columnWidths.clear();

	tableName = "LoginTable";
	countName = "rowid";
	SetColumnNames();

	columnWidths.append(160);
	columnWidths.append(160);
	columnWidths.append(160);
	columnWidths.append(160);

	//设置需要显示数据的表格和翻页的按钮
	pdbPage = new DbPage(this);
	//设置所有列居中显示
	pdbPage->setAllCenter(true);
	pdbPage->setControl(ui.tableView, ui.labPageTotal, ui.labPageCurrent, ui.labRecordsTotal, ui.labRecordsPerpage,
		ui.labSelectTime, 0, ui.btnFirst, ui.btnPreVious, ui.btnNext, ui.btnLast, countName);
	ui.tableView->horizontalHeader()->setStretchLastSection(true);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
}

void QtUserWidgets::SetColumnNames()
{
	columnNames.clear();
	columnNames.append(tr("UserName"));
	columnNames.append(tr("Password"));
	columnNames.append(tr("Limit"));
	columnNames.append(tr("BlankMark"));

}

void QtUserWidgets::slotExcuteSql(QString strDatabase, QString strSql)
{
	//打开数据库,整个应用程序可用
	QSqlDatabase m_dbConn;
	////建立并打开数据库
	if (QSqlDatabase::contains(strDatabase))
		m_dbConn = QSqlDatabase::database(strDatabase);
	else
	{
		QSqlDatabase::removeDatabase(strDatabase);
		m_dbConn = QSqlDatabase::addDatabase("QSQLITE", strDatabase);
	}
	QString	strPath = dataVar::Instance().ConfigFolder + strDatabase + ".db";
	m_dbConn.setDatabaseName(strPath);
	if (!m_dbConn.isOpen())
		m_dbConn.open();
	if (!m_dbConn.isOpen())
		dataVar::Instance().pManager->notify(tr("Warnning"), strPath + tr(" File Open Error!"), NotifyType_Warn);
	//bool ok = m_dbConn.open();
	QSqlQuery query(m_dbConn);
	QString sql = strSql;
	query.exec(sql);
}

void QtUserWidgets::slotUpdate(QString strName)
{
	QStringList namelst;
	{
		QSqlDatabase _dbConn;
		QString sql = QString("SELECT* FROM LoginTable WHERE USERNAME = '%1'").arg(strName);
		_dbConn = QSqlDatabase::database("MyDataBase");
		QString	strPath = dataVar::Instance().ConfigFolder + "MyDataBase" + ".db";
		_dbConn.setDatabaseName(strPath);
		if (!_dbConn.isOpen())
			_dbConn.open();
		QSqlQuery query(_dbConn);
		query.exec(sql);
		// 遍历结果集中的所有列
		while (query.next()) {
			for (int i = 0; i < query.record().count(); ++i) {
				QVariant data = query.value(i);
				namelst.push_back(data.toString());
			}
		}
	}

	if (namelst.size() > 0)		ui.lEdit_User->setText(namelst[0]);
	if (namelst.size() > 1)		ui.lEdit_Passward->setText(namelst[1]);
	if (namelst.size() > 2)		ui.spinBox->setValue(namelst[2].toInt());
	int _iLevel = dataVar::Instance().m_iLevel - 1;
	_iLevel = max(_iLevel, 0);
	ui.spinBox->setMaximum(_iLevel);
}

void QtUserWidgets::slotActive(const QModelIndex &index)
{
	QAbstractItemModel* model = ui.tableView->model();
	QString strName = model->data(model->index(ui.tableView->currentIndex().row(), 0)).toString();
	slotUpdate(strName);
}

void QtUserWidgets::on_btnADD_clicked()
{
	//查询条码是否一样的
	QSqlQuery query(dataVar::Instance().sqlite.sqldatabase);
	QString	sql;
	QStringList CodeNames;
	{
		sql = QString("SELECT * FROM LoginTable WHERE USERNAME = '%1'").arg(ui.lEdit_User->text());
		query.exec(sql);
		while (query.next()) {
			CodeNames.push_back(query.value(0).toString());
		}
		if (CodeNames.size() > 0) {
			dataVar::Instance().pManager->notify(tr("Warnning"), tr("Create Error! DataBase Have duplicate User"), NotifyType_Warn);
			return;
		}
	}

	sql = QString("INSERT INTO LoginTable (USERNAME,PASSWORD,USERLIMIT) VALUES ('%1','%2','%3')")
		.arg(ui.lEdit_User->text())
		.arg(ui.lEdit_Passward->text())
		.arg(QString::number(ui.spinBox->value()))
		;
	emit sigExcuteSql("MyDataBase", sql);
	on_btnSelect_clicked();
}

void QtUserWidgets::on_btnModify_clicked()
{
	//查询条码是否一样的
	QSqlQuery query(dataVar::Instance().sqlite.sqldatabase);
	QString	sql;
	QStringList CodeNames;
	{
		sql = QString("SELECT * FROM LoginTable WHERE USERNAME = '%1'").arg(ui.lEdit_User->text());
		query.exec(sql);
		while (query.next()) {
			CodeNames.push_back(query.value(0).toString());
		}
		if (CodeNames.size() <= 0) {
			dataVar::Instance().pManager->notify(tr("Warnning"), tr("Create Error! DataBase Didn't Have User"), NotifyType_Warn);
			return;
		}
	}

	auto res = MyMessageBox::question(this, tr("Whether Modify or not?"), tr("Alart"));
	if (res != MyMessageBox::Yes) {
		return;
	}
	sql = QString("update LoginTable set USERNAME = '%1',PASSWORD = '%2',USERLIMIT = '%3' where USERNAME = '%4'")
		.arg(ui.lEdit_User->text())
		.arg(ui.lEdit_Passward->text())
		.arg(QString::number(ui.spinBox->value()))
		.arg(ui.lEdit_User->text())
		;
	emit sigExcuteSql("MyDataBase", sql);
	on_btnSelect_clicked();
}

void QtUserWidgets::on_btnDelete_clicked()
{
	auto res = MyMessageBox::question(this, tr("Whether Delete or not?"), tr("Alart"));
	if (res != MyMessageBox::Yes) {
		return;
	}
	if (ui.lEdit_User->text() == "operator" || ui.lEdit_User->text() == "adminstaotor")
	{
		dataVar::Instance().pManager->notify(tr("Warnning"), tr("The username cannot be Delete!"), NotifyType_Warn);
		on_btnSelect_clicked();
		return;
	}
	QString sql = QString("DELETE FROM LoginTable WHERE USERNAME = '%1'")
		.arg(ui.lEdit_User->text())
		;
	emit sigExcuteSql("MyDataBase", sql);
	on_btnSelect_clicked();
}

void QtUserWidgets::resizeEvent(QResizeEvent * ev)
{
	QtWidgetsBase::resizeEvent(ev);
	ResizeUI();
}

void QtUserWidgets::ResizeUI()
{
	int _iWidth = ui.tableView->width() / 4.0;
	ui.tableView->setColumnWidth(0, _iWidth);
	ui.tableView->setColumnWidth(1, _iWidth);
	ui.tableView->setColumnWidth(2, _iWidth);
	ui.tableView->setColumnWidth(3, _iWidth);
}

void QtUserWidgets::showEvent(QShowEvent * ev)
{
	QtWidgetsBase::showEvent(ev);
	on_btnSelect_clicked();
	ResizeUI();
}

void QtUserWidgets::hideEvent(QHideEvent * ev)
{
	QtWidgetsBase::hideEvent(ev);
}

void QtUserWidgets::changeEvent(QEvent * ev)
{
	if (NULL != ev) {
		switch (ev->type()) {
		case QEvent::LanguageChange: {
			RetranslationUi();
		}	break;
		default:
			break;
		}
	}
}

void QtUserWidgets::RetranslationUi()
{
	ui.retranslateUi(this);
	if (pdbPage != nullptr)	pdbPage->RetranslationUi();
	SetColumnNames();
	if (pdbPage) pdbPage->setColumnNames(columnNames);
	if (pdbPage) pdbPage->setColoumnLanName(columnNames);
	if (pdbPage) pdbPage->Update();
	ResizeUI();
}

void QtUserWidgets::on_btnSelect_clicked()
{
	////绑定数据到表格
	//QString sql = "where 1=1";
	//pdbPage->setTableName(tableName);
	//pdbPage->setOrderSql(QString("%1 %2").arg(countName).arg("asc"));
	//pdbPage->setWhereSql(sql);
	//pdbPage->setRecordsPerpage(200);
	//pdbPage->setColumnNames(columnNames);
	//pdbPage->setColumnWidths(columnWidths);
	//pdbPage->setConnName("MyDataBase");
	//pdbPage->select();

	//绑定数据到表格
	QString strSQL;

	QString wheresql = QString(" where USERLIMIT < '%1' ").arg(QString::number(dataVar::Instance().m_iLevel));
	QString sql = QString("select * from LoginTable where USERLIMIT < '%1'").arg(QString::number(dataVar::Instance().m_iLevel));

	pdbPage->setTableName(tableName);
	pdbPage->setOrderSql(QString("%1 %2").arg(countName).arg("asc"));
	pdbPage->setWhereSql(wheresql);
	pdbPage->setRecordsPerpage(200);
	pdbPage->setColumnNames(columnNames);
	pdbPage->setConnName("MyDataBase");
	pdbPage->setColumnWidths(columnWidths);
	pdbPage->selectSql(sql);
	ResizeUI();
}