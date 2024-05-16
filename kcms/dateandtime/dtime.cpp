/*
    Plasma analog-clock drawing code:

    SPDX-FileCopyrightText: 1998 Luca Montecchiani <m.luca@usa.net>
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dtime.h"

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <QGroupBox>
#include <QPainter>
#include <QPushButton>
#include <QTimeEdit>

#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KProcess>
#include <KTreeWidgetSearchLine>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <KSvg/Svg>

#include "timedated_interface.h"

#include "helper.h"

using namespace Qt::StringLiterals;

Dtime::Dtime(QWidget *parent, bool haveTimeDated)
    : QWidget(parent)
    , m_haveTimedated(haveTimeDated)
{
    setupUi(this);

    connect(setDateTimeAuto, &QCheckBox::toggled, this, &Dtime::configChanged);

    timeServerList->setEditable(false);
    connect(timeServerList, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &Dtime::configChanged);
    connect(timeServerList, &QComboBox::editTextChanged, this, &Dtime::configChanged);
    connect(setDateTimeAuto, &QCheckBox::toggled, timeServerList, &QComboBox::setEnabled);
    timeServerList->setEnabled(false);
    timeServerList->setEditable(true);

    if (!haveTimeDated) {
        findNTPutility();
        if (ntpUtility.isEmpty()) {
            QString toolTip = i18n(
                "No NTP utility has been found. "
                "Install 'ntpdate' or 'rdate' command to enable automatic "
                "updating of date and time.");
            setDateTimeAuto->setEnabled(false);
            setDateTimeAuto->setToolTip(toolTip);
            timeServerList->setToolTip(toolTip);
        }
    }

    QVBoxLayout *v2 = new QVBoxLayout(timeBox);
    v2->setContentsMargins(0, 0, 0, 0);

    kclock = new Kclock(timeBox);
    kclock->setObjectName(QStringLiteral("Kclock"));
    kclock->setMinimumSize(200, 200);
    v2->addWidget(kclock);

    QHBoxLayout *v3 = new QHBoxLayout();
    v2->addLayout(v3);

    v3->addStretch();

    timeEdit = new QTimeEdit(timeBox);
    timeEdit->setWrapping(true);
    v3->addWidget(timeEdit);

    v3->addStretch();

    QString wtstr = i18n(
        "Here you can change the system time. Click into the"
        " hours, minutes or seconds field to change the relevant value, either"
        " using the up and down buttons to the right or by entering a new value.");
    timeEdit->setWhatsThis(wtstr);

    connect(timeEdit, &QTimeEdit::timeChanged, this, &Dtime::set_time);
    connect(cal, &KDatePicker::dateChanged, this, &Dtime::changeDate);

    connect(&internalTimer, &QTimer::timeout, this, &Dtime::timeout);

    kclock->setEnabled(false);

    // Timezone
    connect(tzonelist, &K4TimeZoneWidget::itemSelectionChanged, this, &Dtime::configChanged);
    tzonesearch->setTreeWidget(tzonelist);
}

void Dtime::currentZone()
{
    QTimeZone localZone = QTimeZone::systemTimeZone();
    const auto continentCity = localZone.id().split('/');
    // Use the translation catalog of the digitalclock applet until  there is a standard API for city/continent names
    const char *domain = "plasma_applet_org.kde.plasma.digitalclock.mo";
    QString displayName = i18nd(domain, continentCity[0]);
    if (continentCity.size() > 1) {
        displayName += '/' + i18nd(domain, continentCity[1]);
    }
    const QString abbreviation = localZone.abbreviation(QDateTime::currentDateTime());
    if (abbreviation.isEmpty()) {
        m_local->setText(i18nc("%1 is name of time zone", "Current local time zone: %1", displayName));
    } else {
        m_local->setText(i18nc("%1 is name of time zone, %2 is its abbreviation", "Current local time zone: %1 (%2)", displayName, abbreviation));
    }
}

void Dtime::findNTPutility()
{
    QByteArray envpath = qgetenv("PATH");
    if (!envpath.isEmpty() && envpath.startsWith(':')) {
        envpath.remove(0, 1);
    }

    QStringList path = {u"/sbin"_s, u"/usr/sbin"_s};
    if (!envpath.isEmpty()) {
        path += QFile::decodeName(envpath).split(QLatin1Char(':'));
    } else {
        path += {u"/bin"_s, u"/usr/bin"_s};
    }

    for (const QString &possible_ntputility : {u"ntpdate"_s, u"rdate"_s}) {
        ntpUtility = QStandardPaths::findExecutable(possible_ntputility, path);
        if (!ntpUtility.isEmpty()) {
            qDebug() << "ntpUtility = " << ntpUtility;
            return;
        }
    }

    qDebug() << "ntpUtility not found!";
}

void Dtime::set_time()
{
    if (ontimeout)
        return;

    internalTimer.stop();

    time = timeEdit->time();
    kclock->setTime(time);

    Q_EMIT timeChanged(true);
}

void Dtime::changeDate(const QDate &d)
{
    date = d;
    Q_EMIT timeChanged(true);
}

void Dtime::configChanged()
{
    Q_EMIT timeChanged(true);
}

void Dtime::load()
{
    QString currentTimeZone;

    if (m_haveTimedated) {
        OrgFreedesktopTimedate1Interface timeDatedIface(QStringLiteral("org.freedesktop.timedate1"),
                                                        QStringLiteral("/org/freedesktop/timedate1"),
                                                        QDBusConnection::systemBus());
        // the server list is not relevant for timesyncd, it fetches it from the network
        timeServerList->setVisible(false);
        timeServerLabel->setVisible(false);
        setDateTimeAuto->setEnabled(timeDatedIface.canNTP());
        setDateTimeAuto->setChecked(timeDatedIface.nTP());

        currentTimeZone = timeDatedIface.timezone();
    } else {
        // The config is actually written to the system config, but the user does not have any local config,
        // since there is nothing writing it.
        KConfig _config(QStringLiteral("kcmclockrc"), KConfig::NoGlobals);
        KConfigGroup config(&_config, QStringLiteral("NTP"));
        timeServerList->clear();
        timeServerList->addItems(config
                                     .readEntry("servers", i18n("Public Time Server (pool.ntp.org),\
        asia.pool.ntp.org,\
        europe.pool.ntp.org,\
        north-america.pool.ntp.org,\
        oceania.pool.ntp.org"))
                                     .split(',', Qt::SkipEmptyParts));
        setDateTimeAuto->setChecked(config.readEntry("enabled", false));

        if (ntpUtility.isEmpty()) {
            timeServerList->setEnabled(false);
        }
        currentTimeZone = QTimeZone::systemTimeZoneId();
    }

    // Reset to the current date and time
    time = QTime::currentTime();
    date = QDate::currentDate();
    cal->setDate(date);

    // start internal timer
    internalTimer.start(1000);

    timeout();

    // Timezone
    currentZone();

    tzonelist->setSelected(currentTimeZone, true);
    Q_EMIT timeChanged(false);
}

QString Dtime::selectedTimeZone() const
{
    QStringList selectedZones(tzonelist->selection());
    if (!selectedZones.isEmpty()) {
        return selectedZones.first();
    }

    return QString();
}

QStringList Dtime::ntpServers() const
{
    // Save the order, but don't duplicate!
    QStringList list;
    if (timeServerList->count() != 0)
        list.append(timeServerList->currentText());
    for (int i = 0; i < timeServerList->count(); i++) {
        QString text = timeServerList->itemText(i);
        if (!list.contains(text))
            list.append(text);
        // Limit so errors can go away and not stored forever
        if (list.count() == 10)
            break;
    }
    return list;
}

bool Dtime::ntpEnabled() const
{
    return setDateTimeAuto->isChecked();
}

QDateTime Dtime::userTime() const
{
    return QDateTime(date, QTime(timeEdit->time()));
}

void Dtime::processHelperErrors(int code)
{
    if (code & ClockHelper::NTPError) {
        KMessageBox::error(this, i18n("Unable to contact time server: %1.", timeServer));
        setDateTimeAuto->setChecked(false);
    }
    if (code & ClockHelper::DateError) {
        KMessageBox::error(this, i18n("Can not set date."));
    }
    if (code & ClockHelper::TimezoneError)
        KMessageBox::error(this, i18n("Error setting new time zone."), i18n("Time zone Error"));
}

void Dtime::timeout()
{
    // get current time
    time = QTime::currentTime();

    ontimeout = true;
    timeEdit->setTime(time);
    ontimeout = false;

    kclock->setTime(time);
}

QString Dtime::quickHelp() const
{
    return i18n(
        "<h1>Date & Time</h1> This system settings module can be used to set the system date and"
        " time. As these settings do not only affect you as a user, but rather the whole system, you"
        " can only change these settings when you start the System Settings as root. If you do not have"
        " the root password, but feel the system time should be corrected, please contact your system"
        " administrator.");
}

Kclock::Kclock(QWidget *parent)
    : QWidget(parent)
{
    m_imageSet = new KSvg::ImageSet();
    m_imageSet->setBasePath("plasma/desktoptheme");
    auto plasmaConfig = KSharedConfig::openConfig(QStringLiteral("plasmarc"));
    m_imageSet->setImageSetName(plasmaConfig->group("Theme").readEntry("name", "default"));
    m_theme = new KSvg::Svg(this);
    m_theme->setImagePath(QStringLiteral("widgets/clock"));
    m_theme->setContainsMultipleImages(true);
}

Kclock::~Kclock()
{
    delete m_theme;
}

void Kclock::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setClockSize(size());
}

void Kclock::resizeEvent(QResizeEvent *)
{
    setClockSize(size());
}

bool Kclock::event(QEvent *event)
{
    if (event->type() == QEvent::DevicePixelRatioChange) {
        setClockSize(size());
    }
    return QWidget::event(event);
}

void Kclock::setClockSize(const QSize &size)
{
    int dim = qMin(size.width(), size.height());
    QSize newSize = QSize(dim, dim) * devicePixelRatioF();

    if (newSize != m_handsCache.size()) {
        m_faceCache = QPixmap(newSize);
        m_handsCache = QPixmap(newSize);
        m_glassCache = QPixmap(newSize);
        m_handsCache.setDevicePixelRatio(devicePixelRatioF());

        m_theme->resize(QSize(dim, dim));
        m_repaintCache = RepaintAll;
        update();
    }
}

void Kclock::setTime(const QTime &time)
{
    if (time.minute() != this->time.minute() || time.hour() != this->time.hour()) {
        if (m_repaintCache == RepaintNone) {
            m_repaintCache = RepaintHands;
        }
    }
    this->time = time;
    update();
}

void Kclock::drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName)
{
    // this code assumes the following conventions in the svg file:
    // - the _vertical_ position of the hands should be set with respect to the center of the face
    // - the _horizontal_ position of the hands does not matter
    // - the _shadow_ elements should have the same vertical position as their _hand_ element counterpart

    QRectF elementRect;
    QString name = handName + "HandShadow";
    if (m_theme->hasElement(name)) {
        p->save();

        elementRect = m_theme->elementRect(name);
        if (rect.height() < 64)
            elementRect.setWidth(elementRect.width() * 2.5);
        static const QPoint offset = QPoint(2, 3);

        p->translate(rect.x() + (rect.width() / 2) + offset.x(), rect.y() + (rect.height() / 2) + offset.y());
        p->rotate(rotation);
        p->translate(-elementRect.width() / 2, elementRect.y() - verticalTranslation);
        m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

        p->restore();
    }

    p->save();

    name = handName + "Hand";
    elementRect = m_theme->elementRect(name);
    if (rect.height() < 64) {
        elementRect.setWidth(elementRect.width() * 2.5);
    }

    p->translate(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    p->rotate(rotation);
    p->translate(-elementRect.width() / 2, elementRect.y() - verticalTranslation);
    m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

    p->restore();
}

void Kclock::paintInterface(QPainter *p, const QRect &rect)
{
    const bool m_showSecondHand = true;

    // compute hand angles
    const qreal minutes = 6.0 * time.minute() - 180;
    const qreal hours = 30.0 * time.hour() - 180 + ((time.minute() / 59.0) * 30.0);
    qreal seconds = 0;
    if (m_showSecondHand) {
        static const double anglePerSec = 6;
        seconds = anglePerSec * time.second() - 180;
    }

    // paint face and glass cache
    QRect faceRect = m_faceCache.rect();
    QRect targetRect = QRect(QPoint(0, 0), QSize(qRound(m_faceCache.width() / devicePixelRatioF()), qRound(m_faceCache.height() / devicePixelRatioF())));

    if (m_repaintCache == RepaintAll) {
        m_faceCache.fill(Qt::transparent);
        m_glassCache.fill(Qt::transparent);

        QPainter facePainter(&m_faceCache);
        QPainter glassPainter(&m_glassCache);
        facePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        glassPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        m_theme->paint(&facePainter, faceRect, QStringLiteral("ClockFace"));

        glassPainter.save();
        QRectF elementRect = QRectF(QPointF(0, 0), m_theme->elementSize(QStringLiteral("HandCenterScrew")));
        glassPainter.translate(faceRect.width() / 2.0 - elementRect.width() / 2.0, faceRect.height() / 2.0 - elementRect.height() / 2.0);
        m_theme->paint(&glassPainter, elementRect, QStringLiteral("HandCenterScrew"));
        glassPainter.restore();

        m_theme->paint(&glassPainter, faceRect, QStringLiteral("Glass"));

        // get vertical translation, see drawHand() for more details
        m_verticalTranslation = m_theme->elementRect(QStringLiteral("ClockFace")).center().y();
    }

    // paint hour and minute hands cache
    if (m_repaintCache == RepaintHands || m_repaintCache == RepaintAll) {
        m_handsCache.fill(Qt::transparent);

        QPainter handsPainter(&m_handsCache);
        handsPainter.drawPixmap(targetRect, m_faceCache, faceRect);
        handsPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        drawHand(&handsPainter, targetRect, m_verticalTranslation, hours, QStringLiteral("Hour"));
        drawHand(&handsPainter, targetRect, m_verticalTranslation, minutes, QStringLiteral("Minute"));
    }

    // reset repaint cache flag
    m_repaintCache = RepaintNone;

    // paint caches and second hand
    if (targetRect.width() < rect.width()) {
        targetRect.moveLeft((rect.width() - targetRect.width()) / 2);
    }

    p->drawPixmap(targetRect, m_handsCache, faceRect);
    if (m_showSecondHand) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        drawHand(p, targetRect, m_verticalTranslation, seconds, QStringLiteral("Second"));
    }
    p->drawPixmap(targetRect, m_glassCache, faceRect);
}

void Kclock::paintEvent(QPaintEvent *)
{
    QPainter paint(this);

    paint.setRenderHint(QPainter::Antialiasing);
    paintInterface(&paint, rect());
}
