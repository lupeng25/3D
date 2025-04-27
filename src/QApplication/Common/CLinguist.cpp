//Linguist.cpp
#include "CLinguist.h"
#include <QApplication>
#include <QFile>
#include <QDir>

CLinguist::CLinguist()
{
	m_vecpQTranslator.clear();
}

CLinguist& CLinguist::getInstance()
{
	static CLinguist  linguist;
	return linguist;
}

void CLinguist::ChangeLanguage(Language lan)
{
	QString strLanguagePath = QApplication::applicationDirPath() + "/Language/";
	if (lan == this->m_CurrentLanguage || lan == Language::undefined) return;

	for (int i = 0; i < m_vecpQTranslator.size(); i++) {
		if (m_vecpQTranslator[i] != nullptr) {
			qApp->removeTranslator(m_vecpQTranslator[i]);
			delete m_vecpQTranslator[i];
		}
	}
	m_vecpQTranslator.clear();

	bool ret = false;
	switch (lan) {
	case Language_CN: {
		QDir dir(strLanguagePath);
		QFileInfoList fileList = dir.entryInfoList();
		for (QFileInfo f : fileList) {
			if (f.baseName() == "") {
				continue;
			}
			QString	strfileName = f.fileName();
			if ( !strfileName.toLower().contains("zh")	) {
				continue;
			}
			QTranslator* translator = new QTranslator();
			ret = translator->load(strLanguagePath + strfileName);
			if (ret)	{
				//ret = QCoreApplication::installTranslator(translator);
				qApp->installTranslator(translator);
			}
			m_vecpQTranslator.push_back(translator);
		}
	}	break;
	case Language_EN: {
		QDir dir(strLanguagePath);
		QFileInfoList fileList = dir.entryInfoList();
		for (QFileInfo f : fileList) {
			if (f.baseName() == "") {
				continue;
			}
			QString	strfileName = f.fileName();
			if (!strfileName.toLower().contains("en")) {
				continue;
			}
			QTranslator* translator = new QTranslator();
			ret = translator->load(strLanguagePath + strfileName);
			if (ret)		{
				//ret = QCoreApplication::installTranslator(translator);
				qApp->installTranslator(translator);
			}
			m_vecpQTranslator.push_back(translator);

		}
	}	break;
	default:
		break;
	}
	if (ret)
	{
		this->m_CurrentLanguage = lan;
		// 发出语言被切换的信号
		emit sigLanguageChaned();
	}
}
