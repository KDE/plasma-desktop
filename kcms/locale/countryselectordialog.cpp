/***************************************************************************
 *   Copyright (C) 2007 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "countryselectordialog.h"

#include "kcontrollocale.h"

#include <KStandardDirs>

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListView>
#include <QScrollBar>
#include <QStandardPaths>

struct CountryModelItem
{
    CountryModelItem()
    {
    }

    CountryModelItem( CountryModelItem *p, const QString &theText, const QString &theTag )
     : parent(p), text(theText), tag(theTag)
    {
    }

    ~CountryModelItem()
    {
        qDeleteAll(children);
    }

    int row() const
    {
        if (parent) return parent->children.indexOf(const_cast<CountryModelItem*>(this));
        return 0;
    }

    CountryModelItem *parent;
    QList< CountryModelItem* > children;

    QString text, tag;
    KIcon icon;
};



bool CountryModelItemLessThan(CountryModelItem *s1, CountryModelItem *s2)
{
    return s1->text < s2->text;
}



class CountryModel : public QAbstractItemModel
{
    public:
        CountryModel(QObject *parent) : QAbstractItemModel(parent)
        {
            m_rootItem = new CountryModelItem(NULL, QString(), QString());
        }

        CountryModel(CountryModelItem *rootItem, QObject *parent) : QAbstractItemModel(parent)
        {
            m_rootItem = rootItem;
        }

        ~CountryModel()
        {
            if (m_rootItem->parent == NULL) delete m_rootItem;
        }

        void addRegion(const QString &name, const QString &tag)
        {
            CountryModelItem *cmi = new CountryModelItem(m_rootItem, name, tag);
            m_rootItem->children.append(cmi);
        }

        void addSubRegion(const KIcon &flag, const QString &name, const QString &tag, const QString &superRegion)
        {
            CountryModelItem *parent = NULL;
            foreach(CountryModelItem *cm, m_rootItem->children)
            {
                if (cm->tag == superRegion) parent = cm;
            }
            if (parent)
            {
                CountryModelItem *cmi = new CountryModelItem(parent, name, tag);
                cmi->icon = flag;
                parent->children.append(cmi);
            }
        }

        void sort()
        {
            qSort(m_rootItem->children.begin(), m_rootItem->children.end(), CountryModelItemLessThan);
            foreach(CountryModelItem *cm, m_rootItem->children)
            {
                qSort(cm->children.begin(), cm->children.end(), CountryModelItemLessThan);
            }
        }

        int rowCount(const QModelIndex &parent) const
        {
            if (parent.column() > 0)
                return 0;

            CountryModelItem *parentItem;
            if (!parent.isValid())
                parentItem = m_rootItem;
            else
                parentItem = static_cast<CountryModelItem*>(parent.internalPointer());

            return parentItem->children.count();
        }

        int columnCount(const QModelIndex &parent) const
        {
            if (!parent.isValid()) return 1;
            else
            {
                CountryModelItem *p = static_cast<CountryModelItem*>(parent.internalPointer());
                if (p->parent) return 1;
                else return 0;
            }
        }

        QModelIndex index(int row, int column, const QModelIndex &parent) const
        {
            if (!hasIndex(row, column, parent))
                return QModelIndex();

            CountryModelItem *parentItem;
            if (!parent.isValid())
                parentItem = m_rootItem;
            else
                parentItem = static_cast<CountryModelItem*>(parent.internalPointer());

            CountryModelItem *childItem = parentItem->children.at(row);
            if (childItem)
                return createIndex(row, column, childItem);
            else
                return QModelIndex();
        }

        QModelIndex parent(const QModelIndex &index) const
        {
            if (!index.isValid())
                return QModelIndex();

            CountryModelItem *childItem = static_cast<CountryModelItem*>(index.internalPointer());
            CountryModelItem *parentItem = childItem->parent;

            if (parentItem == m_rootItem)
                return QModelIndex();

            return createIndex(parentItem->row(), 0, parentItem);
        }

        QVariant data(const QModelIndex &index, int role) const
        {
            if (index.isValid())
            {
                CountryModelItem *cmi = static_cast<CountryModelItem*>(index.internalPointer());
                if (role == Qt::DisplayRole)
                {
                    return cmi->text;
                }
                else if (role == Qt::DecorationRole)
                {
                    if (cmi->parent->parent) return cmi->icon;
                }
            }
            return QVariant();
        }

    private:
        CountryModelItem *m_rootItem;
};



class CSDListView : public QListView
{
    public:
        CSDListView(QWidget *parent) : QListView(parent)
        {
        }

        void setOtherWidget(QWidget *widget, Qt::Key key)
        {
            m_other = widget;
            m_key = key;
        }

    protected:
        void keyPressEvent(QKeyEvent *event)
        {
            if (event->key() == m_key) m_other->setFocus();
            else QListView::keyPressEvent(event);
        }

    private:
        QWidget *m_other;
        Qt::Key m_key;
};



CountrySelectorDialog::CountrySelectorDialog(QWidget *parent) : KDialog(parent)
{
    setCaption( i18n("Country Selector") );
    setButtons( KDialog::Ok | KDialog::Cancel );

    QWidget *widget = new QWidget(this);
    setMainWidget(widget);
}

bool CountrySelectorDialog::editCountry(KControlLocale *locale)
{
    QHBoxLayout *hbl = new QHBoxLayout(mainWidget());
    CSDListView *lv1 = new CSDListView(mainWidget());
    m_countriesView = new CSDListView(mainWidget());
    hbl->addWidget(lv1);
    hbl->addWidget(m_countriesView);
    lv1->setOtherWidget(m_countriesView, Qt::Key_Right);
    m_countriesView->setOtherWidget(lv1, Qt::Key_Left);

    QString country, region;
    country = locale->country();

    CountryModel *cm = new CountryModel(this);

    const QString &sub = QString::fromLatin1("l10n/");

    QStringList regionlist = KGlobal::dirs()->findAllResources("locale",
                                 sub + QString::fromLatin1("*.desktop"),
                                 KStandardDirs::NoDuplicates);

    QFontMetrics fm(lv1->font());
    int lv1Width = 0;

    foreach(const QString &region, regionlist)
    {
        QString tag = region;
        int index;

        index = tag.lastIndexOf('/');
        if (index != -1)
            tag = tag.mid(index + 1);

        index = tag.lastIndexOf('.');
        if (index != -1)
            tag.truncate(index);

        KConfig entry(region);
        KConfigGroup cg = entry.group("KCM Locale");
        QString name = cg.readEntry("Name", ki18n("without name").toString(locale));

        cm->addRegion(name, tag);
        QString spacedName = name + "  ";
        lv1Width = qMax(lv1Width, fm.width(spacedName));
    }

    // add all languages to the list
    QStringList countrylist = KGlobal::dirs()->findAllResources("locale",
                                 sub + QString::fromLatin1("*/kf5_entry.desktop"),
                                 KStandardDirs::NoDuplicates);
    foreach(const QString &countryFile, countrylist)
    {
        KConfig entry(countryFile);
        KConfigGroup cg = entry.group("KCM Locale");
        QString name = cg.readEntry("Name", ki18n("without name").toString(locale));
        QString parentRegion = cg.readEntry("Region");

        QString tag = countryFile;
        int index = tag.lastIndexOf('/');
        tag.truncate(index);
        index = tag.lastIndexOf('/');
        tag = tag.mid(index + 1);

        QString flag( QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("locale/") + QString::fromLatin1( "l10n/%1/flag.png" ).arg(tag) ) );
        cm->addSubRegion(KIcon(flag), name, tag, parentRegion);
        if (tag == country) region = parentRegion;
    }

    cm->sort();

    cm->addRegion(i18nc("@item:inlistbox Country", "Not set (Generic English)"), "C");
    if (country == "C") region = "C";

    lv1->setModel(cm);
    lv1->setFixedWidth(lv1Width + lv1->verticalScrollBar()->height());
    // + 2 because 1 is "Not set (Generic English)" and the other is for spacing
    lv1->setMinimumHeight((regionlist.count() + 2) * fm.height());

    connect(lv1->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(regionChanged(const QModelIndex &)));
    connect(lv1, SIGNAL(activated(const QModelIndex &)), SLOT(regionActivated()));
    connect(m_countriesView, SIGNAL(activated(const QModelIndex &)), SLOT(accept()));

    for(int i = 0; i < lv1->model()->rowCount(); ++i)
    {
        const QModelIndex &current = lv1->model()->index(i, 0);
        CountryModelItem *cmi = static_cast<CountryModelItem*>(current.internalPointer());
        if (cmi->tag == region)
        {
            lv1->selectionModel()->setCurrentIndex(current, QItemSelectionModel::SelectCurrent);
        }
    }
    if (m_countriesView->model() != NULL)
    {
        for(int i = 0; i < m_countriesView->model()->rowCount(); ++i)
        {
            const QModelIndex &current = m_countriesView->model()->index(i, 0);
            CountryModelItem *cmi = static_cast<CountryModelItem*>(current.internalPointer());
            if (cmi->tag == country)
            {
                m_countriesView->selectionModel()->setCurrentIndex(current, QItemSelectionModel::SelectCurrent);
            }
        }
        m_countriesView->setFocus();
    }
    else lv1->setFocus();

    if (exec() == QDialog::Accepted)
    {
        const QModelIndex &current = m_countriesView->currentIndex();
        if (current.isValid())
        {
            CountryModelItem *cmi = static_cast<CountryModelItem*>(current.internalPointer());
            return locale->setCountry(cmi->tag);
        }
        else if (m_countriesView->model() == NULL)
        {
            return locale->setCountry("C");
        }
    }
    return false;
}

void CountrySelectorDialog::regionChanged(const QModelIndex &current)
{
    delete m_countriesView->model();
    CountryModelItem *cmi = static_cast<CountryModelItem*>(current.internalPointer());
    if (!cmi->children.isEmpty())
    {
        CountryModel *cm = new CountryModel(cmi, this);
        m_countriesView->setModel(cm);
    }
    else
    {
        m_countriesView->setModel(NULL);
    }
}

void CountrySelectorDialog::regionActivated()
{
    if (m_countriesView->model() != NULL) m_countriesView->setFocus();
    else accept();
}
