/*
    SPDX-FileCopyrightText: 2023 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QVariant>
#include <ddcutil_c_api.h>
#include <memory>

class DDCDisplayControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant ref READ ref WRITE setRef NOTIFY refreshed)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY refreshed)
    Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY refreshed)
    Q_PROPERTY(int colorspace READ colorspace WRITE setColorspace NOTIFY refreshed)
    Q_PROPERTY(bool canChangeBrightness READ canChangeBrightness NOTIFY refreshed)
public:
    DDCDisplayControl();

    QVariant ref() const;
    void setRef(QVariant ref);
    int brightness() const;
    int contrast() const;
    int colorspace() const;
    bool canChangeBrightness() const;
    void setBrightness(int value);
    void setContrast(int value);
    void setColorspace(int value);

Q_SIGNALS:
    void refreshed();

private Q_SLOTS:
    void handleValueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value);

private:
    int m_brightness = 0;
    int m_contrast = 0;
    int m_colorspace = 0;
    DDCA_Display_Ref m_ref = nullptr;
};

class Worker : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value);
    void getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature);
Q_SIGNALS:
    void valueUpdated(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int currentValue);
    void valueUpdateFailed(DDCA_Display_Ref ref, const QString &reason);
    void valueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value);
    void getValueFailed(DDCA_Display_Ref ref, const QString &reason);
};

class Controller : public QObject
{
    Q_OBJECT

public:
    static Controller *inst();
    Controller(const Controller &) = delete;
    Controller(Controller &&) = delete;
    Controller &operator=(const Controller &) = delete;

    void updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value);
    void getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature);

Q_SIGNALS:
    void valueUpdated(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int currentValue);
    void valueUpdateFailed(DDCA_Display_Ref ref, const QString &reason);
    /**
     * @brief private signal
     * @param ref
     * @param feature
     * @param value
     */
    void updateValue_p(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value, QPrivateSignal);

    /**
     * @brief private signal
     * @param ref
     * @param feature
     */
    void getValue_p(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, QPrivateSignal);
    void valueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value);
    void getValueFailed(DDCA_Display_Ref ref, const QString &reason);

private:
    Controller();
    QThread m_workthread;
    Worker *m_worker;
};
