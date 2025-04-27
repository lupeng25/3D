#include "QDoubleSlider.h"
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

#include <QMouseEvent>
#include <QStylePainter>
#include <QStyleOptionSlider>

QDoubleSliderPrivate::QDoubleSliderPrivate() :
        lower(0),
        upper(100),
        lowerPos(0),
        upperPos(100),
        offset(0),
        position(0),
        lastPressed(QDoubleSlider::NoHandle),
        mainControl(QDoubleSlider::LowerHandle),
        lowerPressed(QStyle::SC_None),
        upperPressed(QStyle::SC_None),
        movement(QDoubleSlider::FreeMovement),
        firstMovement(false),
        blockTracking(false)
{
}

void QDoubleSliderPrivate::initStyleOption(QStyleOptionSlider* option, QDoubleSlider::SpanHandle handle) const
{
    const QDoubleSlider* p = q_ptr;
    p->initStyleOption(option);
    option->sliderPosition = (handle == QDoubleSlider::LowerHandle ? lowerPos : upperPos);
    option->sliderValue = (handle == QDoubleSlider::LowerHandle ? lower : upper);
}

int QDoubleSliderPrivate::pick(const QPoint& pt) const
{
    return q_ptr->orientation() == Qt::Horizontal ? pt.x() : pt.y();
}

int QDoubleSliderPrivate::pixelPosToRangeValue(int pos) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    int sliderMin = 0;
    int sliderMax = 0;
    int sliderLength = 0;
    const QSlider* p = q_ptr;
    const QRect gr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
    const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
    if (p->orientation() == Qt::Horizontal)
    {
        sliderLength = sr.width();
        sliderMin = gr.x();
        sliderMax = gr.right() - sliderLength + 1;
    }
    else
    {
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;
    }
    return QStyle::sliderValueFromPosition(p->minimum(), p->maximum(), pos - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

void QDoubleSliderPrivate::handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, QDoubleSlider::SpanHandle handle)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    QDoubleSlider* p = q_ptr;
    const QStyle::SubControl oldControl = control;
    control = p->style()->hitTestComplexControl(QStyle::CC_Slider, &opt, pos, p);
    const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
    if (control == QStyle::SC_SliderHandle)
    {
        position = value;
        offset = pick(pos - sr.topLeft());
        lastPressed = handle;
        p->setSliderDown(true);
        emit p->sliderPressed(handle);
    }
    if (control != oldControl)
        p->update(sr);
}

// 自写样式,设置渐变色
void QDoubleSliderPrivate::setupPainter(QPainter* painter, Qt::Orientation orientation, QRect rect) const
{
    QLinearGradient gradient;
    if (orientation == Qt::Horizontal)
        gradient = QLinearGradient(rect.left(), rect.top(), rect.right(), rect.top());
    else
        gradient = QLinearGradient(rect.left(), rect.top(), rect.left(), rect.bottom());

    gradient.setColorAt(0, QColor(231, 80, 229));
    gradient.setColorAt(1, QColor(7,208,255));
    painter->setBrush(gradient);
    painter->setPen(Qt::transparent);
}

void QDoubleSliderPrivate::setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const
{
    QColor highlight = q_ptr->palette().color(QPalette::Highlight);
    QLinearGradient gradient(x1, y1, x2, y2);
    gradient.setColorAt(0, highlight.dark(120));
    gradient.setColorAt(1, highlight.light(108));
    painter->setBrush(gradient);

    if (orientation == Qt::Horizontal)
        painter->setPen(QPen(highlight.dark(130), 0));
    else
        painter->setPen(QPen(highlight.dark(150), 0));
}

void QDoubleSliderPrivate::drawSpan(QStylePainter* painter, const QRect& rect) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    const QSlider* p = q_ptr;    

    // area
    QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
    if (opt.orientation == Qt::Horizontal)
        groove.adjust(0, 0, -1, 0);
    else
        groove.adjust(0, 0, 0, -1);

    // pen & brush
//    painter->setPen(QPen(p->palette().color(QPalette::Dark).light(110), 0));
//    if (opt.orientation == Qt::Horizontal)
//        setupPainter(painter, opt.orientation, groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
//    else
//        setupPainter(painter, opt.orientation, groove.left(), groove.center().y(), groove.right(), groove.center().y());

    // 相交得到的矩形宽度、高度少了1pixel,需要加上
    QRect rt = rect.intersected(groove);
    rt.adjust(0, 0, 1, 1);

    // 调用自写函数修改样式
    setupPainter(painter, opt.orientation, rt);

    // draw groove
    painter->drawRect(rt);
}

