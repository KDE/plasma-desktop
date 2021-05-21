/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "folderplugin.h"
#include "directorypicker.h"
#include "foldermodel.h"
#include "itemviewadapter.h"
#include "labelgenerator.h"
#include "menuhelper.h"
#include "mimetypesmodel.h"
#include "placesmodel.h"
#include "positioner.h"
#include "previewpluginsmodel.h"
#include "rubberband.h"
#include "shortcut.h"
#include "subdialog.h"
#include "viewpropertiesmenu.h"
#include "wheelinterceptor.h"

#include <QQmlContext>
#include <QQmlEngine>

static QObject *menuHelperSingletonProvider(QQmlEngine *engine, QJSEngine *jsEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(jsEngine);
    return new MenuHelper();
}

void FolderPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.private.desktopcontainment.folder"));
    qmlRegisterType<DirectoryPicker>(uri, 0, 1, "DirectoryPicker");
    qmlRegisterType<FolderModel>(uri, 0, 1, "FolderModel");
    qmlRegisterType<ItemViewAdapter>(uri, 0, 1, "ItemViewAdapter");
    qmlRegisterType<LabelGenerator>(uri, 0, 1, "LabelGenerator");
    qmlRegisterSingletonType<MenuHelper>(uri, 0, 1, "MenuHelper", menuHelperSingletonProvider);
    qmlRegisterType<MimeTypesModel>(uri, 0, 1, "MimeTypesModel");
    qmlRegisterType<PlacesModel>(uri, 0, 1, "PlacesModel");
    qmlRegisterType<Positioner>(uri, 0, 1, "Positioner");
    qmlRegisterType<PreviewPluginsModel>(uri, 0, 1, "PreviewPluginsModel");
    qmlRegisterType<RubberBand>(uri, 0, 1, "RubberBand");
    qmlRegisterType<SubDialog>(uri, 0, 1, "SubDialog");
    qmlRegisterType<ViewPropertiesMenu>(uri, 0, 1, "ViewPropertiesMenu");
    qmlRegisterType<WheelInterceptor>(uri, 0, 1, "WheelInterceptor");
    qmlRegisterType<ShortCut>(uri, 0, 1, "ShortCut");
}
