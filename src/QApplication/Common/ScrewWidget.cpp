#include "ScrewWidget.h"
#include <QtMath>

ScrewWidget::ScrewWidget(QWidget *parent)
	: QWidget(parent)
{
	this->resize(600, 600);
	initWidget();
}

ScrewWidget::~ScrewWidget()
{

}

//初始化界面
void ScrewWidget::initWidget()
{
	m_vecScrewPos.clear();
	//初始化变量
	refreshChart();
	m_ScrewColor.insert(0,	ScrewColor(QColor(Qt::yellow),	tr("Idle")));
	m_ScrewColor.insert(1,	ScrewColor(QColor(Qt::green),	tr("OK")));
	//m_ScrewColor.insert(2,	ScrewColor(QColor(Qt::blue),	tr("Other")));
	m_ScrewColor.insert(-1, ScrewColor(QColor(Qt::red),		tr("NG")));

}

void ScrewWidget::ClearScrewPos()
{
	m_vecScrewPos.clear();
}

void ScrewWidget::SetScrewPos(int index,double posX, double posY)
{
	if (m_vecScrewPos.count(index) <= 0) {
		m_vecScrewPos.insert(index, ScrewPos());
	}
	m_vecScrewPos[index].Pos.setX(posX);
	m_vecScrewPos[index].Pos.setY(posY);
}

void ScrewWidget::ClearScrewStatus()
{
	for (auto iter = m_vecScrewPos.begin(); iter != m_vecScrewPos.end(); iter++) {
		iter.value().iScrewStatus = 0;
	}
	update();
}

void ScrewWidget::SetScrewPosStatus(int index, int istatus)
{
	if (m_vecScrewPos.count(index) > 0)
		m_vecScrewPos[index].iScrewStatus = istatus;
	update();
}

void ScrewWidget::setTextTitle(QString strText)
{
	m_strTitle = strText;
}

//更新饼图
void ScrewWidget::refreshChart()
{
	int	i = 0;
	for (auto iter = m_vecScrewPos.begin(); iter != m_vecScrewPos.end(); iter++) {
		if (i == 0)	{
			m_ScrewRect.setTopLeft((*iter).Pos);
			m_ScrewRect.setBottomRight((*iter).Pos);
		}
		if ( (*iter).Pos.x()  < m_ScrewRect.left())	{
			m_ScrewRect.setLeft(	(*iter).Pos.x()	);
		}
		if ( (*iter).Pos.x()  > m_ScrewRect.right()) {
			m_ScrewRect.setRight( (*iter).Pos.x());
		}
		if ( (*iter).Pos.y()  < m_ScrewRect.top()) {
			m_ScrewRect.setTop( (*iter).Pos.y());
		}
		if ( (*iter).Pos.y()  > m_ScrewRect.bottom()) {
			m_ScrewRect.setBottom( (*iter).Pos.y());
		}
		i++;
	}
}

