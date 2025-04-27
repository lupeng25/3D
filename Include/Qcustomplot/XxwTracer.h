#ifndef MYTRACER_H
#define MYTRACER_H

#include <QObject>
#include "qcustomplot.h"
#include <QVector>

struct RectRange
{
	float fValue;
	float fRangeValue;
	RectRange()
	{
		fValue = 0;
		fRangeValue = 2;
	}
};

///
/// \brief The XxwTracer class:在图表中显示鼠标所在位置的x,y值的追踪显示器
///
class QCP_LIB_DECL XxwTracer : public QObject
{
	Q_OBJECT

public:
	enum TracerType
	{
        XAxisTracer,//依附在x轴上显示x值
        YAxisTracer,//依附在y轴上显示y值
        DataTracer//在图中显示x,y值
	};
    explicit XxwTracer(QCustomPlot *_plot, TracerType _type, QObject *parent = Q_NULLPTR);
    ~XxwTracer();
	void setPen(const QPen &pen);
	void setBrush(const QBrush &brush);
	void setText(const QString &text);
	void setLabelPen(const QPen &pen);
	void updatePosition(double xValue, double yValue);
    void setVisible(bool m_visible);

protected:
    bool m_visible;//是否可见
    TracerType m_type;//类型
    QCustomPlot *m_plot;//图表
    QCPItemTracer *m_tracer;//跟踪的点
    QCPItemText *m_label;//显示的数值
    QCPItemLine *m_arrow;//箭头
};

enum LineType
{
	VerticalLine,//垂直线
	HorizonLine, //水平线
	Both,		//同时显示水平和垂直线
	None,		//无需任何
};

///
/// \brief The XxwCrossLine class:用于显示鼠标移动过程中的鼠标位置的直线
///
class QCP_LIB_DECL XxwTraceLine : public QObject
{
public:
    explicit XxwTraceLine(QCustomPlot *_plot, LineType _type = VerticalLine, QObject *parent = Q_NULLPTR);
    ~XxwTraceLine();
    void initLine();
	void setPen(const QPen &pen);
    void updatePosition(double xValue, double yValue);

    void setVisible(bool vis)
    {
        if(m_lineV)
            m_lineV->setVisible(vis);
        if(m_lineH)
            m_lineH->setVisible(vis);
    }

public:
	float m_fValueAxisX;
	float m_fValueAxisY;
	void UpdatePos();
protected:
    bool m_visible;//是否可见
    LineType m_type;//类型
    QCustomPlot *m_plot;//图表
    QCPItemStraightLine *m_lineV; //垂直线
    QCPItemStraightLine *m_lineH; //水平线
};

class QCP_LIB_DECL TraceHint : public QObject
{
public:
	QCPItemText *pTextLabel;
	QCPItemLine *pArrow;
	QCPItemLine *pLineV; //垂直线
	QCPItemLine *pLineH; //水平线
	QPen		linesPen;
public:
	TraceHint(QObject *parent = Q_NULLPTR);
};

/// \brief The XxwCrossLine class:用于显示鼠标移动过程中的鼠标位置的直线
class QCP_LIB_DECL XxwTraceHintLine : public QObject
{
public:
	explicit XxwTraceHintLine(QCustomPlot *_plot, LineType _type = Both, QObject *parent = Q_NULLPTR);
	~XxwTraceHintLine();
public:

	void initLine();
	void setPen(const QPen &pen);
	//void updatePosition(double xValue, double yValue);

	void setVisible(bool vis);
	void setTraceHint(int iIndex,QPointF sPos, QPointF Textpos,QString strName);
	void setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QPen _pen, QString strName);

	void ClearTraceHint();
public:
	float m_fValueAxisX;
	float m_fValueAxisY;
	void UpdatePos();
	QMap<int, TraceHint*> m_mapTraceHint;
protected:
	bool m_visible;//是否可见
	QPen m_linesPen;
	QCustomPlot *m_plot;//图表

};

