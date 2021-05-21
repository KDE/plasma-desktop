/*
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KNETATTACH_H
#define KNETATTACH_H

#include "ui_knetattach.h"

class QUrl;

class KNetAttach : public QWizard, private Ui_KNetAttach
{
    Q_OBJECT

public:
    explicit KNetAttach(QWidget *parent = nullptr);

public Q_SLOTS:
    virtual void setInformationText(const QString &type);

private:
    QString _type;

    bool doConnectionTest(const QUrl &url);
    bool updateForProtocol(const QString &protocol);

private Q_SLOTS:
    void updateParametersPageStatus();
    bool validateCurrentPage() override;
    void updatePort(bool encryption);
    void updateFinishButtonText(bool save);
    void slotHelpClicked();
    void slotPageChanged(int);
};

#endif
