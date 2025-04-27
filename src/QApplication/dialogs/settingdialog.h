/****************************************************************************
**

** 
****************************************************************************/

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H
#include "QFramer/fbasedialog.h"
#include <QFrame>
#include <QWidget>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QPixmap>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
//#include "QFramer/fnavgationbar.h"
#include <QStandardItemModel>
#include "frmSetBase.h"

class SettingDialog : public FBaseDialog
{
    Q_OBJECT
public:
	enum animation_Direction {
		animationTop,
		animationRight,
		animationBottom,
		animationLeft,
		animationTopLeft,
		animationTopRight,
		animationBottomRight,
		animationBottomLeft,
		animationCenter
	};
public:
	int GetData(QJsonObject& strData);
	int SetData(QJsonObject& strData);
	int	NewProject();
	int UpdateUI();
	int ScanTime();
	void RetranslationUi();

protected:
	virtual void LanguageChange();

	QList<QStandardItem*> findItems(QString strData);
	QList<QStandardItem*> findStandardItems(QStandardItem* pItem,QString strData);
protected:
	virtual void showEvent(QShowEvent *event);
	virtual void hideEvent(QHideEvent *event);
public:
    explicit SettingDialog(QWidget *parent = 0);
	virtual	~SettingDialog();
public:
    void initUI();
	void initData();

    void initConnect();
	void addWidget(const QString& tile, const QString &obejctName, frmSetBase* widget);

	void closeEvent(QCloseEvent * ev);
	void resizeEvent(QResizeEvent *ev);
	int AddWndSet(frmSetBase* widget);
	void ToolsTreeWidgetInit();
	void LoadData();
	void ResetData();
	void SaveData();
signals:
	void sigUpDateParam();
	void sigUpDateStackStatus();
	void sigChangeTemperature(bool benable,bool bConnect,double dTemperature, double dHumodity);
	void sigResetAllStack();
	void sigUpDateAllStackStatus(int iMode, QString oldstack, QString newStack);
public slots:
	virtual void cloudAntimation(animation_Direction direction);
	void slotSearchTreeClick(const QModelIndex &index);

	void on_btnSave_clicked();
	void on_btnApply_clicked();
	void on_btnCancer_clicked();
private:
	QStackedWidget*				stackWidget;
	QLineEdit*					lineEditSearch;
	QTreeView*					treeViewSearch;
	QStandardItemModel*			m_pModel = nullptr;      //数据模型对象指针

	//QMap<QString, QString>		m_mapfrmParams;

	QMap<QString, int>			m_mapfrmSetBaseIndex;
	QMap<QString, frmSetBase*>	m_mapfrmSetBase;
	QList<QToolButton*>			m_BtnLst;
};

#endif // SETTINGDIALOG_H
