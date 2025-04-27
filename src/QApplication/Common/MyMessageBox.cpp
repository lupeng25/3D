#include "MyMessageBox.h"
#include <QScreen>

#define MESSAGEWIDTH 450  //界面的大小
#define MESSAGEHEIGHT 225

#define TEXTFONTFAMILY "微软雅黑"  //text的字体样式和大小
#define TEXTFONTSIZE 12

#define BUTTONFONTFAMILY "微软雅黑"  //按钮的字体样式和大小
#define BUTTONFONTSIZE 12

#define BUTTONWIDTH 120  //按钮的大小
#define BUTTONHEIGHT 50


void MyMessageBox::CenterWindow(QWidget * window, QWidget * subwindow)
{
	if (window == nullptr)	return;

	// 获取屏幕的宽度和高度
	int screenWidth = window->width();
	int screenHeight = window->height();

	// 获取子窗口的宽度和高度
	int windowWidth = subwindow->width();
	int windowHeight = subwindow->height();

	// 计算子窗口的左上角坐标
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// 将子窗口移动到屏幕中央
	subwindow->move(x, y);
}

void MyMessageBox::SetMessageBox(Icon icon, const QString & text, const QString & title, StandardButtons buttons)
{
	setTitle(title);
	setIcon(icon);
	setText(text);
	if (buttons != NoButton)
		setStandardButtons(buttons);
}

MyMessageBox::MyMessageBox(QWidget* parent)
	:FBaseDialog(parent) {
	init();
}

MyMessageBox::MyMessageBox(Icon icon, const QString& title, const QString& text, StandardButtons buttons, QWidget* parent) :
	FBaseDialog(parent) {
	init();
	setTitle(title);
	setIcon(icon);
	setText(text);
	if (buttons != NoButton)
		setStandardButtons(buttons);
}

MyMessageBox::~MyMessageBox() {}

void MyMessageBox::setTitle(const QString& title) {
	setWindowTitle(title);
	getTitleBar()->setContentLabel(title);
}

MyMessageBox::Icon MyMessageBox::icon() const {
	return m_icon;
}

void MyMessageBox::setIcon(MyMessageBox::Icon icon) {
	setIconPixmap(standardIcon(icon));
	m_icon = icon;
}

QPixmap MyMessageBox::iconPixmap() const {
	return *m_pIconLabel->pixmap();
}

void MyMessageBox::setIconPixmap(const QPixmap& pixmap) {
	//    m_pIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pIconLabel->setPixmap(pixmap);
	m_icon = NoIcon;
}

QString MyMessageBox::text() const {
	return m_pTextLabel->text();
}

void MyMessageBox::setText(const QString& text) {
	m_pTextLabel->setFont(QFont(TEXTFONTFAMILY, TEXTFONTSIZE));
	m_pTextLabel->setText(text);
}

MyMessageBox::StandardButtons MyMessageBox::standardButtons() const {
	QDialogButtonBox::StandardButtons standardButtons = m_pButtonBox->standardButtons();
	return MyMessageBox::StandardButtons(int(standardButtons));  //不同类型的枚举转换
}

void MyMessageBox::setStandardButtons(StandardButtons buttons) {
	QDialogButtonBox::StandardButtons standardButtons = QDialogButtonBox::StandardButtons(int(buttons));
	m_pButtonBox->setStandardButtons(standardButtons);
	setPushButtonProperty(m_pButtonBox->buttons());
}

MyMessageBox::StandardButton MyMessageBox::standardButton(QAbstractButton* button) const {
	QDialogButtonBox::StandardButton standardButton = m_pButtonBox->standardButton(button);
	return (MyMessageBox::StandardButton)standardButton;  //转化为当前类的StandardButton类型
}

QAbstractButton* MyMessageBox::button(MyMessageBox::StandardButton which) const {
	QDialogButtonBox::StandardButton standardButton = QDialogButtonBox::StandardButton(which);
	return m_pButtonBox->button(standardButton);
}

MyMessageBox::ButtonRole MyMessageBox::buttonRole(QAbstractButton* button) const {
	QDialogButtonBox::ButtonRole buttonRole = m_pButtonBox->buttonRole(button);
	return MyMessageBox::ButtonRole(buttonRole);
}

QAbstractButton* MyMessageBox::clickedButton() const {
	return m_pClickedButton;
}

MyMessageBox::StandardButton MyMessageBox::information(QWidget* parent, const QString& text, QString title, StandardButtons buttons) {
	return showMessageBox(parent, Information, text, title, buttons);
}

MyMessageBox::StandardButton MyMessageBox::question(QWidget* parent, const QString& text, QString title, StandardButtons buttons) {
	return showMessageBox(parent, Question, text, title, buttons);
}

MyMessageBox::StandardButton MyMessageBox::success(QWidget* parent, const QString& text, QString title, StandardButtons buttons) {
	return showMessageBox(parent, Success, text, title, buttons);
}

MyMessageBox::StandardButton MyMessageBox::warning(QWidget* parent, const QString& text, QString title, StandardButtons buttons) {
	return showMessageBox(parent, Warning, text, title, buttons);
}

MyMessageBox::StandardButton MyMessageBox::critical(QWidget* parent, const QString& text, QString title, StandardButtons buttons) {
	return showMessageBox(parent, Critical, text, title, buttons);
}

MyMessageBox::StandardButton MyMessageBox::showMessageBox(QWidget* parent, Icon icon, const QString& text, QString title, StandardButtons buttons) {
	//MyMessageBox msgBox(icon, title, text, buttons, parent);
	MyMessageBox* msgBox = new MyMessageBox(icon, title, text, buttons, parent);
	//静态函数只能调用静态函数，setMessageBoxGeometry()必须声明为静态函数
	setMessageBoxGeometry(parent, *msgBox);
	if (msgBox->exec() == -1)
		return MyMessageBox::Cancel;
	auto res = msgBox->standardButton(msgBox->clickedButton());
	//delete msgBox;
	return res;
}

MyMessageBox::StandardButton MyMessageBox::WaitForShowMessageBox(QWidget * parent, Icon icon, const QString & text, QString title, StandardButtons buttons)
{
	MyMessageBox* msgBox = new MyMessageBox(icon, title, text, buttons, parent);
	//静态函数只能调用静态函数，setMessageBoxGeometry()必须声明为静态函数
	setMessageBoxGeometry(parent, *msgBox);
	msgBox->setClickedButton(nullptr);
	msgBox->show();
	while (msgBox->clickedButton() == nullptr )
	{
		if (!msgBox->isVisible())	
			break;
		qApp->processEvents();
	}
	//if (msgBox->exec() == -1)
	//	return MyMessageBox::Cancel;
	auto res = msgBox->standardButton(msgBox->clickedButton());
	delete msgBox;
	//delete msgBox;
	return res;
}

void MyMessageBox::setMessageBoxGeometry(QWidget* parent, MyMessageBox& msgBox) {
	QRect rect;
	if (parent == nullptr)
	{
		QScreen *screen = QGuiApplication::primaryScreen();
		rect = screen->geometry();//QRect()://= parent->geometry();
	}
	else
	{
		rect = parent->geometry();
	}
	int x = rect.x() + (rect.width() - msgBox.geometry().width()) / 2;
	int y = rect.y() + (rect.height() - msgBox.geometry().height()) / 2;
	msgBox.setGeometry(x, y, msgBox.geometry().width(), msgBox.geometry().height());
	msgBox.move(x, y);
}

MyMessageBox::StandardButton MyMessageBox::WaitShowMessageBox(QWidget * parent, Icon icon, const QString & text, QString title, StandardButtons buttons)
{
	if(this == nullptr)	setMessageBoxGeometry(parent, *this);
	setTitle(title);
	setIcon(icon);
	setText(text);
	if (buttons != NoButton)	setStandardButtons(buttons);
	setClickedButton(nullptr);
	if (exec() == -1)
		return MyMessageBox::Cancel;
	//show();
	//while (clickedButton() == nullptr)	{
	//	if (!isVisible())
	//		break;
	//	qApp->processEvents();
	//}
	//hide();
	auto res = standardButton(clickedButton());

	return res;
}

void MyMessageBox::slotPushButtonClicked(QAbstractButton* button) {
	setClickedButton(button);
	finalize(button);
	//close();
}

