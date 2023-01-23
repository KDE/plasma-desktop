#include "displaycontrol.h"
#include <KLocalizedString>

DisplayControl::DisplayControl(DDCA_Display_Ref ref)
    : m_ref(ref)
{
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
    connect(m_worker, &Worker::valueUpdated, this, [this](DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value) {
        m_lock.unlock();
        Q_EMIT valueUpdated(ref, feature, value);
    });
    connect(m_worker, &Worker::valueUpdateFailed, this, [this](DDCA_Display_Ref ref, const QString &reason) {
        m_lock.unlock();
        Q_EMIT valueUpdateFailed(ref, reason);
    });
    connect(m_worker, &Worker::getValueFailed, this, [this](DDCA_Display_Ref ref, const QString &reason) {
        m_lock.unlock();
        Q_EMIT valueUpdateFailed(ref, reason);
    });
    connect(m_worker, &Worker::valueReturned, this, [this](DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value) {
        m_lock.unlock();
        Q_EMIT valueReturned(ref, feature, value);
    });
    connect(this, &Controller::getValue_p, m_worker, &Worker::getValue);
}

bool Controller::updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value)
{
    auto ret = m_lock.tryLock();
    if (ret) {
        Q_EMIT updateValue_p(ref, feature, value, QPrivateSignal());
    }
    return ret;
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

bool Controller::getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature)
{
    auto ret = m_lock.tryLock();
    if (ret) {
        Q_EMIT getValue_p(ref, feature, QPrivateSignal());
    }
    return ret;
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
