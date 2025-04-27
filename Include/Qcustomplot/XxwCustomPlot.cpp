#include "XxwCustomPlot.h"

double GetDistanceTwoPoint(float x, float y, QPointF ptstart, QPointF ptEnd)
{
	return abs((ptstart.y() - ptEnd.y())* x + (ptEnd.x() - ptstart.x())* y + ptstart.x() * ptEnd.y() - ptstart.y()* ptEnd.x())
		/ sqrt((ptstart.y() - ptEnd.y())* (ptstart.y() - ptEnd.y()) + (ptstart.x() - ptEnd.x()) * (ptstart.x() - ptEnd.x()));
}

//求垂足
QPointF GetCrossPoint(QLineF line, QPointF point)
{
	qreal a		= line.dy();
	qreal b		= -line.dx();
	qreal c		= -a * point.x() - b * point.y();
	qreal y		= (c - b * point.x()) / a;
	qreal x		= (c - a * y) / b;
	return QPointF(x,y);
}

bool IsDistanceTwoPoint(float x, float y, QPointF ptstart, QPointF ptEnd, float xLenth, float yLenth)
{
	QLineF _line;
	_line.setP1(ptstart);
	_line.setP2(ptEnd);
	QPointF _ptPoint(x, y);
	QPointF _Cross = GetCrossPoint(_line, _ptPoint);
	qreal _dx = abs(_Cross.x() - x);
	qreal _dy = abs(_Cross.y() - y);
	if (_dx > xLenth || _dy > yLenth)
	{
		return false;
	}
	return true;
}

//点在点的范围内
bool IsPointInPoint(float x, float y, QPointF ptCenter, float xLenth, float yLenth)
{
	if ((x > ptCenter.x() - xLenth) && (x < ptCenter.x() + xLenth)
		&& (y > ptCenter.y() - yLenth) && (y < ptCenter.y() + yLenth))
	{
		return true;
	}
	return false;
}

//点在线上
int IsPointInLine(float x, float y, QPointF ptStart, QPointF ptEnd, float xLenth,float yLenth)
{
	if (IsPointInPoint(x, y, ptStart, xLenth, yLenth))
		return 5;
	else if (IsPointInPoint(x, y, ptEnd, xLenth, yLenth))
		return 5 + 1;
	if (ptStart.x() == ptEnd.x()
		&& (y > std::min(ptStart.y(), ptEnd.y()) && y < std::max(ptStart.y(), ptEnd.y())))	{	//X方向上的区间
	
		if (x < (ptStart.x() - xLenth) || x >(ptStart.x() + xLenth))	{
			return FALSE;
		}
		return TRUE;
	}
	else if (ptStart.y() == ptEnd.y()
		&& (x > std::min(ptStart.x(), ptEnd.x()) && x < std::max(ptStart.x(), ptEnd.x())))	{
		if (y < (ptStart.y() - yLenth) || y >(ptStart.y() + yLenth))	{
			return FALSE;
		}
		return TRUE;
	}
	if ( x <	std::min(ptStart.x(), ptEnd.x()) 
		|| x >  std::max(ptStart.x(), ptEnd.x())
		|| y <  std::min(ptStart.y(), ptEnd.y()) 
		|| y >  std::max(ptStart.y(), ptEnd.y()))
		return FALSE;

	if (!IsDistanceTwoPoint(x, y, ptStart, ptEnd, xLenth, yLenth ) )
		return TRUE;
	return FALSE;
}

