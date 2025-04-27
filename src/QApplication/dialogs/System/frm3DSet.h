#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include "ui_frm3DSet.h"
#include <QWidget>
#include "frmSetBase.h"

class frm3DSet : public frmSetBase,virtual public QObject
{
    //Q_OBJECT
public:
    frm3DSet(QWidget *parent = Q_NULLPTR);
    ~frm3DSet();
public:
	virtual int GetData(QJsonObject& strData);
	virtual int SetData(QJsonObject& strData);
	virtual int	NewProject();
	int UpdateUI();
	virtual void RetranslationUi();
	virtual QVector<QPair<QString, QString>> GetVecSetName();
private:
    Ui::frm3DSetClass		ui;

	QMap<QString, QString>	m_mapParam;
};