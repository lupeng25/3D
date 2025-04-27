#pragma once

#include <QGraphicsScene>

#ifndef __QGRAPHICSSCENES_EXPORT_
#define _QGRAPHICSSCENES_API_ _declspec(dllexport)
#else
#define _QGRAPHICSSCENES_API_ _declspec(dllimport)
#endif // !__QGRAPHICSSCENES_EXPORT_

class _QGRAPHICSSCENES_API_ QGraphicsScenes : public QGraphicsScene
{
    Q_OBJECT

public:
    QGraphicsScenes(QObject *parent = nullptr);
    void startCreate();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    void updatePoint(QPointF p, QList<QPointF> list, bool isCenter);
    void createFinished();

protected:
    QList<QPointF> Plist;
    bool PolygonFlg;
};
