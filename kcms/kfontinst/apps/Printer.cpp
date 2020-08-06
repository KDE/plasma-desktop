/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config-fontinst.h"
#include "Printer.h"
#include "FcEngine.h"
#include "ActionLabel.h"
#include <QApplication>
#include <QTextStream>
#include <QFile>
#include <QPainter>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QWidget>
#include <QPrinter>
#include <QPrintDialog>
#include <QFrame>
#include <QGridLayout>
#include <QProgressBar>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <KAboutData>

#include "config-workspace.h"

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#endif
#include "CreateParent.h"

// Enable the following to allow printing of non-installed fonts. Does not seem to work :-(
//#define KFI_PRINT_APP_FONTS

using namespace KFI;

static const int constMarginLineBefore=1;
static const int constMarginLineAfter=2;
static const int constMarginFont=4;

inline bool sufficientSpace(int y, int pageHeight, const QFontMetrics &fm)
{
    return (y+constMarginFont+fm.height())<pageHeight;
}

static bool sufficientSpace(int y, QPainter *painter, QFont font, const int *sizes, int pageHeight, int size)
{
    int titleFontHeight=painter->fontMetrics().height(),
        required=titleFontHeight+constMarginLineBefore+constMarginLineAfter;

    for(unsigned int s=0; sizes[s]; ++s)
    {
        font.setPointSize(sizes[s]);
        required+=QFontMetrics(font, painter->device()).height();
        if(sizes[s+1])
            required+=constMarginFont;
    }

    if(0==size)
    {
        font.setPointSize(CFcEngine::constDefaultAlphaSize);
        int fontHeight=QFontMetrics(font, painter->device()).height();

        required+=(3*(constMarginFont+fontHeight))+
                  constMarginLineBefore+constMarginLineAfter;
    }
    return (y+required)<pageHeight;
}

#warning needs porting to adjust for QFont changes
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
static QString getChars(FT_Face face)
{
    QString newStr;

    for(int cmap=0; cmap<face->num_charmaps; ++cmap)
        if(face->charmaps[cmap] && FT_ENCODING_ADOBE_CUSTOM==face->charmaps[cmap]->encoding)
        {
            FT_Select_Charmap(face, FT_ENCODING_ADOBE_CUSTOM);
            break;
        }

    for(unsigned int i=1; i<65535; ++i)
        if(FT_Get_Char_Index(face, i))
        {
            newStr+=QChar(i);
            if(newStr.length()>255)
                break;
        }
    
    return newStr;
}

static QString usableStr(FT_Face face, const QString &str)
{
    unsigned int slen=str.length(),
                 ch;
    QString      newStr;

    for(ch=0; ch<slen; ++ch)
        if(FT_Get_Char_Index(face, str[ch].unicode()))
            newStr+=str[ch];
    return newStr;
}

static QString usableStr(QFont &font, const QString &str)
{
    FT_Face face=font.freetypeFace();
    return face ? usableStr(face, str) : str;
}

static bool hasStr(QFont &font, const QString &str)
{
    FT_Face face=font.freetypeFace();

    if(!face)
        return true;

    for(int ch=0; ch<str.length(); ++ch)
        if(!FT_Get_Char_Index(face, str[ch].unicode()))
            return false;
    return true;
}

static QString previewString(QFont &font, const QString &text, bool onlyDrawChars)
{
    FT_Face face=font.freetypeFace();

    if(!face)
        return text;

    QString valid(usableStr(face, text));
    bool    drawChars=onlyDrawChars ||
                      (!hasStr(font, CFcEngine::getLowercaseLetters()) &&
                       !hasStr(font, CFcEngine::getUppercaseLetters()));
                       
    return valid.length()<(text.length()/2) || drawChars ? getChars(face) : valid; 
}
#else
static QString usableStr(QFont &font, const QString &str)
{
    Q_UNUSED(font)
    return str;
}

static bool hasStr(QFont &font, const QString &str)
{
    Q_UNUSED(font)
    Q_UNUSED(str)
    return true;
}

static QString previewString(QFont &font, const QString &text, bool onlyDrawChars)
{
    Q_UNUSED(font)
    Q_UNUSED(onlyDrawChars)
    return text;
}
#endif

CPrintThread::CPrintThread(QPrinter *printer, const QList<Misc::TFont> &items, int size, QObject *parent)
            : QThread(parent)
            , itsPrinter(printer)
            , itsItems(items)
            , itsSize(size)
            , itsCancelled(false)
{
}

CPrintThread::~CPrintThread()
{
}

void CPrintThread::cancel()
{
    itsCancelled=true;
}

