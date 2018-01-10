#ifndef SCREENSHOTVIEW_H
#define SCREENSHOTVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QGraphicsPolygonItem>
#include <QPixmap>

class ScreenshotView : public QGraphicsView
{
    Q_OBJECT

    bool creatingScreenshot;
    QGraphicsScene* qgs;

    QGraphicsPolygonItem* topRightPoly;
    QGraphicsPolygonItem* bottomLeftPoly;

    QPoint firstPoint;
    QPoint secondPoint;

    QGraphicsLineItem *xMouseLine;
    QGraphicsLineItem *yMouseLine;

    QPixmap wholeScreen;

    QBrush transparentGreyBrush;
    QPen mousePen;
public:
    ScreenshotView(QGraphicsScene *scene, QWidget *parent = nullptr);

    void reset();
    void startSelection();

    void setScreenshot(QPixmap pixmap);
signals:
    void screenshotSelectionReady(QPixmap pixmap);

private:
    void processSelection();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // SCREENSHOTVIEW_H
