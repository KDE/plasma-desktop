/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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
#include "kimpanelapplet.h"
#include "kimpanelsettings.h"

#include <KConfigDialog>
#include <KCModule>
#include <KCMultiDialog>
#include <KService>
#include <KServiceTypeTrader>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QAction>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>
#include <plasma/tooltipmanager.h>

#include <math.h>

static const int s_magic_margin = 4;

KIMPanelApplet::KIMPanelApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent,args),
    m_layout(0),
    m_statusbar(0),
    m_statusbarGraphics(0),
    m_lookup_table(0),
    m_panel_agent(0)
{
    setHasConfigurationInterface(true);
//X     setAcceptDrops(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    // set our default size here
//X     resize((m_visibleIcons / m_rowCount) * s_defaultIconSize +
//X             (s_defaultSpacing * (m_visibleIcons + 1)),
//X            m_rowCount * 22 + s_defaultSpacing * 3);
}

KIMPanelApplet::~KIMPanelApplet()
{
    if (m_statusbar) {
        m_statusbar->close();
        m_statusbar->deleteLater();
    }
    if (m_lookup_table) {
        m_lookup_table->close();
        m_lookup_table->deleteLater();
    }
    m_statusbarGraphics->deleteLater();
    KIM::Settings::self()->writeConfig();
}

void KIMPanelApplet::saveState(KConfigGroup &config) const
{
    Q_UNUSED(config);
}

void KIMPanelApplet::init()
{
    setBackgroundHints(Plasma::Applet::DefaultBackground);

    m_preferIconWidth = qMax((int)KIconLoader::SizeSmall,
        KIM::Settings::self()->preferIconSize());

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/systemtray");

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
        SLOT(themeUpdated()));
    connect(KIM::Settings::self(), SIGNAL(configChanged()),
        SLOT(adjustSelf()));

    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(4,4,4,4);

    m_panel_agent = new PanelAgent(this);

    // Initialize widget which holds all im properties.
    m_statusbarGraphics = new KIMStatusBarGraphics(m_panel_agent);
    m_statusbarGraphics->setContentsMargins(0,0,0,0);

    m_statusbarGraphics->setCollapsible(true);
    m_statusbar = new KIMStatusBar(static_cast<Plasma::Corona *>(scene()));
    QAction *action = new QAction(KIcon("configure"),i18n("IM Panel Settings"),this);
    connect(action,SIGNAL(triggered()),this,SLOT(showConfigurationInterface()));
    m_statusbar->addAction(action);

    connect(m_statusbarGraphics,SIGNAL(iconCountChanged()),SLOT(adjustSelf()));
    connect(m_statusbarGraphics,SIGNAL(collapsed(bool)),SLOT(toggleCollapse(bool)));

    m_lookup_table = new KIMLookupTable(m_panel_agent, static_cast<Plasma::Corona *>(scene()));

    m_logoIcon = new Plasma::IconWidget(KIcon("draw-freehand"),"",this);
    m_logoIcon->hide();
    Plasma::ToolTipContent data(i18n("kimpanel"),i18n("KDE Input Method Panel"),m_logoIcon->icon());
    Plasma::ToolTipManager::self()->setContent(m_logoIcon,data);

    //m_layout->addItem(m_logoIcon);
    m_layout->addItem(m_statusbarGraphics);

    themeUpdated();

    m_panel_agent->created();
}

void KIMPanelApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints & Plasma::FormFactorConstraint) ||
            (constraints & Plasma::SizeConstraint)) {

        adjustSelf();
        //resize(preferredSize());
    }
}

void KIMPanelApplet::createConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent)
}

void KIMPanelApplet::configAccepted()
{
#if 0
    bool changed = false;
    int temp = m_uiConfig.icons->value();

    KConfigGroup cg = config();

    if (temp != m_preferIconWidth) {
        m_preferIconWidth = temp;
        cg.writeEntry("LargestIconWidth", m_preferIconWidth);
        changed = true;
    }

    if (changed) {
        emit configNeedsSaving();
        adjustSelf();
    }
#endif
}

QList<QAction*> KIMPanelApplet::contextualActions()
{
    return m_statusbarGraphics->actions();
}

void KIMPanelApplet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

    m_background->resizeFrame(contentsRect.size());
    m_background->paintFrame(painter, contentsRect, contentsRect);
}

void KIMPanelApplet::adjustSelf()
{
    int iconCount; 
    int i,j;
    QSizeF sizeHint = m_layout->contentsRect().size();
    QSizeF r = sizeHint;

    if (m_statusbar->graphicsWidget() == 0) {
        iconCount = m_statusbarGraphics->iconCount();
    } else {
        iconCount = 1;
    }

    int best_i = 1;
    qreal min_delta = 99999;
    switch (formFactor()) {
    case Plasma::Horizontal:
        i = 1;
        for (i = 1; i <= iconCount; ++i) {
            if (qAbs(r.height()/i - m_preferIconWidth) < min_delta) {
                best_i = i;
                min_delta = qAbs(r.height()/i - m_preferIconWidth);
            }
        }
        j = (iconCount + (best_i - 1)) / best_i;
        sizeHint = QSizeF(j*r.height()/best_i, r.height());
        break;
    case Plasma::Vertical:
        i = 1;
        for (i = 1; i <= iconCount; ++i) {
            if (qAbs(r.width()/i - m_preferIconWidth) < min_delta) {
                best_i = i;
                min_delta = qAbs(r.width()/i - m_preferIconWidth);
            }
        }
        j = (iconCount + (best_i - 1)) / best_i;
        sizeHint = QSizeF(r.width(),j*r.width()/best_i);
        break;
    case Plasma::Planar:
    case Plasma::MediaCenter:
        sizeHint = QSizeF(iconCount*m_preferIconWidth,m_preferIconWidth);
        break;
    }

    qreal left, top, right, bottom;
    m_layout->getContentsMargins(&left,&top,&right,&bottom);
    sizeHint = QSizeF(sizeHint.width() + left + right, sizeHint.height() + top + bottom);

    if (m_statusbar->graphicsWidget() == 0) {
        m_statusbarGraphics->getContentsMargins(&left,&top,&right,&bottom);
        sizeHint = QSizeF(sizeHint.width() + left + right, sizeHint.height() + top + bottom);
    }

    setPreferredSize(sizeHint);
}

void KIMPanelApplet::toggleCollapse(bool b)
{
    if (b) {
        m_layout->removeItem(m_statusbarGraphics);
        m_layout->addItem(m_logoIcon);
        m_statusbarGraphics->showLogo(true);
        m_statusbar->setGraphicsWidget(m_statusbarGraphics);
        disconnect(m_statusbarGraphics,SIGNAL(iconCountChanged()),this,SLOT(adjustSelf()));
    } else {
        m_statusbar->setGraphicsWidget(0);
        m_statusbarGraphics->showLogo(false);
        m_layout->addItem(m_statusbarGraphics);
        m_layout->removeItem(m_logoIcon);
        connect(m_statusbarGraphics,SIGNAL(iconCountChanged()),SLOT(adjustSelf()));
    }
    m_statusbar->setVisible(b);
    m_statusbar->move(KIM::Settings::self()->floatingStatusbarPos());
    m_logoIcon->setVisible(b);
    adjustSelf();
}

void KIMPanelApplet::themeUpdated()
{
    kDebug()<<"Update Theme"<<Plasma::Theme::defaultTheme()->themeName();
}

K_EXPORT_PLASMA_APPLET(kimpanel, KIMPanelApplet)

#include "kimpanelapplet.moc"
