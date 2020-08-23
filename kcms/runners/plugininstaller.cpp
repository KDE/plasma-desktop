/***************************************************************************
 *   Copyright © 2020 Alexander Lohnau <alexander.lohnau@gmx.de>           *
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
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDesktopServices>
#include <KLocalizedString>
#include <KShell>
#include <KLocalizedString>
#include <KOSRelease>
#include <KIO/OpenFileManagerWindowJob>
#include <KMessageBox>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

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
#endif

void fail(const QString &str)
{
    qCritical() << str;
    // This prevents the fallback error message to be shown when it is not required
    static bool failed = false;
    if (failed) {
        return;
    }
    failed = true;
    KMessageBox::error(nullptr, str, i18n("KRunner plugin installation failed"));
    exit(1);
}

void aboartInstallation()
{
    qWarning() << i18n("Installation aborted");
    exit(1);
}

inline QString getCloseMessage(Operation operation)
{
    return operation == Operation::Install
                    ? i18n("Installation executed successfully, you may now close this window")
                    : i18n("Uninstallation executed successfully, you may now close this window");
}

void runScriptInTerminal(const QString &script, const QString &pwd)
{
    KConfigGroup confGroup(KSharedConfig::openConfig(), "General");
    QString exec = confGroup.readPathEntry("TerminalApplication", QStringLiteral("konsole"));

    if (exec == QLatin1String("konsole")) {
        exec += QLatin1String(" --noclose");
    } else if (exec == QLatin1String("xterm")) {
        exec += QLatin1String(" -hold");
    }
    exec += QLatin1String(" -e ");
    exec += KShell::quoteArg(script);

    QProcess process;
    process.setWorkingDirectory(pwd);
    // We don't know if the entry read from the config contains options
    // so we just split it at the end
    QStringList split = KShell::splitArgs(exec);
    process.setProgram(split.takeFirst());
    process.setArguments(split);
    process.start();
    process.waitForFinished(-1);
}

class ScriptConfirmationDialog : public QDialog
{
public:
    ScriptConfirmationDialog(const QString &installerPath, Operation operation, const QString &dir, QWidget *parent = nullptr) : QDialog(parent)
    {
        const auto readmes = QDir(dir).entryList({QStringLiteral("README*")});
        setWindowTitle(i18n("Confirm Installation"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        const bool noInstaller = installerPath.isEmpty();
        QVBoxLayout *layout = new QVBoxLayout(this);
        QString msg;
        if (operation == Operation::Uninstall && noInstaller && readmes.isEmpty()) {
            msg = xi18nc("@info", "This plugin does not provide an uninstall script. Please contact the author. "
                                  "You can try to uninstall the plugin manually.<nl/>"
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (operation == Operation::Uninstall && noInstaller) {
            msg = xi18nc("@info", "This plugin does not provide an uninstallation script. Please contact the author. "
                                  "You can try to uninstall the plugin manually. Please have a look at the README "
                                  "for instructions from the author.<nl/>"
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (noInstaller && readmes.isEmpty()) {
            msg = xi18nc("@info", "This plugin does not provide an installation script. Please contact the author. "
                                  "You can try to install the plugin manually.<nl/>"
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (noInstaller) {
            msg = xi18nc("@info", "This plugin does not provide an installation script. Please contact the author.<nl/>"
                                  "You can try to install the plugin manually; please have a look at the README "
                                  "for instructions from the author.<nl/>"
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (readmes.isEmpty()) {
            msg = xi18nc("@info", "This plugin uses a script for installation which can pose a security risk. "
                           "Please examine the entire plugin's contents before installing, or at least "
                           "read the script's source code.<nl/>"
                           "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else {
             msg = xi18nc("@info", "This plugin uses a script for installation which can pose a security risk. "
                           "Please examine the entire plugin's contents before installing, or at least "
                           "read the README file and the script's source code.<nl/>"
                           "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        }
        QLabel *msgLabel = new QLabel(msg, this);
        msgLabel->setWordWrap(true);
        msgLabel->setMaximumWidth(500);
        layout->addWidget(msgLabel);
        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("emblem-warning"));
        QString okText;
        if (noInstaller && operation == Operation::Uninstall) {
            okText = i18n("Mark entry as uninstalled");
        } else if (noInstaller) {
            okText = i18n("Mark entry as installed");
        } else {
           okText = i18n("Accept Risk And Continue");
        }
        buttonBox->button(QDialogButtonBox::Ok)->setText(okText);
        // If the user clicks cancel or closes the dialog using escape
        connect(buttonBox, &QDialogButtonBox::rejected, this, aboartInstallation);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this, noInstaller](){
            if (noInstaller) {
                exit(0);
            } else {
                done(1);
            }
        });
        connect(this, &QDialog::rejected, this, aboartInstallation);

        QHBoxLayout *helpButtonLayout = new QHBoxLayout(this);
        if (!noInstaller) {
            QPushButton *scriptButton = new QPushButton(QIcon::fromTheme("dialog-scripts"), i18n("View Script"), this);
            connect(scriptButton, &QPushButton::clicked, this, [installerPath]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(installerPath));
            });
            helpButtonLayout->addWidget(scriptButton);
        }
        QPushButton *sourceButton = new QPushButton(QIcon::fromTheme("document-open-folder"), i18n("View Source Directory"), this);
        connect(sourceButton, &QPushButton::clicked, this, [dir]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
        });
        if (readmes.isEmpty() && helpButtonLayout->isEmpty()) {
            // If there is no script and readme we can display the button in the same line
            buttonBox->addButton(sourceButton, QDialogButtonBox::HelpRole);
        } else {
            helpButtonLayout->addWidget(sourceButton);
        }
        if (!readmes.isEmpty()) {
            QPushButton *readmeButton = new QPushButton(QIcon::fromTheme("text-x-readme"), i18n("View %1", readmes.at(0)), this);
            connect(readmeButton, &QPushButton::clicked, this, [dir, readmes]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(dir).absoluteFilePath(readmes.at(0))));
            });
            helpButtonLayout->addWidget(readmeButton);
        }
        helpButtonLayout->setAlignment(Qt::AlignRight);
        layout->addLayout(helpButtonLayout);
        buttonBox->button(QDialogButtonBox::Cancel)->setFocus();
        layout->addWidget(buttonBox);
    }
};


#ifdef HAVE_PACKAGEKIT
class PackagekitConfirmationDialog : public QDialog {
public:
    PackagekitConfirmationDialog(const QString &packagePath, QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle(i18n("Confirm Installation"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        QVBoxLayout *layout = new QVBoxLayout(this);
        QString msg = xi18nc("@info", "You are about to install a binary package. You should only install these from a trusted author/packager.");
        QLabel *msgLabel = new QLabel(msg, this);
        msgLabel->setWordWrap(true);
        msgLabel->setMaximumWidth(500);
        layout->addWidget(msgLabel);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("emblem-warning"));
        buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Accept Risk And Continue"));
        // If the user clicks cancel or closes the dialog using escape
        connect(buttonBox, &QDialogButtonBox::rejected, this, aboartInstallation);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this, packagePath](){
            if (QMimeDatabase().mimeTypeForFile(QFileInfo(packagePath)).name() == QLatin1String("application/x-rpm")
                    && KOSRelease().name().contains(QStringLiteral("openSUSE"), Qt::CaseInsensitive)) {
                const QString command = QStringLiteral("sudo zypper install %1").arg(KShell::quoteArg(packagePath));
                runScriptInTerminal(QStringLiteral("bash -c \"echo %1;%1 && echo %2\"")
                        .arg(command, KShell::quoteArg(getCloseMessage(Operation::Install))), QFileInfo(packagePath).absolutePath());
                exit(0);
            } else {
                done(1);
            }
        });
        connect(this, &QDialog::rejected, this, aboartInstallation);

        QPushButton *highlightFileButton = new QPushButton(QIcon::fromTheme("document-open-folder"), i18n("View File"), this);
        connect(highlightFileButton, &QPushButton::clicked, this, [packagePath]() {
            KIO::highlightInFileManager({QUrl::fromLocalFile(packagePath)});
        });
        buttonBox->addButton(highlightFileButton, QDialogButtonBox::HelpRole);
        buttonBox->button(QDialogButtonBox::Cancel)->setFocus();
        layout->addWidget(buttonBox);
    }
};

void exitWithError(PackageKit::Transaction::Error, const QString &details)
{
    fail(details);
}

void packageKitInstall(const QString &fileName)
{
    PackageKit::Transaction *transaction = PackageKit::Daemon::installFile(fileName);
    QObject::connect(transaction, &PackageKit::Transaction::finished,
                     [=](PackageKit::Transaction::Exit status, uint) {
                        if (status == PackageKit::Transaction::ExitSuccess) {
                            exit(0);
                        }
                        // Sometimes packagekit gets stuck when installing an unsupported package this way we
                        // ensure that we exit. The errorCode slot could provide a better message, that is why we wait
                        QTimer::singleShot(1000, [=](){
                            fail(i18n("Failed to install \"%1\"; exited with status \"%2\"",
                                      fileName, QVariant::fromValue(status).toString()));
                        });
                    });
    QObject::connect(transaction, &PackageKit::Transaction::errorCode, exitWithError);
}

void packageKitUninstall(const QString &fileName)
{
    PackageKit::Transaction *transaction = PackageKit::Daemon::getDetailsLocal(fileName);
    QObject::connect(transaction, &PackageKit::Transaction::details,
                     [=](const PackageKit::Details &details) {
                         PackageKit::Transaction *transaction = PackageKit::Daemon::removePackage(details.packageId());
                         QObject::connect(transaction, &PackageKit::Transaction::finished,
                             [=](PackageKit::Transaction::Exit status, uint) {
                                  if (status == PackageKit::Transaction::ExitSuccess) {
                                         exit(0);
                                  }
                         });
                         QObject::connect(transaction, &PackageKit::Transaction::errorCode, exitWithError);
                     });

    QObject::connect(transaction, &PackageKit::Transaction::errorCode, exitWithError);
    // Fallback error handling
    QObject::connect(transaction, &PackageKit::Transaction::finished,
        [=](PackageKit::Transaction::Exit status, uint) {
            if (status != PackageKit::Transaction::ExitSuccess) {
                QTimer::singleShot(1000, [=]() {
                    fail(i18n("Failed to uninstall \"%1\"; exited with status \"%2\"",
                              fileName, QVariant::fromValue(status).toString()));
                });
            }
        });
    }
#endif

void packageKit(Operation operation, const QString &fileName)
{
#ifdef HAVE_PACKAGEKIT
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
        fail(i18n("The file does not exist!"));
    }
    const QString absPath = fileInfo.absoluteFilePath();
    if (operation == Operation::Install) {
        PackagekitConfirmationDialog(fileName).exec();
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

void executeOperation(const QString &archive, Operation operation)
{
    if (binaryPackages.contains(QMimeDatabase().mimeTypeForFile(archive).name())) {
        packageKit(operation, archive);
    }

    const bool install = operation == Operation::Install;
    QString installerPath;
    const QFileInfoList archiveEntries = QDir(archive).entryInfoList(QDir::Files, QDir::Name);
    const QString scriptPrefix = install ? "install" : "uninstall";
    for (const auto &file : archiveEntries) {
        if (file.baseName() == scriptPrefix) {
            installerPath = file.absoluteFilePath();
            // If the name is exactly install/uninstall we immediately take it
            break;
        } else if (file.baseName().startsWith(scriptPrefix)) {
            installerPath = file.absoluteFilePath();
        }
    }
    // We want the user to be exactly aware of whats going on
    if (install || installerPath.isEmpty()) {
        ScriptConfirmationDialog dlg(installerPath, operation, archive);
        dlg.exec();
    }

    const QString bashCommand = QStringLiteral("echo %1;%1 || $SHELL && echo %2")
            .arg(KShell::quoteArg(installerPath), KShell::quoteArg(getCloseMessage(operation)));
    runScriptInTerminal(QStringLiteral("bash -c %1").arg(KShell::quoteArg(bashCommand)), archive);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

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
