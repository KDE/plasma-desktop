/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "landingpage.h"

#include <KAboutData>
#include <KCModuleInfo>
#include <KColorScheme>
#include <KLocalizedString>
#include <KPackage/PackageLoader>
#include <KPluginFactory>
#include <KService>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QScreen>
#include <QStandardItemModel>

#if HAVE_KUSERFEEDBACK
#include "landingpage_feedbacksettings.h"
#endif

#include "landingpage_kdeglobalssettings.h"
#include "landingpagedata.h"

#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/Terms>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

K_PLUGIN_FACTORY_WITH_JSON(KCMLandingPageFactory, "kcm_landingpage.json", registerPlugin<KCMLandingPage>(); registerPlugin<LandingPageData>();)

// Program to icon hash
static QHash<QString, QString> s_programs = {{"plasmashell", "plasmashell"}, {"plasma-discover", "plasmadiscover"}};

MostUsedModel::MostUsedModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    sort(0, Qt::DescendingOrder);
    setSortRole(ResultModel::ScoreRole);
    setDynamicSortFilter(true);
    // prepare default items
    m_defaultModel = new QStandardItemModel(this);

    KService::Ptr service = KService::serviceByDesktopName(qGuiApp->desktopFileName());
    if (service) {
        const auto actions = service->actions();
        for (const KServiceAction &action : actions) {
            QStandardItem *item = new QStandardItem();
            item->setData(QUrl(QStringLiteral("kcm:%1.desktop").arg(action.name())), ResultModel::ResourceRole);
            m_defaultModel->appendRow(item);
        }
    } else {
        qCritical() << "Failed to find desktop file for" << qGuiApp->desktopFileName();
    }
}

void MostUsedModel::setResultModel(ResultModel *model)
{
    if (m_resultModel == model) {
        return;
    }

    auto updateModel = [this]() {
        if (m_resultModel->rowCount() >= 5) {
            setSourceModel(m_resultModel);
        } else {
            setSourceModel(m_defaultModel);
        }
    };

    m_resultModel = model;

    connect(m_resultModel, &QAbstractItemModel::rowsInserted, this, updateModel);
    connect(m_resultModel, &QAbstractItemModel::rowsRemoved, this, updateModel);

    updateModel();
}

QHash<int, QByteArray> MostUsedModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(Qt::DisplayRole, "display");
    roleNames.insert(Qt::DecorationRole, "decoration");
    roleNames.insert(ResultModel::ScoreRole, "score");
    roleNames.insert(KcmPluginRole, "kcmPlugin");
    return roleNames;
}

bool MostUsedModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QString desktopName = sourceModel()->index(source_row, 0, source_parent).data(ResultModel::ResourceRole).toUrl().path();
    KService::Ptr service = KService::serviceByStorageId(desktopName);
    return service;
}

QVariant MostUsedModel::data(const QModelIndex &index, int role) const
{
    // MenuItem *mi;
    const QString desktopName = QSortFilterProxyModel::data(index, ResultModel::ResourceRole).toUrl().path();

    KService::Ptr service = KService::serviceByStorageId(desktopName);

    if (!service) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return service->name();
    case Qt::DecorationRole:
        return service->icon();
    case KcmPluginRole: {
        return service->desktopEntryName();
        KCModuleInfo info(service);
        return info.handle();
    }
    case ResultModel::ScoreRole:
        return QSortFilterProxyModel::data(index, ResultModel::ScoreRole);
    default:
        return QVariant();
    }
}

LookAndFeelGroup::LookAndFeelGroup(QObject *parent)
    : QObject(parent)
{
    m_package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
}

QString LookAndFeelGroup::id() const
{
    return m_package.metadata().pluginId();
}

QString LookAndFeelGroup::name() const
{
    return m_package.metadata().name();
}

QString LookAndFeelGroup::thumbnail() const
{
    return m_package.filePath("preview");
    ;
}

KCMLandingPage::KCMLandingPage(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new LandingPageData(this))
{
    qmlRegisterType<LandingPageGlobalsSettings>();
#if HAVE_KUSERFEEDBACK
    qmlRegisterType<FeedbackSettings>();
#endif
    qmlRegisterType<MostUsedModel>();
    qmlRegisterType<LookAndFeelGroup>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_landingpage"),
                                       i18n("Quick Settings"),
                                       QStringLiteral("1.1"),
                                       i18n("Landing page with some basic settings."),
                                       KAboutLicense::GPL);

    about->addAuthor(i18n("Marco Martin"), QString(), QStringLiteral("mart@kde.org"));
    setAboutData(about);

    setButtons(Apply | Help);

    m_mostUsedModel = new MostUsedModel(this);
    m_mostUsedModel->setResultModel(new ResultModel(AllResources | Agent(QStringLiteral("org.kde.systemsettings")) | HighScoredFirst | Limit(5), this));

    m_defaultLightLookAndFeel = new LookAndFeelGroup(this);
    m_defaultDarkLookAndFeel = new LookAndFeelGroup(this);

    m_defaultLightLookAndFeel->m_package.setPath(globalsSettings()->defaultLightLookAndFeel());
    m_defaultDarkLookAndFeel->m_package.setPath(globalsSettings()->defaultDarkLookAndFeel());

    connect(globalsSettings(), &LandingPageGlobalsSettings::lookAndFeelPackageChanged, this, [this]() {
        m_lnfDirty = true;
    });

