#ifndef TOUCHPADCONFIG_H
#define TOUCHPADCONFIG_H

#include <KCModule>
#include <KMessageWidget>

#include "touchpadparameters.h"

#include "ui_pointermotion.h"
#include "ui_tap.h"
#include "ui_scroll.h"

class TouchpadBackend;

class TouchpadConfig : public KCModule
{
    Q_OBJECT

public:
    explicit TouchpadConfig(QWidget *parent,
                            const QVariantList &args = QVariantList());

    virtual void save();
    virtual void load();

protected:
    virtual void showEvent(QShowEvent *ev);

private:
    TouchpadBackend *m_backend;
    TouchpadParameters m_config;
    bool m_tabOrderSet;
    bool m_configOutOfSync;

    KMessageWidget *m_errorMessage, *m_differentConfigsMessage;

    Ui::PointerMotionForm m_pointerMotion;
    Ui::TapForm m_tapping;
    Ui::ScrollForm m_scrolling;
};

#endif // TOUCHPADCONFIG_H
