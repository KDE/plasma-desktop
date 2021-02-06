/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <KIO/OpenFileManagerWindowJob>
#include <KLocalizedString>

class PackageKitConfirmationDialog : public QDialog
{
public:
    PackageKitConfirmationDialog(const QString &packagePath, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(i18nc("@title:window", "Confirm Installation"));
        setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        QVBoxLayout *layout = new QVBoxLayout(this);
        QString msg = xi18nc("@info", "You are about to install a binary package. You should only install these from a trusted author/packager.");
        QLabel *msgLabel = new QLabel(msg, this);
        msgLabel->setWordWrap(true);
        layout->addWidget(msgLabel);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("emblem-warning"));
        buttonBox->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button", "Accept Risk And Continue"));
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        QPushButton *highlightFileButton = new QPushButton(QIcon::fromTheme("document-open-folder"), i18nc("@action:button", "View File"), this);
        connect(highlightFileButton, &QPushButton::clicked, this, [packagePath]() {
            KIO::highlightInFileManager({QUrl::fromLocalFile(packagePath)});
        });
        buttonBox->addButton(highlightFileButton, QDialogButtonBox::HelpRole);
        buttonBox->button(QDialogButtonBox::Cancel)->setFocus();
        layout->addWidget(buttonBox);
    }
};
