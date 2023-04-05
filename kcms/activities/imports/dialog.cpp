/*
    SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "dialog.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QPushButton>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWidget>
#include <QString>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <KGlobalAccel>
#include <KLocalizedString>
#include <KMessageWidget>

#include "../kactivities-kcm-features.h"

#include "features_interface.h"
#include "kactivities/controller.h"
#include "kactivities/info.h"

#include "common/dbus/common.h"
#include "utils/continue_with.h"
#include "utils/d_ptr_implementation.h"

class Dialog::Private
{
public:
    Private(Dialog *parent)
        : q(parent)
        , activityName(QStringLiteral("activityName"))
        , activityDescription(QStringLiteral("activityDescription"))
        , activityIcon(QStringLiteral("activityIcon"))
        , activityWallpaper(QStringLiteral("activityWallpaper"))
        , activityIsPrivate(true)
        , activityShortcut(QStringLiteral("activityShortcut"))
        , features(new org::kde::ActivityManager::Features(QStringLiteral(KAMD_DBUS_SERVICE),
                                                           QStringLiteral(KAMD_DBUS_FEATURES_PATH),
                                                           QDBusConnection::sessionBus(),
                                                           q))
    {
    }

    Dialog *const q;
    QVBoxLayout *layout;

    QQuickWidget *view;
    KMessageWidget *message;
    QDialogButtonBox *buttons;
    QString defaultOKText;

    QString activityId;

    QString activityName;
    QString activityDescription;
    QString activityIcon;
    QString activityWallpaper;
    bool activityIsPrivate;
    QString activityShortcut;

    KActivities::Controller activities;
    org::kde::ActivityManager::Features *features;
};

void Dialog::showDialog(const QString &id)
{
    static Dialog *dialog = nullptr;

    // If we use the regular singleton here (static value instead of static ptr),
    // we will crash on exit because of Qt...
    if (!dialog) {
        dialog = new Dialog();
    }

    dialog->init(id);
    dialog->show();
}

Dialog::Dialog(QObject *parent)
    : QDialog()
    , d(this)
{
    resize(550, 400);

    d->layout = new QVBoxLayout(this);

    // Message widget for showing errors
    d->message = new KMessageWidget(this);
    d->message->setMessageType(KMessageWidget::Error);
    d->message->setVisible(false);
    d->message->setWordWrap(true);
    d->layout->addWidget(d->message);

    // Main view
    d->view = new QQuickWidget();
    d->view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->view->rootContext()->setContextProperty(QStringLiteral("dialog"), this);
    d->view->rootContext()->setContextObject(new KLocalizedContext(d->view));

    const QString sourceFile = QStringLiteral(KAMD_KCM_DATADIR) + "qml/activityDialog/GeneralTab.qml";

    if (QFile::exists(sourceFile)) {
        d->view->setSource(QUrl::fromLocalFile(sourceFile));
    } else {
        d->message->setText(i18n("Error loading the QML files. Check your installation.\nMissing %1", sourceFile));
        d->message->setVisible(true);
    }

    d->layout->addWidget(d->view);

    // Buttons
    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->layout->QLayout::addWidget(d->buttons);

    connect(d->buttons->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, this, &Dialog::save);
    connect(d->buttons, &QDialogButtonBox::rejected, this, &Dialog::close);

    d->defaultOKText = d->buttons->button(QDialogButtonBox::Ok)->text();
}

void Dialog::init(const QString &activityId)
{
    setWindowTitle(activityId.isEmpty() ? i18nc("@title:window", "Create a New Activity") //
                                        : i18nc("@title:window", "Activity Settings"));

    d->buttons->button(QDialogButtonBox::Ok)->setText(activityId.isEmpty() ? i18nc("@action:button", "Create") : d->defaultOKText);

    setActivityId(activityId);
    setActivityName(QString());
    setActivityDescription(QString());
    setActivityIcon(QStringLiteral("activities"));
    setActivityIsPrivate(false);

    setActivityShortcut(QKeySequence());

    if (!activityId.isEmpty()) {
        KActivities::Info activityInfo(activityId);

        setActivityName(activityInfo.name());
        setActivityDescription(activityInfo.description());
        setActivityIcon(activityInfo.icon());

        // finding the key shortcut
        const auto shortcuts = KGlobalAccel::self()->globalShortcut(QStringLiteral("ActivityManager"), QStringLiteral("switch-to-activity-") + activityId);
        setActivityShortcut(shortcuts.isEmpty() ? QKeySequence() : shortcuts.first());

        // is private?
        auto result = d->features->GetValue(QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/") + activityId);

        auto watcher = new QDBusPendingCallWatcher(result, this);

        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [&](QDBusPendingCallWatcher *watcher) mutable {
            QDBusPendingReply<QDBusVariant> reply = *watcher;
            setActivityIsPrivate(reply.value().variant().toBool());
            watcher->deleteLater();
        });
    }
}

Dialog::~Dialog()
{
}

#define IMPLEMENT_PROPERTY(Type, PType, PropName)                                                                                                              \
    Type Dialog::activity##PropName() const                                                                                                                    \
    {                                                                                                                                                          \
        auto root = d->view->rootObject();                                                                                                                     \
                                                                                                                                                               \
        if (!root) {                                                                                                                                           \
            qDebug() << "Root does not exist";                                                                                                                 \
            return Type();                                                                                                                                     \
        }                                                                                                                                                      \
                                                                                                                                                               \
        return root->property("activity" #PropName).value<Type>();                                                                                             \
    }                                                                                                                                                          \
                                                                                                                                                               \
    void Dialog::setActivity##PropName(PType value)                                                                                                            \
    {                                                                                                                                                          \
        auto root = d->view->rootObject();                                                                                                                     \
                                                                                                                                                               \
        if (!root) {                                                                                                                                           \
            qDebug() << "Root does not exist";                                                                                                                 \
            return;                                                                                                                                            \
        }                                                                                                                                                      \
                                                                                                                                                               \
        root->setProperty("activity" #PropName, value);                                                                                                        \
    }

IMPLEMENT_PROPERTY(QString, const QString &, Id)
IMPLEMENT_PROPERTY(QString, const QString &, Name)
IMPLEMENT_PROPERTY(QString, const QString &, Description)
IMPLEMENT_PROPERTY(QString, const QString &, Icon)
IMPLEMENT_PROPERTY(QString, const QString &, Wallpaper)
IMPLEMENT_PROPERTY(QKeySequence, const QKeySequence &, Shortcut)
IMPLEMENT_PROPERTY(bool, bool, IsPrivate)
#undef IMPLEMENT_PROPERTY

void Dialog::save()
{
    if (activityId().isEmpty()) {
        create();

    } else {
        saveChanges(activityId());
    }
}

void Dialog::create()
{
    using namespace kamd::utils;
    continue_with(d->activities.addActivity(activityName()), [this](const optional_view<QString> &activityId) {
        if (activityId.is_initialized()) {
            saveChanges(activityId.get());
        }
    });
}

void Dialog::saveChanges(const QString &activityId)
{
    d->activities.setActivityName(activityId, activityName());
    d->activities.setActivityDescription(activityId, activityDescription());
    d->activities.setActivityIcon(activityId, activityIcon());

    // setting the key shortcut
    QAction action(nullptr);
    action.setProperty("isConfigurationAction", true);
    action.setProperty("componentName", QStringLiteral("ActivityManager"));
    action.setObjectName(QStringLiteral("switch-to-activity-") + activityId);
    KGlobalAccel::self()->setShortcut(&action, {activityShortcut()}, KGlobalAccel::NoAutoloading);

    // is private?
    d->features->SetValue(QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/") + activityId, QDBusVariant(activityIsPrivate()));

    close();
}
