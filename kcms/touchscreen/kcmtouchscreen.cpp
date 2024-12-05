/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmtouchscreen.h"
#include "inputdevice.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QGuiApplication>
#include <QScreen>
#include <QStandardItemModel>

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(Touchscreen, "kcm_touchscreen.json")

class OutputsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    OutputsModel()
    {
        setItemRoleNames({
            {Qt::DisplayRole, "display"},
            {Qt::UserRole, "name"},
        });

        reset();

        connect(qGuiApp, &QGuiApplication::screenAdded, this, &OutputsModel::reset);
        connect(qGuiApp, &QGuiApplication::screenRemoved, this, &OutputsModel::reset);
    }

    void reset()
    {
        clear();

        auto screens = qGuiApp->screens();
        auto it = new QStandardItem(i18n("Automatic"));
        appendRow(it);

        for (auto screen : screens) {
            auto geo = screen->geometry();
            auto it = new QStandardItem(i18nc("model - (x,y widthxheight)",
                                              "%1 - (%2,%3 %4×%5)",
                                              screen->model(),
                                              QString::number(geo.x()),
                                              QString::number(geo.y()),
                                              QString::number(geo.width()),
                                              QString::number(geo.height())));
            it->setData(screen->name(), Qt::UserRole);
            appendRow(it);
        }
    }

    Q_SCRIPTABLE QString outputNameAt(int row) const
    {
        return item(row)->data(Qt::UserRole).toString();
    }
    Q_SCRIPTABLE int rowForOutputName(const QString &outputName)
    {
        for (int i = 0, c = rowCount(); i < c; ++i) {
            if (item(i)->data(Qt::UserRole) == outputName) {
                return i;
            }
        }

        return 0;
    }
};

Touchscreen::Touchscreen(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_touchscreensModel(new DevicesModel("touch", this))
{
    const char *uri = "org.kde.plasma.touchscreen.kcm";
    qmlRegisterType<OutputsModel>(uri, 1, 0, "OutputsModel");
    qmlRegisterUncreatableType<InputDevice>(uri, 1, 0, "InputDevice", u"Should be fetched from kcm.touchscreensModel"_s);

    connect(m_touchscreensModel, &DevicesModel::needsSaveChanged, this, &Touchscreen::refreshNeedsSave);
}

Touchscreen::~Touchscreen() = default;

void Touchscreen::refreshNeedsSave()
{
    setNeedsSave(isSaveNeeded());
}

bool Touchscreen::isSaveNeeded() const
{
    return m_touchscreensModel->isSaveNeeded();
}

bool Touchscreen::isDefaults() const
{
    return m_touchscreensModel->isDefaults();
}

void Touchscreen::load()
{
    m_touchscreensModel->load();
}

void Touchscreen::save()
{
    m_touchscreensModel->save();
}

void Touchscreen::defaults()
{
    m_touchscreensModel->defaults();
}

DevicesModel *Touchscreen::touchscreensModel() const
{
    return m_touchscreensModel;
}

#include "kcmtouchscreen.moc"

#include "moc_kcmtouchscreen.cpp"
