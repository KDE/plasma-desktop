/*
 *   Copyright (C) 2009 Wang Hoi <zealot.hoigmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 (or, at
 *   your option, any later version) as published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kimpanel.h"
#include "panelagent.h"
#include "paneladaptor.h"
#include "kdebug.h"
#include "plasma/version.h"
KIMPanel::KIMPanel(QObject* parent, const QVariantList& args)
    : Plasma::Applet(parent, args), 
    m_svg(this),
    m_icon("document"),
    m_panel_agent(0)
{
    m_svg.setImagePath("widgets/background");
    // this will get us the standard applet background, for free!
    setBackgroundHints(DefaultBackground);
    resize(200, 200);
}
KIMPanel::~KIMPanel()
{
    if (hasFailedToLaunch()) {
        // Do some cleanup here
    } else {
        // Save settings
    }
}
void KIMPanel::init()
{
    // setFailedToLaunch(true, "kim dataengine not found");
    m_panel_agent = new PanelAgent(this);
    new PanelAdaptor(m_panel_agent);
    kDebug(1204)<<"suceeds.";
}
void KIMPanel::paintInterface(QPainter *p, 
        const QStyleOptionGraphicsItem *option, 
        const QRect &contentsRect)
{
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    // Now we draw the applet, starting with our svg
    m_svg.resize((int)contentsRect.width(), (int)contentsRect.height());
    m_svg.paint(p, (int)contentsRect.left(), (int)contentsRect.top());

    // We place the icon and text
    p->drawPixmap(7, 0, m_icon.pixmap((int)contentsRect.width(),(int)contentsRect.width()-14));
    p->save();
    p->setPen(Qt::white);
    p->drawText(contentsRect,
            Qt::AlignBottom | Qt::AlignHCenter,
            "Hello Plasmoid!");
    p->restore();
}
#include "kimpanel.moc"
