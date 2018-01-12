#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QWidget>
#include <QList>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>

class InputDialog : public QDialog
{
    Q_OBJECT

    QVBoxLayout *mainLayout;
    QGridLayout *inputsLayout;

    QMap<QString, QLineEdit*> inputs;
    int currentRow;
    QLabel *informationText;
    QPushButton *okayButton;

    void deleteAll(QLayout *layout);
public:
    explicit InputDialog(QWidget *parent = nullptr);

    void reset();
    void setLabelText(QString labelText);
    void addInput(QString labelText, QString key);

    QString getLineEditText(QString key);

signals:
    void done();

public slots:
    void pressedOkayButton();
};

#endif // INPUTDIALOG_H
