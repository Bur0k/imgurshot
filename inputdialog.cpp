#include "inputdialog.h"

#include <QGridLayout>

void InputDialog::deleteAll(QLayout *layout)
{
    while(auto item = layout->takeAt(0))
    {
        if(item->widget())
            delete item->widget();
        else if(item->layout())
            deleteAll(item->layout());
    }
}

InputDialog::InputDialog(QWidget *parent) : QDialog(parent)
{
    mainLayout = new QVBoxLayout();
    inputsLayout = new QGridLayout();
    okayButton = new QPushButton();
    informationText = new QLabel();

    currentRow = 0;

    setLayout(mainLayout);

    mainLayout->addWidget(informationText);
    mainLayout->addLayout(inputsLayout);
    mainLayout->addWidget(okayButton);

    okayButton->setText("Set");
    informationText->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    connect(okayButton, &QPushButton::clicked,
            this, &InputDialog::pressedOkayButton);
}

void InputDialog::reset()
{
    deleteAll(inputsLayout);

    inputs.clear();

    currentRow = 0;
    informationText->setText("");
}

void InputDialog::setLabelText(QString labelText)
{
    informationText->setText(labelText);
}

void InputDialog::addInput(QString labelText, QString key)
{
    QLabel *lineLabel = new QLabel(this);
    lineLabel->setText(labelText);
    inputs[key] = new QLineEdit(this);

    inputsLayout->addWidget(lineLabel, currentRow, 0);
    inputsLayout->addWidget(inputs[key], currentRow, 1);

    currentRow++;
}

void InputDialog::pressedOkayButton()
{
    emit done();
}

QString InputDialog::getLineEditText(QString key)
{
    return inputs[key]->text();
}



