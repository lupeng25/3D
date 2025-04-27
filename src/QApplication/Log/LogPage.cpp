#include "LogPage.h"
#include "Data/dataVar.h"
#include <QToolTip>
#include "Log/savelog.h"
#include "MyEvent.h"
#include <QScrollBar>

LogPage::LogPage(QWidget *parent)
	: FBaseDialog(parent)
{
	normalSize = QSize(1200, 900);
	QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
	ui.setupUi(this);
	mainLayout->addLayout(ui.gridLayout);

	//setAttribute(Qt::WA_StyledBackground, true);
	initTableView();
	initLogTypeComboBox();
	initDateTimeEdit();
	getTitleBar()->setMaxButtonVisible(true);
	initConnect();
	//dataVar::Instance().m_DialogbaseLst.append(this);
}

LogPage::~LogPage()
{
	m_pTableModel->clear();
	//dataVar::Instance().m_DialogbaseLst.removeOne(this);
}

int LogPage::Init()
{
	initUIData();
	return 0;
}

int LogPage::LoadData()
{
	return 0;
}

int LogPage::SaveData()
{
	return 0;
}

int LogPage::UpdateUI()
{
	return 0;
}

QString LogPage::Info()
{
	return QString("Log");
}

QString LogPage::Vision()
{
	return QString("0.0");
}

void LogPage::initTableView()
{
	m_pTableModel = new QStandardItemModel(this);
	ui.tableView->setModel(m_pTableModel);//设置model

	QStringList headerList;
	headerList << "ID" << tr("Time") << tr("User") << tr("Log Type") << tr("Log Content");
	m_pTableModel->setHorizontalHeaderLabels(headerList);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setItemDelegate(new MyItemDelegate(ui.tableView)); // 设置自定义代理
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->verticalHeader()->hide();
	ui.tableView->setMouseTracking(true);
	//ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	setColumnWidth();

	OperateLog::getInstance().setEachPageRows(50);
}
void LogPage::initLogTypeComboBox()
{
	ui.cbxLogType->clear();
	ui.cbxLogType->addItem(tr("All"));
	ui.cbxLogType->addItem(tr("debug"));
	ui.cbxLogType->addItem(tr("info"));
	ui.cbxLogType->addItem(tr("warning"));
	ui.cbxLogType->addItem(tr("critical"));
	ui.cbxLogType->addItem(tr("error"));
	ui.cbxLogType->addItem(tr("trace"));
	ui.cbxLogType->setCurrentIndex(0);
}

void LogPage::initDateTimeEdit()
{
	QDateTime EndTime = QDateTime::currentDateTime();
	QDateTime _StartTime = QDateTime::fromString(EndTime.toString("yyyy-MM-dd"), "yyyy-MM-dd");
	ui.dateTimeStart->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
	ui.dateTimeStart->setCalendarPopup(true);
	ui.dateTimeStart->setDateTime(_StartTime);

	ui.dateTimeEnd->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
	ui.dateTimeEnd->setCalendarPopup(true);
	ui.dateTimeEnd->setDateTime(_StartTime.addDays(1));
}

void LogPage::initConnect()
{
	connect(&OperateLog::getInstance(), &OperateLog::sig_updatePageInfo,this, &LogPage::slot_show_pageinfo);
	connect(ui.tableView, &QTableView::entered, this, &LogPage::showToolTip);

	connect(SaveLog::Instance(), &SaveLog::sigUpdate, this, &LogPage::slotUpdate, Qt::DirectConnection);
	connect(ui.cbxLogType, &QComboBox::currentTextChanged, this, [](const QString & str) {
		OperateLog::getInstance().SetFilterType(str);
	});
}

void LogPage::initUIData()
{
	on_btnSearch_clicked();
}

