/*
    SPDX-FileCopyrightText: 2023 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "ddcDDCDisplayControl.h"
#include "displaymodel.h"
#include "qdebug.h"
#include <KLocalizedString>
#define VCP_BRIGHTNESS 0x10
#define VCP_CONTRAST 0x12
#define VCP_COLORSPACE 0xF0

DDCDisplayControl::DDCDisplayControl()
{
    connect(Controller::inst(), &Controller::valueReturned, this, &DDCDisplayControl::handleValueReturned);
}

QVariant DDCDisplayControl::ref() const
{
    return QVariant::fromValue(m_ref);
}

void DDCDisplayControl::setRef(QVariant ref)
{
    DDCA_Display_Ref_Wrapper wrapper = ref.value<DDCA_Display_Ref_Wrapper>();
    void *ref_p = wrapper.ref;
    if (m_ref == ref_p || ref_p == nullptr) {
        return;
    }
    m_ref = ref_p;
    if (m_ref) {
        Controller::inst()->getValue(ref_p, VCP_BRIGHTNESS);
        Controller::inst()->getValue(ref_p, VCP_CONTRAST);
        Controller::inst()->getValue(ref_p, VCP_COLORSPACE);
    }
}

void DDCDisplayControl::handleValueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value)
{
    if (ref != m_ref) {
        return;
    }
    switch (feature) {
    case VCP_BRIGHTNESS:
        m_brightness = value;
        break;
    case VCP_CONTRAST:
        m_contrast = value;
        break;
    case VCP_COLORSPACE:
        m_colorspace = value;
        break;
    }
    Q_EMIT refreshed();
}

int DDCDisplayControl::brightness() const
{
    return m_brightness;
}

int DDCDisplayControl::contrast() const
{
    return m_contrast;
}

int DDCDisplayControl::colorspace() const
{
    return m_colorspace;
}

void DDCDisplayControl::setBrightness(int value)
{
    if (value == m_brightness) {
        return;
    }
    Controller::inst()->updateValue(m_ref, VCP_BRIGHTNESS, value);
}

void DDCDisplayControl::setContrast(int value)
{
    if (value == m_contrast) {
        return;
    }
    Controller::inst()->updateValue(m_ref, VCP_CONTRAST, value);
}

void DDCDisplayControl::setColorspace(int value)
{
    if (value <= 0 || value == m_colorspace) {
        return;
    }

    Controller::inst()->updateValue(m_ref, VCP_COLORSPACE, value);
}

Controller *Controller::inst()
{
    static Controller singleton;
    return &singleton;
}

Controller::Controller()
    : m_worker(new Worker())
{
    ddca_enable_verify(false);
    m_worker->moveToThread(&m_workthread);
    qRegisterMetaType<DDCA_Display_Ref>("DDCA_Display_Ref");
    qRegisterMetaType<DDCA_Vcp_Feature_Code>("DDCA_Vcp_Feature_Code");
    connect(this, &Controller::updateValue_p, m_worker, &Worker::updateValue);
    connect(m_worker, &Worker::valueUpdated, this, &Controller::valueUpdated);
    connect(m_worker, &Worker::valueUpdateFailed, this, &Controller::valueUpdateFailed);
    connect(m_worker, &Worker::getValueFailed, this, &Controller::getValueFailed);
    connect(m_worker, &Worker::valueReturned, this, &Controller::valueReturned);
    connect(this, &Controller::getValue_p, m_worker, &Worker::getValue);
    connect(m_worker, &Worker::valueUpdateFailed, this, [](DDCA_Display_Ref ref, QString reason) {
        qDebug() << reason;
    });
    connect(m_worker, &Worker::getValueFailed, this, [](DDCA_Display_Ref ref, QString reason) {
        qDebug() << reason;
    });
    m_workthread.start();
}

void Controller::updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value)
{
    Q_EMIT updateValue_p(ref, feature, value, QPrivateSignal());
}

void Worker::updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, int value)
{
    DDCA_Display_Handle dh = nullptr;
    int err = ddca_open_display2(ref, true, &dh);
    if (err != 0) {
        Q_EMIT valueUpdateFailed(ref, i18nc("error description for a failed operation", "failed to open device"));
        return;
    }
    DDCA_Status status = ddca_set_non_table_vcp_value(dh, feature, 0, value);
    if (status != 0) {
        ddca_close_display(dh);
        Q_EMIT valueUpdateFailed(ref, i18nc("error description for a failed operation", "failed to set value"));
        return;
    }

    DDCA_Any_Vcp_Value *valueRef;
    status = ddca_get_any_vcp_value_using_explicit_type(dh, feature, DDCA_NON_TABLE_VCP_VALUE, &valueRef);
    if (status != 0) {
        ddca_close_display(dh);
        Q_EMIT valueUpdateFailed(ref, i18nc("error description for a failed operation", "failed to check updated value"));
        return;
    }

    if (valueRef->value_type != DDCA_NON_TABLE_VCP_VALUE) {
        ddca_close_display(dh);
        Q_EMIT valueUpdateFailed(ref, i18nc("error description for a failed operation, don't translate 'table'", "value type isn't table type"));
        return;
    }
    ddca_close_display(dh);
    Q_EMIT valueUpdated(ref, feature, VALREC_CUR_VAL(valueRef));
}

void Controller::getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature)
{
    Q_EMIT getValue_p(ref, feature, QPrivateSignal());
}

void Worker::getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature)
{
    DDCA_Display_Handle dh = nullptr;
    int err = ddca_open_display2(ref, true, &dh);
    if (err != 0) {
        Q_EMIT getValueFailed(ref, i18nc("error description for a failed operation", "failed to open device"));
        return;
    }

    DDCA_Any_Vcp_Value *valueRef;
    DDCA_Status status = ddca_get_any_vcp_value_using_explicit_type(dh, feature, DDCA_NON_TABLE_VCP_VALUE, &valueRef);
    if (status != 0) {
        ddca_close_display(dh);
        Q_EMIT getValueFailed(ref, i18nc("error description for a failed operation", "failed to get value"));
        return;
    }

    if (valueRef->value_type != DDCA_NON_TABLE_VCP_VALUE) {
        ddca_close_display(dh);
        Q_EMIT getValueFailed(ref, i18nc("error description for a failed operation, don't translate 'table'", "value type isn't table type"));
        return;
    }
    ddca_close_display(dh);
    Q_EMIT valueReturned(ref, feature, VALREC_CUR_VAL(valueRef));
}

#undef VCP_BRIGHTNESS
#undef VCP_CONTRAST
#undef VCP_COLORSPACE
