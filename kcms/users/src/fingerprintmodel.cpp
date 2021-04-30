/*
    Copyright 2020  Devin Lin <espidev@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KLocalizedString>

#include "fingerprintmodel.h"

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

FingerprintModel::FingerprintModel(QObject* parent)
    : QObject(parent)
    , m_managerDbusInterface(new NetReactivatedFprintManagerInterface(QStringLiteral("net.reactivated.Fprint"), QStringLiteral("/net/reactivated/Fprint/Manager"), QDBusConnection::systemBus(), this))
{
    auto reply = m_managerDbusInterface->GetDefaultDevice();
    reply.waitForFinished();
    
    if (reply.isError()) {
        qDebug() << reply.error().message();
        setCurrentError(reply.error().message());
        return;
    }

    QDBusObjectPath path = reply.value();
    m_device = new FprintDevice(path, this);
    
    connect(m_device, &FprintDevice::enrollCompleted, this, &FingerprintModel::handleEnrollCompleted);
    connect(m_device, &FprintDevice::enrollStagePassed, this, &FingerprintModel::handleEnrollStagePassed);
    connect(m_device, &FprintDevice::enrollRetryStage, this, &FingerprintModel::handleEnrollRetryStage);
    connect(m_device, &FprintDevice::enrollFailed, this, &FingerprintModel::handleEnrollFailed);
}

FingerprintModel::~FingerprintModel()
{
    if (m_device) { // just in case device is claimed
        m_device->stopEnrolling();
        m_device->release();
    }
}

QString FingerprintModel::scanType()
{
    return !m_device ? "" : m_device->scanType();
}

QString FingerprintModel::currentError()
{
    return m_currentError;
}

void FingerprintModel::setCurrentError(QString error)
{
    if (m_currentError != error) {
        m_currentError = error;
        Q_EMIT currentErrorChanged();
    }
}

QString FingerprintModel::enrollFeedback()
{
    return m_enrollFeedback;
}

void FingerprintModel::setEnrollFeedback(QString feedback)
{
    m_enrollFeedback = feedback;
    Q_EMIT enrollFeedbackChanged();
}

bool FingerprintModel::currentlyEnrolling()
{
    return m_currentlyEnrolling;
}

bool FingerprintModel::deviceFound()
{
    return m_device != nullptr;
}

double FingerprintModel::enrollProgress()
{
    if (!deviceFound()) {
        return 0;
    }
    return (m_device->numOfEnrollStages() == 0) ? 1 : ((double) m_enrollStage) / m_device->numOfEnrollStages();
}

void FingerprintModel::setEnrollStage(int stage)
{
    m_enrollStage = stage;
    Q_EMIT enrollProgressChanged();
}

FingerprintModel::DialogState FingerprintModel::dialogState()
{
    return m_dialogState;
}

void FingerprintModel::setDialogState(DialogState dialogState)
{
    m_dialogState = dialogState;
    Q_EMIT dialogStateChanged();
}

void FingerprintModel::switchUser(QString username)
{
    m_username = username;
    
    if (deviceFound()) {
        stopEnrolling(); // stop enrolling if ongoing
        m_device->release(); // release from old user
        
        Q_EMIT enrolledFingerprintsChanged();
    }
}

bool FingerprintModel::claimDevice()
{
    if (!deviceFound()) {
        return false;
    }
    
    QDBusError error = m_device->claim(m_username);
    if (error.isValid() && error.name() != "net.reactivated.Fprint.Error.AlreadyInUse") {
        qDebug() << "error claiming:" << error.message();
        setCurrentError(error.message());
        return false;
    }
    return true;
}

void FingerprintModel::startEnrolling(QString finger)
{
    if (!deviceFound()) {
        setCurrentError(i18n("No fingerprint device found."));
        setDialogState(DialogState::FingerprintList);
        return;
    }

    setEnrollStage(0);
    setEnrollFeedback({});
    
    // claim device for user
    if (!claimDevice()) {
        setDialogState(DialogState::FingerprintList);
        return;
    }
    
    QDBusError error = m_device->startEnrolling(finger);
    if (error.isValid()) {
        qDebug() << "error start enrolling:" << error.message();
        setCurrentError(error.message());
        m_device->release();
        setDialogState(DialogState::FingerprintList);
        return;
    }
    
    m_currentlyEnrolling = true;
    Q_EMIT currentlyEnrollingChanged();
    
    setDialogState(DialogState::Enrolling);
}

void FingerprintModel::stopEnrolling()
{
    setDialogState(DialogState::FingerprintList);
    if (m_currentlyEnrolling) {
        m_currentlyEnrolling = false;
        Q_EMIT currentlyEnrollingChanged();
        
        QDBusError error = m_device->stopEnrolling();
        if (error.isValid()) {
            qDebug() << "error stop enrolling:" << error.message();
            setCurrentError(error.message());
            return;
        }
        m_device->release();
    }
}

void FingerprintModel::deleteFingerprint(QString finger)
{
    // claim for user
    if (!claimDevice()) {
        return;
    }

    QDBusError error = m_device->deleteEnrolledFinger(finger);
    if (error.isValid()) {
        qDebug() << "error deleting fingerprint:" << error.message();
        setCurrentError(error.message());
    }

    // release from user
    error = m_device->release();
    if (error.isValid()) {
        qDebug() << "error releasing:" << error.message();
        setCurrentError(error.message());
    }

    Q_EMIT enrolledFingerprintsChanged();
}

void FingerprintModel::clearFingerprints()
{
    // claim for user
    if (!claimDevice()) {
        return;
    }
 
    QDBusError error = m_device->deleteEnrolledFingers();
    if (error.isValid()) {
        qDebug() << "error clearing fingerprints:" << error.message();
        setCurrentError(error.message());
    }
    
    // release from user
    error = m_device->release();
    if (error.isValid()) {
        qDebug() << "error releasing:" << error.message();
        setCurrentError(error.message());
    }
    
    Q_EMIT enrolledFingerprintsChanged();
}

QStringList FingerprintModel::enrolledFingerprintsRaw()
{
    if (deviceFound()) {
        QDBusPendingReply reply = m_device->listEnrolledFingers(m_username);
        reply.waitForFinished();
        if (reply.isError()) {
            // ignore net.reactivated.Fprint.Error.NoEnrolledPrints, as it shows up when there are no fingerprints
            if (reply.error().name() != "net.reactivated.Fprint.Error.NoEnrolledPrints") {
                qDebug() << "error listing enrolled fingers:" << reply.error().message();
                setCurrentError(reply.error().message());
            }
            return QStringList();
        }
        return reply.value();
    } else {
        setCurrentError(i18n("No fingerprint device found."));
        setDialogState(DialogState::FingerprintList);
        return QStringList();
    }
}

QVariantList FingerprintModel::enrolledFingerprints()
{
    // convert fingers list to qlist of Finger objects
    QVariantList fingers;
    for (QString &finger : enrolledFingerprintsRaw()) {
        for (Finger *storedFinger : FINGERS) {
            if (storedFinger->internalName() == finger) {
                fingers.append(QVariant::fromValue(storedFinger));
                break;
            }
        }
    }
    return fingers;
}

QVariantList FingerprintModel::availableFingersToEnroll()
{
    QVariantList list; 
    QStringList enrolled = enrolledFingerprintsRaw();
    
    // add fingerprints to list that are not in the enrolled list
    for (Finger *finger : FINGERS) {
        if (!enrolledFingerprintsRaw().contains(finger->internalName())) {
            list.append(QVariant::fromValue(finger));
        }
    }
    return list;
}

void FingerprintModel::handleEnrollCompleted()
{
    setEnrollStage(m_device->numOfEnrollStages());
    setEnrollFeedback({});
    Q_EMIT enrolledFingerprintsChanged();
    Q_EMIT scanComplete();
    
    // stopEnrolling not called, as it is triggered only when the "complete" button is pressed
    // (only change dialog state change after button is pressed)
    setDialogState(DialogState::EnrollComplete);
}

void FingerprintModel::handleEnrollStagePassed()
{
    setEnrollStage(m_enrollStage + 1);
    setEnrollFeedback({});
    Q_EMIT scanSuccess();
    qDebug() << "fingerprint enroll stage pass:" << enrollProgress();
}

void FingerprintModel::handleEnrollRetryStage(QString feedback)
{
    Q_EMIT scanFailure();
    if (feedback == "enroll-retry-scan") {
        setEnrollFeedback(i18n("Retry scanning your finger."));
    } else if (feedback == "enroll-swipe-too-short") {
        setEnrollFeedback(i18n("Swipe too short. Try again."));
    } else if (feedback == "enroll-finger-not-centered") {
        setEnrollFeedback(i18n("Finger not centered on the reader. Try again."));
    } else if (feedback == "enroll-remove-and-retry") {
        setEnrollFeedback(i18n("Remove your finger from the reader, and try again."));
    }
    qDebug() << "fingerprint enroll stage fail:" << feedback;
}

void FingerprintModel::handleEnrollFailed(QString error)
{
    if (error == "enroll-failed") {
        setCurrentError(i18n("Fingerprint enrollment has failed."));
        stopEnrolling();
    } else if (error == "enroll-data-full") {
        setCurrentError(i18n("There is no space left for this device, delete other fingerprints to continue."));
        stopEnrolling();
    } else if (error == "enroll-disconnected") {
        setCurrentError(i18n("The device was disconnected."));
        m_currentlyEnrolling = false;
        Q_EMIT currentlyEnrollingChanged();
        setDialogState(DialogState::FingerprintList);
    } else if (error == "enroll-unknown-error") {
        setCurrentError(i18n("An unknown error has occurred."));
        stopEnrolling();
    }
}
