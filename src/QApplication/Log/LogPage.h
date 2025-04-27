#pragma once

#include <QWidget>
//#include "ui_logPage.h"
#include <QStandardItemModel>
#include <QMutex>
#include "operatelog.h"
#include "Common/Dialogbase.h"
#include <QPainter>
#include "QStyledItemDelegate.h"
#include "QFramer/fbasedialog.h"
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>


class Ui_LogPageClass
{
public:
	QGridLayout *gridLayout;
	QVBoxLayout *verticalLayout;
	QTableView *tableView;
	QWidget *widget;
	QVBoxLayout *verticalLayout_2;
	QHBoxLayout *horizontalLayout_2;
	QLabel *label_PageNoDes;
	QLabel *label_PageNo;
	QLabel *label_TotalPageDes;
	QLabel *label_TotalPage;
	QPushButton *btnSearch;
	QFormLayout *formLayout_3;
	QLabel *label;
	QComboBox *cbxLogType;
	QLabel *label_2;
	QDateTimeEdit *dateTimeStart;
	QLabel *label_3;
	QDateTimeEdit *dateTimeEnd;
	QPushButton *btnFirstPage;
	QPushButton *btnPreviousPage;
	QPushButton *btnNextPage;
	QPushButton *btnLastPage;
	QHBoxLayout *horizontalLayout_10;
	QLineEdit *lineEdit;
	QPushButton *btnContentSearch;
	QSpacerItem *verticalSpacer;

