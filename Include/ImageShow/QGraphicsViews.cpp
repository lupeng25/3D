#include "ImageShow/QGraphicsViews.h"
#include <QScrollBar>
#include <QMenu>
#include "ImageShow/BaseItem.h"
#include <QFileDialog>
#include <QAction>
#include <QLabel>
#include "ImageShow/QGraphicsScenes.h"
#include <QHBoxLayout>
#include <QThread>
#include <QWidgetAction>
#include "ImageShow/ImageItem.h"
#include "qmutex.h"
#include <QTimer>
#include <QElapsedTimer>

QGraphicsViews::QGraphicsViews(QWidget* parent) : QGraphicsView(parent)
{
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);//解决拖动是背景图片残影
	setDragMode(QGraphicsView::ScrollHandDrag);
	drawBg();
	// 隐藏水平/竖直滚动条
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setBackgroundBrush(Qt::gray);

	// 设置场景范围
	setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);

	// 反锯齿
	setRenderHints(QPainter::Antialiasing);
	pImagItem	= new ImageItem;
	pScene		= new QGraphicsScenes;
	pScene->addItem(pImagItem);
	this->setScene(pScene);
	pdrawItem	= new DrawItem;
	AddItems(pdrawItem);
	//显示灰度值
	pGrayValue = new QLabel(this); 
	pGrayValue->setObjectName("GrayValue");
	pGrayValue->setAlignment(Qt::AlignLeft);
	//pGrayValue->setStyleSheet("color:white; background-color:transparent; font: Microsoft YaHei;font-size: 15px;border: none;");
	pGrayValue->setVisible(false);
	pGrayValue->setFixedWidth(702);

	//显示区域
	pfWidget = new QWidget(this);
	pfWidget->setFixedHeight(25);
	pfWidget->setGeometry(2, 538, 702, 25);
	//pfWidget->setStyleSheet("background-color:transparent;border: none;");
	pfLayout = new QHBoxLayout(pfWidget);
	pfLayout->setSpacing(0);
	pfLayout->setMargin(0);
	pfLayout->addWidget(pGrayValue);
	pfLayout->addStretch();
	connect(pImagItem, &ImageItem::MouseMoveInImg,	this, &QGraphicsViews::sig_MouseMoveInImg);
	connect(pImagItem, &ImageItem::RGBValue,		this, [&](QString InfoVal) {	pGrayValue->setText(InfoVal);	});

	//显示类别
	pqClassifierValue = new QLabel(this);
	pqClassifierValue->setText(tr("图片"));
	pqClassifierValue->setObjectName("ClassifierValue");
	pqClassifierValue->setAlignment(Qt::AlignCenter);
	//pqClassifierValue->setStyleSheet("color:white; background-color:transparent; font: Microsoft YaHei;font-size: 15px;font-weight: bold;border: none;");
	pqClassifierValue->setVisible(true);
	pqClassifierValue->setFixedWidth(300);

	//显示区域
	pfClassifierWidget = new QWidget(this);
	pfClassifierWidget->setFixedHeight(25);
	pfClassifierWidget->setGeometry(width() - 1 - 150, 2, 300, 25);
	//pfClassifierWidget->setStyleSheet("background-color:transparent;border: none;");
	pfClarityLayout = new QHBoxLayout(pfClassifierWidget);
	pfClarityLayout->setSpacing(0);
	pfClarityLayout->setMargin(0);
	pfClarityLayout->addWidget(pqClassifierValue);
	pfClarityLayout->addStretch();
	connect(pImagItem, &ImageItem::ClassifierValue, this, [&](QString InfoVal) {	pqClassifierValue->setText(InfoVal);	});
	SetType(false, false);

}

void QGraphicsViews::ClearSelected()
{
	scene()->selectedItems().clear();
}

void QGraphicsViews::ClearImgShow()
{
	image = QPixmap();
}

void QGraphicsViews::DispImage(QImage& Image, bool bShow)
{
	//加锁
	{
		std::lock_guard<std::mutex> _lock(mutex);
		image = QPixmap::fromImage(Image);
	}
	pImagItem->w = image.width();
	pImagItem->h = image.height();
	if (bShow)	pImagItem->setPixmap(image);
	GetFit();
}

