/*
 *   Copyright (C) 2014. 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>
 *   Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>
 *   Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Self
#include "switcherbackend.h"

// Qt
#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDateTime>
#include <QX11Info>

// Qml and QtQuick
#include <QQmlEngine>
#include <QQuickImageProvider>

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KIO/PreviewJob>
#include <KLocalizedString>
#include <KWindowSystem>
#include <windowtasksmodel.h>
#include <xwindowtasksmodel.h>

// X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

static const char *s_action_name_next_activity = "next activity";
static const char *s_action_name_previous_activity = "previous activity";

namespace
{
bool isPlatformX11()
{
    static const bool isX11 = QX11Info::isPlatformX11();
    return isX11;
}

// Taken from kwin/tabbox/tabbox.cpp
Display *x11_display()
{
    static Display *s_display = nullptr;
    if (!s_display) {
        s_display = QX11Info::display();
    }
    return s_display;
}

bool x11_areKeySymXsDepressed(bool bAll, const uint keySyms[], int nKeySyms)
{
    char keymap[32];

    XQueryKeymap(x11_display(), keymap);

    for (int iKeySym = 0; iKeySym < nKeySyms; iKeySym++) {
        uint keySymX = keySyms[iKeySym];
        uchar keyCodeX = XKeysymToKeycode(x11_display(), keySymX);
        int i = keyCodeX / 8;
        char mask = 1 << (keyCodeX - (i * 8));

        // Abort if bad index value,
        if (i < 0 || i >= 32)
            return false;

        // If ALL keys passed need to be depressed,
        if (bAll) {
            if ((keymap[i] & mask) == 0)
                return false;
        } else {
            // If we are looking for ANY key press, and this key is depressed,
            if (keymap[i] & mask)
                return true;
        }
    }

    // If we were looking for ANY key press, then none was found, return false,
    // If we were looking for ALL key presses, then all were found, return true.
    return bAll;
}

bool x11_areModKeysDepressed(const QKeySequence &seq)
{
    uint rgKeySyms[10];
    int nKeySyms = 0;
    if (seq.isEmpty()) {
        return false;
    }
    int mod = seq[seq.count() - 1] & Qt::KeyboardModifierMask;

    if (mod & Qt::SHIFT) {
        rgKeySyms[nKeySyms++] = XK_Shift_L;
        rgKeySyms[nKeySyms++] = XK_Shift_R;
    }
    if (mod & Qt::CTRL) {
        rgKeySyms[nKeySyms++] = XK_Control_L;
        rgKeySyms[nKeySyms++] = XK_Control_R;
    }
    if (mod & Qt::ALT) {
        rgKeySyms[nKeySyms++] = XK_Alt_L;
        rgKeySyms[nKeySyms++] = XK_Alt_R;
    }
    if (mod & Qt::META) {
        // It would take some code to determine whether the Win key
        // is associated with Super or Meta, so check for both.
        // See bug #140023 for details.
        rgKeySyms[nKeySyms++] = XK_Super_L;
        rgKeySyms[nKeySyms++] = XK_Super_R;
        rgKeySyms[nKeySyms++] = XK_Meta_L;
        rgKeySyms[nKeySyms++] = XK_Meta_R;
    }

    return x11_areKeySymXsDepressed(false, rgKeySyms, nKeySyms);
}

bool x11_isReverseTab(const QKeySequence &prevAction)
{
    if (prevAction == QKeySequence(Qt::ShiftModifier | Qt::Key_Tab)) {
        return x11_areModKeysDepressed(Qt::SHIFT);
    } else {
        return false;
    }
}

class ThumbnailImageResponse : public QQuickImageResponse
{
public:
    ThumbnailImageResponse(const QString &id, const QSize &requestedSize);

    QQuickTextureFactory *textureFactory() const override;

    void run();

private:
    QString m_id;
    QSize m_requestedSize;
    QQuickTextureFactory *m_texture = nullptr;
};

ThumbnailImageResponse::ThumbnailImageResponse(const QString &id, const QSize &requestedSize)
    : m_id(id)
    , m_requestedSize(requestedSize)
    , m_texture(nullptr)
{
    int width = m_requestedSize.width();
    int height = m_requestedSize.height();

    if (width <= 0) {
        width = 320;
    }

    if (height <= 0) {
        height = 240;
    }

    if (m_id.isEmpty()) {
        Q_EMIT finished();
        return;
    }

    const auto file = QUrl::fromUserInput(m_id);

    KFileItemList list;
    list.append(KFileItem(file, QString(), 0));

    auto job = KIO::filePreview(list, QSize(width, height));
    job->setScaleType(KIO::PreviewJob::Scaled);
    job->setIgnoreMaximumSize(true);

    connect(
        job,
        &KIO::PreviewJob::gotPreview,
        this,
        [this, file](const KFileItem &item, const QPixmap &pixmap) {
            Q_UNUSED(item);

            auto image = pixmap.toImage();

            m_texture = QQuickTextureFactory::textureFactoryForImage(image);
            Q_EMIT finished();
        },
        Qt::QueuedConnection);

    connect(job, &KIO::PreviewJob::failed, this, [this, job](const KFileItem &item) {
        Q_UNUSED(item);
        qWarning() << "SwitcherBackend: FAILED to get the thumbnail" << job->errorString() << job->detailedErrorStrings();
        Q_EMIT finished();
    });
}

QQuickTextureFactory *ThumbnailImageResponse::textureFactory() const
{
    return m_texture;
}

class ThumbnailImageProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override
    {
        return new ThumbnailImageResponse(id, requestedSize);
    }
};

} // local namespace

template<typename Handler>
inline void SwitcherBackend::registerShortcut(const QString &actionName, const QString &text, const QKeySequence &shortcut, Handler &&handler)
{
    auto action = new QAction(this);

    m_actionShortcut[actionName] = shortcut;

    action->setObjectName(actionName);
    action->setText(text);

    KGlobalAccel::self()->setShortcut(action, {shortcut});

    using KActivities::Controller;

    connect(action, &QAction::triggered, this, std::forward<Handler>(handler));
}

SwitcherBackend::SwitcherBackend(QObject *parent)
    : QObject(parent)
    , m_shouldShowSwitcher(false)
    , m_dropModeActive(false)
    , m_runningActivitiesModel(new SortedActivitiesModel({KActivities::Info::Running, KActivities::Info::Stopping}, this))
    , m_stoppedActivitiesModel(new SortedActivitiesModel({KActivities::Info::Stopped, KActivities::Info::Starting}, this))
{
    registerShortcut(QString::fromLatin1(s_action_name_next_activity),
                     i18n("Walk through activities"),
                     Qt::META | Qt::Key_Tab,
                     &SwitcherBackend::keybdSwitchToNextActivity);

    registerShortcut(QString::fromLatin1(s_action_name_previous_activity),
                     i18n("Walk through activities (Reverse)"),
                     Qt::META | Qt::SHIFT | Qt::Key_Tab,
                     &SwitcherBackend::keybdSwitchToPreviousActivity);

    connect(this, &SwitcherBackend::shouldShowSwitcherChanged, m_runningActivitiesModel, &SortedActivitiesModel::setInhibitUpdates);

    m_modKeyPollingTimer.setInterval(100);
    connect(&m_modKeyPollingTimer, &QTimer::timeout, this, &SwitcherBackend::showActivitySwitcherIfNeeded);

    m_dropModeHider.setInterval(500);
    m_dropModeHider.setSingleShot(true);
    connect(&m_dropModeHider, &QTimer::timeout, this, [this] {
        setShouldShowSwitcher(false);
    });

    connect(&m_activities, &KActivities::Controller::currentActivityChanged, this, &SwitcherBackend::onCurrentActivityChanged);
    m_previousActivity = m_activities.currentActivity();
}

SwitcherBackend::~SwitcherBackend()
{
}

QObject *SwitcherBackend::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)
    engine->addImageProvider(QStringLiteral("wallpaperthumbnail"), new ThumbnailImageProvider());
    return new SwitcherBackend();
}

void SwitcherBackend::keybdSwitchToNextActivity()
{
    if (isPlatformX11()) {
        // If we are on X11, we have all needed features for meta+tab
        // to work properly
        if (x11_isReverseTab(m_actionShortcut[QString::fromLatin1(s_action_name_previous_activity)])) {
            switchToActivity(Previous);
        } else {
            switchToActivity(Next);
        }

    } else {
        // If we are on wayland, just switch to the next activity
        switchToActivity(Next);
    }
}

void SwitcherBackend::keybdSwitchToPreviousActivity()
{
    switchToActivity(Previous);
}

void SwitcherBackend::switchToActivity(Direction direction)
{
    const auto activityToSet = m_runningActivitiesModel->relativeActivity(direction == Next ? 1 : -1);

    if (activityToSet.isEmpty())
        return;

    QTimer::singleShot(0, this, [this, activityToSet]() {
        setCurrentActivity(activityToSet);
    });

    keybdSwitchedToAnotherActivity();
}

void SwitcherBackend::keybdSwitchedToAnotherActivity()
{
    m_lastInvokedAction = dynamic_cast<QAction *>(sender());

    QTimer::singleShot(90, this, &SwitcherBackend::showActivitySwitcherIfNeeded);
}

void SwitcherBackend::showActivitySwitcherIfNeeded()
{
    if (!m_lastInvokedAction || m_dropModeActive) {
        return;
    }

    auto actionName = m_lastInvokedAction->objectName();

    if (!m_actionShortcut.contains(actionName)) {
        return;
    }

    if (isPlatformX11()) {
        if (!x11_areModKeysDepressed(m_actionShortcut[actionName])) {
            m_lastInvokedAction = nullptr;
            setShouldShowSwitcher(false);
            return;
        }

        setShouldShowSwitcher(true);

    } else {
        // We are not showing the switcher on wayland
        // TODO: This is a regression on wayland
        setShouldShowSwitcher(false);
    }
}

void SwitcherBackend::init()
{
    // nothing
}

void SwitcherBackend::onCurrentActivityChanged(const QString &id)
{
    if (m_shouldShowSwitcher) {
        // If we are showing the switcher because the user is
        // pressing Meta+Tab, we are not ready to commit the
        // activity change to memory
        return;
    }

    if (m_previousActivity == id)
        return;

    // Safe, we have a long-lived Consumer object
    KActivities::Info activity(id);
    Q_EMIT showSwitchNotification(id, activity.name(), activity.icon());

    KConfig config(QStringLiteral("kactivitymanagerd-switcher"));
    KConfigGroup times(&config, "LastUsed");

    const auto now = QDateTime::currentDateTime().toTime_t();

    // Updating the time for the activity we just switched to
    // in the case we do not power off properly, and on the next
    // start, kamd switches to another activity for some reason
    times.writeEntry(id, now);

    if (!m_previousActivity.isEmpty()) {
        // When leaving an activity, say goodbye and fondly remember
        // the last time we saw it
        times.writeEntry(m_previousActivity, now);
    }

    times.sync();

    m_previousActivity = id;
}

bool SwitcherBackend::shouldShowSwitcher() const
{
    return m_shouldShowSwitcher;
}

void SwitcherBackend::setShouldShowSwitcher(bool shouldShowSwitcher)
{
    if (m_shouldShowSwitcher == shouldShowSwitcher)
        return;

    m_shouldShowSwitcher = shouldShowSwitcher;

    if (m_shouldShowSwitcher) {
        // TODO: We really should NOT do this by polling
        m_modKeyPollingTimer.start();
    } else {
        m_modKeyPollingTimer.stop();

        // We might have an unprocessed onCurrentActivityChanged
        onCurrentActivityChanged(m_activities.currentActivity());
    }

    Q_EMIT shouldShowSwitcherChanged(m_shouldShowSwitcher);
}

QAbstractItemModel *SwitcherBackend::runningActivitiesModel() const
{
    return m_runningActivitiesModel;
}

QAbstractItemModel *SwitcherBackend::stoppedActivitiesModel() const
{
    return m_stoppedActivitiesModel;
}

void SwitcherBackend::setCurrentActivity(const QString &activity)
{
    m_activities.setCurrentActivity(activity);
}

void SwitcherBackend::stopActivity(const QString &activity)
{
    m_activities.stopActivity(activity);
}

bool SwitcherBackend::dropEnabled() const
{
#if HAVE_X11
    return true;
#else
    return false;
#endif
}

void SwitcherBackend::dropCopy(QMimeData *mimeData, const QVariant &activityId)
{
    drop(mimeData, Qt::ControlModifier, activityId);
}

void SwitcherBackend::dropMove(QMimeData *mimeData, const QVariant &activityId)
{
    drop(mimeData, 0, activityId);
}

void SwitcherBackend::drop(QMimeData *mimeData, int modifiers, const QVariant &activityId)
{
    setDropMode(false);

#if HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        bool ok = false;
        const QList<WId> &ids = TaskManager::XWindowTasksModel::winIdsFromMimeData(mimeData, &ok);

        if (!ok) {
            return;
        }

        const QString newActivity = activityId.toString();
        const QStringList runningActivities = m_activities.runningActivities();

        if (!runningActivities.contains(newActivity)) {
            return;
        }

        for (const auto &id : ids) {
            QStringList activities = KWindowInfo(id, NET::Properties(), NET::WM2Activities).activities();

            if (modifiers & Qt::ControlModifier) {
                // Add to the activity instead of moving.
                // This is a hack because the task manager reports that
                // is supports only the 'Move' DND action.
                if (!activities.contains(newActivity)) {
                    activities << newActivity;
                }

            } else {
                // Move to this activity
                // if on only one activity, set it to only the new activity
                // if on >1 activity, remove it from the current activity and add it to the new activity

                const QString currentActivity = m_activities.currentActivity();
                activities.removeAll(currentActivity);
                activities << newActivity;
            }

            KWindowSystem::setOnActivities(id, activities);
        }
    }
#endif
}

void SwitcherBackend::setDropMode(bool value)
{
    if (m_dropModeActive == value)
        return;

    m_dropModeActive = value;
    if (value) {
        setShouldShowSwitcher(true);
        m_dropModeHider.stop();
    } else {
        m_dropModeHider.start();
    }
}

void SwitcherBackend::toggleActivityManager()
{
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                  QStringLiteral("/PlasmaShell"),
                                                  QStringLiteral("org.kde.PlasmaShell"),
                                                  QStringLiteral("toggleActivityManager"));
    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}
