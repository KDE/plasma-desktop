/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ITEMVIEWADAPTER_H
#define ITEMVIEWADAPTER_H

#include <QRect>

#include <KAbstractViewAdapter>

class ItemViewAdapter : public KAbstractViewAdapter
{
    Q_OBJECT

    Q_PROPERTY(QObject *adapterView READ adapterView WRITE setAdapterView NOTIFY adapterViewChanged)
    Q_PROPERTY(QAbstractItemModel *adapterModel READ adapterModel WRITE setAdapterModel NOTIFY adapterModelChanged)
    Q_PROPERTY(int adapterIconSize READ adapterIconSize WRITE setAdapterIconSize NOTIFY adapterIconSizeChanged)
    Q_PROPERTY(QRect adapterVisibleArea READ adapterVisibleArea WRITE setAdapterVisibleArea NOTIFY adapterVisibleAreaChanged)

public:
    explicit ItemViewAdapter(QObject *parent = nullptr);

    QAbstractItemModel *model() const override;
    QSize iconSize() const override;
    QPalette palette() const override;
    QRect visibleArea() const override;
    QRect visualRect(const QModelIndex &index) const override;
    void connect(Signal signal, QObject *receiver, const char *slot) override;

    QObject *adapterView() const;
    void setAdapterView(QObject *view);

    QAbstractItemModel *adapterModel() const;
    void setAdapterModel(QAbstractItemModel *model);

    int adapterIconSize() const;
    void setAdapterIconSize(int size);

    QRect adapterVisibleArea() const;
    void setAdapterVisibleArea(QRect rect);

Q_SIGNALS:
    void viewScrolled() const;
    void adapterViewChanged() const;
    void adapterModelChanged() const;
    void adapterIconSizeChanged() const;
    void adapterVisibleAreaChanged() const;

private:
    QObject *m_adapterView;
    QAbstractItemModel *m_adapterModel;
    int m_adapterIconSize;
    QRect m_adapterVisibleArea;
};

#endif
