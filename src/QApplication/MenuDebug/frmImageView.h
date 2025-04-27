#pragma once

#include <QtWidgets/QWidget>
#include "ui_frmImageView.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

class FrmImageView : public QWidget
{
    Q_OBJECT

public:
    FrmImageView(QWidget *parent = Q_NULLPTR);	
	~FrmImageView();
public:
	int GetData(QJsonObject& strData);
	int SetData(QJsonObject& strData);
	int	NewProject();
public slots:
	void slot_ChangeVideoCount(int icount = 1);

	//void AddImgViewSlot(int	iNewIndex,QString strCamera,int icount = 1);
	//void DeleteImgViewSlot(int iNewIndex,QString strCamera,int icount = 1);
	//void ModifyImgViewSlot(int iNewIndex, QString strCamera, int icount = 1);
private:
	Ui::ImageViewClass ui;
};