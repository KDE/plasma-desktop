/*
 * logitechmouse.cpp
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


#include <QPushButton>
#include <QWidget>
#include <QProgressBar>
#include <QTimer>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <config-workspace.h>
#include <config-kcontrol-input.h>
#ifdef HAVE_LIBUSB
#include <usb.h>

#include "logitechmouse.h"

LogitechMouse::LogitechMouse( struct usb_device *usbDev, int mouseCapabilityFlags, QWidget* parent, const char* name )
    : LogitechMouseBase( parent )
    , m_resolution( 0 )
{
    if ( !name )
        setObjectName( "LogitechMouse" );

    cordlessNameLabel->setText( i18n("Mouse type: %1", objectName() ) );

    m_mouseCapabilityFlags = mouseCapabilityFlags;

    m_usbDeviceHandle = usb_open( usbDev );

    if ( !m_usbDeviceHandle ) {
        kWarning() << "Error opening usbfs file: " << usb_strerror() ;
        return;
    }

    if ( mouseCapabilityFlags & USE_CH2 ) {
       m_useSecondChannel = 0x0100;
    } else {
        m_useSecondChannel = 0x0000;
    }

    permissionProblemText->hide();

    if ( mouseCapabilityFlags & HAS_RES ) {
        updateResolution();
        resolutionSelector->setEnabled( true );

        connect( button400cpi, SIGNAL(clicked()), parent, SLOT(changed()) );
        connect( button800cpi, SIGNAL(clicked()), parent, SLOT(changed()) );

        if ( 4 == resolution() ) {
            button800cpi->setChecked( true );
        } else if ( 3 == resolution() ) {
            button400cpi->setChecked( true );
        } else {
            // it must have failed, try to help out
            resolutionSelector->setEnabled(false);
            permissionProblemText->show();
        }
    }

    if ( mouseCapabilityFlags & HAS_CSR ) {

        initCordlessStatusReporting();

        // Do a name
        cordlessNameLabel->setText( i18n("Mouse type: %1", cordlessName() ) );
        cordlessNameLabel->setEnabled( true );

        // Display the battery power level - the level gets updated in updateGUI()
        batteryBox->setEnabled( true );

        // Channel
        channelSelector->setEnabled( true );
        // if the channel is changed, we need to turn off the timer, otherwise it
        // just resets the button to reflect the current status. The timer is
        // started again when we applyChanges()
        connect( channel1, SIGNAL(clicked()), this, SLOT(stopTimerForNow()) );
        connect( channel1, SIGNAL(clicked()), parent, SLOT(changed()) );
        if ( isDualChannelCapable() ) {
            channel2->setEnabled( true );
            connect( channel2, SIGNAL(clicked()), this, SLOT(stopTimerForNow()) );
            connect( channel2, SIGNAL(clicked()), parent, SLOT(changed()) );
        }

        updateGUI();
    }

}

LogitechMouse::~LogitechMouse()
{
    if ( m_usbDeviceHandle )
        usb_close( m_usbDeviceHandle );
}

void LogitechMouse::initCordlessStatusReporting()
{
    updateCordlessStatus();
    doUpdate = new QTimer( this ); // will be automatically deleted
    connect( doUpdate, SIGNAL(timeout()), this, SLOT(updateGUI()) );
    doUpdate->start( 20000 );
}

void LogitechMouse::updateCordlessStatus()
{
    QByteArray status(8, '\0');

    int result = -1;

    if ( m_usbDeviceHandle )
        result = usb_control_msg( m_usbDeviceHandle,
                                    USB_TYPE_VENDOR | USB_ENDPOINT_IN,0x09,
                                    (0x0003 | m_useSecondChannel),
                                    (0x0000 | m_useSecondChannel),
                                    status.data(),
                                    0x0008,
                                    1000);

    if (0 > result) {
        // We probably have a permission problem
        m_channel = 0;
        channelSelector->setEnabled( false );
        batteryBox->setEnabled( false );
        cordlessNameLabel->hide();
        permissionProblemText->show();
    } else {
        // kDebug() << "P6 (connect status): " << (status[0] & 0xFF);
        if ( status[0] & 0x20 ) { // mouse is talking
            m_connectStatus = ( status[0] & 0x80 );
            m_mousePowerup = ( status[0] & 0x40 );
            m_receiverUnlock = ( status[0] & 0x10 );
            m_waitLock = ( status[0] & 0x08 );
        }

        // kDebug() << "P0 (receiver type): " << (status[1] & 0xFF);
        /*
         0x38 = pid C501
         0x39 = pid C502
         0x3B = pid C504
         0x3C = pid C508
         0x3D = pid C506
         0x3E = pid C505
        */

        m_cordlessNameIndex = (status[2] & 0xFF);

        m_batteryLevel = (status[3] & 0x07 );
        if ( status[3] & 0x08 ) {
            m_channel = 2;
        } else {
            m_channel = 1;
        }

        m_cordlessSecurity = ( ( status[4] ) & ( status[5] << 8 ) );

        m_caseShape = ( status[6] & 0x7F );

        // kDebug() << "PB1 (device Capabilities): " << (status[7] & 0xFF);
        m_numberOfButtons = 2 + ( status[7] & 0x07 ); // 9 means something more than 8
        m_twoChannelCapable = ( status[7] & 0x40 );
        m_verticalRoller = ( status[7] & 0x08 );
        m_horizontalRoller = ( status[7] & 0x10 );
        m_has800cpi = ( status[7] & 0x20 );
    }

}

