#pragma once

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include "ui_QtWaitWidgetsClass.h"
#include "WaitingSpinnerWidget.h"
#include <QPropertyAnimation>
#include <vector>
using namespace std;

enum StandardMsgResult
{
	MsgResult_TimeOut = -3,
	MsgResult_NG = -2,
	MsgResult_Close = -1,
	MsgResult_OK = 0
	//MsgResult_Cancer = 1
};

#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H
#include <QMovie>
#include <QLabel>
#include <QDialog>
#include <QPainter>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#define USER_CANCEL -1
class LoadingDialog : public QDialog
{
	Q_OBJECT
public:
	explicit LoadingDialog(QWidget *parent = nullptr);
	~LoadingDialog();
	//设置提示文本
	void setTipsText(QString strTipsText);
	//设置是否显示取消等待按钮
	void setCanCancel(bool bCanCancel);
	//移动到指定窗口中间显示
	void moveToCenter(QWidget* pParent);
protected:
	void paintEvent(QPaintEvent *event) override;
private:
	void initUi();
Q_SIGNALS:
	void cancelWaiting();
	private slots:
	void cancelBtnClicked();
private:
	QFrame *m_pCenterFrame{ nullptr };
	QLabel *m_pMovieLabel{ nullptr };
	QMovie *m_pLoadingMovie{ nullptr };
	QLabel *m_pTipsLabel{ nullptr };
	QPushButton *m_pCancelBtn{ nullptr };
};
#endif // LOADINGDIALOG_H

#ifndef KQTASKTREAD_H
#define KQTASKTREAD_H

#include <QThread>
#include <QWidget>
#include <functional>

class KQTaskTread : public QThread
{
	Q_OBJECT
public:
	KQTaskTread(QWidget* widget, const std::function<StandardMsgResult()>& task);
	KQTaskTread(QWidget* widget);
public:
	void SetTask(const std::function<StandardMsgResult()>& task);
	void SetEndTask(const std::function<StandardMsgResult()>& task);

signals:
	void taskFinish();

	// QThread interface
protected:
	void run() override;

	std::function<StandardMsgResult()>		m_task		= nullptr;//C++11兼容多种函数声明类型，函数指针
	std::function<StandardMsgResult()>		m_endtask	= nullptr;//C++11兼容多种函数声明类型，函数指针
	QWidget*								m_wait_ui;
public:
	StandardMsgResult						m_MsgResult;
};

#endif // KQTASKTREAD_H

#ifndef RADIUSPROGRESSBAR_H
#define RADIUSPROGRESSBAR_H

#include <QProgressBar>

class KQRadiusProgressBar : public QProgressBar
{
	Q_OBJECT
public:
	explicit KQRadiusProgressBar(QWidget *parent = 0);
signals:

public slots :

protected:
	void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
};

#endif // RADIUSPROGRESSBAR_H
#ifndef RINGSPROGRESSBAR_H
#define RINGSPROGRESSBAR_H

#include <QWidget>

class KQRingsProgressBar : public QWidget
{
	Q_OBJECT
public:
	explicit KQRingsProgressBar(QWidget *parent = 0);
	void setRotateDelta(int delta);
	void setPercent(int percent);

signals:

	public slots :

protected:
	void paintEvent(QPaintEvent *);

private:
	int m_rotateAngle;//旋转角度
	int m_percent; //百分比
};

#endif // RINGSPROGRESSBAR_H

#ifndef KQWAITDLG_H
#define KQWAITDLG_H
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <functional>
#include <QVBoxLayout>
#include <QTime>
class KQWaitDlg;

class KQProgressDisplayControl : public QObject
{
	Q_OBJECT
public:
	explicit KQProgressDisplayControl(KQWaitDlg* waitDlg);

signals:
	void setSteped(int nStep);
	void setStepMsged(QString sMsg);
	void setUpdate();
public:
	void setStep(int nStep);
	void setStepMsg(QString sMsg);
private:
	KQWaitDlg* m_waitDlg;
};

class KQWaitDlg : public QDialog
{
	Q_OBJECT
public:
	enum ProgressType
	{
		RingsProgress,   ///<圆环型圆状进度条
		RadiusProgress, ///<圆角型长条进度条
		RSpotsShrinkCircle,
		RSpotsCircle,
		RStickCircle,
		RTubeCircle,
		RTextCircle,
		RTubeBallCircle,
	};

	explicit KQWaitDlg(QString label = "loading", QWidget *parent = 0);
	void WaitFor(const std::function<StandardMsgResult()>& task, ProgressType nType = RSpotsShrinkCircle);
	void WaitFor(const std::function<StandardMsgResult()>& task, const QString& label = "loading",ProgressType nType = RSpotsShrinkCircle);

