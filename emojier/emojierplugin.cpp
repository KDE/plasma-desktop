/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "emojierplugin.h"

#undef signals
#include "emojidict.h"
#include "emojiersettings.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <qqml.h>

int AbstractEmojiModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_emoji.size();
}

QVariant AbstractEmojiModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index,
                    QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid
                        | QAbstractItemModel::CheckIndexOption::DoNotUseParent)
        || index.column() != 0)
        return {};

    const auto *emoji = &m_emoji[index.row()];

    if (emoji->skinTone == Tone::Neutral && m_settings.skinTone() >= Tone::Light) {
        emoji = &m_tonedEmojis[emoji->skinToneVariantIndex + m_settings.skinTone() - Tone::Light];
    }

    switch (role) {
    case Qt::DisplayRole:
        return emoji->content;
    case Qt::ToolTipRole:
        return emoji->description;
    case CategoryRole:
        return emoji->categoryName();
    case AnnotationsRole:
        return emoji->annotations;
    case FallbackDescriptionRole:
        return emoji->fallbackDescription;
    case twoToneIndexRole:
        return m_emoji[index.row()].twoToneVariantIndex;
    }
    return {};
}

int TwoToneEmojiModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else if (m_settings.skinTone() == Tone::Neutral) {
        return TWO_TONE_PAIR_PERMUTATIONS + SKIN_TONE_COUNT;
    } else {
        return 8;
    }
}

QVariant TwoToneEmojiModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index,
                    QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid
                        | QAbstractItemModel::CheckIndexOption::DoNotUseParent)
        || index.column() != 0)
        return {};

    const Emoji *emoji;

    if (m_settings.skinTone() == Tone::Neutral) {
        const int skintone = index.row() / (SKIN_TONE_COUNT + 1) + 1;
        if (index.row() % (SKIN_TONE_COUNT + 1) == 0) {
            const int oneToneIndex = m_twoToneEmojis[m_twoToneIndex].skinToneVariantIndex;
            emoji = &m_tonedEmojis[oneToneIndex + skintone - 1];
        } else {
            emoji = &m_twoToneEmojis[m_twoToneIndex + index.row() - skintone];
        }
    } else {
        const int mappedIndex = twoToneSortFilterMap[m_settings.skinTone()][index.row()];
        emoji = &m_twoToneEmojis[m_twoToneIndex + mappedIndex];
    }

    switch (role) {
    case Qt::DisplayRole:
        return emoji->content;
    case Qt::ToolTipRole:
        return emoji->description;
    default:
        return {};
    }
}

void TwoToneEmojiModel::twoToneDataChanged()
{
    beginResetModel();
    endResetModel();
}

int TwoToneEmojiModel::twoToneIndex()
{
    return m_twoToneIndex;
}

void TwoToneEmojiModel::setTwoToneIndex(int twoToneIndex)
{
    if (twoToneIndex >= TWO_TONE_PAIR_PERMUTATIONS) {
        m_twoToneIndex = twoToneIndex - TWO_TONE_PAIR_PERMUTATIONS;
        twoToneDataChanged();
    }
}

EmojiModel::EmojiModel()
{
    const QHash<QString, QString> specialCases{{QLatin1String{"zh-TW"}, QLatin1String{"zh_Hant"}}, {QLatin1String{"zh-HK"}, QLatin1String{"zh_Hant_HK"}}};
    QLocale locale;
    QStringList dicts;
    auto bcp = locale.bcp47Name();
    bcp = specialCases.value(bcp, bcp);
    bcp.replace(QLatin1Char('-'), QLatin1Char('_'));

    const QString dictName = QLatin1String{"plasma/emoji/"} + bcp + QLatin1String{".dict"};
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, dictName);
    if (!path.isEmpty()) {
        dicts << path;
    }

    for (qsizetype underscoreIndex = -1; (underscoreIndex = bcp.lastIndexOf(QLatin1Char('_'), underscoreIndex)) != -1; --underscoreIndex) {
        const QString genericDictName = QLatin1String{"plasma/emoji/"} + QStringView(bcp).left(underscoreIndex) + QLatin1String{".dict"};
        const QString genericPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, genericDictName);

        if (!genericPath.isEmpty()) {
            dicts << genericPath;
        }
    }

    // Always fallback to en, because some annotation data only have minimum data.
    const QString genericPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("plasma/emoji/en.dict"));
    dicts << genericPath;

    if (dicts.isEmpty()) {
        qWarning() << "could not find emoji dictionaries." << dictName;
        return;
    }

    QSet<QString> categories;
    EmojiDict dict;
    // We load in reverse order, because we want to preserve the order in en.dict.
    // en.dict almost always gives complete set of data.
    for (auto iter = dicts.crbegin(); iter != dicts.crend(); ++iter) {
        dict.load(*iter);
    }
    m_emoji = std::move(dict.m_emojis);
    m_tonedEmojis = std::move(dict.m_tonedEmojis);
    for (const auto &emoji : m_emoji) {
        categories.insert(emoji.categoryName());
    }
    m_categories = categories.values();
    m_categories.sort();

    switch (skinTone()) {
    case Tone::Neutral:
    case Tone::Light:
    case Tone::MediumLight:
    case Tone::Medium:
    case Tone::MediumDark:
    case Tone::Dark:
        break;
    default:
        setSkinTone(Tone::Neutral);
    }

    m_TwoToneEmojiModel = new TwoToneEmojiModel(std::move(dict.m_twoToneEmojis), m_tonedEmojis, m_settings);
}

