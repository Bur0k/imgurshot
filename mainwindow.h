#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "screenshotview.h"
#include <QGraphicsScene>
#include <QMouseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Ui::MainWindow *ui;

    ScreenshotView *screenshotview;


    void setupWindow(QSize size);
    QPixmap takeScreenshot();

    bool resizeToMax;
    QSize resizeTo;


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void start_taking_screenshot();
    void processSelection(QPixmap pixmap);

private:

};

#endif // MAINWINDOW_H
