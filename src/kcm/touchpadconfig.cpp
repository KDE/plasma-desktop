/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "touchpadconfig.h"

#include <cmath>

#include <QScrollArea>
#include <QTimer>
#include <QDBusInterface>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KConfigDialogManager>
#include <KTabWidget>

#include "customslider.h"
#include "sliderpair.h"
#include "touchpadbackend.h"
#include "plugins.h"
#include "testarea.h"

extern "C"
{
    KDE_EXPORT void kcminit_touchpad()
    {
        TouchpadBackend *backend = TouchpadBackend::self();

        if (!backend) {
            return;
        }

        QVariantHash current;
        backend->getConfig(current);
        TouchpadParameters::setSystemDefaults(current);

        TouchpadParameters config;
        backend->applyConfig(config.values());
    }
}

static const QString kcfgPrefix("kcfg_");

static QString getParameterName(QObject *widget)
{
    QString name(widget->objectName());
    if (!name.startsWith(kcfgPrefix)) {
        return QString();
    }
    name.remove(0, kcfgPrefix.length());
    return name;
}

static QString getWidgetName(const QString &i)
{
    return kcfgPrefix + i;
}

static void disableChildren(QWidget *widget,
                            const TouchpadParameters &p,
                            const QStringList &except)
{
    QString name(getParameterName(widget));
    if (!name.isEmpty() && p.findItem(name) && !except.contains(name)) {
        widget->setEnabled(false);
    }

    Q_FOREACH(QObject *child, widget->children()) {
        QWidget *childWidget = qobject_cast<QWidget *>(child);
        if (childWidget) {
            disableChildren(childWidget, p, except);
        }
    }
}

void TouchpadConfig::showEvent(QShowEvent *ev)
{
    QVariantHash currentValues;
    if (!m_backend->getConfig(currentValues)) {
        m_errorMessage->setText(m_backend->errorString());
        m_errorMessage->animatedShow();
    } else {
        for (QVariantHash::Iterator i = currentValues.begin();
             i != currentValues.end(); i++)
        {
            if (i.value().type() != QVariant::Double) {
                continue;
            }

            QObject *widget = findChild<QObject*>(getWidgetName(i.key()));
            if (!widget) {
                continue;
            }

            QVariant decimals = widget->property("decimals");
            bool ok = false;
            double k = std::pow(10.0, decimals.toInt(&ok));
            if (!decimals.isValid() || !ok) {
                continue;
            }

            i.value() = qRound(i.value().toDouble() * k) / k;
        }
    }
    m_config.setValues(currentValues);

    KCModule::showEvent(ev);
}

static void fillWithChoicesWidget(QObject *widget, TouchpadParameters *config)
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

    KComboBox *box = qobject_cast<KComboBox *>(widget);
    if (box) {
        box->addItems(choiceList);
    }
}

static void fillWithChoices(QObject *widget, TouchpadParameters *config)
{
    if (!widget) {
        return;
    }

    fillWithChoicesWidget(widget, config);

    Q_FOREACH(QObject *child, widget->children()) {
        fillWithChoices(child, config);
    }
}

static void copyHelpFromBuddy(QObject *root)
{
    QLabel *asLabel = qobject_cast<QLabel*>(root);
    if (asLabel && asLabel->buddy()) {
        if (asLabel->toolTip().isEmpty()) {
            asLabel->setToolTip(asLabel->buddy()->toolTip());
        }
        if (asLabel->statusTip().isEmpty()) {
            asLabel->setStatusTip(asLabel->buddy()->statusTip());
        }
        if (asLabel->whatsThis().isEmpty()) {
            asLabel->setWhatsThis(asLabel->buddy()->whatsThis());
        }
    }
    Q_FOREACH(QObject *child, root->children()) {
        copyHelpFromBuddy(child);
    }
}

template<typename T>
void addTab(KTabWidget *tabs, T &form)
{
    QScrollArea *container = new QScrollArea(tabs);
    container->setWidgetResizable(true);
    container->setFrameStyle(QFrame::NoFrame);
    container->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QWidget *widget = new QWidget(container);
    form.setupUi(widget);
    copyHelpFromBuddy(widget);
    widget->layout()->setContentsMargins(20, 20, 20, 20);

    container->setWidget(widget);
    tabs->addTab(container, widget->windowTitle());
}

