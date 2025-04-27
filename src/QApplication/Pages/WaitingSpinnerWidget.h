#pragma once

#include <QDialog>
#include <QWidget>
#include <QTimer>
#include <QColor>

class WaitingSpinnerWidget : public QWidget
{
	Q_OBJECT
public:
	/*! Constructor for "standard" widget behaviour - use this
	* constructor if you wish to, e.g. embed your widget in another. */
	WaitingSpinnerWidget(QWidget *parent = 0,
		bool centerOnParent = true,
		bool disableParentWhenSpinning = false);

	/*! Constructor - use this constructor to automatically create a modal
	* ("blocking") spinner on top of the calling widget/window.  If a valid
	* parent widget is provided, "centreOnParent" will ensure that
	* QtWaitingSpinner automatically centres itself on it, if not,
	* "centreOnParent" is ignored. */
	WaitingSpinnerWidget(Qt::WindowModality modality,
		QWidget *parent = 0,
		bool centerOnParent = true,
		bool disableParentWhenSpinning = false);

public slots:
	void start();
	void stop();
public:
	void setColor(QColor color);
	void setRoundness(qreal roundness);
	void setMinimumTrailOpacity(qreal minimumTrailOpacity);
	void setTrailFadePercentage(qreal trail);
	void setRevolutionsPerSecond(qreal revolutionsPerSecond);
	void setNumberOfLines(int lines);
	void setLineLength(int length);
	void setLineWidth(int width);
	void setInnerRadius(int radius);
	void setText(QString text);
	QColor color();
	qreal roundness();
	qreal minimumTrailOpacity();
	qreal trailFadePercentage();
	qreal revolutionsPersSecond();
	int numberOfLines();
	int lineLength();
	int lineWidth();
	int innerRadius();
	bool isSpinning() const;
private slots:
	void rotate();
protected:
	void paintEvent(QPaintEvent *paintEvent);

private:
	static int lineCountDistanceFromPrimary(int current, int primary,int totalNrOfLines);
	static QColor currentLineColor(int distance, int totalNrOfLines,
		qreal trailFadePerc, qreal minOpacity,QColor color);

	void initialize();
	void updateSize();
	void updateTimer();
	void updatePosition();

private:
	QColor  _color;
	qreal   _roundness; // 0..100
	qreal   _minimumTrailOpacity;
	qreal   _trailFadePercentage;
	qreal   _revolutionsPerSecond;
	int     _numberOfLines;
	int     _lineLength;
	int     _lineWidth;
	int     _innerRadius;
private:
	WaitingSpinnerWidget(const WaitingSpinnerWidget&);
	WaitingSpinnerWidget& operator=(const WaitingSpinnerWidget&);

	QTimer *_timer{ nullptr };
	bool    _centerOnParent;
	bool    _disableParentWhenSpinning;
	int     _currentCounter;
	bool    _isSpinning;
};
//LoadingAnimWidget.h
#ifndef LOADINGANIMWIDGET_H
#define LOADINGANIMWIDGET_H
#include <QPropertyAnimation>
#include <QWidget>
class LoadingAnimBase :public QWidget
{
	Q_OBJECT
public:
	LoadingAnimBase(QWidget* parent = nullptr);
	virtual ~LoadingAnimBase();
public slots:

	virtual void exec();
	virtual void start();
	virtual void stop();
protected:
	QPropertyAnimation mAnim;
};

class SpotsShrinkCircle :public LoadingAnimBase {//32个球均匀分布在四周,绕着中心转圈,球越来越小
	Q_OBJECT
	Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
	explicit SpotsShrinkCircle(QWidget * parent = nullptr);
	qreal angle()const;
	void setAngle(qreal an);
protected:
	void paintEvent(QPaintEvent *event);
signals:

public slots :

private:
	qreal mAngle;
};

class SpotsCircle : public LoadingAnimBase//8个紧挨着的球绕着中心转圈,每个球大小一样
{
	Q_OBJECT
	Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
	explicit SpotsCircle(QWidget *parent = nullptr);

	qreal angle()const;
	void setAngle(qreal an);
protected:
	void paintEvent(QPaintEvent *event);

public slots:

private:
	qreal mAngle;
};

class StickCircle :public LoadingAnimBase//12根小棒分布周围,三根小棒是深色的,其他都是淡色的,深色小棒循环占取淡色小棒的位置
{
	Q_OBJECT
	Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
	explicit StickCircle(QWidget* parent = nullptr);
	qreal angle()const;
	void setAngle(qreal an);
	void setShrinkStick(bool shrink);
protected:
	void paintEvent(QPaintEvent *event);

public slots:
	
private:
	qreal mAngle;
	bool mShrink;
};

class TubeCircle :public LoadingAnimBase //一根管子绕着中心旋转
{
	Q_OBJECT
	Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
	explicit TubeCircle(QWidget* parent = nullptr);
	qreal angle()const;
	void setAngle(qreal an);
protected:
	void paintEvent(QPaintEvent *event);

public slots:
	
private:
	qreal mAngle;
};

class TextCircle :public LoadingAnimBase { //一串文本绕着中心旋转
	Q_OBJECT
	Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
	explicit TextCircle(const QString & txt = "Loading...", QWidget* parent = nullptr);
	qreal angle()const;
	void setAngle(qreal an);
	void setText(const QString & txt = "Loading...");
protected:
	void paintEvent(QPaintEvent* event);
private:
	QString mText;
	qreal mAngle;
};

class TubeBallCircle :public LoadingAnimBase //一根封闭的管子,中间有个小球沿着管子滚动
{
	Q_OBJECT
	Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
	explicit TubeBallCircle(QWidget* parent = nullptr);
	qreal angle()const;
	void setAngle(qreal an);
protected:
	void paintEvent(QPaintEvent *event);
public slots:
	
private:
	qreal mAngle;
};

#endif // LOADINGANIMWIDGET_H
