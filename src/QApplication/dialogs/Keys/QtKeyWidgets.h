#pragma once

#include <QDialog>
#include <QWidget>
#include <QDateTime>
#include <QLabel>
#include <QLayout>
#include <QSpacerItem>
#include <QLineEdit>
#include <QToolButton>
#include <QApplication>
//#include "ui_QtKeyWidgets.h"
#include "licence.h"
#include "QFramer/fbasedialog.h"
#include <QShowEvent>

class Ui_QtKeyWidgetsClass
{
public:
	QGridLayout *gridLayout;
	QSpacerItem *verticalSpacer;
	QVBoxLayout *verticalLayout_3;
	QLabel *label;
	QLabel *label_4;
	QHBoxLayout *horizontalLayout_3;
	QVBoxLayout *verticalLayout;
	QHBoxLayout *horizontalLayout;
	QLabel *label_2;
	QLineEdit *lEditMechineCode;
	QHBoxLayout *horizontalLayout_2;
	QLabel *label_3;
	QLineEdit *lEditRegister;
	QVBoxLayout *verticalLayout_2;
	QSpacerItem *verticalSpacer_3;
	QToolButton *btnRegister;
	QSpacerItem *horizontalSpacer_2;
	QSpacerItem *horizontalSpacer;
	QLabel *lbl_RemainderInFor;
	QLabel *lbl_InFor;
	QSpacerItem *verticalSpacer_2;

