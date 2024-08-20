/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xkb_helper.h"
#include "debug.h"

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include <QTime>
#include <QtGui/private/qtx11extras_p.h>

#include "keyboard_config.h"
#include "keyboardsettings.h"
#include "x11_helper.h"

static const char SETXKBMAP_EXEC[] = "setxkbmap";
static const char XMODMAP_EXEC[] = "xmodmap";

static bool setxkbmapNotFound = false;
static QString setxkbmapExe;

static bool xmodmapNotFound = false;
static QString xmodmapExe;

static const QString COMMAND_OPTIONS_SEPARATOR(QStringLiteral(","));

static QString getSetxkbmapExe()
{
    if (setxkbmapNotFound)
        return QString();

    if (setxkbmapExe.isEmpty()) {
        setxkbmapExe = QStandardPaths::findExecutable(SETXKBMAP_EXEC);
        if (setxkbmapExe.isEmpty()) {
            setxkbmapNotFound = true;
            qCCritical(KCM_KEYBOARD) << "Can't find" << SETXKBMAP_EXEC << "- keyboard layouts won't be configured";
            return QString();
        }
    }
    return setxkbmapExe;
}

static void executeXmodmap(const QString &configFileName)
{
    if (xmodmapNotFound)
        return;

    if (QFile(configFileName).exists()) {
        if (xmodmapExe.isEmpty()) {
            xmodmapExe = QStandardPaths::findExecutable(XMODMAP_EXEC);
            if (xmodmapExe.isEmpty()) {
                xmodmapNotFound = true;
                qCCritical(KCM_KEYBOARD) << "Can't find" << XMODMAP_EXEC << "- xmodmap files won't be run";
                return;
            }
        }

        qCDebug(KCM_KEYBOARD) << "Executing" << xmodmapExe << configFileName;
        const int res = QProcess::execute(xmodmapExe, QStringList{configFileName});
        if (res != 0) {
            qCCritical(KCM_KEYBOARD) << "Failed with return code:" << res;
        }
    }
}

static void restoreXmodmap()
{
    // TODO: is just home .Xmodmap enough or should system be involved too?
    //    QString configFileName = QDir("/etc/X11/xinit").filePath(".Xmodmap");
    //    executeXmodmap(configFileName);
    QString configFileName = QDir::home().filePath(QStringLiteral(".Xmodmap"));
    executeXmodmap(configFileName);
}

// TODO: make private
bool XkbHelper::runConfigLayoutCommand(const QStringList &setxkbmapCommandArguments)
{
    QElapsedTimer timer;
    timer.start();

    const auto setxkbmapExe = getSetxkbmapExe();
    if (setxkbmapExe.isEmpty()) {
        return false;
    }

    qCDebug(KCM_KEYBOARD) << "Running" << setxkbmapExe << setxkbmapCommandArguments.join(QLatin1Char(' '));

    const int res = QProcess::execute(setxkbmapExe, setxkbmapCommandArguments);
    if (res != 0) {
        qCCritical(KCM_KEYBOARD) << "Failed with return code:" << res;
        return false;
    }

    // restore Xmodmap mapping reset by setxkbmap
    qCDebug(KCM_KEYBOARD) << "Executed successfully in " << timer.elapsed() << "ms";
    restoreXmodmap();
    qCDebug(KCM_KEYBOARD) << "\t and with xmodmap" << timer.elapsed() << "ms";
    return true;
}

bool XkbHelper::initializeKeyboardLayouts(const QList<LayoutUnit> &layoutUnits)
{
    QStringList layouts;
    QStringList variants;
    for (const auto &layoutUnit : layoutUnits) {
        layouts.append(layoutUnit.layout());
        variants.append(layoutUnit.variant());
    }

    QStringList setxkbmapCommandArguments;
    setxkbmapCommandArguments.append(QStringLiteral("-layout"));
    setxkbmapCommandArguments.append(layouts.join(COMMAND_OPTIONS_SEPARATOR));
    if (!variants.join(QString()).isEmpty()) {
        setxkbmapCommandArguments.append(QStringLiteral("-variant"));
        setxkbmapCommandArguments.append(variants.join(COMMAND_OPTIONS_SEPARATOR));
    }

    return runConfigLayoutCommand(setxkbmapCommandArguments);
}

bool XkbHelper::initializeKeyboardLayouts(KeyboardConfig &config)
{
    QStringList setxkbmapCommandArguments;
    if (!config.keyboardSettings()->keyboardModel().isEmpty()) {
        XkbConfig xkbConfig;
        X11Helper::getGroupNames(QX11Info::display(), &xkbConfig, X11Helper::MODEL_ONLY);
        if (xkbConfig.keyboardModel != config.keyboardSettings()->keyboardModel()) {
            setxkbmapCommandArguments.append(QStringLiteral("-model"));
            setxkbmapCommandArguments.append(config.keyboardSettings()->keyboardModel());
        }
    }
    if (config.keyboardSettings()->configureLayouts()) {
        QStringList layouts;
        QStringList variants;
        const QList<LayoutUnit> defaultLayouts = config.getDefaultLayouts();
        for (const auto &layoutUnit : defaultLayouts) {
            layouts.append(layoutUnit.layout());
            variants.append(layoutUnit.variant());
        }

        setxkbmapCommandArguments.append(QStringLiteral("-layout"));
        setxkbmapCommandArguments.append(layouts.join(COMMAND_OPTIONS_SEPARATOR));
        if (!variants.join(QString()).isEmpty()) {
            setxkbmapCommandArguments.append(QStringLiteral("-variant"));
            setxkbmapCommandArguments.append(variants.join(COMMAND_OPTIONS_SEPARATOR));
        }
    }
    if (config.keyboardSettings()->resetOldXkbOptions()) {
        // Pass -option "" to clear previously set options
        setxkbmapCommandArguments.append(QStringLiteral("-option"));
        setxkbmapCommandArguments.append(QStringLiteral(""));
    }
    const QStringList xkbOpts = config.keyboardSettings()->xkbOptions();
    for (const auto &option : xkbOpts) {
        setxkbmapCommandArguments.append(QStringLiteral("-option"));
        setxkbmapCommandArguments.append(option);
    }

    if (!setxkbmapCommandArguments.isEmpty()) {
        if (config.keyboardSettings()->configureLayouts()) {
            X11Helper::setDefaultLayout();
        }

        return runConfigLayoutCommand(setxkbmapCommandArguments);
    }
    return false;
}
