#include "ArcWidget.h"
#include <QtMath>

ArcWidget::ArcWidget(QWidget *parent)
	: QWidget(parent)
{
	this->resize(600, 600);
	initWidget();
}

ArcWidget::~ArcWidget()
{

}

//初始化界面
void ArcWidget::initWidget()
{
	//初始化变量
	m_radius = 0;
	m_innerWidth = 0;
	m_center = QPoint(0, 0);
	m_startAngle = 0;
	m_textDistance = 0;
	m_totality = 0;
	setInnerWidth(10);
	m_vData.clear();
	refreshChart();
}

//设置外圆半径
void ArcWidget::setRadius(int radius)
{
	m_radius = radius;
}

//设置圆环内径
void ArcWidget::setInnerWidth(int width)
{
	m_innerWidth = width;
}

//设置圆心
void ArcWidget::setCenter(QPoint center)
{
	m_center = center;
}

//设置圆环绘制起点
void ArcWidget::setStartAngle(qreal startAngle)
{
	m_startAngle = startAngle;
}

//设置文本与圆心的距离
void ArcWidget::setTextDistance(int textDistance)
{
	m_textDistance = textDistance;
}

//设置饼图数据
void ArcWidget::setPieData(QVector<PieData> vData)
{
	//获取数据
	m_vData = vData;

	//获取总数
	m_totality = 0;
	for (int i = 0; i<m_vData.size(); i++)
	{
		m_totality += m_vData[i].num;
	}
	this->update();
}

void ArcWidget::setTextTitle(QString strText)
{
	m_strTitle = strText;
}

//更新饼图
void ArcWidget::refreshChart()
{
	//设置圆环图各区域数据名称及颜色
	QVector<PieData> vData;
	QColor m_colors[5] = { QColor("#0286FF"),QColor("#FFA784"),QColor("#7EBBFF"),QColor("#DFEEFF"),QColor("#85D9FD") };
	for (int i = 0; i<3; i++)
	{
		int	leNum = i;   //找到对应lineedit
		if (leNum != 0)
		{
			int num = leNum;
			if (num > 0)   //过滤输入为空或0的数据
			{
				PieData data = PieData(QString("%1").arg(num), num, m_colors[i]);
				vData.push_back(data);
			}
		}
	}
	this->setPieData(vData);
}

