/****************************************************************************
**
** Copyright (C) 2014 dragondjf
**
** QFramer is a frame based on Qt5.3, you will be more efficient with it. 
** As an Qter, Qt give us a nice coding experience. With user interactive experience(UE) 
** become more and more important in modern software, deveployers should consider business and UE.
** So, QFramer is born. QFramer's goal is to be a mature solution 
** which you only need to be focus on your business but UE for all Qters.
**
** Version	: 0.2.5.0
** Author	: dragondjf
** Website	: https://github.com/dragondjf
** Project	: https://github.com/dragondjf/QCFramer
** Blog		: http://my.oschina.net/dragondjf/home/?ft=atme
** Wiki		: https://github.com/dragondjf/QCFramer/wiki
** Lincence: LGPL V2
** QQ: 465398889
** Email: dragondjf@gmail.com, ding465398889@163.com, 465398889@qq.com
** 
****************************************************************************/

#include "ftitlebar.h"
#include "ftoolbutton.h"
#include <QHBoxLayout>
#include <QEvent>

FTitleBar* FTitleBar::instance = NULL;

FTitleBar::FTitleBar(QWidget *parent) :
    QFrame(parent)
{
    initData();
    initUI();
    initConnect();
    fixButton->click();
}

FTitleBar* FTitleBar::getInstace()
{
    if(!instance)
    {
        instance = new FTitleBar();
    }
    return instance;
}

void FTitleBar::RegisterBuutton(FToolButton * pbtn)
{
	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	pbtn->setSizePolicy(sizePolicy);
	m_vecpBtn.push_back(pbtn);
	m_playout->addWidget(pbtn);
}

void FTitleBar::initData()
{
	m_vecpBtn.clear();
    title = tr("");

	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);

    logoButton = new FToolButton();
    logoButton->setObjectName(QString("titleToolButton"));
    titleLabel = new QLabel;
	titleLabel->setObjectName(QString("titleLabel"));
	ContentLabel = new QLabel;
	ContentLabel->setObjectName(QString("ContentLabel"));
	//ContentLabel->setText("");
    settingButton = new FToolButton();
    settingButton->setFocusPolicy(Qt::NoFocus);
    settingButton->setObjectName(QString("titleToolButton"));
    skinButton = new FToolButton();
    skinButton->setObjectName(QString("titleToolButton"));
	skinButton->setSizePolicy(sizePolicy);

    fixButton = new FToolButton();
    fixButton->setObjectName(QString("titleToolButton"));
    fixButton->setFocusPolicy(Qt::NoFocus);
	fixButton->setSizePolicy(sizePolicy);

    minButton = new FToolButton();
    minButton->setObjectName(QString("titleToolButton"));
    maxButton = new FToolButton();
    maxButton->setObjectName(QString("titleToolButton"));
    closeButton = new FToolButton();
    closeButton->setObjectName(QString("titleToolButton"));

    logoButton->setFocusPolicy(Qt::NoFocus);
    settingButton->setFocusPolicy(Qt::NoFocus);
    skinButton->setFocusPolicy(Qt::NoFocus);
    fixButton->setFocusPolicy(Qt::NoFocus);
    minButton->setFocusPolicy(Qt::NoFocus);
    maxButton->setFocusPolicy(Qt::NoFocus);
    closeButton->setFocusPolicy(Qt::NoFocus);

    normal_max_flag = true;
    fixflag = false;
    maxIcon =  new QIcon(QString(":/skin/icon/icon/icons/dark/appbar.fullscreen.box.png"));
    normalIcon =  new QIcon(QString(":/skin/icon/icon/icons/dark/appbar.app.png"));

	ChangeLanguage();
}

