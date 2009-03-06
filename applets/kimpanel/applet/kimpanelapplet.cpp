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

#include <KConfigDialog>
#include <KDesktopFile>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QDrag>
#include <QMouseEvent>
#include <QMimeData>
#include <QToolButton>

#include <KDialog>
#include <KMimeType>
#include <KStandardDirs>
#include <KWindowSystem>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include "math.h"

static const int s_defaultIconSize = 16;
static const int s_defaultSpacing = 0;

KIMPanelApplet::KIMPanelApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args),
    m_layout(0),
    m_rowCount(2),
    m_addAction(0),
    m_removeAction(0),
    m_panel_agent(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    // set our default size here
//X     resize((m_visibleIcons / m_rowCount) * s_defaultIconSize +
//X             (s_defaultSpacing * (m_visibleIcons + 1)),
//X            m_rowCount * 22 + s_defaultSpacing * 3);
}

KIMPanelApplet::~KIMPanelApplet()
{
}

void KIMPanelApplet::saveState(KConfigGroup &config) const
{
}

void KIMPanelApplet::init()
{
    KConfigGroup cg = config();
    cg.writeEntry("visibleIcons", 100);
    m_rowCount = qMax(1, cg.readEntry("rowCount", m_rowCount));

    // Initalize background
    m_background = new Plasma::FrameSvg();
    m_background->setImagePath("widgets/panel-background");

    // Initialize layout
    m_layout = new QGraphicsGridLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setRowMinimumHeight(0,s_defaultIconSize);
    m_layout->setColumnMinimumWidth(0,s_defaultIconSize);
//    setLayout(m_layout);

    // Initial "more icons" arrow
    m_arrow = new Plasma::IconWidget(this);
    m_arrow->setIcon(KIcon("arrow-right"));
    connect(m_arrow, SIGNAL(clicked()), SLOT(showDialog()));

    
    m_panel_agent = new PanelAgent(this);

    connect(m_panel_agent,
        SIGNAL(registerProperties(const QList<Property> &)),
        SLOT(registerProperties(const QList<Property> &)));
    connect(m_panel_agent,
        SIGNAL(updateProperty(const Property &)),
        SLOT(updateProperty(const Property &)));
    connect(this,SIGNAL(triggerProperty(const QString &)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));

    m_panel_agent->created();
    refactorUi();
}

QSizeF KIMPanelApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
/*
    if (which == Qt::PreferredSize) {
        QSizeF sizeHint = size();
        if (!m_layout) {
            return sizeHint;
        }
        qreal factor = m_layout->rowCount() * m_layout->columnCount();
        if (factor) {
            sizeHint.setWidth(sizeHint.height() / factor);
        }
        return sizeHint;
    }
*/
    return QGraphicsWidget::sizeHint(which, constraint);
}

void KIMPanelApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        //TODO: don't call so often
        refactorUi();
    }
    kDebug() << geometry() << contentsRect() << size()
        << "SizeHint(Min/Max/Prefer):"
        << sizeHint(Qt::MinimumSize) << sizeHint(Qt::MaximumSize) << sizeHint(Qt::PreferredSize)
        << "Eff.SizeHint:"
        << effectiveSizeHint(Qt::MinimumSize) << effectiveSizeHint(Qt::MaximumSize)
        << effectiveSizeHint(Qt::PreferredSize);
}

void KIMPanelApplet::refactorUi()
{
    clearLayout(m_layout);

    int rowCount;
    int iconWidth;
    if (formFactor() == Plasma::Vertical) {
        rowCount = qMin(m_rowCount, int(size().width()) / (s_defaultIconSize + s_defaultSpacing));
        // prevent possible division by zero if size().width() is 0
        rowCount = qMax(1, rowCount );
        iconWidth = size().width() / rowCount;
    } else {
        rowCount = qMin(m_rowCount, int(size().height()) / (s_defaultIconSize + s_defaultSpacing));
        // prevent possible division by zero if size().height() is 0
        rowCount = qMax(1, rowCount );
        iconWidth = qMax(s_defaultIconSize, int(size().height()) / rowCount);
    }

    const QSizeF minSize = QSizeF(iconWidth, iconWidth);
    const QSizeF maxSize = QSizeF(iconWidth, iconWidth);
    int count = 0;
//X     kDebug() << m_icons.count() << iconWidth << "pixel icons in" << rowCount
//X              << "rows, with a max of" << m_visibleIcons << "visible";
    foreach (Plasma::IconWidget *icon, m_icons) {
        //icon->setMinimumSize(minSize);
        //icon->setMaximumSize(maxSize);
        icon->resize(minSize);

        icon->show();
        m_layout->addItem(icon,0,0);

        ++count;
    }

    m_arrow->resize(minSize);
    m_layout->addItem(m_arrow,0,0);
    m_arrow->show();

    m_layout->updateGeometry();

    kDebug() << m_layout->rowCount() << m_layout->columnCount();
//X     setMinimumWidth(22);
//X     setMinimumHeight(22);
}

