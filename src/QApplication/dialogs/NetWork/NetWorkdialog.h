/****************************************************************************
**

** 
****************************************************************************/

#ifndef NetWorkdialog_H
#define NetWorkdialog_H
#include "QFramer/fbasedialog.h"
#include <QWidget>
#include "frmInstrumentation.h"

class NetWorkdialog : public FBaseDialog
{
    Q_OBJECT
public:
    explicit NetWorkdialog(QWidget *parent = 0);
	virtual ~NetWorkdialog();
public:
    void initUI();
    void initConnect();

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);
protected:
	virtual void closeEvent(QCloseEvent * event);
	virtual void LanguageChange();
protected:
	frmInstrumentation* pfrmInstrumentation{ nullptr	};
};

#endif // SETTINGDIALOG_H