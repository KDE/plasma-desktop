#ifndef FCITXIMCONFIGMODEL_H
#define FCITXIMCONFIGMODEL_H

#include <QAbstractListModel>
#include "layout_list_sort_filter_proxy_model.h"

class FcitxIMListModel : public LayoutListSortFilterProxyModel
{
    Q_OBJECT

public:
    FcitxIMListModel(QObject *parent);
    QHash<int, QByteArray> roleNames() const override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

class FcitxIMConfigModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool isRealIM READ isRealIM NOTIFY isRealIMChanged)
    Q_PROPERTY(bool isLatinSwitchingEnabled READ isLatinSwitchingEnabled WRITE setIsLatinSwitchingEnabled NOTIFY isLatinSwitchingEnabledChanged)
    Q_PROPERTY(FcitxIMListModel *latinModeLayoutList READ latinModeLayoutList NOTIFY latinModeLayoutListChanged)
    Q_PROPERTY(int selectedLatinLayoutIndex READ selectedLatinLayoutIndex WRITE setSelectedLatinLayoutIndex NOTIFY selectedLatinLayoutIndexChanged)

public:
    explicit FcitxIMConfigModel(QString imName, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    struct Roles {
        enum {
            NameRole = Qt::UserRole + 1,
            GroupRole,
            TypeRole,
            DescriptionRole,
            DataRole,
            CurrentValueRole
        };
    };

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void save() const;

    enum ConfigType {
        IntegerType,
        ColorType,
        CharType,
        StringType,
        I18NStringType,
        BooleanType,
        FileType,
        FontType,
        HotkeyType,
        EnumType
    };
    Q_ENUM(ConfigType)

    FcitxIMListModel* latinModeLayoutList() const;

    bool isLatinSwitchingEnabled() const;
    void setIsLatinSwitchingEnabled(bool isLatinSwitchingEnabled);

    int selectedLatinLayoutIndex() const;
    void setSelectedLatinLayoutIndex(int selectedLatinLayoutIndex);

    QString selectedLatinLayoutId() const;
    void setSelectedLatinLayoutId(const QString &selectedLatinLayoutId);

    bool isRealIM() const;

Q_SIGNALS:
    void isLatinSwitchingEnabledChanged();
    void selectedLatinLayoutIndexChanged();
    void latinModeLayoutListChanged();
    void isRealIMChanged();

private:

    QString m_imName;
    QString m_addonName;

    struct ConfigItem {
        QString group;
        QString name;
        ConfigType type;
        QString description;
        QVariant default_value;
        QVariant current_value;
        QVariant data;
    };

    QList<ConfigItem> m_configs;

    FcitxIMListModel* m_latinModeLayoutList;
    QString m_selectedLatinLayoutId;
    bool m_isLatinSwitchingEnabled;
};

Q_DECLARE_METATYPE(FcitxIMConfigModel*)

#endif // FCITXIMCONFIGMODEL_H
