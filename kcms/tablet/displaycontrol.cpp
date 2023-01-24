#include "displaycontrol.h"
#include <KLocalizedString>
#define VCP_BRIGHTNESS 0x10
#define VCP_CONTRAST 0x12

DisplayControl::DisplayControl()
{
    connect(Controller::inst(), &Controller::valueReturned, this, &DisplayControl::handleValueReturned);
}

DDCA_Display_Ref DisplayControl::ref() const
{
    return m_ref;
}

void DisplayControl::setRef(DDCA_Display_Ref ref)
{
    if (m_ref == ref) {
        return;
    }
    m_ref = ref;
    if (m_ref) {
        Controller::inst()->getValue(ref, VCP_BRIGHTNESS);
        Controller::inst()->getValue(ref, VCP_CONTRAST);
    }
}

void DisplayControl::handleValueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value)
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
    }
    Q_EMIT refreshed();
}

void DisplayControl::setBrightness(int value)
{
    Controller::inst()->updateValue(m_ref, VCP_BRIGHTNESS, value);
}

void DisplayControl::setContrast(int value)
{
    Controller::inst()->updateValue(m_ref, VCP_CONTRAST, value);
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
    connect(this, &Controller::updateValue_p, m_worker, &Worker::updateValue);
    connect(m_worker, &Worker::valueUpdated, this, &Controller::valueUpdated);
    connect(m_worker, &Worker::valueUpdateFailed, this, &Controller::valueUpdateFailed);
    connect(m_worker, &Worker::getValueFailed, this, &Controller::getValueFailed);
    connect(m_worker, &Worker::valueReturned, this, &Controller::valueReturned);
    connect(this, &Controller::getValue_p, m_worker, &Worker::getValue);
}

void Controller::updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value)
{
    Q_EMIT updateValue_p(ref, feature, value, QPrivateSignal());
}

void Controller::Worker::updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value)
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
    Q_EMIT valueUpdated(ref, feature, VALREC_CUR_VAL(valueRef));
}

void Controller::getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature)
{
    Q_EMIT getValue_p(ref, feature, QPrivateSignal());
}

void Controller::Worker::getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature)
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
        Q_EMIT getValueFailed(ref, i18nc("error description for a failed operation", "failed to check updated value"));
        return;
    }

    if (valueRef->value_type != DDCA_NON_TABLE_VCP_VALUE) {
        ddca_close_display(dh);
        Q_EMIT getValueFailed(ref, i18nc("error description for a failed operation, don't translate 'table'", "value type isn't table type"));
        return;
    }
    Q_EMIT valueReturned(ref, feature, VALREC_CUR_VAL(valueRef));
}

#undef VCP_BRIGHTNESS
#undef VCP_CONTRAST