	void setupUi(QWidget *LogPageClass)
	{
		if (LogPageClass->objectName().isEmpty())
			LogPageClass->setObjectName(QString::fromUtf8("LogPageClass"));
		LogPageClass->resize(735, 576);
		gridLayout = new QGridLayout/*(LogPageClass)*/;
		gridLayout->setSpacing(0);
		gridLayout->setContentsMargins(11, 11, 11, 11);
		gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
		gridLayout->setContentsMargins(0, 0, 0, 0);
		verticalLayout = new QVBoxLayout();
		verticalLayout->setSpacing(0);
		verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
		tableView = new QTableView(LogPageClass);
		tableView->setObjectName(QString::fromUtf8("tableView"));
		tableView->setMinimumSize(QSize(400, 0));
		tableView->setLineWidth(0);
		tableView->setShowGrid(true);
		tableView->setGridStyle(Qt::SolidLine);
		tableView->horizontalHeader()->setStretchLastSection(true);
		tableView->verticalHeader()->setStretchLastSection(false);

		verticalLayout->addWidget(tableView);


		gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

		widget = new QWidget(LogPageClass);
		widget->setObjectName(QString::fromUtf8("widget"));
		widget->setMinimumSize(QSize(200, 0));
		verticalLayout_2 = new QVBoxLayout(widget);
		verticalLayout_2->setSpacing(10);
		verticalLayout_2->setContentsMargins(11, 11, 11, 11);
		verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
		horizontalLayout_2 = new QHBoxLayout();
		horizontalLayout_2->setSpacing(6);
		horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
		label_PageNoDes = new QLabel(widget);
		label_PageNoDes->setObjectName(QString::fromUtf8("label_PageNoDes"));
		label_PageNoDes->setMinimumSize(QSize(0, 30));

		horizontalLayout_2->addWidget(label_PageNoDes);

		label_PageNo = new QLabel(widget);
		label_PageNo->setObjectName(QString::fromUtf8("label_PageNo"));
		label_PageNo->setMinimumSize(QSize(0, 30));

		horizontalLayout_2->addWidget(label_PageNo);

		label_TotalPageDes = new QLabel(widget);
		label_TotalPageDes->setObjectName(QString::fromUtf8("label_TotalPageDes"));
		label_TotalPageDes->setMinimumSize(QSize(0, 30));

		horizontalLayout_2->addWidget(label_TotalPageDes);

		label_TotalPage = new QLabel(widget);
		label_TotalPage->setObjectName(QString::fromUtf8("label_TotalPage"));
		label_TotalPage->setMinimumSize(QSize(0, 30));

		horizontalLayout_2->addWidget(label_TotalPage);


		verticalLayout_2->addLayout(horizontalLayout_2);

		btnSearch = new QPushButton(widget);
		btnSearch->setObjectName(QString::fromUtf8("btnSearch"));
		btnSearch->setMinimumSize(QSize(60, 50));

		verticalLayout_2->addWidget(btnSearch);

		formLayout_3 = new QFormLayout();
		formLayout_3->setSpacing(6);
		formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
		formLayout_3->setContentsMargins(4, -1, -1, -1);
		label = new QLabel(widget);
		label->setObjectName(QString::fromUtf8("label"));
		label->setMinimumSize(QSize(60, 40));

		formLayout_3->setWidget(0, QFormLayout::LabelRole, label);

		cbxLogType = new QComboBox(widget);
		cbxLogType->setObjectName(QString::fromUtf8("cbxLogType"));
		cbxLogType->setMinimumSize(QSize(60, 40));

		formLayout_3->setWidget(0, QFormLayout::FieldRole, cbxLogType);

		label_2 = new QLabel(widget);
		label_2->setObjectName(QString::fromUtf8("label_2"));
		label_2->setMinimumSize(QSize(60, 50));

		formLayout_3->setWidget(1, QFormLayout::LabelRole, label_2);

		dateTimeStart = new QDateTimeEdit(widget);
		dateTimeStart->setObjectName(QString::fromUtf8("dateTimeStart"));
		dateTimeStart->setMinimumSize(QSize(60, 50));

		formLayout_3->setWidget(1, QFormLayout::FieldRole, dateTimeStart);

		label_3 = new QLabel(widget);
		label_3->setObjectName(QString::fromUtf8("label_3"));
		label_3->setMinimumSize(QSize(60, 50));

		formLayout_3->setWidget(2, QFormLayout::LabelRole, label_3);

		dateTimeEnd = new QDateTimeEdit(widget);
		dateTimeEnd->setObjectName(QString::fromUtf8("dateTimeEnd"));
		dateTimeEnd->setMinimumSize(QSize(60, 50));

		formLayout_3->setWidget(2, QFormLayout::FieldRole, dateTimeEnd);


		verticalLayout_2->addLayout(formLayout_3);

		btnFirstPage = new QPushButton(widget);
		btnFirstPage->setObjectName(QString::fromUtf8("btnFirstPage"));
		btnFirstPage->setMinimumSize(QSize(60, 50));

		verticalLayout_2->addWidget(btnFirstPage);

		btnPreviousPage = new QPushButton(widget);
		btnPreviousPage->setObjectName(QString::fromUtf8("btnPreviousPage"));
		btnPreviousPage->setMinimumSize(QSize(60, 50));

		verticalLayout_2->addWidget(btnPreviousPage);

		btnNextPage = new QPushButton(widget);
		btnNextPage->setObjectName(QString::fromUtf8("btnNextPage"));
		btnNextPage->setMinimumSize(QSize(60, 50));

		verticalLayout_2->addWidget(btnNextPage);

		btnLastPage = new QPushButton(widget);
		btnLastPage->setObjectName(QString::fromUtf8("btnLastPage"));
		btnLastPage->setMinimumSize(QSize(60, 50));

		verticalLayout_2->addWidget(btnLastPage);

		horizontalLayout_10 = new QHBoxLayout();
		horizontalLayout_10->setSpacing(6);
		horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
		lineEdit = new QLineEdit(widget);
		lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
		QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
		lineEdit->setSizePolicy(sizePolicy);
		lineEdit->setMinimumSize(QSize(60, 50));

		horizontalLayout_10->addWidget(lineEdit);

		btnContentSearch = new QPushButton(widget);
		btnContentSearch->setObjectName(QString::fromUtf8("btnContentSearch"));
		btnContentSearch->setMinimumSize(QSize(60, 50));

		horizontalLayout_10->addWidget(btnContentSearch);


		verticalLayout_2->addLayout(horizontalLayout_10);

		verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

		verticalLayout_2->addItem(verticalSpacer);


		gridLayout->addWidget(widget, 0, 1, 1, 1);


		retranslateUi(LogPageClass);

		QMetaObject::connectSlotsByName(LogPageClass);
	} // setupUi

