#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include "ui_frmNormalSet.h"
#include <QWidget>
#include "frmSetBase.h"
#include "Common/CLinguist.h"

class frmNormalSet : public frmSetBase
{
    Q_OBJECT

public:
    explicit frmNormalSet(QWidget *parent = Q_NULLPTR);
    ~frmNormalSet();
public:
	virtual int GetData(QJsonObject& strData);
	virtual int SetData(QJsonObject& strData);
	virtual int	NewProject();
	int UpdateUI();
	virtual void RetranslationUi();
	virtual QVector<QPair<QString, QString>> GetVecSetName();
private:
	void SetProcessAutoRunSelf(const QString& appPath);
	void RemoveProcessAutoRunSelf(const QString& appPath);
private:
    Ui::frmNormalSetClass		ui;
};