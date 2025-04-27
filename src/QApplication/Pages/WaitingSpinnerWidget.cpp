#include "WaitingSpinnerWidget.h"// Standard includes
#include <cmath>
#include <algorithm>
#include <QPainter>
#include <QTimer>
// Qt includes

WaitingSpinnerWidget::WaitingSpinnerWidget(QWidget *parent,
	bool centerOnParent,
	bool disableParentWhenSpinning)
	: QWidget(parent),
	_centerOnParent(centerOnParent),
	_disableParentWhenSpinning(disableParentWhenSpinning) {

	initialize();
	setWindowFlag(Qt::FramelessWindowHint); // 设置窗口无边框
}

WaitingSpinnerWidget::WaitingSpinnerWidget(Qt::WindowModality modality,
	QWidget *parent,
	bool centerOnParent,
	bool disableParentWhenSpinning)
	: QWidget(parent, Qt::Dialog | Qt::FramelessWindowHint),
	_centerOnParent(centerOnParent),
	_disableParentWhenSpinning(disableParentWhenSpinning) {
	initialize();

	// We need to set the window modality AFTER we've hidden the
	// widget for the first time since changing this property while
	// the widget is visible has no effect.
	setWindowModality(modality);
	setAttribute(Qt::WA_TranslucentBackground);
}

void WaitingSpinnerWidget::initialize() {
	_color = Qt::black;
	_roundness = 100.0;
	_minimumTrailOpacity = 3.14159265358979323846;
	_trailFadePercentage = 80.0;
	_revolutionsPerSecond = 1.57079632679489661923;
	_numberOfLines = 20;
	_lineLength = 10;
	_lineWidth = 2;
	_innerRadius = 10;
	_currentCounter = 0;
	_isSpinning = false;

	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(rotate()));
	updateSize();
	updateTimer();
	hide();
}

void WaitingSpinnerWidget::paintEvent(QPaintEvent *) {
	updatePosition();
	QPainter painter(this);
	painter.fillRect(this->rect(), Qt::transparent);
	painter.setRenderHint(QPainter::Antialiasing, true);

	if (_currentCounter >= _numberOfLines) {
		_currentCounter = 0;
	}

	painter.setPen(Qt::NoPen);
	for (int i = 0; i < _numberOfLines; ++i) {
		painter.save();
		painter.translate(_innerRadius + _lineLength,
			_innerRadius + _lineLength);
		qreal rotateAngle =
			static_cast<qreal>(360 * i) / static_cast<qreal>(_numberOfLines);
		painter.rotate(rotateAngle);
		painter.translate(_innerRadius, 0);
		int distance =
			lineCountDistanceFromPrimary(i, _currentCounter, _numberOfLines);
		QColor color =
			currentLineColor(distance, _numberOfLines, _trailFadePercentage,
				_minimumTrailOpacity, _color);
		painter.setBrush(color);
		// TODO improve the way rounded rect is painted
		painter.drawRoundedRect(
			QRect(0, -_lineWidth / 2, _lineLength, _lineWidth), _roundness,
			_roundness, Qt::RelativeSize);
		painter.restore();
	}
}

void WaitingSpinnerWidget::start() {
	updatePosition();
	_isSpinning = true;
	show();

	if (parentWidget() && _disableParentWhenSpinning) {
		parentWidget()->setEnabled(false);
	}

	if (!_timer->isActive()) {
		_timer->start();
		_currentCounter = 0;
	}
}

void WaitingSpinnerWidget::stop() {
	_isSpinning = false;
	hide();

	if (parentWidget() && _disableParentWhenSpinning) {
		parentWidget()->setEnabled(true);
	}

	if (_timer->isActive()) {
		_timer->stop();
		_currentCounter = 0;
	}
}

void WaitingSpinnerWidget::setNumberOfLines(int lines) {
	_numberOfLines = lines;
	_currentCounter = 0;
	updateTimer();
}

void WaitingSpinnerWidget::setLineLength(int length) {
	_lineLength = length;
	updateSize();
}

void WaitingSpinnerWidget::setLineWidth(int width) {
	_lineWidth = width;
	updateSize();
}

void WaitingSpinnerWidget::setInnerRadius(int radius) {
	_innerRadius = radius;
	updateSize();
}

