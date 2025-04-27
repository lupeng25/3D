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
/// \brief The XxwTracer class:��ͼ������ʾ�������λ�õ�x,yֵ��׷����ʾ��
///
class QCP_LIB_DECL XxwTracer : public QObject
{
	Q_OBJECT

public:
	enum TracerType
	{
        XAxisTracer,//������x������ʾxֵ
        YAxisTracer,//������y������ʾyֵ
        DataTracer//��ͼ����ʾx,yֵ
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
    bool m_visible;//�Ƿ�ɼ�
    TracerType m_type;//����
    QCustomPlot *m_plot;//ͼ��
    QCPItemTracer *m_tracer;//���ٵĵ�
    QCPItemText *m_label;//��ʾ����ֵ
    QCPItemLine *m_arrow;//��ͷ
};

enum LineType
{
	VerticalLine,//��ֱ��
	HorizonLine, //ˮƽ��
	Both,		//ͬʱ��ʾˮƽ�ʹ�ֱ��
	None,		//�����κ�
};

///
/// \brief The XxwCrossLine class:������ʾ����ƶ������е����λ�õ�ֱ��
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
    bool m_visible;//�Ƿ�ɼ�
    LineType m_type;//����
    QCustomPlot *m_plot;//ͼ��
    QCPItemStraightLine *m_lineV; //��ֱ��
    QCPItemStraightLine *m_lineH; //ˮƽ��
};

class QCP_LIB_DECL TraceHint : public QObject
{
public:
	QCPItemText *pTextLabel;
	QCPItemLine *pArrow;
	QCPItemLine *pLineV; //��ֱ��
	QCPItemLine *pLineH; //ˮƽ��
	QPen		linesPen;
public:
	TraceHint(QObject *parent = Q_NULLPTR);
};

/// \brief The XxwCrossLine class:������ʾ����ƶ������е����λ�õ�ֱ��
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
	bool m_visible;//�Ƿ�ɼ�
	QPen m_linesPen;
	QCustomPlot *m_plot;//ͼ��

};

/// \brief The XxwCrossLine class:������ʾ��ϵ��߶�
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
	bool		m_bExternLine;	//�Ƿ�ɼ�
	bool		m_visible;	//�Ƿ�ɼ�
	LineType	m_type;//����
	QCustomPlot *m_plot;	//ͼ��
	QCPItemLine *m_pItemLine;	//ֱ��
};

/// \brief The XxwCrossLine class:������ʾ��ϵ��߶�
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
	bool		m_bExternLine;	//�Ƿ�ɼ�
	bool		m_visible;		//�Ƿ�ɼ�
	QCustomPlot *m_plot;		//ͼ��
	QCPCurve	*m_pQCPCurve;	//����
};

/// \brief The XxwCrossLine class:������ʾ��ϵ��߶�
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
	bool		m_visible;	//�Ƿ�ɼ�
	QCustomPlot *m_plot;	//ͼ��
	QCPItemLine *m_pItemLine;	//ֱ��
};

/// \brief The XxwCrossLine class:������ʾ����ƶ������е����λ�õ�ֱ��
class QCP_LIB_DECL XxwTraceRect : public QObject
{
public:
	enum LimitType
	{
		LimitTypeNone		,	//��������
		LimitTypeUpAndDown	,	//������
		LimitTypeUp,		//����
		LimitTypeDown,		//����
		LimitTypeRange,		//����
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
	QCPItemRect *m_lineUp = nullptr; //����
	QCPItemRect *m_lineDn = nullptr; //����
	LimitType	m_type;//����
protected:
	bool		m_visible;//�Ƿ�ɼ�
	QCustomPlot *m_plot = nullptr;//ͼ��

};

#endif // MYTRACER_H