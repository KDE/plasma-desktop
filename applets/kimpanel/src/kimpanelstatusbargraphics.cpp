/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kimpanelstatusbargraphics.h"
#include "kimpanelsettings.h"
#include "paintutils.h"
#include "icongridlayout.h"

// Qt
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QMenu>
#include <QSignalMapper>
#include <QTimer>

// KDE
#include <KIcon>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <Plasma/PaintUtils>
#include <KToggleAction>

KimpanelStatusBarGraphics::KimpanelStatusBarGraphics(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_layout(new IconGridLayout(this)),
      m_startIMIcon(new Plasma::IconWidget(this)),
      m_propertyMapper(new QSignalMapper(this)),
      m_svg(0)
{
    m_startIMIcon->setIcon(KIcon("draw-freehand"));
    m_startIMIcon->hide();
    Plasma::ToolTipContent data(i18n("Start Input Method"), i18n("Start Input Method"), KIcon("draw-freehand"));
    Plasma::ToolTipManager::self()->setContent(m_startIMIcon, data);
    connect(m_startIMIcon, SIGNAL(clicked()), this, SIGNAL(startIM()));

    m_filterAction = new QAction(KIcon("view-filter"), i18n("Icon Filter"), this);
    m_filterMenu = new QMenu;
    m_filterAction->setMenu(m_filterMenu);

    m_configureAction = new QAction(KIcon("configure"), i18n("Configure Input Method"), this);
    connect(m_configureAction, SIGNAL(triggered()), this, SIGNAL(configure()));

    m_reloadConfigAction = new QAction(KIcon("view-refresh"), i18n("Reload Config"), this);
    connect(m_reloadConfigAction, SIGNAL(triggered()), this, SIGNAL(reloadConfig()));

    m_exitAction = new QAction(KIcon("application-exit"), i18n("Exit Input Method"), this);
    connect(m_exitAction, SIGNAL(triggered()), this, SIGNAL(exitIM()));

    setLayout(m_layout);
    connect(m_propertyMapper, SIGNAL(mapped(QString)), this, SIGNAL(triggerProperty(QString)));

    QStringList list = KimpanelSettings::self()->statusbarHiddenProperties();
    Q_FOREACH(const QString & str, list)
    m_hiddenProperties.insert(str);

    updateIcon();
}

KimpanelStatusBarGraphics::~KimpanelStatusBarGraphics()
{
}

void KimpanelStatusBarGraphics::updateProperties(const QVariant& var)
{
    QVariantList list = var.toList();

    m_props.clear();
    QSet<QString> keyset;

    Q_FOREACH(const QVariant & property, list) {
        QMap< QString, QVariant > map = property.toMap();
        QString key = map["key"].toString();
        if (key.isEmpty())
            continue;

        if (keyset.contains(key))
            continue;

        QString label = map["label"].toString();
        QString icon = map["icon"].toString();
        QString tip = map["tip"].toString();
        int state = map["states"].toInt();

        KimpanelProperty newProperty(key, label, icon, tip, state);

        m_props << newProperty;
        keyset.insert(key);
    }

    QMutableMapIterator<QString, Plasma::IconWidget *> i(m_iconMap);
    while (i.hasNext()) {
        i.next();
        if (!keyset.contains(i.key())) {
            delete i.value();
            i.remove();
        }
    }

    updateIcon();
}

void KimpanelStatusBarGraphics::updateIcon()
{
    m_filterMenu->clear();
    while (m_layout->count() > 0) {
        m_layout->removeAt(0);
    }
    Q_FOREACH(const KimpanelProperty & property, m_props) {
        Plasma::IconWidget* iconWidget = NULL;
        if (m_iconMap.contains(property.key))
            iconWidget = m_iconMap[property.key];
        else {
            iconWidget = new Plasma::IconWidget;
            m_iconMap[property.key] = iconWidget;
            iconWidget->setMaximumIconSize(QSizeF(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
            iconWidget->setMinimumIconSize(QSizeF(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
            m_propertyMapper->setMapping(iconWidget, property.key);
            connect(iconWidget, SIGNAL(clicked()), m_propertyMapper, SLOT(map()));
        }

        KIcon icon;
        if (property.icon.isEmpty()) {
            if (!property.label.isEmpty()) {
                if (!m_svg) {
                    m_svg = new Plasma::Svg(this);
                    m_svg->setImagePath("widgets/labeltexture");
                    m_svg->setContainsMultipleImages(true);
                }

                QFont font = KimpanelSettings::self()->font();
                font.setPixelSize(KIconLoader::SizeSmallMedium);
                QString iconString;
                // FIXME: since qt doesn't provide wcswidth equivalent yet, we only check ascii for now
                if (property.label.length() >= 2 && property.label[0].unicode() < 128 && property.label[1].unicode() < 128)
                    iconString = property.label.left(2);
                else
                    iconString = property.label.left(1);
                QPixmap pixmap = Plasma::PaintUtils::texturedText(iconString, KimpanelSettings::self()->font(), m_svg);

                icon = KIcon(pixmap);
            }
        }
        else
            icon = KIcon(property.icon);
        iconWidget->setIcon(icon);
        Plasma::ToolTipContent data(property.label, property.tip, icon);
        Plasma::ToolTipManager::self()->setContent(iconWidget, data);

        if (m_hiddenProperties.contains(property.key))
            iconWidget->hide();
        else {
            iconWidget->show();
            m_layout->addItem(iconWidget);
        }

        QAction *hiddenAction = new KToggleAction(property.label, m_filterMenu);
        hiddenAction->setCheckable(true);
        hiddenAction->setChecked(! m_hiddenProperties.contains(property.key));
        hiddenAction->setData(property.key);
        connect(hiddenAction, SIGNAL(toggled(bool)), this, SLOT(hiddenActionToggled()));
        m_filterMenu->addAction(hiddenAction);
    }

    if (m_layout->count() == 0) {
        m_layout->addItem(m_startIMIcon);
        m_startIMIcon->show();
    } else
        m_startIMIcon->hide();
}

QList<QAction *> KimpanelStatusBarGraphics::contextualActions() const
{
    QList<QAction *> result;

    result << m_filterAction;
    result << m_configureAction;
    result << m_reloadConfigAction;
    result << m_exitAction;

    return result;
}

void KimpanelStatusBarGraphics::hiddenActionToggled()
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (action) {
        QString key = action->data().toString();

        if (action->isChecked()) {
            m_hiddenProperties.remove(key);
            if (m_iconMap.value(key))
                m_iconMap.value(key)->show();
        } else {
            m_hiddenProperties.insert(key);
            if (m_iconMap.value(key))
                m_iconMap.value(key)->hide();
        }

        updateIcon();

        KimpanelSettings::self()->setStatusbarHiddenProperties(m_hiddenProperties.toList());
        KimpanelSettings::self()->writeConfig();
    } else {
        kWarning() << "qobject_cast failed";
    }
}

void KimpanelStatusBarGraphics::execMenu(const QVariant &var)
{
    if (!var.isValid())
        return;

    QVariantList list = var.toList();
    QList<KimpanelProperty> propList;

    Q_FOREACH(const QVariant & property, list) {
        QMap< QString, QVariant > map = property.toMap();
        QString key = map["key"].toString();
        if (key.isEmpty())
            continue;

        QString label = map["label"].toString();
        QString icon = map["icon"].toString();
        QString tip = map["tip"].toString();
        int state = map["states"].toInt();

        KimpanelProperty newProperty(key, label, icon, tip, state);

        propList << newProperty;
    }

    if (propList.length() == 0)
        return;

    QMenu *menu = new QMenu();
    QAction *action;
    QSignalMapper *mapper = new QSignalMapper(menu);
    connect(mapper, SIGNAL(mapped(QString)), this, SLOT(delayedTriggerProperty(QString)));
    foreach(const KimpanelProperty & prop, propList) {
        action = new QAction(KIcon(prop.icon), prop.label, menu);
        mapper->setMapping(action, prop.key);
        connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
        menu->addAction(action);
    }
    menu->exec(QCursor::pos());
    delete menu;
}



void KimpanelStatusBarGraphics::delayedTriggerProperty(const QString& key)
{
    DelayedSignalContainer *delayObj = new DelayedSignalContainer(key, this);
    connect(delayObj, SIGNAL(notify(QString)), this, SIGNAL(triggerProperty(QString)));
    delayObj->launch(50);
}


void KimpanelStatusBarGraphics::setLayoutMode(IconGridLayout::Mode mode)
{
    m_layout->setMode(mode);
}

void KimpanelStatusBarGraphics::setPreferredIconSize(int size)
{
    QSizeF newSize(size, size);
    if (newSize == m_preferredIconSize) {
        return;
    }
    m_preferredIconSize = newSize;
    m_startIMIcon->setPreferredIconSize(newSize);
    Q_FOREACH(Plasma::IconWidget * icon, m_iconMap.values()) {
        icon->setPreferredIconSize(newSize);
    }
}

