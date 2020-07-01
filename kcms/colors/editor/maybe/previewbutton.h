/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KDECOARTIONS_PREVIEW_BUTTON_ITEM_H
#define KDECOARTIONS_PREVIEW_BUTTON_ITEM_H

#include <QQuickPaintedItem>
#include <QColor>
#include <QPointer>
#include <KDecoration2/DecorationButton>

namespace KDecoration2
{
class Decoration;

namespace Preview
{
class PreviewBridge;
class Settings;

class PreviewButtonItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(KDecoration2::Preview::PreviewBridge *bridge READ bridge WRITE setBridge NOTIFY bridgeChanged)
    Q_PROPERTY(KDecoration2::Preview::Settings *settings READ settings WRITE setSettings NOTIFY settingsChanged)
    Q_PROPERTY(int type READ typeAsInt WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    explicit PreviewButtonItem(QQuickItem *parent = nullptr);
    ~PreviewButtonItem() override;
    void paint(QPainter *painter) override;

    PreviewBridge *bridge() const;
    void setBridge(PreviewBridge *bridge);

    Settings *settings() const;
    void setSettings(Settings *settings);

    KDecoration2::DecorationButtonType type() const;
    int typeAsInt() const;
    void setType(KDecoration2::DecorationButtonType type);
    void setType(int type);

    const QColor &color() const { return m_color; }
    void setColor(const QColor &color);

Q_SIGNALS:
    void bridgeChanged();
    void typeChanged();
    void settingsChanged();

protected:
    void componentComplete() override;

private:
    void createButton();
    void syncGeometry();
    QColor m_color;
    QPointer<KDecoration2::Preview::PreviewBridge> m_bridge;
    QPointer<KDecoration2::Preview::Settings> m_settings;
    KDecoration2::Decoration *m_decoration = nullptr;
    KDecoration2::DecorationButton *m_button = nullptr;
    KDecoration2::DecorationButtonType m_type = KDecoration2::DecorationButtonType::Custom;

};

} // Preview
} // KDecoration2

#endif
