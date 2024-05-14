/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <KQuickManagedConfigModule>
#include <QColor>
#include <QString>

class MouseSettings;
class BellSettings;
class KeyboardSettings;
class KeyboardFiltersSettings;
class ActivationGesturesSettings;
class ScreenReaderSettings;
class AccessibilityData;
class ShakeCursorSettings;

class KAccessConfig : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(MouseSettings *mouseSettings READ mouseSettings CONSTANT)
    Q_PROPERTY(BellSettings *bellSettings READ bellSettings CONSTANT)
    Q_PROPERTY(KeyboardSettings *keyboardSettings READ keyboardSettings CONSTANT)
    Q_PROPERTY(KeyboardFiltersSettings *keyboardFiltersSettings READ keyboardFiltersSettings CONSTANT)
    Q_PROPERTY(ActivationGesturesSettings *activationGesturesSettings READ activationGesturesSettings CONSTANT)
    Q_PROPERTY(ScreenReaderSettings *screenReaderSettings READ screenReaderSettings CONSTANT)
    Q_PROPERTY(ShakeCursorSettings *shakeCursorSettings READ shakeCursorSettings CONSTANT)
    Q_PROPERTY(QString orcaLaunchFeedback READ orcaLaunchFeedback WRITE setOrcaLaunchFeedback NOTIFY orcaLaunchFeedbackChanged)
    Q_PROPERTY(QString desktopShortcutInfo MEMBER m_desktopShortcutInfo CONSTANT)
    Q_PROPERTY(bool screenReaderInstalled MEMBER m_screenReaderInstalled CONSTANT)
    Q_PROPERTY(bool bellIsDefaults READ bellIsDefaults NOTIFY bellIsDefaultsChanged)
    Q_PROPERTY(bool mouseIsDefaults READ mouseIsDefaults NOTIFY mouseIsDefaultsChanged)
    Q_PROPERTY(bool keyboardFiltersIsDefaults READ keyboardFiltersIsDefaults NOTIFY keyboardFiltersIsDefaultsChanged)
    Q_PROPERTY(bool keyboardModifiersIsDefaults READ keyboardModifiersIsDefaults NOTIFY keyboardModifiersIsDefaultsChanged)
    Q_PROPERTY(bool activationGesturesIsDefaults READ activationGesturesIsDefaults NOTIFY activationGesturesIsDefaultsChanged)
    Q_PROPERTY(bool screenReaderIsDefaults READ screenReaderIsDefaults NOTIFY screenReaderIsDefaultsChanged)
    Q_PROPERTY(bool shakeCursorIsDefaults READ shakeCursorIsDefaults NOTIFY shakeCursorIsDefaultsChanged)

public:
    KAccessConfig(QObject *parent, const KPluginMetaData &);
    ~KAccessConfig() override;

    void save() override;

    Q_INVOKABLE void configureKNotify();
    Q_INVOKABLE void launchOrcaConfiguration();
    Q_INVOKABLE bool orcaInstalled();

    QString orcaLaunchFeedback() const;

    MouseSettings *mouseSettings() const;
    BellSettings *bellSettings() const;
    KeyboardSettings *keyboardSettings() const;
    KeyboardFiltersSettings *keyboardFiltersSettings() const;
    ActivationGesturesSettings *activationGesturesSettings() const;
    ScreenReaderSettings *screenReaderSettings() const;
    ShakeCursorSettings *shakeCursorSettings() const;

    bool bellIsDefaults() const;
    bool mouseIsDefaults() const;
    bool keyboardFiltersIsDefaults() const;
    bool keyboardModifiersIsDefaults() const;
    bool activationGesturesIsDefaults() const;
    bool screenReaderIsDefaults() const;
    bool shakeCursorIsDefaults() const;

Q_SIGNALS:
    void orcaLaunchFeedbackChanged();
    void bellIsDefaultsChanged();
    void mouseIsDefaultsChanged();
    void keyboardFiltersIsDefaultsChanged();
    void keyboardModifiersIsDefaultsChanged();
    void activationGesturesIsDefaultsChanged();
    void screenReaderIsDefaultsChanged();
    void shakeCursorIsDefaultsChanged();

private:
    void setOrcaLaunchFeedback(const QString &value);

    AccessibilityData *m_data;
    QString m_orcaLaunchFeedback;
    QString m_desktopShortcutInfo;
    bool m_screenReaderInstalled;
};