void QDoubleSliderPrivate::drawHandle(QStylePainter* painter, QDoubleSlider::SpanHandle handle) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    opt.subControls = QStyle::SC_SliderHandle;
    QStyle::SubControl pressed = (handle == QDoubleSlider::LowerHandle ? lowerPressed : upperPressed);
    if (pressed == QStyle::SC_SliderHandle)
    {
        opt.activeSubControls = pressed;
        opt.state |= QStyle::State_Sunken;
    }
    painter->drawComplexControl(QStyle::CC_Slider, opt);
}

void QDoubleSliderPrivate::triggerAction(QAbstractSlider::SliderAction action, bool main)
{
    int value = 0;
    bool no = false;
    bool up = false;
    const int min = q_ptr->minimum();
    const int max = q_ptr->maximum();
    const QDoubleSlider::SpanHandle altControl = (mainControl == QDoubleSlider::LowerHandle ? QDoubleSlider::UpperHandle : QDoubleSlider::LowerHandle);

    blockTracking = true;

    switch (action)
    {
    case QAbstractSlider::SliderSingleStepAdd:
        if ((main && mainControl == QDoubleSlider::UpperHandle) || (!main && altControl == QDoubleSlider::UpperHandle))
        {
            value = qBound(min, upper + q_ptr->singleStep(), max);
            up = true;
            break;
        }
        value = qBound(min, lower + q_ptr->singleStep(), max);
        break;
	case QAbstractSlider::SliderSingleStepSub: {
		if ((main && mainControl == QDoubleSlider::UpperHandle) || (!main && altControl == QDoubleSlider::UpperHandle))
		{
			value = qBound(min, upper - q_ptr->singleStep(), max);
			up = true;
			break;
		}
		value = qBound(min, lower - q_ptr->singleStep(), max);
	}	break;
    case QAbstractSlider::SliderToMinimum:
        value = min;
        if ((main && mainControl == QDoubleSlider::UpperHandle) || (!main && altControl == QDoubleSlider::UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderToMaximum:
        value = max;
        if ((main && mainControl == QDoubleSlider::UpperHandle) || (!main && altControl == QDoubleSlider::UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderMove:
        if ((main && mainControl == QDoubleSlider::UpperHandle) || (!main && altControl == QDoubleSlider::UpperHandle))
            up = true;
    case QAbstractSlider::SliderNoAction:
        no = true;
        break;
    default:
        qWarning("QDoubleSliderPrivate::triggerAction: Unknown action");
        break;
    }

    if (!no && !up)
    {
        if (movement == QDoubleSlider::NoCrossing)
            value = qMin(value, upper);
        else if (movement == QDoubleSlider::NoOverlapping)
            value = qMin(value, upper - 1);

        if (movement == QDoubleSlider::FreeMovement && value > upper)
        {
            swapControls();
            q_ptr->setUpperPosition(value);
        }
        else
        {
            q_ptr->setLowerPosition(value);
        }
    }
    else if (!no)
    {
        if (movement == QDoubleSlider::NoCrossing)
            value = qMax(value, lower);
        else if (movement == QDoubleSlider::NoOverlapping)
            value = qMax(value, lower + 1);

        if (movement == QDoubleSlider::FreeMovement && value < lower)
        {
            swapControls();
            q_ptr->setLowerPosition(value);
        }
        else
        {
            q_ptr->setUpperPosition(value);
        }
    }

    blockTracking = false;
    q_ptr->setLowerValue(lowerPos);
    q_ptr->setUpperValue(upperPos);
}

void QDoubleSliderPrivate::swapControls()
{
    qSwap(lower, upper);
    qSwap(lowerPressed, upperPressed);
    lastPressed = (lastPressed == QDoubleSlider::LowerHandle ? QDoubleSlider::UpperHandle : QDoubleSlider::LowerHandle);
    mainControl = (mainControl == QDoubleSlider::LowerHandle ? QDoubleSlider::UpperHandle : QDoubleSlider::LowerHandle);
}

void QDoubleSliderPrivate::updateRange(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    // setSpan() takes care of keeping span in range
    q_ptr->setSpan(lower, upper);
}

void QDoubleSliderPrivate::movePressedHandle()
{
    switch (lastPressed)
    {
        case QDoubleSlider::LowerHandle:
            if (lowerPos != lower)
            {
                bool main = (mainControl == QDoubleSlider::LowerHandle);
                triggerAction(QAbstractSlider::SliderMove, main);
            }
            break;
        case QDoubleSlider::UpperHandle:
            if (upperPos != upper)
            {
                bool main = (mainControl == QDoubleSlider::UpperHandle);
                triggerAction(QAbstractSlider::SliderMove, main);
            }
            break;
        default:
            break;
    }
}

/*!
    \class QDoubleSlider
    \inmodule QxtWidgets
    \brief The QDoubleSlider widget is a QSlider with two handles.
    QDoubleSlider is a slider with two handles. QDoubleSlider is
    handy for letting user to choose an span between min/max.
    The span color is calculated based on QPalette::Highlight.
    The keys are bound according to the following table:
    \table
    \header \o Orientation    \o Key           \o Handle
    \row    \o Qt::Horizontal \o Qt::Key_Left  \o lower
    \row    \o Qt::Horizontal \o Qt::Key_Right \o lower
    \row    \o Qt::Horizontal \o Qt::Key_Up    \o upper
    \row    \o Qt::Horizontal \o Qt::Key_Down  \o upper
    \row    \o Qt::Vertical   \o Qt::Key_Up    \o lower
    \row    \o Qt::Vertical   \o Qt::Key_Down  \o lower
    \row    \o Qt::Vertical   \o Qt::Key_Left  \o upper
    \row    \o Qt::Vertical   \o Qt::Key_Right \o upper
    \endtable
    Keys are bound by the time the slider is created. A key is bound
    to same handle for the lifetime of the slider. So even if the handle
    representation might change from lower to upper, the same key binding
    remains.
    \image QDoubleSlider.png "QDoubleSlider in Plastique style."
    \bold {Note:} QDoubleSlider inherits QSlider for implementation specific
    reasons. Adjusting any single handle specific properties like
    \list
    \o QAbstractSlider::sliderPosition
    \o QAbstractSlider::value
    \endlist
    has no effect. However, all slider specific properties like
    \list
    \o QAbstractSlider::invertedAppearance
    \o QAbstractSlider::invertedControls
    \o QAbstractSlider::minimum
    \o QAbstractSlider::maximum
    \o QAbstractSlider::orientation
    \o QAbstractSlider::pageStep
    \o QAbstractSlider::singleStep
    \o QSlider::tickInterval
    \o QSlider::tickPosition
    \endlist
    are taken into consideration.
 */

/*!
    \enum QDoubleSlider::HandleMovementMode
    This enum describes the available handle movement modes.
    \value FreeMovement The handles can be moved freely.
    \value NoCrossing The handles cannot cross, but they can still overlap each other. The lower and upper values can be the same.
    \value NoOverlapping The handles cannot overlap each other. The lower and upper values cannot be the same.
 */

/*!
    \enum QDoubleSlider::SpanHandle
    This enum describes the available span handles.
    \omitvalue NoHandle \omit Internal only (for now). \endomit
    \value LowerHandle The lower boundary handle.
    \value UpperHandle The upper boundary handle.
 */

/*!
    \fn QDoubleSlider::lowerValueChanged(int lower)
    This signal is emitted whenever the \a lower value has changed.
 */

/*!
    \fn QDoubleSlider::upperValueChanged(int upper)
    This signal is emitted whenever the \a upper value has changed.
 */

/*!
    \fn QDoubleSlider::spanChanged(int lower, int upper)
    This signal is emitted whenever both the \a lower and the \a upper
    values have changed ie. the span has changed.
 */

/*!
    \fn QDoubleSlider::lowerPositionChanged(int lower)
    This signal is emitted whenever the \a lower position has changed.
 */

/*!
    \fn QDoubleSlider::upperPositionChanged(int upper)
    This signal is emitted whenever the \a upper position has changed.
 */

/*!
    \fn QDoubleSlider::sliderPressed(SpanHandle handle)
    This signal is emitted whenever the \a handle has been pressed.
 */

/*!
    Constructs a new QDoubleSlider with \a parent.
 */
QDoubleSlider::QDoubleSlider(QWidget* parent) : QSlider(parent), d_ptr(new QDoubleSliderPrivate())
{
    d_ptr->q_ptr = this;
    connect(this, SIGNAL(rangeChanged(int, int)), d_ptr, SLOT(updateRange(int, int)));
    connect(this, SIGNAL(sliderReleased()), d_ptr, SLOT(movePressedHandle()));
}

/*!
    Constructs a new QDoubleSlider with \a orientation and \a parent.
 */
QDoubleSlider::QDoubleSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent), d_ptr(new QDoubleSliderPrivate())
{
    d_ptr->q_ptr = this;
    connect(this, SIGNAL(rangeChanged(int, int)), d_ptr, SLOT(updateRange(int, int)));
    connect(this, SIGNAL(sliderReleased()), d_ptr, SLOT(movePressedHandle()));
}

/*!
    Destructs the span slider.
 */
QDoubleSlider::~QDoubleSlider()
{
}

/*!
    \property QDoubleSlider::handleMovementMode
    \brief the handle movement mode
 */
QDoubleSlider::HandleMovementMode QDoubleSlider::handleMovementMode() const
{
    return d_ptr->movement;
}

void QDoubleSlider::setHandleMovementMode(QDoubleSlider::HandleMovementMode mode)
{
    d_ptr->movement = mode;
}

/*!
    \property QDoubleSlider::lowerValue
    \brief the lower value of the span
 */
int QDoubleSlider::lowerValue() const
{
    return qMin(d_ptr->lower, d_ptr->upper);
}

void QDoubleSlider::setLowerValue(int lower)
{
    setSpan(lower, d_ptr->upper);
}

/*!
    \property QDoubleSlider::upperValue
    \brief the upper value of the span
 */
int QDoubleSlider::upperValue() const
{
    return qMax(d_ptr->lower, d_ptr->upper);
}

void QDoubleSlider::setUpperValue(int upper)
{
    setSpan(d_ptr->lower, upper);
}

/*!
    Sets the span from \a lower to \a upper.
 */
void QDoubleSlider::setSpan(int lower, int upper)
{
    const int low = qBound(minimum(), qMin(lower, upper), maximum());
    const int upp = qBound(minimum(), qMax(lower, upper), maximum());
    if (low != d_ptr->lower || upp != d_ptr->upper)
    {
        if (low != d_ptr->lower)
        {
            d_ptr->lower = low;
            d_ptr->lowerPos = low;
            emit lowerValueChanged(low);
        }
        if (upp != d_ptr->upper)
        {
            d_ptr->upper = upp;
            d_ptr->upperPos = upp;
            emit upperValueChanged(upp);
        }
        emit spanChanged(d_ptr->lower, d_ptr->upper);
        update();
    }
}

/*!
    \property QDoubleSlider::lowerPosition
    \brief the lower position of the span
 */
int QDoubleSlider::lowerPosition() const
{
    return d_ptr->lowerPos;
}

void QDoubleSlider::setLowerPosition(int lower)
{
    if (d_ptr->lowerPos != lower)
    {
        d_ptr->lowerPos = lower;
        if (!hasTracking())
            update();
        if (isSliderDown())
            emit lowerPositionChanged(lower);
        if (hasTracking() && !d_ptr->blockTracking)
        {
            bool main = (d_ptr->mainControl == QDoubleSlider::LowerHandle);
            d_ptr->triggerAction(SliderMove, main);
        }
    }
}

/*!
    \property QDoubleSlider::upperPosition
    \brief the upper position of the span
 */
int QDoubleSlider::upperPosition() const
{
    return d_ptr->upperPos;
}

void QDoubleSlider::setUpperPosition(int upper)
{
    if (d_ptr->upperPos != upper)
    {
        d_ptr->upperPos = upper;
        if (!hasTracking())
            update();
        if (isSliderDown())
            emit upperPositionChanged(upper);
        if (hasTracking() && !d_ptr->blockTracking)
        {
            bool main = (d_ptr->mainControl == QDoubleSlider::UpperHandle);
            d_ptr->triggerAction(SliderMove, main);
        }
    }
}

/*!
    \reimp
 */
void QDoubleSlider::keyPressEvent(QKeyEvent* event)
{
    QSlider::keyPressEvent(event);

    bool main = true;
    SliderAction action = SliderNoAction;
    switch (event->key())
    {
    case Qt::Key_Left:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Right:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Up:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Down:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Home:
        main   = (d_ptr->mainControl == QDoubleSlider::LowerHandle);
        action = SliderToMinimum;
        break;
    case Qt::Key_End:
        main   = (d_ptr->mainControl == QDoubleSlider::UpperHandle);
        action = SliderToMaximum;
        break;
    default:
        event->ignore();
        break;
    }

    if (action)
        d_ptr->triggerAction(action, main);
}

/*!
    \reimp
 */
void QDoubleSlider::mousePressEvent(QMouseEvent* event)
{
    if (minimum() == maximum() || (event->buttons() ^ event->button()))
    {
        event->ignore();
        return;
    }

    d_ptr->handleMousePress(event->pos(), d_ptr->upperPressed, d_ptr->upper, QDoubleSlider::UpperHandle);
    if (d_ptr->upperPressed != QStyle::SC_SliderHandle)
        d_ptr->handleMousePress(event->pos(), d_ptr->lowerPressed, d_ptr->lower, QDoubleSlider::LowerHandle);

    d_ptr->firstMovement = true;
    event->accept();
}

/*!
    \reimp
 */
void QDoubleSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (d_ptr->lowerPressed != QStyle::SC_SliderHandle && d_ptr->upperPressed != QStyle::SC_SliderHandle)
    {
        event->ignore();
        return;
    }

    QStyleOptionSlider opt;
    d_ptr->initStyleOption(&opt);
    const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
    int newPosition = d_ptr->pixelPosToRangeValue(d_ptr->pick(event->pos()) - d_ptr->offset);
    if (m >= 0)
    {
        const QRect r = rect().adjusted(-m, -m, m, m);
        if (!r.contains(event->pos()))
        {
            newPosition = d_ptr->position;
        }
    }

    // pick the preferred handle on the first movement
    if (d_ptr->firstMovement)
    {
        if (d_ptr->lower == d_ptr->upper)
        {
            if (newPosition < lowerValue())
            {
                d_ptr->swapControls();
                d_ptr->firstMovement = false;
            }
        }
        else
        {
            d_ptr->firstMovement = false;
        }
    }

    if (d_ptr->lowerPressed == QStyle::SC_SliderHandle)
    {
        if (d_ptr->movement == NoCrossing)
            newPosition = qMin(newPosition, upperValue());
        else if (d_ptr->movement == NoOverlapping)
            newPosition = qMin(newPosition, upperValue() - 1);

        if (d_ptr->movement == FreeMovement && newPosition > d_ptr->upper)
        {
            d_ptr->swapControls();
            setUpperPosition(newPosition);
        }
        else
        {
            setLowerPosition(newPosition);
        }
    }
    else if (d_ptr->upperPressed == QStyle::SC_SliderHandle)
    {
        if (d_ptr->movement == NoCrossing)
            newPosition = qMax(newPosition, lowerValue());
        else if (d_ptr->movement == NoOverlapping)
            newPosition = qMax(newPosition, lowerValue() + 1);

        if (d_ptr->movement == FreeMovement && newPosition < d_ptr->lower)
        {
            d_ptr->swapControls();
            setLowerPosition(newPosition);
        }
        else
        {
            setUpperPosition(newPosition);
        }
    }
    event->accept();
}

/*!
    \reimp
 */
void QDoubleSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QSlider::mouseReleaseEvent(event);
    setSliderDown(false);
    d_ptr->lowerPressed = QStyle::SC_None;
    d_ptr->upperPressed = QStyle::SC_None;
    update();
}

/*!
    \reimp
 */
void QDoubleSlider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);    

    // groove & ticks
    QStyleOptionSlider opt;
    d_ptr->initStyleOption(&opt);
    opt.sliderValue = 0;
    opt.sliderPosition = 0;
    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // handle rects
    opt.sliderPosition = d_ptr->lowerPos;
    const QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int lrv  = d_ptr->pick(lr.center());
    opt.sliderPosition = d_ptr->upperPos;
    const QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int urv  = d_ptr->pick(ur.center());

    // span
    const int minv = qMin(lrv, urv);
    const int maxv = qMax(lrv, urv);
    const QPoint c = QRect(lr.center(), ur.center()).center();
    QRect spanRect;
    if (orientation() == Qt::Horizontal)
        spanRect = QRect(QPoint(minv, c.y() - 10), QPoint(maxv, c.y() + 10));
    else
        spanRect = QRect(QPoint(c.x() - 10, minv), QPoint(c.x() + 10, maxv));
    d_ptr->drawSpan(&painter, spanRect);

    // handles
    switch (d_ptr->lastPressed)
    {
    case QDoubleSlider::LowerHandle:
        d_ptr->drawHandle(&painter, QDoubleSlider::UpperHandle);
        d_ptr->drawHandle(&painter, QDoubleSlider::LowerHandle);
        break;
    case QDoubleSlider::UpperHandle:
    default:
        d_ptr->drawHandle(&painter, QDoubleSlider::LowerHandle);
        d_ptr->drawHandle(&painter, QDoubleSlider::UpperHandle);
        break;
    }
}
