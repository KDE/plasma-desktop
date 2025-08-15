/*
 *  SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KConfigWatcher>
#include <KDEDModule>

#include "kcmaccessibilityscreenreader.h"

class ScreenReader : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.screenreader")

public:
    ScreenReader(QObject *parent);

    Q_SCRIPTABLE void toggle();
    Q_SCRIPTABLE bool enabled() const;
    Q_SCRIPTABLE bool enabledByDefault() const;
    Q_SCRIPTABLE void setEnabled(bool enabled);

private:
    bool startScreenReaderSystemd();
    void startScreenReaderFallback();

    bool stopScreenReaderSystemd();
    void stopScreenReaderFallback();

    std::optional<qint64> m_orcaPid;
    ScreenReaderSettings m_settings;
};
