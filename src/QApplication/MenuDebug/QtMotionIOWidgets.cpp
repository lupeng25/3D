#include "QtMotionIOWidgets.h"
#include "Motion\MOtionIO.h"
#include "Data/dataVar.h"
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableView>
#include <QFile>
#include "qdebug.h"
#include <QDoubleSpinBox>
#include <QMenu>
#include <QPushButton>
#include "Motion/AutoMotion.h"

QtMotionIOWidgets::QtMotionIOWidgets(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);
	initUI();
}

QtMotionIOWidgets::~QtMotionIOWidgets()
{
	CloseUI();
}

void QtMotionIOWidgets::initUI()
{
	m_mapAxisTab.clear();
	m_pTabMode = new QWidget(this);
	QGridLayout *gridLayout_2 = new QGridLayout(m_pTabMode);
	gridLayout_2->setSpacing(4);
	gridLayout_2->setContentsMargins(10, 0, 10, 0);
	m_pTextlabel = new QLabel(m_pTabMode);
	m_pTextlabel->setMinimumSize(120,40);
	m_pTextlabel->setText(tr("Mode"));
	gridLayout_2->addWidget(m_pTextlabel,0,0,Qt::AlignCenter);
	m_pQComboBox = new QComboBox(m_pTabMode);
	m_pQComboBox->clear();
	m_pQComboBox->addItem(tr("IOMode"));
	m_pQComboBox->addItem(tr("LevelMode"));
	m_pQComboBox->setMinimumSize(200, 40);

	gridLayout_2->addWidget(m_pQComboBox, 0, 1, Qt::AlignCenter);
	m_pTabMode->setMinimumSize(300,40);
	m_pTabMode->show();
}

bool QtMotionIOWidgets::CheckPageAvalible()
{
	return false;
}

int QtMotionIOWidgets::CloseUI()
{
	IOAutoMotion::getInstance().CloseMechineLight();
	m_pTimer->stop();
	if (m_pTimer != nullptr)	delete m_pTimer;	m_pTimer = nullptr;
	return MotionIO::getInstance().Exit();
}

int QtMotionIOWidgets::initData(QString& strError)
{
	int _iCardNum = MotionIO::getInstance().Init(strError);
	if (_iCardNum <= 0)	{
		qCritical() << strError;
		return -1;
	}
	for (size_t i = 0; i < ui.tabWidget->count(); i++)	ui.tabWidget->removeTab(i);
	ui.tabWidget->clear();
	
	const int iGap = 1;
	_iCardNum = MotionIO::getInstance().GetCardNum();
	for (int iCard = 0; iCard < _iCardNum; iCard++)	{
		QString	strObject = tr("Card") + QString::number(iCard);
		QWidget* tab = new QWidget();
		ui.tabWidget->addTab(tab, strObject);
		QGridLayout *gridLayout_2 = new QGridLayout(tab);
		gridLayout_2->setSpacing(0);
		gridLayout_2->setContentsMargins(0, 0, 0, 0);

		QScrollArea *scrollArea = new QScrollArea(tab);
		scrollArea->setWidgetResizable(true);

		QWidget *scrollAreaWidget = new QWidget();
		scrollAreaWidget->setGeometry(QRect(0, 0, 600, 500));
		scrollAreaWidget->setStyleSheet(QString::fromUtf8("background-color: transparent;"));

		QVBoxLayout *verticalLayout = new QVBoxLayout(scrollAreaWidget);
		verticalLayout->setSpacing(0);
		verticalLayout->setContentsMargins(0, 0, 0, 0);

		QGroupBox *gridGroupBox = new QGroupBox(scrollAreaWidget);
		gridGroupBox->setTitle(tr("InPut"));
		gridGroupBox->setAlignment(Qt::AlignCenter);
		QGridLayout *m_pIOInputLayout = new QGridLayout(gridGroupBox);
		m_pIOInputLayout->setSpacing(iGap);
		m_pIOInputLayout->setContentsMargins(iGap, iGap, iGap, iGap);
		verticalLayout->addWidget(gridGroupBox);

		QGroupBox *gridGroupBox_2 = new QGroupBox(scrollAreaWidget);
		gridGroupBox_2->setTitle(tr("OutPut"));
		gridGroupBox_2->setAlignment(Qt::AlignCenter);
		QGridLayout *m_pIOOutPutLayout = new QGridLayout(gridGroupBox_2);
		m_pIOOutPutLayout->setContentsMargins(iGap, iGap, iGap, iGap);
		m_pIOOutPutLayout->setSpacing(iGap);
		verticalLayout->addWidget(gridGroupBox_2);

		QGroupBox *gridGroupBox_3 = new QGroupBox(scrollAreaWidget);
		gridGroupBox_3->setTitle(tr("Axis"));
		gridGroupBox_3->setAlignment(Qt::AlignCenter);
		QGridLayout *m_pAxisLayout = new QGridLayout(gridGroupBox_3);
		m_pAxisLayout->setContentsMargins(0,2 * iGap, 0, 0);
		m_pAxisLayout->setSpacing(0);
		verticalLayout->addWidget(gridGroupBox_3);

		QTableWidget* pTableWidget = new QTableWidget(gridGroupBox_3);
		if (pTableWidget->columnCount() < 22)
			pTableWidget->setColumnCount(22);
		m_mapAxisTab.insert(iCard, pTableWidget);
		{
			QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
			__qtablewidgetitem->setText(tr("Index"));
			__qtablewidgetitem->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
			QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
			__qtablewidgetitem1->setText(tr("eAxisType"));
			__qtablewidgetitem1->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
			QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
			__qtablewidgetitem2->setText(tr("dAxisUnit"));
			__qtablewidgetitem2->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
			QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
			__qtablewidgetitem3->setText(tr("eAlarmType"));
			__qtablewidgetitem3->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
			QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
			__qtablewidgetitem4->setText(tr("iAlarmSigIndex"));
			__qtablewidgetitem4->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem4);
			QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
			__qtablewidgetitem5->setText(tr("eAxisEnable"));
			__qtablewidgetitem5->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(5, __qtablewidgetitem5);
			QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
			__qtablewidgetitem6->setText(tr("eAxisHardPositiveLimit"));
			__qtablewidgetitem6->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(6, __qtablewidgetitem6);
			QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
			__qtablewidgetitem7->setText(tr("iHardPositiveSigIndex"));
			__qtablewidgetitem7->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(7, __qtablewidgetitem7);
			QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
			__qtablewidgetitem8->setText(tr("eAxisHardNegitiveLimit"));
			__qtablewidgetitem8->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(8, __qtablewidgetitem8);
			QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
			__qtablewidgetitem9->setText(tr("iHardNegitiveSigIndex"));
			__qtablewidgetitem9->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(9, __qtablewidgetitem9);
			QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
			__qtablewidgetitem10->setText(tr("bSoftLimit"));
			__qtablewidgetitem10->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(10, __qtablewidgetitem10);
			QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
			__qtablewidgetitem11->setText(tr("dPositiveLimitPos"));
			__qtablewidgetitem11->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(11, __qtablewidgetitem11);
			QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
			__qtablewidgetitem12->setText(tr("dNegetiveLimitPos"));
			__qtablewidgetitem12->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(12, __qtablewidgetitem12);
			QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
			__qtablewidgetitem13->setText(tr("eHomeOrign"));
			__qtablewidgetitem13->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(13, __qtablewidgetitem13);
			QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
			__qtablewidgetitem14->setText(tr("iOrignSigIndex"));
			__qtablewidgetitem14->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(14, __qtablewidgetitem14);
			QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
			__qtablewidgetitem15->setText(tr("eHomeMode"));
			__qtablewidgetitem15->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(15, __qtablewidgetitem15);
			{
				QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
				__qtablewidgetitem16->setText(tr("eMoveDir"));
				__qtablewidgetitem16->setToolTip(__qtablewidgetitem->text());
				pTableWidget->setHorizontalHeaderItem(16, __qtablewidgetitem16);
			}	{
				QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
				__qtablewidgetitem16->setText(tr("eHomeOffset"));
				__qtablewidgetitem16->setToolTip(__qtablewidgetitem->text());
				pTableWidget->setHorizontalHeaderItem(17, __qtablewidgetitem16);
			}
			QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
			__qtablewidgetitem17->setText(tr("dMoveHighSpeed"));
			__qtablewidgetitem17->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(18, __qtablewidgetitem17);
			QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
			__qtablewidgetitem18->setText(tr("dMoveLowSpeed"));
			__qtablewidgetitem18->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(19, __qtablewidgetitem18);
			QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
			__qtablewidgetitem19->setText(tr("dMoveSSpeed"));
			__qtablewidgetitem19->setToolTip(__qtablewidgetitem->text());
			pTableWidget->setHorizontalHeaderItem(20, __qtablewidgetitem19);
			{
				QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
				__qtablewidgetitem20->setText(tr("Home"));
				__qtablewidgetitem20->setToolTip(__qtablewidgetitem->text());
				pTableWidget->setHorizontalHeaderItem(21, __qtablewidgetitem20);
			}
			int iRowCount = pTableWidget->rowCount();
			for (int i = 0; i < iRowCount; i++)	pTableWidget->removeRow(0);
			pTableWidget->setMinimumHeight(400);
		}
		pTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		pTableWidget->horizontalHeader()->setVisible(true);
		pTableWidget->horizontalHeader()->setCascadingSectionResizes(true);
		pTableWidget->horizontalHeader()->setDefaultSectionSize(70);
		pTableWidget->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));
		pTableWidget->horizontalHeader()->setStretchLastSection(true);
		pTableWidget->verticalHeader()->setVisible(false);
		pTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(pTableWidget, &QTableWidget::customContextMenuRequested, this, &QtMotionIOWidgets::slotContextMenuRequested);
		m_pAxisLayout->addWidget(pTableWidget, 0, 0, 1, 1);
		scrollArea->setWidget(scrollAreaWidget);
		gridLayout_2->addWidget(scrollArea, 0, 0, 1, 1);
		ui.tabWidget->setCurrentIndex(0);

		//输入
		int _iLineRowNumber = 6;
		for (int iPoNo = 0; iPoNo < MotionIO::getInstance().GetInPortNum(); iPoNo++)	{
			QCheckBox* _ptrCbx = new QCheckBox();
			QString _strKeys = QString::number(iCard) + "-" + QString::number(iPoNo).sprintf("%d", iPoNo);
			_ptrCbx->setProperty("strKey", _strKeys);
			_ptrCbx->setProperty("InPut", "1");
			_ptrCbx->setCheckable(true);
			_ptrCbx->setText(tr("BackUp"));
			_ptrCbx->setToolTip(_strKeys + " " + QString::number(iPoNo + 1));
			_ptrCbx->setObjectName("btnShow");
			connect(_ptrCbx, SIGNAL(pressed()), this, SLOT(slotPressIOOutPut()));//
			m_mapInPutCheckBox[(iCard)].push_back(_ptrCbx);

			int _iColMod	= iPoNo % _iLineRowNumber;
			int _iColIndex	= iPoNo / _iLineRowNumber;
			m_pIOInputLayout->addWidget(_ptrCbx, _iColIndex, _iColMod, Qt::AlignLeft);
		}
		//输出
		for (int iPoNo = 0; iPoNo < MotionIO::getInstance().GetOutPortNum(); iPoNo++)	{
			QCheckBox* _ptrCbx = new QCheckBox();
			QString _strKeys = QString::number(iCard) + "-" + QString::number(iPoNo).sprintf("%d", iPoNo);
			_ptrCbx->setProperty("strKey", _strKeys);
			_ptrCbx->setProperty("InPut", "0");
			_ptrCbx->setCheckable(true);
			_ptrCbx->setText(tr("BackUp"));
			_ptrCbx->setToolTip(_strKeys + " " + QString::number(iPoNo + 1));
			_ptrCbx->setObjectName("btnShow");
			connect(_ptrCbx, SIGNAL(pressed()), this, SLOT(slotPressIOOutPut()));//
			m_mapOutPutCheckBox[(iCard)].push_back(_ptrCbx);
			int _iColMod	= iPoNo % _iLineRowNumber;
			int _iColIndex	= iPoNo / _iLineRowNumber;
			m_pIOOutPutLayout->addWidget(_ptrCbx,_iColIndex,_iColMod,Qt::AlignLeft);
		}
	}

	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));//每隔800毫秒刷新
	m_pTimer->start(800);
	if (_iCardNum > 0)	{	return 0;	}
	else {	strError = tr("Didn't Find IO Card");	return -1;	}
}

