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
#include <QX11Info>
#include <QTimer>
#include <QDateTime>

// KDE
#include <kglobalaccel.h>
#include <klocalizedstring.h>
#include <KIO/PreviewJob>
#include <KConfig>
#include <KConfigGroup>

// X11
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xlib.h>

#define ACTION_NAME_NEXT_ACTIVITY "next activity"
#define ACTION_NAME_PREVIOUS_ACTIVITY "previous activity"

namespace {
    bool isPlatformX11()
    {
        static const bool isX11 = QX11Info::isPlatformX11();
        return isX11;
    }

    // Taken from kwin/tabbox/tabbox.cpp
    Display* x11_display()
    {
        static Display *s_display = nullptr;
        if (!s_display) {
            s_display = QX11Info::display();
        }
        return s_display;
    }

    bool x11_areKeySymXsDepressed(bool bAll, const uint keySyms[], int nKeySyms) {
        char keymap[32];

        XQueryKeymap(x11_display(), keymap);

        for (int iKeySym = 0; iKeySym < nKeySyms; iKeySym++) {
            uint keySymX = keySyms[ iKeySym ];
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

    bool x11_areModKeysDepressed(const QKeySequence& seq) {
        uint rgKeySyms[10];
        int nKeySyms = 0;
        if (seq.isEmpty()) {
            return false;
        }
        int mod = seq[seq.count()-1] & Qt::KeyboardModifierMask;

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

    bool x11_isReverseTab(const QKeySequence &prevAction) {

        if (prevAction == QKeySequence(Qt::ShiftModifier | Qt::Key_Tab)) {
            return x11_areModKeysDepressed(Qt::SHIFT);
        } else {
            return false;
        }
    }


} // local namespace

template <typename Handler>
inline void SwitcherBackend::registerShortcut(const QString &actionName,
                                              const QString &text,
                                              const QKeySequence &shortcut,
                                              Handler &&handler)
{
    auto action = new QAction(this);

    m_actionShortcut[actionName] = shortcut;

    action->setObjectName(actionName);
    action->setText(text);

    KGlobalAccel::self()->setShortcut(action, { shortcut });

    using KActivities::Controller;

    connect(action, &QAction::triggered, this, std::forward<Handler>(handler));
}

SwitcherBackend::SwitcherBackend(QObject *parent)
    : QObject(parent)
    , m_lastInvokedAction(Q_NULLPTR)
    , m_shouldShowSwitcher(false)
    , m_runningActivitiesModel(new SortedActivitiesModel({KActivities::Info::Running, KActivities::Info::Stopping}, this))
    , m_stoppedActivitiesModel(new SortedActivitiesModel({KActivities::Info::Stopped, KActivities::Info::Starting}, this))
{
    m_wallpaperCache = new KImageCache("activityswitcher_wallpaper_preview", 10485760);

    registerShortcut(ACTION_NAME_NEXT_ACTIVITY,
                     i18n("Walk through activities"),
                     Qt::META + Qt::Key_Tab,
                     &SwitcherBackend::keybdSwitchToNextActivity);

    registerShortcut(ACTION_NAME_PREVIOUS_ACTIVITY,
                     i18n("Walk through activities (Reverse)"),
                     Qt::META + Qt::SHIFT + Qt::Key_Tab,
                     &SwitcherBackend::keybdSwitchToPreviousActivity);

    connect(this, &SwitcherBackend::shouldShowSwitcherChanged,
            m_runningActivitiesModel, &SortedActivitiesModel::setInhibitUpdates);

    connect(&m_modKeyPollingTimer, &QTimer::timeout,
            this, &SwitcherBackend::showActivitySwitcherIfNeeded);
    connect(&m_activities, &KActivities::Controller::currentActivityChanged,
            this, &SwitcherBackend::onCurrentActivityChanged);
    m_previousActivity = m_activities.currentActivity();
}

SwitcherBackend::~SwitcherBackend()
{
    delete m_wallpaperCache;
}

QObject *SwitcherBackend::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new SwitcherBackend();
}

void SwitcherBackend::keybdSwitchToNextActivity()
{
    if (isPlatformX11()) {
        // If we are on X11, we have all needed features for meta+tab
        // to work properly
        if (x11_isReverseTab(m_actionShortcut[ACTION_NAME_PREVIOUS_ACTIVITY])) {
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
    const auto activityToSet =
        m_runningActivitiesModel->relativeActivity(direction == Next ? 1 : -1);

    if (activityToSet.isEmpty()) return;

    QTimer::singleShot(150, this, [this,activityToSet] () {
                setCurrentActivity(activityToSet);
            });

    keybdSwitchedToAnotherActivity();
}

void SwitcherBackend::keybdSwitchedToAnotherActivity()
{
    m_lastInvokedAction = dynamic_cast<QAction*>(sender());

    QTimer::singleShot(0, this, &SwitcherBackend::showActivitySwitcherIfNeeded);
}

void SwitcherBackend::showActivitySwitcherIfNeeded()
{
    if (!m_lastInvokedAction) {
        return;
    }

    auto actionName = m_lastInvokedAction->objectName();

    if (!m_actionShortcut.contains(actionName)) {
        return;
    }

    if (isPlatformX11()) {
        if (!x11_areModKeysDepressed(m_actionShortcut[actionName])) {
            m_lastInvokedAction = Q_NULLPTR;
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

    if (m_previousActivity == id) return;

    // Safe, we have a long-lived Consumer object
    KActivities::Info activity(id);
    emit showSwitchNotification(id, activity.name(), activity.icon());

    KConfig config("kactivitymanagerd-switcher");
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

void SwitcherBackend::setShouldShowSwitcher(const bool &shouldShowSwitcher)
{
    if (m_shouldShowSwitcher == shouldShowSwitcher) return;

    m_shouldShowSwitcher = shouldShowSwitcher;

    if (m_shouldShowSwitcher) {
        // TODO: We really should NOT do this by polling
        m_modKeyPollingTimer.start(100);
    } else {
        m_modKeyPollingTimer.stop();

        // We might have an unprocessed onCurrentActivityChanged
        onCurrentActivityChanged(m_activities.currentActivity());
    }

    emit shouldShowSwitcherChanged(m_shouldShowSwitcher);
}

QPixmap SwitcherBackend::wallpaperThumbnail(const QString &path, int width, int height,
            const QJSValue &_callback)
{
    QPixmap preview = QPixmap(QSize(1, 1));

    QJSValue callback(_callback);

    if (path.isEmpty()) {
        callback.call({false});
        return preview;
    }

    if (width == 0) {
        width = 320;
    }

    if (height == 0) {
        height = 240;
    }


    const auto pixmapKey = path + "/"
        + QString::number(width) + "x"
        + QString::number(height);

    if (m_wallpaperCache->findPixmap(pixmapKey, &preview)) {
        return preview;
    }

    QUrl file = QUrl::fromLocalFile(path);

    if (!m_previewJobs.contains(file) && file.isValid()) {
        m_previewJobs.insert(file);

        KFileItemList list;
        list.append(KFileItem(file, QString(), 0));

        KIO::PreviewJob* job =
            KIO::filePreview(list, QSize(width, height));
        job->setScaleType(KIO::PreviewJob::Scaled);
        job->setIgnoreMaximumSize(true);

        connect(job, &KIO::PreviewJob::gotPreview,
                this, [=] (const KFileItem& item, const QPixmap& pixmap) mutable {
                    Q_UNUSED(item);
                    m_wallpaperCache->insertPixmap(pixmapKey, pixmap);
                    m_previewJobs.remove(file);

                    callback.call({true});
                });

        connect(job, &KIO::PreviewJob::failed,
                this, [=] (const KFileItem& item) mutable {
                    Q_UNUSED(item);
                    m_previewJobs.remove(file);

                    qWarning() << "SwitcherBackend: FAILED to get the thumbnail for "
                               << path << job->detailedErrorStrings(&file);
                    callback.call({false});
                });

    }

    return preview;
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

#include "switcherbackend.moc"