QColor WaitingSpinnerWidget::color() {
	return _color;
}

qreal WaitingSpinnerWidget::roundness() {
	return _roundness;
}

qreal WaitingSpinnerWidget::minimumTrailOpacity() {
	return _minimumTrailOpacity;
}

qreal WaitingSpinnerWidget::trailFadePercentage() {
	return _trailFadePercentage;
}

qreal WaitingSpinnerWidget::revolutionsPersSecond() {
	return _revolutionsPerSecond;
}

int WaitingSpinnerWidget::numberOfLines() {
	return _numberOfLines;
}

int WaitingSpinnerWidget::lineLength() {
	return _lineLength;
}

int WaitingSpinnerWidget::lineWidth() {
	return _lineWidth;
}

int WaitingSpinnerWidget::innerRadius() {
	return _innerRadius;
}

bool WaitingSpinnerWidget::isSpinning() const {
	return _isSpinning;
}

void WaitingSpinnerWidget::setRoundness(qreal roundness) {
	_roundness = std::max(0.0, std::min(100.0, roundness));
}

void WaitingSpinnerWidget::setColor(QColor color) {
	_color = color;
}

void WaitingSpinnerWidget::setRevolutionsPerSecond(qreal revolutionsPerSecond) {
	_revolutionsPerSecond = revolutionsPerSecond;
	updateTimer();
}

void WaitingSpinnerWidget::setTrailFadePercentage(qreal trail) {
	_trailFadePercentage = trail;
}

void WaitingSpinnerWidget::setMinimumTrailOpacity(qreal minimumTrailOpacity) {
	_minimumTrailOpacity = minimumTrailOpacity;
}

void WaitingSpinnerWidget::rotate() {
	++_currentCounter;
	if (_currentCounter >= _numberOfLines) {
		_currentCounter = 0;
	}
	update();
}

void WaitingSpinnerWidget::updateSize() {
	int size = (_innerRadius + _lineLength) * 2;
	setFixedSize(size, size);
}

void WaitingSpinnerWidget::updateTimer() {
	_timer->setInterval(1000 / (_numberOfLines * _revolutionsPerSecond));
}

void WaitingSpinnerWidget::updatePosition() {
	if (parentWidget() && _centerOnParent) {
		move(parentWidget()->width() / 2 - width() / 2,
			parentWidget()->height() / 2 - height() / 2);
	}
}

int WaitingSpinnerWidget::lineCountDistanceFromPrimary(int current, int primary,
	int totalNrOfLines) {
	int distance = primary - current;
	if (distance < 0) {
		distance += totalNrOfLines;
	}
	return distance;
}

QColor WaitingSpinnerWidget::currentLineColor(int countDistance, int totalNrOfLines,
	qreal trailFadePerc, qreal minOpacity,
	QColor color) {
	if (countDistance == 0) {
		return color;
	}
	const qreal minAlphaF = minOpacity / 100.0;
	int distanceThreshold =
		static_cast<int>(ceil((totalNrOfLines - 1) * trailFadePerc / 100.0));
	if (countDistance > distanceThreshold) {
		color.setAlphaF(minAlphaF);
	}
	else {
		qreal alphaDiff = color.alphaF() - minAlphaF;
		qreal gradient = alphaDiff / static_cast<qreal>(distanceThreshold + 1);
		qreal resultAlpha = color.alphaF() - gradient * countDistance;

		// If alpha is out of bounds, clip it.
		resultAlpha = std::min(1.0, std::max(0.0, resultAlpha));
		color.setAlphaF(resultAlpha);
	}
	return color;
}

//LoadingAnimWidget.cpp
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

LoadingAnimBase::LoadingAnimBase(QWidget* parent) :QWidget(parent) {}

LoadingAnimBase::~LoadingAnimBase() {}

void LoadingAnimBase::exec() {
	if (mAnim.state() == QAbstractAnimation::Stopped) {
		start();
	}
	else {
		stop();
	}
}

void LoadingAnimBase::start() {
	mAnim.setStartValue(0);
	mAnim.setEndValue(360);
	mAnim.start();
}

void LoadingAnimBase::stop() {
	mAnim.stop();
}

