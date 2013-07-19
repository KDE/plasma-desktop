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
        if (config.applyConfigOnInit()) {
            backend->applyConfig(&config);
        }
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

static QString getParameterName(QObject *widget)
{
    static const QString kcfgPrefix("kcfg_");
    QString name(widget->objectName());
    if (!name.startsWith(kcfgPrefix)) {
        return QString();
    }
    name.remove(0, kcfgPrefix.length());
    return name;
}

static void disableChildren(QWidget *widget,
                            const QStringList &enable = QStringList())
{
    QString name(getParameterName(widget));
    if (!name.isEmpty()) {
        widget->setEnabled(enable.contains(name));
    }

    Q_FOREACH(QObject *child, widget->children()) {
        QWidget *childWidget = qobject_cast<QWidget *>(child);
        if (childWidget) {
            disableChildren(childWidget, enable);
        }
    }
}

static void populateChoices(QObject *widget, TouchpadParameters *config)
{
    QString name(getParameterName(widget));
    if (name.isEmpty()) {
        return;
    }
    KConfigSkeletonItem *item = config->findItem(name);
    if (!item) {
        return;
    }
    KCoreConfigSkeleton::ItemEnum *e =
            dynamic_cast<KCoreConfigSkeleton::ItemEnum *>(item);
    if (!e) {
        return;
    }

    QStringList choiceList;
    Q_FOREACH(const KCoreConfigSkeleton::ItemEnum::Choice &c, e->choices()) {
        if (c.label.isEmpty()) {
            choiceList.append(c.name);
        } else {
            choiceList.append(c.label);
        }
    }

    QComboBox *box = qobject_cast<QComboBox *>(widget);
    if (box) {
        box->addItems(choiceList);
    }
}

static void populateChild(QObject *widget, TouchpadParameters *config)
{
    if (!widget) {
        return;
    }

    populateChoices(widget, config);

    Q_FOREACH(QObject *child, widget->children()) {
        populateChild(child, config);
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

    populateChild(this, &m_config);

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

    m_config.setApplyConfigOnInit(true);

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