void LogitechMouse::updateGUI()
{
    updateCordlessStatus();

    batteryBar->setValue( batteryLevel() );

    if ( isDualChannelCapable() ) {
        if ( 2 == channel() ) {
            channel2->setChecked( true );
        } else if ( 1 == channel() ) {
            channel1->setChecked( true );
        } // else it might have failed - we don't do anything
    }
}

void LogitechMouse::stopTimerForNow()
{
    doUpdate->stop();
}

void LogitechMouse::applyChanges()
{
    if ( m_mouseCapabilityFlags & HAS_RES ) {
        if ( ( resolution() == 4 ) && ( button400cpi->isChecked() ) ) {
            // then we are in 800cpi mode, but want 400cpi
            setLogitechTo400();
        } else if ( ( resolution() == 3 ) && (button800cpi->isChecked() ) ) {
            // then we are in 400 cpi mode, but want 800 cpi
            setLogitechTo800();
        }
    }

    if ( isDualChannelCapable() ) {
        if ( ( channel() == 2 ) && ( channel1->isChecked() ) ) {
           // we are on channel 2, but want channel 1
           setChannel1();
           KMessageBox::information(this, i18n("RF channel 1 has been set. Please press Connect button on mouse to re-establish link"), i18n("Press Connect Button") );
        } else if ( ( channel() == 1 ) && ( channel2->isChecked() ) ) {
            // we are on channel 1, but want channel 2
            setChannel2();
            KMessageBox::information(this, i18n("RF channel 2 has been set. Please press Connect button on mouse to re-establish link"), i18n("Press Connect Button") );
        }

        initCordlessStatusReporting();
    }
}

void LogitechMouse::save(KConfig * /*config*/)
{
    kDebug() << "Logitech mouse settings not saved - not implemented yet";
}

quint8 LogitechMouse::resolution()
{
    // kDebug() << "resolution: " << m_resolution;
    if ( 0 == m_resolution ) {
        updateResolution();
    }
    return m_resolution;
}

void LogitechMouse::updateResolution()
{
    char resolution;

    int result = -1;

    if ( m_usbDeviceHandle )
        result = usb_control_msg( m_usbDeviceHandle,
                                   USB_TYPE_VENDOR | USB_ENDPOINT_IN,
                                   0x01,
                                   0x000E,
                                   0x0000,
                                   &resolution,
                                   0x0001,
                                   100);

    // kDebug() << "resolution is: " << resolution;
    if (0 > result) {
        kWarning() << "Error getting resolution from device : " << usb_strerror() ;
        m_resolution = 0;
    } else {
        m_resolution = resolution;
    }
}

