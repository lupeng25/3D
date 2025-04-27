/****************************************************************************
**

** 
****************************************************************************/

#ifndef USERDIALOG_H
#define USERDIALOG_H
#include "QFramer/fbasedialog.h"
#include <QWidget>
#include "LogIn.h"

class userdialog : public FBaseDialog
{
    Q_OBJECT
public:
    explicit userdialog(QWidget *parent = 0);
	virtual ~userdialog();
public:
    void initUI();
    void initConnect();
protected:
	virtual void closeEvent(QCloseEvent * event);
	virtual void LanguageChange();
signals:
	void sigLogOut();
	void sigLogChanged(QString);
public slots:
	void slotLogOut();
protected:
	LogIn* pLogInPage{ nullptr	};
};

#endif // SETTINGDIALOG_H