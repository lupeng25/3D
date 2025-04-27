#pragma once

#include <QtWidgets/QWidget>
#include "ui_frmToolFlow.h"
#include <QGridLayout>
#include "../Common/qttreewidget.h"
#include <QTreeView>
#include <QStandardItemModel>

typedef std::pair<QString, std::list<QString> > ToolsPair;

class frmToolFlow : public QWidget
{
    Q_OBJECT
public:
	frmToolFlow(QWidget* parent = Q_NULLPTR);
	virtual ~frmToolFlow();
public:
	int GetData(QJsonObject& strData);
	int SetData(QJsonObject& strData);
	int	NewProject();
public slots:
	int AddToolFlow(QString strTop = "流程",QString strFlow = "Top");
	int DeleteToolFlow(QString strTop = "流程", QString strFlow = "Top");

	void slot_btnFlowAdd_clicked();
	void slot_btnFlowDelete_clicked();
	void slot_btnMoveUp_clicked();
	void slot_btnMoveDown_clicked();
	void slot_btnReName_clicked();
	void slot_SetFlowName();
private:
    Ui::frmToolFlowClass ui;
private slots:
	void slot_FlowDoubleClick(const QModelIndex &index);
	void slot_FlowClick(const QModelIndex &index);

public:
	QString	FindNum(QString str);
protected:
	void changeEvent(QEvent * ev);
	void contextMenuEvent(QContextMenuEvent *event)override;
public:
Q_SIGNALS:
	void sig_ChangeFlow(QString strKey);
	void sig_ReNameFlow(QString strKey,QString strOld,QString strNew);
	void sig_AddFlow(QString strKey,QString strShow);
	void sig_DeleteFlow(QString strKey);
	void sig_SetFlowEnable(QString strKey,bool Endble);

private:   
    void ToolsTreeWidgetInit();
	QList<QStandardItem*> findItems(QString strData);
	QList<QStandardItem*> findStandardItems(QStandardItem* pItem, QString strData);
	void UpdateStandardItems(QStandardItem* pItem);
private:
	QTreeView* ToolTreeWidget = nullptr;
	QStandardItemModel* m_ptrItems = nullptr;
	
    std::vector<ToolsPair> ToolNamesVec;
    std::list<QString> ToolNameList;
	QLineEdit*		m_plEdit{nullptr};

};