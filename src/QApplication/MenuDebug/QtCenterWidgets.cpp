#include "QtCenterWidgets.h"
#include "MenuDebug/QFlowChart.h"

QtCenterWidgets::QtCenterWidgets(QWidget *parent)
	: QWidget(parent)
{
	m_vecpWidgetsBase.clear();
	ui.setupUi(this);
	m_vecpWidgetsBase.push_back(ui.page);
}

QtCenterWidgets::~QtCenterWidgets()
{
}

void QtCenterWidgets::initUI()
{
	//AddWndSet(new QFlowChart());
}

bool QtCenterWidgets::CheckPageAvalible()
{
	return false;
}

int QtCenterWidgets::CloseUI()
{
	return 0;
}

int QtCenterWidgets::initData(QString & strError)
{
	return 0;
}

int QtCenterWidgets::initUIData(QString & strError)
{
	return 0;
}

int QtCenterWidgets::GetData(QJsonObject & strData)
{
	QJsonArray _dataArray;
	for (int i = 0; i < m_vecpWidgetsBase.size(); i++){
		QJsonObject _data;
		m_vecpWidgetsBase[i]->GetData(_data);
		_dataArray.append(_data);
	}
	strData.insert("QtCenterWidgets", _dataArray);
	return 0;
}

int QtCenterWidgets::SetData(QJsonObject & strData)
{
	QJsonArray _dataArray = strData.find("QtCenterWidgets").value().toArray();
	for (int i = 0; i < _dataArray.count(); i++)	{
		QJsonObject _Object = _dataArray.at(i).toObject();
		if (i < m_vecpWidgetsBase.size() )
			m_vecpWidgetsBase[i]->SetData(_Object);
	}

	return 0;
}

int QtCenterWidgets::GetSystemData(QJsonObject & strData)
{
	QJsonArray _dataArray;
	for (int i = 0; i < m_vecpWidgetsBase.size(); i++) {
		QJsonObject _data;
		m_vecpWidgetsBase[i]->GetData(_data);
		_dataArray.append(_data);
	}
	strData.insert("QtCenterWidgets", _dataArray);
	return 0;
}

int QtCenterWidgets::SetSystemData(QJsonObject & strData)
{
	QJsonArray _dataArray = strData.find("QtCenterWidgets").value().toArray();
	for (int i = 0; i < _dataArray.count(); i++) {
		QJsonObject _Object = _dataArray.at(i).toObject();
		if (i < m_vecpWidgetsBase.size())
			m_vecpWidgetsBase[i]->SetData(_Object);
	}
	return 0;
}

int QtCenterWidgets::AddWndSet(QtWidgetsBase * widget)
{
	QWidget* tab = new QWidget();
	QGridLayout *gridLayout_2 = new QGridLayout(tab);
	gridLayout_2->setSpacing(0);
	gridLayout_2->setContentsMargins(0, 0, 0, 0);
	gridLayout_2->addWidget(widget);
	m_vecpWidgetsBase.push_back(widget);

	ui.stackedWidget->addWidget(tab);
	ui.stackedWidget->setCurrentWidget(tab);
	return 0;
}
