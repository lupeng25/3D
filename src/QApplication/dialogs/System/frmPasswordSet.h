#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include "ui_frmPasswordSet.h"
#include <QWidget>
#include "frmSetBase.h"

class frmPasswordSet : public frmSetBase
{
    Q_OBJECT

public:
    explicit frmPasswordSet(QWidget *parent = Q_NULLPTR);
    ~frmPasswordSet();
public:
	virtual bool CheckPageAvalible();
	virtual int GetData(QJsonObject& strData);
	virtual int SetData(QJsonObject& strData);
	virtual int	NewProject();
	int UpdateUI();
	virtual void RetranslationUi();
	virtual QVector<QPair<QString, QString>> GetVecSetName();
private:
    Ui::frmPasswordSetClass		ui;

	QMap<QString, QString>	m_mapParam;
};