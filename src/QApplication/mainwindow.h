#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "SARibbonMainWindow.h"
#include <QDateTime>
#include <QLabel>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QHideEvent>
#include <QStackedWidget>

#include "Common/settingmenu.h"
#include "Common/thememenu.h"
#include "Pages/QtWidgetsBase.h"
#include "MenuDebug/QtCenterWidgets.h"
#include "dialogs/NetWork/NetWorkdialog.h"
#include "dialogs/GlobalVar/frmGlobalVariable.h"
#include <QSystemTrayIcon>

class SARibbonCategory;
class SARibbonContextCategory;
class SARibbonCustomizeWidget;
class SARibbonActionsManager;
class SARibbonQuickAccessBar;
class SARibbonButtonGroupWidget;
class SARibbonPannel;
class QTextEdit;
class QComboBox;
class QCloseEvent;
class SARibbonApplicationButton;
class SARibbonToolButton;
class FToolButton;

class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* par = nullptr);
	static MainWindow* getInstance();
public:
	void Init();
	int initData(QString& strError);
	int initUIData(QString& strError);
	void Exit();
	void animationClose();
private:
    // 创建ribbon-application菜单示例
    void createRibbonApplicationButton();
    // 创建main菜单示例
    void createCategoryMain(SARibbonCategory* page);
	void createCategoryView(SARibbonCategory* page);
	void createCategoryMotion(SARibbonCategory* page);
	void createCategoryVision(SARibbonCategory* page);
    void createCategoryOther(SARibbonCategory* page);
	void createCategoryAbout(SARibbonCategory* page);
    void createQuickAccessBar(SARibbonQuickAccessBar* quickAccessBar);
    void createRightButtonGroup();
    // 创建ActionsManager，实现actions的管理以及SARibbonBar的自定义
    void createActionsManager();
    QAction* createAction(const QString& text, const QString& iconurl, const QString& objName);
    QAction* createAction(const QString& text, const QString& iconurl);

	SARibbonApplicationButton* createActionButton(const QString& text, const QString& iconurl, const QString& objName);
	void ChangeProjectStatus();
	int SaveProject(QString strFilePath);
	void OpenProject(QString strFilePath);
	void SaveProjectPath();
private slots:
    void onActionHelpTriggered();
    void onFontComWidgetCurrentFontChanged(const QFont& f);
    void onCheckBoxAlignmentCenterClicked(bool checked);
	void slotTimeOut();
	void slotLogOut();
	void slotChangeLanguage(int iLanguage);
	void onActionNetCommunicateTriggered();
	void onActionGlobalValTriggered();;
	void onActionImageLayoutTriggered();
	void onActionResetTriggered();
	void onActionStartTriggered();
	void onActionStopTriggered();

	void onActionNewTriggered();
	void onActionOpenTriggered();
	void onActionSaveTriggered();
	void onActionSaveAsTriggered();
protected:
    void closeEvent(QCloseEvent* ev) override;
	void changeEvent(QEvent * ev);
	virtual void LanguageChange();
private:
    SARibbonActionsManager*						m_pActionsManager { nullptr };
    QMenu*										m_pMenuApplicationBtn{ nullptr };
	QMenu*										m_pLanguageLst{ nullptr };
	SettingMenu*								m_pSettingLst{ nullptr };
	ThemeMenu*									m_themeMenu{ nullptr };
	QTimer *									m_pTimer{ nullptr };
	QDateTime 									m_QDateTime;
	QDateTime									m_SoftDogTime;
	bool										m_bFirstCheck;
	QDateTime									m_startTime;
	bool										m_bClose;
	QDateTime									m_CloseTime;
	QLabel *									m_pLblTime{ nullptr };
	QToolButton *								m_pLblUser{ nullptr };
	QVector<SARibbonCategory*>					m_pVecCategory;
	FToolButton*								m_pErrorMsgButton{ nullptr };
	QMap<QString, int>							m_mapfrmSetBaseIndex;
	QMap<QString, QtWidgetsBase*>				m_mapfrmWidgetsBase;
	QtCenterWidgets		*						m_pQtCenterWidgets{ nullptr };
	QLabel*										m_pCurrentProjectName{ nullptr };
	QVector<QAction*>							m_vecAllAction;
	QVector<QToolButton*>						m_vecAllQPushButton;
	NetWorkdialog*								m_pNetWorkdialog{ nullptr };
	frmGlobalVariable*							m_pfrmGlobalVariable{ nullptr };
};
#endif  // MAINWINDOW_H