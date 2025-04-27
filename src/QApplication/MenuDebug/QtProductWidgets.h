#pragma once

#include <QWidget>
#include "ui_QtProductWidgets.h"
#include "Pages/QtWidgetsBase.h"
//#include "Motion/IMC100API.h"
#include "Data/dataVar.h"
#include "Motion\MOtionIO.h"
#include "ImageShow/QGraphicsViews.h"

#include "Camerainterface.h"
#include <QRunnable>
#include <QThreadPool>
#include <QButtonGroup>
#include <QtConcurrent>
#include "Log/MyEvent.h"
#include "halconcpp\HalconCpp.h"
using namespace HalconCpp;
const QString strCameraName = "C4";

class QtProductWidgets;

class VisionProductTask : public QRunnable {
public:
	QtProductWidgets*		m_ptrVision{ nullptr };
	HalconCpp::HObject		m_hImage;
	QString					m_strCamName;
	void run() override;
public:
	int RecieveImgBuffer(QString strCamName, unsigned char* gray, int iwidth, int iheight, int ibit, QString imgtype, int icount);
};

class QtProductWidgets : public QtWidgetsBase, public CameraStream
{
	Q_OBJECT
public:
	QtProductWidgets(QWidget *parent = Q_NULLPTR);
	~QtProductWidgets();
public:
	QImage ByteToQImage(unsigned char* bytes, int width, int height, int bytesPerLine, QImage::Format imageFormat);
	
	virtual int RecieveBuffer(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	//当使用当前接口时8为使用Rgray 使用 其他都应该Ggray Bgray为nullptr 
	virtual int RecieveRGBBuffer(QString strCamName, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

public:
	void Connected();
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);
	virtual int	NewProject();

	virtual	int GetSystemData(QJsonObject& strData);
	virtual	int SetSystemData(QJsonObject& strData);

	virtual int SCanTimerUI();
	void UpDateUI();
	bool CheckCylinderSafety();
signals:
	void sigSetQTableWidget(int irow, int iCol,QWidget *widget,QTableWidget * pTable);

public slots :
	void on_btnTest_clicked();
	void on_btnTrriger_clicked();

	void on_btnStopAllAxis_clicked();

	void slotbtnMoveAxisPX_Pressed();
	void slotbtnMoveAxisNX_Pressed();
	void slotbtnMoveAxisPY_Pressed();
	void slotbtnMoveAxisNY_Pressed();
	void slotbtnMoveAxisPZ_Pressed();
	void slotbtnMoveAxisNZ_Pressed();

	void slotbtnStopAxis();

	//void on_btnPhotoLearn_clicked();
	//void on_btnPhotoMove_clicked();

private slots :
	void slotContextMenuRequested(const QPoint &pos);
	void slotWasteContextMenuRequested(const QPoint &pos);
	void OnAddPos();
	void OnDeletePos();
	void OnClearPos();
	void OnMoveModify();
	void OnMouseClicked(QList<QGraphicsItem *>& item);

	void slotUpDateExposure();
	void slotUpDateGain();
	void slotUpDateLight();
	void slotSetSpeed();
	void slotChangeCamera(int);

	void slotPressIOOutPut();

	void slotUpDateParam(int index);
	void slotChangeDistance(int index);
	void UpDateParam();

	void slotChangeAxisSpeedParam();
	void AddtabWidgetRow(QTableWidget*tabWidget,int row, AxisSpeed speed);
	int GettabWidgetSpeed(QTableWidget*tabWidget,QMap<int, AxisSpeed>& speed);

	void slotChangeTab(int itab);

	void slotUpDateCameras();
	void slot_PoscellClicked(int row, int column);
	void slotChangeCameraParamIndex(int iIndex);
private slots :

	void on_btnWorkApplyPos_clicked();
	void slot_SetQTableWidget(int iRow, int iCol, QWidget *widget, QTableWidget * pTable);
private slots :
	void on_btnPortIdentify_clicked();
	void on_btnWorkZifuTestPos_clicked();
private:
	void ResizeUI();
	void UpdateIndex();

	void moveRow(QTableWidget* pTable, int nFrom, int nTo);

	int ChangeTableToData(QTableWidget* pTable, QJsonArray& strData);
	int ChangeDataToTable(QJsonArray& strData, QTableWidget* pTable);
protected:
	virtual void resizeEvent(QResizeEvent *ev);
	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);
	virtual	bool event(QEvent *ev) override;

private:
	Ui::QtProductWidgetsClass	ui;
	int							m_icomId;
	QGraphicsViews*				m_pMarkGraphViews{ nullptr };
	QVector<QCheckBox*>			m_vecIOOutPut;
	QButtonGroup*				m_pBtnGroupRadio{ nullptr };
	Camerainterface*			m_pCamera{ nullptr };
	int							m_iModeIndex = 0;
public:
	int Excute(QString strCamName, HalconCpp::HObject& image);

	CameraParam				m_MarkBparam;
	CameraResultParam		m_MatchResult;
	QCreateEvent*			m_pQCreateEvent{ nullptr };
	HObject					m_dstImage;
public:
	bool m_bDealVisionMode = false;
	void SetDealMode(bool flag = false) { m_bDealVisionMode = flag; }
};