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

#pragma once

#include "fprintdevice.h"
#include <qobjectdefs.h>

#include <KLocalizedString>

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

class Finger : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString internalName READ internalName CONSTANT)
    Q_PROPERTY(QString friendlyName READ friendlyName CONSTANT)
    
public:
    explicit Finger(const Finger &finger, QObject* parent = nullptr)
    : QObject(parent)
    , m_internalName(finger.internalName())
    , m_friendlyName(finger.friendlyName())
    {}

    explicit Finger(QString internalName = "", QString friendlyName = "", QObject* parent = nullptr)
    : QObject(parent)
    , m_internalName(internalName)
    , m_friendlyName(friendlyName)
    {}
    
    QString internalName() const
    {
        return m_internalName;
    }
    QString friendlyName() const
    {
        return m_friendlyName;
    }
private:
    QString m_internalName = "", m_friendlyName = "";
};

class FingerprintModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scanType READ scanType CONSTANT)
    Q_PROPERTY(QString currentError READ currentError WRITE setCurrentError NOTIFY currentErrorChanged) // error for ui to display
    Q_PROPERTY(QString enrollFeedback READ enrollFeedback WRITE setEnrollFeedback NOTIFY enrollFeedbackChanged)
    Q_PROPERTY(QVariantList enrolledFingerprints READ enrolledFingerprints NOTIFY enrolledFingerprintsChanged)
    Q_PROPERTY(QVariantList availableFingersToEnroll READ availableFingersToEnroll NOTIFY enrolledFingerprintsChanged)
    Q_PROPERTY(bool deviceFound READ deviceFound NOTIFY devicesFoundChanged)
    Q_PROPERTY(bool currentlyEnrolling READ currentlyEnrolling NOTIFY currentlyEnrollingChanged)
    Q_PROPERTY(double enrollProgress READ enrollProgress NOTIFY enrollProgressChanged)
    Q_PROPERTY(DialogState dialogState READ dialogState WRITE setDialogState NOTIFY dialogStateChanged)
    
public:
    explicit FingerprintModel(QObject* parent = nullptr);
    ~FingerprintModel();

    enum DialogState {
        FingerprintList,
        PickFinger,
        Enrolling,
        EnrollComplete,
    };
    Q_ENUM(DialogState)
    
    const QList<Finger *> FINGERS = {
        new Finger("right-index-finger", i18n("Right index finger"), this),
        new Finger("right-middle-finger", i18n("Right middle finger"), this),
        new Finger("right-ring-finger", i18n("Right ring finger"), this),
        new Finger("right-little-finger", i18n("Right little finger"), this),
        new Finger("right-thumb", i18n("Right thumb"), this),
        new Finger("left-index-finger", i18n("Left index finger"), this),
        new Finger("left-middle-finger", i18n("Left middle finger"), this),
        new Finger("left-ring-finger", i18n("Left ring finger"), this),
        new Finger("left-little-finger", i18n("Left little finger"), this),
        new Finger("left-thumb", i18n("Left thumb"), this)
    };
    
    Q_INVOKABLE void switchUser(QString username);
    bool claimDevice();
    
    Q_INVOKABLE void startEnrolling(QString finger);
    Q_INVOKABLE void stopEnrolling();
    Q_INVOKABLE void deleteFingerprint(QString finger);
    Q_INVOKABLE void clearFingerprints();
    
    QStringList enrolledFingerprintsRaw();
    QVariantList enrolledFingerprints();
    QVariantList availableFingersToEnroll();
    
    QString scanType();
    QString currentError();
    void setCurrentError(QString error);
    QString enrollFeedback();
    void setEnrollFeedback(QString feedback);
    bool currentlyEnrolling();
    bool deviceFound();
    double enrollProgress();
    void setEnrollStage(int stage);
    DialogState dialogState();
    void setDialogState(DialogState dialogState);
    
public Q_SLOTS:
    void handleEnrollCompleted();
    void handleEnrollStagePassed();
    void handleEnrollRetryStage(QString feedback);
    void handleEnrollFailed(QString error);

Q_SIGNALS:
    void currentErrorChanged();
    void enrollFeedbackChanged();
    void enrolledFingerprintsChanged();
    void devicesFoundChanged();
    void currentlyEnrollingChanged();
    void enrollProgressChanged();
    void dialogStateChanged();

    void scanComplete();
    void scanSuccess();
    void scanFailure();
    
private:
    QString m_username; // set to "" if it is the currently logged in user
    QString m_currentError, m_enrollFeedback;
    
    DialogState m_dialogState = DialogState::FingerprintList;
    
    bool m_currentlyEnrolling = false;
    int m_enrollStage = 0;
    
    FprintDevice *m_device = nullptr;
    
    NetReactivatedFprintManagerInterface *m_managerDbusInterface;
};