void LogitechMouse::setLogitechTo800()
{
    int result = usb_control_msg( m_usbDeviceHandle,
                                  USB_TYPE_VENDOR,
                                  0x02,
                                  0x000E,
                                  4,
                                  NULL,
                                  0x0000,
                                  100);
    if (0 > result) {
        kWarning() << "Error setting resolution on device: " << usb_strerror() ;
    }
}

void LogitechMouse::setLogitechTo400()
{
    int result = usb_control_msg( m_usbDeviceHandle,
                                  USB_TYPE_VENDOR,
                                  0x02,
                                  0x000E,
                                  3,
                                  NULL,
                                  0x0000,
                                  100);
    if (0 > result) {
        kWarning() << "Error setting resolution on device: " << usb_strerror() ;
    }
}

quint8 LogitechMouse::batteryLevel() const
{
    return m_batteryLevel;
}


quint8 LogitechMouse::channel() const
{
    return m_channel;
}

bool LogitechMouse::isDualChannelCapable() const
{
    return m_twoChannelCapable;
}

void LogitechMouse::setChannel1()
{
    int result =  usb_control_msg( m_usbDeviceHandle,
                                   USB_TYPE_VENDOR,
                                   0x02,
                                   (0x0008 | m_useSecondChannel),
                                   (0x0000 | m_useSecondChannel),
                                   NULL,
                                   0x0000,
                                   1000);

    if (0 > result) {
        kWarning() << "Error setting mouse to channel 1 : " << usb_strerror() ;
    }

}

void LogitechMouse::setChannel2()
{
    int result =  usb_control_msg( m_usbDeviceHandle,
                                   USB_TYPE_VENDOR,
                                   0x02,
                                   (0x0008 | m_useSecondChannel),
                                   (0x0001 | m_useSecondChannel),
                                   NULL,
                                   0x0000,
                                   1000);

    if (0 > result) {
        kWarning() << "Error setting mouse to channel 2 : " << usb_strerror() ;
    }

}

QString LogitechMouse::cordlessName() const
{
    switch ( m_cordlessNameIndex ) {
    case 0x00:
        return i18nc( "no cordless mouse", "none" );
        break;
    case 0x04:
        return i18n( "Cordless Mouse" );
        break;
    case 0x05:
        return i18n( "Cordless Wheel Mouse" );
        break;
    case 0x06:
        return i18n( "Cordless MouseMan Wheel" );
        break;
    case 0x07:
        return i18n( "Cordless Wheel Mouse" );
        break;
    case 0x08:
        return i18n( "Cordless Wheel Mouse" );
        break;
    case 0x09:
        return i18n( "Cordless TrackMan Wheel" );
        break;
    case 0x0A:
        return i18n( "TrackMan Live" );
        break;
    case 0x0C:
        return i18n( "Cordless TrackMan FX" );
        break;
    case 0x0D:
        return i18n( "Cordless MouseMan Optical" );
        break;
    case 0x0E:
        return i18n( "Cordless Optical Mouse" );
        break;
    case 0x0F:
        return i18n( "Cordless Mouse" );
        break;
    case 0x12:
        return i18n( "Cordless MouseMan Optical (2ch)" );
        break;
    case 0x13:
        return i18n( "Cordless Optical Mouse (2ch)" );
        break;
    case 0x14:
        return i18n( "Cordless Mouse (2ch)" );
        break;
    case 0x82:
        return i18n( "Cordless Optical TrackMan" );
        break;
    case 0x8A:
        return i18n( "MX700 Cordless Optical Mouse" );
        break;
    case 0x8B:
        return i18n( "MX700 Cordless Optical Mouse (2ch)" );
        break;
    default:
        return i18n( "Unknown mouse");
    }
}

#include "logitechmouse.moc"

#endif

