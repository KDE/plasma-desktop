/*
    Copyright (c) 1998, 2008, 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "konq_nameandurlinputdialog.h"

#include <klineedit.h>
#include <kurlrequester.h>
#include <kprotocolmanager.h>
#include <kstandardguiitem.h>
#include <khbox.h>
#include <QLayout>
#include <QLabel>

class KonqNameAndUrlInputDialogPrivate
{
public:
    KonqNameAndUrlInputDialogPrivate(KonqNameAndUrlInputDialog* qq) : m_leName(0), m_urlRequester(0), m_fileNameEdited(false), q(qq) {}

    void _k_slotClear();
    void _k_slotNameTextChanged( const QString&);
    void _k_slotURLTextChanged(const QString&);

    /**
     * The line edit widget for the fileName
     */
    KLineEdit *m_leName;
    /**
     * The URL requester for the URL :)
     */
    KUrlRequester *m_urlRequester;
    /**
     * True if the filename was manually edited.
     */
    bool m_fileNameEdited;
    KonqNameAndUrlInputDialog* q;
};

KonqNameAndUrlInputDialog::KonqNameAndUrlInputDialog(const QString& nameLabel, const QString& urlLabel, const KUrl& startDir, QWidget *parent)
    : KDialog( parent ), d(new KonqNameAndUrlInputDialogPrivate(this))
{
    setButtons( Ok | Cancel | User1 );
    setButtonGuiItem( User1, KStandardGuiItem::clear() );

    QFrame *plainPage = new QFrame( this );
    setMainWidget( plainPage );

    QVBoxLayout * topLayout = new QVBoxLayout( plainPage );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( spacingHint() );

    // First line: filename
    KHBox * fileNameBox = new KHBox( plainPage );
    topLayout->addWidget( fileNameBox );

    QLabel * label = new QLabel(nameLabel, fileNameBox);
    d->m_leName = new KLineEdit(fileNameBox);
    d->m_leName->setMinimumWidth(d->m_leName->sizeHint().width() * 3);
    label->setBuddy(d->m_leName);
    d->m_leName->setSelection(0, d->m_leName->text().length()); // autoselect
    connect(d->m_leName, SIGNAL(textChanged(QString)),
            SLOT(_k_slotNameTextChanged(QString)));

    // Second line: url
    KHBox * urlBox = new KHBox( plainPage );
    topLayout->addWidget( urlBox );
    label = new QLabel(urlLabel, urlBox);
    d->m_urlRequester = new KUrlRequester(urlBox);
    d->m_urlRequester->setStartDir(startDir);
    d->m_urlRequester->setMode( KFile::File | KFile::Directory );

    d->m_urlRequester->setMinimumWidth( d->m_urlRequester->sizeHint().width() * 3 );
    connect(d->m_urlRequester->lineEdit(), SIGNAL(textChanged(QString)),
            SLOT(_k_slotURLTextChanged(QString)));
    label->setBuddy(d->m_urlRequester);

    connect(this, SIGNAL(user1Clicked()), this, SLOT(_k_slotClear()));
    d->m_fileNameEdited = false;
}

KonqNameAndUrlInputDialog::~KonqNameAndUrlInputDialog()
{
    delete d;
}

KUrl KonqNameAndUrlInputDialog::url() const
{
    if ( result() == QDialog::Accepted ) {
        return d->m_urlRequester->url();
    }
    else
        return KUrl();
}

QString KonqNameAndUrlInputDialog::name() const
{
    if ( result() == QDialog::Accepted )
        return d->m_leName->text();
    else
        return QString();
}

void KonqNameAndUrlInputDialogPrivate::_k_slotClear()
{
    m_leName->clear();
    m_urlRequester->clear();
    m_fileNameEdited = false;
}

void KonqNameAndUrlInputDialogPrivate::_k_slotNameTextChanged( const QString& )
{
    m_fileNameEdited = true;
    q->enableButtonOk(!m_leName->text().isEmpty() && !m_urlRequester->url().isEmpty());
}

void KonqNameAndUrlInputDialogPrivate::_k_slotURLTextChanged( const QString& )
{
    if (!m_fileNameEdited) {
        // use URL as default value for the filename
        // (we copy only its filename if protocol supports listing,
        // but for HTTP we don't want tons of index.html links)
        KUrl url( m_urlRequester->url() );
        if (KProtocolManager::supportsListing(url) && !url.fileName().isEmpty())
            m_leName->setText( url.fileName() );
        else
            m_leName->setText( url.url() );
        m_fileNameEdited = false; // slotNameTextChanged set it to true erroneously
    }
    q->enableButtonOk(!m_leName->text().isEmpty() && !m_urlRequester->url().isEmpty());
}

void KonqNameAndUrlInputDialog::setSuggestedName(const QString& name)
{
    d->m_leName->setText(name);
    d->m_urlRequester->setFocus();
}

void KonqNameAndUrlInputDialog::setSuggestedUrl(const KUrl& url)
{
    d->m_urlRequester->setUrl(url);
}

#include "konq_nameandurlinputdialog.moc"
