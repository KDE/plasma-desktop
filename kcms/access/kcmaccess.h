/*
    Copyright 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    Copyright 2014 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __kcmaccess_h__
#define __kcmaccess_h__

#include <KQuickAddons/ManagedConfigModule>
#include <QColor>
#include <QString>

class MouseSettings;
class BellSettings;
class KeyboardSettings;
class KeyboardFiltersSettings;
class ScreenReaderSettings;
class AccessibilityData;

class KAccessConfig : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(MouseSettings *mouseSettings READ mouseSettings CONSTANT)
    Q_PROPERTY(BellSettings *bellSettings READ bellSettings CONSTANT)
    Q_PROPERTY(KeyboardSettings *keyboardSettings READ keyboardSettings CONSTANT)
    Q_PROPERTY(KeyboardFiltersSettings *keyboardFiltersSettings READ keyboardFiltersSettings CONSTANT)
    Q_PROPERTY(ScreenReaderSettings *screenReaderSettings READ screenReaderSettings CONSTANT)
    Q_PROPERTY(QString orcaLaunchFeedback READ orcaLaunchFeedback WRITE setOrcaLaunchFeedback NOTIFY orcaLaunchFeedbackChanged)
    Q_PROPERTY(QString desktopShortcutInfo MEMBER m_desktopShortcutInfo CONSTANT)
    Q_PROPERTY(bool screenReaderInstalled MEMBER m_screenReaderInstalled CONSTANT)
    Q_PROPERTY(bool bellIsDefaults READ bellIsDefaults NOTIFY bellIsDefaultsChanged)
    Q_PROPERTY(bool mouseIsDefaults READ mouseIsDefaults NOTIFY mouseIsDefaultsChanged)
    Q_PROPERTY(bool keyboardFiltersIsDefaults READ keyboardFiltersIsDefaults NOTIFY keyboardFiltersIsDefaultsChanged)
    Q_PROPERTY(bool keyboardModifiersIsDefaults READ keyboardModifiersIsDefaults NOTIFY keyboardModifiersIsDefaultsChanged)
    Q_PROPERTY(bool screenReaderIsDefaults READ screenReaderIsDefaults NOTIFY screenReaderIsDefaultsChanged)

public:
    KAccessConfig(QObject *parent, const QVariantList &);
    ~KAccessConfig() override;

    void save() override;

    Q_INVOKABLE void configureKNotify(QQuickItem *parent);
    Q_INVOKABLE void launchOrcaConfiguration();
    Q_INVOKABLE bool orcaInstalled();

    QString orcaLaunchFeedback() const;

    MouseSettings *mouseSettings() const;
    BellSettings *bellSettings() const;
    KeyboardSettings *keyboardSettings() const;
    KeyboardFiltersSettings *keyboardFiltersSettings() const;
    ScreenReaderSettings *screenReaderSettings() const;

    bool bellIsDefaults() const;
    bool mouseIsDefaults() const;
    bool keyboardFiltersIsDefaults() const;
    bool keyboardModifiersIsDefaults() const;
    bool screenReaderIsDefaults() const;

Q_SIGNALS:
    void orcaLaunchFeedbackChanged();
    void bellIsDefaultsChanged();
    void mouseIsDefaultsChanged();
    void keyboardFiltersIsDefaultsChanged();
    void keyboardModifiersIsDefaultsChanged();
    void screenReaderIsDefaultsChanged();

private:
    void setOrcaLaunchFeedback(const QString &value);

    AccessibilityData *m_data;
    QString m_orcaLaunchFeedback;
    QString m_desktopShortcutInfo;
    bool m_screenReaderInstalled;
};

#endif
