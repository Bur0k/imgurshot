#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QDebug>
#include <string>
#include <QString>
#include <QPainter>
#include <QColor>
#include <QGraphicsLineItem>

#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent, Qt::FramelessWindowHint | Qt::MaximizeUsingFullscreenGeometryHint),
    ui(new Ui::MainWindow), resizeToMax(false)
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);

    screenshotview = new ScreenshotView(scene, this);


    screenshotview->show();
    setCentralWidget(screenshotview);
    centralWidget()->setMouseTracking(true);


    screenshotview->setStyleSheet( "QGraphicsView { border-style: none; }" );
    screenshotview->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    screenshotview->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

    QShortcut *shortcut= new QShortcut(QKeySequence(Qt::Key::Key_A), this);

    connect(shortcut, &QShortcut::activated,
            this, &MainWindow::start_taking_screenshot);
    connect(screenshotview, &ScreenshotView::screenshotSelectionReady,
            this, &MainWindow::processSelection);

    start_taking_screenshot();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupWindow(QSize size)
{
    this->setMinimumSize(size);
    this->setMaximumSize(size);
    resizeToMax = true;
    resizeTo = size;
    resize(size);
    move(0, 0);
}

QPixmap MainWindow::takeScreenshot()
{
    auto screens = QGuiApplication::screens();

    QRect none = QRect(-1,-1,-1,-1);
    QRect virtualSize = none;
    for(auto screen: screens)
    {
        QRect virtualGeometry = screen->virtualGeometry();
        if (virtualSize == none)
            virtualSize = virtualGeometry;
        else if (virtualGeometry != virtualSize)
            qDebug() << "Different virtual geometries!";
    }

    QPixmap screenshot(virtualSize.width(), virtualSize.height());
    QPainter resultPainter(&screenshot);

    for(auto screen: screens)
    {
        QPixmap screenPixmap = screen->grabWindow(0);

        QRect screenGeometry = screen->geometry();

        resultPainter.drawPixmap(screenGeometry, screenPixmap, QRect(0,0,screen->size().width(),screen->size().height()));
    }
    return screenshot;
}

void MainWindow::start_taking_screenshot()
{
    screenshotview->reset();

    QPixmap screenshot = takeScreenshot();
    setupWindow(screenshot.size());

    screenshotview->setScreenshot(screenshot);
    screenshotview->startSelection();
}

void MainWindow::processSelection(QPixmap pixmap)
{
    pixmap.save("result.png");
    close();
}