XxwCustomPlot::XxwCustomPlot(QWidget *parent)
    :QCustomPlot(parent)
    ,m_isShowTracer(false)
    ,m_xTracer(Q_NULLPTR)
    ,m_yTracer(Q_NULLPTR)
    ,m_dataTracers(QList<XxwTracer *>())
    ,m_lineTracer(Q_NULLPTR)
	,m_XxwLimitRect(Q_NULLPTR)
	,m_iSelectedLimitMode(-1)
{
	m_XxwLimitRectLst.clear();
	m_XxwLimitRectLst.push_back((m_XxwLimitRect = new XxwTraceRect(this, XxwTraceRect::LimitTypeUpAndDown)));
	m_XxwTraceItemLineLst.clear();
	m_XxwTraceItemLineLst.push_back((m_XxwTraceItemLine = new XxwDetectItemLine(this)));
	ClearDrawLines();
}

XxwCustomPlot::~XxwCustomPlot()
{
	for (auto iter : m_XxwLimitRectLst)	{
		if (iter != nullptr) delete iter;	iter = nullptr;
	}
	m_XxwLimitRectLst.clear();
	m_XxwLimitRect = nullptr;

	for (auto iter : m_XxwTraceItemLineLst)	{
		if (iter != nullptr) delete iter;	iter = nullptr;
	}
	m_XxwTraceItemLineLst.clear();
	m_XxwTraceItemLine = nullptr;

	ClearDrawLines();
	if (m_xTracer != nullptr)	delete m_xTracer; m_xTracer = nullptr;
	if (m_yTracer != nullptr)	delete m_yTracer; m_yTracer = nullptr;
}

