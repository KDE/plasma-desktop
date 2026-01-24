/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kcmaccess.h"

#include <math.h>
#include <stdlib.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileDialog>
#include <QProcess>
#include <QQuickItem>
#include <QStandardPaths>
#include <QValidator>
#include <QWindow>

#include <KActionCollection>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KKeyServer>
#include <KLocalizedString>
#include <KNotifyConfigWidget>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <KWindowSystem>

#include "kcmaccessibilityactivationgestures.h"
#include "kcmaccessibilitybell.h"
#include "kcmaccessibilitycolorblindnesscorrection.h"
#include "kcmaccessibilitydata.h"
#include "kcmaccessibilityinvert.h"
#include "kcmaccessibilitykeyboard.h"
#include "kcmaccessibilitykeyboardfilters.h"
#include "kcmaccessibilitymouse.h"
#include "kcmaccessibilityscreenreader.h"
#include "kcmaccessibilityshakecursor.h"
#include "kcmaccessibilityzoommagnifier.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMAccessFactory, "kcm_access.json", registerPlugin<KAccessConfig>(); registerPlugin<AccessibilityData>();)

class IntValidatorWithSuffix : public QIntValidator
{
    Q_GADGET
public:
    State validate(QString &text, int &pos) const override
    {
        QString strippedText;
        QRegularExpression re("\\d+");
        QRegularExpressionMatchIterator i = re.globalMatch(text);

        QString numbers;
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            strippedText += match.captured();
        }
        return QIntValidator::validate(strippedText, pos);
    }
};

KAccessConfig::KAccessConfig(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new AccessibilityData(this))
{
    qmlRegisterAnonymousType<MouseSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<BellSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<KeyboardSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<KeyboardFiltersSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ActivationGesturesSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ScreenReaderSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ShakeCursorSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ColorblindnessCorrectionSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<InvertSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ZoomMagnifierSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterType<IntValidatorWithSuffix>("org.kde.plasma.access.kcm", 0, 0, "IntValidatorWithSuffix");

    int tryOrcaRun = QProcess::execute(QStringLiteral("orca"), {QStringLiteral("--version")});
    m_screenReaderInstalled = tryOrcaRun != -2;

    setButtons(KAbstractConfigModule::Apply | KAbstractConfigModule::Default | KAbstractConfigModule::Help);

    connect(m_data->bellSettings(), &BellSettings::configChanged, this, &KAccessConfig::bellIsDefaultsChanged);
    connect(m_data->mouseSettings(), &MouseSettings::configChanged, this, &KAccessConfig::mouseIsDefaultsChanged);
    connect(m_data->activationGesturesSettings(), &ActivationGesturesSettings::configChanged, this, &KAccessConfig::activationGesturesIsDefaultsChanged);
    connect(m_data->keyboardFiltersSettings(), &KeyboardFiltersSettings::configChanged, this, &KAccessConfig::keyboardFiltersIsDefaultsChanged);
    connect(m_data->keyboardSettings(), &KeyboardSettings::configChanged, this, &KAccessConfig::keyboardModifiersIsDefaultsChanged);
    connect(m_data->screenReaderSettings(), &ScreenReaderSettings::configChanged, this, &KAccessConfig::screenReaderIsDefaultsChanged);
    connect(m_data->shakeCursorSettings(), &ShakeCursorSettings::configChanged, this, &KAccessConfig::shakeCursorIsDefaultsChanged);
    connect(m_data->colorblindnessCorrectionSettings(),
            &ColorblindnessCorrectionSettings::configChanged,
            this,
            &KAccessConfig::colorblindnessCorrectionIsDefaultsChanged);
    connect(m_data->invertSettings(), &InvertSettings::configChanged, this, &KAccessConfig::invertIsDefaultsChanged);
    connect(m_data->zoomMagnifierSettings(), &ZoomMagnifierSettings::configChanged, this, &KAccessConfig::zoomMagnifierIsDefaultsChanged);
}

