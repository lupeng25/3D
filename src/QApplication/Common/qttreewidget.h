#pragma once

#include <QTreeWidget>
#include <QMenu>

class QtTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	explicit QtTreeWidget(QWidget* parent = 0);
	~QtTreeWidget();

protected:
	//bool eventFilter(QObject *watched, QEvent *event);
	//virtual void mousePressEvent(QMouseEvent *event);
private:
	QPixmap SetDragCurorPixmap(QPixmap pixmap, QString strText);

private slots:
	void ItemsExpanded();
	void ItemsCollapse();
protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void startDrag(Qt::DropActions supportedActions) override;

private:
	QMenu *			videoMenu;			//右键菜单
	QAction*		actionCollapse = nullptr;
	QAction*		actionExpanded = nullptr;

};
