/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalizedString>
#include <KMessageBox>
#include <KOSRelease>
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QUrl>

#include "KRunnerPlugininstallerRcJob.h"
#include "config-workspace.h"
#if HAVE_PACKAGEKIT
#include "PackageKitJob.h"
#endif
#include "ScriptJob.h"
#include "ZypperRPMJob.h"

void fail(const QString &str)
{
    if (str.isEmpty()) {
        // 130 means Ctrl+C as an exit code this is interpreted by KNewStuff as cancel operation
        // and no error will be displayed to the user, BUG: 436355
        qApp->exit(130);
    } else {
        KMessageBox::error(nullptr, str, i18nc("@info", "KRunner plugin installation failed"));
        qApp->exit(1);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    if (qEnvironmentVariableIsSet("PLUGININSTALLER_TEST_MODE")) {
        QStandardPaths::setTestModeEnabled(true);
    }

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("command"), i18nc("@info:shell", "Command to execute: install or uninstall"));
    parser.addPositionalArgument(QStringLiteral("path"), i18nc("@info:shell", "Path to archive or extracted folder"));
    QCommandLineOption noConfirm(QStringLiteral("no-confirm"), i18nc("@info:shell", "Do not show a visual confirmation dialog"));
    parser.addOption(noConfirm);
    parser.addHelpOption();
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qWarning() << "Command is required";
        return 1;
    }
    if (args.size() == 1) {
        qWarning() << "Path to archive/extracted folder is required";
        return 1;
    }

    const QString cmd = args.at(0);
    const QString file = args.at(1);
    const QStringList binaryPackages = {
        QStringLiteral("application/vnd.debian.binary-package"),
        QStringLiteral("application/x-rpm"),
        QStringLiteral("application/x-xz"),
        QStringLiteral("application/zstd"),
    };
    bool install;
    if (cmd == QLatin1String("install")) {
        install = true;
    } else if (cmd == QLatin1String("uninstall")) {
        install = false;
    } else {
        qWarning() << "Unsupported command" << cmd;
        return 1;
    }

    std::unique_ptr<AbstractJob> job;
    QFileInfo fileInfo(file);
    const QString mimeType = QMimeDatabase().mimeTypeForFile(fileInfo).name();
    if (mimeType == QLatin1String("application/x-rpm") && KOSRelease().idLike().contains(u"suse")) {
        job.reset(new ZypperRPMJob());
    } else if (binaryPackages.contains(mimeType)) {
#if HAVE_PACKAGEKIT
        job.reset(new PackageKitJob());
#else
        fail(i18nc("@info", "No PackageKit support"));
#endif
    } else if (const QString rcPath = QDir(file).filePath("krunner-plugininstallerrc"); fileInfo.isDir() && QFileInfo::exists(rcPath)) {
        fileInfo = QFileInfo(rcPath);
        job.reset(new KRunnerPluginInstallerRcJob(parser.isSet(noConfirm)));
    } else {
        job.reset(new ScriptJob());
    }

    QObject::connect(
        job.get(),
        &AbstractJob::finished,
        qApp,
        []() {
            qApp->exit();
        },
        Qt::QueuedConnection);
    QObject::connect(
        job.get(),
        &AbstractJob::error,
        qApp,
        [](const QString &error) {
            fail(error);
        },
        Qt::QueuedConnection);

    job->executeOperation(fileInfo, mimeType, install);

    return app.exec();
}
