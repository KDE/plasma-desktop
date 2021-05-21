/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "caldialog.h"
#include "joydevice.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>

CalDialog::CalDialog(QWidget *parent, JoyDevice *joy)
    : QDialog(parent)
    , joydev(joy)
{
    setObjectName(QStringLiteral("calibrateDialog"));
    setModal(true);
    setWindowTitle(i18n("Calibration"));

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setSpacing(0);

    text = new QLabel(this);
    text->setMinimumHeight(200);
    valueLbl = new QLabel(this);

    main->addWidget(text);
    main->addWidget(valueLbl);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    QPushButton *Next = buttonBox->addButton(i18n("Next"), QDialogButtonBox::ApplyRole);
    Next->setDefault(true);
    main->addWidget(buttonBox);
    setLayout(main);

    connect(Next, &QPushButton::clicked, this, &CalDialog::slotNext);
}

void CalDialog::calibrate()
{
    text->setText(i18n("Please wait a moment to calculate the precision"));
    setResult(-1);
    show();

    // calibrate precision (which min,max delivers the joystick in its center position)
    // get values through the normal idle procedure
    QTimer ti;
    ti.setSingleShot(true); // single shot
    ti.start(2000); // in 2 seconds

    // normally I'd like to hide the 'Next' button in this step,
    // but it does not work - which means: in the steps after the first,
    // the 'Next' button does not have the focus (to be the default button)

    do {
        qApp->processEvents(QEventLoop::AllEvents, 2000);
    } while (ti.isActive() && (result() != QDialog::Rejected));

    if (result() == QDialog::Rejected)
        return; // user cancelled the dialog

    joydev->calcPrecision();

    int i, lastVal;
    int min[2], center[2], max[2];
    QString hint;

    for (i = 0; i < joydev->numAxes(); i++) {
        if (i == 0)
            hint = i18n("(usually X)");
        else if (i == 1)
            hint = i18n("(usually Y)");
        else
            hint = QLatin1String("");

        // minimum position
        text->setText(
            i18n("<qt>Calibration is about to check the value range your device delivers.<br /><br />"
                 "Please move <b>axis %1 %2</b> on your device to the <b>minimum</b> position.<br /><br />"
                 "Press any button on the device or click on the 'Next' button "
                 "to continue with the next step.</qt>",
                 i + 1,
                 hint));
        waitButton(i, true, lastVal);

        if (result() == QDialog::Rejected)
            return; // user cancelled the dialog

        joydev->resetMinMax(i, lastVal);
        if (result() != -2)
            waitButton(i, false, lastVal);

        if (result() == QDialog::Rejected)
            return; // user cancelled the dialog

        min[0] = joydev->axisMin(i);
        min[1] = joydev->axisMax(i);

        // center position
        text->setText(
            i18n("<qt>Calibration is about to check the value range your device delivers.<br /><br />"
                 "Please move <b>axis %1 %2</b> on your device to the <b>center</b> position.<br /><br />"
                 "Press any button on the device or click on the 'Next' button "
                 "to continue with the next step.</qt>",
                 i + 1,
                 hint));
        waitButton(i, true, lastVal);

        if (result() == QDialog::Rejected)
            return; // user cancelled the dialog

        joydev->resetMinMax(i, lastVal);
        if (result() != -2)
            waitButton(i, false, lastVal);

        if (result() == QDialog::Rejected)
            return; // user canceled the dialog

        center[0] = joydev->axisMin(i);
        center[1] = joydev->axisMax(i);

        // maximum position
        text->setText(
            i18n("<qt>Calibration is about to check the value range your device delivers.<br /><br />"
                 "Please move <b>axis %1 %2</b> on your device to the <b>maximum</b> position.<br /><br />"
                 "Press any button on the device or click on the 'Next' button "
                 "to continue with the next step.</qt>",
                 i + 1,
                 hint));
        waitButton(i, true, lastVal);

        if (result() == QDialog::Rejected)
            return; // user cancelled the dialog

        joydev->resetMinMax(i, lastVal);
        if (result() != -2)
            waitButton(i, false, lastVal);

        if (result() == QDialog::Rejected)
            return; // user canceled the dialog

        max[0] = joydev->axisMin(i);
        max[1] = joydev->axisMax(i);

        joydev->calcCorrection(i, min, center, max);
    }

    JoyDevice::ErrorCode ret = joydev->applyCalibration();

    if (ret != JoyDevice::SUCCESS) {
        KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
        reject();
    }

    KMessageBox::information(this, i18n("You have successfully calibrated your device"), i18n("Calibration Success"));
    accept();
}

void CalDialog::waitButton(int axis, bool press, int &lastVal)
{
    JoyDevice::EventType type;
    int number, value;
    bool button = false;
    lastVal = 0;

    setResult(-1);
    // loop until the user presses a button on the device or on the dialog
    do {
        qApp->processEvents(QEventLoop::AllEvents, 100);

        if (joydev->getEvent(type, number, value)) {
            button = ((type == JoyDevice::BUTTON) && (press ? (value == 1) : (value == 0)));

            if ((type == JoyDevice::AXIS) && (number == axis))
                valueLbl->setText(i18n("Value Axis %1: %2", axis + 1, lastVal = value));
        }
    } while (!button && (result() == -1));
}

// Next button

void CalDialog::slotNext()
{
    setResult(-2);
}
