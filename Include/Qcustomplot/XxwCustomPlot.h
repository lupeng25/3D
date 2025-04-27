#ifndef XCUSTOMPLOT_H
#define XCUSTOMPLOT_H

#include "XxwTracer.h"
#include "qcustomplot.h"
#include <QObject>
#include <QList>

class QCP_LIB_DECL XxwCustomPlot:public QCustomPlot
{
    Q_OBJECT

public:
    XxwCustomPlot(QWidget *parent = 0);
	~XxwCustomPlot();
protected:
	virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
public:
	void InitPlot();
	void UpDatePlot();
	void ResetLimit();
	void ClearDrawLines();

	QRectF GetQRectF();

	void ResetALLLimit();
	void RePlotALLTrace();
public:
	void SetSearchRectVisable(bool bVisable = false);
	void SetTraceItemLineVisable(bool bVisable = false);
	void SetTraceItemCircleVisable(bool bVisable = false);
	void DrawAllItemClear();
	void DrawClearAllItem();

	void DrawItemCircle(int index, const QPointF&_ptCenter,double radius);
	void DrawItemLines(int index, const QLineF&_line,QString strName, const QPen& pen = QPen(Qt::blue, 2, Qt::SolidLine));
	void DrawItemPoint(int index, const QVector<QPointF>& Pos, const QVector<QString>& strName);
	void DrawItemPoint(int index, const QVector<QPointF>& Pos, const QVector<QString>& strName, const QVector<QPen>& vecPen);
	void DrawItemPoint(int index, const QVector<double>& x, const QVector<double>& y, const QVector<QString>& strName);
	void DrawItemPoint(int index, const QVector<double>& x, const QVector<double>& y, const QVector<QString>& strName, const QVector<QPen>& vecPen);
public:
    ///
    /// \brief 设置是否显示鼠标追踪器
    /// \param show:是否显示
	void showTracer(bool show);

    /// \brief 是否显示鼠标追踪器
    /// \return
    bool isShowTracer(){	return m_isShowTracer;	};

Q_SIGNALS:
	void sig_XxwTraceRect(XxwTraceRect*traceRect);
	void sig_ChangeTraceRect(XxwTraceRect*traceRect);
	void sig_ChangeDetectItemLine(XxwDetectItemLine*ItemLine);
private:
	int JudgeMousePoint(QPointF& ptMouse,QPointF& ptOffset,RectRange& range);
	int JudgeMouseLinePoint(QPointF& ptMouse, QPointF& ptOffset);
private:
    bool						m_isShowTracer;//是否显示追踪器（鼠标在图中移动，显示对应的值）
    XxwTracer *					m_xTracer;//x轴
    XxwTracer *					m_yTracer;//y轴
    QList<XxwTracer*>			m_dataTracers;//
    XxwTraceLine*				m_lineTracer;//直线

	RectRange					m_CurrentRectRange;
	QLineF						m_CurrentQLineF;
	QPointF						m_MousePressPt;
	QPointF						m_MouseLimitOffset;
	int							m_iSelectedLimitMode;
public:		//指示线
	QMap<int,XxwTraceHintLine*>		m_pMaplineTracers;		//用来追踪的指示箭头
	QMap<int,XxwItemLine*>			m_pMapDrawItemlines;	//用来追踪的线
	QMap<int,XxwDetectItemCircle*>	m_pMapTraceItemCircles;	//所有的列表

public:		//当前用的范围
	QList<XxwTraceRect*>		m_XxwLimitRectLst;	//显示限制区域
	XxwTraceRect*				m_XxwLimitRect		= nullptr;

	QList<XxwDetectItemLine*>	m_XxwTraceItemLineLst;	//所有的列表
	XxwDetectItemLine*			m_XxwTraceItemLine	= nullptr;
	int							m_iSelectedTraceItemMode;
	QPointF						m_MouseLineOffset;

};
#endif // XCUSTOMPLOT_H