#ifndef KCM_KEYBOARD_DATAMODEL_H
#define KCM_KEYBOARD_DATAMODEL_H

#include <QObject>

#include <memory>

#include "keyboard_model_model.h"
#include "../xkb_rules.h"

enum class TriState {
    ON = 0,
    OFF = 1,
    UNCHANGED = 2
};

class HardwareConfigModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(KeyboardModelModel* keyboardModelModel READ keyboardModelModel NOTIFY keyboardModelModelChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int numlockOnStartup READ numlockOnStartup WRITE setNumlockOnStartup NOTIFY numlockOnStartupChanged)
    Q_PROPERTY(int keyboardRepeat READ keyboardRepeat WRITE setKeyboardRepeat NOTIFY keyboardRepeatChanged)
    Q_PROPERTY(int repeatDelay READ repeatDelay WRITE setRepeatDelay NOTIFY repeatDelayChanged)
    Q_PROPERTY(double repeatRate READ repeatRate WRITE setRepeatRate NOTIFY repeatRateChanged)

public:
    explicit HardwareConfigModel(QObject* parent);

public:
    KeyboardModelModel* keyboardModelModel();

    int currentIndex();
    void setCurrentIndex(int idx);

    int numlockOnStartup();
    void setNumlockOnStartup(int state);

    int keyboardRepeat();
    void setKeyboardRepeat(int state);

    int repeatDelay();
    void setRepeatDelay(int ms);

    double repeatRate();
    void setRepeatRate(double rate);

    QString keyboardModel() const;
    void setKeyboardModel(const QString &keyboardModel);

Q_SIGNALS:
    void keyboardModelModelChanged();
    void currentIndexChanged();
    void numlockOnStartupChanged();
    void keyboardRepeatChanged();
    void repeatDelayChanged();
    void repeatRateChanged();

private:
    KeyboardModelModel m_keyboard_model_model;

    TriState m_numlockOnStartup;
    TriState m_keyboardRepeat;
    int m_repeatDelay;
    double m_repeatRate;
    QString m_keyboardModel;
};

#endif // KCM_KEYBOARD_DATAMODEL_H
