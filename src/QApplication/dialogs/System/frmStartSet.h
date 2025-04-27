#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include "ui_frmStartSet.h"
#include <QWidget>
#include "frmSetBase.h"

class frmStartSet : public frmSetBase
{
    Q_OBJECT

public:
    explicit frmStartSet(QWidget *parent = Q_NULLPTR);
    ~frmStartSet();
public:
	virtual int GetData(QJsonObject& strData);
	virtual int SetData(QJsonObject& strData);
	virtual int	NewProject();
	int UpdateUI();
	virtual void RetranslationUi();

	virtual	QVector<QPair<QString, QString>> GetVecSetName();
private slots :
	void slotUpDateStartImages();
	void on_btnSoftWareImg_clicked();

private:	
	void setTextAlignment(QComboBox* cbx, Qt::Alignment alignment = Qt::AlignCenter);
	void SetCurrentName(QString strName);
	void UpdateMinMax();
protected:
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
private:
    Ui::frmStartSetClass		ui;

	QMap<QString, QString>	m_mapParam;
};