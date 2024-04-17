/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configcontainer.h"

#include "cursortheme.h"
#include "inputbackend.h"
#include "libinput_config.h"
#include "logging.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowSystem>

#include <memory> // std::unique_ptr

extern "C" {
Q_DECL_EXPORT void kcminit()
{
    std::unique_ptr<InputBackend> backend(InputBackend::implementation());
    if (backend) {
        backend->kcmInit();
    }

#if BUILD_KCM_MOUSE_X11
    if (KWindowSystem::isPlatformX11()) {
        auto config = KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals);
        KConfigGroup group = config->group(QStringLiteral("Mouse"));
        const QString theme = group.readEntry("cursorTheme", QStringLiteral("breeze_cursors"));
        const int size = group.readEntry("cursorSize", 24);

        // Note: If you update this code, update kapplymousetheme as well.

        CursorTheme::applyCursorTheme(theme, size);
    }
#endif
}
}

ConfigContainer::ConfigContainer(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    InputBackend *backend = InputBackend::implementation(this);

    if (!backend) {
        qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
        return;
    }

    m_config = new LibinputConfig(this, backend);
}

void ConfigContainer::load()
{
    if (m_config) {
        m_config->load();
    }
}

void ConfigContainer::save()
{
    if (m_config) {
        m_config->save();
    }
}

void ConfigContainer::defaults()
{
    if (m_config) {
        m_config->defaults();
    }
}