EmojiModel::~EmojiModel()
{
    m_settings.save();
    delete m_TwoToneEmojiModel;
}

QString EmojiModel::findFirstEmojiForCategory(const QString &category)
{
    for (const Emoji &emoji : m_emoji) {
        if (emoji.categoryName() == category)
            return emoji.content;
    }
    return {};
}

void EmojiModel::setSkinTone(int skinTone)
{
    m_settings.setSkinTone(skinTone);
    Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::DisplayRole, Qt::ToolTipRole});
    Q_EMIT skinToneChanged();
    m_TwoToneEmojiModel->twoToneDataChanged();
}

int EmojiModel::skinTone() const
{
    return m_settings.skinTone();
}

RecentEmojiModel::RecentEmojiModel()
{
    refresh();
}

void RecentEmojiModel::includeRecent(const QString &emoji, const QString &emojiDescription)
{
    const auto oldCount = m_emoji.count();

    QStringList recent = m_settings.recent();
    QStringList recentDescriptions = m_settings.recentDescriptions();

    const int idx = recent.indexOf(emoji);
    if (idx > 0) {
        Q_EMIT beginMoveRows(QModelIndex(), idx, idx, QModelIndex(), 0);
        recent.move(idx, 0);
        recentDescriptions.move(idx, 0);
        m_emoji.move(idx, 0);
        Q_EMIT endMoveRows();
    } else if (idx < 0) {
        Q_EMIT beginInsertRows(QModelIndex(), 0, 0);
        recent.prepend(emoji);
        recentDescriptions.prepend(emojiDescription);
        m_emoji.prepend(Emoji{emoji, emojiDescription, {}, 0, {}, Tone::HasNoVariants, 0, 0});
        Q_EMIT endInsertRows();

        if (recent.size() > 50) {
            Q_EMIT beginRemoveRows(QModelIndex(), 50, recent.size() - 1);
            recent = recent.mid(0, 50);
            recentDescriptions = recentDescriptions.mid(0, 50);
            m_emoji = m_emoji.mid(0, 50);
            Q_EMIT endRemoveRows();
        }
    }

    m_settings.setRecent(recent);
    m_settings.setRecentDescriptions(recentDescriptions);
    m_settings.save();

    if (m_emoji.count() != oldCount) {
        Q_EMIT countChanged();
    }
}

void RecentEmojiModel::clearHistory()
{
    m_settings.setRecent(QStringList());
    m_settings.setRecentDescriptions(QStringList());
    m_settings.save();

    refresh(); // emits countChanged.
}

void RecentEmojiModel::refresh()
{
    const auto oldCount = m_emoji.count();

    beginResetModel();
    auto recent = m_settings.recent();
    auto recentDescriptions = m_settings.recentDescriptions();
    int i = 0;
    m_emoji.clear();
    for (const QString &c : recent) {
        m_emoji += {c, recentDescriptions.at(i++), {}, 0, {}, Tone::HasNoVariants, 0, 0};
    }
    endResetModel();

    if (m_emoji.count() != oldCount) {
        Q_EMIT countChanged();
    }
}

QString CategoryModelFilter::category() const
{
    return m_category;
}
void CategoryModelFilter::setCategory(const QString &category)
{
    if (m_category != category) {
        m_category = category;
        invalidateFilter();
    }
}

bool CategoryModelFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return m_category.isEmpty() || sourceModel()->index(source_row, 0, source_parent).data(EmojiModel::CategoryRole).toString() == m_category;
}

QString SearchModelFilter::search() const
{
    return m_search;
}
void SearchModelFilter::setSearch(const QString &search)
{
    if (m_search != search) {
        m_search = search;
        invalidateFilter();
    }
}

bool SearchModelFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    return idx.data(Qt::ToolTipRole).toString().contains(m_search, Qt::CaseInsensitive)
        || idx.data(AbstractEmojiModel::FallbackDescriptionRole).toString().contains(m_search, Qt::CaseInsensitive)
        || !idx.data(AbstractEmojiModel::AnnotationsRole).toStringList().filter(m_search, Qt::CaseInsensitive).isEmpty();
}

void CopyHelperPrivate::copyTextToClipboard(const QString &text)
{
    QClipboard *clipboard = qGuiApp->clipboard();
    clipboard->setText(text, QClipboard::Clipboard);
    clipboard->setText(text, QClipboard::Selection);
}

#include "emojierplugin.moc"

#include "moc_emojierplugin.cpp"