void XxwCustomPlot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);

	//当前鼠标位置（像素坐标）
	int x_pos = event->pos().x();
	int y_pos = event->pos().y();

	//像素坐标转成实际的x,y轴的坐标
	float x_val = this->xAxis->pixelToCoord(x_pos);
	float y_val = this->yAxis->pixelToCoord(y_pos);

    if(m_isShowTracer)	{
        if(Q_NULLPTR == m_xTracer)	m_xTracer = new XxwTracer(this, XxwTracer::XAxisTracer);//x轴
        m_xTracer->updatePosition(x_val, y_val);

        if(Q_NULLPTR == m_yTracer)	m_yTracer = new XxwTracer(this, XxwTracer::YAxisTracer);//y轴
        m_yTracer->updatePosition(x_val, y_val);

        int nTracerCount = m_dataTracers.count();
        int nGraphCount = graphCount();
        if(nTracerCount < nGraphCount)	{
            for(int i = nTracerCount; i < nGraphCount; ++i)	{
                XxwTracer *tracer = new XxwTracer(this, XxwTracer::DataTracer);
                m_dataTracers.append(tracer);
            }
        }
        else if(nTracerCount > nGraphCount)	{
            for(int i = nGraphCount; i < nTracerCount; ++i)	{
                XxwTracer *tracer = m_dataTracers[i];
                if(tracer)	{
                    tracer->setVisible(false);
                }
            }
        }
        for (int i = 0; i < nGraphCount; ++i)	{
            XxwTracer *tracer = m_dataTracers[i];
            if(!tracer)	tracer = new XxwTracer(this, XxwTracer::DataTracer);
            tracer->setVisible(true);
            tracer->setPen(this->graph(i)->pen());
            tracer->setBrush(Qt::NoBrush);
            tracer->setLabelPen(this->graph(i)->pen());
            auto iter = this->graph(i)->data()->findBegin(x_val);
            double value = iter->mainValue();
			tracer->updatePosition(x_val, value);
        }
        if(Q_NULLPTR == m_lineTracer)	m_lineTracer = new XxwTraceLine(this,Both);//直线
        m_lineTracer->updatePosition(x_val, y_val);
		RePlotALLTrace();
		//if (Q_NULLPTR != m_XxwLimitRect)	m_XxwLimitRect->RePlotALLTrace();
    }
	//用来显示的指示线
	for (auto& iter : m_pMaplineTracers)	{
		if (iter != nullptr)	{
			iter->UpdatePos();
		}
	}
	if (m_XxwLimitRect != Q_NULLPTR)	{
		switch (m_iSelectedLimitMode)	{
		case 1:	{
			m_XxwLimitRect->updatePosition(
				m_XxwLimitRect->m_RectUpRange.fValue,
				m_XxwLimitRect->m_RectUpRange.fRangeValue,
				x_val + m_MouseLimitOffset.x(),
				m_XxwLimitRect->m_RectDnRange.fRangeValue);
		}break;
		case 2:	{
			m_XxwLimitRect->updatePosition(
				x_val + m_MouseLimitOffset.x(),
				m_XxwLimitRect->m_RectUpRange.fRangeValue,
				m_XxwLimitRect->m_RectDnRange.fValue,
				m_XxwLimitRect->m_RectDnRange.fRangeValue);
		}break;
		case 3:	{
			m_XxwLimitRect->updatePosition(
				m_XxwLimitRect->m_RectUpRange.fValue,
				x_val - m_XxwLimitRect->m_RectUpRange.fValue,
				m_XxwLimitRect->m_RectDnRange.fValue,
				m_XxwLimitRect->m_RectDnRange.fRangeValue);
		}break;
		case 4:	{
			m_XxwLimitRect->updatePosition(
				x_val,
				m_CurrentRectRange.fRangeValue + m_CurrentRectRange.fValue - (x_val),
				m_XxwLimitRect->m_RectDnRange.fValue,
				m_XxwLimitRect->m_RectDnRange.fRangeValue);
		}break;
		case 5:	{
			m_XxwLimitRect->updatePosition(
				m_XxwLimitRect->m_RectUpRange.fValue,
				m_XxwLimitRect->m_RectUpRange.fRangeValue,
				m_XxwLimitRect->m_RectDnRange.fValue,
				x_val - m_XxwLimitRect->m_RectDnRange.fValue);
		}break;
		case 6:	{
			m_XxwLimitRect->updatePosition(
				m_XxwLimitRect->m_RectUpRange.fValue,
				m_XxwLimitRect->m_RectUpRange.fRangeValue,
				x_val,
				m_CurrentRectRange.fRangeValue + m_CurrentRectRange.fValue - (x_val));
		}break;
		default: {

		}	break;
		}
	}

	if (m_XxwTraceItemLine != Q_NULLPTR)
		switch (m_iSelectedTraceItemMode)
		{
		case 1:		//直线上
		{
			QLineF _line;
			_line.setP1(QPointF(m_CurrentQLineF.p1() + QPointF(x_val - m_MousePressPt.x() , y_val - m_MousePressPt.y())));
			_line.setP2(QPointF(m_CurrentQLineF.p2() + QPointF(x_val - m_MousePressPt.x(), y_val - m_MousePressPt.y())));
			m_XxwTraceItemLine->updatePosition(_line);
		}	break;
		case 5:		//首点
		{
			QLineF _line;
			_line.setP1(QPointF(x_val, y_val));
			_line.setP2(m_CurrentQLineF.p2());
			m_XxwTraceItemLine->updatePosition(_line);
		}	break;
		case 6:		//尾点
		{
			QLineF _line;
			_line.setP1(m_CurrentQLineF.p1());
			_line.setP2(QPointF(x_val, y_val));
			m_XxwTraceItemLine->updatePosition(_line);
		}	break;
		default:
			break;
		}

	RePlotALLTrace(); 
}

