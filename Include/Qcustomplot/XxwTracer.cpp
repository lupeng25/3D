#include "XxwTracer.h"

XxwTracer::XxwTracer(QCustomPlot *_plot, TracerType _type, QObject *parent)
    : QObject(parent),
      m_plot(_plot),
      m_type(_type)
{
    m_visible = true;
    m_tracer = Q_NULLPTR;// 跟踪的点
    m_label = Q_NULLPTR;// 显示的数值
    m_arrow = Q_NULLPTR;// 箭头
    if (m_plot)
    {
        QColor clrDefault(Qt::red);
        QBrush brushDefault(Qt::NoBrush);
        QPen penDefault(clrDefault);
        //        penDefault.setBrush(brushDefault);
        penDefault.setWidthF(0.5);

        m_tracer = new QCPItemTracer(m_plot);
        m_tracer->setStyle(QCPItemTracer::tsCircle);
        m_tracer->setPen(penDefault);
        m_tracer->setBrush(brushDefault);

        m_label = new QCPItemText(m_plot);
        m_label->setLayer("overlay");
        m_label->setClipToAxisRect(false);
        m_label->setPadding(QMargins(0, 0, 0, 0));
        m_label->setBrush(brushDefault);
        m_label->setPen(penDefault);
        m_label->position->setParentAnchor(m_tracer->position);
		//	m_label->setFont(QFont("宋体", 8));
        m_label->setFont(QFont("Arial", 8));
        m_label->setColor(clrDefault);
        m_label->setText("");

        m_arrow = new QCPItemLine(m_plot);
        QPen  arrowPen(clrDefault, 1);
        m_arrow->setPen(penDefault);
        m_arrow->setLayer("overlay");
        m_arrow->setClipToAxisRect(false);
        m_arrow->setHead(QCPLineEnding::esSpikeArrow);//设置头部为箭头形状

        switch (m_type)
        {
        case XAxisTracer:
        {
            m_tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
            m_tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
            m_tracer->setSize(7);
            m_label->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);

            m_arrow->end->setParentAnchor(m_tracer->position);
            m_arrow->start->setParentAnchor(m_arrow->end);
            m_arrow->start->setCoords(0, 20);//偏移量
            break;
        }
        case YAxisTracer:
        {
            m_tracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
            m_tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);
            m_tracer->setSize(7);
            m_label->setPositionAlignment(Qt::AlignRight | Qt::AlignHCenter);

            m_arrow->end->setParentAnchor(m_tracer->position);
            m_arrow->start->setParentAnchor(m_label->position);
            m_arrow->start->setCoords(-20, 0);//偏移量
            break;
        }
        case DataTracer:
        {
            m_tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
            m_tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);
            m_tracer->setSize(5);

            m_label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            m_arrow->end->setParentAnchor(m_tracer->position);
            m_arrow->start->setParentAnchor(m_arrow->end);
            m_arrow->start->setCoords(20, 0);
            break;
        }
        default:
            break;
        }
        setVisible(false);
    }
}

XxwTracer::~XxwTracer()
{
    if(m_plot)
    {
        if (m_tracer)
            m_plot->removeItem(m_tracer);
        if (m_label)
            m_plot->removeItem(m_label);
        if (m_arrow)
            m_plot->removeItem(m_arrow);
    }
}

void XxwTracer::setPen(const QPen &pen)
{
    if(m_tracer)
        m_tracer->setPen(pen);
    if(m_arrow)
        m_arrow->setPen(pen);
}

void XxwTracer::setBrush(const QBrush &brush)
{
    if(m_tracer)
        m_tracer->setBrush(brush);
}

void XxwTracer::setLabelPen(const QPen &pen)
{
    if(m_label)
    {
        m_label->setPen(pen);
        m_label->setBrush(Qt::NoBrush);
        m_label->setColor(pen.color());
    }
}

void XxwTracer::setText(const QString &text)
{
    if(m_label)
        m_label->setText(text);
}

void XxwTracer::setVisible(bool vis)
{
    m_visible = vis;
    if(m_tracer)
        m_tracer->setVisible(m_visible);
    if(m_label)
        m_label->setVisible(m_visible);
    if(m_arrow)
        m_arrow->setVisible(m_visible);
}

void XxwTracer::updatePosition(double xValue, double yValue)
{
    if (!m_visible)
    {
        setVisible(true);
        m_visible = true;
    }

    if (yValue > m_plot->yAxis->range().upper)	yValue = m_plot->yAxis->range().upper;

    switch (m_type)
    {
    case XAxisTracer:
    {
        m_tracer->position->setCoords(xValue, 1);
        m_label->position->setCoords(0, 15);
        m_arrow->start->setCoords(0, 15);
        m_arrow->end->setCoords(0, 0);
        setText(QString::number(xValue,'f',3));
        break;
    }
    case YAxisTracer:
    {
        m_tracer->position->setCoords(0, yValue);
        m_label->position->setCoords(-20, 0);
		//	m_arrow->start->setCoords(20, 0);
		//	m_arrow->end->setCoords(0, 0);
        setText(QString::number(yValue, 'f', 3));
        break;
    }
    case DataTracer:
    {
        m_tracer->position->setCoords(xValue, yValue);
        m_label->position->setCoords(20, 0);
        setText(QString("x:%1,y:%2").arg(xValue).arg(yValue));
        break;
    }
    default:
        break;
    }
}

