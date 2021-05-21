/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003, 2012 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joywidget.h"
#include "caldialog.h"
#include "joydevice.h"
#include "poswidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageWidget>
#include <KUrlCompletion>

#include <stdio.h>

static QString PRESSED = I18N_NOOP("PRESSED");

class TableWidget : public QTableWidget
{
public:
    TableWidget(int row, int col)
        : QTableWidget(row, col)
    {
    }

    QSize sizeHint() const override
    {
        return QSize(150, 100); // return a smaller size than the Qt default(256, 192)
    }
};

JoyWidget::JoyWidget(QWidget *parent)
    : QWidget(parent)
    , idle(nullptr)
    , joydev(nullptr)
{
    QVBoxLayout *mainVbox = new QVBoxLayout(this);
    int defaultSpacing = style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    mainVbox->setSpacing(defaultSpacing);
    mainVbox->setContentsMargins(0, 0, 0, 0);

    // create area to show an icon + message if no joystick was detected
    {
        messageBox = new KMessageWidget(this);
        messageBox->setMessageType(KMessageWidget::Error);
        messageBox->setCloseButtonVisible(false);
        messageBox->hide();
        messageBox->setWordWrap(true);

        mainVbox->addWidget(messageBox);
    }

    QHBoxLayout *devHbox = new QHBoxLayout;
    devHbox->setSpacing(defaultSpacing);
    devHbox->addWidget(new QLabel(i18n("Device:")));
    devHbox->addWidget(device = new KComboBox(true));

    device->setInsertPolicy(QComboBox::NoInsert);
    KUrlCompletion *kc = new KUrlCompletion(KUrlCompletion::FileCompletion);
    device->setCompletionObject(kc);
    device->setAutoDeleteCompletionObject(true);
    connect(device, SIGNAL(activated(QString)), this, SLOT(deviceChanged(QString)));
    connect(device, SIGNAL(returnPressed(QString)), this, SLOT(deviceChanged(QString)));
    devHbox->setStretchFactor(device, 3);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing(defaultSpacing);

    mainVbox->addLayout(devHbox);
    mainVbox->addLayout(hbox);

    QVBoxLayout *vboxLeft = new QVBoxLayout;
    vboxLeft->setSpacing(defaultSpacing);
    vboxLeft->addWidget(new QLabel(i18nc("Cue for deflection of the stick", "Position:")));
    vboxLeft->addWidget(xyPos = new PosWidget);

    vboxLeft->addWidget(trace = new QCheckBox(i18n("Show trace")));
    connect(trace, &QAbstractButton::toggled, this, &JoyWidget::traceChanged);

    QVBoxLayout *vboxMid = new QVBoxLayout;
    vboxMid->setSpacing(defaultSpacing);

    QVBoxLayout *vboxRight = new QVBoxLayout;
    vboxRight->setSpacing(defaultSpacing);

    // calculate the column width we need
    QFontMetrics fm(font());
    int colWidth = qMax(fm.horizontalAdvance(PRESSED), fm.horizontalAdvance(QStringLiteral("-32767"))) + 10; // -32767 largest string

    vboxMid->addWidget(new QLabel(i18n("Buttons:")));
    buttonTbl = new TableWidget(0, 1);
    buttonTbl->setSelectionMode(QAbstractItemView::NoSelection);
    buttonTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    buttonTbl->setHorizontalHeaderLabels(QStringList(i18n("State")));
    buttonTbl->setSortingEnabled(false);
    buttonTbl->horizontalHeader()->setSectionsClickable(false);
    buttonTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    buttonTbl->horizontalHeader()->resizeSection(0, colWidth);
    buttonTbl->verticalHeader()->setSectionsClickable(false);
    vboxMid->addWidget(buttonTbl);

    vboxRight->addWidget(new QLabel(i18n("Axes:")));
    axesTbl = new TableWidget(0, 1);
    axesTbl->setSelectionMode(QAbstractItemView::NoSelection);
    axesTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    axesTbl->setHorizontalHeaderLabels(QStringList(i18n("Value")));
    axesTbl->setSortingEnabled(false);
    axesTbl->horizontalHeader()->setSectionsClickable(false);
    axesTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    axesTbl->horizontalHeader()->resizeSection(0, colWidth);
    axesTbl->verticalHeader()->setSectionsClickable(false);
    vboxRight->addWidget(axesTbl);

    hbox->addLayout(vboxLeft);
    hbox->addLayout(vboxMid);
    hbox->addLayout(vboxRight);

    // calibrate button
    calibrate = new QPushButton(i18n("Calibrate"));
    connect(calibrate, &QAbstractButton::clicked, this, &JoyWidget::calibrateDevice);
    calibrate->setEnabled(false);

    vboxLeft->addStretch();
    vboxLeft->addWidget(calibrate);

    // set up a timer for idle processing of joystick events
    idle = new QTimer(this);
    connect(idle, &QTimer::timeout, this, &JoyWidget::checkDevice);

    // check which devicefiles we have
    init();
}

