/*
 *  dtime.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
 *
 *  Plasma analog-clock drawing code:
 *
 *  Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *  Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include "dtime.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QPainter>
#include <QTimeEdit>

#include <QCheckBox>
#include <QPaintEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kcolorscheme.h>
#include <ksystemtimezone.h>
#include <KTreeWidgetSearchLine>
#include <KGlobal>

#include <Plasma/Svg>


#include "helper.h"

Dtime::Dtime(QWidget * parent)
  : QWidget(parent)
{
  setupUi(this);

  connect(setDateTimeAuto, &QCheckBox::toggled, this, &Dtime::serverTimeCheck);
  connect(setDateTimeAuto, &QCheckBox::toggled, this, &Dtime::configChanged);

  timeServerList->setEditable(false);
  connect(timeServerList, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &Dtime::configChanged);
  connect(timeServerList, &QComboBox::editTextChanged, this, &Dtime::configChanged);
  connect(setDateTimeAuto, &QCheckBox::toggled, timeServerList, &QComboBox::setEnabled);
  timeServerList->setEnabled(false);
  timeServerList->setEditable(true);
  findNTPutility();
  if (ntpUtility.isEmpty()) {
      QString toolTip = i18n("No NTP utility has been found. "
                             "Install 'ntpdate' or 'rdate' command to enable automatic "
                             "updating of date and time.");
      setDateTimeAuto->setEnabled(false);
      setDateTimeAuto->setToolTip(toolTip);
      timeServerList->setToolTip(toolTip);
  }

  QVBoxLayout *v2 = new QVBoxLayout( timeBox );
  v2->setMargin( 0 );

  kclock = new Kclock( timeBox );
  kclock->setObjectName("Kclock");
  kclock->setMinimumSize(150,150);
  v2->addWidget( kclock );

  v2->addSpacing( KDialog::spacingHint() );

  QHBoxLayout *v3 = new QHBoxLayout( );
  v2->addLayout( v3 );

  v3->addStretch();

  timeEdit = new QTimeEdit( timeBox );
  timeEdit->setWrapping(true);
  timeEdit->setDisplayFormat(KLocale::global()->use12Clock() ? "hh:mm:ss ap" : "HH:mm:ss");
  v3->addWidget(timeEdit);

  v3->addStretch();

  QString wtstr = i18n("Here you can change the system time. Click into the"
    " hours, minutes or seconds field to change the relevant value, either"
    " using the up and down buttons to the right or by entering a new value.");
  timeEdit->setWhatsThis( wtstr );

  connect(timeEdit, &QTimeEdit::timeChanged, this, &Dtime::set_time);
  connect(cal, &KDatePicker::dateChanged, this, &Dtime::changeDate);

  connect(&internalTimer, &QTimer::timeout, this, &Dtime::timeout);

  kclock->setEnabled(false);

  //Timezone
  connect(tzonelist, &K4TimeZoneWidget::itemSelectionChanged, this, &Dtime::configChanged);
  tzonesearch->setTreeWidget(tzonelist);
}

void Dtime::currentZone()
{
    KTimeZone localZone = KSystemTimeZones::local();

    if (localZone.abbreviations().isEmpty()) {
        m_local->setText(i18nc("%1 is name of time zone", "Current local time zone: %1",
                              K4TimeZoneWidget::displayName(localZone)));
    } else {
        m_local->setText(i18nc("%1 is name of time zone, %2 is its abbreviation",
                               "Current local time zone: %1 (%2)",
                              K4TimeZoneWidget::displayName(localZone),
                              QString::fromUtf8(localZone.abbreviations().first())));
    }
}

void Dtime::serverTimeCheck() {
  // Enable time and date if the ntp utility is missing
  bool enabled = ntpUtility.isEmpty() || !setDateTimeAuto->isChecked();
  cal->setEnabled(enabled);
  timeEdit->setEnabled(enabled);
  //kclock->setEnabled(enabled);
}

void Dtime::findNTPutility(){
  QByteArray envpath = qgetenv("PATH");
  if (!envpath.isEmpty() && envpath.startsWith(':')) {
    envpath = envpath.mid(1);
  }

  QString path = "/sbin:/usr/sbin:";
  if (!envpath.isEmpty()) {
    path += QFile::decodeName(envpath);
  } else {
    path += QLatin1String("/bin:/usr/bin");
  }

  foreach(const QString &possible_ntputility, QStringList() << "ntpdate" << "rdate" ) {
    if( !((ntpUtility = KStandardDirs::findExe(possible_ntputility, path)).isEmpty()) ) {
      kDebug() << "ntpUtility = " << ntpUtility;
      return;
    }
  }

  kDebug() << "ntpUtility not found!";
}

void Dtime::set_time()
{
  if( ontimeout )
    return;

  internalTimer.stop();

  time = timeEdit->time();
  kclock->setTime( time );

  emit timeChanged( true );
}

void Dtime::changeDate(const QDate &d)
{
  date = d;
  emit timeChanged( true );
}

void Dtime::configChanged(){
  emit timeChanged( true );
}

void Dtime::load()
{
  // The config is actually written to the system config, but the user does not have any local config,
  // since there is nothing writing it.
  KConfig _config( "kcmclockrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "NTP");
  timeServerList->clear();
  timeServerList->addItems(config.readEntry("servers",
    i18n("Public Time Server (pool.ntp.org),\
asia.pool.ntp.org,\
europe.pool.ntp.org,\
north-america.pool.ntp.org,\
oceania.pool.ntp.org")).split(',', QString::SkipEmptyParts));
  setDateTimeAuto->setChecked(config.readEntry("enabled", false));

  if (ntpUtility.isEmpty()) {
    timeServerList->setEnabled(false);
  }

  // Reset to the current date and time
  time = QTime::currentTime();
  date = QDate::currentDate();
  cal->setDate(date);

  // start internal timer
  internalTimer.start( 1000 );

  timeout();

  //Timezone
  currentZone();

  // read the currently set time zone
  tzonelist->setSelected(KSystemTimeZones::local().name(), true);
  emit timeChanged(false);
}

void Dtime::save( QVariantMap& helperargs )
{
  // Save the order, but don't duplicate!
  QStringList list;
  if( timeServerList->count() != 0)
    list.append(timeServerList->currentText());
  for ( int i=0; i<timeServerList->count();i++ ) {
    QString text = timeServerList->itemText(i);
    if( !list.contains(text) )
      list.append(text);
    // Limit so errors can go away and not stored forever
    if( list.count() == 10)
      break;
  }

  helperargs["ntp"] = true;
  helperargs["ntpServers"] = list;
  helperargs["ntpEnabled"] = setDateTimeAuto->isChecked();

  if(setDateTimeAuto->isChecked()) {
    // NTP Time setting - done in helper
    timeServer = timeServerList->currentText();
    kDebug() << "Setting date from time server " << timeServer;
  }
  else {
    // User time setting
    QDateTime dt(date, QTime(timeEdit->time()));

    kDebug() << "Set date " << dt;

    helperargs["date"] = true;
    helperargs["newdate"] = QString::number(dt.toTime_t());
    helperargs["olddate"] = QString::number(::time(0));
  }

  // restart time
  internalTimer.start( 1000 );

  QStringList selectedZones(tzonelist->selection());

  if (!selectedZones.isEmpty()) {
    helperargs["tz"] = true;
    helperargs["tzone"] = selectedZones.first();
  } else {
    helperargs["tzreset"] = true; // make the helper reset the timezone
  }

  currentZone();
}

void Dtime::processHelperErrors( int code )
{
  if( code & ClockHelper::NTPError ) {
    KMessageBox::error( this, i18n("Unable to contact time server: %1.", timeServer) );
    setDateTimeAuto->setChecked( false );
  }
  if( code & ClockHelper::DateError ) {
    KMessageBox::error( this, i18n("Can not set date."));
  }
  if( code & ClockHelper::TimezoneError)
    KMessageBox::error( this, i18n("Error setting new time zone."),
                        i18n("Time zone Error"));
}

void Dtime::timeout()
{
  // get current time
  time = QTime::currentTime();

  ontimeout = true;
  timeEdit->setTime(time);
  ontimeout = false;

  kclock->setTime( time );
}

QString Dtime::quickHelp() const
{
  return i18n("<h1>Date & Time</h1> This system settings module can be used to set the system date and"
    " time. As these settings do not only affect you as a user, but rather the whole system, you"
    " can only change these settings when you start the System Settings as root. If you do not have"
    " the root password, but feel the system time should be corrected, please contact your system"
    " administrator.");
}

Kclock::Kclock(QWidget *parent)
    : QWidget(parent)
{
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/clock");
    m_theme->setContainsMultipleImages(true);
}

Kclock::~Kclock()
{
    delete m_theme;
}

void Kclock::showEvent( QShowEvent *event )
{
    setClockSize( size() );
    QWidget::showEvent( event );
}

void Kclock::resizeEvent( QResizeEvent * )
{
    setClockSize( size() );
}

void Kclock::setClockSize(const QSize &size)
{
    int dim = qMin(size.width(), size.height());
    QSize newSize = QSize(dim, dim);

    if (newSize != m_faceCache.size()) {
        m_faceCache = QPixmap(newSize);
        m_handsCache = QPixmap(newSize);
        m_glassCache = QPixmap(newSize);

        m_theme->resize(newSize);
        m_repaintCache = RepaintAll;
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
        if( rect.height() < 64 )
            elementRect.setWidth( elementRect.width() * 2.5 );
        static const QPoint offset = QPoint(2, 3);

        p->translate(rect.x() + (rect.width() / 2) + offset.x(), rect.y() + (rect.height() / 2) + offset.y());
        p->rotate(rotation);
        p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
        m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

        p->restore();
    }

    p->save();

    name = handName + "Hand";
    elementRect = m_theme->elementRect(name);
    if (rect.height() < 64) {
        elementRect.setWidth(elementRect.width() * 2.5);
    }

    p->translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    p->rotate(rotation);
    p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
    m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

    p->restore();
}

void Kclock::paintInterface(QPainter *p, const QRect &rect)
{
    const bool m_showSecondHand = true;

    // compute hand angles
    const qreal minutes = 6.0 * time.minute() - 180;
    const qreal hours = 30.0 * time.hour() - 180 +
            ((time.minute() / 59.0) * 30.0);
    qreal seconds = 0;
    if (m_showSecondHand) {
        static const double anglePerSec = 6;
        seconds = anglePerSec * time.second() - 180;
    }

    // paint face and glass cache
    QRect faceRect = m_faceCache.rect();
    if (m_repaintCache == RepaintAll) {
        m_faceCache.fill(Qt::transparent);
        m_glassCache.fill(Qt::transparent);

        QPainter facePainter(&m_faceCache);
        QPainter glassPainter(&m_glassCache);
        facePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        glassPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        m_theme->paint(&facePainter, m_faceCache.rect(), "ClockFace");

        glassPainter.save();
        QRectF elementRect = QRectF(QPointF(0, 0), m_theme->elementSize("HandCenterScrew"));
        glassPainter.translate(faceRect.width() / 2 - elementRect.width() / 2, faceRect.height() / 2 - elementRect.height() / 2);
        m_theme->paint(&glassPainter, elementRect, "HandCenterScrew");
        glassPainter.restore();

        m_theme->paint(&glassPainter, faceRect, "Glass");

        // get vertical translation, see drawHand() for more details
        m_verticalTranslation = m_theme->elementRect("ClockFace").center().y();
    }

    // paint hour and minute hands cache
    if (m_repaintCache == RepaintHands || m_repaintCache == RepaintAll) {
        m_handsCache.fill(Qt::transparent);

        QPainter handsPainter(&m_handsCache);
        handsPainter.drawPixmap(faceRect, m_faceCache, faceRect);
        handsPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        drawHand(&handsPainter, faceRect, m_verticalTranslation, hours, "Hour");
        drawHand(&handsPainter, faceRect, m_verticalTranslation, minutes, "Minute");
    }

    // reset repaint cache flag
    m_repaintCache = RepaintNone;

    // paint caches and second hand
    QRect targetRect = faceRect;
    if (targetRect.width() < rect.width()) {
        targetRect.moveLeft((rect.width() - targetRect.width()) / 2);
    }

    p->drawPixmap(targetRect, m_handsCache, faceRect);
    if (m_showSecondHand) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        drawHand(p, targetRect, m_verticalTranslation, seconds, "Second");
    }
    p->drawPixmap(targetRect, m_glassCache, faceRect);
}

void Kclock::paintEvent( QPaintEvent * )
{
  QPainter paint(this);

  paint.setRenderHint(QPainter::Antialiasing);
  paintInterface(&paint, rect());
}