XxwTraceLine::XxwTraceLine(QCustomPlot *_plot, LineType _type, QObject *parent)
    : QObject(parent),	m_type(_type),	m_plot(_plot)
{
    m_lineV = Q_NULLPTR;
    m_lineH = Q_NULLPTR;
    initLine();
}

XxwTraceLine::~XxwTraceLine()
{
    if(m_plot)
    {
        if (m_lineV)	m_plot->removeItem(m_lineV);
        if (m_lineH)	m_plot->removeItem(m_lineH);
    }
}

void XxwTraceLine::initLine()
{
    if(m_plot)
    {
        QPen linesPen(Qt::red, 1, Qt::DashLine);
        if(VerticalLine == m_type || Both == m_type)
        {
            m_lineV = new QCPItemStraightLine(m_plot);//垂直线
            m_lineV->setLayer("overlay");
            m_lineV->setPen(linesPen);
            m_lineV->setClipToAxisRect(true);
            m_lineV->point1->setCoords(0, 0);
            m_lineV->point2->setCoords(0, 0);
        }
        if(HorizonLine == m_type || Both == m_type)
        {
            m_lineH = new QCPItemStraightLine(m_plot);//水平线
            m_lineH->setLayer("overlay");
            m_lineH->setPen(linesPen);
            m_lineH->setClipToAxisRect(true);
            m_lineH->point1->setCoords(0, 0);
            m_lineH->point2->setCoords(0, 0);
        }
    }
}

void XxwTraceLine::setPen(const QPen & pen)
{
	if (m_lineV)	m_lineV->setPen(pen);
	if (m_lineH)	m_lineH->setPen(pen);
}

void XxwTraceLine::updatePosition(double xValue, double yValue)
{
    if(VerticalLine == m_type || Both == m_type)
    {
        if(m_lineV)
        {
            m_lineV->point1->setCoords(xValue, m_plot->yAxis->range().lower);
            m_lineV->point2->setCoords(xValue, m_plot->yAxis->range().upper);
        }
    }
    if(HorizonLine == m_type || Both == m_type)
    {
        if(m_lineH)
        {
            m_lineH->point1->setCoords(m_plot->xAxis->range().lower, yValue);
            m_lineH->point2->setCoords(m_plot->xAxis->range().upper, yValue);
        }
    }
}

void XxwTraceLine::UpdatePos()
{
	if (VerticalLine == m_type || Both == m_type)
	{
		if (m_lineV)
		{
			m_lineV->point1->setCoords(m_fValueAxisX, m_plot->yAxis->range().lower);
			m_lineV->point2->setCoords(m_fValueAxisX, m_plot->yAxis->range().upper);
		}
	}
	if (HorizonLine == m_type || Both == m_type)
	{
		if (m_lineH)
		{
			m_lineH->point1->setCoords(m_plot->xAxis->range().lower, m_fValueAxisY);
			m_lineH->point2->setCoords(m_plot->xAxis->range().upper, m_fValueAxisY);
		}
	}
}

TraceHint::TraceHint(QObject * parent)
	: QObject(parent)
{
	pTextLabel = Q_NULLPTR;
	pArrow = Q_NULLPTR;
	pLineV = Q_NULLPTR;
	pLineH = Q_NULLPTR;
}

XxwTraceRect::XxwTraceRect(QCustomPlot * _plot, LimitType _type, QObject * parent)
	: QObject(parent),
	m_type(_type),
	m_plot(_plot)
{
	m_lineUp = Q_NULLPTR;
	m_lineDn = Q_NULLPTR;
	m_QColor = new QColor(rand() % 256, rand() % 256, rand() % 256);
	initTrace();
}

XxwTraceRect::~XxwTraceRect()
{
	if (m_plot)
	{
		if (m_lineUp)	m_plot->removeItem(m_lineUp);
		//delete m_lineUp; m_lineUp = nullptr;
		if (m_lineDn)	m_plot->removeItem(m_lineDn);
		//delete m_lineDn; m_lineDn = nullptr;
	}
	if (m_QColor != nullptr) delete m_QColor; m_QColor = nullptr;
}

void XxwTraceRect::initTrace()
{
	if (m_plot)
	{
		QPen linesPen(*m_QColor, 2, Qt::SolidLine);
		m_lineUp = new QCPItemRect(m_plot);//垂直线
		m_lineUp->setLayer("overlay");
		m_lineUp->setPen(linesPen);
		m_lineUp->setClipToAxisRect(true);
		m_lineUp->topLeft->setCoords(0, 0);
		m_lineUp->bottomRight->setCoords(0, 0);
		m_lineUp->setVisible(false);
		float fDurTime = m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;
		m_RectUpRange.fRangeValue = fDurTime / 20.0;
		m_RectUpRange.fValue = m_plot->xAxis->range().upper - m_RectUpRange.fRangeValue;

		m_lineDn = new QCPItemRect(m_plot);//水平线
		m_lineDn->setLayer("overlay");
		m_lineDn->setPen(linesPen);
		m_lineDn->setClipToAxisRect(true);
		fDurTime = m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;
		m_RectDnRange.fRangeValue = fDurTime / 20.0;
		m_RectDnRange.fValue = m_plot->xAxis->range().lower;
		m_lineDn->topLeft->setCoords(0, 0);
		m_lineDn->bottomRight->setCoords(0, 0);
		m_lineDn->setVisible(false);

		if (LimitTypeUpAndDown == m_type || LimitTypeUp == m_type || LimitTypeRange == m_type)
		{
			m_lineUp->setVisible(true);
		}
		if (LimitTypeUpAndDown == m_type || LimitTypeDown == m_type)
		{
			m_lineDn->setVisible(true);
		}

	}
}