void QGraphicsViews::ClearAllDrawItem()
{
	pdrawItem->ClearAll();
}

void QGraphicsViews::DispPoint(QVector<QPointF>& list, QColor color)
{
	pdrawItem->AddRegion(list, color);
}

void QGraphicsViews::DispLineRectPoint(QVector<sColorLineRect>& list, QColor color)
{
	pdrawItem->AddLineRect(list, color);
}

void QGraphicsViews::DispRRectanglePoint(QVector<sColorRRectangle>& list, QColor color)
{
	pdrawItem->AddRRectangle(list, color);
}

void QGraphicsViews::DispLinePoint(QVector<sColorLine>& list, QColor color)
{
	pdrawItem->AddLine(list, color);
}

void QGraphicsViews::DispTextList(QVector<sDrawText>& list)
{
	pdrawItem->AddTextList(list);
}

void QGraphicsViews::DispCrossPoint(QVector<sDrawCross>& list)
{
	pdrawItem->AddCross(list);
}

void QGraphicsViews::DispColorPoints(QVector<sColorPoint>& list)
{
	pdrawItem->AddColorPoints(list);
}

void QGraphicsViews::DispColorCircle(QVector<sColorCircle>& list)
{
	pdrawItem->AddColorCircles(list);
}

void QGraphicsViews::DispREllipsePoint(QVector<sColorRRectangle>& list)
{
	pdrawItem->AddColorEllipse(list);
}

void QGraphicsViews::AddItems(BaseItem* item)
{
	item->scale = &ZoomValue;
	int _itype = item->type();
	this->scene()->addItem(item);
}

void QGraphicsViews::RegisteredMenu(QString strkey, QString strText, QString strIcon)
{
	bool bFind = false;
	for (auto iter = m_mapMenuStr.begin(); iter != m_mapMenuStr.end(); iter++){
		if (iter.key() == strkey){
			iter.value().strText = strText;
			iter.value().strIcon = strIcon;
			bFind = true;
		}
	}
	menuStr _menu;
	_menu.strText = strText;
	_menu.strIcon = strIcon;
	if (!bFind){ m_mapMenuStr.insert(strkey, _menu);	}
}

int QGraphicsViews::calcBytesPerLine(int width, int bitCounts)
{
	//把图像的宽度数值换算成4的整数倍
	int step = 0;
	if (bitCounts >= 8)	{
		step = (width * bitCounts / 8 + 3) / 4 * 4;
	}
	else{
		step = (width * bitCounts + 31) / 32 * 4;
	}

	return step;
}

//设定信息模块是否显示
void QGraphicsViews::SetType(bool InfoFlg, bool GrayValueFlg)
{
	pImagItem->setAcceptHoverEvents(!InfoFlg);
	pGrayValue->setVisible(!InfoFlg);
	pqClassifierValue->setVisible(!InfoFlg);
}

void QGraphicsViews::ClearObj()
{
	for (auto& item : scene()->items()) {	if (item->type() == 10)	scene()->removeItem(item);	}
}

QGraphicsViews::~QGraphicsViews()
{
	ClearObj();
	if (pImagItem			!= nullptr)		delete pImagItem;			pImagItem			= nullptr;
	if (pdrawItem			!= nullptr)		delete pdrawItem;			pdrawItem			= nullptr;
	if (pGrayValue			!= nullptr)		delete pGrayValue;			pGrayValue			= nullptr;
	if (pqClassifierValue	!= nullptr)		delete pqClassifierValue;	pqClassifierValue	= nullptr;
	if (pfClarityLayout		!= nullptr)		delete pfClarityLayout;		pfClarityLayout		= nullptr;
	if (pfClassifierWidget	!= nullptr)		delete pfClassifierWidget;	pfClassifierWidget	= nullptr;
	if (pfLayout			!= nullptr)		delete pfLayout;			pfLayout			= nullptr;
	if (pfWidget			!= nullptr)		delete pfWidget;			pfWidget			= nullptr;
	if (pScene				!= nullptr)		delete pScene;				pScene				= nullptr;
}

