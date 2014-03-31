/*
 * logitechmouse.h
 *
 * Copyright (C) 2004 Brad Hards <bradh@frogmouth.net>
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
 */


#ifndef __LOGITECHMOUSE_H__
#define __LOGITECHMOUSE_H__

#include <QPushButton>

#include <kconfig.h>

#include <config-workspace.h>

#include "ui_logitechmouse_base.h"

#include <usb.h>

#define VENDOR_LOGITECH 0x046D
#define HAS_RES 0x01   /* mouse supports variable resolution */
#define HAS_SS  0x02   /* mouse supports smart scroll control */
#define HAS_CSR  0x04  /* mouse supports cordless status reporting and control */
#define HAS_SSR  0x08  /* mouse supports smart scroll reporting */
#define USE_CH2  0x10  /* mouse needs to use the second channel */

class LogitechMouseBase : public QWidget, public Ui::LogitechMouseBase
{
public:
  LogitechMouseBase( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};



class LogitechMouse : public LogitechMouseBase
{
    Q_OBJECT

public:
    LogitechMouse( struct usb_device *usbDev, int mouseCapabilityFlags, QWidget* parent = 0, const char* name=0);
    ~LogitechMouse();
    void applyChanges();
    void save(KConfig *config);

protected Q_SLOTS:
    void setChannel1();
    void setChannel2();
    void updateGUI();
    void stopTimerForNow();

private:
    void initCordlessStatusReporting();
    void updateCordlessStatus();

    void setLogitechTo400();
    void setLogitechTo800();

    QString cordlessName() const ;
    quint8 resolution();
    void updateResolution();
    quint8 batteryLevel() const;
    quint8 channel() const;
    bool isDualChannelCapable() const;

    QTimer *doUpdate;

    struct usb_dev_handle *m_usbDeviceHandle;
    bool m_connectStatus; // true if the CONNECT button on the mouse is pressed
    bool m_mousePowerup; // true if we are doing "just out of the box" auto-locking
    bool m_receiverUnlock; // true if mouse has been disconnected by a long press
                           // of the receiver's CONNECT button
    bool m_waitLock; // true if receiver searching for new mouse because the
                     // CONNECT button on the receiver was pressed
    quint8 m_batteryLevel;
    quint8 m_channel;
    quint8 m_cordlessNameIndex; // this gets convered into a QString in cordlessName()
    quint16 m_cordlessSecurity;
    quint16 m_useSecondChannel;
    quint8 m_caseShape;
    quint8 m_numberOfButtons;
    quint8 m_resolution;
    bool m_twoChannelCapable; // true if the mouse supports dual channels
    bool m_verticalRoller; // true if the mouse has a vertical roller (wheel)
    bool m_horizontalRoller; // true if the mouse has a horizontal roller (wheel)
    bool m_has800cpi; // true if the mouse does 800cpi resolution
    int m_mouseCapabilityFlags;
};

#endif

