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
#include "previewbridge.h"
#include "previewclient.h"
#include "previewitem.h"
#include "previewsettings.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>

#include <KCModule>
#include <KPluginLoader>
#include <KPluginFactory>
#include <KPluginTrader>

#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QVBoxLayout>
#include <QWindow>

namespace KDecoration2
{
namespace Preview
{

static const QString s_pluginName = QStringLiteral("org.kde.kdecoration2");

PreviewBridge::PreviewBridge(QObject *parent)
    : DecorationBridge(parent)
    , m_lastCreatedClient(nullptr)
    , m_lastCreatedSettings(nullptr)
    , m_valid(false)
{
    connect(this, &PreviewBridge::pluginChanged, this, &PreviewBridge::createFactory);
}

PreviewBridge::~PreviewBridge() = default;

std::unique_ptr<DecoratedClientPrivate> PreviewBridge::createClient(DecoratedClient *client, Decoration *decoration)
{
    auto ptr = std::unique_ptr<PreviewClient>(new PreviewClient(client, decoration));
    m_lastCreatedClient = ptr.get();
    return ptr;
}

void PreviewBridge::update(Decoration *decoration, const QRect &geometry)
{
    Q_UNUSED(geometry)
    auto it = std::find_if(m_previewItems.constBegin(), m_previewItems.constEnd(), [decoration](PreviewItem *item) {
        return item->decoration() == decoration;
    });
    if (it != m_previewItems.constEnd()) {
        (*it)->update();
    }
}

std::unique_ptr<DecorationSettingsPrivate> PreviewBridge::settings(DecorationSettings *parent)
{
    auto ptr = std::unique_ptr<PreviewSettings>(new PreviewSettings(parent));
    m_lastCreatedSettings = ptr.get();
    return ptr;
}

void PreviewBridge::registerPreviewItem(PreviewItem *item)
{
    m_previewItems.append(item);
}

void PreviewBridge::unregisterPreviewItem(PreviewItem *item)
{
    m_previewItems.removeAll(item);
}

void PreviewBridge::setPlugin(const QString &plugin)
{
    if (m_plugin == plugin) {
        return;
    }
    m_plugin = plugin;
    emit pluginChanged();
}

QString PreviewBridge::theme() const
{
    return m_theme;
}

void PreviewBridge::setTheme(const QString &theme)
{
    if (m_theme == theme) {
        return;
    }
    m_theme = theme;
    emit themeChanged();
}

QString PreviewBridge::plugin() const
{
    return m_plugin;
}

void PreviewBridge::createFactory()
{
    m_factory.clear();

    if (m_plugin.isNull()) {
        setValid(false);
        qWarning() << "Plugin not set";
        return;
    }

    const auto offers = KPluginTrader::self()->query(s_pluginName, s_pluginName);
    auto item = std::find_if(offers.constBegin(), offers.constEnd(), [this](const auto &plugin) { return plugin.pluginName() == m_plugin; });
    if (item != offers.constEnd()) {
        KPluginLoader loader(item->libraryPath());
        m_factory = loader.factory();
    }

    setValid(!m_factory.isNull());
}

bool PreviewBridge::isValid() const
{
    return m_valid;
}

void PreviewBridge::setValid(bool valid)
{
    if (m_valid == valid) {
        return;
    }
    m_valid = valid;
    emit validChanged();
}

Decoration *PreviewBridge::createDecoration(QObject *parent)
{
    if (!m_valid) {
        return nullptr;
    }
    QVariantMap args({ {QStringLiteral("bridge"), QVariant::fromValue(this)} });
    if (!m_theme.isNull()) {
        args.insert(QStringLiteral("theme"), m_theme);
    }
    return m_factory->create<KDecoration2::Decoration>(parent, QVariantList({args}));
}

DecorationButton *PreviewBridge::createButton(KDecoration2::Decoration *decoration, KDecoration2::DecorationButtonType type, QObject *parent)
{
    if (!m_valid) {
        return nullptr;
    }
    return m_factory->create<KDecoration2::DecorationButton>(QStringLiteral("button"), parent, QVariantList({QVariant::fromValue(type), QVariant::fromValue(decoration)}));
}

void PreviewBridge::configure(QQuickItem *ctx)
{
    if (!m_valid) {
        return;
    }
    //setup the UI
    QDialog *dialog = new QDialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if (m_lastCreatedClient) {
        dialog->setWindowTitle(m_lastCreatedClient->caption());
    }

    // create the KCModule through the plugintrader
    QVariantMap args;
    if (!m_theme.isNull()) {
        args.insert(QStringLiteral("theme"), m_theme);
    }
    KCModule *kcm = m_factory->create<KCModule>(QStringLiteral("kcmodule"), dialog, QVariantList({args}));
    if (!kcm) {
        return;
    }

    auto save = [this,kcm] {
        kcm->save();
        if (m_lastCreatedSettings) {
            emit m_lastCreatedSettings->decorationSettings()->reconfigured();
        }
        // Send signal to all kwin instances
        QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KWin"),
                                                          QStringLiteral("org.kde.KWin"),
                                                          QStringLiteral("reloadConfig"));
        QDBusConnection::sessionBus().send(message);
    };
    connect(dialog, &QDialog::accepted, this, save);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                     QDialogButtonBox::Cancel |
                                                     QDialogButtonBox::RestoreDefaults |
                                                     QDialogButtonBox::Reset,
                                                     dialog);

    QPushButton *reset = buttons->button(QDialogButtonBox::Reset);
    reset->setEnabled(false);
    // Here we connect our buttons with the dialog
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    connect(reset, &QPushButton::clicked, kcm, &KCModule::load);
    auto changedSignal = static_cast<void(KCModule::*)(bool)>(&KCModule::changed);
    connect(kcm, changedSignal, reset, &QPushButton::setEnabled);
    connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, kcm, &KCModule::defaults);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(kcm);
    layout->addWidget(buttons);

    if (ctx->window()) {
        dialog->winId(); // so it creates windowHandle
        dialog->windowHandle()->setTransientParent(QQuickRenderControl::renderWindowFor(ctx->window()));
        dialog->setModal(true);
    }

    dialog->show();
}

BridgeItem::BridgeItem(QObject *parent)
    : QObject(parent)
    , m_bridge(new PreviewBridge())
{
    connect(m_bridge, &PreviewBridge::themeChanged, this, &BridgeItem::themeChanged);
    connect(m_bridge, &PreviewBridge::pluginChanged, this, &BridgeItem::pluginChanged);
    connect(m_bridge, &PreviewBridge::validChanged, this, &BridgeItem::validChanged);
}

BridgeItem::~BridgeItem()
{
    m_bridge->deleteLater();
}

}
}
