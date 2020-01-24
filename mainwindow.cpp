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
#include <QDesktopServices>
#include <QDir>
#include <QDateTime>
#include <QClipboard>
#include <QTimer>
#include <QKeyEvent>

#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QApplication *Application) :
    QMainWindow(nullptr, Qt::FramelessWindowHint | Qt::MaximizeUsingFullscreenGeometryHint),
    ui(new Ui::MainWindow), resizeToMax(false), application(Application)
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
            this, &MainWindow::start_taking_screenshot, Qt::QueuedConnection);
    connect(screenshotview, &ScreenshotView::screenshotSelectionReady,
            this, &MainWindow::processSelection, Qt::QueuedConnection);

    connect(&im, &ImgurManager::requestClientIdAndSecret,
            this, &MainWindow::processClientIdAndSecretRequest, Qt::QueuedConnection);
    connect(&im, &ImgurManager::requestPin,
            this, &MainWindow::processPinRequest, Qt::QueuedConnection);
    connect(&im, &ImgurManager::uploaded,
            this, &MainWindow::processUploaded, Qt::QueuedConnection);


    inputDialog = new InputDialog();

    connect(inputDialog, &InputDialog::done,
            this, &MainWindow::processDialogResults, Qt::QueuedConnection);


    im.checkState();
    if (im.getState() == ImgurManager::State::ReadyToUpload)
    {
        start_taking_screenshot();
        show();
    }
    else
        hide();
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
    emit move(0, 0);
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
    emit hide();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timestamp = currentDateTime.toString("yyyy.MM.dd hh:mm:ss");

    QString tempPath = QDir::tempPath();

    QString filePath = tempPath + "/" + timestamp + ".png";

    pixmap.save(filePath);

    im.upload(filePath, "imgshot " + timestamp);
}

void MainWindow::processClientIdAndSecretRequest(QUrl link)
{
    QDesktopServices::openUrl(link);

    inputDialog->reset();

    QString labelText = "You need to generate a Client-Id and a Client-Secret first.\n"
                        "In the following site Enter an Application name, for example \"imgurshot\".\n"
                        "Choose \"OAuth 2 authorization without a callback URL\" as the Authorization type.\n"
                        "Input your e-mail address you used for imgur.\n"
                        + link.toString();
    inputDialog->setLabelText(labelText);

    inputDialog->addInput("Client ID:", "clientId");
    inputDialog->addInput("Client secret:", "clientSecret");

    inputDialog->show();
}

void MainWindow::processPinRequest(QUrl link)
{
    QDesktopServices::openUrl(link);

    inputDialog->reset();

    QString labelText = "You need to generate get the PIN.\n"
                        "In the following site log in and copypaste the PIN\n"
                        + link.toString();
    inputDialog->setLabelText(labelText);

    inputDialog->addInput("PIN:", "pin");

    inputDialog->show();
}

void MainWindow::processDialogResults()
{
    if (im.getState() == ImgurManager::State::ClientIdAndSecretNeeded)
    {
        QString clientId = inputDialog->getLineEditText("clientId").trimmed();
        QString clientSecret = inputDialog->getLineEditText("clientSecret").trimmed();

        im.setClientIdAndSecret(clientId, clientSecret);

        inputDialog->close();
    }
    else if (im.getState() == ImgurManager::State::WaitingForPin)
    {
        QString pin = inputDialog->getLineEditText("pin").trimmed();

        im.processPin(pin);

        inputDialog->close();
    }
}

void MainWindow::processUploaded(QString link)
{
    emit QApplication::clipboard()->setText(link);
    qDebug() << QApplication::clipboard()->text();

    //Currently bad hack
    QTimer *closeTimer  = new QTimer(this);
    closeTimer->setSingleShot(true);
    closeTimer->setInterval(1000);
    connect(closeTimer, &QTimer::timeout, this, [this]
    {
        application->quit();
    }, Qt::QueuedConnection);
    emit closeTimer->start();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        application->quit();
    }
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    if (im.getState() == ImgurManager::State::ReadyToUpload
            && size() == resizeTo
            && pos() != QPoint(0, 0))
    {
        emit move(0, 0);
    }
}