	void WaitFor(const std::function<StandardMsgResult()>& task,std::function<StandardMsgResult()> endtask,ProgressType nType = RSpotsShrinkCircle);
	void WaitFor(const std::function<StandardMsgResult()>& task,std::function<StandardMsgResult()> endtask,const QString& label = "loading",ProgressType nType = RSpotsShrinkCircle);
	void Start(ProgressType type = RadiusProgress);
	static void WaitForFunction(const std::function<StandardMsgResult()>& task,QString label = "loading", QWidget *parent = nullptr,ProgressType nType = RSpotsShrinkCircle);

public slots:
	void setStep(int nStep);
	void setStepMsg(QString sMsg);
	void setUpdate();
	void updateMsg();
	// QWidget interface
protected:
	void showEvent(QShowEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void closeEvent(QCloseEvent *) override;

private:
	KQTaskTread*				m_thread = nullptr;
	//std::function<void()>   m_task;

	ProgressType m_progressType;
	QString m_sMsg;

	QVBoxLayout* m_Layout;
	QLabel*						m_label = nullptr;
	KQRadiusProgressBar*		m_radiusProgress = nullptr;
	KQRingsProgressBar*			m_ringsProgress = nullptr;
	SpotsShrinkCircle*			m_pSpotsShrinkCircle{ nullptr };
	SpotsCircle*				m_pSpotsCircle{ nullptr };
	StickCircle*				m_pStickCircle{ nullptr };
	TubeCircle*					m_pTubeCircle{ nullptr };
	TextCircle*					m_pTextCircle{ nullptr };
	TubeBallCircle*				m_pTubeBallCircle{ nullptr };
	QTimer *					m_timer{ nullptr };
	int m_nDotjs, m_nStep;
};
#endif // KQWAITDLG_H

class QtWaitWidgetsClass : public QDialog
{
	Q_OBJECT
public:

	enum StandardMsgType
	{
		MsgType_NoNe = 0x00000000,
		MsgType_Close = 0x00000400
	};
	Q_ENUM(StandardMsgType)
	Q_DECLARE_FLAGS(StandardMsgTypes, StandardMsgType)
	Q_FLAG(StandardMsgTypes)
	QtWaitWidgetsClass(QWidget *parent = Q_NULLPTR);
	~QtWaitWidgetsClass();
protected:
	void ClearVecBtn();
protected:
	void changeEvent(QEvent * ev);
	void resizeEvent(QResizeEvent * ev);
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void closeEvent(QCloseEvent *) override;
	void showEvent(QShowEvent *event) override;
public:
	//StandardMsgs standardButtons() const;
	void setStandardButtons(StandardMsgType buttons);
	void ShowMessage(QString strMsg);
	void ShowQProgressBar(bool bshow = false);
	StandardMsgResult WaitForShow(const std::function<StandardMsgResult()>& task, const QString& label = "loading", StandardMsgType msg = MsgType_NoNe);
	StandardMsgResult WaitForExec(const std::function<StandardMsgResult()>& task, const QString& label = "loading", StandardMsgType msg = MsgType_NoNe);
	StandardMsgResult WaitForAfterShow(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask,const QString& label = "loading", StandardMsgType msg = MsgType_NoNe);
	StandardMsgResult WaitForAfterExec(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask,const QString& label = "loading", StandardMsgType msg = MsgType_NoNe);

	static StandardMsgResult WaitForFunction(const std::function<StandardMsgResult()>& task, QString label = "loading", StandardMsgType msg = MsgType_NoNe, QWidget *parent = nullptr);
	static StandardMsgResult WaitForShowFunction(const std::function<StandardMsgResult()>& task, QString label = "loading", StandardMsgType msg = MsgType_NoNe, QWidget *parent = nullptr);
	static StandardMsgResult WaitForMsgShowFunction(const std::function<StandardMsgResult()>& task, QString label = "loading", StandardMsgType msg = MsgType_NoNe, QWidget *parent = nullptr);
	static StandardMsgResult WaitForMsgExecFunction(const std::function<StandardMsgResult()>& task, QString label = "loading", StandardMsgType msg = MsgType_NoNe, QWidget *parent = nullptr);
	static void SetStandardMsgResult(StandardMsgResult Result);

	static StandardMsgResult WaitForMsgAfterShowFunction(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask, QString label = "loading", StandardMsgType msg = MsgType_NoNe, QWidget *parent = nullptr);
	static StandardMsgResult WaitForMsgAfterExecFunction(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask, QString label = "loading", StandardMsgType msg = MsgType_NoNe, QWidget *parent = nullptr);

public slots:
	
	
private slots :
	void slotBtnClose();
	void slotbtnOKClicked();
	void slotbtnCancerClicked();
	void slotTimeOut();
private:
	Ui::QtWaitWidgetsClass	ui;
	KQTaskTread*			m_thread = nullptr;
	QPushButton *			pBtnClose;
	vector<QPushButton*>	pvecBtn2D;
	bool					isDragging = false;
	QPoint					dragPos;
	QTimer *				m_timer{ nullptr };
	bool					m_bBreak = false;
	QProgressBar*			m_progressBar{ nullptr };

public:
	//StandardMsgResult		m_iRetn = MsgResult_OK;
};
//在全局任意地方使用"|"操作符计算自定义的枚举量，需要使用Q_DECLARE_OPERATORS_FOR_FLAGS宏
Q_DECLARE_OPERATORS_FOR_FLAGS(QtWaitWidgetsClass::StandardMsgTypes)