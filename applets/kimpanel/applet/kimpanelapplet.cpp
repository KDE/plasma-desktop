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
  : Plasma::Applet(parent,args),
    m_layout(0),
    m_widget(0)
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
}

void KIMPanelApplet::saveState(KConfigGroup &config) const
{
}

void KIMPanelApplet::init()
{
//X     setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed,QSizePolicy::DefaultType);


    KConfigGroup cg = config();
    cg.writeEntry("visibleIcons", 100);
//    m_rowCount = qMax(1, cg.readEntry("rowCount", m_rowCount));

    // Initalize background
    m_background = new Plasma::FrameSvg();
    m_background->setImagePath("widgets/panel-background");

    // Initialize layout
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0,0,0,0);

    m_widget = new KIMPanelWidget(this);

    m_layout->addItem(m_widget);

//X     m_layout->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed,QSizePolicy::DefaultType);
   connect(m_widget,SIGNAL(iconCountChanged(int)),SLOT(adjustSelf(int)));
    
}

void KIMPanelApplet::timeout() 
{
    //kDebug() << sizeConstraint();
//X     resize(effectiveSizeHint(Qt::PreferredSize));
}

/*
QSizeF KIMPanelApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{

    QSizeF sizeHint = size();
    if (!m_layout) {
        return sizeHint;
    }
    sizeHint.setWidth(left + right + m_layout->effectiveSizeHint(which).width());
    sizeHint.setHeight(top + bottom + m_layout->effectiveSizeHint(which).height());
    kDebug() << sizeHint;
    return sizeHint;
}
*/

void KIMPanelApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints & Plasma::FormFactorConstraint) ||
        (constraints & Plasma::SizeConstraint)) {
        //TODO: don't call so often
        adjustSelf(m_widget->iconCount());
        //resize(preferredSize());
    }
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

void KIMPanelApplet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

//    QRect r = rect().toRect();
//    m_background->setElementPrefix("lastelements");

    painter->save();

    //m_background->setElementPrefix(QString());
    //m_background->resizeFrame(contentsRect.size());
    //m_background->paintFrame(painter, contentsRect, contentsRect);

    painter->restore();
}

void KIMPanelApplet::adjustSelf(int iconCount)
{
    int iconWidth; 
    QSizeF sizeHint = geometry().size();
    switch (formFactor()) {
    case Plasma::Horizontal:
        iconWidth = qMax(geometry().height(), (qreal)KIconLoader::SizeSmallMedium);
        sizeHint = QSizeF(iconCount*iconWidth, geometry().height());
        break;
    case Plasma::Vertical:
        iconWidth = qMax(geometry().width(), (qreal)KIconLoader::SizeSmallMedium);
        sizeHint = QSizeF(geometry().width(),iconCount*iconWidth);
        break;
    case Plasma::Planar:
    case Plasma::MediaCenter:
        iconWidth = KIconLoader::SizeSmallMedium;
        sizeHint = QSizeF(iconCount*iconWidth,iconWidth);
        break;
    }
    
    qreal left, top, right, bottom;
    getContentsMargins(&left,&top,&right,&bottom);
    sizeHint = QSizeF(sizeHint.width() + left + right, sizeHint.height() + top + bottom);
    kDebug() << sizeHint;
    setPreferredSize(sizeHint);
}

K_EXPORT_PLASMA_APPLET(kimpanel, KIMPanelApplet)

#include "kimpanelapplet.moc"