void QGraphicsViews::SetToFit(qreal val)
{
	ZoomFrame(val / ZoomValue);
	ZoomValue = val;
	QScrollBar* pHbar = this->horizontalScrollBar();
	pHbar->setSliderPosition(PixX);
	QScrollBar* pVbar = this->verticalScrollBar();
	pVbar->setSliderPosition(PixY);
}

void QGraphicsViews::ClearSelectedObj()
{
	for (auto& item: scene()->selectedItems()){
		if (item->type() == 10)
			scene()->removeItem(item);
	}
}

void QGraphicsViews::setText(const QString & text)
{
	if(pqClassifierValue != nullptr)
		pqClassifierValue->setText(text);
}

void QGraphicsViews::UpdateImg()
{
	std::lock_guard<std::mutex> _lock(mutex);
	pImagItem->setPixmap(image);
	//image.save("d:\\B123.bmp");

}

void QGraphicsViews::NeedPaint(BaseItem* item, bool& bvalue, bool currentvalue)
{
	bool bPaint = false;
	if (bvalue != currentvalue)	{
		bPaint = true;
		bvalue = currentvalue;
	}

	if (bPaint){ item->update();	}
}

void QGraphicsViews::SetMouseModel(Qt::CursorShape shape)
{
	if (shape != m_curShape){	viewport()->setCursor(shape);}
	m_curShape = shape;
}

void QGraphicsViews::ZoomFrame(double value)
{
	this->scale(value, value);
}

void QGraphicsViews::GetFit()
{
	if (this->width() < 1 || image.width() < 1)		return;

	//图片自适应方法
	double winWidth		= this->width();
	double winHeight	= this->height();
	double ScaleWidth	= (image.width()	+ 1) / winWidth;
	double ScaleHeight	= (image.height()	+ 1) / winHeight;
	double row1, column1;
	double s = 0;
	if (ScaleWidth >= ScaleHeight){
		row1 = -(1) * ((winHeight * ScaleWidth) - image.height()) / 2;
		column1 = 0;
		s = 1 / ScaleWidth;
	}
	else{
		row1 = 0;
		column1 = -(1.0) * ((winWidth * ScaleHeight) - image.width()) / 2;
		s = 1 / ScaleHeight;
	}
	if (ZoomFit != s || PixX != column1 * s){
		ZoomFit = s;
		PixX = column1 * s;
		PixY = row1 * s;
		SetToFit(s);
	}
}

void QGraphicsViews::drawBg()
{
	bgPix.fill(color1);
	QPainter painter(&bgPix);
	painter.fillRect(0, 0, 18, 18, color2);
	painter.fillRect(18, 18, 18, 18, color2);
	painter.end();
}

void QGraphicsViews::mousePressEvent(QMouseEvent* ev)
{
	QGraphicsView::mousePressEvent(ev);
	bool bNoDragMode = false;
	m_iSelectedIndex = -1;
	for (auto& item : scene()->items())	{ 
		if (item->type() == 10 && item->isVisible() )	{
			DIRECTION dir;
			int	_iIndex = -1;
			if (((BaseItem*)item)->IsMouseInItem(mapToScene(ev->pos()),dir,_iIndex)) {
				m_iSelectedIndex = _iIndex;
				((BaseItem*)item)->m_bSelected = true;
				((BaseItem*)item)->SelectItem(true);
				((BaseItem*)item)->setSelected(true);
				if (_iIndex <= 0){	((BaseItem*)item)->setFlag(QGraphicsItem::ItemIsMovable, true);	}
				else { ((BaseItem*)item)->setFlag(QGraphicsItem::ItemIsMovable, false); }
				bNoDragMode = true;
				((BaseItem*)item)->setFlag(QGraphicsItem::ItemIsFocusable, true);
				((BaseItem*)item)->setFlag(QGraphicsItem::ItemIsSelectable, true);
			}	
			else {
				((BaseItem*)item)->setSelected(false);
				((BaseItem*)item)->SelectItem(false);
				((BaseItem*)item)->setFlag(QGraphicsItem::ItemIsFocusable, false);
				((BaseItem*)item)->setFlag(QGraphicsItem::ItemIsSelectable, false);
				((BaseItem*)item)->setFlag(QGraphicsItem::ItemIsMovable, false);
				((BaseItem*)item)->m_bSelected = false;
			}
		}
	}
	if (bNoDragMode)	{	setDragMode(QGraphicsView::NoDrag);	}
	else	{	setDragMode(QGraphicsView::ScrollHandDrag);		}

	emit sig_mousePressEvent(ev);
}