KAccessConfig::~KAccessConfig()
{
}

void KAccessConfig::configureKNotify()
{
    auto *notifyWidget = KNotifyConfigWidget::configure(QApplication::activeWindow(), QStringLiteral("kaccess"));

    // HACK: Make dialog properly modal, non-ideal but safe
    QDialog *dialog = qobject_cast<QDialog *>(notifyWidget->parent());
    if (dialog) {
        dialog->hide();
        dialog->winId();
        dialog->windowHandle()->setTransientParent(QApplication::activeWindow()->windowHandle());
        dialog->setWindowModality(Qt::WindowModal);
        dialog->show();
    }
}

void KAccessConfig::configureInvertShortcuts()
{
    KShortcutsDialog *dialog = new KShortcutsDialog(KShortcutsEditor::GlobalAction, KShortcutsEditor::LetterShortcutsDisallowed);

    KActionCollection *actionCollection = new KActionCollection(dialog, QStringLiteral("kwin"));
    actionCollection->setComponentDisplayName(i18n("KWin"));

    QAction *a = actionCollection->addAction(QStringLiteral("Invert"));
    a->setText(i18n("Toggle Invert Effect"));
    a->setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_I));
    KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_I));

    QAction *b = actionCollection->addAction(QStringLiteral("InvertWindow"));
    b->setText(i18n("Toggle Invert Effect on Window"));
    b->setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setDefaultShortcut(b, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_U));
    KGlobalAccel::self()->setShortcut(b, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_U));

    dialog->addCollection(actionCollection);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->winId();
    dialog->windowHandle()->setTransientParent(QApplication::activeWindow()->windowHandle());
    dialog->setWindowModality(Qt::WindowModal);

    dialog->configure();
}

void KAccessConfig::configureZoomMagnifyShortcuts()
{
    KShortcutsDialog *dialog = new KShortcutsDialog(KShortcutsEditor::GlobalAction, KShortcutsEditor::LetterShortcutsDisallowed);

    KActionCollection *actionCollection = new KActionCollection(dialog, QStringLiteral("kwin"));
    actionCollection->setComponentDisplayName(i18n("KWin"));

    if (zoomMagnifierSettings()->zoom()) {
        // Show zoom effect shortcuts
        QAction *a;

        a = actionCollection->addAction(KStandardActions::ZoomIn);
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Plus) << (Qt::META | Qt::Key_Equal));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Plus) << (Qt::META | Qt::Key_Equal));

        a = actionCollection->addAction(KStandardActions::ZoomOut);
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Minus));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Minus));

        a = actionCollection->addAction(KStandardActions::ActualSize);
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_0));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_0));

        a = actionCollection->addAction(QStringLiteral("MoveZoomLeft"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
        a->setText(i18n("Move Left"));
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Left));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Left));

        a = actionCollection->addAction(QStringLiteral("MoveZoomRight"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
        a->setText(i18n("Move Right"));
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Right));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Right));

        a = actionCollection->addAction(QStringLiteral("MoveZoomUp"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
        a->setText(i18n("Move Up"));
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Up));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Up));

        a = actionCollection->addAction(QStringLiteral("MoveZoomDown"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
        a->setText(i18n("Move Down"));
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Down));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_Down));

        a = actionCollection->addAction(QStringLiteral("MoveMouseToFocus"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("view-restore")));
        a->setText(i18n("Move Mouse to Focus"));
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_F5));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_F5));

        a = actionCollection->addAction(QStringLiteral("MoveMouseToCenter"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("view-restore")));
        a->setText(i18n("Move Mouse to Center"));
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_F6));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_F6));
    } else if (zoomMagnifierSettings()->magnifier()) {
        // Show magnifier effect shortcuts
        QAction *a;

        a = actionCollection->addAction(KStandardActions::ZoomIn);
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Plus) << (Qt::META | Qt::Key_Equal));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Plus) << (Qt::META | Qt::Key_Equal));

        a = actionCollection->addAction(KStandardActions::ZoomOut);
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Minus));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_Minus));

        a = actionCollection->addAction(KStandardActions::ActualSize);
        a->setProperty("isConfigurationAction", true);
        KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_0));
        KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::META | Qt::Key_0));
    }

    dialog->addCollection(actionCollection);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->winId();
    dialog->windowHandle()->setTransientParent(QApplication::activeWindow()->windowHandle());
    dialog->setWindowModality(Qt::WindowModal);

    dialog->configure();
}