void FTitleBar::initUI()
{
    setTitleBarHeight(Title_Height + 10);
    setObjectName(QString("FTitleBar"));
    setLogoButton(":/image/icon/images/QApplication.png");
    //setTitleLabel(tr("QApplication"), "QApplication");
    setSettingButton(":/skin/icon/icon/icons/dark/appbar.control.down.png");
    setSkinButton(":/skin/icon/icon/icons/dark/appbar.clothes.shirt.png");
    setFixButton(":/skin/icon/icon/icons/dark/appbar.lock.png");
    setMinButton(":/skin/icon/icon/icons/dark/appbar.minus.png");
    setMaxButton(":/skin/icon/icon/icons/dark/appbar.app.png");
    setCloseButton(":/skin/icon/icon/icons/dark/appbar.close.png", "close");
	m_playout = new QHBoxLayout;
	m_playout->setContentsMargins(0, 0, 0, 0);
	m_playout->setSpacing(0);

    QHBoxLayout* mainlayout = new QHBoxLayout;
    mainlayout->addWidget(logoButton);
    mainlayout->addWidget(titleLabel);
    mainlayout->addStretch();
	mainlayout->addWidget(ContentLabel);
	mainlayout->addStretch();
    mainlayout->addWidget(settingButton);
	mainlayout->addLayout(m_playout);
    mainlayout->addWidget(skinButton);
    mainlayout->addWidget(fixButton);
    mainlayout->addWidget(minButton);
    mainlayout->addWidget(maxButton);
    mainlayout->addWidget(closeButton);
    mainlayout->setContentsMargins(0, 0, 2, 0);
    mainlayout->setSpacing(0);
    setLayout(mainlayout);
}

void FTitleBar::ChangeLanguage()
{
	settingButton->setToolTip(tr("Setting"));
	skinButton->setToolTip(tr("skin"));
	if (fixflag) {
		fixButton->setToolTip(tr("locked"));
	}
	else {
		fixButton->setToolTip(tr("unlock"));
	}

	minButton->setToolTip(tr("min"));	

	if (normal_max_flag) {
		maxButton->setToolTip(tr("max"));
	}
	else {
		maxButton->setToolTip(tr("Restore"));
	}

	closeButton->setToolTip(tr("close"));
	for (auto Iter : m_vecpBtn) {
		Iter->setToolTip(tr(Iter->objectName().toStdString().c_str()));
	}
}

void FTitleBar::initConnect()
{
    connect(settingButton,	SIGNAL(clicked()), settingButton,	SLOT(showMenu()));
    connect(skinButton,		SIGNAL(clicked()), skinButton,		SLOT(showMenu()));
    connect(fixButton,		SIGNAL(clicked()), this,			SLOT(changeFix()));
    connect(maxButton,		SIGNAL(clicked()), this,			SIGNAL(maximumed()));
    connect(minButton,		SIGNAL(clicked()), this,			SIGNAL(minimuned()));
    connect(closeButton,	SIGNAL(clicked()), this,			SIGNAL(closed()));
    connect(maxButton,		SIGNAL(clicked()), this,			SLOT(switchMaxMin()));
}

void FTitleBar::changeFix()
{
    if(fixflag)    {
        fixButton->setIcon(QIcon(":/skin/icon/icon/icons/dark/appbar.lock.png"));
		fixButton->setToolTip(tr("locked"));
    }
    else{
        fixButton->setIcon(QIcon(":/skin/icon/icon/icons/dark/appbar.unlock.keyhole.png"));
		fixButton->setToolTip(tr("unlock"));
    }
    fixflag = ! fixflag;
}

void FTitleBar::updateMaxRestore()
{
	if (isMaximized())	{
		maxButton->setIcon(*normalIcon);
		maxButton->setToolTip(tr("max"));
		normal_max_flag = true;
	}
	else {
		maxButton->setIcon(*maxIcon);
		maxButton->setToolTip(tr("Restore"));
		normal_max_flag = false;
	}
}

void FTitleBar::updateMaxRestoreStatus()
{
	if (normal_max_flag)	{
		maxButton->setIcon(*normalIcon);
		maxButton->setToolTip(tr("max"));
		normal_max_flag = true;
	}
	else {
		maxButton->setIcon(*maxIcon);
		maxButton->setToolTip(tr("Restore"));
		normal_max_flag = false;
	}

}

