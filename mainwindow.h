#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "screenshotview.h"
#include <QGraphicsScene>
#include <QMouseEvent>
#include "imgurmanager.h"
#include <QSettings>
#include "inputdialog.h"

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

    ImgurManager im;

    InputDialog *inputDialog;

    QApplication *application;
public:
    explicit MainWindow(QApplication *application);
    ~MainWindow();

public slots:
    void start_taking_screenshot();
    void processSelection(QPixmap pixmap);

    void processClientIdAndSecretRequest(QUrl link);
    void processPinRequest(QUrl link);

    void processDialogResults();

    void processUploaded(QString link);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
private:

};

#endif // MAINWINDOW_H
