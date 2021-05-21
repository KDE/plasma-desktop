/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