TouchpadConfig::TouchpadConfig(QWidget *parent, const QVariantList &args)
    : KCModule(TouchpadPluginFactory::componentData(), parent, args)
{
    setAboutData(new KAboutData(*componentData().aboutData()));

    QGridLayout *layout = new QGridLayout(this);

    m_errorMessage = new KMessageWidget(this);
    m_errorMessage->setMessageType(KMessageWidget::Error);
    m_errorMessage->setVisible(false);
    layout->addWidget(m_errorMessage, 0, 0, 1, 2);

    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 1);

    KTabWidget *tabs = new KTabWidget(this);

    addTab(tabs, m_tapping);
    addTab(tabs, m_scrolling);
    addTab(tabs, m_pointerMotion);
    addTab(tabs, m_sensitivity);

    layout->addWidget(tabs, 1, 0, 1, 1);

    m_testArea = new TestArea(this);
    layout->addWidget(m_testArea, 1, 1);
    connect(m_testArea, SIGNAL(enter()), SLOT(beginTesting()));
    connect(m_testArea, SIGNAL(leave()), SLOT(endTesting()));
    connect(this, SIGNAL(changed(bool)), SLOT(onChanged()));

    static const CustomSlider::SqrtInterpolator interpolator;
    m_pointerMotion.kcfg_MinSpeed->setInterpolator(&interpolator);
    m_pointerMotion.kcfg_MaxSpeed->setInterpolator(&interpolator);
    m_pointerMotion.kcfg_AccelFactor->setInterpolator(&interpolator);

    new SliderPair(m_pointerMotion.kcfg_MinSpeed,
                   m_pointerMotion.kcfg_MaxSpeed, this);
    new SliderPair(m_sensitivity.kcfg_FingerLow,
                   m_sensitivity.kcfg_FingerHigh, this);
    new SliderPair(m_pointerMotion.kcfg_PressureMotionMinZ,
                   m_pointerMotion.kcfg_PressureMotionMaxZ, this);

    fillWithChoices(this, &m_config);

    KConfigDialogManager::changedMap()->insert("CustomSlider",
                                               SIGNAL(valueChanged(double)));
    m_manager = addConfig(&m_config, this);
    addConfig(&m_daemonSettings, this);

    m_backend = TouchpadBackend::self();
    disableChildren(this, m_config, m_backend->supportedParameters());

    KComboBox *mouseCombo = new KComboBox(true, this);
    mouseCombo->addItems(
                m_backend->listMouses(m_daemonSettings.mouseBlacklist()));
    m_sensitivity.kcfg_MouseBlacklist->setCustomEditor(mouseCombo);
}

void TouchpadConfig::save()
{
    KCModule::save();

    if (m_backend->applyConfig(m_config.values())) {
        m_errorMessage->animatedHide();
    } else {
        m_errorMessage->setText(m_backend->errorString());
        m_errorMessage->animatedShow();
    }

    static const QString kded("org.kde.kded");
    static const QString modulePath("/modules/touchpad");
    QDBusInterface daemonInterface(kded, modulePath);
    daemonInterface.call(QDBus::NoBlock, "reloadSettings");
}

void TouchpadConfig::hideEvent(QHideEvent *e)
{
    endTesting();
    KCModule::hideEvent(e);
}

TouchpadConfig::~TouchpadConfig()
{
    endTesting();
}

void TouchpadConfig::onChanged()
{
    if (m_testArea->underMouse()) {
        beginTesting();
    }
}

void TouchpadConfig::beginTesting()
{
    if (!m_prevConfig) {
        m_prevConfig.reset(new QVariantHash());
        m_backend->getConfig(*m_prevConfig.data());
    }

    QVariantHash oldConfig(m_config.values());
    m_config.setTemporary(true);

    m_manager->updateSettings();
    m_backend->applyConfig(m_config.values());

    m_config.setTemporary(false);
    m_config.setValues(oldConfig);
}

void TouchpadConfig::endTesting()
{
    if (!m_prevConfig) {
        return;
    }
    m_backend->applyConfig(*m_prevConfig.data());
    m_prevConfig.reset();
}