/// \brief The XxwCrossLine class:用于显示拟合的线段
class QCP_LIB_DECL XxwDetectItemLine : public QObject
{
public:
	explicit XxwDetectItemLine(QCustomPlot *_plot, LineType type = None,QObject *parent = Q_NULLPTR);
	~XxwDetectItemLine();
public:
	void initLine();
	void setPosLine(QLineF line,bool bExternLine);
	void setPen(const QPen &pen);
	void updatePosition(QLineF line);

	void setVisible(bool vis);
	void setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QString strName);
	void ClearTraceHint();
	void SetQLineF(QLineF & _line);
public:
	QLineF		m_fLine;
	QMap<int, TraceHint*> m_mapTraceHint;
public:
	bool		m_bExternLine;	//是否可见
	bool		m_visible;	//是否可见
	LineType	m_type;//类型
	QCustomPlot *m_plot;	//图表
	QCPItemLine *m_pItemLine;	//直线
};

/// \brief The XxwCrossLine class:用于显示拟合的线段
class QCP_LIB_DECL XxwDetectItemCircle : public QObject
{
public:
	explicit XxwDetectItemCircle(QCustomPlot *_plot,QObject *parent = Q_NULLPTR);
	~XxwDetectItemCircle();
public:
	void initItem();
	void setPen(const QPen &pen);
	void updatePosition(QPointF sPos,double radius);

	void setVisible(bool vis);
	void setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QString strName);
	void ClearTraceHint();
	void SetCircle(QPointF sPos, double radius);
public:
	QPointF		m_CenterPos;
	double		m_dRadius;
	QMap<int, TraceHint*> m_mapTraceHint;
public:
	bool		m_bExternLine;	//是否可见
	bool		m_visible;		//是否可见
	QCustomPlot *m_plot;		//图表
	QCPCurve	*m_pQCPCurve;	//曲线
};

/// \brief The XxwCrossLine class:用于显示拟合的线段
class QCP_LIB_DECL XxwItemLine : public QObject
{
public:
	explicit XxwItemLine(QCustomPlot *_plot,QObject *parent = Q_NULLPTR);
	~XxwItemLine();
public:
	void initLine();
	void setPen(const QPen &pen);
	void updatePosition(double xValue, double yValue);

	void setVisible(bool vis);
	void setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QString strName);
	void ClearTraceHint();
	void SetQLineF(const QLineF & _line);
public:
	float m_fValueAxisX;
	float m_fValueAxisY;
	QMap<int, TraceHint*> m_mapTraceHint;
protected:
	bool		m_visible;	//是否可见
	QCustomPlot *m_plot;	//图表
	QCPItemLine *m_pItemLine;	//直线
};

/// \brief The XxwCrossLine class:用于显示鼠标移动过程中的鼠标位置的直线
class QCP_LIB_DECL XxwTraceRect : public QObject
{
public:
	enum LimitType
	{
		LimitTypeNone		,	//无上下限
		LimitTypeUpAndDown	,	//上下限
		LimitTypeUp,		//上限
		LimitTypeDown,		//下限
		LimitTypeRange,		//区间
	};
	explicit XxwTraceRect(QCustomPlot *_plot, LimitType _type = LimitTypeUpAndDown, QObject *parent = Q_NULLPTR);
	~XxwTraceRect();
	void initTrace();
	void updatePosition(double xValue,float xRange, double yValue, float yRange);
	void InitUpPosition(double xValue, float xRange);
	void InitDnPosition(double yValue, float yRange);
public:
	void RePlotTrace();
	void ResetTrace();
	void setVisible(bool vis);

	void ResetLimit();
	void ChangeLimitMode(LimitType type);
	QColor*	m_QColor =  nullptr;
public:
	RectRange	m_RectUpRange;
	RectRange	m_RectDnRange;
	QCPItemRect *m_lineUp = nullptr; //下限
	QCPItemRect *m_lineDn = nullptr; //上限
	LimitType	m_type;//类型
protected:
	bool		m_visible;//是否可见
	QCustomPlot *m_plot = nullptr;//图表

};

#endif // MYTRACER_H