JoyWidget::~JoyWidget()
{
    delete joydev;
}

void JoyWidget::init()
{
    // check which devicefiles we have
    int i;
    bool first = true;
    char dev[30];

    device->clear();
    buttonTbl->setRowCount(0);
    axesTbl->setRowCount(0);

    for (i = 0; i < 5; i++) // check the first 5 devices
    {
        sprintf(dev, "/dev/js%d", i); // first look in /dev
        JoyDevice *joy = new JoyDevice(dev);

        if (joy->open() != JoyDevice::SUCCESS) {
            delete joy;
            sprintf(dev, "/dev/input/js%d", i); // then look in /dev/input
            joy = new JoyDevice(dev);

            if (joy->open() != JoyDevice::SUCCESS) {
                delete joy;
                continue; // try next number
            }
        }

        // we found one

        device->addItem(QStringLiteral("%1 (%2)").arg(joy->text()).arg(joy->device()));

        // display values for first device
        if (first) {
            showDeviceProps(joy); // this sets the joy object into this->joydev
            first = false;
        } else
            delete joy;
    }

    /* KDE 4: Remove this check(and i18n) when all KCM wrappers properly test modules */
    if (device->count() == 0) {
        messageBox->show();
        messageBox->setText(QStringLiteral("<qt>%1</qt>")
                                .arg(i18n("No joystick device automatically found on this computer.<br />"
                                          "Checks were done in /dev/js[0-4] and /dev/input/js[0-4]<br />"
                                          "If you know that there is one attached, please enter the correct device file.")));
    }
}

void JoyWidget::traceChanged(bool state)
{
    xyPos->showTrace(state);
}

void JoyWidget::restoreCurrDev()
{
    if (!joydev) // no device open
    {
        device->setEditText(QLatin1String(""));
        calibrate->setEnabled(false);
    } else {
        // try to find the current open device in the combobox list
        int index = device->findText(joydev->device(), Qt::MatchContains);

        if (index == -1) // the current open device is one the user entered (not in the list)
            device->setEditText(joydev->device());
        else
            device->setEditText(device->itemText(index));
    }
}

void JoyWidget::deviceChanged(const QString &dev)
{
    // find "/dev" in given string
    int start, stop;
    QString devName;

    if ((start = dev.indexOf(QLatin1String("/dev"))) == -1) {
        KMessageBox::sorry(this,
                           i18n("The given device name is invalid (does not contain /dev).\n"
                                "Please select a device from the list or\n"
                                "enter a device file, like /dev/js0."),
                           i18n("Unknown Device"));

        restoreCurrDev();
        return;
    }

    if ((stop = dev.indexOf(QLatin1Char(')'), start)) != -1) // seems to be text selected from our list
        devName = dev.mid(start, stop - start);
    else
        devName = dev.mid(start);

    if (joydev && (devName == joydev->device()))
        return; // user selected the current device; ignore it

    JoyDevice *joy = new JoyDevice(devName);
    JoyDevice::ErrorCode ret = joy->open();

    if (ret != JoyDevice::SUCCESS) {
        KMessageBox::error(this, joy->errText(ret), i18n("Device Error"));

        delete joy;
        restoreCurrDev();
        return;
    }

    showDeviceProps(joy);
}