	void retranslateUi(QWidget *LogPageClass)
	{
		LogPageClass->setWindowTitle(QApplication::translate("LogPageClass", "logPage", nullptr));
		label_PageNoDes->setText(QApplication::translate("LogPageClass", "PageNo:", nullptr));
		label_PageNo->setText(QApplication::translate("LogPageClass", "0", nullptr));
		label_TotalPageDes->setText(QApplication::translate("LogPageClass", "Total Pages:", nullptr));
		label_TotalPage->setText(QApplication::translate("LogPageClass", "0", nullptr));
		btnSearch->setText(QApplication::translate("LogPageClass", "Search", nullptr));
		label->setText(QApplication::translate("LogPageClass", "Log Type:", nullptr));
		label_2->setText(QApplication::translate("LogPageClass", "Start Time:", nullptr));
		label_3->setText(QApplication::translate("LogPageClass", "End Time:", nullptr));
		btnFirstPage->setText(QApplication::translate("LogPageClass", "First Page", nullptr));
		btnPreviousPage->setText(QApplication::translate("LogPageClass", "Previous Page", nullptr));
		btnNextPage->setText(QApplication::translate("LogPageClass", "Next Page", nullptr));
		btnLastPage->setText(QApplication::translate("LogPageClass", "Last Page", nullptr));
		btnContentSearch->setText(QApplication::translate("LogPageClass", "Search", nullptr));
	} // retranslateUi

};

namespace Ui {
	class LogPageClass : public Ui_LogPageClass {};
} // namespace Ui

class LogPage : public FBaseDialog
{
	Q_OBJECT
public:
	LogPage(QWidget *parent = nullptr);
	~LogPage();

public:
	virtual	int Init();
	virtual	int LoadData();
	virtual	int SaveData();
	virtual	int UpdateUI();
	QString Info();
	QString Vision();
public:
	void initTableView();
	void initLogTypeComboBox();
	void initDateTimeEdit();
	void initConnect();
	void initUIData();

	//通过获取到的列表更新日志
	void showLogByList(QList<OPERATE_LOG_T> logList);

	void setColumnWidth();
	void resizeEvent(QResizeEvent *); //拉伸

public slots:
	//显示总页数和页码
	void slot_show_pageinfo(int nPages, int nPageNo);
	void showToolTip(const QModelIndex &index);
	void slotUpdate();
	void UpdateLog();
public slots:
	void on_btnSearch_clicked();
	void on_btnFirstPage_clicked();
	void on_btnLastPage_clicked();
	void on_btnPreviousPage_clicked();
	void on_btnNextPage_clicked();
	void on_btnContentSearch_clicked();
protected:
	bool event(QEvent *ev) override;
	void closeEvent(QCloseEvent* e) override;
	void showEvent(QShowEvent *ev);
	void changeEvent(QEvent * e);
private:
	Ui::LogPageClass ui;

private:
	QStandardItemModel *m_pTableModel{ nullptr };
	QMutex m_lock;
};

class MyItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit MyItemDelegate(QObject* parent = nullptr)
		: QStyledItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {

		QString str = index.sibling(index.row(), 3).data().toString();
		if (str == tr("trace") || str == ("trace"))
		{
			QBrush brush(Qt::gray);
			painter->fillRect(option.rect, brush);
			QStyledItemDelegate::paint(painter, option, index);
		}
		else if (str == tr("debug") || str == ("debug"))
		{
			QBrush brush(Qt::lightGray);
			painter->fillRect(option.rect, brush);
			QStyledItemDelegate::paint(painter, option, index);
		}
		else if (str == tr("info") || str == ("info"))
		{
			QBrush brush(Qt::darkGray);
			painter->fillRect(option.rect, brush);
			QStyledItemDelegate::paint(painter, option, index);
		}
		else if (str == tr("warning") || str == ("warning"))
		{
			QBrush brush(Qt::darkYellow);
			
			painter->setPen(Qt::darkRed); // 红色前景
			painter->fillRect(option.rect, brush);
			QStyledItemDelegate::paint(painter, option, index);
		}
		else if (str == tr("critical") || str == ("critical"))
		{
			QBrush brush(Qt::red);
			painter->setPen(Qt::red); // 红色前景
			painter->fillRect(option.rect, brush);
			QStyledItemDelegate::paint(painter, option, index);
		}
		else if (str == tr("fatal") || str == ("fatal"))
		{
			QBrush brush(Qt::red);
			painter->fillRect(option.rect, brush);
			QStyledItemDelegate::paint(painter, option, index);
		}
		else
		{
			QStyledItemDelegate::paint(painter, option, index);
		}

	}
};