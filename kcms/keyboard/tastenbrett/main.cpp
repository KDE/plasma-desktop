/*
    Copyright 2019 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCommandLineParser>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QX11Info>

#include <KAboutData>
#include <KLocalizedString>

#include "application.h"
#include "config-workspace.h"
#include "doodad.h"
#include "geometry.h"

// kind-of copy from xkb_rules.cpp (less complicated)
static QString getRulesName()
{
    XkbRF_VarDefsRec vd;
    char *tmp = nullptr;

    if (XkbRF_GetNamesProp(QX11Info::display(), &tmp, &vd) && tmp != nullptr) {
        const QString name(tmp);
        XFree(tmp);
        return name;
    }

    return QStringLiteral("evdev"); // default to evdev
}

static QString findXkbRulesFile()
{
    const QString rulesName = getRulesName();
    return QStringLiteral("%1/rules/%2").arg(XKBDIR, rulesName);
}

int main(int argc, char *argv[])
{
    setenv("QT_QPA_PLATFORM", "xcb", 1);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    Application app(argc, argv);
    Q_ASSERT(app.platformName() == QStringLiteral("xcb"));

    KAboutData aboutData(QStringLiteral("tastenbrett"),
                         i18nc("app display name", "Keyboard Preview"),
                         QStringLiteral("1.0"),
                         i18nc("app description", "Keyboard layout visualization"),
                         KAboutLicense::GPL);
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    QCommandLineOption modelOption(QStringList { "m", "model" }, {}, QStringLiteral("MODEL"));
    parser.addOption(modelOption);
    QCommandLineOption layoutOption(QStringList { "l", "layout" }, {}, QStringLiteral("LAYOUT"));
    parser.addOption(layoutOption);
    QCommandLineOption variantOption(QStringList { "a", "variant" }, {}, QStringLiteral("VARIANT"));
    parser.addOption(variantOption);
    QCommandLineOption optionsOption(QStringList { "o", "options" }, {}, QStringLiteral("OPTIONS"));
    parser.addOption(optionsOption);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    XkbRF_VarDefsRec varDefs;
    memset(&varDefs, 0, sizeof(XkbRF_VarDefsRec));

    // Models worth testing for obvious mistakes:
    // pc104, tm2020 (fancy), kinesis (fancy)
    QString model = parser.value(modelOption);
    const QString layout = parser.value(layoutOption);
    const QString variant = parser.value(variantOption);
    const QString options = parser.value(optionsOption);

    // Hold these so so we can pass data into xkb getter.
    QByteArray modelArray = model.toUtf8();
    QByteArray layoutArray = layout.toUtf8();
    QByteArray variantArray = variant.toUtf8();
    QByteArray optionsArray = options.toUtf8();

    varDefs.model = modelArray.data();
    varDefs.layout = layoutArray.data();
    varDefs.variant = variantArray.data();
    varDefs.options = optionsArray.data();

    XkbRF_RulesPtr rules = XkbRF_Load(findXkbRulesFile().toUtf8().data(), // needs to be non-const!
                                      qgetenv("LOCALE").data(),
                                      True,
                                      True);
    Q_ASSERT(rules);
    QSharedPointer<XkbRF_RulesRec> rulesCleanup(rules, [](XkbRF_RulesPtr obj) {
        XkbRF_Free(obj, True);
    });

    XkbComponentNamesRec componentNames;
    memset(&componentNames, 0, sizeof(XkbComponentNamesRec));

    XkbRF_GetComponents(rules, &varDefs, &componentNames);

    XkbDescPtr xkb = XkbGetKeyboardByName(QX11Info::display(),
                                          XkbUseCoreKbd,
                                          &componentNames,
                                          0,
                                          XkbGBN_GeometryMask |
                                          XkbGBN_KeyNamesMask |
                                          XkbGBN_OtherNamesMask |
                                          XkbGBN_ClientSymbolsMask |
                                          XkbGBN_IndicatorMapMask,
                                          false);
    Q_ASSERT(xkb);
    QSharedPointer<XkbDescRec> xkbCleanup(xkb, [](XkbDescPtr obj) {
        XkbFreeKeyboard(obj, 0, True);
    });

    Geometry geometry(xkb->geom, xkb);

    // Register the doodads so we can perform easy type checks with them
    // and determine how to render the individual object.
    const char uri[] = "org.kde.tastenbrett.private";
    qmlRegisterUncreatableType<TextDoodad>(uri, 1, 0, "TextDoodad", QString());
    qmlRegisterUncreatableType<LogoDoodad>(uri, 1, 0, "LogoDoodad", QString());
    qmlRegisterUncreatableType<ShapeDoodad>(uri, 1, 0, "ShapeDoodad", QString());
    qmlRegisterUncreatableType<IndicatorDoodad>(uri, 1, 0, "IndicatorDoodad", QString());

    // The way this is currently written we need the engine after
    // we have a geometry, lest geometry is dtor'd before the engine
    // causing exhaustive error spam on shutdown.
    // Also, the above stuff is blocking, but optimizing it is hardly
    // worth the effort. The Xkb calls altogether take ~8ms (I am not
    // certain putting xkb into a qfuture is thread-safe or even
    // faster). Constructing our QObjects takes 1ms.
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.rootContext()->setContextProperty("geometry", &geometry);
    engine.load(url);

    return app.exec();
}