void JoyWidget::showDeviceProps(JoyDevice *joy)
{
    joydev = joy;

    buttonTbl->setRowCount(joydev->numButtons());

    axesTbl->setRowCount(joydev->numAxes());
    if (joydev->numAxes() >= 2) {
        axesTbl->setVerticalHeaderItem(0, new QTableWidgetItem(i18n("1(x)")));
        axesTbl->setVerticalHeaderItem(1, new QTableWidgetItem(i18n("2(y)")));
    }

    calibrate->setEnabled(true);
    idle->start(0);

    // make both tables use the same space for header; this looks nicer
    // TODO: Don't know how to do this in Qt4; the following does no longer work
    // Probably by setting a sizeHint for every single header item ?
    /*
    buttonTbl->verticalHeader()->setFixedWidth(qMax(buttonTbl->verticalHeader()->width(),
                                                      axesTbl->verticalHeader()->width()));
    axesTbl->verticalHeader()->setFixedWidth(buttonTbl->verticalHeader()->width());
    */
}

void JoyWidget::checkDevice()
{
    if (!joydev)
        return; // no open device yet

    JoyDevice::EventType type;
    int number, value;

    if (!joydev->getEvent(type, number, value))
        return;

    if (type == JoyDevice::BUTTON) {
        if (!buttonTbl->item(number, 0))
            buttonTbl->setItem(number, 0, new QTableWidgetItem());

        if (value == 0) // button release
            buttonTbl->item(number, 0)->setText(QStringLiteral("-"));
        else
            buttonTbl->item(number, 0)->setText(PRESSED);
    }

    if (type == JoyDevice::AXIS) {
        if (number == 0) // x-axis
            xyPos->changeX(value);

        if (number == 1) // y-axis
            xyPos->changeY(value);

        if (!axesTbl->item(number, 0))
            axesTbl->setItem(number, 0, new QTableWidgetItem());

        axesTbl->item(number, 0)->setText(QStringLiteral("%1").arg(int(value)));
    }
}

void JoyWidget::calibrateDevice()
{
    if (!joydev)
        return; // just to be save

    JoyDevice::ErrorCode ret = joydev->initCalibration();

    if (ret != JoyDevice::SUCCESS) {
        KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
        return;
    }

    if (KMessageBox::messageBox(this,
                                KMessageBox::Information,
                                i18n("<qt>Calibration is about to check the precision.<br /><br />"
                                     "<b>Please move all axes to their center position and then "
                                     "do not touch the joystick anymore.</b><br /><br />"
                                     "Click OK to start the calibration.</qt>"),
                                i18n("Calibration"),
                                KStandardGuiItem::ok(),
                                KStandardGuiItem::cancel())
        != KMessageBox::Ok)
        return;

    idle->stop(); // stop the joystick event getting; this must be done inside the calibrate dialog

    CalDialog dlg(this, joydev);
    dlg.calibrate();

    // user canceled somewhere during calibration, therefore the device is in a bad state
    if (dlg.result() == QDialog::Rejected)
        joydev->restoreCorr();

    idle->start(0); // continue with event getting
}

void JoyWidget::resetCalibration()
{
    if (!joydev)
        return; // just to be save

    JoyDevice::ErrorCode ret = joydev->restoreCorr();

    if (ret != JoyDevice::SUCCESS) {
        KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
    } else {
        KMessageBox::information(this, i18n("Restored all calibration values for joystick device %1.", joydev->device()), i18n("Calibration Success"));
    }
}