void QGraphicsViews::resizeEvent(QResizeEvent* event)
{
	pfWidget->setGeometry(2, height() - 30, 702, 25);
	pfClassifierWidget->setGeometry((width() - 1 - 300) / 2.0, 2, 300, 25);
	QGraphicsView::resizeEvent(event);
	QTimer::singleShot(100, this, SLOT(GetFit()));
}

void QGraphicsViews::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);
	QList<QGraphicsItem *> selecteds;
	for (auto& item : scene()->items()) {
		if (item->type() == 10)	
			if (((BaseItem*)item)->m_bSelected){
				item->setSelected(true);
				selecteds.append(item);
			}
	}
	emit sig_MouseClicked(selecteds);
	m_iSelectedIndex = -1;

	emit sig_mouseReleaseEvent(event);
}

void QGraphicsViews::mouseDoubleClickEvent(QMouseEvent* event)
{
	OnNormalScreen();
	GetFit();
	SetToFit(ZoomFit);
	QGraphicsView::mouseDoubleClickEvent(event);
}

void QGraphicsViews::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	emit sig_ShapePointChange();
	Qt::CursorShape _cursor = Qt::ArrowCursor;
	for (auto& item : scene()->items())	{
		if (item->type() == 10 && item->isVisible()) {
			DIRECTION _dir = NONE;
			int	_iIndex = -1;
			((BaseItem*)item)->IsMouseInItem(mapToScene(event->pos()), _dir, _iIndex);
			switch (_dir) {
			case DIRECTION::NONE: {	
				NeedPaint((BaseItem*)item,((BaseItem*)item)->m_bShowMoveSelected, false);
			}	break;
			case DIRECTION::NORTHWEST:
			case DIRECTION::NORTH:
			case DIRECTION::NORTHEAST:
			case DIRECTION::EAST:
			case DIRECTION::SOUTHEAST:
			case DIRECTION::SOUTH:
			case DIRECTION::SOUTHWEST:
			case DIRECTION::WEST:	{
				if (_cursor == Qt::ArrowCursor)		_cursor = Qt::CrossCursor;
				NeedPaint((BaseItem*)item,((BaseItem*)item)->m_bShowMoveSelected,true);
			}	break;
			case DIRECTION::INRECT: {
				if (_cursor == Qt::ArrowCursor)		_cursor = Qt::SizeAllCursor;
				NeedPaint((BaseItem*)item,((BaseItem*)item)->m_bShowMoveSelected, true);
			}	break;
			case DIRECTION::STARTPOINT:
			case DIRECTION::ENDPOINT:
			default: {
				if (_cursor == Qt::ArrowCursor)		_cursor = Qt::OpenHandCursor;
				NeedPaint((BaseItem*)item,((BaseItem*)item)->m_bShowMoveSelected, false);
			}	break;	}
		}
	}
	SetMouseModel(_cursor);

	QList<QGraphicsItem *> selecteds;
	bool _bChange = false;
	for (auto& item : scene()->items())	{
		if (item->type() == 10){
			if (((BaseItem*)item)->m_bSelected){
				selecteds.append(item);	_bChange = true;
			}
		}
	}
	emit sig_MouseMouve(selecteds);
	if (selecteds.size() == 1 && m_iSelectedIndex > 0 ) {
		((BaseItem*)selecteds[0])->updatePos(m_iSelectedIndex, selecteds[0]->mapFromScene(mapToScene(event->pos())));
		selecteds[0]->update();
	}

	emit sig_mouseMoveEvent(event);
	update();
}