void FTitleBar::RestoreMaxStatus()
{
	maxButton->setIcon(*normalIcon);
	maxButton->setToolTip(tr("max"));
	normal_max_flag = true;
}

bool FTitleBar::getFixedflag()
{
    return fixflag;
}

bool FTitleBar::getNormalMaxFlag()
{
	return normal_max_flag;
}

void FTitleBar::clearChecked()
{
    settingButton->setChecked(false);
    skinButton->setChecked(false);
    fixButton->setChecked(false);
    minButton->setChecked(false);
    maxButton->setChecked(false);
    closeButton->setChecked(false);
}

void FTitleBar::setSettingMenu(QMenu *menu)
{
    settingButton->setMenu(menu);
}

void FTitleBar::switchMaxMin()
{
    if(normal_max_flag)
    {
        maxButton->setIcon(*maxIcon);
		maxButton->setToolTip(tr("Restore"));
        normal_max_flag = false;
    }
    else{
        maxButton->setIcon(*normalIcon);
		maxButton->setToolTip(tr("max"));
        normal_max_flag = true;
    }
}

void FTitleBar::changeSkin()
{
//    BgSkinPopup* skinPopup = new BgSkinPopup;
//    skinPopup->move(QWidget::mapToGlobal(skinButton->geometry().bottomLeft()));
//    skinPopup->show();
}

void FTitleBar::setTitleBarHeight(int height)
{
    setFixedHeight(height);
    logoButton->setIconSize(QSize(height, height));
    titleLabel->setFixedHeight(height);
	ContentLabel->setFixedHeight(height);
    settingButton->setIconSize(QSize(height, height));
    skinButton->setIconSize(QSize(height, height));
    minButton->setIconSize(QSize(height, height));
    maxButton->setIconSize(QSize(height, height));
    closeButton->setIconSize(QSize(height, height));
}

void FTitleBar::setTitleBarWidth(int width)
{
	setMinimumWidth(width);
	logoButton->setIconSize(QSize(width, width));
	titleLabel->setMinimumWidth(width);
	ContentLabel->setMinimumWidth(width);
	settingButton->setMinimumWidth(width);
	skinButton->setMinimumWidth(width);
	fixButton->setMinimumWidth(width);
	for (auto Iter : m_vecpBtn) {
		Iter->setMinimumWidth(width);
	}
}

void FTitleBar::setLogoButton(const char *str, const char *objectName)
{
    logoButton->setIcon(QIcon(QString(str)));
    logoButton->setIconSize(QSize(height(), height()));
    if(objectName)
        logoButton->setObjectName(QString(objectName));
}

FToolButton* FTitleBar::getLogoButton()
{
    return logoButton;
}

void FTitleBar::setLogoButtonVisible(bool visible)
{
    logoButton->setVisible(visible);
}

bool FTitleBar::isLogoButtonVisible()
{
    return logoButton->isVisible();
}

void FTitleBar::setTitleLabel(QString str, const char *objectName)
{
    titleLabel->setText(str);
    titleLabel->setFixedHeight(height());
    if (objectName)
    {
        titleLabel->setObjectName(QString(objectName));
    }
}

QLabel* FTitleBar::getTitleLabel()
{
    return titleLabel;
}

void FTitleBar::setTitleLabelVisible(bool visible)
{
    titleLabel->setVisible(visible);
}

bool FTitleBar::isTitleLabelVisible()
{
    return titleLabel->isVisible();
}

void FTitleBar::setContentLabel(QString str, const char *objectName)
{
	ContentLabel->setText(str);
	ContentLabel->setFixedHeight(height());
	if (objectName)
	{
		ContentLabel->setObjectName(QString(objectName));
	}
}

QLabel* FTitleBar::getContentLabel()
{
	return ContentLabel;
}

void FTitleBar::setContentLabelVisible(bool visible)
{
	ContentLabel->setVisible(visible);
}

bool FTitleBar::isContentLabelVisible()
{
	return ContentLabel->isVisible();
}

