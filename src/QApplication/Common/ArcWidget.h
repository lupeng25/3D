#pragma once

#include <QWidget>
#include <QPainter>
#include <QMap>

struct PieData
{
	QString name;   //名称
	int num;        //数量
	QColor color;   //颜色

	PieData(QString name, int num, QColor color)
	{
		this->name = name;
		this->num = num;
		this->color = color;
	}
	PieData() {
		name = "";
		num = 3;
		color = QColor(255, 0, 0);
	}
};

class ArcWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ArcWidget(QWidget *parent = 0);
	~ArcWidget();


public:
	void initWidget();

	//圆环图各参数函数接口
	void setRadius(int radius);
	void setInnerWidth(int width);
	void setCenter(QPoint center);
	void setStartAngle(qreal startAngle);
	void setTextDistance(int textDistance);
	void setPieData(QVector<PieData> vData);
	void setTextTitle(QString strText);

	void refreshChart();
protected:
	void paintEvent(QPaintEvent *ev);
	void resizeEvent(QResizeEvent *ev);

private:

private:
	int m_radius;         //外圆半径
	int m_innerWidth;     //圆环内径
	QPoint m_center;      //圆心坐标
	qreal m_startAngle;   //圆环绘制起点
	int m_textDistance;   //文本与圆心的距离
	qreal m_totality;     //总数
	QVector<PieData> m_vData;   //数据容
	QString	m_strTitle;
};