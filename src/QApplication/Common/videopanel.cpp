#include "videopanel.h"
#include "videobox.h"
#include "qevent.h"
#include "qmenu.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qtimer.h"
#include "qdebug.h"
#include "../ImageShow/QGraphicsViews.h"
#include "../ImageShow/ImgGraphicsViews.h"
#include "../data/datavar.h"
#include "databaseVar.h"

VideoPanel::VideoPanel(QWidget *parent) : QWidget(parent)
{
    this->initControl();
    this->initForm();
	videoMenu = new QMenu(this);
    this->initMenu();
}

VideoPanel::~VideoPanel()
{
	ClearMenu();

	databaseVar::Instance().VideoWidgets.clear();
	if (gridLayout != nullptr)	delete gridLayout; gridLayout = nullptr;
	for (int i = 0; i < widgets.size(); i++)
		if (widgets.at(i) != nullptr)	
			delete widgets.at(i); 
	widgets.clear();
}

bool VideoPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
		ImgGraphicsViews *widget = (ImgGraphicsViews *) watched;
        if (!videoMax) {
            videoMax = true;
            videoBox->hide_video_all();
            gridLayout->addWidget(widget, 0, 0);
            widget->setVisible(true);
        } else {
            videoMax = false;
            videoBox->show_video_all();
        }
        widget->setFocus();
    }
    return QWidget::eventFilter(watched, event);
}

void VideoPanel::changeEvent(QEvent * event)
{
	if (NULL != event) {
		switch (event->type()) {
		case QEvent::LanguageChange: {
			for (size_t i = 0; i < databaseVar::Instance().VideoWidgets.size(); i++)	{
				((ImgGraphicsViews *)databaseVar::Instance().VideoWidgets[i])->setText(tr("通道 ") + QString::number(i + 1));;
			}

		}	break;
		default:
			break;
		}
	}
}

void VideoPanel::ShowDefaultVideoType()
{
	if (videoCount <= 0)	return;

	if (videoCount > 0 && videoCount <= 4)
	{	videoType = "1_4";	}
	else if (videoCount > 4 && videoCount <= 6)
	{	videoType = "1_6";	}
	else if (videoCount > 6 && videoCount <= 8)
	{	videoType = "1_8";	}
	else if (videoCount > 8 && videoCount <= 9)
	{	videoType = "1_9";	}
	else if (videoCount > 9 && videoCount <= 13)
	{	videoType = "1_13";	}
	else if (videoCount > 13 && videoCount <= 16)
	{	videoType = "1_16";	}
	else if (videoCount > 16 && videoCount <= 25)
	{	videoType = "1_25";	}
	else if (videoCount > 25 && videoCount <= 36)
	{	videoType = "1_36";	}
	else if (videoCount > 36 && videoCount <= 64)
	{	videoType = "1_64";	}
	else
	{	videoType = "1_64";	}

}

QSize VideoPanel::minimumSizeHint() const
{
    return QSize(80, 60);
}

void VideoPanel::ClearMenu()
{
	ClearMenus(videoMenu);
	if (videoMenu != nullptr)
	{
		videoMenu->clear();
	}
}

void VideoPanel::ClearMenus(QMenu * menu)
{
	if (menu == nullptr)	return;
	if(menu->actions().size() > 0)
		for (auto iter : menu->actions())
		{
			if (iter == nullptr)	return;
			QString text = iter->text();
			QMenu * _menu = iter->menu();
			if (_menu == nullptr) {
				delete iter;
				iter = nullptr;
			};
			ClearMenus(_menu);		
		}
}

void VideoPanel::initControl()
{
    gridLayout = new QGridLayout;
    gridLayout->setSpacing(1);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gridLayout);
}

void VideoPanel::initForm()
{
    videoMax = false;
    videoCount = 1;
    videoType = "1_16";

    for (int i = 0; i < videoCount; ++i) {

		ImgGraphicsViews *widget = new ImgGraphicsViews(this);
        widget->setObjectName(QString("video%1").arg(i + 1));
        widget->installEventFilter(this);
        widget->setFocusPolicy(Qt::StrongFocus);
        widget->setAlignment(Qt::AlignCenter);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		//二选一可以选择显示文字,也可以选择显示背景图片
		widget->setText(tr("通道 ") + QString::number(i + 1));
        //widget->setText(QString("通道 %1").arg(i + 1));
        //widget->setPixmap(QPixmap(":/bg_novideo.png"));
        widgets << widget;
		databaseVar::Instance().VideoWidgets << widget;
    }
}

void VideoPanel::initMenu()
{
    //单独关联信号槽
    actionFull = new QAction("切换全屏模式", videoMenu);
    connect(actionFull, SIGNAL(triggered(bool)), this, SLOT(full()));

    //通过QAction类方式添加子菜单
    videoMenu->addAction(actionFull);
    //videoMenu->addAction(actionPoll);
    videoMenu->addSeparator();

    //实例化通道布局类
    videoBox = new VideoBox(this);
    QList<bool> enable;
    enable << true << true << true << true << true << true << true << true << true;

	videoBox->CameravideoCount = videoCount;

    videoBox->InitMenus(videoCount,videoMenu, enable);
	ShowDefaultVideoType();
    videoBox->setVideoType(videoType);
    videoBox->setLayout(gridLayout);
    videoBox->setWidgets(widgets);

    videoBox->show_video_all();
}

void VideoPanel::full()
{
    if (actionFull->text() == "切换全屏模式") {
        Q_EMIT fullScreen(true);
        actionFull->setText("切换正常模式");
    } else {
        Q_EMIT fullScreen(false);
        actionFull->setText("切换全屏模式");
    }

    //执行全屏处理
}

void VideoPanel::poll()
{
    if (actionPoll->text() == "启动轮询视频") {
        actionPoll->setText("停止轮询视频");
    } else {
        actionPoll->setText("启动轮询视频");
    }

    //执行轮询处理
}

void VideoPanel::changevideocount(int icount)
{
	if (icount <= 0)				return;
	videoCount						= icount;
	if (widgets.size() > icount)
	{
		for (int i = widgets.size() - 1; i >= icount; i--)
		{
			ImgGraphicsViews *widget			= (ImgGraphicsViews *)widgets.back();
			delete widget; widget	= nullptr;
			widgets.pop_back();
			databaseVar::Instance().VideoWidgets.pop_back();
		}
	}
	else
	{
		for (int i = widgets.size(); i < icount; i++)	{
			ImgGraphicsViews *widget			= new ImgGraphicsViews(this);
			widget->setObjectName(QString("video%1").arg(i + 1));
			widget->installEventFilter(this);
			widget->setFocusPolicy(Qt::StrongFocus);
			widget->setAlignment(Qt::AlignCenter);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

			//二选一可以选择显示文字,也可以选择显示背景图片
			widget->setText(tr("通道 ") + QString::number(i + 1));

			widgets.push_back(widget);
			databaseVar::Instance().VideoWidgets.push_back(widget);
		}
	}
	videoBox->CameravideoCount = icount;
	initMenu();

}

void VideoPanel::snapshot_video_one()
{
	ClearMenu();
}

int Index = 1;
void VideoPanel::snapshot_video_all()
{
	ClearMenu();
	changevideocount(Index);
	Index++;
}
