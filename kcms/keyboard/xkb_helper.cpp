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
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QX11Info>

#include <KProcess>

#include "keyboard_config.h"

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
        return QLatin1String("");

    if (setxkbmapExe.isEmpty()) {
        setxkbmapExe = QStandardPaths::findExecutable(SETXKBMAP_EXEC);
        if (setxkbmapExe.isEmpty()) {
            setxkbmapNotFound = true;
            qCCritical(KCM_KEYBOARD) << "Can't find" << SETXKBMAP_EXEC << "- keyboard layouts won't be configured";
            return QLatin1String("");
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

        KProcess xmodmapProcess;
        xmodmapProcess << xmodmapExe;
        xmodmapProcess << configFileName;
        qCDebug(KCM_KEYBOARD) << "Executing" << xmodmapProcess.program().join(QLatin1Char(' '));
        if (xmodmapProcess.execute() != 0) {
            qCCritical(KCM_KEYBOARD) << "Failed to execute " << xmodmapProcess.program();
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

    KProcess setxkbmapProcess;
    setxkbmapProcess << getSetxkbmapExe() << setxkbmapCommandArguments;
    int res = setxkbmapProcess.execute();

    if (res == 0) { // restore Xmodmap mapping reset by setxkbmap
        qCDebug(KCM_KEYBOARD) << "Executed successfully in " << timer.elapsed() << "ms" << setxkbmapProcess.program().join(QLatin1Char(' '));
        restoreXmodmap();
        qCDebug(KCM_KEYBOARD) << "\t and with xmodmap" << timer.elapsed() << "ms";
        return true;
    } else {
        qCCritical(KCM_KEYBOARD) << "Failed to run" << setxkbmapProcess.program().join(QLatin1Char(' ')) << "return code:" << res;
    }
    return false;
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
    if (!variants.join(QLatin1String("")).isEmpty()) {
        setxkbmapCommandArguments.append(QStringLiteral("-variant"));
        setxkbmapCommandArguments.append(variants.join(COMMAND_OPTIONS_SEPARATOR));
    }

    return runConfigLayoutCommand(setxkbmapCommandArguments);
}

bool XkbHelper::initializeKeyboardLayouts(KeyboardConfig &config)
{
    QStringList setxkbmapCommandArguments;
    if (!config.keyboardModel.isEmpty()) {
        XkbConfig xkbConfig;
        X11Helper::getGroupNames(QX11Info::display(), &xkbConfig, X11Helper::MODEL_ONLY);
        if (xkbConfig.keyboardModel != config.keyboardModel) {
            setxkbmapCommandArguments.append(QStringLiteral("-model"));
            setxkbmapCommandArguments.append(config.keyboardModel);
        }
    }
    if (config.configureLayouts) {
        QStringList layouts;
        QStringList variants;
        const QList<LayoutUnit> defaultLayouts = config.getDefaultLayouts();
        for (const auto &layoutUnit : defaultLayouts) {
            layouts.append(layoutUnit.layout());
            variants.append(layoutUnit.variant());
        }

        setxkbmapCommandArguments.append(QStringLiteral("-layout"));
        setxkbmapCommandArguments.append(layouts.join(COMMAND_OPTIONS_SEPARATOR));
        if (!variants.join(QLatin1String("")).isEmpty()) {
            setxkbmapCommandArguments.append(QStringLiteral("-variant"));
            setxkbmapCommandArguments.append(variants.join(COMMAND_OPTIONS_SEPARATOR));
        }
    }
    if (config.resetOldXkbOptions) {
        setxkbmapCommandArguments.append(QStringLiteral("-option"));
    }
    if (!config.xkbOptions.isEmpty()) {
        setxkbmapCommandArguments.append(QStringLiteral("-option"));
        setxkbmapCommandArguments.append(config.xkbOptions.join(COMMAND_OPTIONS_SEPARATOR));
    }

    if (!setxkbmapCommandArguments.isEmpty()) {
        return runConfigLayoutCommand(setxkbmapCommandArguments);
        if (config.configureLayouts) {
            X11Helper::setDefaultLayout();
        }
    }
    return false;
}