void XxwTraceRect::updatePosition(double xValue, float xRange, double yValue, float yRange)
{
	if (LimitTypeUpAndDown == m_type || LimitTypeUp == m_type || LimitTypeRange == m_type)
	{
		if (m_lineUp)
		{
			m_RectUpRange.fValue		= xValue;
			m_RectUpRange.fRangeValue	= xRange;
			m_lineUp->topLeft->setCoords(m_RectUpRange.fValue, m_plot->yAxis->range().lower);
			m_lineUp->bottomRight->setCoords(m_RectUpRange.fValue + m_RectUpRange.fRangeValue, m_plot->yAxis->range().upper);
		}
	}
	if (LimitTypeUpAndDown == m_type || LimitTypeDown == m_type)
	{
		if (m_lineDn)
		{
			m_RectDnRange.fValue		= yValue;
			m_RectDnRange.fRangeValue	= yRange;
			m_lineDn->topLeft->setCoords(m_RectDnRange.fValue, m_plot->yAxis->range().lower);
			m_lineDn->bottomRight->setCoords(m_RectDnRange.fValue + m_RectDnRange.fRangeValue, m_plot->yAxis->range().upper);
		}
	}
}

void XxwTraceRect::InitUpPosition(double xValue, float xRange)
{
	if (m_lineUp)
	{
		float fDurTime				= m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;
		m_RectUpRange.fRangeValue	= fDurTime / 20.0;
		m_RectUpRange.fValue		= xValue;
		m_RectUpRange.fRangeValue	= std::max(xRange, m_RectUpRange.fRangeValue);
		m_lineUp->topLeft->setCoords(m_RectUpRange.fValue, m_plot->yAxis->range().lower);
		m_lineUp->bottomRight->setCoords(m_RectUpRange.fValue + m_RectUpRange.fRangeValue, m_plot->yAxis->range().upper);
	}
}

void XxwTraceRect::InitDnPosition(double yValue, float yRange)
{
	if (m_lineDn)
	{
		float fDurTime				= m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;
		m_RectDnRange.fRangeValue	= fDurTime / 20.0;
		m_RectDnRange.fValue		= yValue;
		m_RectDnRange.fRangeValue	= std::max(yRange, m_RectDnRange.fRangeValue);
		m_lineDn->topLeft->setCoords(m_RectDnRange.fValue, m_plot->yAxis->range().lower);
		m_lineDn->bottomRight->setCoords(m_RectDnRange.fValue + m_RectDnRange.fRangeValue, m_plot->yAxis->range().upper);
	}
}

void XxwTraceRect::ResetTrace()
{
	if (m_lineUp)
	{
		float fDurTime				= m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;
		m_RectUpRange.fRangeValue	= fDurTime / 20.0;
		m_RectUpRange.fValue		= m_plot->xAxis->range().upper - m_RectUpRange.fRangeValue;
		m_lineUp->topLeft->setCoords(m_RectUpRange.fValue, m_plot->yAxis->range().lower);
		m_lineUp->bottomRight->setCoords(m_RectUpRange.fValue + m_RectUpRange.fRangeValue, m_plot->yAxis->range().upper);
	}
	if (m_lineDn)
	{
		float fDurTime				= m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;
		m_RectDnRange.fValue		= m_plot->xAxis->range().lower;
		m_RectDnRange.fRangeValue	= fDurTime / 20.0;
		m_lineDn->topLeft->setCoords(m_RectDnRange.fValue, m_plot->yAxis->range().lower);
		m_lineDn->bottomRight->setCoords(m_RectDnRange.fValue + m_RectDnRange.fRangeValue, m_plot->yAxis->range().upper);

	}
}

void XxwTraceRect::setVisible(bool vis)
{
	switch (m_type)
	{
	case XxwTraceRect::LimitTypeNone:
	{
		if (m_lineUp)	m_lineUp->setVisible(false);
		if (m_lineDn)	m_lineDn->setVisible(false);
	}	break;
	case XxwTraceRect::LimitTypeUpAndDown:
	{
		if (m_lineUp)	m_lineUp->setVisible(vis);
		if (m_lineDn)	m_lineDn->setVisible(vis);
	}	break;
	case XxwTraceRect::LimitTypeUp:
	{
		if (m_lineUp)	m_lineUp->setVisible(vis);
		if (m_lineDn)	m_lineDn->setVisible(false);
	}	break;
	case XxwTraceRect::LimitTypeDown:
	{
		if (m_lineUp)	m_lineUp->setVisible(false);
		if (m_lineDn)	m_lineDn->setVisible(vis);
	}	break;
	case XxwTraceRect::LimitTypeRange:
	{
		if (m_lineUp)	m_lineUp->setVisible(vis);
		if (m_lineDn)	m_lineDn->setVisible(false);
	}	break;
	default:
		break;
	}
	m_visible = vis;
}

