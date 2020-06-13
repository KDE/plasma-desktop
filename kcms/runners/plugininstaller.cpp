/***************************************************************************
 *   Copyright © 2020 Alexander Lohnau <>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QCommandLineParser>
#include <QMimeDatabase>
#include <QUrl>
#include <QGuiApplication>
#include <KLocalizedString>
#include <KShell>

#include <config-workspace.h>

const static QStringList binaryPackages = {QStringLiteral("application/vnd.debian.binary-package"),
                                     QStringLiteral("application/x-rpm"),
                                     QStringLiteral("application/x-xz"),
                                     QStringLiteral("application/zstd")};

enum class Operation {
    Install,
    Uninstall
};

#ifdef HAVE_PACKAGEKIT
#include <PackageKit/Daemon>
#include <PackageKit/Details>
#include <PackageKit/Transaction>
#else
#include <QDesktopServices>
#endif

Q_NORETURN void fail(const QString &str)
{
    qCritical() << str;
    const QStringList args = {"--detailederror" ,i18n("Dolphin service menu installation failed"),  str};
    QProcess::startDetached("kdialog", args);

    exit(1);
}

#ifdef HAVE_PACKAGEKIT
void packageKitInstall(const QString &fileName)
{
    PackageKit::Transaction *transaction = PackageKit::Daemon::installFile(fileName);

    const auto exitWithError = [=](PackageKit::Transaction::Error, const QString &details) {
       fail(details);
    };

    QObject::connect(transaction, &PackageKit::Transaction::finished,
                     [=](PackageKit::Transaction::Exit status, uint) {
                        if (status == PackageKit::Transaction::ExitSuccess) {
                            exit(0);
                        }
                        // Fallback error handling
                        QTimer::singleShot(500, [=](){
                            fail(i18n("Failed to install \"%1\", exited with status \"%2\"",
                                      fileName, QVariant::fromValue(status).toString()));
                        });
                    });
    QObject::connect(transaction, &PackageKit::Transaction::errorCode, exitWithError);
}

void packageKitUninstall(const QString &fileName)
{
    const auto exitWithError = [=](PackageKit::Transaction::Error, const QString &details) {
        fail(details);
    };
    const auto uninstallLambda = [=](PackageKit::Transaction::Exit status, uint) {
        if (status == PackageKit::Transaction::ExitSuccess) {
            exit(0);
        }
    };

    PackageKit::Transaction *transaction = PackageKit::Daemon::getDetailsLocal(fileName);
    QObject::connect(transaction, &PackageKit::Transaction::details,
                     [=](const PackageKit::Details &details) {
                         PackageKit::Transaction *transaction = PackageKit::Daemon::removePackage(details.packageId());
                         QObject::connect(transaction, &PackageKit::Transaction::finished, uninstallLambda);
                         QObject::connect(transaction, &PackageKit::Transaction::errorCode, exitWithError);
                     });

    QObject::connect(transaction, &PackageKit::Transaction::errorCode, exitWithError);
    // Fallback error handling
    QObject::connect(transaction, &PackageKit::Transaction::finished,
        [=](PackageKit::Transaction::Exit status, uint) {
            if (status != PackageKit::Transaction::ExitSuccess) {
                QTimer::singleShot(500, [=]() {
                    fail(i18n("Failed to uninstall \"%1\", exited with status \"%2\"",
                              fileName, QVariant::fromValue(status).toString()));
                });
            }
        });
    }
#endif

Q_NORETURN void packageKit(Operation operation, const QString &fileName)
{
#ifdef HAVE_PACKAGEKIT
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
        fail(i18n("The file does not exist!"));
    }
    const QString absPath = fileInfo.absoluteFilePath();
    if (operation == Operation
::Install) {
        packageKitInstall(absPath);
    } else {
        packageKitUninstall(absPath);
    }
    QGuiApplication::exec(); // For event handling, no return after signals finish
    fail(i18n("Unknown error when installing package"));
#else
    Q_UNUSED(operation)
    QDesktopServices::openUrl(QUrl(fileName));
    exit(0);
#endif
}

QString findRecursive(const QString &dir, const QString &basename)
{
    QDirIterator it(dir, QStringList{basename}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        return QFileInfo(it.next()).canonicalFilePath();
    }

    return QString();
}

bool runScript(const QString &path)
{
    QProcess process;
    process.setWorkingDirectory(QFileInfo(path).absolutePath());

    if (!QStandardPaths::findExecutable("konsole").isEmpty()) {
        const QString bashCommand = KShell::quoteArg(path) + ' ' + "|| $SHELL";
        // If the install script fails a shell opens and the user can fix the problem
        // without an error konsole closes
        process.start("konsole", QStringList() << "-e" << "bash" << "-c" << bashCommand, QIODevice::NotOpen);
    } else {
        process.start(path, QIODevice::NotOpen);
    }
    if (!process.waitForStarted()) {
        fail(i18n("Failed to run installer script %1", path));
    }

    // Wait until installer exits, without timeout
    if (!process.waitForFinished(-1)) {
        qWarning() << "Failed to wait on installer:" << process.program() << process.arguments().join(" ");
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qWarning() << "Installer script exited with error:" << process.program() << process.arguments().join(" ");
        return false;
    }

    return true;
}

bool executeOperation(const QString &archive, Operation operation)
{
    if (binaryPackages.contains(QMimeDatabase().mimeTypeForFile(archive).name())) {
        packageKit(Operation::Install, archive);
    }

    const bool install = operation == Operation::Install;
    // Give krunner-install higher priority
    QString installerPath;
    QStringList basenames;
    if (install) { 
        basenames = QStringList({"krunner-install", "install"});
     } else {
        basenames = QStringList({"krunner-uninstall", "uninstall"});
    }
    for (const auto &basename : basenames) {
        const auto path = findRecursive(archive, basename);
        if (!path.isEmpty()) {
            installerPath = path;
            break;
        }
    }

    if (installerPath.isEmpty()) {
        fail(i18n("Failed to find an %1 script in %2", install ? i18n("install") : i18n("uninstall"), archive));
    }

    return runScript(installerPath);
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("command"), i18nc("@info:shell", "Command to execute: install or uninstall."));
    parser.addPositionalArgument(QStringLiteral("path"), i18nc("@info:shell", "Path to archive."));
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        fail(i18n("Command is required."));
    }
    if (args.size() == 1) {
        fail(i18n("Path to archive is required."));
    }

    const QString cmd = args[0];
    const QString archive = args[1];

    if (cmd == QLatin1String("install")) {
        executeOperation(archive, Operation::Install);
    } else if (cmd == QLatin1String("uninstall")) {
        executeOperation(archive, Operation::Uninstall);
    } else {
        fail(i18n("Unsupported command %1", cmd));
    }

    return 0;
}
