#ifndef TOUCHPADCONFIG_H
#define TOUCHPADCONFIG_H

#include <KCModule>

#include "touchpadparameters.h"
#include "ui_touchpadconfig.h"

class TouchpadBackend;

class TouchpadConfig : public KCModule, private Ui_TouchpadConfig
{
    Q_OBJECT

public:
    explicit TouchpadConfig(QWidget *parent,
                            const QVariantList &args = QVariantList());

    virtual void save();
    virtual void load();

protected:
    virtual void showEvent(QShowEvent *ev);

private Q_SLOTS:
    void showError(const QString &);

private:
    void differentConfigs();

    TouchpadBackend *m_backend;
    TouchpadParameters m_config;
    bool m_firstLoad;
    bool m_tabOrderSet;
};

#endif // TOUCHPADCONFIG_H
