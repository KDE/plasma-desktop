/*
    Copyright 1997 Mark Donohoe
    Copyright 1999 Lars Knoll
    Copyright 2000 Rik Hemsley
    Copyright 2015 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>
    Copyright 2017 Marco Martin <mart@kde.org>
    Copyright 2019 Benjamin Port <benjamin.port@enioka.com>

    Ported to kcontrol2 by Geert Jansen.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef FONTS_H
#define FONTS_H

#include <config-X11.h>
#include <QAbstractItemModel>
#include <QStandardItemModel>

#include <KQuickAddons/ManagedConfigModule>

class FontsData;
class FontsSettings;
class FontsAASettings;

/**
 * The Desktop/fonts tab in kcontrol.
 */
class KFonts : public KQuickAddons::ManagedConfigModule
{
Q_OBJECT
    Q_PROPERTY(FontsSettings *fontsSettings READ fontsSettings CONSTANT)
    Q_PROPERTY(FontsAASettings *fontsAASettings READ fontsAASettings CONSTANT)
    Q_PROPERTY(QAbstractItemModel *subPixelOptionsModel READ subPixelOptionsModel CONSTANT)
    Q_PROPERTY(int subPixelCurrentIndex READ subPixelCurrentIndex WRITE setSubPixelCurrentIndex NOTIFY subPixelCurrentIndexChanged)
    Q_PROPERTY(QAbstractItemModel *hintingOptionsModel READ hintingOptionsModel CONSTANT)
    Q_PROPERTY(int hintingCurrentIndex READ hintingCurrentIndex WRITE setHintingCurrentIndex NOTIFY hintingCurrentIndexChanged)

public:
    KFonts(QObject *parent, const QVariantList &);
    ~KFonts() override;

    FontsSettings *fontsSettings() const;
    FontsAASettings *fontsAASettings() const;

    int subPixelCurrentIndex() const;
    void setHintingCurrentIndex(int idx);
    int hintingCurrentIndex() const;
    void setSubPixelCurrentIndex(int idx);
    QAbstractItemModel *subPixelOptionsModel() const;
    QAbstractItemModel *hintingOptionsModel() const;

public Q_SLOTS:
    void load() override;
    void save() override;
    Q_INVOKABLE void adjustAllFonts();
    Q_INVOKABLE void adjustFont(const QFont &font, const QString &category);

Q_SIGNALS:
    void fontsHaveChanged();
    void hintingCurrentIndexChanged();
    void subPixelCurrentIndexChanged();
    void aliasingChangeApplied();
    void fontDpiSettingsChanged();

private:
    QFont applyFontDiff(const QFont &fnt, const QFont &newFont, int fontDiffFlags);

    FontsData *m_data;
    QStandardItemModel *m_subPixelOptionsModel;
    QStandardItemModel *m_hintingOptionsModel;
};

#endif