void XxwCustomPlot::mousePressEvent(QMouseEvent * event)
{
	QCustomPlot::mousePressEvent(event);

	//当前鼠标位置（像素坐标）
	int x_pos = event->pos().x();
	int y_pos = event->pos().y();

	//像素坐标转成实际的x,y轴的坐标
	float x_val = this->xAxis->pixelToCoord(x_pos);
	float y_val = this->yAxis->pixelToCoord(y_pos);

	m_MousePressPt		= QPointF(x_val, y_val);
	m_MouseLimitOffset	= QPointF(0,0);

	if (m_XxwLimitRect != Q_NULLPTR)
	{
		if (m_XxwLimitRect->m_lineDn != Q_NULLPTR)	m_XxwLimitRect->m_lineDn->setPen(QPen(*m_XxwLimitRect->m_QColor, 2));
		if (m_XxwLimitRect->m_lineUp != Q_NULLPTR)	m_XxwLimitRect->m_lineUp->setPen(QPen(*m_XxwLimitRect->m_QColor, 2));
	}

	m_iSelectedLimitMode	=	JudgeMousePoint(m_MousePressPt, m_MouseLimitOffset, m_CurrentRectRange);
	if (m_XxwLimitRect == Q_NULLPTR) { RePlotALLTrace(); goto Next; }
	switch (m_iSelectedLimitMode)
	{
	case 1:
	case 5:
	case 6: {
		if(m_XxwLimitRect->m_lineDn != Q_NULLPTR)	m_XxwLimitRect->m_lineDn->setPen(QPen(Qt::red, 3, Qt::DashLine));
		if (m_XxwLimitRect->m_lineUp != Q_NULLPTR)	m_XxwLimitRect->m_lineUp->setPen(QPen(*m_XxwLimitRect->m_QColor, 2));
		emit sig_XxwTraceRect(m_XxwLimitRect);
	}; break;
	case 2:
	case 3:
	case 4: {
		if (m_XxwLimitRect->m_lineDn != Q_NULLPTR)	m_XxwLimitRect->m_lineDn->setPen(QPen(*m_XxwLimitRect->m_QColor, 2));
		if (m_XxwLimitRect->m_lineUp != Q_NULLPTR)	m_XxwLimitRect->m_lineUp->setPen(QPen(Qt::red, 3, Qt::DashLine));
		emit sig_XxwTraceRect(m_XxwLimitRect);
	}; break;
	default:{
	}	break;
	}

Next:
	if (m_XxwLimitRect == Q_NULLPTR)
	{
		m_iSelectedTraceItemMode = JudgeMouseLinePoint(m_MousePressPt, m_MouseLineOffset);
		if (m_XxwTraceItemLine == Q_NULLPTR) { RePlotALLTrace(); return; }
		m_CurrentQLineF.setP1(m_XxwTraceItemLine->m_pItemLine->start->coords());
		m_CurrentQLineF.setP2(m_XxwTraceItemLine->m_pItemLine->end->coords());
	}

	switch (m_iSelectedTraceItemMode)
	{
	case 0:
	{
		
	} break;
	case 1:
	{

	} break;
	case 5:
	{

	} break;
	case 6:
	{

	} break;
	default:
		break;
	}
}

void XxwCustomPlot::mouseReleaseEvent(QMouseEvent * event)
{
	QCustomPlot::mouseReleaseEvent(event);

	//当前鼠标位置（像素坐标）
	int x_pos = event->pos().x();
	int y_pos = event->pos().y();

	//像素坐标转成实际的x,y轴的坐标
	float x_val = this->xAxis->pixelToCoord(x_pos);
	float y_val = this->yAxis->pixelToCoord(y_pos);
	if ((Q_NULLPTR != m_XxwLimitRect && m_iSelectedLimitMode != -1) )
		emit sig_ChangeTraceRect(m_XxwLimitRect);
	else if((Q_NULLPTR != m_XxwTraceItemLine && m_iSelectedTraceItemMode != -1))
		emit sig_ChangeDetectItemLine(m_XxwTraceItemLine);

	if (m_XxwLimitRect != Q_NULLPTR)
	{
		if (m_XxwLimitRect->m_lineDn != Q_NULLPTR)	m_XxwLimitRect->m_lineDn->setPen(QPen(*m_XxwLimitRect->m_QColor, 2));
		if (m_XxwLimitRect->m_lineUp != Q_NULLPTR)	m_XxwLimitRect->m_lineUp->setPen(QPen(*m_XxwLimitRect->m_QColor, 2));
	}
	m_XxwLimitRect				= Q_NULLPTR;
	m_XxwTraceItemLine			= Q_NULLPTR;
	m_iSelectedLimitMode		= -1;
	m_iSelectedTraceItemMode	= -1;
}

