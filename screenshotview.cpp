#include "screenshotview.h"

#include <QDebug>
#include <QGraphicsLineItem>
#include <QPoint>
#include <QPolygon>
#include <QPainter>

ScreenshotView::ScreenshotView(QGraphicsScene *scene, QWidget *parent)
    :QGraphicsView (scene, parent), qgs(scene),
      transparentGreyBrush(QColor::fromRgb(0, 0, 0, 170)),
      mousePen(QColor::fromRgb(255, 20, 20, 255), 1)
{
    reset();
}

void ScreenshotView::reset()
{
    firstPoint = secondPoint = QPoint(-1, -1);
    xMouseLine = yMouseLine = nullptr;
    topRightPoly = bottomLeftPoly = nullptr;
    creatingScreenshot = false;
    qgs->clear();
}

void ScreenshotView::startSelection()
{
    creatingScreenshot = true;
}

void ScreenshotView::setScreenshot(QPixmap pixmap)
{
    wholeScreen = pixmap;
    qgs->addPixmap(wholeScreen);
}

void ScreenshotView::processSelection()
{
    QPoint tl = firstPoint;
    QPoint br = secondPoint;

    if (tl.x() > br.x())
    {
        int tmp = tl.x();
        tl.setX(br.x());
        br.setX(tmp);
    }
    if (tl.y() > br.y())
    {
        int tmp = tl.y();
        tl.setY(br.y());
        br.setY(tmp);
    }
    int w = br.x() - tl.x();
    int h = br.y() - tl.y();
    QPixmap selection(w, h);
    QPainter painter(&selection);

    painter.drawPixmap(QRect(0, 0, w, h), wholeScreen, QRect(tl, br));

    emit screenshotSelectionReady(selection);
}


void ScreenshotView::mousePressEvent(QMouseEvent *event)
{
    if(creatingScreenshot)
    {
        if(event->button() == Qt::LeftButton)
        {
            firstPoint = event->pos();
        }
    }
}

void ScreenshotView::mouseReleaseEvent(QMouseEvent *event)
{
    if(creatingScreenshot)
    {
        if (firstPoint.x() != -1)
        {
            secondPoint = event->pos();
            creatingScreenshot = false;
            processSelection();
            reset();
        }
    }
}

void ScreenshotView::mouseMoveEvent(QMouseEvent *event)
{
    if(creatingScreenshot)
    {
        static QPoint oldPos;
        QPoint mousePos = event->pos();
        if (oldPos == mousePos)
            return;
        oldPos = mousePos;


        if (firstPoint.x() != -1)
        {
            QPoint tl = firstPoint;
            QPoint br = mousePos;

            if (tl.x() > br.x())
            {
                int tmp = tl.x();
                tl.setX(br.x());
                br.setX(tmp);
            }
            if (tl.y() > br.y())
            {
                int tmp = tl.y();
                tl.setY(br.y());
                br.setY(tmp);
            }

            if (topRightPoly)
                qgs->removeItem(topRightPoly);
            if (bottomLeftPoly)
                qgs->removeItem(bottomLeftPoly);

            QPolygon topRight;
            topRight << QPoint(0, 0) << QPoint(size().width(), 0);
            topRight << QPoint(size().width(), br.y()) << QPoint(br.x(), br.y());
            topRight << QPoint(br.x(), tl.y()) << QPoint(0, tl.y());
            topRightPoly = new QGraphicsPolygonItem(topRight);

            QPolygon bottomLeft;
            bottomLeft << QPoint(0, tl.y()) << QPoint(0, size().height());
            bottomLeft << QPoint(size().width(), size().height()) << QPoint(size().width(), br.y());
            bottomLeft << QPoint(tl.x(), br.y()) << QPoint(tl.x(), tl.y());
            bottomLeftPoly = new QGraphicsPolygonItem(bottomLeft);


            topRightPoly->setBrush(transparentGreyBrush);
            topRightPoly->setPen(Qt::NoPen);
            bottomLeftPoly->setBrush(transparentGreyBrush);
            bottomLeftPoly->setPen(Qt::NoPen);

            qgs->addItem(topRightPoly);
            qgs->addItem(bottomLeftPoly);
        }

        if (xMouseLine)
            qgs->removeItem(xMouseLine);
        if (yMouseLine)
            qgs->removeItem(yMouseLine);

        xMouseLine = qgs->addLine(mousePos.x() - 10, mousePos.y(), mousePos.x() + 10, mousePos.y(), mousePen);
        yMouseLine = qgs->addLine(mousePos.x(), mousePos.y() - 10, mousePos.x(), mousePos.y() + 10, mousePen);
    }
}