#if HAVE_KUSERFEEDBACK
    QVector<QProcess *> processes;
    for (const auto &exec : s_programs.keys()) {
        QProcess *p = new QProcess(this);
        p->setProgram(exec);
        p->setArguments({QStringLiteral("--feedback")});
        p->start();
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &KCMLandingPage::programFinished);
        processes << p;
    }
#endif
}

inline void swap(QJsonValueRef v1, QJsonValueRef v2)
{
    QJsonValue temp(v1);
    v1 = QJsonValue(v2);
    v2 = temp;
}

#if HAVE_KUSERFEEDBACK
void KCMLandingPage::programFinished(int exitCode)
{
    auto mo = KUserFeedback::Provider::staticMetaObject;
    const int modeEnumIdx = mo.indexOfEnumerator("TelemetryMode");
    Q_ASSERT(modeEnumIdx >= 0);
    const auto modeEnum = mo.enumerator(modeEnumIdx);

    QProcess *p = qobject_cast<QProcess *>(sender());
    const QString program = p->program();

    if (exitCode) {
        qWarning() << "Could not check" << program;
        return;
    }

    QTextStream stream(p);
    for (QString line; stream.readLineInto(&line);) {
        int sepIdx = line.indexOf(QLatin1String(": "));
        if (sepIdx < 0) {
            break;
        }

        const QString mode = line.left(sepIdx);
        bool ok;
        const int modeValue = modeEnum.keyToValue(qPrintable(mode), &ok);
        if (!ok) {
            qWarning() << "error:" << mode << "is not a valid mode";
            continue;
        }

        const QString description = line.mid(sepIdx + 1);
        m_uses[modeValue][description] << s_programs[program];
    }
    p->deleteLater();

    m_feedbackSources = {};
    for (auto it = m_uses.constBegin(), itEnd = m_uses.constEnd(); it != itEnd; ++it) {
        const auto modeUses = *it;
        for (auto itMode = modeUses.constBegin(), itModeEnd = modeUses.constEnd(); itMode != itModeEnd; ++itMode) {
            m_feedbackSources << QJsonObject({{"mode", it.key()}, {"icons", *itMode}, {"description", itMode.key()}});
        }
    }
    std::sort(m_feedbackSources.begin(), m_feedbackSources.end(), [](const QJsonValue &valueL, const QJsonValue &valueR) {
        const QJsonObject objL(valueL.toObject()), objR(valueR.toObject());
        const auto modeL = objL["mode"].toInt(), modeR = objR["mode"].toInt();
        return modeL < modeR || (modeL == modeR && objL["description"].toString() < objR["description"].toString());
    });
    Q_EMIT feedbackSourcesChanged();
}
#endif

MostUsedModel *KCMLandingPage::mostUsedModel() const
{
    return m_mostUsedModel;
}

LandingPageGlobalsSettings *KCMLandingPage::globalsSettings() const
{
#if HAVE_KUSERFEEDBACK
    return m_data->landingPageGlobalsSettings();
#else
    return m_data->settings();
#endif
}

#if HAVE_KUSERFEEDBACK
bool KCMLandingPage::feedbackEnabled() const
{
    KUserFeedback::Provider p;
    return p.isEnabled();
}

FeedbackSettings *KCMLandingPage::feedbackSettings() const
{
    return m_data->feedbackSettings();
}
#endif

void KCMLandingPage::save()
{
    ManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(3 /*KGlobalSettings::SettingsChanged*/);
    args.append(0 /*KGlobalSettings::SETTINGS_MOUSE*/);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);

    if (m_lnfDirty) {
        QProcess::startDetached(QStringLiteral("plasma-apply-lookandfeel"),
                                QStringList({QStringLiteral("-a"), globalsSettings()->lookAndFeelPackage()}));
    }
}

static void copyEntry(KConfigGroup &from, KConfigGroup &to, const QString &entry)
{
    if (from.hasKey(entry)) {
        to.writeEntry(entry, from.readEntry(entry));
    }
}

LookAndFeelGroup *KCMLandingPage::defaultLightLookAndFeel() const
{
    return m_defaultLightLookAndFeel;
}

LookAndFeelGroup *KCMLandingPage::defaultDarkLookAndFeel() const
{
    return m_defaultDarkLookAndFeel;
}

void KCMLandingPage::openWallpaperDialog()
{
    QString connector;

    QQuickItem *item = mainUi();
    if (!item) {
        return;
    }

    QQuickWindow *quickWindow = item->window();
    if (!quickWindow) {
        return;
    }

    QWindow *window = QQuickRenderControl::renderWindowFor(quickWindow);
    if (!window) {
        return;
    }

    QScreen *screen = window->screen();
    if (screen) {
        connector = screen->name();
    }

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                          QStringLiteral("/PlasmaShell"),
                                                          QStringLiteral("org.kde.PlasmaShell"),
                                                          QStringLiteral("evaluateScript"));

    QList<QVariant> args;
    args << QStringLiteral(R"(
        let id = screenForConnector("%1");

        if (id >= 0) {
            let desktop = desktopForScreen(id);
            desktop.showConfigurationInterface();
        })")
                .arg(connector);

    message.setArguments(args);

    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}

Q_INVOKABLE void KCMLandingPage::openKCM(const QString &kcm)
{
    QProcess::startDetached(QStringLiteral("systemsettings5"), QStringList({kcm}));
}

#include "landingpage.moc"
