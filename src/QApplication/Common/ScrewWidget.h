#pragma once

#include <QWidget>
#include <QPainter>
#include <QMap>
#include <QColor>
struct ScrewColor
{
	QColor color;
	QString	strColorName;
	void Rst() {
		strColorName = "";
		color = QColor(Qt::yellow);
	}
	ScrewColor() {
		Rst();
	}
	ScrewColor(QColor col, QString name) {
		color = col;
		strColorName = name;
	}
};
struct ScrewPos
{
	int	iScrewStatus;
	QPointF Pos;
	void Rst() {
		iScrewStatus	= 0;
		Pos				= QPointF();
	}
};

class ScrewWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScrewWidget(QWidget *parent = 0);
	~ScrewWidget();

public:
	void initWidget();
	void ClearScrewPos();
	void SetScrewPos(int index, double posX, double posY);


	void ClearScrewStatus();
	void SetScrewPosStatus(int index, int istatus);
	//void initWidget();

	//圆环图各参数函数接口
	void setTextTitle(QString strText);
	void refreshChart();
protected:
	void paintEvent(QPaintEvent *ev);
	void resizeEvent(QResizeEvent *ev);

private:
	QMap<int,ScrewPos>	m_vecScrewPos;
	QMap<int, ScrewColor>	m_ScrewColor;
private:
	QRectF				m_ScrewRect;
	QString	m_strTitle;
};