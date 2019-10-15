#include "hardware_config_model.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QDebug>
#include <QVariant>

HardwareConfigModel::HardwareConfigModel(QObject* parent)
    : QObject(parent)
    , m_keyboard_model_model(*XkbRules::self())
{
}

KeyboardModelModel* HardwareConfigModel::keyboardModelModel()
{
    return &m_keyboard_model_model;
}

int HardwareConfigModel::currentIndex()
{
    int index = m_keyboard_model_model.findIndexByName(m_keyboardModel);
    if (index >= 0)
        return index;
    return m_keyboard_model_model.findIndexByName("pc104");
}

void HardwareConfigModel::setCurrentIndex(int idx)
{
    QModelIndex mindex = m_keyboard_model_model.index(idx);
    QVariant data = m_keyboard_model_model.data(mindex, KeyboardModelModel::Roles::NameRole);
    m_keyboardModel = data.value<QString>();
    emit currentIndexChanged();
}

int HardwareConfigModel::numlockOnStartup()
{
    return static_cast<int>(m_numlockOnStartup);
}

void HardwareConfigModel::setNumlockOnStartup(int state)
{
    if (m_numlockOnStartup != static_cast<TriState>(state)) {
        m_numlockOnStartup = static_cast<TriState>(state);
        emit numlockOnStartupChanged();
    }
}

int HardwareConfigModel::keyboardRepeat()
{
    return static_cast<int>(m_keyboardRepeat);
}

void HardwareConfigModel::setKeyboardRepeat(int state)
{
    if (m_keyboardRepeat != static_cast<TriState>(state)) {
        m_keyboardRepeat = static_cast<TriState>(state);
        emit keyboardRepeatChanged();
    }
}

int HardwareConfigModel::repeatDelay()
{
    return m_repeatDelay;
}

void HardwareConfigModel::setRepeatDelay(int ms)
{
    if (m_repeatDelay != ms) {
        m_repeatDelay = ms;
        emit repeatDelayChanged();
    }
}

double HardwareConfigModel::repeatRate()
{
    return m_repeatRate;
}

void HardwareConfigModel::setRepeatRate(double rate)
{
    if (m_repeatRate != rate) {
        m_repeatRate = rate;
        emit repeatRateChanged();
    }
}

QString HardwareConfigModel::keyboardModel() const
{
    return m_keyboardModel;
}

void HardwareConfigModel::setKeyboardModel(const QString &keyboardModel)
{
    if (m_keyboardModel != keyboardModel) {
        m_keyboardModel = keyboardModel;
        emit currentIndexChanged();
    }
}
