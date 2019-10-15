#include "main.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QAbstractListModel>
#include <QString>

class KeyboardModelModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit KeyboardModelModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    struct Roles {
        enum {
            DescriptionRole = Qt::UserRole + 1,
            NameRole,
        };
    };

    QHash<int, QByteArray> roleNames() const override;

private:
    struct KeyboardModel {
        QString vendor, type, name;
    };
    QList<KeyboardModel> m_model_list;
};

KeyboardModelModel::KeyboardModelModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_model_list << KeyboardModel{"Generic", "Generic 101-key PC", "101"};
}

int KeyboardModelModel::rowCount(const QModelIndex& parent) const
{
    return m_model_list.size();
}

QVariant KeyboardModelModel::data(const QModelIndex& index, int role) const
{
    auto model = m_model_list.at(index.row());
    if (role == Roles::DescriptionRole)
        return i18nc("vendor | keyboard model", "%1 | %2", model.vendor, model.type);
    if (role == Roles::NameRole)
        return model.name;
    return QVariant::Invalid;
}

Qt::ItemFlags KeyboardModelModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QHash<int, QByteArray> KeyboardModelModel::roleNames() const
{
    return { { Roles::DescriptionRole, "description" }, { Roles::NameRole, "name" } };
}

K_PLUGIN_FACTORY_WITH_JSON(NewKeyboardModuleFactory,
                           "kcm_new_keyboard.json",
                           registerPlugin<KcmNewKeyboard>();)

KcmNewKeyboard::KcmNewKeyboard(QObject* parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args)
{
    KAboutData* about = new KAboutData(
        "kcm_new_keyboard", i18n("..."),
        QStringLiteral("1.0"), QString(), KAboutLicense::GPL);

    about->addAuthor(i18n("Park Gun"));
    setAboutData(about);
    setButtons(Help | Apply | Default);

    qmlRegisterType<KeyboardModelModel>("org.kde.kcm.keyboard", 1, 0, "KeyboardModelModel");
}

KcmNewKeyboard::~KcmNewKeyboard()
{
}

void KcmNewKeyboard::load()
{
}

void KcmNewKeyboard::save()
{
}

void KcmNewKeyboard::defaults()
{
}

void KcmNewKeyboard::setKeyboarModel(const QString& model)
{
    m_keyboard_model = model;
}

QString KcmNewKeyboard::keyboardModel() const
{
    return m_keyboard_model;
}

#include <main.moc>
