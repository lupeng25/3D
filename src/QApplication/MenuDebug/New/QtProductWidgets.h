#pragma once

#include <QWidget>
#include "ui_QtProductWidgets.h"
#include "Pages/QtWidgetsBase.h"
#include "Motion/IMC100API.h"
#include "Data/dataVar.h"
#include "Motion\MOtionIO.h"
#include "ImageShow/QGraphicsViews.h"

#include "Camerainterface.h"
#include <QRunnable>
#include <QThreadPool>
#include <QButtonGroup>
#include "halconcpp\HalconCpp.h"
using namespace HalconCpp;
const QString strCameraName = "C4";

class QtProductWidgets;

class VisionProductTask : public QRunnable {
public:
	QtProductWidgets*	m_ptrVision{ nullptr };
	HalconCpp::HObject		m_hImage;
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

public slots :
	//void on_btnVisionMove_clicked();
	void on_btnTrriger_clicked();

	void on_btnWorkPosMove_clicked();
	void on_btnWorkPhotoPosMove_clicked();
	void on_btnWorkClampPosLearn_clicked();
	void on_btnWorkPosLearn_clicked();

	void on_btnWastePosMove_clicked();
	void on_btnWastePosLearn_clicked();

	void on_btnPhotoSafePosLearn_clicked();
	//void on_btnMoveAxisPZ_clicked();
	//void on_btnMoveAxisNZ_clicked();

	void on_btnStopAllAxis_clicked();

	void slotbtnMoveAxisPX_Pressed();
	void slotbtnMoveAxisNX_Pressed();
	void slotbtnMoveAxisPY_Pressed();
	void slotbtnMoveAxisNY_Pressed();
	void slotbtnMoveAxisPZ_Pressed();
	void slotbtnMoveAxisNZ_Pressed();

	void slotbtnStopAxis();

	void on_btnPhotoLearn_clicked();
	void on_btnPhotoMove_clicked();

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
	void slotChangeCameraParamIndex(int iIndex);
private:
	void ResizeUI();
	void UpdateIndex();

	void moveRow(QTableWidget* pTable, int nFrom, int nTo);
protected:
	virtual void resizeEvent(QResizeEvent *ev);
	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);
	virtual	bool event(QEvent *ev) override;

private:
	Ui::QtProductWidgetsClass	ui;
	int							m_icomId;
	QGraphicsViews*				m_pMarkGraphViews{ nullptr };
	//QGraphicsViews*				m_pMarkScanCodeGraphViews{ nullptr };

	QVector<QCheckBox*>			m_vecIOOutPut;
	QButtonGroup*				m_pBtnGroupRadio{ nullptr };

	HalconCpp::HObject			m_hImage;
	Camerainterface*			m_pCamera{ nullptr };
public:
	int Excute(HalconCpp::HObject& image);

	CameraParam				m_param;
	ResultRetn				m_ResultRetn;
};