void ArcWidget::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);   //抗锯齿

	//绘制背景
	painter.setBrush(Qt::white);
	painter.setPen(Qt::NoPen);   //去除背景边框
	painter.drawRoundedRect(0, 0, width(), height(), 8, 8);
	//painter.drawRoundedRect(10, 10, 360, 360, 8, 8);

	//绘制饼图
	qreal startAngle = m_startAngle;   //绘制起点
	qreal spanAngle = 0;   //各区域占比

	double _iAllNum = 0;
	for (int i = 0; i< m_vData.size(); i++)
	{
		painter.setPen(m_vData[i].color);
		painter.setBrush(m_vData[i].color);
		if (m_totality)   //防止总数为0
		{
			spanAngle = m_vData[i].num * 360 / m_totality;
		}
		_iAllNum += m_vData[i].num;
		if (!(startAngle < 0.001 
			&& startAngle > -0.001
			&& spanAngle < 0.001
			&& spanAngle > -0.001))	{
			painter.drawPie(m_center.x() - m_radius, m_center.y() - m_radius, m_radius * 2, m_radius * 2, startAngle * 16, spanAngle * 16);
		}
		startAngle += spanAngle;
	}

	//绘制区域名称和占比
	startAngle = m_startAngle;
	spanAngle = 0;
	for (int i = 0; i< m_vData.size(); i++)
	{
		painter.setPen(QColor("#333333"));
		painter.setFont(QFont("STSongti-SC-Bold, STSongti-SC", 10));
		if (m_totality)
		{
			spanAngle = m_vData[i].num * 360 / m_totality;
		}
		int textAngle = startAngle + spanAngle / 2;
		QString text = QString("%1").arg(m_vData[i].name)/*.arg(QString::number(m_vData[i].num / _iAllNum * 100,'f',2))*/;
		int textWidth = painter.fontMetrics().horizontalAdvance(text);
		int textHeight = painter.fontMetrics().height();
		int textX = m_center.x() + m_textDistance * qCos(textAngle * M_PI / 180) - textWidth / 2;
		int textY = m_center.y() - m_textDistance * qSin(textAngle * M_PI / 180) + textHeight / 2;
		startAngle += spanAngle;

		//绘制文本
		QRect rect(textX, textY - textHeight, textWidth + 10, textHeight * 2);
		if (!(startAngle < 0.001
			&& startAngle > -0.001
			&& spanAngle < 0.001
			&& spanAngle > -0.001)) {
			painter.drawText(rect, Qt::AlignCenter, text);
		}

		//绘制连接线,文本要靠近对应区域,需要修改连接线终点位置
		painter.setPen(m_vData[i].color);
		int lineStartX = m_center.x() + (m_radius - 10) * qCos(textAngle * M_PI / 180);
		int lineStartY = m_center.y() - (m_radius - 10) * qSin(textAngle * M_PI / 180);
		int lineEndX = 0;
		int lineEndY = 0;
		if (textX < lineStartX)   //文本在左边
		{
			//可自行根据实际进行位置偏移的修改
			lineEndX = textX + textWidth / 2;
			if (textY < lineStartY)   //文本在上边
			{
				lineEndY = textY + textHeight + 5;
			}
			else
			{
				lineEndY = textY - textHeight - 5;
			}
		}
		else
		{
			lineEndX = textX + textWidth / 2;
			if (textY < lineStartY)
			{
				lineEndY = textY;
			}
			else
			{
				lineEndY = textY - textHeight - 5;
			}
		}
		//painter.drawLine(lineStartX, lineStartY, lineEndX, lineEndY);

		//绘制终点
		painter.setPen(QPen(m_vData[i].color, 5));
		if (!(startAngle < 0.001
			&& startAngle > -0.001
			&& spanAngle < 0.001
			&& spanAngle > -0.001)) {
			painter.drawPoint(lineEndX, lineEndY);
		}

		//将终点设为空心圆
		//painter.drawEllipse(QPoint(lineEndX,lineEndY),5,5);
		//painter.setPen(QPen(QColor("#FFFFFF"),1));
		//painter.setBrush(QColor("#FFFFFF"));
		//painter.drawEllipse(QPoint(lineEndX,lineEndY),3,3);
	}

	//绘制内圆,将饼图变为圆环
	painter.setPen(QPen(QColor("#FFFFFF"), 10));
	painter.setBrush(QColor("#FFFFFF"));
	painter.drawEllipse(m_center, m_innerWidth, m_innerWidth);
	//	

	painter.setPen(QPen(QColor(0, 0, 0), 10));
	int textWidth = painter.fontMetrics().horizontalAdvance(m_strTitle);
	int itextHeight = painter.fontMetrics().height();
	QRect rect((width() - textWidth) / 2.0, 10, textWidth + 10, itextHeight * 2);
	painter.drawText(rect, Qt::AlignCenter, m_strTitle);

	painter.setPen(QPen(QColor(0,0,0), 5));
	for (int i = 0; i < m_vData.size(); i++)
	{
		QString text;
		if (_iAllNum == 0) 	{
			text = QString("%1 %2%").arg(m_vData[i].name).arg(QString::number(0, 'f', 2));
		}
		else {
			text = QString("%1 %2%").arg(m_vData[i].name).arg(QString::number(abs(m_vData[i].num / _iAllNum * 100.0), 'f', 2));
		}
		int textWidth = painter.fontMetrics().horizontalAdvance(text);
		int textHeight = painter.fontMetrics().height() ;
		QRect rect(	10 , itextHeight * 2 + textHeight * i + 10, textWidth + 10, textHeight );
		painter.drawText(rect, Qt::AlignCenter, text);
	}
}

void ArcWidget::resizeEvent(QResizeEvent * ev)
{
	QWidget::resizeEvent(ev);
	m_center.setX(width() / 2);
	m_center.setY(height() / 2);
	m_radius = width() / 3;
	m_innerWidth = 0;
	setTextDistance(m_radius  / 2.0);

}