void XxwCustomPlot::wheelEvent(QWheelEvent * event)
{
	QCustomPlot::wheelEvent(event);

	//用来显示的指示线
	for (auto& iter : m_pMaplineTracers)
	{
		if (iter != nullptr)
		{
			iter->UpdatePos();
		}
	}

	RePlotALLTrace();
}

void XxwCustomPlot::InitPlot()
{
	if (Q_NULLPTR != m_XxwLimitRect)	m_XxwLimitRect->ResetTrace();


}

void XxwCustomPlot::UpDatePlot()
{
	if (Q_NULLPTR != m_XxwLimitRect)	m_XxwLimitRect->RePlotTrace();

}

void XxwCustomPlot::ResetLimit()
{
	if (Q_NULLPTR != m_XxwLimitRect)	m_XxwLimitRect->ResetLimit();
}

void XxwCustomPlot::ClearDrawLines()
{
	for (auto& iter : m_pMapDrawItemlines)
	{
		if (iter != nullptr) delete iter;
		iter = nullptr;
	}
	m_pMapDrawItemlines.clear();
}

QRectF XxwCustomPlot::GetQRectF()
{
	return QRectF(xAxis->range().lower,yAxis->range().lower, abs(xAxis->range().upper - xAxis->range().lower), abs(yAxis->range().upper - yAxis->range().lower));
}

void XxwCustomPlot::SetSearchRectVisable(bool bVisable)
{
	if (m_XxwLimitRect != Q_NULLPTR)	m_XxwLimitRect->setVisible(bVisable);
	for (auto& iter : m_XxwLimitRectLst)
	{
		if (iter != nullptr)	iter->setVisible(bVisable);
	}
}

void XxwCustomPlot::SetTraceItemLineVisable(bool bVisable)
{
	if (m_XxwTraceItemLine != Q_NULLPTR)	m_XxwTraceItemLine->setVisible(bVisable);
	for (auto& iter : m_XxwTraceItemLineLst)
	{
		if (iter != nullptr)	iter->setVisible(bVisable);
	}
}

void XxwCustomPlot::SetTraceItemCircleVisable(bool bVisable)
{
	for (auto& iter : m_pMapTraceItemCircles)
	{
		if (iter != nullptr)	iter->setVisible(bVisable);
	}
}

void XxwCustomPlot::DrawAllItemClear()
{
	for (auto& iter : m_pMapDrawItemlines)	{
		if (iter != nullptr)	iter->setVisible(false);
	}
	for (auto& iter : m_pMaplineTracers)	{
		if (iter != nullptr)	iter->setVisible(false);
	}
	for (auto& iter : m_pMapTraceItemCircles)	{
		if (iter != nullptr)	iter->setVisible(false);
	}
	//for (auto& iter : m_XxwLimitRectLst)	{
	//	if (iter != nullptr)	iter->setVisible(false);
	//}
	//for (auto& iter : m_XxwTraceItemLineLst) {
	//	if (iter != nullptr)	iter->setVisible(false);
	//}
	//if (m_XxwTraceItemLine != nullptr)	{
	//	m_XxwTraceItemLine->setVisible(false);
	//}
	//if (m_XxwLimitRect != nullptr) {
	//	m_XxwLimitRect->setVisible(false);
	//}
}