void ScrewWidget::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);   //抗锯齿
	refreshChart();
	//绘制背景
	painter.setBrush(Qt::white);
	painter.setPen(Qt::NoPen);   //去除背景边框
	//painter.drawRoundedRect(10, 10, 360, 360, 8, 8);
	for (auto iter = m_vecScrewPos.begin(); iter != m_vecScrewPos.end(); iter++)	{
		QPointF _ptCenter;
		_ptCenter.setX(iter.value().Pos.x() - (m_ScrewRect.left() + 5));
		_ptCenter.setY(iter.value().Pos.y() - (m_ScrewRect.top() + 5));

		QPointF _ptDrawCenter;
		double	_dXLenth		= (iter.value().Pos.x() - m_ScrewRect.center().x());
		double	_dXBoardWidth	= ((m_ScrewRect.width() + 20) / 2.0);
		double	_dXScale		= (m_ScrewRect.width() + 20)  / width();

		double	_dYLenth		= (iter.value().Pos.y() - m_ScrewRect.center().y());
		double	_dYBoardWidth	= ((m_ScrewRect.height() + 20) / 2.0);
		double	_dYScale		= (m_ScrewRect.height() + 20)  / height();
		double	_dScale			= qMax<double>(_dXScale, _dYScale);

		_ptDrawCenter.setX(_dXLenth / _dScale/*_dXBoardWidth  * width() / 2.0*/ + width() /2.0 );
		_ptDrawCenter.setY(_dYLenth / _dScale/*_dYBoardWidth  * height() / 2.0*/ + height() / 2.0);
		_ptDrawCenter.setY(height() - _ptDrawCenter.y());

		//_ptDrawCenter.setX(_dXLenth / _dXBoardWidth  * width() / 2.0 + width() / 2.0);
		//_ptDrawCenter.setY(_dYLenth / _dYBoardWidth  * height() / 2.0 + height() / 2.0);

		for (auto Coloriter = m_ScrewColor.begin(); Coloriter != m_ScrewColor.end(); Coloriter++) {
			if (iter.value().iScrewStatus == Coloriter.key())	{
				painter.setBrush(QBrush(Coloriter.value().color));
				painter.drawEllipse(_ptDrawCenter, 20, 20);
			}
		}
		painter.setPen(QPen(QColor(0, 0, 0)));

		{	
			int textWidth = painter.fontMetrics().horizontalAdvance(QString::number(iter.key()));
			int itextHeight = painter.fontMetrics().height();
			QRect rect((_ptDrawCenter.x() - textWidth / 2.0), _ptDrawCenter.y() - itextHeight / 2.0, textWidth, itextHeight);
			painter.drawText(rect, Qt::AlignCenter, QString::number(iter.key() + 1));
		}
		//{
		//	QString _AxisStr = QString("%1,%2").arg( QString::number(iter.value().Pos.x(),'f',2) ).arg( QString::number(iter.value().Pos.y(), 'f', 2) ) ;
		//	int textWidth = painter.fontMetrics().horizontalAdvance(_AxisStr);
		//	int itextHeight = painter.fontMetrics().height();
		//	QRect rect((_ptDrawCenter.x() + textWidth / 2.0), _ptDrawCenter.y() - itextHeight / 2.0, textWidth, itextHeight);
		//	if (rect.right() > width()){
		//		rect.setLeft((_ptDrawCenter.x() - textWidth / 2.0) - textWidth);
		//	}
		//	if (rect.bottom() > height()) {
		//		rect.setBottom((_ptDrawCenter.y() - itextHeight / 2.0) - itextHeight);
		//	}
		//	painter.drawText(rect, Qt::AlignCenter, _AxisStr);
		//}


		int _iIndex = 0;
		for (auto Coloriter = m_ScrewColor.begin(); Coloriter != m_ScrewColor.end(); Coloriter++) {
			int textWidth	= painter.fontMetrics().horizontalAdvance(Coloriter.value().strColorName);
			int itextHeight = painter.fontMetrics().height();

			painter.setBrush(QBrush(Coloriter.value().color));
			QRect rect(width() - itextHeight, _iIndex * (itextHeight + 4), itextHeight, itextHeight);
			painter.drawRect(rect);

			QRect _rect = rect;
			_rect.setLeft(rect.left() - textWidth  - 10);
			_rect.setWidth(textWidth);
			_rect.setHeight(itextHeight);

			painter.drawText(_rect, Qt::AlignCenter, Coloriter.value().strColorName);

			_iIndex++;
		}
	}
	painter.setPen(QPen(QColor(0, 0, 0), 10));
	int textWidth = painter.fontMetrics().horizontalAdvance(m_strTitle);
	int itextHeight = painter.fontMetrics().height();
	QRect rect((width() - textWidth) / 2.0, 2, textWidth, itextHeight );
	painter.drawText(rect, Qt::AlignCenter, m_strTitle);
}

void ScrewWidget::resizeEvent(QResizeEvent * ev)
{
	QWidget::resizeEvent(ev);

}
