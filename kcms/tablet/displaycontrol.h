#pragma once
#include <QMutex>
#include <QObject>
#include <QThread>
#include <ddcutil_c_api.h>
#include <memory>

class DisplayControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DDCA_Display_Ref ref READ ref WRITE setRef NOTIFY refreshed)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY refreshed)
    Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY refreshed)
public:
    DisplayControl();

    DDCA_Display_Ref ref() const;
    void setRef(DDCA_Display_Ref ref);
    int brightness() const;
    int contrast() const;
    void setBrightness(int value);
    void setContrast(int value);

Q_SIGNALS:
    void refreshed();

private Q_SLOTS:
    void handleValueReturned(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value);

private:
    int m_brightness = 0;
    int m_contrast = 0;
    DDCA_Display_Ref m_ref = nullptr;
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

    void updateValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature, uint8_t value);
    void getValue(DDCA_Display_Ref ref, DDCA_Vcp_Feature_Code feature);

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
    QThread m_workthread;
    Worker *m_worker;
};