void XxwCustomPlot::DrawClearAllItem()
{
	for (auto& iter : m_pMapDrawItemlines) {
		if (iter != nullptr)	iter->setVisible(false);
	}
	for (auto& iter : m_pMaplineTracers) {
		if (iter != nullptr)	iter->setVisible(false);
	}
	for (auto& iter : m_pMapTraceItemCircles) {
		if (iter != nullptr)	iter->setVisible(false);
	}
	for (auto& iter : m_XxwLimitRectLst)	{
		if (iter != nullptr)	iter->setVisible(false);
	}
	for (auto& iter : m_XxwTraceItemLineLst) {
		if (iter != nullptr)	iter->setVisible(false);
	}
	if (m_XxwTraceItemLine != nullptr)	{
		m_XxwTraceItemLine->setVisible(false);
	}
	if (m_XxwLimitRect != nullptr) {
		m_XxwLimitRect->setVisible(false);
	}
}

void XxwCustomPlot::DrawItemCircle(int index, const QPointF & _ptCenter, double radius)
{
	if (m_pMapTraceItemCircles.count(index) <= 0)
		m_pMapTraceItemCircles.insert(index, new XxwDetectItemCircle(this));
	m_pMapTraceItemCircles[index]->setVisible(true);
	m_pMapTraceItemCircles[index]->SetCircle(_ptCenter, radius);
}

void XxwCustomPlot::DrawItemLines(int index, const QLineF & _line, QString strName, const QPen& pen)
{
	if (m_pMapDrawItemlines.count(index) <= 0)
		m_pMapDrawItemlines.insert(index,new XxwItemLine(this));
	m_pMapDrawItemlines[index]->setVisible(true);
	m_pMapDrawItemlines[index]->SetQLineF(_line);
	m_pMapDrawItemlines[index]->setPen(pen);

	if (!strName.isEmpty())	m_pMapDrawItemlines[index]->setTraceHint(0, _line.center(), _line.center(), strName);

}

void XxwCustomPlot::DrawItemPoint(int index, const  QVector<QPointF>& Pos, const  QVector<QString>& strName)
{
	if (m_pMaplineTracers.count(index) <= 0)
		m_pMaplineTracers.insert(index, new XxwTraceHintLine(this));
	//m_pMaplineTracers[index]->setVisible(true);
	for (size_t i = 0; i < Pos.size(); i++)
	{
		QPointF sPos = Pos[i];
		if(i < (strName.size()))
			m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, strName[i]);
		else m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, "");
	}
}

void XxwCustomPlot::DrawItemPoint(int index, const  QVector<QPointF>& Pos, const  QVector<QString>& strName, const QVector<QPen>& vecPen)
{
	if (m_pMaplineTracers.count(index) <= 0)
		m_pMaplineTracers.insert(index, new XxwTraceHintLine(this));
	//m_pMaplineTracers[index]->setVisible(true);
	for (size_t i = 0; i < Pos.size(); i++)
	{
		QPointF sPos = Pos[i];
		if (i < (strName.size()))
		{
			if (i < (vecPen.size()))
			{
				m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, vecPen[i], strName[i]);
			}
			else m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, strName[i]);
		}
		else {
			if (i < (vecPen.size()))
			{
				m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, vecPen[i], "");
			}
			else m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, "");

			m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, "");
		}
	}
}

void XxwCustomPlot::DrawItemPoint(int index, const  QVector<double>& x, const QVector<double>& y, const QVector<QString>& strName)
{
	if (m_pMaplineTracers.count(index) <= 0)
		m_pMaplineTracers.insert(index, new XxwTraceHintLine(this));
	//m_pMaplineTracers[index]->setVisible(true);
	for (size_t i = 0; i < x.size(); i++)
	{
		QPointF sPos;
		sPos.setX(x[i]);
		sPos.setY(y[i]);
		if (i < (strName.size()))
			m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, strName[i]);
		else m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, "");
	}
}