void FTitleBar::setSettingButton(const char *str, const char *objectName)
{
    settingButton->setIcon(QIcon(QString(str)));
    settingButton->setIconSize(QSize(height(), height()));
    if (objectName)
    {
        settingButton->setObjectName(QString(objectName));
    }
}

FToolButton* FTitleBar::getSettingButton()
{
    return settingButton;
}

void FTitleBar::setSettingButtonVisible(bool visible)
{
    settingButton->setVisible(visible);
}

bool FTitleBar::isSettingButtonVisible()
{
    return settingButton->isVisible();
}

void FTitleBar::setSkinButton(const char *str, const char *objectName)
{

    skinButton->setIcon(QIcon(QString(str)));
    skinButton->setIconSize(QSize(height(), height()));
    if(objectName)
    {
        skinButton->setObjectName(QString(objectName));
    }
}

FToolButton* FTitleBar::getSkinButton()
{
    return skinButton;
}

void FTitleBar::setSkinButtonVisible(bool visible)
{
    skinButton->setVisible(visible);
}

bool FTitleBar::isSkinButtonVisible()
{
    return skinButton->isVisible();
}

void FTitleBar::setFixButton(const char *str, const char *objectName)
{
    fixButton->setIcon(QIcon(QString(str)));
    fixButton->setIconSize(QSize(height(), height()));
    if(objectName)    {
        fixButton->setObjectName(QString(objectName));
    }
}

FToolButton* FTitleBar::getFixButton()
{
    return fixButton;
}

void FTitleBar::setFixButtonVisible(bool visible)
{
    fixButton->setVisible(visible);
}

bool FTitleBar::isFixButtonVisible()
{
    return fixButton->isVisible();
}

void FTitleBar::setMinButton(const char *str, const char *objectName)
{
    minButton->setIcon(QIcon(QString(str)));
    minButton->setIconSize(QSize(height(), height()));
    if (objectName)    {
        minButton->setObjectName(QString(objectName));
    }
}

FToolButton* FTitleBar::getMinButton()
{
    return minButton;
}

void FTitleBar::setMinButtonVisible(bool visible)
{
    minButton->setVisible(visible);
}

bool FTitleBar::isMinButtonVisible()
{
    return minButton->isVisible();
}

void FTitleBar::setMaxButton(const char *str, const char *objectName)
{
    maxButton->setIcon(QIcon(QString(str)));
    maxButton->setIconSize(QSize(height(), height()));
    if (objectName)   {
        maxButton->setObjectName(QString(objectName));
    }
}

FToolButton* FTitleBar::getMaxButton()
{
    return maxButton;
}

void FTitleBar::setMaxButtonVisible(bool visible)
{
    maxButton->setVisible(visible);
}

bool FTitleBar::isMaxButtonVisible()
{
    return maxButton->isVisible();
}

void FTitleBar::setCloseButton(const char *str, const char *objectName)
{
    closeButton->setIcon(QIcon(QString(str)));
    closeButton->setIconSize(QSize(height(), height()));
    if (objectName)    {
        closeButton->setObjectName(QString(objectName));
    }
}

FToolButton* FTitleBar::getCloseButton()
{
    return closeButton;
}

void FTitleBar::setCloseButtonVisible(bool visible)
{
    closeButton->setVisible(visible);
}

bool FTitleBar::isCloseButtonVisible()
{
    return closeButton->isVisible();
}
//保存窗口最大化前窗口的位置以及大小
void FTitleBar::saveRestoreInfo(const QPoint point, const QSize size)
{
	m_restorePos = point;
	m_restoreSize = size;
}

//获取窗口最大化前窗口的位置以及大小
void FTitleBar::getRestoreInfo(QPoint& point, QSize& size)
{
	point = m_restorePos;
	size = m_restoreSize;
}

void FTitleBar::changeEvent(QEvent * ev)
{
	QFrame::changeEvent(ev);
	switch (ev->type())	{
	case QEvent::LanguageChange: {
		ChangeLanguage();
	}	break;
	default:
		break;
	}
}
