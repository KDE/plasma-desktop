#include "touchpadconfig.h"

#include <cmath>

#include <QBoxLayout>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KConfigDialogManager>

#include "touchpadbackend.h"

K_PLUGIN_FACTORY(TouchpadConfigFactory, registerPlugin<TouchpadConfig>();)
K_EXPORT_PLUGIN(TouchpadConfigFactory("kcm_touchpad"))

extern "C"
{
    KDE_EXPORT void kcminit_touchpad()
    {
        TouchpadBackend *backend = TouchpadBackend::self();

        if (!backend) {
            return;
        }

        TouchpadParameters config;
        backend->getConfig(&config);
        Q_FOREACH(KConfigSkeletonItem *i, config.items()) {
            i->swapDefault();
        }

        config.readConfig();
        backend->applyConfig(&config);
    }
}

class NonlinearInterpolator : public CustomSlider::Interpolator
{
public:
    double absolute(double relative, double minimum, double maximum) const
    {
        relative *= relative;
        return CustomSlider::Interpolator::absolute(relative, minimum, maximum);
    }

    double relative(double absolute, double minimum, double maximum) const
    {
        double value = CustomSlider::Interpolator::relative(absolute,
                                                            minimum, maximum);
        return std::sqrt(value);
    }
};

static void disableChildren(QWidget *widget,
                            const QStringList &enable = QStringList())
{
    static const QString kcfgPrefix("kcfg_");

    QString name(widget->objectName());
    if (name.startsWith(kcfgPrefix)) {
        name.remove(0, kcfgPrefix.length());
        widget->setEnabled(enable.contains(name));
    }

    Q_FOREACH(QObject *child, widget->children()) {
        QWidget *childWidget = qobject_cast<QWidget *>(child);
        if (childWidget) {
            disableChildren(childWidget, enable);
        }
    }
}

TouchpadConfig::TouchpadConfig(QWidget *parent, const QVariantList &args)
    : KCModule(TouchpadConfigFactory::componentData(), parent, args)
{
    setupUi(this);

    m_message = new KMessageWidget(this);
    m_message->setVisible(false);
    verticalLayout->insertWidget(0, m_message);

    static const NonlinearInterpolator interpolator;
    kcfg_MinSpeed->setInterpolator(&interpolator);
    kcfg_MaxSpeed->setInterpolator(&interpolator);
    kcfg_AccelFactor->setInterpolator(&interpolator);

    QStringList mouseButtons;
    mouseButtons << "Disabled"
                 << "Left button" << "Middle button" << "Right button";
    kcfg_TapButton1->insertItems(0, mouseButtons);
    kcfg_TapButton2->insertItems(0, mouseButtons);
    kcfg_TapButton3->insertItems(0, mouseButtons);

    m_backend = TouchpadBackend::self();
    if (!m_backend) {
        disableChildren(this);
        setButtons(NoAdditionalButton);
        showError("No backend found");
        return;
    }
    connect(m_backend, SIGNAL(error(QString)), SLOT(showError(QString)));

    KConfigDialogManager::changedMap()->insert("CustomSlider",
                                               SIGNAL(valueChanged(double)));
    addConfig(&m_config, this);
}

void TouchpadConfig::showError(const QString &errorString)
{
    m_message->setText(errorString);
    m_message->setMessageType(KMessageWidget::Error);
    m_message->animatedShow();
}

void TouchpadConfig::save()
{
    if (!m_backend) {
        return;
    }

    m_message->animatedHide();
    KCModule::save();
    m_backend->applyConfig(&m_config);

    load();
}

void TouchpadConfig::load()
{
    if (m_backend) {
        QStringList supportedParams;
        m_backend->getConfig(&m_config, &supportedParams);
        disableChildren(this, supportedParams);
    }
    KCModule::load();
}