void XxwCustomPlot::DrawItemPoint(int index,const QVector<double>& x, const QVector<double>& y,const QVector<QString>& strName, const QVector<QPen>& vecPen)
{
	if (m_pMaplineTracers.count(index) <= 0)
		m_pMaplineTracers.insert(index, new XxwTraceHintLine(this));
	//m_pMaplineTracers[index]->setVisible(true);
	for (size_t i = 0; i < x.size(); i++)
	{
		QPointF sPos;
		sPos.setX(x[i]);
		sPos.setY(y[i]);
		if (i < (strName.size()))
		{
			if (i < (vecPen.size()))
			{
				m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, vecPen[i], strName[i]);
			}
			else m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, strName[i]);
		}
		else {
			if (i < (vecPen.size()))
			{
				m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, vecPen[i], "");
			}
			else m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, "");

			m_pMaplineTracers[index]->setTraceHint(i, sPos, sPos, "");
		}
	}
}

void XxwCustomPlot::showTracer(bool show)
{
	m_isShowTracer = show;
	if (m_xTracer)	m_xTracer->setVisible(m_isShowTracer);
	if (m_yTracer)	m_yTracer->setVisible(m_isShowTracer);
	foreach(XxwTracer *tracer, m_dataTracers)
	{
		if (tracer)		tracer->setVisible(m_isShowTracer);
	}
	if (m_lineTracer)	m_lineTracer->setVisible(m_isShowTracer);
}

int XxwCustomPlot::JudgeMousePoint(QPointF & pt, QPointF& ptOffset,RectRange& range)
{
	if (m_XxwLimitRectLst.size() <= 0)	return -1;
	float fDurTime		= (xAxis->range().upper - xAxis->range().lower) / 30.0;

	for (int i = 0; i < m_XxwLimitRectLst.size(); i++)
	{
		float _fUpDurTime = m_XxwLimitRectLst[i]->m_RectUpRange.fRangeValue / 10.0;//std::max(m_XxwLimitRectLst[i]->m_RectUpRange.fRangeValue / 10.0, fDurTime );
		float _fDnDurTime = m_XxwLimitRectLst[i]->m_RectDnRange.fRangeValue / 10.0;//std::max(m_XxwLimitRectLst[i]->m_RectDnRange.fRangeValue / 10.0, fDurTime);
		if (pt.x() >= (m_XxwLimitRectLst[i]->m_RectDnRange.fValue - _fDnDurTime)		//下限下下移
			&& pt.x() <= (m_XxwLimitRectLst[i]->m_RectDnRange.fValue + _fDnDurTime))
		{
			m_XxwLimitRect = m_XxwLimitRectLst[i];
			range = m_XxwLimitRectLst[i]->m_RectDnRange;
			ptOffset.setX(pt.x() - (m_XxwLimitRectLst[i]->m_RectDnRange.fValue + m_XxwLimitRectLst[i]->m_RectDnRange.fRangeValue));
			ptOffset.setY(0);
			return 6;
		}
		else if (pt.x() >= (m_XxwLimitRectLst[i]->m_RectDnRange.fValue + m_XxwLimitRectLst[i]->m_RectDnRange.fRangeValue - _fDnDurTime)	//下限上上移
			&& pt.x() <= (m_XxwLimitRectLst[i]->m_RectDnRange.fValue + m_XxwLimitRectLst[i]->m_RectDnRange.fRangeValue + _fDnDurTime))
		{
			m_XxwLimitRect = m_XxwLimitRectLst[i];
			range = m_XxwLimitRectLst[i]->m_RectDnRange;
			ptOffset.setX(pt.x() - (m_XxwLimitRectLst[i]->m_RectDnRange.fValue));
			ptOffset.setY(0);
			return 5;
		}
		else if (pt.x() >= (m_XxwLimitRectLst[i]->m_RectUpRange.fValue - _fUpDurTime)	//上限下下移
			&& pt.x() <= (m_XxwLimitRectLst[i]->m_RectUpRange.fValue + _fUpDurTime))
		{
			m_XxwLimitRect = m_XxwLimitRectLst[i];
			range = m_XxwLimitRectLst[i]->m_RectUpRange;
			ptOffset.setX(pt.x() - (m_XxwLimitRectLst[i]->m_RectUpRange.fValue + m_XxwLimitRectLst[i]->m_RectUpRange.fRangeValue));
			ptOffset.setY(0);
			return 4;
		}
		else if (pt.x() >= (m_XxwLimitRectLst[i]->m_RectUpRange.fValue + m_XxwLimitRectLst[i]->m_RectUpRange.fRangeValue - _fUpDurTime)		//上限上上移
			&& pt.x() <= (m_XxwLimitRectLst[i]->m_RectUpRange.fValue + m_XxwLimitRectLst[i]->m_RectUpRange.fRangeValue + _fUpDurTime))
		{
			m_XxwLimitRect = m_XxwLimitRectLst[i];
			range = m_XxwLimitRectLst[i]->m_RectUpRange;
			ptOffset.setX(pt.x() - (m_XxwLimitRectLst[i]->m_RectUpRange.fValue));
			ptOffset.setY(0);
			return 3;
		}
		else if (pt.x() >= m_XxwLimitRectLst[i]->m_RectUpRange.fValue		//上限平移
			&& pt.x() <= (m_XxwLimitRectLst[i]->m_RectUpRange.fRangeValue + m_XxwLimitRectLst[i]->m_RectUpRange.fValue))
		{
			m_XxwLimitRect = m_XxwLimitRectLst[i];
			range = m_XxwLimitRectLst[i]->m_RectUpRange;
			ptOffset.setX(-pt.x() + m_XxwLimitRectLst[i]->m_RectUpRange.fValue);
			ptOffset.setY(0);
			return 2;
		}
		else if (pt.x() >= m_XxwLimitRectLst[i]->m_RectDnRange.fValue	//下限平移
			&&		pt.x() <= (m_XxwLimitRectLst[i]->m_RectDnRange.fRangeValue + m_XxwLimitRectLst[i]->m_RectDnRange.fValue))
		{
			m_XxwLimitRect = m_XxwLimitRectLst[i];
			range = m_XxwLimitRectLst[i]->m_RectDnRange;
			ptOffset.setX(-pt.x() + m_XxwLimitRectLst[i]->m_RectDnRange.fValue);
			ptOffset.setY(0);
			return 1;
		}
	}
	m_XxwLimitRect = nullptr;
	return 0;
}