void KAccessConfig::launchOrcaConfiguration()
{
    const QStringList gsettingArgs = {QStringLiteral("set"),
                                      QStringLiteral("org.gnome.desktop.a11y.applications"),
                                      QStringLiteral("screen-reader-enabled"),
                                      QStringLiteral("true")};

    int ret = QProcess::execute(QStringLiteral("gsettings"), gsettingArgs);
    if (ret) {
        const QString errorStr = QLatin1String("gsettings ") + gsettingArgs.join(QLatin1Char(' '));
        setOrcaLaunchFeedback(i18n("Could not set gsettings for Orca: \"%1\" failed", errorStr));
        return;
    }

    qint64 pid = 0;
    bool started = QProcess::startDetached(QStringLiteral("orca"), {QStringLiteral("--setup")}, QString(), &pid);
    if (!started) {
        setOrcaLaunchFeedback(i18n("Error: Could not launch \"orca --setup\""));
    }
}

void KAccessConfig::save()
{
    const bool shakeCursorSaveNeeded = m_data->shakeCursorSettings()->findItem(QStringLiteral("ShakeCursor"))->isSaveNeeded();
    const bool shakeCursorMagnificationSaveNeeded = m_data->shakeCursorSettings()->findItem(QStringLiteral("ShakeCursorMagnification"))->isSaveNeeded();

    const bool colorblindnessCorrectionSaveNeeded =
        m_data->colorblindnessCorrectionSettings()->findItem(QStringLiteral("ColorblindnessCorrection"))->isSaveNeeded();
    const bool colorblindnessCorrectionSettingsSaveNeeded = m_data->colorblindnessCorrectionSettings()->findItem(QStringLiteral("Mode"))->isSaveNeeded()
        || m_data->colorblindnessCorrectionSettings()->findItem(QStringLiteral("Intensity"))->isSaveNeeded();

    const bool invertSaveNeeded = m_data->invertSettings()->findItem(QStringLiteral("Invert"))->isSaveNeeded();

    const bool zoomMagnifierSaveNeeded = m_data->zoomMagnifierSettings()->findItem(QStringLiteral("Zoom"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("Magnifier"))->isSaveNeeded();
    const bool zoomSettingsSaveNeeded = m_data->zoomMagnifierSettings()->findItem(QStringLiteral("SharedZoomFactor"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("ZoomMousePointer"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("ZoomMouseTracking"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("ZoomEnableFocusTracking"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("ZoomEnableTextCaretTracking"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("ZoomPixelGridZoom"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("ZoomPointerAxisGestureModifiers"))->isSaveNeeded();
    const bool magnifierSettingsSaveNeeded = m_data->zoomMagnifierSettings()->findItem(QStringLiteral("SharedZoomFactor"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("MagnifierWidth"))->isSaveNeeded()
        || m_data->zoomMagnifierSettings()->findItem(QStringLiteral("MagnifierHeight"))->isSaveNeeded();

    KQuickManagedConfigModule::save();

    if (bellSettings()->systemBell() || bellSettings()->customBell() || bellSettings()->visibleBell()) {
        KConfig _cfg(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
        KConfigGroup cfg(&_cfg, QStringLiteral("General"));
        cfg.writeEntry("UseSystemBell", true);
        cfg.sync();
    }

    // make kaccess reread the configuration
    // turning a11y features off needs to be done by kaccess
    // so run it to clear any enabled features and it will exit if it should
    QProcess::startDetached(QStringLiteral("kaccess"), {});

    if (shakeCursorSaveNeeded) {
        QDBusMessage reloadMessage =
            QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                           QStringLiteral("/Effects"),
                                           QStringLiteral("org.kde.kwin.Effects"),
                                           shakeCursorSettings()->shakeCursor() ? QStringLiteral("loadEffect") : QStringLiteral("unloadEffect"));
        reloadMessage.setArguments({QStringLiteral("shakecursor")});
        QDBusConnection::sessionBus().call(reloadMessage);
    }
    if (shakeCursorMagnificationSaveNeeded) {
        QDBusMessage reconfigureMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                         QStringLiteral("/Effects"),
                                                                         QStringLiteral("org.kde.kwin.Effects"),
                                                                         QStringLiteral("reconfigureEffect"));
        reconfigureMessage.setArguments({QStringLiteral("shakecursor")});
        QDBusConnection::sessionBus().call(reconfigureMessage);
    }

    if (colorblindnessCorrectionSaveNeeded) {
        QDBusMessage reloadMessage = QDBusMessage::createMethodCall(
            QStringLiteral("org.kde.KWin"),
            QStringLiteral("/Effects"),
            QStringLiteral("org.kde.kwin.Effects"),
            colorblindnessCorrectionSettings()->colorblindnessCorrection() ? QStringLiteral("loadEffect") : QStringLiteral("unloadEffect"));
        reloadMessage.setArguments({QStringLiteral("colorblindnesscorrection")});
        QDBusConnection::sessionBus().call(reloadMessage);
    }

    if (colorblindnessCorrectionSaveNeeded || colorblindnessCorrectionSettingsSaveNeeded) {
        QDBusMessage reconfigureMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                         QStringLiteral("/Effects"),
                                                                         QStringLiteral("org.kde.kwin.Effects"),
                                                                         QStringLiteral("reconfigureEffect"));
        reconfigureMessage.setArguments({QStringLiteral("colorblindnesscorrection")});
        QDBusConnection::sessionBus().call(reconfigureMessage);
    }

    if (invertSaveNeeded) {
        QDBusMessage reloadMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                    QStringLiteral("/Effects"),
                                                                    QStringLiteral("org.kde.kwin.Effects"),
                                                                    invertSettings()->invert() ? QStringLiteral("loadEffect") : QStringLiteral("unloadEffect"));
        reloadMessage.setArguments({QStringLiteral("invert")});
        QDBusConnection::sessionBus().call(reloadMessage);
    }

    if (zoomMagnifierSaveNeeded) {
        // Unload both effects
        QDBusMessage reloadMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                    QStringLiteral("/Effects"),
                                                                    QStringLiteral("org.kde.kwin.Effects"),
                                                                    QStringLiteral("unloadEffect"));
        reloadMessage.setArguments({QStringLiteral("zoom")});
        QDBusConnection::sessionBus().call(reloadMessage);
        reloadMessage.setArguments({QStringLiteral("magnifier")});
        QDBusConnection::sessionBus().call(reloadMessage);

        if (zoomMagnifierSettings()->zoom() || zoomMagnifierSettings()->magnifier()) {
            // Load the specified effect
            QDBusMessage reloadMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                        QStringLiteral("/Effects"),
                                                                        QStringLiteral("org.kde.kwin.Effects"),
                                                                        QStringLiteral("loadEffect"));
            reloadMessage.setArguments({zoomMagnifierSettings()->zoom() ? QStringLiteral("zoom") : QStringLiteral("magnifier")});
            QDBusConnection::sessionBus().call(reloadMessage);
        }
    }

    if (zoomSettingsSaveNeeded) {
        QDBusMessage reconfigureMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                         QStringLiteral("/Effects"),
                                                                         QStringLiteral("org.kde.kwin.Effects"),
                                                                         QStringLiteral("reconfigureEffect"));
        reconfigureMessage.setArguments({QStringLiteral("zoom")});
        QDBusConnection::sessionBus().call(reconfigureMessage);
    }

    if (magnifierSettingsSaveNeeded) {
        QDBusMessage reconfigureMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                         QStringLiteral("/Effects"),
                                                                         QStringLiteral("org.kde.kwin.Effects"),
                                                                         QStringLiteral("reconfigureEffect"));
        reconfigureMessage.setArguments({QStringLiteral("magnifier")});
        QDBusConnection::sessionBus().call(reconfigureMessage);
    }
}

QString KAccessConfig::orcaLaunchFeedback() const
{
    return m_orcaLaunchFeedback;
}

void KAccessConfig::setOrcaLaunchFeedback(const QString &value)
{
    if (m_orcaLaunchFeedback != value) {
        m_orcaLaunchFeedback = value;
        Q_EMIT orcaLaunchFeedbackChanged();
    }
}

bool KAccessConfig::orcaInstalled()
{
    int tryOrcaRun = QProcess::execute(QStringLiteral("orca"), {QStringLiteral("--version")});
    // If the process cannot be started, -2 is returned.
    return tryOrcaRun != -2;
}

bool KAccessConfig::isPlatformX11() const
{
    return KWindowSystem::isPlatformX11();
}

MouseSettings *KAccessConfig::mouseSettings() const
{
    return m_data->mouseSettings();
}

BellSettings *KAccessConfig::bellSettings() const
{
    return m_data->bellSettings();
}

KeyboardSettings *KAccessConfig::keyboardSettings() const
{
    return m_data->keyboardSettings();
}

KeyboardFiltersSettings *KAccessConfig::keyboardFiltersSettings() const
{
    return m_data->keyboardFiltersSettings();
}

ActivationGesturesSettings *KAccessConfig::activationGesturesSettings() const
{
    return m_data->activationGesturesSettings();
}

ScreenReaderSettings *KAccessConfig::screenReaderSettings() const
{
    return m_data->screenReaderSettings();
}

ShakeCursorSettings *KAccessConfig::shakeCursorSettings() const
{
    return m_data->shakeCursorSettings();
}

ColorblindnessCorrectionSettings *KAccessConfig::colorblindnessCorrectionSettings() const
{
    return m_data->colorblindnessCorrectionSettings();
}

InvertSettings *KAccessConfig::invertSettings() const
{
    return m_data->invertSettings();
}

ZoomMagnifierSettings *KAccessConfig::zoomMagnifierSettings() const
{
    return m_data->zoomMagnifierSettings();
}

bool KAccessConfig::bellIsDefaults() const
{
    return bellSettings()->isDefaults();
}

bool KAccessConfig::mouseIsDefaults() const
{
    return mouseSettings()->isDefaults();
}

bool KAccessConfig::keyboardFiltersIsDefaults() const
{
    return keyboardFiltersSettings()->isDefaults();
}

bool KAccessConfig::keyboardModifiersIsDefaults() const
{
    return keyboardSettings()->isDefaults();
}

bool KAccessConfig::activationGesturesIsDefaults() const
{
    return activationGesturesSettings()->isDefaults();
}

bool KAccessConfig::screenReaderIsDefaults() const
{
    return screenReaderSettings()->isDefaults();
}

bool KAccessConfig::shakeCursorIsDefaults() const
{
    return shakeCursorSettings()->isDefaults();
}

bool KAccessConfig::colorblindnessCorrectionIsDefaults() const
{
    return colorblindnessCorrectionSettings()->isDefaults();
}

bool KAccessConfig::invertIsDefaults() const
{
    return invertSettings()->isDefaults();
}

bool KAccessConfig::zoomMagnifierIsDefaults() const
{
    return zoomMagnifierSettings()->isDefaults();
}

#include "kcmaccess.moc"
#include "moc_kcmaccess.cpp"