void LogPage::showLogByList(QList<OPERATE_LOG_T> logList)
{
	if (m_pTableModel->rowCount() > 0)
		m_pTableModel->removeRows(0, m_pTableModel->rowCount());

	if (logList.count() <= 0)
		return;

	QList<OPERATE_LOG_T>::iterator it;
	int nRow = 0;
	for (it = logList.begin(); it != logList.end(); ++it)
	{
		m_pTableModel->insertRow(nRow, QModelIndex());
		
		m_pTableModel->setData(m_pTableModel->index(nRow, 0), nRow + 1);
		m_pTableModel->setData(m_pTableModel->index(nRow, 1), it->sTime);
		m_pTableModel->setData(m_pTableModel->index(nRow, 2), tr(it->sUser.toStdString().c_str()));
		m_pTableModel->setData(m_pTableModel->index(nRow, 3), tr(it->sType.toStdString().c_str()));
		m_pTableModel->setData(m_pTableModel->index(nRow, 4), it->sContent);
		for (int j = 0; j < 4; j++)	m_pTableModel->item(nRow, j)->setTextAlignment(Qt::AlignCenter);
		nRow++;
	}
}

void LogPage::setColumnWidth()
{
	QMutexLocker guard(&m_lock);
	int nWidth = ui.tableView->width() - 4;
	ui.tableView->setColumnWidth(0, qRound(nWidth*0.05));
	ui.tableView->setColumnWidth(1, qRound(nWidth*0.15));
	ui.tableView->setColumnWidth(2, qRound(nWidth*0.1));
	ui.tableView->setColumnWidth(3, qRound(nWidth*0.1));
	ui.tableView->setColumnWidth(4, qRound(nWidth*0.6));
}

void LogPage::resizeEvent(QResizeEvent *)
{
	setColumnWidth();
}

void LogPage::slot_show_pageinfo(int nPages, int nPageNo)
{
	QString sPages = QString("%1").arg(nPages);
	QString sPageNo = QString("%1").arg(nPageNo);
	ui.label_TotalPage->setText(sPages);
	ui.label_PageNo->setText(sPageNo);
}

void LogPage::on_btnSearch_clicked()
{
	QDateTime startTime = ui.dateTimeStart->dateTime();
	QDateTime endTime = ui.dateTimeEnd->dateTime();
	QString sType = ui.cbxLogType->currentText();
	OperateLog::getInstance().LoadLog(startTime, endTime);
	QList<OPERATE_LOG_T> logList = OperateLog::getInstance().getLogListByTimeAndType(startTime, endTime, sType);

	on_btnLastPage_clicked();
}

void LogPage::showToolTip(const QModelIndex & index)
{
	if (!index.isValid())
		return;

	QToolTip::showText(QCursor::pos(), index.data().toString());
}

void LogPage::slotUpdate()	//更新一条
{
	QApplication::postEvent(this, new MsgEvent(0, 0,""));
}

void LogPage::UpdateLog()
{
	m_lock.lock();
	int nRow = m_pTableModel->rowCount();
	QString strlogType = ui.cbxLogType->currentText();

	auto& iter = OperateLog::getInstance().m_logList;
	iter.last();
	if (strlogType == "All" || strlogType == (iter.last()).sType)
	{
		if (nRow >= OperateLog::getInstance().getEachPageRows())
		{
			if (m_pTableModel->rowCount() > 0)
				m_pTableModel->removeRows(0, m_pTableModel->rowCount());
			nRow = 0;

			int nTotalPages = OperateLog::getInstance().getLastPageNo();
			if (nTotalPages == 1)
			{

			}
			else
			{
				int nTotalPages = OperateLog::getInstance().getTotalPages();
				QList<OPERATE_LOG_T> logList = OperateLog::getInstance().getAllLogListByPage(nTotalPages, strlogType);
				for (auto Aiter : logList)
				{
					m_pTableModel->insertRow(nRow, QModelIndex());
					m_pTableModel->setData(m_pTableModel->index(nRow, 0), nRow + 1);
					m_pTableModel->setData(m_pTableModel->index(nRow, 1), (Aiter).sTime);
					m_pTableModel->setData(m_pTableModel->index(nRow, 2), tr((Aiter).sUser.toStdString().c_str()));
					m_pTableModel->setData(m_pTableModel->index(nRow, 3), tr((Aiter).sType.toStdString().c_str()));
					m_pTableModel->setData(m_pTableModel->index(nRow, 4), (Aiter).sContent);
					for (int j = 0; j < 4; j++)	m_pTableModel->item(nRow, j)->setTextAlignment(Qt::AlignCenter);
					nRow++;
				}

				m_lock.unlock();
				return;
			}
		}
		m_pTableModel->insertRow(nRow, QModelIndex());

		m_pTableModel->setData(m_pTableModel->index(nRow, 0), nRow + 1);
		m_pTableModel->setData(m_pTableModel->index(nRow, 1), (iter.last()).sTime);
		m_pTableModel->setData(m_pTableModel->index(nRow, 2), tr((iter.last()).sUser.toStdString().c_str()));
		m_pTableModel->setData(m_pTableModel->index(nRow, 3), tr((iter.last()).sType.toStdString().c_str()));
		m_pTableModel->setData(m_pTableModel->index(nRow, 4), (iter.last()).sContent);
		for (int j = 0; j < 4; j++)	m_pTableModel->item(nRow, j)->setTextAlignment(Qt::AlignCenter);
	}
	m_lock.unlock();
}