SpotsCircle::SpotsCircle(QWidget *parent) : LoadingAnimBase(parent)
{
	mAnim.setPropertyName("angle");
	mAnim.setTargetObject(this);
	mAnim.setDuration(2000);
	mAnim.setLoopCount(-1);//run forever
	mAnim.setEasingCurve(QEasingCurve::Linear);
	resize(200, 200);
	setFixedSize(200, 200);
	mAngle = 0;
}

void SpotsCircle::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);
	int x = width();
	int y = height();

	painter.setPen(Qt::NoPen);
	painter.translate(x / 2, y / 2);
	QColor color = QColor("black");
	qreal alpha = color.alphaF();
	painter.rotate(mAngle);
	for (int i = 0; i<16; ++i) {//16个小球
		qreal rw = 1.0 / 8 * x;
		qreal r = rw / 2;
		painter.setBrush(QBrush(color));
		alpha *= 0.8;
		color.setAlphaF(alpha);
		painter.drawEllipse(0, x / 2 - rw, r, r);
		auto transform = painter.transform();
		transform.rotate(-2);
		painter.setTransform(transform);
	}
}

qreal SpotsCircle::angle()const {
	return mAngle;
}

void SpotsCircle::setAngle(qreal an) {
	mAngle = an;
	update();
}

SpotsShrinkCircle::SpotsShrinkCircle(QWidget *parent) : LoadingAnimBase(parent)
{
	mAnim.setPropertyName("angle");
	mAnim.setTargetObject(this);
	mAnim.setDuration(3000);
	mAnim.setLoopCount(-1);//run forever
	mAnim.setEasingCurve(QEasingCurve::Linear);
	resize(200, 200);
	setFixedSize(200, 200);
	mAngle = 0;
}

void SpotsShrinkCircle::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);
	int x = width();
	int y = height();
	qreal ang = mAngle;

	painter.setPen(Qt::NoPen);
	painter.translate(x / 2, y / 2);
	QColor color = QColor("black");
	qreal alpha = color.alphaF();
	painter.rotate(ang);
	qreal rw = 1.0 / 7 * x;
	qreal r = rw / 2;
	for (int i = 0; i<12; ++i) {
		painter.setBrush(QBrush(color));
		alpha *= 0.9;
		color.setAlphaF(alpha);
		painter.drawEllipse(0, x / 2 - rw, r, r);
		r *= 0.95;
		auto transform = painter.transform();
		transform.rotate(-360 / 18);
		painter.setTransform(transform);
	}
}

qreal SpotsShrinkCircle::angle()const {
	return mAngle;
}

void SpotsShrinkCircle::setAngle(qreal an) {
	mAngle = an;
	update();
}

void StickCircle::setShrinkStick(bool shrink) {
	mShrink = shrink;
}

StickCircle::StickCircle(QWidget *parent) : LoadingAnimBase(parent)
{
	mAnim.setPropertyName("angle");
	mAnim.setTargetObject(this);
	mAnim.setDuration(2000);
	mAnim.setLoopCount(-1);//run forever
	mAnim.setEasingCurve(QEasingCurve::Linear);
	resize(200, 200);
	setFixedSize(200, 200);
	mAngle = 0;
	mShrink = false;
}

void StickCircle::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);
	int x = width();
	int y = height();
	qreal ang = mAngle;

	painter.setPen(Qt::NoPen);
	painter.translate(x / 2, y / 2);
	QColor lightColor = QColor("black");
	lightColor.setAlphaF(0.1);

	const int stickw = 10;//小棍子的宽度
	qreal stickl = x / 4;//小棍子的长度

	QColor highlight = QColor("black");
	const int startIdx = ang / 30;//开头的深色小棍子的索引号
	const int arr[14] = { 0,1,2,3,4,5,6,7,8,9,10,11,0,1 };
	QList<int> hightlightList{ arr[startIdx],arr[startIdx + 1],arr[startIdx + 2] };
	QList<qreal> alphaList{ 0.5,0.75,0.99 };//三根深色的小棍子的透明度

	for (int i = 0; i < 12; ++i) {
		if (hightlightList.contains(i)) {
			auto hc = highlight;
			hc.setAlphaF(alphaList[0]);
			painter.setBrush(QBrush(hc));
			alphaList.pop_front();
		}
		else {
			painter.setBrush(QBrush(lightColor));
		}
		painter.drawRoundedRect(QRectF(-stickw / 2, 0.375*y - stickl / 2 - 1, stickw, stickl), stickw / 2, stickw / 2);
		auto transform = painter.transform();
		transform.rotate(30);
		painter.setTransform(transform);
	}
}

