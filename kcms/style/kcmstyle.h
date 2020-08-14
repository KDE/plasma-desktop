/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2019 Kai Uwe Broulik <kde@broulik.de>
 * Copyright (C) 2019 Cyril Rossi <cyril.rossi@enioka.com>
 *
 * Portions Copyright (C) TrollTech AS.
 *
 * Based on kcmdisplay
 * Copyright (C) 1997-2002 kcmdisplay Authors.
 * (see Help -> About Style Settings)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KCMSTYLE_H
#define KCMSTYLE_H

#include <QPointer>

#include <KQuickAddons/ManagedConfigModule>

#include "gtkpage.h"

class QQuickItem;

class StyleSettings;
class StyleData;
class StylesModel;
class StyleConfigDialog;

class KCMStyle : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(GtkPage *gtkPage MEMBER m_gtkPage CONSTANT)
    Q_PROPERTY(StylesModel *model READ model CONSTANT)
    Q_PROPERTY(StyleSettings *styleSettings READ styleSettings CONSTANT)
    Q_PROPERTY(ToolBarStyle mainToolBarStyle READ mainToolBarStyle WRITE setMainToolBarStyle NOTIFY mainToolBarStyleChanged)
    Q_PROPERTY(ToolBarStyle otherToolBarStyle READ otherToolBarStyle WRITE setOtherToolBarStyle NOTIFY otherToolBarStyleChanged)

public:
    KCMStyle(QObject *parent, const QVariantList &args);
    ~KCMStyle() override;

    enum ToolBarStyle {
        NoText,
        TextOnly,
        TextBesideIcon,
        TextUnderIcon
    };
    Q_ENUM(ToolBarStyle)

    StylesModel *model() const;

    StyleSettings *styleSettings() const;

    ToolBarStyle mainToolBarStyle() const;
    void setMainToolBarStyle(ToolBarStyle style);
    Q_SIGNAL void mainToolBarStyleChanged();

    ToolBarStyle otherToolBarStyle() const;
    void setOtherToolBarStyle(ToolBarStyle style);
    Q_SIGNAL void otherToolBarStyleChanged();

    Q_INVOKABLE void configure(const QString &title, const QString &styleName, QQuickItem *ctx = nullptr);
    Q_INVOKABLE bool gtkConfigKdedModuleLoaded();

    void load() override;
    void save() override;
    void defaults() override;

Q_SIGNALS:
    void showErrorMessage(const QString &message);
    void styleReconfigured(const QString &styleName);

private:
    void loadSettingsToModel();

    StyleData *m_data;
    StylesModel *m_model;

    QString m_previousStyle;
    bool m_effectsDirty = false;

    ToolBarStyle m_mainToolBarStyle = NoText;
    ToolBarStyle m_otherToolBarStyle = NoText;

    QPointer<StyleConfigDialog> m_styleConfigDialog;
    GtkPage *m_gtkPage;
};

#endif // __KCMSTYLE_H