void XxwTraceRect::ResetLimit()
{
	if (m_lineUp)
	{
		float fDurTime				= m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;

		m_RectUpRange.fRangeValue = fDurTime / 20.0;
		m_RectUpRange.fValue = m_plot->xAxis->range().upper - m_RectUpRange.fRangeValue;
		m_lineUp->topLeft->setCoords(m_RectUpRange.fValue, m_plot->yAxis->range().lower);
		m_lineUp->bottomRight->setCoords(m_RectUpRange.fValue + m_RectUpRange.fRangeValue, m_plot->yAxis->range().upper);
	}
	if (m_lineDn)
	{
		float fDurTime				= m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;

		m_RectDnRange.fValue		= m_plot->xAxis->range().lower;
		m_RectDnRange.fRangeValue	= fDurTime / 20.0;
		m_lineDn->topLeft->setCoords(m_RectDnRange.fValue, m_plot->yAxis->range().lower);
		m_lineDn->bottomRight->setCoords(m_RectDnRange.fValue + m_RectDnRange.fRangeValue, m_plot->yAxis->range().upper);
	}
}

void XxwTraceRect::ChangeLimitMode(LimitType type)
{
	switch (type)
	{
	case XxwTraceRect::LimitTypeNone:
	{
		if (m_lineUp)	m_lineUp->setVisible(false);
		if (m_lineDn)	m_lineDn->setVisible(false);
	
	}	break;
	case XxwTraceRect::LimitTypeUpAndDown:
	{
		if (m_lineUp)	m_lineUp->setVisible(m_visible);
		if (m_lineDn)	m_lineDn->setVisible(m_visible);
	}	break;
	case XxwTraceRect::LimitTypeUp:
	{
		if (m_lineUp)	m_lineUp->setVisible(m_visible);
		if (m_lineDn)	m_lineDn->setVisible(false);
	}	break;
	case XxwTraceRect::LimitTypeDown:
	{
		if (m_lineUp)	m_lineUp->setVisible(false);
		if (m_lineDn)	m_lineDn->setVisible(m_visible);
	}	break;
	case XxwTraceRect::LimitTypeRange:
	{
		if (m_lineUp)	m_lineUp->setVisible(m_visible);
		if (m_lineDn)	m_lineDn->setVisible(false);
	}	break;
	default:
		break;
	}
	m_type = type;
}

void XxwTraceRect::RePlotTrace()
{
	if (m_lineUp)
	{
		m_lineUp->topLeft->setCoords(m_RectUpRange.fValue, m_plot->yAxis->range().lower);
		m_lineUp->bottomRight->setCoords(m_RectUpRange.fValue + m_RectUpRange.fRangeValue, m_plot->yAxis->range().upper);
	}
	if (m_lineDn)
	{
		m_lineDn->topLeft->setCoords(m_RectDnRange.fValue, m_plot->yAxis->range().lower);
		m_lineDn->bottomRight->setCoords(m_RectDnRange.fValue + m_RectDnRange.fRangeValue, m_plot->yAxis->range().upper);
	}
}

XxwTraceHintLine::XxwTraceHintLine(QCustomPlot * _plot, LineType _type, QObject * parent)
	: QObject(parent), m_plot(_plot)
{
	ClearTraceHint();
	initLine();
}

XxwTraceHintLine::~XxwTraceHintLine()
{
	if (m_plot)
	{
		ClearTraceHint();
	}
}

void XxwTraceHintLine::initLine()
{
	if (m_plot)
	{
		m_linesPen = QPen(Qt::blue, 2, Qt::SolidLine);

	}
}

void XxwTraceHintLine::setPen(const QPen & pen)
{
	m_linesPen = pen;
}

void XxwTraceHintLine::setVisible(bool vis)
{
	for (auto& iter		:		m_mapTraceHint)
	{
		if (iter->pArrow)		iter->pArrow->setVisible(vis);
		if (iter->pTextLabel)	iter->pTextLabel->setVisible(vis);
		if (iter->pLineV)		iter->pLineV->setVisible(vis);
		if (iter->pLineH)		iter->pLineH->setVisible(vis);
	}
}

void XxwTraceHintLine::setTraceHint(int iIndex,QPointF sPos,QPointF Textpos,QString strName)
{
	setTraceHint(iIndex, sPos, Textpos, QPen(Qt::blue, 2, Qt::SolidLine),strName);
}