void CPrintThread::run()
{
    QPainter   painter;
    QFont      sans("sans", 12, QFont::Bold);
    bool       changedFontEmbeddingSetting(false);
    QString    str(CFcEngine(false).getPreviewString());

    if(!itsPrinter->fontEmbeddingEnabled())
    {
        itsPrinter->setFontEmbeddingEnabled(true);
        changedFontEmbeddingSetting=true;
    }

    itsPrinter->setResolution(72);
    painter.begin(itsPrinter);

    int       pageWidth=painter.device()->width(),
              pageHeight=painter.device()->height(),
              y=0,
              oneSize[2]={itsSize, 0};
    const int *sizes=oneSize;
    bool      firstFont(true);

    if(0==itsSize)
        sizes=CFcEngine::constScalableSizes;

    painter.setClipping(true);
    painter.setClipRect(0, 0, pageWidth, pageHeight);

    QList<Misc::TFont>::ConstIterator it(itsItems.constBegin()),
                                      end(itsItems.constEnd());

    for(int i=0; it!=end && !itsCancelled; ++it, ++i)
    {
        QString name(FC::createName((*it).family, (*it).styleInfo));
        emit progress(i, name);

        unsigned int s=0;
        QFont        font;

#ifdef KFI_PRINT_APP_FONTS
        QString      family;

        if(-1!=appFont[(*it).family])
        {
            family=QFontDatabase::applicationFontFamilies(appFont[(*it).family]).first();
            font=QFont(family);
        }
#else
        font=CFcEngine::getQFont((*it).family, (*it).styleInfo, CFcEngine::constDefaultAlphaSize);
#endif
        painter.setFont(sans);

        if(!firstFont && !sufficientSpace(y, &painter, font, sizes, pageHeight, itsSize))
        {
            itsPrinter->newPage();
            y=0;
        }
        painter.setFont(sans);
        y+=painter.fontMetrics().height();
        painter.drawText(0, y, name);

        y+=constMarginLineBefore;
        painter.drawLine(0, y, pageWidth, y);
        y+=constMarginLineAfter;
        
        bool              onlyDrawChars=false;
        Qt::TextElideMode em=Qt::LeftToRight==QApplication::layoutDirection() ? Qt::ElideRight : Qt::ElideLeft;

        if(0==itsSize)
        {
            font.setPointSize(CFcEngine::constDefaultAlphaSize);
            painter.setFont(font);

            QFontMetrics fm(font, painter.device());
            bool         lc=hasStr(font, CFcEngine::getLowercaseLetters()),
                         uc=hasStr(font, CFcEngine::getUppercaseLetters());

            onlyDrawChars=!lc && !uc;
            
            if(lc || uc)
                y+=CFcEngine::constDefaultAlphaSize;
            
            if(lc)
            {
                painter.drawText(0, y, fm.elidedText(CFcEngine::getLowercaseLetters(), em, pageWidth));
                y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
            }
            
            if(uc)
            {
                painter.drawText(0, y, fm.elidedText(CFcEngine::getUppercaseLetters(), em, pageWidth));
                y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
            }
            
            if(lc || uc)
            {
                QString validPunc(usableStr(font, CFcEngine::getPunctuation()));
                if(validPunc.length()>=(CFcEngine::getPunctuation().length()/2))
                {
                    painter.drawText(0, y, fm.elidedText(CFcEngine::getPunctuation(), em, pageWidth));
                    y+=constMarginFont+constMarginLineBefore;
                }
                painter.drawLine(0, y, pageWidth, y);
                y+=constMarginLineAfter;
            }
        }
        
        for(; sizes[s]; ++s)
        {
            y+=sizes[s];
            font.setPointSize(sizes[s]);
            painter.setFont(font);
            
            QFontMetrics fm(font, painter.device());

            if(sufficientSpace(y, pageHeight, fm))
            {
                painter.drawText(0, y, fm.elidedText(previewString(font, str, onlyDrawChars), em, pageWidth));
                if(sizes[s+1])
                    y+=constMarginFont;
            }
            else
                break;
        }
        y+=(s<1 || sizes[s-1]<25 ? 14 : 28);
        firstFont=false;
    }
    emit progress(itsItems.count(), QString());
    painter.end();

    //
    // Did we change the users font settings? If so, reset to their previous values...
    if(changedFontEmbeddingSetting)
        itsPrinter->setFontEmbeddingEnabled(false);
}

CPrinter::CPrinter(QWidget *parent)
        : QDialog(parent)
{
    setWindowTitle(i18n("Print"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CPrinter::slotCancelClicked);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QFrame *page = new QFrame(this);
    QGridLayout *layout=new QGridLayout(page);
    itsStatusLabel=new QLabel(page);
    itsProgress=new QProgressBar(page);
    layout->addWidget(itsActionLabel = new CActionLabel(this), 0, 0, 2, 1);
    layout->addWidget(itsStatusLabel, 0, 1);
    layout->addWidget(itsProgress, 1, 1);
    itsProgress->setRange(0, 100);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 2, 0);

    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
    setMinimumSize(420, 80);
}

CPrinter::~CPrinter()
{
}

void CPrinter::print(const QList<Misc::TFont> &items, int size)
{
    #ifdef HAVE_LOCALE_H
    char *oldLocale=setlocale(LC_NUMERIC, "C");
    #endif
                
    QPrinter     printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, parentWidget());

    if(dialog->exec())
    {
        CPrintThread *thread = new CPrintThread(&printer, items, size, this);

        itsProgress->setRange(0, items.count());
        itsProgress->setValue(0);
        progress(0, QString());
        connect(thread, &CPrintThread::progress, this, &CPrinter::progress);
        connect(thread, &QThread::finished, this, &QDialog::accept);
        connect(this, &CPrinter::cancelled, thread, &CPrintThread::cancel);
        itsActionLabel->startAnimation();
        thread->start();
        exec();
        delete thread;
    }

    delete dialog;
    
    #ifdef HAVE_LOCALE_H
    if(oldLocale)
        setlocale(LC_NUMERIC, oldLocale);
    #endif
}

void CPrinter::progress(int p, const QString &label)
{
    if(!label.isEmpty())
        itsStatusLabel->setText(label);
    itsProgress->setValue(p);
}

void CPrinter::slotCancelClicked()
{
    itsStatusLabel->setText(i18n("Canceling..."));
    emit cancelled();
}

void CPrinter::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    e->ignore();
    slotCancelClicked();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(KFI_CATALOGUE);
    KAboutData aboutData("kfontprint", i18n("Font Printer"), WORKSPACE_VERSION_STRING, i18n("Simple font printer"),
                         KAboutLicense::GPL, i18n("(C) Craig Drummond, 2007"));
    KAboutData::setApplicationData(aboutData);

    QGuiApplication::setWindowIcon(QIcon::fromTheme("kfontprint"));

    QCommandLineParser parser;
    const QCommandLineOption embedOption(QLatin1String("embed"), i18n("Makes the dialog transient for an X app specified by winid"), QLatin1String("winid"));
    parser.addOption(embedOption);
    const QCommandLineOption sizeOption(QLatin1String("size"), i18n("Size index to print fonts"), QLatin1String("index"));
    parser.addOption(sizeOption);
    const QCommandLineOption pfontOption(QLatin1String("pfont"), i18n("Font to print, specified as \"Family,Style\" where Style is a 24-bit decimal number composed as: <weight><width><slant>"), QLatin1String("font"));
    parser.addOption(pfontOption);
    const QCommandLineOption listfileOption(QLatin1String("listfile"), i18n("File containing list of fonts to print"), QLatin1String("file"));
    parser.addOption(listfileOption);
    const QCommandLineOption deletefileOption(QLatin1String("deletefile"), i18n("Remove file containing list of fonts to print"));
    parser.addOption(deletefileOption);

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QList<Misc::TFont> fonts;
    int                size(parser.value(sizeOption).toInt());

    if(size>-1 && size<256)
    {
        QString listFile(parser.value(listfileOption));

        if(!listFile.isEmpty())
        {
            QFile f(listFile);

            if(f.open(QIODevice::ReadOnly))
            {
                QTextStream str(&f);

                while (!str.atEnd())
                {
                    QString family(str.readLine()),
                            style(str.readLine());

                    if(!family.isEmpty() && !style.isEmpty())
                        fonts.append(Misc::TFont(family, style.toUInt()));
                    else
                        break;
                }
                f.close();
            }

            if(parser.isSet(deletefileOption))
                ::unlink(listFile.toLocal8Bit().constData());
        }
        else
        {
            QStringList                fl(parser.values(pfontOption));
            QStringList::ConstIterator it(fl.begin()),
                                       end(fl.end());

            for(; it!=end; ++it)
            {
                QString f(*it);

                int commaPos=f.lastIndexOf(',');

                if(-1!=commaPos)
                    fonts.append(Misc::TFont(f.left(commaPos), f.mid(commaPos+1).toUInt()));
            }
        }

        if(!fonts.isEmpty())
        {
            CPrinter(createParent(parser.value(embedOption).toInt(nullptr, 16))).print(fonts, size);

            return 0;
        }
    }

    return -1;
}

