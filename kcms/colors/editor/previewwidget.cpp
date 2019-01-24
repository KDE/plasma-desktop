/* Preview widget for KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * eventFilter code Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "previewwidget.h"

#include <KColorScheme>


PreviewWidget::PreviewWidget(QWidget *parent) : QFrame(parent)
{
    setupUi(this);

    // set correct colors on... lots of things
    setAutoFillBackground(true);
    frame->setBackgroundRole(QPalette::Base);
    viewWidget->setBackgroundRole(QPalette::Base);
    labelView0->setBackgroundRole(QPalette::Base);
    labelView3->setBackgroundRole(QPalette::Base);
    labelView4->setBackgroundRole(QPalette::Base);
    labelView2->setBackgroundRole(QPalette::Base);
    labelView1->setBackgroundRole(QPalette::Base);
    labelView5->setBackgroundRole(QPalette::Base);
    labelView6->setBackgroundRole(QPalette::Base);
    labelView7->setBackgroundRole(QPalette::Base);
    selectionWidget->setBackgroundRole(QPalette::Highlight);
    labelSelection0->setBackgroundRole(QPalette::Highlight);
    labelSelection3->setBackgroundRole(QPalette::Highlight);
    labelSelection4->setBackgroundRole(QPalette::Highlight);
    labelSelection2->setBackgroundRole(QPalette::Highlight);
    labelSelection1->setBackgroundRole(QPalette::Highlight);
    labelSelection5->setBackgroundRole(QPalette::Highlight);
    labelSelection6->setBackgroundRole(QPalette::Highlight);
    labelSelection7->setBackgroundRole(QPalette::Highlight);

    QList<QWidget*> widgets = findChildren<QWidget*>();
    foreach (QWidget* widget, widgets)
    {
        widget->installEventFilter(this);
        widget->setFocusPolicy(Qt::NoFocus);
    }
}

PreviewWidget::~PreviewWidget()
{
}

bool PreviewWidget::eventFilter(QObject *, QEvent *ev)
{
    switch (ev->type())
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::Wheel:
        case QEvent::ContextMenu:
            return true; // ignore
        default:
            break;
    }
    return false;
}

inline void copyPaletteBrush(QPalette &palette, QPalette::ColorGroup state,
                             QPalette::ColorRole role)
{
    palette.setBrush(QPalette::Active, role, palette.brush(state, role));
    if (state == QPalette::Disabled)
        // ### hack, while Qt has no inactive+disabled state
        // TODO copy from Inactive+Disabled to Inactive instead
        palette.setBrush(QPalette::Inactive, role,
                         palette.brush(QPalette::Disabled, role));
}

void PreviewWidget::setPaletteRecursive(QWidget *widget,
                                        const QPalette &palette)
{
    widget->setPalette(palette);

    const QObjectList children = widget->children();
    foreach (QObject *child, children) {
        if (child->isWidgetType())
            setPaletteRecursive((QWidget*)child, palette);
    }
}

inline void adjustWidgetForeground(QWidget *widget, QPalette::ColorGroup state,
                                   const KSharedConfigPtr &config,
                                   QPalette::ColorRole color,
                                   KColorScheme::ColorSet set,
                                   KColorScheme::ForegroundRole role)
{
    QPalette palette = widget->palette();
    KColorScheme::adjustForeground(palette, role, color, set, config);
    copyPaletteBrush(palette, state, color);
    widget->setPalette(palette);
}

void PreviewWidget::setPalette(const KSharedConfigPtr &config,
                               QPalette::ColorGroup state)
{
    QPalette palette = KColorScheme::createApplicationPalette(config);

    if (state != QPalette::Active) {
        copyPaletteBrush(palette, state, QPalette::Base);
        copyPaletteBrush(palette, state, QPalette::Text);
        copyPaletteBrush(palette, state, QPalette::Window);
        copyPaletteBrush(palette, state, QPalette::WindowText);
        copyPaletteBrush(palette, state, QPalette::Button);
        copyPaletteBrush(palette, state, QPalette::ButtonText);
        copyPaletteBrush(palette, state, QPalette::Highlight);
        copyPaletteBrush(palette, state, QPalette::HighlightedText);
        copyPaletteBrush(palette, state, QPalette::AlternateBase);
        copyPaletteBrush(palette, state, QPalette::Link);
        copyPaletteBrush(palette, state, QPalette::LinkVisited);
        copyPaletteBrush(palette, state, QPalette::Light);
        copyPaletteBrush(palette, state, QPalette::Midlight);
        copyPaletteBrush(palette, state, QPalette::Mid);
        copyPaletteBrush(palette, state, QPalette::Dark);
        copyPaletteBrush(palette, state, QPalette::Shadow);
    }

    setPaletteRecursive(this, palette);

#define ADJUST_WIDGET_FOREGROUND(w,c,s,r) \
    adjustWidgetForeground(w, state, config, QPalette::c, KColorScheme::s, KColorScheme::r);

    ADJUST_WIDGET_FOREGROUND(labelView1, Text, View, InactiveText);
    ADJUST_WIDGET_FOREGROUND(labelView2, Text, View, ActiveText);
    ADJUST_WIDGET_FOREGROUND(labelView3, Text, View, LinkText);
    ADJUST_WIDGET_FOREGROUND(labelView4, Text, View, VisitedText);
    ADJUST_WIDGET_FOREGROUND(labelView5, Text, View, NegativeText);
    ADJUST_WIDGET_FOREGROUND(labelView6, Text, View, NeutralText);
    ADJUST_WIDGET_FOREGROUND(labelView7, Text, View, PositiveText);

    ADJUST_WIDGET_FOREGROUND(labelSelection1, HighlightedText, Selection, InactiveText);
    ADJUST_WIDGET_FOREGROUND(labelSelection2, HighlightedText, Selection, ActiveText);
    ADJUST_WIDGET_FOREGROUND(labelSelection3, HighlightedText, Selection, LinkText);
    ADJUST_WIDGET_FOREGROUND(labelSelection4, HighlightedText, Selection, VisitedText);
    ADJUST_WIDGET_FOREGROUND(labelSelection5, HighlightedText, Selection, NegativeText);
    ADJUST_WIDGET_FOREGROUND(labelSelection6, HighlightedText, Selection, NeutralText);
    ADJUST_WIDGET_FOREGROUND(labelSelection7, HighlightedText, Selection, PositiveText);
}