void XxwTraceHintLine::setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QPen _pen, QString strName)
{
	float fDurTime = m_plot->yAxis->range().upper - m_plot->yAxis->range().lower;
	fDurTime = fDurTime * 0.1;

	m_fValueAxisX = sPos.x();
	m_fValueAxisY = sPos.y();

	if (m_mapTraceHint.count(iIndex) <= 0)
	{
		TraceHint* _pTraceHint = new TraceHint(this);
		m_mapTraceHint.insert(iIndex, _pTraceHint);
	}
	if (m_mapTraceHint[iIndex]->pTextLabel == nullptr)	m_mapTraceHint[iIndex]->pTextLabel = new QCPItemText(m_plot);
	if ((Textpos.y() + fDurTime) > m_plot->yAxis->range().upper)
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() - fDurTime); // place position at center/top of axis rect
	else
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() + fDurTime); // place position at center/top of axis rect

	if (m_mapTraceHint[iIndex]->pLineV == nullptr)				m_mapTraceHint[iIndex]->pLineV = new QCPItemLine(m_plot);
	m_mapTraceHint[iIndex]->pLineV->setPen(_pen);

	m_mapTraceHint[iIndex]->pLineV->start->setCoords(sPos.x(), sPos.y() - fDurTime * 0.5);
	m_mapTraceHint[iIndex]->pLineV->end->setCoords(sPos.x(), sPos.y() + fDurTime * 0.5); // point to (4, 1.6) in x-y-plot coordinates

	fDurTime = m_plot->xAxis->range().upper - m_plot->xAxis->range().lower;
	fDurTime = fDurTime * 0.05;
	if (m_mapTraceHint[iIndex]->pLineH == nullptr)	m_mapTraceHint[iIndex]->pLineH = new QCPItemLine(m_plot);
	m_mapTraceHint[iIndex]->pLineH->setPen(_pen);

	m_mapTraceHint[iIndex]->pLineH->start->setCoords(sPos.x() - fDurTime, sPos.y());
	m_mapTraceHint[iIndex]->pLineH->end->setCoords(sPos.x() + fDurTime, sPos.y()); // point to (4, 1.6) in x-y-plot coordinates

	m_mapTraceHint[iIndex]->pTextLabel->setText(strName);
	m_mapTraceHint[iIndex]->pTextLabel->setFont(QFont(m_plot->font().family(), 9)); // make font a bit larger
	m_mapTraceHint[iIndex]->pTextLabel->setPen(QPen(Qt::black)); // show black border around text

	if (m_mapTraceHint[iIndex]->pArrow == nullptr)	m_mapTraceHint[iIndex]->pArrow = new QCPItemLine(m_plot);
	m_mapTraceHint[iIndex]->pArrow->start->setParentAnchor(m_mapTraceHint[iIndex]->pTextLabel->bottom);
	m_mapTraceHint[iIndex]->pArrow->end->setCoords(sPos.x(), sPos.y()); // point to (4, 1.6) in x-y-plot coordinates
	m_mapTraceHint[iIndex]->pArrow->setHead(QCPLineEnding::esSpikeArrow);

	if (!strName.isEmpty())
	{
		m_mapTraceHint[iIndex]->pArrow->setVisible(true);
		m_mapTraceHint[iIndex]->pTextLabel->setVisible(true);
		m_mapTraceHint[iIndex]->pLineV->setVisible(true);
		m_mapTraceHint[iIndex]->pLineH->setVisible(true);
	}
	else
	{
		m_mapTraceHint[iIndex]->pArrow->setVisible(false);
		m_mapTraceHint[iIndex]->pTextLabel->setVisible(false);
		m_mapTraceHint[iIndex]->pLineV->setVisible(false);
		m_mapTraceHint[iIndex]->pLineH->setVisible(false);
	}
}

void XxwTraceHintLine::ClearTraceHint()
{
	for (auto& iter : m_mapTraceHint)
	{
		if (iter->pLineV)		m_plot->removeItem(iter->pLineV);
		if (iter->pLineH)		m_plot->removeItem(iter->pLineH);
		if (iter->pArrow)		m_plot->removeItem(iter->pArrow);
		if (iter->pTextLabel)	m_plot->removeItem(iter->pTextLabel);
	}
	m_mapTraceHint.clear();
}

void XxwTraceHintLine::UpdatePos()
{
	for (auto& iter : m_mapTraceHint)
	{
		float fDurTime = m_plot->yAxis->range().upper - m_plot->yAxis->range().lower;
		fDurTime = fDurTime * 0.1;

		QPointF _ptCenter =	iter->pArrow->end->coords();

		iter->pLineV->start->setCoords(_ptCenter.x(),	_ptCenter.y() - fDurTime * 0.5);
		iter->pLineV->end->setCoords(_ptCenter.x(),		_ptCenter.y() + fDurTime * 0.5);

		fDurTime = m_plot->xAxis->range().upper		-	m_plot->xAxis->range().lower;
		fDurTime = fDurTime * 0.1;
		iter->pLineH->start->setCoords(_ptCenter.x()	- fDurTime * 0.5, _ptCenter.y());
		iter->pLineH->end->setCoords(_ptCenter.x()		+ fDurTime * 0.5, _ptCenter.y());
	}

}

XxwItemLine::XxwItemLine(QCustomPlot * _plot, QObject * parent)
	: QObject(parent), m_plot(_plot)
{
	m_pItemLine = Q_NULLPTR;
	ClearTraceHint();
}

XxwItemLine::~XxwItemLine()
{
	if (m_plot)
	{
		if (m_pItemLine)		m_plot->removeItem(m_pItemLine);
	}
	ClearTraceHint();
}