void KIMPanelApplet::showDialog()
{
}

void KIMPanelApplet::createConfigurationInterface(KConfigDialog *parent)
{
#if 0
    QWidget *widget = new QWidget(parent);
    uiConfig.setupUi(widget);
    connect(parent, SIGNAL(accepted()), SLOT(configAccepted()));
    uiConfig.rowCount->setValue(m_rowCount);
    uiConfig.dialogRowCount->setValue(m_dialogRowCount);
    uiConfig.dialogRowCount->hide();
    uiConfig.dialogrowLabel->hide();
    uiConfig.icons->setValue(m_visibleIcons);
    parent->addPage(widget, i18n("General"), icon());
#endif
}

void KIMPanelApplet::configAccepted()
{
#if 0
    bool changed = false;
    int temp = uiConfig.rowCount->value();

    KConfigGroup cg = config();
    if (temp != m_rowCount) {
        m_rowCount = temp;
        cg.writeEntry("rowCount", m_rowCount);
        changed = true;
    }

    temp = uiConfig.icons->value();
    if (temp != m_visibleIcons) {
        m_visibleIcons = temp;
        cg.writeEntry("visibleIcons", m_visibleIcons);
        changed = true;
    }

    temp = uiConfig.dialogRowCount->value();
    if (temp != m_dialogRowCount) {
        m_dialogRowCount = temp;
        cg.writeEntry("dialogRowCount", m_dialogRowCount);
        changed = true;
    }

    if (changed) {
        emit configNeedsSaving();
        refactorUi();
    }
#endif
}

QList<QAction*> KIMPanelApplet::contextActions(Plasma::IconWidget *icon)
{
#if 0
    QList<QAction*> tempActions;
    if (!m_addAction) {
        m_addAction = new QAction(KIcon("list-add"), i18n("Add Icon..."), this);
        connect(m_addAction, SIGNAL(triggered(bool)), this, SLOT(showAddInterface()));
    }

    tempActions << m_addAction;

    if (icon) {
        m_rightClickedIcon = icon;
        if (!m_removeAction) {
            m_removeAction = new QAction(KIcon("list-remove"), i18n("Remove Icon"), this);
            connect(m_removeAction, SIGNAL(triggered(bool)), this, SLOT(removeCurrentIcon()));
        }
        tempActions << m_removeAction;
    }
    return tempActions;
#endif
    return QList<QAction *>();
}

void KIMPanelApplet::clearLayout(QGraphicsLayout *layout)
{
    while (layout->count() > 0) {
        QGraphicsLayoutItem *item = layout->itemAt(0);
        layout->removeAt(0);
        item->graphicsItem()->hide();
    }
    layout->invalidate();
}

void KIMPanelApplet::showAddInterface()
{
#if 0
    if (!m_addDialog) {
        m_addDialog = new KDialog;
        m_addDialog->setCaption(i18n("Add Shortcut"));

        QWidget *widget = new QWidget;
        addUi.setupUi(widget);
        m_addDialog->setMainWidget(widget);
        connect(m_addDialog, SIGNAL(okClicked()), this, SLOT(addAccepted()));
    }
    m_addDialog->show();
#endif
}


void KIMPanelApplet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

    QRect r = rect().toRect();
    m_background->setElementPrefix("lastelements");

    painter->save();

    //m_background->setElementPrefix(QString());
    m_background->resizeFrame(r.size());
    m_background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));

    painter->restore();
}

void KIMPanelApplet::updateProperty(const Property &prop)
{
#if 0
    if (!prop_map.contains(prop.key)) {
        kWarning() << prop.key <<"not contained";
        return;
    }

    QToolButton *prop_button = prop_map.value(prop.key);

    QPixmap icon_pixmap;

    KIcon icon;

    if (!prop.icon.isEmpty()) {
//        icon_pixmap = KIcon(prop.icon).pixmap(KIconLoader::SizeSmall,KIconLoader::SizeSmall,Qt::KeepAspectRatio);
        icon = KIcon(prop.icon);
    } else {
        icon = KIcon(renderText(prop.label).scaled(KIconLoader::SizeSmall,KIconLoader::SizeSmall));
    }

    prop_button->setIcon(icon);
    prop_button->setToolTip(prop.tip);
#endif
}

void KIMPanelApplet::registerProperties(const QList<Property> &props)
{
/*
    m_pending_reg_properties = props;
    if (m_timer_id == -1) {
        m_timer_id = startTimer(0);
    }
*/
    m_icons.clear();
    Q_FOREACH (const Property &prop, props) {
        kDebug() << prop.key << prop.label;
        Plasma::IconWidget *icon = new Plasma::IconWidget(KIcon(prop.icon),"",this);
        m_icons << icon;
    }
    refactorUi();
}






K_EXPORT_PLASMA_APPLET(kimpanel, KIMPanelApplet)

#include "kimpanelapplet.moc"
