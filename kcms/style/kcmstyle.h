/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
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

#include <QHash>
#include <QLayout>
#include <QMap>

#include <kcmodule.h>

class KConfig;
class StylePreview;
class StyleConfig;

struct StyleEntry {
    QString name;
    QString desc;
    QString configPage;
    bool hidden;
};

class KCMStyle : public KCModule
{
    Q_OBJECT

public:
    KCMStyle( QWidget* parent, const QVariantList& );
    ~KCMStyle() override;

    void load() override;
    void save() override;
    void defaults() override;

    static QString defaultStyle();

protected:
    bool findStyle( const QString& str, int& combobox_item );
    void switchStyle(const QString& styleName, bool force = false);
    void setStyleRecursive(QWidget* w, QStyle* s);

    void loadStyle( KConfig& config );
    void loadEffects( KConfig& config );
    void addWhatsThis();

    void changeEvent( QEvent *event ) override;

protected Q_SLOTS:
    void styleSpecificConfig();
    void updateConfigButton();

    void setStyleDirty();
    void setEffectsDirty();

    void styleChanged();

private:
    QString currentStyle();
    static QString toolbarButtonText(int index);
    static int toolbarButtonIndex(const QString &text);
    static QString menuBarStyleText(int index);
    static int menuBarStyleIndex(const QString &text);

    bool m_bStyleDirty, m_bEffectsDirty;
    QHash <QString,StyleEntry*> styleEntries;
    QMap  <QString,QString>     nameToStyleKey;

    QVBoxLayout* mainLayout;

    // Widgets
    StylePreview* stylePreview;
    StyleConfig* styleConfig;
    QStyle* appliedStyle;
    QPalette palette;
};

#endif // __KCMSTYLE_H

// vim: set noet ts=4:
