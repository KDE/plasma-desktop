/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iso_codes.h"
#include "debug.h"

#include <KLocalizedString>

#include <QXmlAttributes>

class IsoCodesPrivate
{
public:
    IsoCodesPrivate(const QString &isoCode_, const QString &isoCodesXmlDir_)
        : isoCode(isoCode_)
        , isoCodesXmlDir(isoCodesXmlDir_)
        , loaded(false)
    {
    }
    void buildIsoEntryList();

    const QString isoCode;
    const QString isoCodesXmlDir;
    QList<IsoCodeEntry> isoEntryList;
    bool loaded;
};

class XmlHandler : public QXmlDefaultHandler
{
public:
    XmlHandler(const QString &isoCode_, QList<IsoCodeEntry> &isoEntryList_)
        : isoCode(isoCode_)
        , qName("iso_" + isoCode + "_entry")
        , isoEntryList(isoEntryList_)
    {
    }

    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes) override;
    //    bool fatalError(const QXmlParseException &exception);
    //    QString errorString() const;

private:
    const QString isoCode;
    const QString qName;
    QList<IsoCodeEntry> &isoEntryList;
};

bool XmlHandler::startElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
    if (qName == this->qName) {
        IsoCodeEntry entry;
        for (int i = 0; i < attributes.count(); i++) {
            entry.insert(attributes.qName(i), attributes.value(i));
        }
        isoEntryList.append(entry);
    }
    return true;
}

IsoCodes::IsoCodes(const QString &isoCode, const QString &isoCodesXmlDir)
    : d(new IsoCodesPrivate(isoCode, isoCodesXmlDir))
{
}

IsoCodes::~IsoCodes()
{
    delete d;
}

QList<IsoCodeEntry> IsoCodes::getEntryList()
{
    if (!d->loaded) {
        d->buildIsoEntryList();
    }
    return d->isoEntryList;
}

// const char* IsoCodes::iso_639="639";
const char IsoCodes::iso_639_3[] = "639_3";
const char IsoCodes::attr_name[] = "name";
// const char* IsoCodes::attr_iso_639_2B_code="iso_639_2B_code";
// const char* IsoCodes::attr_iso_639_2T_code="iso_639_2T_code";
// const char* IsoCodes::attr_iso_639_1_code="iso_639_1_code";
const char IsoCodes::attr_iso_639_3_id[] = "id";

const IsoCodeEntry *IsoCodes::getEntry(const QString &attributeName, const QString &attributeValue)
{
    if (!d->loaded) {
        d->buildIsoEntryList();
    }
    for (QList<IsoCodeEntry>::Iterator it = d->isoEntryList.begin(); it != d->isoEntryList.end(); ++it) {
        const IsoCodeEntry *isoCodeEntry = &(*it);
        if (isoCodeEntry->value(attributeName) == attributeValue)
            return isoCodeEntry;
    }
    return nullptr;
}

void IsoCodesPrivate::buildIsoEntryList()
{
    loaded = true;

    QFile file(QStringLiteral("%1/iso_%2.xml").arg(isoCodesXmlDir, isoCode));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCCritical(KCM_KEYBOARD) << "Can't open the xml file" << file.fileName();
        return;
    }

    XmlHandler xmlHandler(isoCode, isoEntryList);

    QXmlSimpleReader reader;
    reader.setContentHandler(&xmlHandler);
    reader.setErrorHandler(&xmlHandler);

    QXmlInputSource xmlInputSource(&file);

    if (!reader.parse(xmlInputSource)) {
        qCCritical(KCM_KEYBOARD) << "Failed to parse the xml file" << file.fileName();
        return;
    }

    qCDebug(KCM_KEYBOARD) << "Loaded" << isoEntryList.count() << ("iso entry definitions for iso" + isoCode) << "from" << file.fileName();
}
