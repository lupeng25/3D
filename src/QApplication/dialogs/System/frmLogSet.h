#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include "ui_frmLogSet.h"
#include <QWidget>
#include "frmSetBase.h"

class frmLogSet : public frmSetBase
{
    Q_OBJECT

public:
    explicit frmLogSet(QWidget *parent = Q_NULLPTR);   
    ~frmLogSet();
public:
	virtual int GetData(QJsonObject& strData);
	virtual int SetData(QJsonObject& strData);
	virtual int	NewProject();
	int UpdateUI();
	virtual void RetranslationUi();
	virtual QVector<QPair<QString, QString>> GetVecSetName();
private:
    Ui::frmLogSetClass		ui;

	QMap<QString, QString>	m_mapParam;
};