int QtMotionIOWidgets::GetData(QJsonObject & strData)
{
	QJsonObject AllParam;

	strData.insert("QtMotionIOWidgets", AllParam);
	return 0;
}

int QtMotionIOWidgets::SetData(QJsonObject & strData)
{
	QJsonObject AllParam = strData.find("QtMotionIOWidgets").value().toObject();

	return 0;
}

int QtMotionIOWidgets::GetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	int	_iCardNum = MotionIO::getInstance().GetCardNum();
	//int	_iPortNum = MotionIO::getInstance().GetPortNum();
	QJsonArray CardsParam;
	for (int iCard = 0; iCard < _iCardNum; iCard++)	{
		QJsonObject CardParam;
		//输入
		QJsonArray InPutParam;
		for (int iPort = 0; iPort < MotionIO::getInstance().GetInPortNum(); iPort++){
			bool bInLevel = true;
			QString strInName = m_mapInPutCheckBox[iCard][iPort]->text();
			MotionIO::getInstance().GetInLevelStatus(iCard, iPort, bInLevel);
			QJsonArray PortParam = {
				QString::number(iCard),
				QString::number(iPort),
				QString::number(bInLevel),
				strInName,
			};
			InPutParam.append(PortParam);
		}
		CardParam.insert("InPutParam", InPutParam);

		//输出
		QJsonArray OutPutParam;
		for (int iPort = 0; iPort < MotionIO::getInstance().GetOutPortNum(); iPort++)	{
			bool bOutLevel = true;
			QString strOutName = m_mapOutPutCheckBox[iCard][iPort]->text();
			MotionIO::getInstance().GetOutLevelStatus(iCard, iPort, bOutLevel);
			QJsonArray PortParam = {
				QString::number(iCard),
				QString::number(iPort),
				QString::number(bOutLevel),
				strOutName,
			};
			OutPutParam.append(PortParam);
		}
		CardParam.insert("OutPutParam", OutPutParam);

		CardsParam.append(CardParam);
	}
	CommonParam.insert("CardsParam", CardsParam);

	QJsonArray AllPositionParam;
	for (auto iter : m_mapAxisTab) {
		if (iter != nullptr) {
			QJsonArray PositionParam;
			for (int iRow = 0; iRow < iter->rowCount(); iRow++) {
				QJsonArray ConFig;
				for (int iCol = 0; iCol < iter->columnCount(); iCol++) {
					QTableWidgetItem * currentItem = iter->item(iRow, iCol);
					QWidget* _pQWidget = iter->cellWidget(iRow, iCol);
					if (currentItem != nullptr)
						ConFig.append(currentItem->text());
					else if (_pQWidget != nullptr) {
						QString	strClass = _pQWidget->metaObject()->className();
						if (strClass == "QDoubleSpinBox") {
							ConFig.append(QString::number(((QDoubleSpinBox*)_pQWidget)->value()));
						}
						if (strClass == "QSpinBox") {
							ConFig.append(QString::number(((QSpinBox*)_pQWidget)->value()));
						}
						else if (strClass == "QLabel") {
							ConFig.append(QString(((QLabel*)_pQWidget)->text()));
						}
						else if (strClass == "QComboBox") {
							ConFig.append(QString::number(((QComboBox*)_pQWidget)->currentIndex()));
						}
						else if (strClass == "QCheckBox") {
							ConFig.append(QString::number(((QCheckBox*)_pQWidget)->isChecked()));
						}
					}
				}
				PositionParam.append(ConFig);
			}
			AllPositionParam.append(PositionParam);
		}
	}
	CommonParam.insert("AllAxisParam", AllPositionParam);

	strData.insert("QtMotionIOWidgets", CommonParam);

	QString strError;
	if (InitAxis(strError) < 0) {
		qCritical() << strError;
		return -1;
	}
	return 0;
}