void LogPage::on_btnFirstPage_clicked()
{
	QList<OPERATE_LOG_T> logList = OperateLog::getInstance().getLogListByPage(1);
	showLogByList(logList);
}

void LogPage::on_btnLastPage_clicked()
{
	int nTotalPages = OperateLog::getInstance().getTotalPages();
	QList<OPERATE_LOG_T> logList = OperateLog::getInstance().getLogListByPage(nTotalPages);
	showLogByList(logList);
}

void LogPage::on_btnPreviousPage_clicked()
{
	int nCurPageNo = OperateLog::getInstance().getCurPageNo();
	if (nCurPageNo > 1)
	{
		QList<OPERATE_LOG_T> logList = OperateLog::getInstance().getLogListByPage(nCurPageNo - 1);
		showLogByList(logList);
	}
}

void LogPage::on_btnNextPage_clicked()
{
	int nCurPageNo = OperateLog::getInstance().getCurPageNo();
	int nTotalPages = OperateLog::getInstance().getTotalPages();

	if (nCurPageNo > 0 && nCurPageNo < nTotalPages)
	{
		QList<OPERATE_LOG_T> logList = OperateLog::getInstance().getLogListByPage(nCurPageNo + 1);
		showLogByList(logList);
	}
}

void LogPage::on_btnContentSearch_clicked()
{
	QString sContent = ui.lineEdit->text();
	QList<OPERATE_LOG_T> logList = OperateLog::getInstance().getLogListByContent(sContent);
	showLogByList(logList);
}

bool LogPage::event(QEvent * ev)
{
	if (ev->type() == MsgEvent::EventType)
	{
		MsgEvent* MyEvent = (MsgEvent*)ev;
		if (MyEvent != nullptr)
			if(MyEvent->iIndex == 0 
				&& MyEvent->isubIndex == 0)
			{
				//UpdateLog();
			};
	}
	return QWidget::event(ev);
}

void LogPage::closeEvent(QCloseEvent * ev)
{
	ev->ignore();
	hide();
}

void LogPage::showEvent(QShowEvent * ev)
{
	FBaseDialog::showEvent(ev);
	on_btnLastPage_clicked();
	if (ui.tableView->verticalScrollBar() != nullptr)
		ui.tableView->verticalScrollBar()->setValue(ui.tableView->verticalScrollBar()->maximum());
}

void LogPage::changeEvent(QEvent * ev)
{
	if (NULL != ev) {
		switch (ev->type()) {
		case QEvent::LanguageChange: {
			ui.retranslateUi(this);
			getTitleBar()->getTitleLabel()->setText(tr("Log"));
			getTitleBar()->getContentLabel()->setText(tr(""));
			QStringList headerList;
			headerList << tr("ID") << tr("Time") << tr("User") << tr("Log Type") << tr("Log Content");
			m_pTableModel->setHorizontalHeaderLabels(headerList);
			UpdateLog();
		}	break;
		default:
			break;
		}
	}
	QWidget::changeEvent(ev);
}