	void setupUi(FBaseDialog *QtKeyWidgetsClass)
	{
		if (QtKeyWidgetsClass->objectName().isEmpty())
			QtKeyWidgetsClass->setObjectName(QString::fromUtf8("QtKeyWidgetsClass"));
		QtKeyWidgetsClass->resize(1331, 540);
		gridLayout = new QGridLayout/*(QtKeyWidgetsClass)*/;
		gridLayout->setSpacing(0);
		gridLayout->setContentsMargins(11, 11, 11, 11);
		gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
		gridLayout->setContentsMargins(0, 0, 0, 0);
		verticalSpacer = new QSpacerItem(17, 56, QSizePolicy::Minimum, QSizePolicy::Expanding);

		gridLayout->addItem(verticalSpacer, 0, 1, 1, 1);

		verticalLayout_3 = new QVBoxLayout();
		verticalLayout_3->setSpacing(6);
		verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
		label = new QLabel(QtKeyWidgetsClass);
		label->setObjectName(QString::fromUtf8("label"));
		label->setMinimumSize(QSize(0, 80));
		QFont font;
		font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
		font.setPointSize(28);
		font.setBold(false);
		font.setItalic(false);
		font.setWeight(50);
		label->setFont(font);
		label->setStyleSheet(QString::fromUtf8("font: 28pt \"\345\276\256\350\275\257\351\233\205\351\273\221\";"));
		label->setScaledContents(true);
		label->setAlignment(Qt::AlignCenter);

		verticalLayout_3->addWidget(label);

		label_4 = new QLabel(QtKeyWidgetsClass);
		label_4->setObjectName(QString::fromUtf8("label_4"));
		label_4->setMinimumSize(QSize(0, 80));
		QFont font1;
		font1.setPointSize(20);
		font1.setBold(true);
		font1.setWeight(75);
		label_4->setFont(font1);
		label_4->setStyleSheet(QString::fromUtf8("color: rgb(255, 0, 0);"));
		label_4->setAlignment(Qt::AlignCenter);

		verticalLayout_3->addWidget(label_4);

		horizontalLayout_3 = new QHBoxLayout();
		horizontalLayout_3->setSpacing(6);
		horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
		verticalLayout = new QVBoxLayout();
		verticalLayout->setSpacing(6);
		verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
		horizontalLayout = new QHBoxLayout();
		horizontalLayout->setSpacing(6);
		horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
		label_2 = new QLabel(QtKeyWidgetsClass);
		label_2->setObjectName(QString::fromUtf8("label_2"));
		label_2->setMinimumSize(QSize(200, 0));
		QFont font2;
		font2.setPointSize(20);
		label_2->setFont(font2);
		label_2->setAlignment(Qt::AlignCenter);

		horizontalLayout->addWidget(label_2);

		lEditMechineCode = new QLineEdit(QtKeyWidgetsClass);
		lEditMechineCode->setObjectName(QString::fromUtf8("lEditMechineCode"));
		lEditMechineCode->setEnabled(true);
		lEditMechineCode->setMinimumSize(QSize(600, 80));
		QFont font3;
		font3.setPointSize(12);
		lEditMechineCode->setFont(font3);
		lEditMechineCode->setAlignment(Qt::AlignCenter);
		lEditMechineCode->setReadOnly(true);

		horizontalLayout->addWidget(lEditMechineCode);


		verticalLayout->addLayout(horizontalLayout);

		horizontalLayout_2 = new QHBoxLayout();
		horizontalLayout_2->setSpacing(6);
		horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
		label_3 = new QLabel(QtKeyWidgetsClass);
		label_3->setObjectName(QString::fromUtf8("label_3"));
		label_3->setMinimumSize(QSize(200, 0));
		label_3->setFont(font2);
		label_3->setAlignment(Qt::AlignCenter);

		horizontalLayout_2->addWidget(label_3);

		lEditRegister = new QLineEdit(QtKeyWidgetsClass);
		lEditRegister->setObjectName(QString::fromUtf8("lEditRegister"));
		lEditRegister->setMinimumSize(QSize(600, 80));
		lEditRegister->setFont(font3);
		lEditRegister->setAlignment(Qt::AlignCenter);

		horizontalLayout_2->addWidget(lEditRegister);


		verticalLayout->addLayout(horizontalLayout_2);


		horizontalLayout_3->addLayout(verticalLayout);

		verticalLayout_2 = new QVBoxLayout();
		verticalLayout_2->setSpacing(6);
		verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
		verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

		verticalLayout_2->addItem(verticalSpacer_3);

		btnRegister = new QToolButton(QtKeyWidgetsClass);
		btnRegister->setObjectName(QString::fromUtf8("btnRegister"));
		btnRegister->setMinimumSize(QSize(150, 80));
		btnRegister->setFont(font2);

		verticalLayout_2->addWidget(btnRegister);


		horizontalLayout_3->addLayout(verticalLayout_2);


		verticalLayout_3->addLayout(horizontalLayout_3);


		gridLayout->addLayout(verticalLayout_3, 1, 1, 2, 2);

		horizontalSpacer_2 = new QSpacerItem(178, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

		gridLayout->addItem(horizontalSpacer_2, 1, 3, 1, 1);

		horizontalSpacer = new QSpacerItem(179, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

		gridLayout->addItem(horizontalSpacer, 2, 0, 1, 1);

		lbl_RemainderInFor = new QLabel(QtKeyWidgetsClass);
		lbl_RemainderInFor->setObjectName(QString::fromUtf8("lbl_RemainderInFor"));
		lbl_RemainderInFor->setMinimumSize(QSize(0, 80));
		lbl_RemainderInFor->setFont(font1);
		lbl_RemainderInFor->setAlignment(Qt::AlignCenter);

		gridLayout->addWidget(lbl_RemainderInFor, 3, 1, 1, 1);

		lbl_InFor = new QLabel(QtKeyWidgetsClass);
		lbl_InFor->setObjectName(QString::fromUtf8("lbl_InFor"));
		lbl_InFor->setMinimumSize(QSize(0, 80));
		lbl_InFor->setFont(font1);
		lbl_InFor->setAlignment(Qt::AlignCenter);

		gridLayout->addWidget(lbl_InFor, 3, 2, 1, 1);

		verticalSpacer_2 = new QSpacerItem(17, 56, QSizePolicy::Minimum, QSizePolicy::Expanding);

		gridLayout->addItem(verticalSpacer_2, 4, 2, 1, 1);


		retranslateUi(QtKeyWidgetsClass);

		QMetaObject::connectSlotsByName(QtKeyWidgetsClass);
	} // setupUi

	void retranslateUi(FBaseDialog *QtKeyWidgetsClass)
	{
		QtKeyWidgetsClass->setWindowTitle(QApplication::translate("QtKeyWidgetsClass", "QtKeyWidgets", nullptr));
		label->setText(QApplication::translate("QtKeyWidgetsClass", "\350\257\267\350\276\223\345\205\245\346\263\250\345\206\214\347\240\201", nullptr));
		label_4->setText(QApplication::translate("QtKeyWidgetsClass", "\350\257\267\350\260\250\346\205\216\344\277\256\346\224\271\347\263\273\347\273\237\346\227\266\351\227\264!\344\277\256\346\224\271\346\227\266\351\227\264\345\217\257\350\203\275\345\257\274\350\207\264\347\263\273\347\273\237\344\270\215\345\217\257\347\224\250", nullptr));
		label_2->setText(QApplication::translate("QtKeyWidgetsClass", "\346\234\272\345\231\250\347\240\201", nullptr));
		lEditMechineCode->setPlaceholderText(QApplication::translate("QtKeyWidgetsClass", "\346\234\272\345\231\250\347\240\201", nullptr));
		label_3->setText(QApplication::translate("QtKeyWidgetsClass", "\346\263\250\345\206\214\347\240\201", nullptr));
		lEditRegister->setPlaceholderText(QApplication::translate("QtKeyWidgetsClass", "\346\263\250\345\206\214\347\240\201", nullptr));
		btnRegister->setText(QApplication::translate("QtKeyWidgetsClass", "\346\263\250\345\206\214", nullptr));
		lbl_RemainderInFor->setText(QString());
		lbl_InFor->setText(QString());
	} // retranslateUi

};

namespace Ui {
	class QtKeyWidgetsClass : public Ui_QtKeyWidgetsClass {};
} // namespace Ui

class QtKeyWidgets : public FBaseDialog
{
	Q_OBJECT

public:
	QtKeyWidgets(QWidget *parent = nullptr);
	~QtKeyWidgets();

public:
	void ClearLayout(QLayout *layout);

public:
	void initUIData();
	EnumLicenceRetn CheckSoftDog(double &dRemainderDays);

protected:
	virtual void LanguageChange();
protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void closeEvent(QCloseEvent *ev);
public slots:
	void on_btnRegister_clicked();
private:
	Ui::QtKeyWidgetsClass ui;
private:
	bool		isDragging = false;
	QPoint		dragPos;
};
