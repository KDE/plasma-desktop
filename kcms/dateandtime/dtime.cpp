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
#include <QQmlEngine>
#include <QTimeEdit>

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KProcess>
#include <KTreeWidgetSearchLine>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QQmlContext>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KLocalizedContext>
#include <KSvg/Svg>

#include "timedated_interface.h"

using namespace Qt::StringLiterals;

Dtime::Dtime(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(setDateTimeAuto, &QCheckBox::toggled, this, &Dtime::configChanged);

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

    tabWidget->tabBar()->setExpanding(true);

    auto engine = timezoneViewer->engine();
    engine->rootContext()->setContextObject(new KLocalizedContext(engine));

    timezoneViewer->rootContext()->setContextProperty("DTime", this);
    timezoneViewer->setSource(QUrl("qrc:/kcm/kcm_clock/main.qml"));
    timezoneViewer->resize(QSize(500, 600));
    timezoneViewer->setResizeMode(QQuickWidget::SizeRootObjectToView);
    timezoneViewer->setClearColor(Qt::transparent);
    timezoneViewer->setAttribute(Qt::WA_AlwaysStackOnTop);
}

void Dtime::currentZone()
{
    QTimeZone localZone = QTimeZone::systemTimeZone();
    setSelectedTimeZone(localZone.id());
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

    OrgFreedesktopTimedate1Interface timeDatedIface(QStringLiteral("org.freedesktop.timedate1"),
                                                    QStringLiteral("/org/freedesktop/timedate1"),
                                                    QDBusConnection::systemBus());
    setDateTimeAuto->setEnabled(timeDatedIface.canNTP());
    setDateTimeAuto->setChecked(timeDatedIface.nTP());

    currentTimeZone = timeDatedIface.timezone();

    // Reset to the current date and time
    time = QTime::currentTime();
    date = QDate::currentDate();
    cal->setDate(date);

    // start internal timer
    internalTimer.start(1000);

    timeout();

    // Timezone
    currentZone();

    Q_EMIT timeChanged(false);
}

QString Dtime::selectedTimeZone() const
{
    return m_selectedTimeZone;
}

void Dtime::setSelectedTimeZone(QString selectedTimeZone)
{
    if (m_selectedTimeZone == selectedTimeZone) {
        return;
    }

    m_selectedTimeZone = selectedTimeZone;
    Q_EMIT selectedTimeZoneChanged(true);
}

bool Dtime::ntpEnabled() const
{
    return setDateTimeAuto->isChecked();
}

QDateTime Dtime::userTime() const
{
    return QDateTime(date, QTime(timeEdit->time()));
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

#include "moc_dtime.cpp"