qreal StickCircle::angle()const {
	return mAngle;
}

void StickCircle::setAngle(qreal an) {
	mAngle = an;
	update();
}

TextCircle::TextCircle(const QString & txt, QWidget* parent) :mText(txt), LoadingAnimBase(parent) {
	mAnim.setPropertyName("angle");
	mAnim.setTargetObject(this);
	mAnim.setDuration(2000);
	mAnim.setLoopCount(-1);//run forever
	mAnim.setEasingCurve(QEasingCurve::Linear);
	//resize(200,200);
	setFixedSize(200, 200);
	mAngle = 0;
}

qreal TextCircle::angle()const { 
	return mAngle; 
}

void TextCircle::setAngle(qreal an) {
	mAngle = an;
	update();
}

void TextCircle::setText(const QString & txt)
{
	mText = txt;
}

void TextCircle::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	int x = width();
	int y = height();

	QPen pen("black");
	pen.setWidth(4);
	painter.setPen(pen);

	painter.translate(x / 2, y / 2);
	painter.rotate(mAngle);

	for (int i = 0; i < mText.length(); ++i) {
		const QString s = QString(mText[i]);
		painter.drawText(QRect(-10, y / 2 - 22, 20, 20), Qt::AlignTop, s);//中下方画一个字符
		painter.rotate(-8);//每个字符之间间距8度
	}
}

TubeCircle::TubeCircle(QWidget *parent) : LoadingAnimBase(parent)
{
	mAnim.setPropertyName("angle");
	mAnim.setTargetObject(this);
	mAnim.setDuration(2000);
	mAnim.setLoopCount(-1);//run forever
	mAnim.setEasingCurve(QEasingCurve::Linear);
	resize(200, 200);
	setFixedSize(200, 200);
	mAngle = 0;
}

void TubeCircle::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);
	int x = width();
	int y = height();
	qreal ang = mAngle;

	QPen pen(QColor("black"));
	pen.setWidth(x / 20);
	pen.setCapStyle(Qt::RoundCap);
	painter.setPen(pen);
	painter.translate(x / 2, y / 2);
	painter.rotate(ang);
	const int adjust = x / 20;
	auto rect = this->rect().adjusted(adjust, adjust, -adjust, -adjust);
	rect.translate(-x / 2, -y / 2);
	painter.drawArc(rect, 30 * 16, -300 * 16);
}

qreal TubeCircle::angle()const {
	return mAngle;
}

void TubeCircle::setAngle(qreal an) {
	mAngle = an;
	update();
}

TubeBallCircle::TubeBallCircle(QWidget *parent) : LoadingAnimBase(parent)
{
	mAnim.setPropertyName("angle");
	mAnim.setTargetObject(this);
	mAnim.setDuration(2000);
	mAnim.setLoopCount(-1);//run forever
	mAnim.setEasingCurve(QEasingCurve::Linear);
	resize(200, 200);
	setFixedSize(200, 200);
	mAngle = 0;
}

void TubeBallCircle::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);
	int x = width();
	int y = height();
	qreal ang = mAngle;

	QColor co("gray");
	co.setAlphaF(0.3);
	QPen pen(co);
	const int penw = x / 8;
	pen.setWidth(penw);
	pen.setCapStyle(Qt::RoundCap);
	painter.setPen(pen);
	painter.translate(x / 2, y / 2);
	painter.rotate(ang);
	const int adjust = penw / 2;
	auto rect = this->rect().adjusted(adjust, adjust, -adjust, -adjust);
	rect.translate(-x / 2, -y / 2);
	painter.drawArc(rect, 30 * 16, 360 * 16);//画轨道

	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor("black"));
	painter.drawEllipse(QPoint(0, y / 2 - penw / 2), penw / 2, penw / 2);//画小球
}

qreal TubeBallCircle::angle()const {
	return mAngle;
}

void TubeBallCircle::setAngle(qreal an) {
	mAngle = an;
	update();
}
