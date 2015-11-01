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
// #include <QDebug>
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
    // Taken from kwin/tabbox/tabbox.cpp
    Display* display()
    {
        static Display *s_display = nullptr;
        if (!s_display) {
            s_display = QX11Info::display();
        }
        return s_display;
    }

    bool areKeySymXsDepressed(bool bAll, const uint keySyms[], int nKeySyms) {
        char keymap[32];

        XQueryKeymap(display(), keymap);

        for (int iKeySym = 0; iKeySym < nKeySyms; iKeySym++) {
            uint keySymX = keySyms[ iKeySym ];
            uchar keyCodeX = XKeysymToKeycode(display(), keySymX);
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

    bool areModKeysDepressed(const QKeySequence& seq) {
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

        return areKeySymXsDepressed(false, rgKeySyms, nKeySyms);
    }

    bool isReverseTab(const QKeySequence &prevAction) {

        if (Qt::SHIFT & prevAction && Qt::Key_Tab & prevAction) {
            return areModKeysDepressed(Qt::SHIFT);
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

    connect(&m_modKeyPollingTimer, &QTimer::timeout,
            this, &SwitcherBackend::showActivitySwitcherIfNeeded);
    connect(&m_activities, &KActivities::Controller::currentActivityChanged,
            this, &SwitcherBackend::currentActivityChangedSlot);
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
    if (isReverseTab(m_actionShortcut[ACTION_NAME_PREVIOUS_ACTIVITY])) {
        switchToActivity(Previous);
    } else {
        switchToActivity(Next);
    }
}

void SwitcherBackend::keybdSwitchToPreviousActivity()
{
    switchToActivity(Previous);
}

void SwitcherBackend::switchToActivity(Direction direction)
{
    auto runningActivities
        = m_activities.activities(KActivities::Info::Running);

    if (runningActivities.count() == 0) {
        return;
    }

    // Sorting this every time is not really (or at all) efficient,
    // but at least we do not need to connect to too many Info objects
    std::sort(runningActivities.begin(), runningActivities.end(),
        [] (const QString &left, const QString &right) {
            using KActivities::Info;
            const QString &leftName = Info(left).name().toLower();
            const QString &rightName = Info(right).name().toLower();

            return
                (leftName < rightName) ||
                (leftName == rightName && left < right);
        });

    auto index = std::max(
        0, runningActivities.indexOf(m_activities.currentActivity()));

    index += direction == Next ? 1 : -1;

    if (index < 0) {
        index = runningActivities.count() - 1;
    } else if (index >= runningActivities.count()) {
        index = 0;
    }

    // TODO: This is evil, but plasmashell goes into a dead-lock if
    // the activity is changed while one tries to open the switcher O.o
    // m_activities.setCurrentActivity(runningActivities[index]);
    const auto activityToSet = runningActivities[index];
    QTimer::singleShot(150, this, [this,activityToSet] () {
                m_activities.setCurrentActivity(activityToSet);
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

    if (!areModKeysDepressed(m_actionShortcut[actionName])) {
        m_lastInvokedAction = Q_NULLPTR;
        setShouldShowSwitcher(false);
        return;
    }

    setShouldShowSwitcher(true);

}

void SwitcherBackend::init()
{
    // nothing
}

void SwitcherBackend::currentActivityChangedSlot(const QString &id)
{
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
        // the time we saw it
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

    // qDebug() << "SwitcherBackend: Requesting wallpaper: " << path << width << height;

    if (width == 0) {
        width = 320;
    }

    if (height == 0) {
        height = 240;
    }


    const auto pixmapKey = path + "/"
        + QString::number(width) + "x"
        + QString::number(height);

    // qDebug() << "SwitcherBackend: Wallpaper cache id is: " << pixmapKey;

    if (m_wallpaperCache->findPixmap(pixmapKey, &preview)) {
        return preview;
    }

    QUrl file(path);

    // qDebug() << "SwitcherBackend: Cache miss. We need to generate the thumbnail: " << file;

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
                    m_previewJobs.remove(path);

                    // qDebug() << "SwitcherBackend: Got the thumbnail for " << path << "saving under" << pixmapKey;
                    callback.call({true});
                });

        connect(job, &KIO::PreviewJob::failed,
                this, [=] (const KFileItem& item) mutable {
                    Q_UNUSED(item);
                    m_previewJobs.remove(path);

                    qWarning() << "SwitcherBackend: FAILED to get the thumbnail for "
                               << path << job->detailedErrorStrings(&file);
                    callback.call({false});
                });

    }

    return preview;
}

QString SwitcherBackend::lastTimeUsedString(const QString &activity)
{
    KConfig config("kactivitymanagerd-switcher");
    KConfigGroup times(&config, "LastUsed");

    const auto now = QDateTime::currentDateTime().toTime_t();
    const auto time = times.readEntry(activity, 0);

    if (time == 0) return i18n("Used some time ago");

    auto diff = now - time;

    // We do not need to be precise
    const auto seconds = diff % 60; diff /= 60;
    const auto minutes = diff % 60; diff /= 60;
    const auto hours   = diff % 24; diff /= 24;
    const auto days    = diff % 30; diff /= 30;
    const auto months  = diff % 12; diff /= 12;
    const auto years   = diff;

    return (years > 0)   ? i18n("Used a long time ago")
         : (months > 0)  ? i18ncp("amount in months",  "Used a month ago",  "Used %1 months ago", months)
         : (days > 0)    ? i18ncp("amount in days",    "Used a day ago",    "Used %1 days ago",   days)
         : (hours > 0)   ? i18ncp("amount in hours",   "Used an hour ago",  "Used %1 hours ago",  hours)
         : (minutes > 0) ? i18ncp("amount in minutes", "Used a minute ago", "Used %1 minutes ago",  minutes)
         :                 i18n("Used a moment ago");
}


#include "switcherbackend.moc"