void XxwItemLine::initLine()
{
	if (m_plot)
	{
		QPen linesPen(Qt::blue, 2, Qt::SolidLine);
		m_pItemLine = new QCPItemLine(m_plot);//垂直线
		m_pItemLine->setLayer("overlay");
		m_pItemLine->setPen(linesPen);
		m_pItemLine->setClipToAxisRect(true);
	}
}

void XxwItemLine::setPen(const QPen & pen)
{
	if (m_pItemLine)	m_pItemLine->setPen(pen);
}

void XxwItemLine::updatePosition(double xValue, double yValue)
{

}

void XxwItemLine::setVisible(bool vis)
{
	if (m_pItemLine)			m_pItemLine->setVisible(vis);
	for (auto& iter :			m_mapTraceHint)
	{
		if (iter->pArrow)		iter->pArrow->setVisible(vis);
		if (iter->pTextLabel)	iter->pTextLabel->setVisible(vis);
	}
}

void XxwItemLine::setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QString strName)
{
	if (m_mapTraceHint.count(iIndex) <= 0)
	{
		TraceHint* _pTraceHint = new TraceHint(this);
		m_mapTraceHint.insert(iIndex, _pTraceHint);
	}
	float fDurTime = m_plot->yAxis->range().upper - m_plot->yAxis->range().lower;
	fDurTime = fDurTime * 0.1;

	if (m_mapTraceHint[iIndex]->pTextLabel == nullptr)	m_mapTraceHint[iIndex]->pTextLabel = new QCPItemText(m_plot);
	if ((Textpos.y() + fDurTime) > m_plot->yAxis->range().upper)
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() - fDurTime); // place position at center/top of axis rect
	else
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() + fDurTime); // place position at center/top of axis rect

	m_mapTraceHint[iIndex]->pTextLabel->setText(strName);
	m_mapTraceHint[iIndex]->pTextLabel->setFont(QFont(m_plot->font().family(), 9)); // make font a bit larger
	m_mapTraceHint[iIndex]->pTextLabel->setPen(QPen(Qt::black)); // show black border around text

	if (m_mapTraceHint[iIndex]->pArrow == nullptr)	m_mapTraceHint[iIndex]->pArrow = new QCPItemLine(m_plot);
	m_mapTraceHint[iIndex]->pArrow->start->setParentAnchor(m_mapTraceHint[iIndex]->pTextLabel->bottom);
	m_mapTraceHint[iIndex]->pArrow->end->setCoords(sPos.x(), sPos.y()); // point to (4, 1.6) in x-y-plot coordinates
	m_mapTraceHint[iIndex]->pArrow->setHead(QCPLineEnding::esSpikeArrow);
}

void XxwItemLine::ClearTraceHint()
{
	for (auto& iter : m_mapTraceHint)
	{
		if (iter->pArrow)		m_plot->removeItem(iter->pArrow);
		if (iter->pTextLabel)	m_plot->removeItem(iter->pTextLabel);
	}
	m_mapTraceHint.clear();
}

void XxwItemLine::SetQLineF(const QLineF & _line)
{
	if (m_pItemLine == nullptr)
	{
		initLine();
	}
	if (m_pItemLine != nullptr)
	{
		m_pItemLine->start->setCoords(_line.p1());
		m_pItemLine->end->setCoords(_line.p2());
	}
}

XxwDetectItemLine::XxwDetectItemLine(QCustomPlot * _plot, LineType type, QObject * parent)
	: QObject(parent), m_plot(_plot), m_type(type)
{
	m_pItemLine = Q_NULLPTR;
	ClearTraceHint();
	initLine();
	m_bExternLine = false;
}

XxwDetectItemLine::~XxwDetectItemLine()
{
	if (m_plot)
	{
		if (m_pItemLine)		m_plot->removeItem(m_pItemLine);
	}
	ClearTraceHint();
}

void XxwDetectItemLine::initLine()
{
	if (m_plot)
	{
		QPen linesPen(Qt::blue, 2, Qt::SolidLine);
		m_pItemLine = new QCPItemLine(m_plot);//垂直线
		m_pItemLine->setLayer("overlay");
		m_pItemLine->setPen(linesPen);
		m_pItemLine->setClipToAxisRect(true);
	}
}

void XxwDetectItemLine::setPosLine(QLineF line, bool bExternLine)
{
	m_bExternLine = bExternLine;
	m_fLine = line;
	if (m_pItemLine)
	{
		m_pItemLine->start->setCoords(line.p1());
		m_pItemLine->end->setCoords(line.p2());
	}
}

void XxwDetectItemLine::setPen(const QPen & pen)
{
	if (m_pItemLine)	m_pItemLine->setPen(pen);
}

