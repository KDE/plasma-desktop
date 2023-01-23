#pragma once
#include <QMutex>
#include <QObject>
#include <QThread>
#include <ddcutil_c_api.h>
#include <memory>

class DisplayControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int brightness READ brightness NOTIFY refreshed)
    Q_PROPERTY(int contrast READ contrast NOTIFY refreshed)
public:
    DisplayControl(DDCA_Display_Ref ref);
    int brightness() const;
    int contrast() const;

Q_SIGNALS:
    void loaded();
    void refreshed();

private:
    int m_brightness = 0;
    int m_contrast = 0;
    DDCA_Display_Ref m_ref;
};

class Controller : public QObject
{
    class Worker : public QObject
    {
        Q_OBJECT
    public Q_SLOTS:
        void updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value);
        void getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature);
    Q_SIGNALS:
        void valueUpdated(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t currentValue);
        void valueUpdateFailed(DDCA_Display_Ref ref, const QString &reason);
        void valueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value);
        void getValueFailed(DDCA_Display_Ref ref, const QString &reason);
    };
    Q_OBJECT

public:
    static Controller *inst();
    Controller(const Controller &) = delete;
    Controller(Controller &&) = delete;
    Controller &operator=(const Controller &) = delete;
    /**
     * @brief updateValue
     * @param ref
     * @param feature
     * @param value
     * @return return false if there is an ongoing operation
     */
    bool updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value);
    /**
     * @brief getValue
     * @param ref
     * @param feature
     * @return return false if there is an ongoing operation
     */
    bool getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature);

Q_SIGNALS:
    void valueUpdated(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t currentValue);
    void valueUpdateFailed(DDCA_Display_Ref ref, const QString &reason);
    /**
     * @brief private signal
     * @param ref
     * @param feature
     * @param value
     */
    void updateValue_p(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value, QPrivateSignal);

    /**
     * @brief private signal
     * @param ref
     * @param feature
     */
    void getValue_p(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, QPrivateSignal);
    void valueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value);
    void getValueFailed(DDCA_Display_Ref ref, const QString &reason);

private:
    Controller();
    QMutex m_lock;
    QThread m_workthread;
    Worker *m_worker;
};
