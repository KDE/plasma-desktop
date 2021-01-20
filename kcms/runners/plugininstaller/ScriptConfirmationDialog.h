/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalizedString>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

class ScriptConfirmationDialog : public QDialog
{
public:
    ScriptConfirmationDialog(const QString &installerPath, bool install, const QString &dir, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        const auto readmes = QDir(dir).entryList({QStringLiteral("README*")});
        setWindowTitle(i18nc("@title:window", "Confirm Installation"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        const bool noInstaller = installerPath.isEmpty();
        QVBoxLayout *layout = new QVBoxLayout(this);
        QString msg;
        if (!install && noInstaller && readmes.isEmpty()) {
            msg = xi18nc("@info",
                         "This plugin does not provide an uninstallation script. Please contact the author. "
                         "You can try to uninstall the plugin manually.<nl/>"
                         "If you do not feel capable or comfortable with this, click <interface>Cancel</interface>  now.");
        } else if (!install && noInstaller) {
            msg = xi18nc("@info",
                         "This plugin does not provide an uninstallation script. Please contact the author. "
                         "You can try to uninstall the plugin manually. Please have a look at the README "
                         "for instructions from the author.<nl/>"
                         "If you do not feel capable or comfortable with this, click <interface>Cancel</interface>  now.");
        } else if (noInstaller && readmes.isEmpty()) {
            msg = xi18nc("@info",
                         "This plugin does not provide an installation script. Please contact the author. "
                         "You can try to install the plugin manually.<nl/>"
                         "If you do not feel capable or comfortable with this, click <interface>Cancel</interface>  now.");
        } else if (noInstaller) {
            msg = xi18nc("@info",
                         "This plugin does not provide an installation script. Please contact the author. "
                         "You can try to install the plugin manually. Please have a look at the README "
                         "for instructions from the author.<nl/>"
                         "If you do not feel capable or comfortable with this, click <interface>Cancel</interface>  now.");
        } else if (readmes.isEmpty()) {
            msg = xi18nc("@info",
                         "This plugin uses a script for installation which can pose a security risk. "
                         "Please examine the entire plugin's contents before installing, or at least "
                         "read the script's source code.<nl/>"
                         "If you do not feel capable or comfortable with this, click <interface>Cancel</interface>  now.");
        } else {
            msg = xi18nc("@info",
                         "This plugin uses a script for installation which can pose a security risk. "
                         "Please examine the entire plugin's contents before installing, or at least "
                         "read the README file and the script's source code.<nl/>"
                         "If you do not feel capable or comfortable with this, click <interface>Cancel</interface>  now.");
        }
        QLabel *msgLabel = new QLabel(msg, this);
        msgLabel->setWordWrap(true);
        layout->addWidget(msgLabel);
        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("emblem-warning"));
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        QString okText;
        if (noInstaller && !install) {
            okText = i18nc("@action:button", "Mark entry as uninstalled");
        } else if (noInstaller) {
            okText = i18nc("@action:button", "Mark entry as installed");
        } else {
            okText = i18nc("@action:button", "Accept Risk And Continue");
        }
        buttonBox->button(QDialogButtonBox::Ok)->setText(okText);

        QHBoxLayout *helpButtonLayout = new QHBoxLayout(this);
        if (!noInstaller) {
            QPushButton *scriptButton = new QPushButton(QIcon::fromTheme("dialog-scripts"), i18nc("@action:button", "View Script"), this);
            connect(scriptButton, &QPushButton::clicked, this, [installerPath]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(installerPath));
            });
            helpButtonLayout->addWidget(scriptButton);
        }
        QPushButton *sourceButton = new QPushButton(QIcon::fromTheme("document-open-folder"), i18nc("@action:button", "View Source Directory"), this);
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
            QPushButton *readmeButton = new QPushButton(QIcon::fromTheme("text-x-readme"), i18nc("@action:button", "View %1", readmes.at(0)), this);
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
