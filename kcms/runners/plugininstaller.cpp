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
#include <KToolInvocation>

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

inline bool isSUSEDistro()
{
    return KOSRelease().name().contains(QStringLiteral("openSUSE"), Qt::CaseInsensitive);
}

class ScriptConfirmationDialog : public QDialog
{
public:
    ScriptConfirmationDialog(const QString &installerPath, Operation operation, const QString &dir, QWidget *parent = nullptr) : QDialog(parent)
    {
        const auto readmes = QDir(dir).entryList({QStringLiteral("README*")});
        setWindowTitle(i18n("KRunner Plugin Installer Confirmation Dialog"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        const bool noInstaller = installerPath.isEmpty();
        QVBoxLayout *layout = new QVBoxLayout(this);
        QString msg;
        if (operation == Operation::Uninstall && noInstaller && readmes.isEmpty()) {
            msg = xi18nc("@info", "This plugin does not provide an uninstall script. Please contact the author."
                                  "You can try to uninstall the plugin manually."
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (operation == Operation::Uninstall && noInstaller) {
            msg = xi18nc("@info", "This plugin does not provide an uninstallation script. Please contact the author."
                                  "You can try to uninstall the plugin manually, please have a look at the README"
                                  "for instructions from the author."
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (noInstaller && readmes.isEmpty()) {
            msg = xi18nc("@info", "This plugin does not provide an installation script. Please contact the author."
                                  "You can try to install the plugin manually."
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (noInstaller) {
            msg = xi18nc("@info", "This plugin does not provide an installation script. Please contact the author."
                                  "You can try to install the plugin manually, please have a look at the README"
                                  "for instructions from the author."
                                  "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else if (readmes.isEmpty()) {
            msg = xi18nc("@info", "This plugin uses a script for installation which can pose a security risk."
                           "Please examine the entire plugin's contents before installing, or at least"
                           "read the script's source code."
                           "If you do not feel capable or comfortable with this, click \"Cancel\" now.");
        } else {
             msg = xi18nc("@info", "This plugin uses a script for installation which can pose a security risk."
                           "Please examine the entire plugin's contents before installing, or at least"
                           "read the README file and the script's source code."
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
        const auto rejectLambda = []{
            qWarning() << i18n("Installation aborted");
            exit(1);
        };
        // If the user clicks cancel or closes the dialog using escape
        connect(buttonBox, &QDialogButtonBox::rejected, this, rejectLambda);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this, noInstaller](){
            if (noInstaller) {
                exit(0);
            } else {
                done(1);
            }
        });
        connect(this, &QDialog::rejected, this, rejectLambda);

        QHBoxLayout *helpButtonLayout = new QHBoxLayout(this);
        if (!noInstaller) {
            QPushButton *scriptButton = new QPushButton(QIcon::fromTheme("text-x-script"), i18n("View Script"), this);
            connect(scriptButton, &QPushButton::clicked, this, [installerPath]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(installerPath));
            });
            helpButtonLayout->addWidget(scriptButton);
        }
        QPushButton *sourceButton = new QPushButton(QIcon::fromTheme("inode-directory"), i18n("View Source Directory"), this);
        connect(sourceButton, &QPushButton::clicked, this, [dir]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
        });
        helpButtonLayout->addWidget(sourceButton);
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


class PackagekitConfirmationDialog : public QDialog {
public:
    PackagekitConfirmationDialog(const QString &packagePath, QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle(i18n("KRunner Plugin Installer Confirmation Dialog"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        QVBoxLayout *layout = new QVBoxLayout(this);
        const bool isRPM = packagePath.endsWith(QLatin1String(".rpm"));
        QString msg = xi18nc("@info", "You are about to install a binary package, you should only install these from a trusted "
                                  "author/packager.");
        QLabel *msgLabel = new QLabel(msg, this);
        msgLabel->setWordWrap(true);
        msgLabel->setMaximumWidth(500);
        layout->addWidget(msgLabel);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("emblem-warning"));
        buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Accept Risk And Continue"));
        const auto rejectLambda = []{
            qWarning() << i18n("Installation aborted");
            exit(1);
        };
        // If the user clicks cancel or closes the dialog using escape
        connect(buttonBox, &QDialogButtonBox::rejected, this, rejectLambda);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this, isRPM, packagePath](){
            if (isRPM && isSUSEDistro()) {
                const QString command = QStringLiteral("sudo zyper install %1").arg(KShell::quoteArg(packagePath));
                KToolInvocation::invokeTerminal(QStringLiteral("bash -c \"echo %1;%1\"").arg(command));
                exit(0);
            } else {
                done(1);
            }
        });
        connect(this, &QDialog::rejected, this, rejectLambda);

        QPushButton *highlightFileButton = new QPushButton(QIcon::fromTheme("document-open-folder"), i18n("View File"), this);
        connect(highlightFileButton, &QPushButton::clicked, this, [packagePath]() {
            KIO::highlightInFileManager({QUrl::fromLocalFile(packagePath)});
        });
        buttonBox->addButton(highlightFileButton, QDialogButtonBox::HelpRole);
        buttonBox->button(QDialogButtonBox::Cancel)->setFocus();
        layout->addWidget(buttonBox);
    }
};

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
                        // Fallback error handling, sometimes packagekit gets stuck when installing an unsupported package
                        // this way we ensure that we exit
                        QTimer::singleShot(1000, [=](){
                            fail(i18n("Failed to install \"%1\"; exited with status \"%2\"",
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
    if (operation == Operation
::Install) {
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
        packageKit(operation, archive);
    }

    const bool install = operation == Operation::Install;
    // Give krunner-install higher priority
    QString installerPath;
    const QStringList archiveEntries = QDir(archive).entryList(QDir::Files, QDir::Name);
    const QString scripPrefix = install ? "install" : "uninstall";
    for (const auto &name : archiveEntries) {
        if(name.startsWith(scripPrefix)) {
            installerPath = QDir(archive).filePath(name);
            break;
        }
    }
    // We want the user to be exactly aware of whats going on
    if (install || installerPath.isEmpty()) {
        ScriptConfirmationDialog dlg(installerPath, operation, archive);
        dlg.exec();
    }

    return runScript(installerPath);
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