void MyMessageBox::init() {
	normalSize = QSize(900, 500);
	//resize(QSize(MESSAGEWIDTH, MESSAGEHEIGHT));
	setWindowIcon(QIcon(":/image/icon/images/Info.png"));
	setWindowTitle(tr("Message"));
	setObjectName("Message");
	//对于继承的QDialog，其Flags必须包含Qt::Dialog，不能自定义CustomizeWindowHint
	//否则在构造中指定了parent之后会出现：标题栏消失以及窗口不在正中间
	//setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);  //设置右上角按钮
	//Qt::WindowFlags flags = Qt::Dialog;
	//flags |= Qt::WindowCloseButtonHint;
	//setWindowFlags(flags);  //去掉标题栏右上角的问号
	setAttribute(Qt::WA_DeleteOnClose);                                   //关闭窗口时将窗口释放掉即释放内存

	m_pIconLabel = new QLabel(this);
	m_pIconLabel->setObjectName(QLatin1String("iconLabel"));
	m_pIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);  //图标不可拉伸和缩小，固定大小
	m_pTextLabel = new QLabel(this);
	m_pTextLabel->setObjectName(QLatin1String("textLabel"));
	m_pTextLabel->setWordWrap(true);
	m_pTextLabel->setMinimumWidth(500);
	//m_pTextLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);  //label中的内容可用鼠标选择文本复制，链接激活
	m_pTextLabel->setOpenExternalLinks(true);                           //label中的内容若为链接，可直接点击打开
	m_pLineLabel = new QLabel(this);
	m_pLineLabel->setFrameStyle(QFrame::HLine | QFrame::Sunken);  //Sunken:凹陷，Raised：凸起
	m_pButtonBox = new QDialogButtonBox(this);                    //默认按钮为水平布局Qt::Horizontal
	m_pButtonBox->setObjectName(QLatin1String("buttonBox"));
	connect(m_pButtonBox, &QDialogButtonBox::clicked, this, &MyMessageBox::slotPushButtonClicked);

	setupLayout();

	setModal(true);

	//setStyleSheet("QDialog{background-color:rgb(255 , 255 , 255);}"
	//	"QPushButton{background-color:#E0E0E0;border: 1px solid #A6A6A6;border-radius:5px;}"
	//	"QPushButton:hover{color:white;background-color:#4188FF;border-radius:5px;border: 0px}"
	//	"QPushButton:pressed{padding-left:3px;padding-top:3px;}");
}

void MyMessageBox::setupLayout() {
	QHBoxLayout* HLay = new QHBoxLayout;
	HLay->addWidget(m_pIconLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
	HLay->addWidget(m_pTextLabel, 8, Qt::AlignCenter);
	QHBoxLayout* HLay1 = new QHBoxLayout;
	HLay1->addWidget(m_pButtonBox, Qt::AlignRight);
	QMargins margin;
	margin.setRight(9);
	margin.setBottom(6);
	HLay1->setContentsMargins(margin);  //调节按钮不要太靠右
	QVBoxLayout* VLay = new QVBoxLayout;
	VLay->addLayout(HLay, 10);
	VLay->addWidget(m_pLineLabel, 3);
	VLay->addLayout(HLay1, 0);
	VLay->setSpacing(0);

	QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
	mainLayout->setContentsMargins(1, 1, 1, 1);
	mainLayout->addLayout(VLay);
	//setLayout(VLay);
}

QPixmap MyMessageBox::standardIcon(Icon icon) {
	QPixmap pixmap;
	switch (icon) {
	case MyMessageBox::Information:
		pixmap.load(":/image/icon/images/info.png");
		break;
	case MyMessageBox::Question:
		pixmap.load(":/image/icon/images/question.png");
		break;
	case MyMessageBox::Success:
		pixmap.load(":/image/icon/images/completed.png");
		break;
	case MyMessageBox::Warning:
		pixmap.load(":/image/icon/images/warning.png");
		break;
	case MyMessageBox::Critical:
		pixmap.load(":/image/icon/images/error.png");
		break;
	default:
		break;
	}
	if (!pixmap.isNull())
		return pixmap;
	return QPixmap();
}

void MyMessageBox::setClickedButton(QAbstractButton* button) {
	m_pClickedButton = button;
}

void MyMessageBox::finalize(QAbstractButton* button) {
	int dialogCode = dialogCodeForButton(button);
	if (dialogCode == QDialog::Accepted) {
		emit accept();
	}
	else if (dialogCode == QDialog::Rejected) {
		emit reject();
	}
}

int MyMessageBox::dialogCodeForButton(QAbstractButton* button) const {
	switch (buttonRole(button)) {
	case MyMessageBox::AcceptRole:
	case MyMessageBox::YesRole:
		return QDialog::Accepted;
	case MyMessageBox::RejectRole:
	case MyMessageBox::NoRole:
		return QDialog::Rejected;
	default:
		return -1;
	}
}

void MyMessageBox::setPushButtonProperty(QList<QAbstractButton*> buttonList) {
	for (int i = 0; i < buttonList.size(); i++) {
		QPushButton* pushButton = qobject_cast<QPushButton*>(buttonList.at(i));
		pushButton->setMinimumSize(BUTTONWIDTH, BUTTONHEIGHT);
		pushButton->setFont(QFont(BUTTONFONTFAMILY, BUTTONFONTSIZE));
	}
}

