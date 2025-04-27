#pragma once

#include <QDialog>
#include "ui_frmDeComposeTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"

#include <QTableWidget>
#include "DeComposeTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"

class QtVariantProperty;
class QtProperty;

#include "../../frmBaseTool.h"
class frmDeComposeTool : public frmBaseTool
{
	Q_OBJECT
public:
	frmDeComposeTool(QWidget* parent = Q_NULLPTR);
   ~frmDeComposeTool();
private:
	Ui::frmDeComposeToolClass ui;
public:
	int Load();
	int Save();
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	int	ExecuteComplete();

private:
	void initShapeProperty();

private slots:
	void slot_valueChanged(QtProperty *property, const QVariant &value);

private slots:
	void slot_ChangeMatchImg_Index(int Index);

private:
	QGraphicsViews*		m_pView = nullptr;
	QVector<HObject>	m_VecDstImage;
private:
	class QtVariantPropertyManager	*variantManager;
	class QtTreePropertyBrowser		*propertyEditor;
	class QtVariantEditorFactory	*variantFactory;

	QMap<QtProperty *, QString>			propertyToId;
	QMap<QString, QtVariantProperty *>	idToProperty;

	void addProperty(QtVariantProperty *property, const QString &id);
};