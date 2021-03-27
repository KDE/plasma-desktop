/*
 *  dtime.h
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
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

#ifndef dtime_included
#define dtime_included

#include "ui_dateandtime.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPaintEvent>
#include <QSpinBox>
#include <QTime>
#include <QTimer>
#include <QWidget>

#include <KDatePicker>

class Kclock;
class QTimeEdit;

namespace Plasma
{
class Svg;
}

class Dtime : public QWidget, public Ui::DateAndTime
{
    Q_OBJECT
public:
    explicit Dtime(QWidget *parent, bool haveTimedated);

    void processHelperErrors(int code);
    void load();

    QString selectedTimeZone() const;
    QStringList ntpServers() const;
    bool ntpEnabled() const;
    QDateTime userTime() const;

    QString quickHelp() const;

Q_SIGNALS:
    void timeChanged(bool);

private Q_SLOTS:
    void configChanged();
    void serverTimeCheck();
    void timeout();
    void set_time();
    void changeDate(const QDate &);

private:
    void currentZone();
    void findNTPutility();
    QString ntpUtility;

    QTimeEdit *timeEdit;

    Kclock *kclock;

    QTime time;
    QDate date;
    QTimer internalTimer;

    QString timeServer;
    int BufI;
    bool refresh;
    bool ontimeout;
    bool m_haveTimedated;
};

class Kclock : public QWidget
{
    Q_OBJECT

public:
    Kclock(QWidget *parent = nullptr);
    ~Kclock() override;

    void setTime(const QTime &);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setClockSize(const QSize &size);
    void drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName);
    void paintInterface(QPainter *p, const QRect &rect);

private:
    QTime time;
    Plasma::Svg *m_theme;
    enum RepaintCache {
        RepaintNone,
        RepaintAll,
        RepaintHands,
    };
    RepaintCache m_repaintCache;
    QPixmap m_faceCache;
    QPixmap m_handsCache;
    QPixmap m_glassCache;
    qreal m_verticalTranslation;
};

#endif // dtime_included