void QGraphicsViews::wheelEvent(QWheelEvent* event)
{
	if ((event->delta() > 0) && (ZoomValue >= 50)) //最大放大到原始图像的50倍
		return;
	else if ((event->delta() < 0) && (ZoomValue <= 0.02))
		return;
	else{
		if (event->delta() > 0) { //鼠标滚轮向前滚动
			ZoomValue *= 1.1; //每次放大10%
			ZoomFrame(1.1);
		}
		else		{
			ZoomFrame(0.9);
			ZoomValue *= 0.9; //每次缩小10%
		}
	}
	update();
}

void QGraphicsViews::drawBackground(QPainter* painter, const QRectF& rect)
{
	Q_UNUSED(rect);
}

void QGraphicsViews::paintEvent(QPaintEvent* ev)
{
	QPainter paint(this->viewport());
	paint.drawTiledPixmap(QRect(QPoint(0, 0), QPoint(this->width(), this->height())), bgPix);  //绘制背景

	if (ev != nullptr)	QGraphicsView::paintEvent(ev);
}

bool QGraphicsViews::event(QEvent* ev)
{
	if (ev->type() == QEvent::Paint){
		pdrawItem->update();
		pImagItem->update();
	}
	return QGraphicsView::event(ev);
}

//右键菜单
void QGraphicsViews::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu* menu = new QMenu(this);
	QString	strText = tr("ZoomOut");
	QPushButton* zoomInAction_buf = new QPushButton(menu);
	zoomInAction_buf->setObjectName("ZoomOut");
	zoomInAction_buf->setText(tr("ZoomOut"));
	zoomInAction_buf->setIconSize(QSize(23, 23));
	zoomInAction_buf->setMinimumSize(QSize(120, 40));
	//zoomInAction_buf->setIcon(QIcon(":/icon/icons/light/ZoomOut.png"));
	QWidgetAction* zoomInAction = new QWidgetAction(this);
	zoomInAction->setDefaultWidget(zoomInAction_buf);
	connect(zoomInAction_buf, &QPushButton::clicked, this, &QGraphicsViews::OnZoomInImage);
	connect(zoomInAction_buf, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(zoomInAction);
	menu->addSeparator();

	QPushButton* zoomOutAction_buf = new QPushButton(menu);
	zoomOutAction_buf->setObjectName("ZoomIn");
	zoomOutAction_buf->setText(tr("ZoomIn"));
	zoomOutAction_buf->setIconSize(QSize(23, 23));
	zoomOutAction_buf->setMinimumSize(QSize(120, 40));
	//zoomOutAction_buf->setIcon(QIcon(":/icon/icons/light/ZoomIn.png"));
	QWidgetAction* zoomOutAction = new QWidgetAction(this);
	zoomOutAction->setDefaultWidget(zoomOutAction_buf);
	connect(zoomOutAction_buf, &QPushButton::clicked, this, &QGraphicsViews::OnZoomOutImage);
	connect(zoomOutAction_buf, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(zoomOutAction);
	menu->addSeparator();

	QPushButton* presetAction_buf = new QPushButton(menu);
	presetAction_buf->setObjectName("FitImage");
	presetAction_buf->setText(tr("FitImage"));
	presetAction_buf->setIconSize(QSize(22, 22));
	presetAction_buf->setMinimumSize(QSize(120, 40));
	//presetAction_buf->setIcon(QIcon(":/icon/icons/light/AutoFit.png"));
	QWidgetAction* presetAction = new QWidgetAction(this);
	presetAction->setDefaultWidget(presetAction_buf);
	connect(presetAction_buf, &QPushButton::clicked, this, &QGraphicsViews::OnFitImage);
	connect(presetAction_buf, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(presetAction);
	menu->addSeparator();

	QPushButton* FullScreenAction_buf = new QPushButton(menu);
	FullScreenAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* FullScreenAction = new QWidgetAction(this);
	FullScreenAction_buf->setIconSize(QSize(22, 22));
	FullScreenAction->setDefaultWidget(FullScreenAction_buf);
	if (windowState() != Qt::WindowFullScreen) {
		FullScreenAction_buf->setObjectName("FullScreeen");
		FullScreenAction_buf->setText(tr("FullScreeen"));
		//FullScreenAction_buf->setIcon(QIcon(":/icon/icons/light/FullScreeen.png"));
		connect(FullScreenAction_buf, &QPushButton::clicked, this, &QGraphicsViews::OnFullScreen);
	}
	else {
		FullScreenAction_buf->setObjectName("DisFullScreeen");
		FullScreenAction_buf->setText(tr("DisFullScreeen"));
		//FullScreenAction_buf->setIcon(QIcon(":/icon/icons/light/DisFullScreeen.png"));
		connect(FullScreenAction_buf, &QPushButton::clicked, this, &QGraphicsViews::OnNormalScreen);
	}
	connect(FullScreenAction_buf, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(FullScreenAction);
	menu->addSeparator();

	for (auto iter = m_mapMenuStr.begin(); iter != m_mapMenuStr.end(); iter++){
		QPushButton* presetAction_buf = new QPushButton(menu);
		presetAction_buf->setText(iter.value().strText);
		presetAction_buf->setIconSize(QSize(22, 22));
		presetAction_buf->setIcon(QIcon(iter.value().strIcon));
		QWidgetAction* presetAction = new QWidgetAction(this);
		presetAction->setDefaultWidget(presetAction_buf);
		connect(presetAction_buf, &QPushButton::clicked,this,[=](){
			emit sig_MouseMenuClicked(iter.key());
		});
		connect(presetAction_buf, &QPushButton::clicked, menu, &QMenu::close);
		menu->addAction(presetAction);
		menu->addSeparator();
	}

	menu->exec(QCursor::pos());
	if (zoomInAction_buf != nullptr)	delete	zoomInAction_buf;		zoomInAction_buf		= nullptr;
	if (zoomInAction != nullptr)		delete	zoomInAction;			zoomInAction			= nullptr;
	if (zoomOutAction_buf != nullptr)	delete	zoomOutAction_buf;		zoomOutAction_buf		= nullptr;
	if (zoomOutAction != nullptr)		delete	zoomOutAction;			zoomOutAction			= nullptr;
	if (presetAction_buf != nullptr)	delete	presetAction_buf;		presetAction_buf		= nullptr;
	if (presetAction != nullptr)		delete	presetAction;			presetAction			= nullptr;
	if (FullScreenAction_buf != nullptr)delete	FullScreenAction_buf;	FullScreenAction_buf	= nullptr;
	if (FullScreenAction != nullptr)	delete	FullScreenAction;		FullScreenAction		= nullptr;
	if (menu != nullptr)				delete	menu;					menu					= nullptr;
}

void QGraphicsViews::OnZoomInImage()
{
	double tmp_buf	= ZoomValue;
	tmp_buf			*= 1.1;
	double tmp		= tmp_buf / ZoomValue;
	ZoomValue		*= tmp;
	this->scale(tmp, tmp);
}

void QGraphicsViews::OnZoomOutImage()
{
	double tmp_buf		= ZoomValue;
	tmp_buf				*= 0.9;
	double tmp			= tmp_buf / ZoomValue;
	ZoomValue			*= tmp;
	this->scale(tmp,	tmp);
}

void QGraphicsViews::OnFitImage()
{
	GetFit();
	SetToFit(ZoomFit);
}

void QGraphicsViews::OnShowCrossImage()
{
	pImagItem->m_bShowCross = !pImagItem->m_bShowCross;
	pImagItem->update();
}

void QGraphicsViews::OnFullScreen()
{
	setWindowFlags(Qt::Window);
	showFullScreen();
}

void QGraphicsViews::OnNormalScreen()
{
	setWindowFlags(Qt::SubWindow);
	showNormal();
}