int QtMotionIOWidgets::SetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtMotionIOWidgets").value().toObject();
	QJsonArray CardsParam	= CommonParam.find("CardsParam").value().toArray();

	int	_iCardNum = MotionIO::getInstance().GetCardNum();
	for (int iCard = 0; iCard < CardsParam.count(); iCard++)	{
		QJsonObject CardParam = CardsParam.at(iCard).toObject();
		QJsonArray InPutParam = CardParam.find("InPutParam").value().toArray();
		for (int iPort = 0; iPort < InPutParam.count(); iPort++)	{
			QJsonArray PortParam = InPutParam.at(iPort).toArray();
			bool bInLevel		= PortParam.at(2).toString().toInt();
			QString strInName	= PortParam.at(3).toString();
			MotionIO::getInstance().SetInName(iCard, iPort, strInName);
			MotionIO::getInstance().SetInLevelStatus(iCard, iPort, bInLevel);
			if (m_mapInPutCheckBox.count(iCard) > 0) {
				if (m_mapInPutCheckBox[iCard].size() > 0 ) {
					if (iPort < m_mapInPutCheckBox[iCard].size())	{
						m_mapInPutCheckBox[iCard][iPort]->setText(strInName);
						m_mapInPutCheckBox[iCard][iPort]->setToolTip(strInName + " " + QString::number(iPort + 1));
					}
				}
			}
		}
		QJsonArray OutPutParam = CardParam.find("OutPutParam").value().toArray();
		for (int iPort = 0; iPort < OutPutParam.count(); iPort++)	{
			QJsonArray PortParam = OutPutParam.at(iPort).toArray();
			bool bOutLevel			= PortParam.at(2).toString().toInt();
			QString strOutName		= PortParam.at(3).toString();
			MotionIO::getInstance().SetOutName(iCard, iPort, strOutName);
			MotionIO::getInstance().SetOutLevelStatus(iCard, iPort, bOutLevel);
			if (m_mapOutPutCheckBox.count(iCard) > 0) {
				if (m_mapOutPutCheckBox[iCard].size() > 0) {
					if (iPort < m_mapOutPutCheckBox[iCard].size()) {
						m_mapOutPutCheckBox[iCard][iPort]->setText(strOutName);
						m_mapOutPutCheckBox[iCard][iPort]->setToolTip(strOutName + " " + QString::number(iPort + 1));
					}
				}
			}
		}
	}

	QJsonArray AllPositionParam = CommonParam.find("AllAxisParam").value().toArray();
	for (int iTab = 0; iTab < AllPositionParam.count(); iTab++) {
		if (m_mapAxisTab.count() > 0) {
			if (m_mapAxisTab[iTab] != nullptr) {
				int iRowCount = m_mapAxisTab[iTab]->rowCount();
				for (int i = 0; i < iRowCount; i++)	m_mapAxisTab[iTab]->removeRow(0);
			}
		}
		QJsonArray PositionParam = AllPositionParam.at(iTab).toArray();
		for (int iRow = 0; iRow < PositionParam.count(); iRow++) {
			QJsonArray CParam = PositionParam.at(iRow).toArray();
			m_mapAxisTab[iTab]->insertRow(iRow);
			int iColCount = m_mapAxisTab[iTab]->columnCount();
			for (int iCol = 0; iCol < iColCount; iCol++) {
				QString strValue = CParam.at(iCol).toString();
				QWidget* _pQWidget = nullptr;
				QTableWidgetItem* _pItem = nullptr;
				switch (iCol) {
				case 0: {
					_pItem = new QTableWidgetItem();
					_pItem->setTextAlignment(Qt::AlignCenter);
					_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
					_pItem->setText(strValue);
					_pItem->setToolTip(strValue);
				}	break;
				case 1: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("Maichongfangxiang"));
					((QComboBox*)_pQWidget)->addItem(tr("Maichongfangxiang+CodeInput"));
					((QComboBox*)_pQWidget)->addItem(tr("Maichongfangxiang+CodeInputO"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 2: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(1);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					//((QDoubleSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
				}	break;
				case 3: {		//报警
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("None"));
					((QComboBox*)_pQWidget)->addItem(tr("Positive"));
					((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 4: {
					_pQWidget = new QSpinBox();
					((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QSpinBox*)_pQWidget)->setMinimum(-1);
					//((QSpinBox*)_pQWidget)->setFocusPolicy();
					((QSpinBox*)_pQWidget)->setMaximum(64);
					((QSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 5: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("None"));
					((QComboBox*)_pQWidget)->addItem(tr("Positive"));
					((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 6: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("None"));
					((QComboBox*)_pQWidget)->addItem(tr("Positive"));
					((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 7: {
					_pQWidget = new QSpinBox();
					((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QSpinBox*)_pQWidget)->setMinimum(-1);
					((QSpinBox*)_pQWidget)->setMaximum(64);
					((QSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 8: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("None"));
					((QComboBox*)_pQWidget)->addItem(tr("Positive"));
					((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 9: {
					_pQWidget = new QSpinBox();
					((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QSpinBox*)_pQWidget)->setMinimum(-1);
					((QSpinBox*)_pQWidget)->setMaximum(64);
					((QSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 10: {		//软限位
					_pQWidget = new QCheckBox();
					((QCheckBox*)_pQWidget)->setCheckable(true);
					((QCheckBox*)_pQWidget)->setChecked(strValue.toDouble());
					((QCheckBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 11: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QDoubleSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 12: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QDoubleSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 13: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("None"));
					((QComboBox*)_pQWidget)->addItem(tr("Positive"));
					((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 14: {
					_pQWidget = new QSpinBox();
					((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QSpinBox*)_pQWidget)->setMinimum(-1);
					((QSpinBox*)_pQWidget)->setMaximum(64);
					((QSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 15: {		//回零模式
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("TwoceHome"));
					((QComboBox*)_pQWidget)->addItem(tr("OnceHome"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 16: {		//移动方向
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("DirPositive"));
					((QComboBox*)_pQWidget)->addItem(tr("DirNegitive"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toInt());
					((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 17: {		//回零偏移
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QDoubleSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 18: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QDoubleSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 19: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QDoubleSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 20: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
					//((QDoubleSpinBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
				}	break;
				case 21: {
					_pQWidget = new QPushButton(this);
					((QPushButton*)_pQWidget)->setObjectName(QString::number(iRow));
					((QPushButton*)_pQWidget)->setText(tr("Home"));
					((QPushButton*)_pQWidget)->setToolTip(tr("Home"));
					//((QPushButton*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
					connect(_pQWidget, SIGNAL(pressed()), this, SLOT(slotHomeAxis()));//
				}	break;
				default: {	}	break;
				}
				if (_pItem != nullptr) {
					_pItem->setTextAlignment(Qt::AlignCenter);
					m_mapAxisTab[iTab]->setItem(iRow, iCol, _pItem);
				}
				if (_pQWidget != nullptr) m_mapAxisTab[iTab]->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}
	QString strError;
	if (InitAxis(strError) < 0) {
		qCritical() << strError;
		return -1;
	}
	return 0;
}

int QtMotionIOWidgets::Load()
{
	QString strPath = QApplication::applicationDirPath();
	QString strFilePath = strPath + "/config/IOconfig.json";
	QString strData;
	QFile file(strFilePath);
	if (!file.exists())
	{
		qCritical() << tr("Not Exist") << strFilePath;
		return -1;
	}
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QByteArray t = file.readAll();
	strData = QString(t);
	file.close();
	// 开始解析 解析成功返回QJsonDocument对象否则返回null
	QJsonParseError err_rpt;
	QJsonDocument root_document = QJsonDocument::fromJson(strData.toUtf8(), &err_rpt);
	if (err_rpt.error != QJsonParseError::NoError && !root_document.isNull())
	{
		qCritical() << tr("Not Exist") << strFilePath;
		return -1;
	}
	// 获取根节点
	QJsonObject root = root_document.object();
	QJsonObject CommonParam		= root.find("QtMotionIOWidgets").value().toObject();
	QJsonArray CardsParam		= CommonParam.find("CardsParam").value().toArray();
	int	_iCardNum				= MotionIO::getInstance().GetCardNum();
	for (int iCard = 0; iCard < CardsParam.count(); iCard++)
	{
		QJsonObject CardParam		= CardsParam.at(iCard).toObject();
		QJsonArray InPutParam		= CardParam.find("InPutParam").value().toArray();
		for (int iPort = 0; iPort < InPutParam.count(); iPort++) {
			QJsonArray PortParam	= InPutParam.at(iPort).toArray();
			bool bInLevel			= PortParam.at(2).toString().toInt();
			QString strInName		= PortParam.at(3).toString();
			MotionIO::getInstance().SetInName(iCard, iPort, strInName);
			MotionIO::getInstance().SetInLevelStatus(iCard, iPort, bInLevel);
			if (m_mapInPutCheckBox.count(iCard) > 0) {
				if (m_mapInPutCheckBox[iCard].size() > 0) {
					if (iPort < m_mapInPutCheckBox[iCard].size()) {
						m_mapInPutCheckBox[iCard][iPort]->setText(strInName);
						m_mapInPutCheckBox[iCard][iPort]->setToolTip(strInName + " " + QString::number(iPort + 1));
					}
				}
			}
		}
		QJsonArray OutPutParam		= CardParam.find("OutPutParam").value().toArray();
		for (int iPort = 0; iPort < OutPutParam.count(); iPort++) {
			QJsonArray PortParam	= OutPutParam.at(iPort).toArray();
			bool bOutLevel			= PortParam.at(2).toString().toInt();
			QString strOutName		= PortParam.at(3).toString();
			MotionIO::getInstance().SetOutName(iCard, iPort, strOutName);
			MotionIO::getInstance().SetOutLevelStatus(iCard, iPort, bOutLevel);
			if (m_mapOutPutCheckBox.count(iCard) > 0) {
				if (m_mapOutPutCheckBox[iCard].size() > 0) {
					if (iPort < m_mapOutPutCheckBox[iCard].size()) {
						m_mapOutPutCheckBox[iCard][iPort]->setText(strOutName);
						m_mapOutPutCheckBox[iCard][iPort]->setToolTip(strOutName + " " + QString::number(iPort + 1));
					}
				}
			}
		}
	}
	return 0;
}

int QtMotionIOWidgets::Save()
{
	QString strPath = QApplication::applicationDirPath();
	QString strFilePath = strPath + "/config/IOconfig.json";
	QJsonObject root;

	QJsonObject CommonParam;
	int	_iCardNum = MotionIO::getInstance().GetCardNum();
	//int	_iPortNum = MotionIO::getInstance().GetPortNum();
	QJsonArray CardsParam;
	for (int iCard = 0; iCard < _iCardNum; iCard++)
	{
		QJsonObject CardParam;
		//输入
		QJsonArray InPutParam;
		for (int iPort = 0; iPort < MotionIO::getInstance().GetInPortNum(); iPort++) {
			bool bInLevel = true;
			QString strInName = m_mapInPutCheckBox[iCard][iPort]->text();
			MotionIO::getInstance().GetInLevelStatus(iCard, iPort, bInLevel);
			QJsonArray PortParam = {
				QString::number(iCard),
				QString::number(iPort),
				QString::number(bInLevel),
				strInName,
			};
			InPutParam.append(PortParam);
		}
		CardParam.insert("InPutParam", InPutParam);

		//输入
		QJsonArray OutPutParam;
		for (int iPort = 0; iPort < MotionIO::getInstance().GetOutPortNum(); iPort++) {
			bool bOutLevel = true;
			QString strOutName = m_mapOutPutCheckBox[iCard][iPort]->text();
			MotionIO::getInstance().GetOutLevelStatus(iCard, iPort, bOutLevel);
			QJsonArray PortParam = {
				QString::number(iCard),
				QString::number(iPort),
				QString::number(bOutLevel),
				strOutName,
			};
			InPutParam.append(PortParam);
		}
		CardParam.insert("OutPutParam", OutPutParam);

		CardsParam.append(CardParam);
	}
	CommonParam.insert("CardsParam", CardsParam);
	root.insert("QtMotionIOWidgets", CommonParam);

	QJsonParseError err_rpt;
	QJsonDocument	root_document;

	root_document.setObject(root);
	QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
	QString	strData = QString(root_string_compact);
	QFile file(strFilePath);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	file.write(strData.toUtf8());
	file.close();
	return 0;
}

int QtMotionIOWidgets::NewProject()
{
	return 0;
}

void QtMotionIOWidgets::slotHomeAxis()
{
	QPushButton* _pBtn = (QPushButton*)sender();
	if (_pBtn == nullptr)	return;
	int	_iIndex = _pBtn->objectName().toInt();
	if (!MotionIO::getInstance().IsAxisMoveStop(_iIndex))	{
		dataVar::Instance().pManager->notify(tr("Axis Isn't Stoped!"), NotifyType_Error);
		return;
	}
	QString strError;
	if (MotionIO::getInstance().MoveHome(_iIndex, strError) < 0){
		dataVar::Instance().pManager->notify(tr("Axis ") + QString::number(_iIndex) + tr("Home Error!"), NotifyType_Error);
	}
}

void QtMotionIOWidgets::showEvent(QShowEvent *ev)
{
	QtWidgetsBase::showEvent(ev);
	m_pQComboBox->setCurrentIndex(0);
	ResizeUI();
}

void QtMotionIOWidgets::resizeEvent(QResizeEvent * ev)
{
	QtWidgetsBase::resizeEvent(ev);
	ResizeUI();
}

void QtMotionIOWidgets::ResizeUI()
{
	m_pTabMode->setMinimumHeight(ui.tabWidget->tabBar()->height());
	m_pTextlabel->setMinimumHeight(ui.tabWidget->tabBar()->height());
	m_pQComboBox->setMinimumHeight(ui.tabWidget->tabBar()->height());
	m_pTabMode->setGeometry(ui.tabWidget->width() - m_pTabMode->width(),0, m_pTabMode->width(), m_pTabMode->height());
	for (auto Iter : m_mapAxisTab) {
		if (Iter != nullptr) {
			int _iWidth = width() / Iter->columnCount() + 20;
			for (int i = 0; i < Iter->columnCount(); i++) {
				if (i == 11 || i == 12)	{
					Iter->setColumnWidth(i, _iWidth + 20);
				}
				else Iter->setColumnWidth(i, _iWidth - 2);
			}
		}
	}
}

int QtMotionIOWidgets::InitAxis(QString& strError)
{
	m_mapAxisParam.clear();
	for (auto iter : m_mapAxisTab) {
		if (iter != nullptr) {
			for (int iRow = 0; iRow < iter->rowCount(); iRow++) {
				AxisParam	_Axis;
				_Axis.iIndex = iRow;
				for (int iCol = 0; iCol < iter->columnCount(); iCol++) {
					QTableWidgetItem * currentItem = iter->item(iRow, iCol);
					QWidget* _pQWidget = iter->cellWidget(iRow, iCol);
					if (currentItem != nullptr) {			}
					else if (_pQWidget != nullptr) {
						switch (iCol)	{
						case 1: {
							_Axis.eAxisType = (EnumAxisType)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 2: {
							_Axis.dAxisUnit = (double)((QDoubleSpinBox*)_pQWidget)->value();
						}	break;
						case 3: {
							_Axis.eAlarmType = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 4: {
							_Axis.iAlarmSigIndex = ((QSpinBox*)_pQWidget)->value();
						}	break;
						case 5: {
							_Axis.eAxisEnable = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 6: {
							_Axis.eAxisHardPositiveLimit = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 7: {
							_Axis.iHardPositiveSigIndex = ((QSpinBox*)_pQWidget)->value();
						}	break;
						case 8: {
							_Axis.eAxisHardNegitiveLimit = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 9: {
							_Axis.iHardNegitiveSigIndex = ((QSpinBox*)_pQWidget)->value();
						}	break;
						case 10: {
							_Axis.bSoftLimit = ((QCheckBox*)_pQWidget)->isChecked();
						}	break;
						case 11: {
							_Axis.dPositiveLimitPos = (double)((QDoubleSpinBox*)_pQWidget)->value();
						}	break;
						case 12: {
							_Axis.dNegetiveLimitPos = (double)((QDoubleSpinBox*)_pQWidget)->value();
						}	break;
						case 13: {
							_Axis.eHomeOrign	= (EnumType)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 14: {
							_Axis.iOrignSigIndex = ((QSpinBox*)_pQWidget)->value();
						}	break;
						case 15: {
							_Axis.eHomeMode = (EnumHomeMode)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 16: {
							_Axis.eMoveDir = (EnumMoveDir)((QComboBox*)_pQWidget)->currentIndex();
						}	break;
						case 17: {
							_Axis.dMoveHomeOffset = (double)((QDoubleSpinBox*)_pQWidget)->value();
						}	break;
						case 18: {
							_Axis.dMoveHighSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
						}	break;
						case 19: {
							_Axis.dMoveLowSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
						}	break;
						case 20: {
							_Axis.dMoveSSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
						}	break;
						default:
							break;
						}
					}
				}
				m_mapAxisParam.insert(iRow,_Axis);
			}
		}
	}

	for (auto iter = m_mapAxisParam.begin(); iter != m_mapAxisParam.end(); iter++){
		if (MotionIO::getInstance().SetAxisParam(iter.value(), strError) != 0) {
			return -1;
		};
	}

	return 0;
}

void QtMotionIOWidgets::slotPressIOOutPut()
{
	auto pushbtn = (QCheckBox*)sender();
	if (pushbtn == nullptr)	return;
	QString strKey		= pushbtn->property("strKey").toString();
	bool bIsInPut		= pushbtn->property("InPut").toString().toInt();

	QStringList strLst	= strKey.split("-");
	if (strLst.size() > 1)	{
		int iCard = strLst[0].toInt();
		int iPort = strLst[1].toInt();
		bool bStatus = false;
		if (bIsInPut)		{
			switch (m_pQComboBox->currentIndex()) {
			case 0: {			} break;
			case 1: {
				MotionIO::getInstance().GetInLevelStatus(iCard, iPort, bStatus);
				MotionIO::getInstance().SetInLevelStatus(iCard, iPort, !bStatus);
			} break;
			}
		}
		else		{
			switch (m_pQComboBox->currentIndex()) {
			case 0: {
				MotionIO::getInstance().GetOutPortStatus(iCard, iPort, bStatus);
				MotionIO::getInstance().SetOutPortStatus(iCard, iPort, !bStatus);
			} break;
			case 1: {
				MotionIO::getInstance().GetOutLevelStatus(iCard, iPort, bStatus);
				MotionIO::getInstance().SetOutLevelStatus(iCard, iPort, !bStatus);

			} break;
			}
		}
	}
}

void QtMotionIOWidgets::slotContextMenuRequested(const QPoint & pos)
{
	QMenu* menu = new QMenu(this);
	QAction *AddPosAction_buf = new QAction(tr("Add Axis"));
	QObject::connect(AddPosAction_buf, &QAction::triggered, this, &QtMotionIOWidgets::AddAxis);
	menu->addAction(AddPosAction_buf);
	QAction *DeletePosAction_buf = new QAction(tr("Delete Axis"));
	QObject::connect(DeletePosAction_buf, &QAction::triggered, this, &QtMotionIOWidgets::DeleteAxis);
	menu->addAction(DeletePosAction_buf);
	QAction *ClearPosAction_buf = new QAction(tr("Modify Axis"));
	QObject::connect(ClearPosAction_buf, &QAction::triggered, this, &QtMotionIOWidgets::ModifyAxis);
	menu->addAction(ClearPosAction_buf);
	menu->addSeparator();
	menu->exec(QCursor::pos());
	if (AddPosAction_buf != nullptr)	delete	AddPosAction_buf;		AddPosAction_buf = nullptr;
	if (DeletePosAction_buf != nullptr)	delete	DeletePosAction_buf;	DeletePosAction_buf = nullptr;
	if (ClearPosAction_buf != nullptr)	delete	ClearPosAction_buf;		ClearPosAction_buf = nullptr;
	if (menu != nullptr)				delete	menu;					menu = nullptr;
}

void QtMotionIOWidgets::AddAxis()
{
	int iCurrent =	ui.tabWidget->currentIndex();
	if (m_mapAxisTab.count(iCurrent ) > 0)
	{
		int iRowCount = m_mapAxisTab[iCurrent]->rowCount();
		int iColCount = m_mapAxisTab[iCurrent]->columnCount();
		m_mapAxisTab[iCurrent]->setRowCount(iRowCount + 1);

		AxisParam _axis;
		QString	_strError;
		MotionIO::getInstance().GetAxisParam(iRowCount, _axis, _strError);
		for (int iCol = 0; iCol < iColCount; iCol++) {
			QWidget* _pQWidget = nullptr;
			QTableWidgetItem* _pItem = nullptr;
			switch (iCol) {
			case 0: {
				_pItem = new QTableWidgetItem();
				_pItem->setTextAlignment(Qt::AlignCenter);
				_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
				_pItem->setText(QString::number(iRowCount));
				_pItem->setToolTip(_pItem->text());
			}	break;
			case 1: {
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("Maichongfangxiang"));
				((QComboBox*)_pQWidget)->addItem(tr("Maichongfangxiang+CodeInput"));
				((QComboBox*)_pQWidget)->addItem(tr("Maichongfangxiang+CodeInputO"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eAxisType);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 2: {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QDoubleSpinBox*)_pQWidget)->setMinimum(1);
				((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
				((QDoubleSpinBox*)_pQWidget)->setValue(_axis.dAxisUnit);
			}	break;
			case 3: {	//报警
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("None"));
				((QComboBox*)_pQWidget)->addItem(tr("Positive"));
				((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eAlarmType);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 4: {
				_pQWidget = new QSpinBox();
				((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QSpinBox*)_pQWidget)->setMinimum(-1);
				((QSpinBox*)_pQWidget)->setMaximum(64);
				((QSpinBox*)_pQWidget)->setValue(_axis.iAlarmSigIndex);
			}	break;
			case 5: {
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("None"));
				((QComboBox*)_pQWidget)->addItem(tr("Positive"));
				((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eAxisEnable);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 6: {
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("None"));
				((QComboBox*)_pQWidget)->addItem(tr("Positive"));
				((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eAxisHardPositiveLimit);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 7: {
				_pQWidget = new QSpinBox();
				((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QSpinBox*)_pQWidget)->setMinimum(-1);
				((QSpinBox*)_pQWidget)->setMaximum(64);
				((QSpinBox*)_pQWidget)->setValue(_axis.iHardPositiveSigIndex);
			}	break;
			case 8: {
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("None"));
				((QComboBox*)_pQWidget)->addItem(tr("Positive"));
				((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eAxisHardNegitiveLimit);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 9: {
				_pQWidget = new QSpinBox();
				((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QSpinBox*)_pQWidget)->setMinimum(-1);
				((QSpinBox*)_pQWidget)->setMaximum(64);
				((QSpinBox*)_pQWidget)->setValue(_axis.iHardNegitiveSigIndex);
			}	break;
			case 10: {		//软限位
				_pQWidget = new QCheckBox();
				((QCheckBox*)_pQWidget)->setCheckable(true);
				((QCheckBox*)_pQWidget)->setChecked(_axis.bSoftLimit);
				((QCheckBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 11: {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999999);
				((QDoubleSpinBox*)_pQWidget)->setMaximum(999999999);
				((QDoubleSpinBox*)_pQWidget)->setValue(_axis.dPositiveLimitPos);
			}	break;
			case 12: {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999999);
				((QDoubleSpinBox*)_pQWidget)->setMaximum(999999999);
				((QDoubleSpinBox*)_pQWidget)->setValue(_axis.dNegetiveLimitPos);

			}	break;
			case 13: {
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("None"));
				((QComboBox*)_pQWidget)->addItem(tr("Positive"));
				((QComboBox*)_pQWidget)->addItem(tr("Negitive"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eHomeOrign);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 14: {
				_pQWidget = new QSpinBox();
				((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QSpinBox*)_pQWidget)->setMinimum(-1);
				((QSpinBox*)_pQWidget)->setMaximum(64);
				((QSpinBox*)_pQWidget)->setValue(_axis.iOrignSigIndex);

			}	break;
			case 15: {		//回零模式
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("TwoceHome"));
				((QComboBox*)_pQWidget)->addItem(tr("OnceHome"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eHomeMode);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 16: {		//移动方向
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->addItem(tr("DirPositive"));
				((QComboBox*)_pQWidget)->addItem(tr("DirNegitive"));
				((QComboBox*)_pQWidget)->setCurrentIndex((int)_axis.eMoveDir);
				((QComboBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}	break;
			case 17: {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
				((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
				((QDoubleSpinBox*)_pQWidget)->setValue(_axis.dMoveHomeOffset);

			}	break;
			case 18: {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
				((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
				((QDoubleSpinBox*)_pQWidget)->setValue(_axis.dMoveHighSpeed);
			}	break;
			case 19: {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
				((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
				((QDoubleSpinBox*)_pQWidget)->setValue(_axis.dMoveLowSpeed);
			}	break;
			case 20: {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				((QDoubleSpinBox*)_pQWidget)->setMinimum(-99999);
				((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
				((QDoubleSpinBox*)_pQWidget)->setValue(_axis.dMoveSSpeed);
			}	break;
			case 21: {
				_pQWidget = new QPushButton(this);
				((QPushButton*)_pQWidget)->setObjectName(QString::number(iRowCount));
				((QPushButton*)_pQWidget)->setText(tr("Home"));
				((QPushButton*)_pQWidget)->setToolTip(tr("Home"));
				connect(_pQWidget, SIGNAL(pressed()), this, SLOT(slotHomeAxis()));//
			}	break;
			default: {	}	break;
			}
			if (_pItem != nullptr) {
				_pItem->setTextAlignment(Qt::AlignCenter);
				m_mapAxisTab[iCurrent]->setItem(iRowCount, iCol, _pItem);
			}
			if (_pQWidget != nullptr) m_mapAxisTab[iCurrent]->setCellWidget(iRowCount, iCol, _pQWidget);
		}

	}
}

void QtMotionIOWidgets::DeleteAxis()
{
	int iCurrent = ui.tabWidget->currentIndex();
	if (m_mapAxisTab.count(iCurrent) > 0)
	{
		int iRowCount		= m_mapAxisTab[iCurrent]->rowCount();
		int iCurrentIndex	= m_mapAxisTab[iCurrent]->currentIndex().row();
		m_mapAxisTab[iCurrent]->removeRow(iCurrentIndex);
	}
}

void QtMotionIOWidgets::ModifyAxis()
{
	int iCurrent = ui.tabWidget->currentIndex();
	if (m_mapAxisTab.count(iCurrent) > 0)	{
		int _iCurrentIndex = m_mapAxisTab[iCurrent]->currentIndex().row();
		if (_iCurrentIndex >= 0)	{
			AxisParam	_Axis;
			_Axis.iIndex	= _iCurrentIndex;
			for (int iCol = 0; iCol < m_mapAxisTab[iCurrent]->columnCount(); iCol++) {
				QTableWidgetItem * currentItem = m_mapAxisTab[iCurrent]->item(_iCurrentIndex, iCol);
				QWidget* _pQWidget = m_mapAxisTab[iCurrent]->cellWidget(_iCurrentIndex, iCol);
				if (currentItem != nullptr) {	}
				else if (_pQWidget != nullptr) {
					switch (iCol) {
					case 1: {
						_Axis.eAxisType = (EnumAxisType)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 2: {
						_Axis.dAxisUnit = (double)((QDoubleSpinBox*)_pQWidget)->value();
					}	break;
					case 3: {
						_Axis.eAlarmType = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 4: {
						_Axis.iAlarmSigIndex = ((QSpinBox*)_pQWidget)->value();
					}	break;
					case 5: {
						_Axis.eAxisEnable = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 6: {
						_Axis.eAxisHardPositiveLimit = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 7: {
						_Axis.iHardPositiveSigIndex = ((QSpinBox*)_pQWidget)->value();
					}	break;
					case 8: {
						_Axis.eAxisHardNegitiveLimit = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 9: {
						_Axis.iHardNegitiveSigIndex = ((QSpinBox*)_pQWidget)->value();
					}	break;
					case 10: {
						_Axis.bSoftLimit = ((QCheckBox*)_pQWidget)->isChecked();
					}	break;
					case 11: {
						_Axis.dPositiveLimitPos = (double)((QDoubleSpinBox*)_pQWidget)->value();
					}	break;
					case 12: {
						_Axis.dNegetiveLimitPos = (double)((QDoubleSpinBox*)_pQWidget)->value();
					}	break;
					case 13: {
						_Axis.eHomeOrign = (EnumType)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 14: {
						_Axis.iOrignSigIndex = ((QSpinBox*)_pQWidget)->value();
					}	break;
					case 15: {
						_Axis.eHomeMode = (EnumHomeMode)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 16: {
						_Axis.eMoveDir = (EnumMoveDir)((QComboBox*)_pQWidget)->currentIndex();
					}	break;
					case 17: {
						_Axis.dMoveHomeOffset = (double)((QDoubleSpinBox*)_pQWidget)->value();
					}	break;
					case 18: {
						_Axis.dMoveHighSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
					}	break;
					case 19: {
						_Axis.dMoveLowSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
					}	break;
					case 20: {
						_Axis.dMoveSSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
					}	break;
					default:
						break;
					}
				}
			}
			QString	_strError;
			if (MotionIO::getInstance().SetAxisParam(_Axis, _strError) < 0) {
				qCritical() << _strError;
			}
		}
	}
}

void QtMotionIOWidgets::slotTimeOut()
{
	if (isVisible() && m_pQComboBox != nullptr)	{

		for (auto& iter = m_mapInPutCheckBox.begin(); iter != m_mapInPutCheckBox.end(); iter++) {
			for (int iPort = 0; iPort < iter.value().size(); iPort++) {
				if (iter.value()[iPort] != nullptr) {
					bool bStatus = false;
					switch (m_pQComboBox->currentIndex())	{
					case 0: {	MotionIO::getInstance().GetInPortStatus(iter.key(),  iPort, bStatus);	} break;
					case 1: {	MotionIO::getInstance().GetInLevelStatus(iter.key(), iPort, bStatus);	} break;
					}
					(iter.value()[iPort])->setChecked(bStatus);
				}
			}
		}
		for (auto& iter = m_mapOutPutCheckBox.begin(); iter != m_mapOutPutCheckBox.end(); iter++) {
			for (int iPort = 0; iPort < iter.value().size(); iPort++) {
				if (iter.value()[iPort] != nullptr) {
					bool bStatus = false;
					switch (m_pQComboBox->currentIndex()) 
					{
					case 0: {	MotionIO::getInstance().GetOutPortStatus(iter.key(), iPort, bStatus);	} break;
					case 1: {	MotionIO::getInstance().GetOutLevelStatus(iter.key(), iPort, bStatus);	} break;
					}
					(iter.value()[iPort])->setChecked(bStatus);
				}
			}
		}


	}
}