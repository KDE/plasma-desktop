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
#ifndef KDECOARTIONS_PREVIEW_BRIDGE_H
#define KDECOARTIONS_PREVIEW_BRIDGE_H

#include <KDecoration2/Private/DecorationBridge>
#include <KDecoration2/DecorationButton>

#include <QList>
#include <QPointer>

class QQuickItem;

class KPluginFactory;

namespace KDecoration2
{
namespace Preview
{

class PreviewClient;
class PreviewItem;
class PreviewSettings;

class PreviewBridge : public DecorationBridge
{
    Q_OBJECT
    Q_PROPERTY(QString plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
public:
    explicit PreviewBridge(QObject *parent = nullptr);
    ~PreviewBridge() override;
    std::unique_ptr<DecoratedClientPrivate> createClient(DecoratedClient *client, Decoration *decoration) override;
    void update(Decoration* decoration, const QRect& geometry) override;
    std::unique_ptr<DecorationSettingsPrivate> settings(DecorationSettings *parent) override;

    PreviewClient *lastCreatedClient() {
        return m_lastCreatedClient;
    }
    PreviewSettings *lastCreatedSettings() {
        return m_lastCreatedSettings;
    }

    void registerPreviewItem(PreviewItem *item);
    void unregisterPreviewItem(PreviewItem *item);

    void setPlugin(const QString &plugin);
    QString plugin() const;
    void setTheme(const QString &theme);
    QString theme() const;
    bool isValid() const;

    KDecoration2::Decoration *createDecoration(QObject *parent = nullptr);
    KDecoration2::DecorationButton *createButton(KDecoration2::Decoration *decoration, KDecoration2::DecorationButtonType type, QObject *parent = nullptr);

public Q_SLOTS:
    void configure(QQuickItem *ctx);

Q_SIGNALS:
    void pluginChanged();
    void themeChanged();
    void validChanged();

private:
    void createFactory();
    void setValid(bool valid);
    PreviewClient *m_lastCreatedClient;
    PreviewSettings *m_lastCreatedSettings;
    QList<PreviewItem*> m_previewItems;
    QString m_plugin;
    QString m_theme;
    QPointer<KPluginFactory> m_factory;
    bool m_valid;
};

class BridgeItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(KDecoration2::Preview::PreviewBridge *bridge READ bridge CONSTANT)

public:
    explicit BridgeItem(QObject *parent = nullptr);
    ~BridgeItem() override;

    void setPlugin(const QString &plugin) {
        m_bridge->setPlugin(plugin);
    }
    QString plugin() const {
        return m_bridge->plugin();
    }
    void setTheme(const QString &theme) {
        m_bridge->setTheme(theme);
    }
    QString theme() const {
        return m_bridge->theme();
    }
    bool isValid() const {
        return m_bridge->isValid();
    }

    PreviewBridge *bridge() const {
        return m_bridge;
    }

Q_SIGNALS:
    void pluginChanged();
    void themeChanged();
    void validChanged();

private:
    PreviewBridge *m_bridge;

};

}
}

Q_DECLARE_METATYPE(KDecoration2::Preview::PreviewBridge *)

#endif
