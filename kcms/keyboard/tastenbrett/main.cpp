/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QCommandLineParser>
#include <QDebug>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QX11Info>

#include <KAboutData>
#include <KLocalizedString>

#include <memory>

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

    QCommandLineOption modelOption(QStringList{"m", "model"}, {}, QStringLiteral("MODEL"));
    parser.addOption(modelOption);
    QCommandLineOption layoutOption(QStringList{"l", "layout"}, {}, QStringLiteral("LAYOUT"));
    parser.addOption(layoutOption);
    QCommandLineOption variantOption(QStringList{"a", "variant"}, {}, QStringLiteral("VARIANT"));
    parser.addOption(variantOption);
    QCommandLineOption optionsOption(QStringList{"o", "options"}, {}, QStringLiteral("OPTIONS"));
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
    std::unique_ptr<XkbRF_RulesRec, std::function<void(XkbRF_RulesPtr)>> rulesCleanup(rules, [](XkbRF_RulesPtr obj) {
        XkbRF_Free(obj, True);
    });

    XkbComponentNamesRec componentNames;
    memset(&componentNames, 0, sizeof(XkbComponentNamesRec));
    XkbRF_GetComponents(rules, &varDefs, &componentNames);

    QString errorDescription;
    QString errorDetails;
    std::unique_ptr<Geometry> geometry;

    XkbDescPtr xkb =
        XkbGetKeyboardByName(QX11Info::display(),
                             XkbUseCoreKbd,
                             &componentNames,
                             0,
                             XkbGBN_GeometryMask | XkbGBN_KeyNamesMask | XkbGBN_OtherNamesMask | XkbGBN_ClientSymbolsMask | XkbGBN_IndicatorMapMask,
                             false);
    if (!xkb) {
        QProcess setxkbmap;
        QProcess xkbcomp;

        setxkbmap.setStandardOutputProcess(&xkbcomp);
        xkbcomp.setProcessChannelMode(QProcess::MergedChannels); // combine in single channel
        setxkbmap.start(QStringLiteral("setxkbmap"),
                        {QStringLiteral("-print"),
                         QStringLiteral("-model"),
                         model,
                         QStringLiteral("-layout"),
                         layout,
                         QStringLiteral("-variant"),
                         variant,
                         QStringLiteral("-option"),
                         options});
        xkbcomp.start(QStringLiteral("xkbcomp"), {QStringLiteral("-")});
        setxkbmap.waitForFinished();
        xkbcomp.waitForFinished();

        errorDescription = i18nc("@label",
                                 "The keyboard geometry failed to load."
                                 " This often indicates that the selected model does not support a specific layout"
                                 " or layout variant."
                                 " This problem will likely also present when you try to use this combination of model, layout and variant.");
        errorDetails = xkbcomp.readAllStandardOutput();
    } else {
        Q_ASSERT(xkb);
        geometry.reset(new Geometry(xkb->geom, xkb));
    }

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
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.rootContext()->setContextProperty("geometry", geometry.get());
    engine.rootContext()->setContextProperty("errorDescription", errorDescription);
    engine.rootContext()->setContextProperty("errorDetails", errorDetails);
    engine.load(url);

    return app.exec();
}

#include "main.moc"
