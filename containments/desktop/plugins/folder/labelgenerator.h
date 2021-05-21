/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LABELGENERATOR_H
#define LABELGENERATOR_H

#include <QObject>
#include <QPointer>

class KFilePlacesModel;
class FolderModel;

class LabelGenerator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(FolderModel *folderModel READ folderModel WRITE setFolderModel NOTIFY folderModelChanged)
    Q_PROPERTY(bool rtl READ rtl WRITE setRtl NOTIFY rtlChanged)
    Q_PROPERTY(int labelMode READ labelMode WRITE setLabelMode NOTIFY labelModeChanged)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText NOTIFY labelTextChanged)
    Q_PROPERTY(QString displayLabel READ displayLabel NOTIFY displayLabelChanged)

public:
    explicit LabelGenerator(QObject *parent = nullptr);
    ~LabelGenerator() override;

    FolderModel *folderModel() const;
    void setFolderModel(FolderModel *folderModel);

    bool rtl() const;
    void setRtl(bool rtl);

    int labelMode() const;
    void setLabelMode(int mode);

    QString labelText() const;
    void setLabelText(const QString &text);

    QString displayLabel() const;

Q_SIGNALS:
    void folderModelChanged();
    void rtlChanged();
    void labelModeChanged();
    void labelTextChanged();
    void displayLabelChanged();

private:
    void updateDisplayLabel();
    QString generatedDisplayLabel();

    static int s_instanceCount;
    static KFilePlacesModel *s_placesModel;
    bool m_placesConnected = false;

    void connectPlacesModel();

    QPointer<FolderModel> m_folderModel;
    bool m_rtl;
    int m_labelMode;
    QString m_displayLabel;
    QString m_labelText;
};

#endif