void XxwDetectItemLine::updatePosition(QLineF line)
{
	if (m_pItemLine == nullptr)
	{
		initLine();
	}
	float _fYHigh	= m_plot->yAxis->range().upper;
	float _fYlower	= m_plot->yAxis->range().lower;
	float _fXHigh	= m_plot->xAxis->range().upper;
	float _fXlower	= m_plot->xAxis->range().lower;
	QPointF	_IntersectPos1, _IntersectPos2;
	bool _bNeedUpDate = true;
	switch (m_type)
	{
	case VerticalLine:
	{
		if (m_pItemLine != nullptr)
		{
			QLineF _upLine;

			_upLine.setP1(QPointF(_fXlower, _fYlower));
			_upLine.setP2(QPointF(_fXHigh,	_fYlower));
			switch (_upLine.intersect(line,&_IntersectPos1))
			{
			case QLineF::BoundedIntersection:
			case QLineF::UnboundedIntersection:
				_bNeedUpDate &= false;
				break;
			default:
				_bNeedUpDate &= true;
				break;
			}

			QLineF _dnLine;
			_dnLine.setP1(QPointF(_fXlower, _fYHigh));
			_dnLine.setP2(QPointF(_fXHigh,	_fYHigh));
			switch (_dnLine.intersect(line, &_IntersectPos2))
			{
			case QLineF::UnboundedIntersection:
			case QLineF::BoundedIntersection:
				_bNeedUpDate &= false;
				break;
			default:
				_bNeedUpDate &= true;
				break;
			}
		}
	}	break;
	case HorizonLine:
	{
		if (m_pItemLine != nullptr)
		{
			QLineF _upLine;

			_upLine.setP1(QPointF(_fXlower, _fYlower));
			_upLine.setP2(QPointF(_fXlower, _fYHigh));
			switch (_upLine.intersect(line, &_IntersectPos1))
			{
			case QLineF::BoundedIntersection:
			case QLineF::UnboundedIntersection:
				_bNeedUpDate &= false;
				break;
			default:
				_bNeedUpDate &= true;
				break;
			}

			QLineF _dnLine;
			_dnLine.setP1(QPointF(_fXHigh, _fYlower));
			_dnLine.setP2(QPointF(_fXHigh, _fYHigh));
			switch (_dnLine.intersect(line, &_IntersectPos2))
			{
			case QLineF::UnboundedIntersection:
			case QLineF::BoundedIntersection:
				_bNeedUpDate &= false;
				break;
			default:
				_bNeedUpDate &= true;
				break;
			}
		}
	}	break;
	case None:
	{

	}	break;
	default:
	{

	}	break;
	}
	if (!_bNeedUpDate)
	{
		if (m_pItemLine != nullptr)
		{
			m_pItemLine->start->setCoords(_IntersectPos1);
			m_pItemLine->end->setCoords(_IntersectPos2);
		}
	}

}

void XxwDetectItemLine::setVisible(bool vis)
{
	if (m_pItemLine)			m_pItemLine->setVisible(vis);
	for (auto& iter		:		m_mapTraceHint)
	{
		if (iter->pArrow)		iter->pArrow->setVisible(vis);
		if (iter->pTextLabel)	iter->pTextLabel->setVisible(vis);
	}
	m_visible = vis;
}

void XxwDetectItemLine::setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QString strName)
{
	if (m_mapTraceHint.count(iIndex) <= 0)
	{
		TraceHint* _pTraceHint = new TraceHint(this);
		m_mapTraceHint.insert(iIndex, _pTraceHint);
	}

	float fDurTime = m_plot->yAxis->range().upper - m_plot->yAxis->range().lower;
	fDurTime = fDurTime * 0.1;

	if (m_mapTraceHint[iIndex]->pTextLabel == nullptr)	m_mapTraceHint[iIndex]->pTextLabel = new QCPItemText(m_plot);
	if ((Textpos.y() + fDurTime) > m_plot->yAxis->range().upper)
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() - fDurTime); // place position at center/top of axis rect
	else
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() + fDurTime); // place position at center/top of axis rect

	m_mapTraceHint[iIndex]->pTextLabel->setText(strName);
	m_mapTraceHint[iIndex]->pTextLabel->setFont(QFont(m_plot->font().family(), 9)); // make font a bit larger
	m_mapTraceHint[iIndex]->pTextLabel->setPen(QPen(Qt::black)); // show black border around text

	if (m_mapTraceHint[iIndex]->pArrow == nullptr)	m_mapTraceHint[iIndex]->pArrow = new QCPItemLine(m_plot);
	m_mapTraceHint[iIndex]->pArrow->start->setParentAnchor(m_mapTraceHint[iIndex]->pTextLabel->bottom);
	m_mapTraceHint[iIndex]->pArrow->end->setCoords(sPos.x(), sPos.y()); // point to (4, 1.6) in x-y-plot coordinates
	m_mapTraceHint[iIndex]->pArrow->setHead(QCPLineEnding::esSpikeArrow);
}

void XxwDetectItemLine::ClearTraceHint()
{
	for (auto& iter		:		m_mapTraceHint)
	{
		if (iter->pArrow)		m_plot->removeItem(iter->pArrow);
		if (iter->pTextLabel)	m_plot->removeItem(iter->pTextLabel);
	}
	m_mapTraceHint.clear();
}

void XxwDetectItemLine::SetQLineF(QLineF & _line)
{
	if (m_pItemLine == nullptr)
	{
		initLine();
	}
	if (m_pItemLine != nullptr)
	{
		m_pItemLine->start->setCoords(_line.p1());
		m_pItemLine->end->setCoords(_line.p2());
	}
}

