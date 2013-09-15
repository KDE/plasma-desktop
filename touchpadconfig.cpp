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

static bool tabBefore(QWidget *first, QWidget *second)
{
    return qMakePair(first->geometry().top(), first->geometry().left()) <
            qMakePair(second->geometry().top(), second->geometry().left());
}

static void tabOrder(QWidget *widget, QWidget* &prev)
{
    if (widget->focusPolicy() & Qt::TabFocus && !widget->focusProxy()) {
        if (prev) {
            QWidget::setTabOrder(prev, widget);
        }
        prev = widget;
    }

    QList<QWidget *> children;
    Q_FOREACH(QObject *child, widget->children()) {
        QWidget *w = qobject_cast<QWidget *>(child);
        if (w) {
            children.append(w);
        }
    }

    qSort(children.begin(), children.end(), tabBefore);
    QWidget *prevChild = 0;
    Q_FOREACH(QWidget *child, children) {
        QLabel *label = qobject_cast<QLabel *>(prevChild);
        if (label && !label->buddy() && (child->focusPolicy() & Qt::TabFocus)) {
            label->setBuddy(child);
        }
        tabOrder(child, prev);
        prevChild = child;
    }
}

void TouchpadConfig::showEvent(QShowEvent *ev)
{
    KCModule::showEvent(ev);

    if (m_tabOrderSet) {
        return;
    }

    QWidget *prev = 0;
    tabOrder(this, prev);

    m_tabOrderSet = true;
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

static QString getWidgetName(KConfigSkeletonItem *i)
{
    return kcfgPrefix + i->name();
}

static void disableChildren(QWidget *widget,
                            const QStringList &except = QStringList())
{
    QString name(getParameterName(widget));
    if (!name.isEmpty() && !except.contains(name)) {
        widget->setEnabled(false);
    }

    Q_FOREACH(QObject *child, widget->children()) {
        QWidget *childWidget = qobject_cast<QWidget *>(child);
        if (childWidget) {
            disableChildren(childWidget, except);
        }
    }
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

static bool variantFuzzyCompare(const QVariant &a, const QVariant &b)
{
    if (a.type() == QVariant::Double && b.type() == QVariant::Double) {
        return qFuzzyCompare(static_cast<float>(a.toDouble()),
                             static_cast<float>(b.toDouble()));
    }
    return a == b;
}

bool TouchpadConfig::compareConfigs(const TouchpadParameters &a,
                                    const TouchpadParameters &b) const
{
    Q_FOREACH(KConfigSkeletonItem *i, a.items()) {
        if (!findChild<QWidget*>(getWidgetName(i))) {
            continue;
        }

        KConfigSkeletonItem *j = b.findItem(i->name());
        if (!j || !variantFuzzyCompare(i->property(), j->property())) {
            return false;
        }
    }
    return true;
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
    widget->layout()->setContentsMargins(20, 20, 20, 20);

    container->setWidget(widget);
    tabs->addTab(container, widget->windowTitle());
}

static void checkUi(QObject *w, TouchpadParameters &config)
{
    Q_FOREACH(QObject *child, w->children()) {
        QString param(getParameterName(child));
        if (!param.isEmpty()) {
            Q_ASSERT(config.findItem(param));
        }
        checkUi(child, config);
    }
}

//Qt Designer freezes when slider is enabled for KIntNumInput
static void enableSliders(QObject *w)
{
    if (qobject_cast<KIntNumInput*>(w)) {
        w->setProperty("sliderEnabled", true);
    }

    Q_FOREACH(QObject *child, w->children()) {
        enableSliders(child);
    }
}

TouchpadConfig::TouchpadConfig(QWidget *parent, const QVariantList &args)
    : KCModule(TouchpadPluginFactory::componentData(), parent, args),
      m_tabOrderSet(false), m_configOutOfSync(false)
{
    setAboutData(new KAboutData(*componentData().aboutData()));

    QGridLayout *layout = new QGridLayout(this);

    m_errorMessage = new KMessageWidget(this);
    m_errorMessage->setMessageType(KMessageWidget::Error);
    m_errorMessage->setVisible(false);
    layout->addWidget(m_errorMessage, 0, 0, 1, 2);

    m_differentConfigsMessage = new KMessageWidget(
                i18n("Saved configuration differs from active configuration"),
                this);
    m_differentConfigsMessage->setMessageType(KMessageWidget::Warning);
    m_differentConfigsMessage->setVisible(false);
    layout->addWidget(m_differentConfigsMessage, 1, 0, 1, 2);

    KTabWidget *tabs = new KTabWidget(this);

    addTab(tabs, m_tapping);
    addTab(tabs, m_scrolling);
    addTab(tabs, m_pointerMotion);
    addTab(tabs, m_sensitivity);

    checkUi(tabs, m_config);

    enableSliders(tabs);

    layout->addWidget(tabs, 2, 0, 1, 1);

    TestArea *testArea = new TestArea(this);
    layout->addWidget(testArea, 2, 1);
    connect(testArea, SIGNAL(enter()), SLOT(beginTesting()));
    connect(testArea, SIGNAL(leave()), SLOT(endTesting()));

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

    m_backend = TouchpadBackend::self();
    disableChildren(this, m_backend->supportedParameters());

    KConfigDialogManager::changedMap()->insert("CustomSlider",
                                               SIGNAL(valueChanged(double)));
    m_manager = addConfig(&m_config, this);

    if (!m_backend->getConfig(&m_config)) {
        m_errorMessage->setText(m_backend->errorString());
        QTimer::singleShot(0, m_errorMessage, SLOT(animatedShow()));
        return;
    }

    Q_FOREACH (KConfigSkeletonItem *i, m_config.items()) {
        if (i->property().type() != QVariant::Double) {
            continue;
        }

        QObject *widget = findChild<QObject*>(getWidgetName(i));
        if (!widget) {
            continue;
        }

        QVariant decimals = widget->property("decimals");
        bool ok = false;
        double k = std::pow(10.0, decimals.toInt(&ok));
        if (!decimals.isValid() || !ok) {
            continue;
        }

        i->setProperty(qRound(i->property().toDouble() * k) / k);
    }

    TouchpadParameters saved;
    if (!compareConfigs(m_config, saved)) {
        QTimer::singleShot(0, m_differentConfigsMessage, SLOT(animatedShow()));
        m_configOutOfSync = true;
    }
}

void TouchpadConfig::save()
{
    KCModule::save();

    if (m_backend->applyConfig(&m_config)) {
        m_errorMessage->animatedHide();
        m_differentConfigsMessage->animatedHide();
        m_configOutOfSync = false;
    } else {
        m_errorMessage->setText(m_backend->errorString());
        m_errorMessage->animatedShow();
    }
}

void TouchpadConfig::load()
{
    KCModule::load();

    if (m_configOutOfSync) {
        m_config.loadFrom(m_config.config());
        QTimer::singleShot(0, this, SLOT(changed()));
    }
}

void TouchpadConfig::hideEvent(QHideEvent *e)
{
    endTesting();
    KCModule::hideEvent(e);
}

void TouchpadConfig::beginTesting()
{
    m_prevConfig.reset(new TouchpadParameters());
    m_backend->getConfig(m_prevConfig.data());

    TouchpadParameters oldConfig;
    oldConfig.loadFrom(&m_config);
    m_config.setTemporary(true);

    m_manager->updateSettings();
    m_backend->applyConfig(&m_config);

    m_config.setTemporary(false);
    m_config.loadFrom(&oldConfig);
}

void TouchpadConfig::endTesting()
{
    if (!m_prevConfig) {
        return;
    }
    m_backend->applyConfig(m_prevConfig.data());
    m_prevConfig.reset();
}
