/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "emojierplugin.h"

#undef signals
#include "emojiersettings.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <ibus.h>
#include <qqml.h>

struct Emoji {
    QString content;
    QString description;
    QString category;
    QStringList annotations;
};

class AbstractEmojiModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum EmojiRole { CategoryRole = Qt::UserRole + 1, AnnotationsRole };

    int rowCount(const QModelIndex &parent = {}) const override
    {
        return parent.isValid() ? 0 : m_emoji.count();
    }
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!checkIndex(index,
                        QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid
                            | QAbstractItemModel::CheckIndexOption::DoNotUseParent)
            || index.column() != 0)
            return {};

        const auto &emoji = m_emoji[index.row()];
        switch (role) {
        case Qt::DisplayRole:
            return emoji.content;
        case Qt::ToolTipRole:
            return emoji.description;
        case CategoryRole:
            return emoji.category;
        case AnnotationsRole:
            return emoji.annotations;
        }
        return {};
    }

protected:
    QVector<Emoji> m_emoji;
};

class EmojiModel : public AbstractEmojiModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList categories MEMBER m_categories CONSTANT)
public:
    enum EmojiRole { CategoryRole = Qt::UserRole + 1 };

    EmojiModel()
    {
        QLocale locale;
        QVector<QString> dicts;
        const auto bcp = locale.bcp47Name();
        const QString dictName = QLatin1String{"ibus/dicts/emoji-"} + QString(bcp).replace(QLatin1Char('-'), QLatin1Char('_')) + QLatin1String{".dict"};
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, dictName);
        if (!path.isEmpty()) {
            dicts << path;
        }

        const auto idxSpecific = bcp.indexOf(QLatin1Char('-'));
        if (idxSpecific > 0) {
            const QString genericDictName = QLatin1String{"ibus/dicts/emoji-"} + bcp.leftRef(idxSpecific) + QLatin1String{".dict"};
            const QString genericPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, genericDictName);

            if (!genericPath.isEmpty()) {
                dicts << genericPath;
            }
        }

        if (dicts.isEmpty()) {
            const QString genericPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("ibus/dicts/emoji-en.dict"));

            if (!genericPath.isEmpty()) {
                dicts << genericPath;
            }
        }

        if (dicts.isEmpty()) {
            qWarning() << "could not find ibus emoji dictionaries." << dictName;
            return;
        }

        QSet<QString> categories;
        QSet<QString> processedEmoji;
        for (const auto &dictPath : qAsConst(dicts)) {
            GSList *list = ibus_emoji_data_load(dictPath.toUtf8().constData());
            m_emoji.reserve(g_slist_length(list));
            for (GSList *l = list; l; l = l->next) {
                IBusEmojiData *data = (IBusEmojiData *)l->data;
                if (!IBUS_IS_EMOJI_DATA(data)) {
                    qWarning() << "Your dict format is no longer supported.\n"
                                  "Need to create the dictionaries again.";
                    g_slist_free(list);
                    return;
                }

                const QString emoji = QString::fromUtf8(ibus_emoji_data_get_emoji(data));
                const QString description = QString::fromUtf8(ibus_emoji_data_get_description(data));
                if (description == QString::fromUtf8("↑↑↑") || description.isEmpty() || processedEmoji.contains(emoji)) {
                    continue;
                }

                QStringList annotations;
                const auto annotations_glib = ibus_emoji_data_get_annotations(data);
                for (GSList *l = annotations_glib; l; l = l->next) {
                    annotations << QString::fromUtf8((const gchar *)l->data);
                }

                const QString category = QString::fromUtf8(ibus_emoji_data_get_category(data));
                categories.insert(category);
                m_emoji += {emoji, description, category, annotations};
                processedEmoji << emoji;
            }
            g_slist_free(list);
        }

        categories.remove({});
        m_categories = categories.values();
        m_categories.sort();
    }

    Q_SCRIPTABLE QString findFirstEmojiForCategory(const QString &category)
    {
        for (const Emoji &emoji : m_emoji) {
            if (emoji.category == category)
                return emoji.content;
        }
        return {};
    }

private:
    QStringList m_categories;
};

class RecentEmojiModel : public AbstractEmojiModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount CONSTANT)
public:
    RecentEmojiModel()
    {
        refresh();
    }

    Q_SCRIPTABLE void includeRecent(const QString &emoji, const QString &emojiDescription)
    {
        QStringList recent = m_settings.recent();
        QStringList recentDescriptions = m_settings.recentDescriptions();

        const int idx = recent.indexOf(emoji);
        if (idx >= 0) {
            recent.removeAt(idx);
            recentDescriptions.removeAt(idx);
        }
        recent.prepend(emoji);
        recent = recent.mid(0, 50);
        m_settings.setRecent(recent);

        recentDescriptions.prepend(emojiDescription);
        recentDescriptions = recentDescriptions.mid(0, 50);
        m_settings.setRecentDescriptions(recentDescriptions);
        m_settings.save();

        refresh();
    }

    Q_INVOKABLE void clearHistory()
    {
        m_settings.setRecent(QStringList());
        m_settings.setRecentDescriptions(QStringList());
        m_settings.save();

        refresh();
    }

private:
    void refresh()
    {
        beginResetModel();
        auto recent = m_settings.recent();
        auto recentDescriptions = m_settings.recentDescriptions();
        int i = 0;
        m_emoji.clear();
        for (const QString &c : recent) {
            m_emoji += {c, recentDescriptions.at(i++), QString{}, {}};
        }
        endResetModel();
    }

    EmojierSettings m_settings;
};

class CategoryModelFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString category READ category WRITE setCategory)
public:
    QString category() const
    {
        return m_category;
    }
    void setCategory(const QString &category)
    {
        if (m_category != category) {
            m_category = category;
            invalidateFilter();
        }
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        return m_category.isEmpty() || sourceModel()->index(source_row, 0, source_parent).data(EmojiModel::CategoryRole).toString() == m_category;
    }

private:
    QString m_category;
};

class SearchModelFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString search READ search WRITE setSearch)
public:
    QString search() const
    {
        return m_search;
    }
    void setSearch(const QString &search)
    {
        if (m_search != search) {
            m_search = search;
            invalidateFilter();
        }
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        const auto idx = sourceModel()->index(source_row, 0, source_parent);
        return idx.data(Qt::ToolTipRole).toString().contains(m_search, Qt::CaseInsensitive)
            || idx.data(AbstractEmojiModel::AnnotationsRole).toStringList().contains(m_search, Qt::CaseInsensitive);
    }

private:
    QString m_search;
};

class CopyHelperPrivate : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE static void copyTextToClipboard(const QString &text)
    {
        QClipboard *clipboard = qGuiApp->clipboard();
        clipboard->setText(text, QClipboard::Clipboard);
        clipboard->setText(text, QClipboard::Selection);
    }
};

void EmojierDeclarativePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.emoji"));

    qmlRegisterType<EmojiModel>(uri, 1, 0, "EmojiModel");
    qmlRegisterType<CategoryModelFilter>(uri, 1, 0, "CategoryModelFilter");
    qmlRegisterType<SearchModelFilter>(uri, 1, 0, "SearchModelFilter");
    qmlRegisterType<RecentEmojiModel>(uri, 1, 0, "RecentEmojiModel");
    qmlRegisterSingletonType<CopyHelperPrivate>(uri, 1, 0, "CopyHelper", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new CopyHelperPrivate;
    });
}

#include "emojierplugin.moc"