XxwDetectItemCircle::XxwDetectItemCircle(QCustomPlot * _plot, QObject * parent)
	: QObject(parent), m_plot(_plot)
{
	m_pQCPCurve = Q_NULLPTR;
	ClearTraceHint();
	initItem();
}

XxwDetectItemCircle::~XxwDetectItemCircle()
{
	//if (m_plot)
	//{
	//	if (m_pQCPCurve)		m_plot->removeItem(m_pQCPCurve);
	//}
	ClearTraceHint();
}

void XxwDetectItemCircle::initItem()
{
	if (m_plot)
	{
		QPen linesPen(Qt::blue, 2, Qt::SolidLine);
		m_pQCPCurve = new QCPCurve(m_plot->xAxis, m_plot->yAxis);//垂直线
		m_pQCPCurve->setLayer("overlay");
		m_pQCPCurve->setPen(linesPen);
		m_pQCPCurve->setBrush(QBrush(QColor(0, 0, 255, 20)));
	}
}

void XxwDetectItemCircle::setPen(const QPen & pen)
{
	if (m_pQCPCurve)	m_pQCPCurve->setPen(pen);
}

void XxwDetectItemCircle::updatePosition(QPointF sPos, double radius)
{
	QVector<QCPCurveData> dataSpiral111(360);
	for (size_t i = 0; i < 360; i++)
	{
		double phi = i / 180.0 * M_PI;
		double x = radius * cos(phi);
		double y = radius * sin(phi);
		dataSpiral111[i] = QCPCurveData(i, x + sPos.x(), y + sPos.y());
	}
	m_pQCPCurve->data()->set(dataSpiral111, true);
	m_pQCPCurve->setPen(QPen(Qt::blue));
	m_pQCPCurve->setBrush(QBrush(QColor(0, 0, 255, 20)));
}

void XxwDetectItemCircle::setVisible(bool vis)
{
	if (m_pQCPCurve)			m_pQCPCurve->setVisible(vis);
	for (auto& iter : m_mapTraceHint)
	{
		if (iter->pArrow)		iter->pArrow->setVisible(vis);
		if (iter->pTextLabel)	iter->pTextLabel->setVisible(vis);
	}
	m_visible = vis;
}

void XxwDetectItemCircle::setTraceHint(int iIndex, QPointF sPos, QPointF Textpos, QString strName)
{
	if (m_mapTraceHint.count(iIndex) <= 0)
	{
		TraceHint* _pTraceHint = new TraceHint(this);
		m_mapTraceHint.insert(iIndex, _pTraceHint);
	}
	float fDurTime = m_plot->yAxis->range().upper - m_plot->yAxis->range().lower;
	fDurTime = fDurTime * 0.1;

	if (m_mapTraceHint[iIndex]->pTextLabel == nullptr)	m_mapTraceHint[iIndex]->pTextLabel = new QCPItemText(m_plot);
	if ((Textpos.y() + fDurTime) > m_plot->yAxis->range().upper)
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() - fDurTime); // place position at center/top of axis rect
	else
		m_mapTraceHint[iIndex]->pTextLabel->position->setCoords(Textpos.x(), Textpos.y() + fDurTime); // place position at center/top of axis rect

	m_mapTraceHint[iIndex]->pTextLabel->setText(strName);
	m_mapTraceHint[iIndex]->pTextLabel->setFont(QFont(m_plot->font().family(), 9)); // make font a bit larger
	m_mapTraceHint[iIndex]->pTextLabel->setPen(QPen(Qt::black)); // show black border around text

	if (m_mapTraceHint[iIndex]->pArrow == nullptr)	m_mapTraceHint[iIndex]->pArrow = new QCPItemLine(m_plot);
	m_mapTraceHint[iIndex]->pArrow->start->setParentAnchor(m_mapTraceHint[iIndex]->pTextLabel->bottom);
	m_mapTraceHint[iIndex]->pArrow->end->setCoords(sPos.x(), sPos.y()); // point to (4, 1.6) in x-y-plot coordinates
	m_mapTraceHint[iIndex]->pArrow->setHead(QCPLineEnding::esSpikeArrow);
}

void XxwDetectItemCircle::ClearTraceHint()
{
	for (auto& iter : m_mapTraceHint)
	{
		if (iter->pArrow)		m_plot->removeItem(iter->pArrow);
		if (iter->pTextLabel)	m_plot->removeItem(iter->pTextLabel);
	}
	m_mapTraceHint.clear();
}

void XxwDetectItemCircle::SetCircle(QPointF sPos, double radius)
{
	QVector<QCPCurveData> dataSpiral111(360);
	for (size_t i = 0; i < 360; i++)
	{
		double phi = i / 180.0 * M_PI;
		double x = radius * cos(phi);
		double y = radius * sin(phi);
		dataSpiral111[i] = QCPCurveData(i, x + sPos.x(), y + sPos.y());
	}
	m_pQCPCurve->data()->set(dataSpiral111, true);
	m_pQCPCurve->setPen(QPen(Qt::blue));
	m_pQCPCurve->setBrush(QBrush(QColor(0, 0, 255, 20)));
	m_CenterPos = sPos;
	m_dRadius	= radius;
}
