#pragma once

#include <QtWidgets/QWidget>
#include "ui_frmToolsTree.h"
#include <QGridLayout>
#include "Common/qttreewidget.h"

typedef std::pair<QString, std::list<QString> > ToolsPair;

class frmToolsTree : public QWidget
{
    Q_OBJECT

public:
    frmToolsTree(QWidget* parent = Q_NULLPTR);

	QList<QTreeWidgetItem*> findItems(QString strData);
	QList<QTreeWidgetItem*> findStandardItems(QTreeWidgetItem* pItem, QString strData);
private:
    Ui::frmToolsTreeClass ui;
private slots:
	void FlowButtonSlot(QTreeWidgetItem *item, int column);

private:   
    void ToolsTreeWidgetInit();

protected:
	void changeEvent(QEvent * ev);
private:
	QtTreeWidget* ToolTreeWidget{ nullptr };
};
