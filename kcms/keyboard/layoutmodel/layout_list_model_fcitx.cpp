#include "layout_list_model_fcitx.h"

#include <KLocalizedString>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <algorithm>
#include <exception>
#include <memory>

#include "input_sources.h"

#include "../fcitx/fcitxqtinputmethodproxy.h"
#include "fcitx_im_config_model.h"

LayoutListModelFcitx::LayoutListModelFcitx(QObject* parent)
    : QAbstractItemModel(parent),
      m_layouts(new QVector<Layout>)
{
    FcitxQtInputMethodItem::registerMetaType();

    if (InputSources::self()->currentSource() == InputSources::Sources::FcitxSource) {
        load();
    }

    // Listen for fcitx dying or becoming alive
    QObject::connect(InputSources::self(), &InputSources::currentSourceChanged,
        [&](int newSource) {
            if (newSource == InputSources::Sources::FcitxSource) {
                load();
            } else {
                beginResetModel();
                m_layouts->clear();
                endResetModel();
            }
        });
}

void LayoutListModelFcitx::load()
{
    if (InputSources::self()->currentSource() == InputSources::Sources::FcitxSource) {
        beginResetModel();
        FcitxQtInputMethodProxy proxy("org.fcitx.Fcitx", "/inputmethod",
            QDBusConnection::sessionBus());
        auto im_list = proxy.iMList();

        m_layouts->clear();
        m_numEnabled = 0;
        for (auto const& im : im_list) {
            Layout layout;
            layout.save_name = FcitxQtInputMethodItem::saveName(im.uniqueName());
            layout.description = im.name();
            layout.layout_id = im.uniqueName();
            layout.language = im.langCode();
            layout.enabled = im.enabled();
            if (layout.enabled) {
                m_numEnabled++;
            }

            *m_layouts << layout;
        }
        endResetModel();
    }
}

int LayoutListModelFcitx::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_layouts->count();
    }

    return 0;
}

int LayoutListModelFcitx::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Qt::ItemFlags LayoutListModelFcitx::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QModelIndex LayoutListModelFcitx::index(int row,
    int column,
    const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (0 <= row && row < m_layouts->count()) {
            return createIndex(row, column, &(*m_layouts)[row]);
        }
    }

    return QModelIndex();
}

QModelIndex LayoutListModelFcitx::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

QVariant LayoutListModelFcitx::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Layout* layout = static_cast<Layout*>(index.internalPointer());
    switch (role) {
    case Roles::NameRole:
        return layout->layout_id;
    case Roles::DescriptionRole:
        return layout->description;
    case Roles::EnabledRole:
        return layout->enabled;
    case Roles::LanguagesRole:
        return layout->language;
    case Roles::IsConfigurableRole:
        return true;
    case Roles::SourceRole:
        return InputSources::Sources::FcitxSource;
    case Roles::ConfigModelRole:
        // TODO: manage memory?
        return QVariant::fromValue(new FcitxIMConfigModel(layout->layout_id, nullptr));
    case Qt::DisplayRole:
    case Roles::SaveNameRole:
        return layout->save_name;
    case Roles::ShortNameRole:
        if (layout->layout_id.startsWith("fcitx-keyboard-")) {
            return layout->layout_id.right(layout->layout_id.size() - 15);
        }
        return layout->layout_id;
    }

    return QVariant();
}