int XxwCustomPlot::JudgeMouseLinePoint(QPointF& ptMouse, QPointF & ptOffset)
{
	if (m_XxwTraceItemLineLst.size() <= 0)	return -1;
	float fxLenth = (xAxis->range().upper - xAxis->range().lower) / 30.0;
	float fyLenth = (yAxis->range().upper - yAxis->range().lower) / 30.0;
	int _iRetn = 0;
	for (int i = 0; i < m_XxwTraceItemLineLst.size(); i++)
	{
		QPointF _startPos	= m_XxwTraceItemLineLst[i]->m_pItemLine->start->coords();
		QPointF _endPos		= m_XxwTraceItemLineLst[i]->m_pItemLine->end->coords();
		
		//判定当前的位置是否在线上
		if ((_iRetn = (IsPointInLine(ptMouse.x(), ptMouse.y(), _startPos, _endPos, fxLenth, fyLenth))) > 0 )
		{
			m_XxwTraceItemLine = m_XxwTraceItemLineLst[i];
			return _iRetn;
		}
	}
	return 0;
}

void XxwCustomPlot::ResetALLLimit()
{
	for (size_t i = 0; i < m_XxwLimitRectLst.size(); i++)
	{
		m_XxwLimitRectLst[i]->ResetLimit();
	}
	this->replot();//曲线重绘
}

void XxwCustomPlot::RePlotALLTrace()
{
	for (size_t i = 0; i < m_XxwLimitRectLst.size(); i++)
	{
		m_XxwLimitRectLst[i]->RePlotTrace();
	}

	this->replot();//曲线重绘
}
