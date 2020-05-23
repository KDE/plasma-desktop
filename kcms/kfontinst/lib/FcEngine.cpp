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

#include "FcEngine.h"

#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QX11Info>
#include <KConfig>
#include <math.h>
#include <xcb/xcb_image.h>
#include <QFontDatabase>
#include <QApplication>
#include "File.h"
#include <KConfigGroup>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrender.h>
//#define KFI_FC_DEBUG

#define KFI_PREVIEW_GROUP      "KFontInst Preview Settings"
#define KFI_PREVIEW_STRING_KEY "String"

namespace KFI
{
bool      CFcEngine::theirFcDirty(true);
const int CFcEngine::constScalableSizes[]={8, 10, 12, 24, 36, 48, 64, 72, 96, 0 };
const int CFcEngine::constDefaultAlphaSize=24;

static int fcToQtWeight(int weight)
{
    switch(weight)
    {
        case FC_WEIGHT_THIN:
            return 0;
        case FC_WEIGHT_EXTRALIGHT:
            return QFont::Light>>1;
        case FC_WEIGHT_LIGHT:
            return QFont::Light;
        default:
        case FC_WEIGHT_REGULAR:
            return QFont::Normal;
        case FC_WEIGHT_MEDIUM:
#ifdef KFI_HAVE_MEDIUM_WEIGHT
            return (QFont::Normal+QFont::DemiBold)>>1;
#endif
            return QFont::Normal;
        case FC_WEIGHT_DEMIBOLD:
            return QFont::DemiBold;
        case FC_WEIGHT_BOLD:
            return QFont::Bold;
        case FC_WEIGHT_EXTRABOLD:
            return (QFont::Bold+QFont::Black)>>1;
        case FC_WEIGHT_BLACK:
            return QFont::Black;
    }
}

#ifndef KFI_FC_NO_WIDTHS
static int fcToQtWidth(int weight)
{
    switch(weight)
    {
        case KFI_FC_WIDTH_ULTRACONDENSED:
            return QFont::UltraCondensed;
        case KFI_FC_WIDTH_EXTRACONDENSED:
            return QFont::ExtraCondensed;
        case KFI_FC_WIDTH_CONDENSED:
            return QFont::Condensed;
        case KFI_FC_WIDTH_SEMICONDENSED:
            return QFont::SemiCondensed;
        default:
        case KFI_FC_WIDTH_NORMAL:
            return QFont::Unstretched;
        case KFI_FC_WIDTH_SEMIEXPANDED:
            return QFont::SemiExpanded;
        case KFI_FC_WIDTH_EXPANDED:
            return QFont::Expanded;
        case KFI_FC_WIDTH_EXTRAEXPANDED:
            return QFont::ExtraExpanded;
        case KFI_FC_WIDTH_ULTRAEXPANDED:
            return QFont::UltraExpanded;
    }
}
#endif

static bool fcToQtSlant(int slant)
{
    return FC_SLANT_ROMAN==slant ? false : true;
}

inline bool equal(double d1, double d2)
{
    return (fabs(d1 - d2) < 0.0001);
}

inline bool equalWeight(int a, int b)
{
    return a==b || FC::weight(a)==FC::weight(b);
}

#ifndef KFI_FC_NO_WIDTHS
inline bool equalWidth(int a, int b)
{
    return a==b || FC::width(a)==FC::width(b);
}
#endif

inline bool equalSlant(int a, int b)
{
    return a==b || FC::slant(a)==FC::slant(b);
}

static void closeFont(XftFont *&font)
{
    if(font)
        XftFontClose(QX11Info::display(), font);
    font= nullptr;
}

class CFcEngine::Xft
{
    public:

    struct Pix
    {
        Pix() : currentW(0), currentH(0), allocatedW(0), allocatedH(0) { }

        static int getSize(int s)
        {
            static const int constBlockSize=64;

            return ((s/constBlockSize)+(s%constBlockSize ? 1 : 0))*constBlockSize;
        }

        bool allocate(int w, int h)
        {
            int requiredW=getSize(w),
                requiredH=getSize(h);

            currentW=w;
            currentH=h;
            if(requiredW!=allocatedW || requiredH!=allocatedH)
            {
                free();

                if(w && h)
                {
                    allocatedW=requiredW;
                    allocatedH=requiredH;
                    x11=XCreatePixmap(QX11Info::display(), RootWindow(QX11Info::display(), 0), allocatedW, allocatedH,
                                      DefaultDepth(QX11Info::display(), 0));
                    return true;
                }
            }

            return false;
        }

        void free()
        {
            if(allocatedW && allocatedH)
            {
                XFreePixmap(QX11Info::display(), x11);
                allocatedW=allocatedH=0;
            }
        }
        
        int    currentW,
               currentH,
               allocatedW,
               allocatedH;
        Pixmap x11;
    };
    
    Xft();
    ~Xft();

    bool init(const QColor &txt, const QColor &bgnd, int w, int h);
    void freeColors();
    bool drawChar32Centre(XftFont *xftFont, quint32 ch, int w, int h) const;
    bool drawChar32(XftFont *xftFont, quint32 ch,int &x, int &y, int w, int h,
                    int fontHeight, QRect &r) const;
    bool drawString(XftFont *xftFont, const QString &text, int x, int &y, int h) const;
    void drawString(const QString &text, int x, int &y, int h) const;
    bool drawGlyph(XftFont *xftFont, FT_UInt i, int &x, int &y, int w, int h,
                   int fontHeight,bool oneLine, QRect &r) const;
    bool drawAllGlyphs(XftFont *xftFont, int fontHeight, int &x, int &y, int w, int h,
                       bool oneLine=false, int max=-1, QRect *used= nullptr) const;
    bool drawAllChars(XftFont *xftFont, int fontHeight, int &x, int &y, int w, int h,
                      bool oneLine=false, int max=-1, QRect *used= nullptr) const;
    QImage toImage(int w, int h) const;

    private:

    XftDraw  *itsDraw;
    XftColor itsTxtColor,
             itsBgndColor;
    Pix      itsPix;
    QImage::Format imageFormat;
};

CFcEngine::Xft::Xft()
{
    itsDraw= nullptr;
    itsTxtColor.color.alpha=0x0000;
    init(Qt::black, Qt::white, 64, 64);
}

CFcEngine::Xft::~Xft()
{
    freeColors();
    if(itsDraw)
    {
        XftDrawDestroy(itsDraw);
        itsDraw= nullptr;
    }
}

bool CFcEngine::Xft::init(const QColor &txt, const QColor &bnd, int w, int h)
{
    if(itsDraw &&
       (txt.red()<<8 != itsTxtColor.color.red ||
        txt.green()<<8 != itsTxtColor.color.green ||
        txt.blue()<<8 != itsTxtColor.color.blue ||
        bnd.red()<<8 != itsBgndColor.color.red ||
        bnd.green()<<8 != itsBgndColor.color.green ||
        bnd.blue()<<8 != itsBgndColor.color.blue))
        freeColors();

    if(0x0000==itsTxtColor.color.alpha)
    {
        XRenderColor xrenderCol;
        Visual       *visual=DefaultVisual(QX11Info::display(), 0);
        Colormap     colorMap=DefaultColormap(QX11Info::display(), 0);

        xrenderCol.red=bnd.red()<<8;
        xrenderCol.green=bnd.green()<<8;
        xrenderCol.blue=bnd.green()<<8;
        xrenderCol.alpha=0xFFFF;
        XftColorAllocValue(QX11Info::display(), visual, colorMap, &xrenderCol, &itsBgndColor);
        xrenderCol.red=txt.red()<<8;
        xrenderCol.green=txt.green()<<8;
        xrenderCol.blue=txt.green()<<8;
        xrenderCol.alpha=0xFFFF;
        XftColorAllocValue(QX11Info::display(), visual, colorMap, &xrenderCol, &itsTxtColor);
    }

    XVisualInfo defaultVinfo;
    defaultVinfo.depth = DefaultDepth(QX11Info::display(), 0);
    // normal/failsafe
    imageFormat = QImage::Format_RGB32; // 24bit
    switch (defaultVinfo.depth) {
        case 32: imageFormat = QImage::Format_ARGB32_Premultiplied; break;
        case 30: imageFormat = QImage::Format_RGB30; break;
        case 16: imageFormat = QImage::Format_RGB16; break;
        case 8: imageFormat = QImage::Format_Grayscale8; break;
        default: break;
    }
    if (defaultVinfo.depth == 30 || defaultVinfo.depth == 32) {
        // detect correct format
        int num_vinfo = 0;
        defaultVinfo.visual = DefaultVisual(QX11Info::display(), 0);
        defaultVinfo.screen = 0;
        defaultVinfo.visualid = XVisualIDFromVisual(defaultVinfo.visual);
        XVisualInfo *vinfo = XGetVisualInfo(QX11Info::display(), VisualIDMask|VisualScreenMask|VisualDepthMask, &defaultVinfo, &num_vinfo);
        for (int i = 0; i < num_vinfo; ++i) {
            if (vinfo[i].visual == defaultVinfo.visual) {
                if (defaultVinfo.depth == 30) {
                    if (vinfo[i].red_mask == 0x3ff)
                        imageFormat = QImage::Format_BGR30;
                    else if (vinfo[i].blue_mask == 0x3ff)
                        imageFormat = QImage::Format_RGB30;
                } else if (defaultVinfo.depth == 32) {
                    if (vinfo[i].blue_mask == 0xff)
                        imageFormat = QImage::Format_ARGB32_Premultiplied;
                    else if (vinfo[i].red_mask == 0x3ff)
                        imageFormat = QImage::Format_A2BGR30_Premultiplied;
                    else if (vinfo[i].blue_mask == 0x3ff)
                        imageFormat = QImage::Format_A2RGB30_Premultiplied;
                }
                break;
            }
        }
        XFree(vinfo);
    }

    if(itsPix.allocate(w, h) && itsDraw)
        XftDrawChange(itsDraw, itsPix.x11);
    
    if(!itsDraw)
        itsDraw=XftDrawCreate(QX11Info::display(), itsPix.x11, DefaultVisual(QX11Info::display(), 0),
                              DefaultColormap(QX11Info::display(), 0));

    if(itsDraw)
        XftDrawRect(itsDraw, &itsBgndColor, 0, 0, w, h);

    return itsDraw;
}

void CFcEngine::Xft::freeColors()
{
    XftColorFree(QX11Info::display(), DefaultVisual(QX11Info::display(), 0),
                 DefaultColormap(QX11Info::display(), 0), &itsTxtColor);
    XftColorFree(QX11Info::display(), DefaultVisual(QX11Info::display(), 0),
                 DefaultColormap(QX11Info::display(), 0), &itsBgndColor);
    itsTxtColor.color.alpha=0x0000;
}

bool CFcEngine::Xft::drawChar32Centre(XftFont *xftFont, quint32 ch, int w, int h) const
{
    if(XftCharExists(QX11Info::display(), xftFont, ch))
    {
        XGlyphInfo extents;

        XftTextExtents32(QX11Info::display(), xftFont, &ch, 1, &extents);

        int rx(((w-extents.width)/2)+extents.x),
            ry(((h-extents.height)/2)+(extents.y));

        XftDrawString32(itsDraw, &itsTxtColor, xftFont, rx, ry, &ch, 1);
        return true;
    }

    return false;
}

static const int constBorder=2;

bool CFcEngine::Xft::drawChar32(XftFont *xftFont, quint32 ch, int &x, int &y, int w, int h,
                                int fontHeight, QRect &r) const
{
    r=QRect();
    if(XftCharExists(QX11Info::display(), xftFont, ch))
    {
        XGlyphInfo extents;

        XftTextExtents32(QX11Info::display(), xftFont, &ch, 1, &extents);

        if(extents.x>0)
            x+=extents.x;

        if(x+extents.width+constBorder>w)
        {
            x=0;
            if(extents.x>0)
                x+=extents.x;
            y+=fontHeight+constBorder;
        }

        if(y<h)
        {
            r=QRect(x-extents.x, y-extents.y, extents.width+constBorder, extents.height);

            XftDrawString32(itsDraw, &itsTxtColor, xftFont, x, y, &ch, 1);
            x+=extents.xOff+constBorder;
            return true;
        }
        return false;
    }

    return true;
}

bool CFcEngine::Xft::drawString(XftFont *xftFont, const QString &text, int x, int &y, int h) const
{
    XGlyphInfo     extents;
    const FcChar16 *str=(FcChar16 *)(text.utf16());

    XftTextExtents16(QX11Info::display(), xftFont, str, text.length(), &extents);
    if(y+extents.height<=h)
        XftDrawString16(itsDraw, &itsTxtColor, xftFont, x, y+extents.y, str, text.length());
    if(extents.height>0)
    {
        y+=extents.height;
        return true;
    }
    return false;
}

void CFcEngine::Xft::drawString(const QString &text, int x, int &y, int h) const
{
    QFont   qt(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    XftFont *xftFont=XftFontOpen(QX11Info::display(), 0,
                                 FC_FAMILY, FcTypeString, (const FcChar8 *)(qt.family().toUtf8().data()),
                                 FC_WEIGHT, FcTypeInteger, qt.bold() ? FC_WEIGHT_BOLD : FC_WEIGHT_REGULAR,
                                 FC_SLANT, FcTypeInteger, qt.italic() ? FC_SLANT_ITALIC : FC_SLANT_ROMAN,
                                 FC_SIZE, FcTypeDouble, (double)qt.pointSize(),
                                 NULL);

    if(xftFont)
    {
        drawString(xftFont, text, x, y, h);
        closeFont(xftFont);
    }
}

bool CFcEngine::Xft::drawGlyph(XftFont *xftFont, FT_UInt i, int &x, int &y, int w, int h, int fontHeight,
                               bool oneLine, QRect &r) const
{
    XGlyphInfo extents;

    XftGlyphExtents(QX11Info::display(), xftFont, &i, 1, &extents);

    if(0==extents.width || 0==extents.height)
    {
        r=QRect(0, 0, 0, 0);
        return true;
    }

    if(x+extents.width+2>w)
    {
        if(oneLine)
            return false;

        x=0;
        y+=fontHeight+2;
    }

    if(y<h)
    {
        XftDrawGlyphs(itsDraw, &itsTxtColor, xftFont, x, y, &i, 1);
        r=QRect(x-extents.x, y-extents.y, extents.width+constBorder, extents.height);
        x+=extents.width+2;

        return true;
    }
    return false;
}

bool CFcEngine::Xft::drawAllGlyphs(XftFont *xftFont, int fontHeight, int &x, int &y, int w, int h,
                                   bool oneLine, int max, QRect *used) const
{
    bool rv(false);

    if(xftFont)
    {
        FT_Face face=XftLockFace(xftFont);

        if(face)
        {
            int   space(fontHeight/10),
                  drawn(0);
            QRect r;

            if(!space)
                space=1;

            rv=true;
            y+=fontHeight;
            for(int i=1; i<face->num_glyphs && y<h; ++i)
                if(drawGlyph(xftFont, i, x, y, w, h, fontHeight, oneLine, r))
                {
                    if(r.height()>0)
                    {
                        if(used)
                        {
                            if(used->isEmpty())
                                *used=r;
                            else
                                *used=used->united(r);
                        }
                        if(max>0 && ++drawn>=max)
                            break;
                    }
                }
                else
                    break;

            if(oneLine)
                x=0;
            XftUnlockFace(xftFont);
        }
    }

    return rv;
}

bool CFcEngine::Xft::drawAllChars(XftFont *xftFont, int fontHeight, int &x, int &y, int w, int h,
                                  bool oneLine, int max, QRect *used) const
{
    bool rv(false);

    if(xftFont)
    {
        FT_Face face=XftLockFace(xftFont);

        if(face)
        {
            int   space(fontHeight/10),
                  drawn(0);
            QRect r;

            if(!space)
                space=1;

            rv=true;
            y+=fontHeight;

            FT_Select_Charmap(face, FT_ENCODING_UNICODE);
            for(int cmap=0; cmap<face->num_charmaps; ++cmap)
                if(face->charmaps[cmap] && FT_ENCODING_ADOBE_CUSTOM==face->charmaps[cmap]->encoding)
                {
                    FT_Select_Charmap(face, FT_ENCODING_ADOBE_CUSTOM);
                    break;
                }

            for(unsigned int i=1; i<65535 && y<h; ++i)
            {
                int glyph=FT_Get_Char_Index(face, i);

                if(glyph)
                {
                    if(drawGlyph(xftFont, glyph, x, y, w, h, fontHeight, oneLine, r))
                    {
                        if(r.height()>0)
                        {
                            if(used)
                            {
                                if(used->isEmpty())
                                    *used=r;
                                else
                                    *used=used->united(r);
                            }
                            if(max>0 && ++drawn>=max)
                                break;
                        }
                    }
                    else
                        break;
                }
            }

            if(oneLine)
                x=0;
            XftUnlockFace(xftFont);
        }
    }

    return rv;
}

void cleanupXImage(void *data)
{
    xcb_image_destroy((xcb_image_t*)data);
}

QImage CFcEngine::Xft::toImage(int w, int h) const
{
    if (!XftDrawPicture(itsDraw)) {
        return QImage();
    }
    xcb_image_t *xImage = xcb_image_get(QX11Info::connection(), itsPix.x11, 0, 0, itsPix.currentW,
                                        itsPix.currentH, ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
    if (!xImage) {
        return QImage();
    }
    return QImage(xImage->data, xImage->width, xImage->height, xImage->stride, imageFormat, &cleanupXImage, xImage);
}
    
inline int point2Pixel(int point)
{
    return (point*QX11Info::appDpiX()+36)/72;
}

static bool hasStr(XftFont *font, QString &str)
{
    unsigned int slen=str.length(),
                 ch;

    for(ch=0; ch<slen; ++ch)
        if(!FcCharSetHasChar(font->charset, str[ch].unicode()))
            return false;
    return true;
}

static QString usableStr(XftFont *font, QString &str)
{
    unsigned int slen=str.length(),
                 ch;
    QString      newStr;

    for(ch=0; ch<slen; ++ch)
        if(FcCharSetHasChar(font->charset, str[ch].unicode()))
            newStr+=str[ch];
    return newStr;
}

static bool isFileName(const QString &name, quint32 style)
{
    return QChar('/')==name[0] || KFI_NO_STYLE_INFO==style;
}

static void setTransparentBackground(QImage &img, const QColor &col)
{
    // Convert background to transparent, and text to correct colour...
    img=img.convertToFormat(QImage::Format_ARGB32);
    for(int x=0; x<img.width(); ++x)
        for(int y=0; y<img.height(); ++y)
        {
            int v(qRed(img.pixel(x, y)));
            img.setPixel(x, y, qRgba(qMin(col.red()+v, 255),
                                     qMin(col.green()+v, 255),
                                     qMin(col.blue()+v, 255),
                                     255-v));
        }
}

CFcEngine::CFcEngine(bool init)
         : itsIndex(-1),
           itsIndexCount(1),
           itsAlphaSizeIndex(-1),
           itsPreviewString(getDefaultPreviewString()),
           itsXft(nullptr)
{
    if(init)
        reinit();
}

CFcEngine::~CFcEngine()
{
    // Clear any fonts that may have been added...
    FcConfigAppFontClear(FcConfigGetCurrent());
    delete itsXft;
}

void CFcEngine::readConfig(KConfig &cfg)
{
    cfg.group(KFI_PREVIEW_GROUP).readEntry(KFI_PREVIEW_STRING_KEY, getDefaultPreviewString());
}

void CFcEngine::writeConfig(KConfig &cfg)
{
    cfg.group(KFI_PREVIEW_GROUP).writeEntry(KFI_PREVIEW_STRING_KEY, itsPreviewString);
}

QImage CFcEngine::drawPreview(const QString &name, quint32 style, int faceNo, const QColor &txt, const QColor &bgnd, int h)
{
    QImage img;

    if(!name.isEmpty() &&
         ((name==itsName && style==itsStyle && File::equalIndex(faceNo, itsIndex)) ||
          parse(name, style, faceNo)) )
    {
        static const int constOffset=2;
        static const int constInitialWidth=1536;

        getSizes();

        if(!itsSizes.isEmpty())
        {
            //
            // Calculate size of text...
            int  fSize=((int)(h*0.75))-2,
                 origHeight(0);
            bool needAlpha(bgnd.alpha()<255);

            if(!itsScalable) // Then need to get nearest size...
            {
                int bSize=0;

                for(int s=0; s<itsSizes.size(); ++s)
                    if (itsSizes[s]<=fSize || 0==bSize)
                        bSize=itsSizes[s];
                fSize=bSize;
                if(bSize>h)
                {
                    origHeight=h;
                    h=bSize+8;
                }
            }

            if(xft()->init(needAlpha ? Qt::black : txt, needAlpha ? Qt::white : bgnd, constInitialWidth, h))
            {
                XftFont *xftFont=getFont(fSize);
                QString text(itsPreviewString);

                if(xftFont)
                {
                    bool rv=false;
                    int  usedWidth=0;

                    if(hasStr(xftFont, text) || hasStr(xftFont, text=text.toUpper()) ||
                       hasStr(xftFont, text=text.toLower()))
                    {
                        XGlyphInfo     extents;
                        const FcChar16 *str=(FcChar16 *)(text.utf16());

                        XftTextExtents16(QX11Info::display(), xftFont, str, text.length(),
                                         &extents);

                        int y=(h-extents.height)/2;

                        rv=xft()->drawString(xftFont, text, constOffset, y, h);
                        usedWidth=extents.width;
                    }
                    else
                    {
                        int   x=constOffset,
                              y=constOffset;
                        QRect used;

                        rv=xft()->drawAllGlyphs(xftFont, fSize, x, y, constInitialWidth, h, true, text.length(), &used);
                        if(rv)
                            usedWidth=used.width();
                    }

                    if(rv)
                    {
                        img=xft()->toImage(constInitialWidth, h);
                        if(!img.isNull())
                        {
                            if(origHeight)
                            {
                                int width=(int)((usedWidth*(double)(((double)h)/((double)origHeight)))+0.5);
                                img=img.scaledToHeight(origHeight, Qt::SmoothTransformation)
                                       .copy(0, 0, width+(2*constOffset)<constInitialWidth
                                                    ? width+(2*constOffset)
                                                    : constInitialWidth,
                                             origHeight);
                            }
                            else
                                img=img.copy(0, 0, usedWidth+(2*constOffset)<constInitialWidth
                                                    ? usedWidth+(2*constOffset)
                                                    : constInitialWidth,
                                             h);

                            if(needAlpha)
                                setTransparentBackground(img, txt);
                        }
                    }
                    closeFont(xftFont);
                }
            }
        }
    }

    return img;
}

QImage CFcEngine::draw(const QString &name, quint32 style, int faceNo, const QColor &txt, const QColor &bgnd, int fSize, const QString &text_)
{
    QImage img;
    QString text = text_;

    if(!name.isEmpty() &&
         ((name==itsName && style==itsStyle) ||
          parse(name, style, faceNo)) )
    {

        getSizes();
        
        if(!itsSizes.isEmpty())
        {
            if(!itsScalable) // Then need to get nearest size...
            {
                int bSize=0;

                for(int s=0; s<itsSizes.size(); ++s)
                    if (itsSizes[s]<=fSize || 0==bSize)
                        bSize=itsSizes[s];
                fSize=bSize;
            }

            int h = fSize;
            int w = 0;
            
            XftFont *xftFont=getFont(fSize);

            if(xftFont)
            {
                XGlyphInfo     extents;
                const FcChar16 *str=(FcChar16 *)(text.utf16());

                XftTextExtents16(QX11Info::display(), xftFont, str, text.length(),
                                    &extents);

                h = extents.height;
                w = extents.width;
        
                bool needAlpha(bgnd.alpha()<255);
                
                if(xft()->init(needAlpha ? Qt::black : txt, needAlpha ? Qt::white : bgnd, w, h))
                {
                    bool rv=false;

                    if(hasStr(xftFont, text) || hasStr(xftFont, text=text.toUpper()) ||
                    hasStr(xftFont, text=text.toLower()))
                    {
                        XGlyphInfo     extents;
                        const FcChar16 *str=(FcChar16 *)(text.utf16());

                        XftTextExtents16(QX11Info::display(), xftFont, str, text.length(),
                                        &extents);

                        int   x=0,
                              y=0;
                              
                        rv=xft()->drawString(xftFont, text, x, y, h);
                    }
                    else
                    {
                        int   x=0,
                              y=0;
                        QRect used;

                        rv=xft()->drawAllGlyphs(xftFont, h, x, y, w, h, true, text.length(), &used);
                    }

                    if(rv)
                    {
                        img=xft()->toImage(w, h);
                        if(!img.isNull())
                        {
                            img=img.copy(0, 0, w, h);

                            if(needAlpha)
                                setTransparentBackground(img, txt);
                        }
                    }
                }
                closeFont(xftFont);
            }
        }
    }

    return img;
}

QImage CFcEngine::draw(const QString &name, quint32 style, int faceNo, const QColor &txt, const QColor &bgnd,
                       int w, int h, bool thumb, const QList<TRange> &range, QList<TChar> *chars)
{
    QImage img;
    const qreal dpr = qApp->devicePixelRatio();
    w = w * dpr;
    h = h * dpr;
    bool   rv=false;

    if(chars)
        chars->clear();

    if(!name.isEmpty() &&
         ((name==itsName && style==itsStyle && File::equalIndex(faceNo, itsIndex)) ||
          parse(name, style, faceNo)) )
    {
        //
        // We allow kio_thumbnail to cache our thumbs. Normal is 128x128, and large is 256x256
        // ...if kio_thumbnail asks us for a bigger size, then it is probably the file info dialog, in
        // which case treat it as a normal preview...
        if(thumb && (h>256 || w!=h))
            thumb=false;

        int x=0, y=0;

        getSizes();

        if(!itsSizes.isEmpty())
        {
            int  imgWidth(thumb && itsScalable ? w*4 : w),
                 imgHeight(thumb && itsScalable ? h*4 : h);
            bool needAlpha(bgnd.alpha()<255);

            if(xft()->init(needAlpha ? Qt::black : txt, needAlpha ? Qt::white : bgnd, imgWidth, imgHeight))
            {
                XftFont *xftFont=nullptr;
                int     line1Pos(0),
                        line2Pos(0);
                QRect   used(0, 0, 0, 0);

                if(thumb)
                {
                    QString text(itsScalable
                                    ? i18nc("First letter of the alphabet (in upper then lower case)", "Aa")
                                    : i18nc("All letters of the alphabet (in upper/lower case pairs), followed by numbers",
                                            "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789"));
                    //
                    // Calculate size of text...
                    int fSize=h;

                    if(!itsScalable) // Then need to get nearest size...
                    {
                        int bSize=0;

                        for(int s=0; s<itsSizes.size(); ++s)
                            if (itsSizes[s]<=fSize || 0==bSize)
                                bSize=itsSizes[s];
                        fSize=bSize;
                    }

                    xftFont=getFont(fSize);

                    if(xftFont)
                    {
                        QString valid(usableStr(xftFont, text));

                        y=fSize;
                        rv=true;

                        if(itsScalable)
                        {
                            if(valid.length()!=text.length())
                            {
                                text=getPunctuation().mid(1, 2);  // '1' '2'
                                valid=usableStr(xftFont, text);
                            }
                        }
                        else
                            if(valid.length()<(text.length()/2))
                                for(int i=0; i<3; ++i)
                                {
                                    text=0==i ? getUppercaseLetters() : 1==i ? getLowercaseLetters() : getPunctuation();
                                    valid=usableStr(xftFont, text);

                                    if(valid.length()>=(text.length()/2))
                                        break;
                                }

                        if(itsScalable
                            ? valid.length()!=text.length()
                            : valid.length()<(text.length()/2))
                            xft()->drawAllChars(xftFont, fSize, x, y, imgWidth, imgHeight, true,
                                                itsScalable ? 2 : -1, itsScalable ? &used : nullptr);
                        else
                        {
                            QVector<uint> ucs4(valid.toUcs4());
                            QRect         r;

                            for(int ch=0; ch<ucs4.size(); ++ch) // Display char by char so wraps...
                                if(xft()->drawChar32(xftFont, ucs4[ch], x, y, imgWidth, imgHeight, fSize, r))
                                {
                                    if(used.isEmpty())
                                        used=r;
                                    else
                                        used=used.united(r);
                                }
                                else
                                    break;
                        }

                        closeFont(xftFont);
                    }
                }
                else if(0==range.count())
                {
                    QString lowercase(getLowercaseLetters()),
                            uppercase(getUppercaseLetters()),
                            punctuation(getPunctuation());

                    drawName(x, y, h);
                    y+=4;
                    line1Pos=y;
                    y+=8;

                    xftFont=getFont(alphaSize());
                    if(xftFont)
                    {
                        bool lc(hasStr(xftFont, lowercase)),
                             uc(hasStr(xftFont, uppercase)),
                             drawGlyphs=!lc && !uc;

                        if(drawGlyphs)
                            y-=8;
                        else
                        {
                            QString validPunc(usableStr(xftFont, punctuation));
                            bool    punc(validPunc.length()>=(punctuation.length()/2));

                            if(lc)
                                xft()->drawString(xftFont, lowercase, x, y, h);
                            if(uc)
                                xft()->drawString(xftFont, uppercase, x, y, h);
                            if(punc)
                                xft()->drawString(xftFont, validPunc, x, y, h);
                            if(lc || uc || punc)
                                line2Pos=y+2;
                            y+=8;
                        }

                        QString previewString(getPreviewString());

                        if(!drawGlyphs)
                        {
                            if(!lc && uc)
                                previewString=previewString.toUpper();
                            if(!uc && lc)
                                previewString=previewString.toLower();
                        }

                        closeFont(xftFont);
                        for(int s=0; s<itsSizes.size(); ++s)
                            if((xftFont=getFont(itsSizes[s])))
                            {
                                int fontHeight=xftFont->ascent+xftFont->descent;

                                rv=true;
                                if(drawGlyphs)
                                    xft()->drawAllChars(xftFont, fontHeight, x, y, w, h,
                                                         itsSizes.count()>1);
                                else
                                    xft()->drawString(xftFont, previewString, x, y, h);
                                closeFont(xftFont);
                            }
                    }
                }
                else if(1==range.count() && (range.first().null() || 0==range.first().to))
                {
                    if(range.first().null())
                    {
                        drawName(x, y, h);

                        if((xftFont=getFont(alphaSize())))
                        {
                            int fontHeight=xftFont->ascent+xftFont->descent;

                            xft()->drawAllGlyphs(xftFont, fontHeight, x, y, w, h, false);
                            rv=true;
                            closeFont(xftFont);
                        }
                    }
                    else if((xftFont=getFont(int(imgWidth*0.85))))
                    {
                        rv=xft()->drawChar32Centre(xftFont, (*(range.begin())).from,
                                                   imgWidth, imgHeight);
                        closeFont(xftFont);
                    }
                }
                else
                {
                    QList<TRange>::ConstIterator it(range.begin()),
                                                 end(range.end());

                    if((xftFont=getFont(alphaSize())))
                    {
                        rv=true;
                        drawName(x, y, h);
                        y+=alphaSize();

                        bool  stop=false;
                        int   fontHeight=xftFont->ascent+xftFont->descent, xOrig(x), yOrig(y);
                        QRect r;

                        for(it=range.begin(); it!=end && !stop; ++it)
                            for(quint32 c=(*it).from; c<=(*it).to && !stop; ++c)
                            {
                                if(xft()->drawChar32(xftFont, c, x, y, w, h, fontHeight, r))
                                {
                                    if(chars && !r.isEmpty())
                                        chars->append(TChar(r, c));
                                }
                                else
                                    stop=true;
                            }

                        if(x==xOrig && y==yOrig)
                        {
                            // No characters found within the selected range...
                            xft()->drawString(i18n("No characters found."), x, y, h);
                            rv=true;
                        }
                        closeFont(xftFont);
                    }
                }

                if(rv)
                {
                    img=xft()->toImage(imgWidth, imgHeight);
                    if(!img.isNull() && line1Pos)
                    {
                        QPainter p(&img);

                        p.setPen(txt);
                        p.drawLine(0, line1Pos, w-1, line1Pos);
                        if(line2Pos)
                            p.drawLine(0, line2Pos, w-1, line2Pos);
                    }
                        
                    if(!img.isNull())
                    {
                        if(itsScalable && !used.isEmpty() && (used.width()<imgWidth || used.height()<imgHeight))
                            img=img.copy(used);
                        if(needAlpha)
                            setTransparentBackground(img, txt);
                    }
                }
            }
        }
    }

    img.setDevicePixelRatio(dpr);
    return img;
}

QString CFcEngine::getDefaultPreviewString()
{
    return i18nc("A sentence that uses all of the letters of the alphabet",
                 "The quick brown fox jumps over the lazy dog");
}

QString CFcEngine::getUppercaseLetters()
{
    return i18nc("All of the letters of the alphabet, uppercase", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

QString CFcEngine::getLowercaseLetters()
{
    return i18nc("All of the letters of the alphabet, lowercase", "abcdefghijklmnopqrstuvwxyz");
}

QString CFcEngine::getPunctuation()
{
    return i18nc("Numbers and characters", "0123456789.:,;(*!?'/\\\")£$€%^&-+@~#<>{}[]"); //krazy:exclude=i18ncheckarg
}

#ifdef KFI_USE_TRANSLATED_FAMILY_NAME
//
// Try to get the 'string' that matches the users KDE locale..
QString CFcEngine::getFcLangString(FcPattern *pat, const char *val, const char *valLang)
{
    QString                    rv;
    QStringList                kdeLangs=KLocale::global()->languageList(),
                               fontLangs;
    QStringList::ConstIterator it(kdeLangs.begin()),
                               end(kdeLangs.end());

    // Create list of langs that this font's 'val' is encoded in...
    for(int i=0; true; ++i)
    {
        QString lang=getFcString(pat, valLang, i);

        if(lang.isEmpty())
            break;
        else
            fontLangs.append(lang);
    }

    // Now go through the user's KDE locale, and try to find a font match...
    for(; it!=end; ++it)
    {
        int index=fontLangs.findIndex(*it);

        if(-1!=index)
        {
            rv=getFcString(pat, val, index);

            if(!rv.isEmpty())
                break;
        }
    }

    if(rv.isEmpty())
        rv=getFcString(pat, val, 0);
    return rv;
}
#endif

QFont CFcEngine::getQFont(const QString &family, quint32 style, int size)
{
    int weight,
        width,
        slant;

    FC::decomposeStyleVal(style, weight, width, slant);

    QFont font(family, size, fcToQtWeight(weight), fcToQtSlant(slant));

#ifndef KFI_FC_NO_WIDTHS
    font.setStretch(fcToQtWidth(width));
#endif
    return font;
}

bool CFcEngine::parse(const QString &name, quint32 style, int face)
{
    if(name.isEmpty())
        return false;

    reinit();

    itsName=name;
    itsStyle=style;
    itsSizes.clear();
    itsInstalled=!isFileName(name, style);

    if(!itsInstalled)
    {
        int       count;
        FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(itsName).data()),
                                       face<1 ? 0 : face, nullptr, &count);
        if(!pat)
            return false;
        itsDescriptiveName=FC::createName(pat);
        FcPatternDestroy(pat);
    }
    else
        itsDescriptiveName=FC::createName(itsName, itsStyle);

    itsIndex=face<1 ? 0 : face;

    if(!itsInstalled)  // Then add to fontconfig's list, so that Xft can display it...
        addFontFile(itsName);
    return true;
}

XftFont * CFcEngine::queryFont()
{
    static const int constQuerySize=8;

#ifdef KFI_FC_DEBUG
    qDebug();
#endif

    XftFont *f=getFont(constQuerySize);

    if(f && !isCorrect(f, true))
        closeFont(f);

    if(itsInstalled && !f)
    {
        // Perhaps it is a newly installed font? If so try re-initialising fontconfig...
        theirFcDirty=true;
        reinit();

        f=getFont(constQuerySize);

        // This time don't bother checking family - we've re-inited fc anyway, so things should be
        // up to date... And for "Symbol" Fc returns "Standard Symbols L", so wont match anyway!
        if(f && !isCorrect(f, false))
            closeFont(f);
    }
#ifdef KFI_FC_DEBUG
    qDebug() << "ret" << (int)f;
#endif
    return f;
}

XftFont * CFcEngine::getFont(int size)
{
    XftFont *f=nullptr;

#ifdef KFI_FC_DEBUG
    qDebug() << itsName << ' ' << itsStyle << ' ' << size;
#endif

    if(itsInstalled)
    {
        int weight,
            width,
            slant;

        FC::decomposeStyleVal(itsStyle, weight, width, slant);

#ifndef KFI_FC_NO_WIDTHS
        if(KFI_NULL_SETTING!=width)
            f=XftFontOpen(QX11Info::display(), 0,
                          FC_FAMILY, FcTypeString, (const FcChar8 *)(itsName.toUtf8().data()),
                          FC_WEIGHT, FcTypeInteger, weight,
                          FC_SLANT, FcTypeInteger, slant,
                          FC_WIDTH, FcTypeInteger, width,
                          FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                          NULL);
        else
#endif
            f=XftFontOpen(QX11Info::display(), 0,
                          FC_FAMILY, FcTypeString, (const FcChar8 *)(itsName.toUtf8().data()),
                          FC_WEIGHT, FcTypeInteger, weight,
                          FC_SLANT, FcTypeInteger, slant,
                          FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                          NULL);
    }
    else
    {
        FcPattern *pattern = FcPatternBuild(nullptr,
                                            FC_FILE, FcTypeString,
                                                     QFile::encodeName(itsName).constData(),
                                            FC_INDEX, FcTypeInteger, itsIndex<0 ? 0 : itsIndex,
                                            FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                                            NULL);
        f=XftFontOpenPattern(QX11Info::display(), pattern);
    }

#ifdef KFI_FC_DEBUG
    qDebug() << "ret: " << (int)f;
#endif

    return f;
}

bool CFcEngine::isCorrect(XftFont *f, bool checkFamily)
{
    int     iv,
            weight,
            width,
            slant;
    FcChar8 *str;

    if(itsInstalled)
        FC::decomposeStyleVal(itsStyle, weight, width, slant);
        
#ifdef KFI_FC_DEBUG
    QString     xxx;
    QTextStream s(&xxx);
    if(f)
    {
        if(itsInstalled)
        {
            s << "weight:";
            if(FcResultMatch==FcPatternGetInteger(f->pattern, FC_WEIGHT, 0, &iv))
               s << iv << '/' << weight;
            else
                s << "no";

            s << " slant:";
            if(FcResultMatch==FcPatternGetInteger(f->pattern, FC_SLANT, 0, &iv))
                s << iv << '/' << slant;
            else
                s << "no";

            s << " width:";
            if(FcResultMatch==FcPatternGetInteger(f->pattern, FC_WIDTH, 0, &iv))
                s << iv << '/' << width;
            else
                s << "no";

            s << " fam:";
            if(checkFamily)
                if(FcResultMatch==FcPatternGetString(f->pattern, FC_FAMILY, 0, &str) && str)
                    s << QString::fromUtf8((char *)str) << '/' << itsName;
                else
                    s << "no";
            else
                s << "ok";
        }
        else
            s << "NOT Installed...   ";
    }
    else
        s << "No font!!!  ";
    qDebug() << "isCorrect? " << xxx;
#endif

    return
        f
            ? itsInstalled
                ? FcResultMatch==FcPatternGetInteger(f->pattern, FC_WEIGHT, 0, &iv) &&
                   equalWeight(iv, weight) &&
                  FcResultMatch==FcPatternGetInteger(f->pattern, FC_SLANT, 0, &iv) &&
                   equalSlant(iv, slant) &&
    #ifndef KFI_FC_NO_WIDTHS
                  (KFI_NULL_SETTING==width ||
                    (FcResultMatch==FcPatternGetInteger(f->pattern, FC_WIDTH, 0, &iv) &&
                     equalWidth(iv, width))) &&
    #endif
                  (!checkFamily ||
                    (FcResultMatch==FcPatternGetString(f->pattern, FC_FAMILY, 0, &str) && str &&
                     QString::fromUtf8((char *)str)==itsName))
                : (itsIndex<0 || (FcResultMatch==FcPatternGetInteger(f->pattern, FC_INDEX, 0, &iv) && itsIndex==iv)) &&
                  FcResultMatch==FcPatternGetString(f->pattern, FC_FILE, 0, &str) && str &&
                  QString::fromUtf8((char *)str)==itsName
            : false;
}

void CFcEngine::getSizes()
{
    if(!itsSizes.isEmpty())
        return;

#ifdef KFI_FC_DEBUG
    qDebug();
#endif
    XftFont *f=queryFont();
    int     alphaSize(itsSizes.size()>itsAlphaSizeIndex && itsAlphaSizeIndex>=0 ? itsSizes[itsAlphaSizeIndex] : constDefaultAlphaSize);

    itsScalable=FcTrue;

    itsAlphaSizeIndex=0;

    if(f)
    {
        double px(0.0);

        if(itsInstalled)
        {
            if(FcResultMatch!=FcPatternGetBool(f->pattern, FC_SCALABLE, 0, &itsScalable))
                itsScalable=FcFalse;

            if(!itsScalable)
            {
                FcPattern   *pat=nullptr;
                FcObjectSet *os  = FcObjectSetBuild(FC_PIXEL_SIZE, (void*)nullptr);
                int         weight,
                            width,
                            slant;

                FC::decomposeStyleVal(itsStyle, weight, width, slant);

#ifndef KFI_FC_NO_WIDTHS
                if(KFI_NULL_SETTING!=width)
                    pat=FcPatternBuild(nullptr,
                                   FC_FAMILY, FcTypeString,
                                        (const FcChar8 *)(itsName.toUtf8().data()),
                                   FC_WEIGHT, FcTypeInteger, weight,
                                   FC_SLANT, FcTypeInteger, slant,
                                   FC_WIDTH, FcTypeInteger, width,
                                   NULL);
                else
#endif
                    pat=FcPatternBuild(nullptr,
                                   FC_FAMILY, FcTypeString,
                                        (const FcChar8 *)(itsName.toUtf8().data()),
                                   FC_WEIGHT, FcTypeInteger, weight,
                                   FC_SLANT, FcTypeInteger, slant,
                                   NULL);

                FcFontSet *set=FcFontList(nullptr, pat, os);

                FcPatternDestroy(pat);
                FcObjectSetDestroy(os);

                if (set)
                {
                    int size(0);
#ifdef KFI_FC_DEBUG
                    qDebug() << "got fixed sizes: " << set->nfont;
#endif
                    itsSizes.reserve(set->nfont);
                    for (int i = 0; i < set->nfont; i++)
                        if(FcResultMatch==FcPatternGetDouble(set->fonts[i], FC_PIXEL_SIZE, 0, &px))
                        {
                            itsSizes.push_back((int)px);

#ifdef KFI_FC_DEBUG
                            qDebug() << "got fixed: " << px;
#endif
                            if (px<=alphaSize)
                                itsAlphaSizeIndex=size;
                            size++;
                        }
                    FcFontSetDestroy(set);
                }
            }
        }
        else
        {
            FT_Face face=XftLockFace(f);

            if(face)
            {
                itsIndexCount=face->num_faces;
                if(!(itsScalable=FT_IS_SCALABLE(face)))
                {
                    int numSizes=face->num_fixed_sizes,
                        size;


                    itsSizes.reserve(numSizes);

#ifdef KFI_FC_DEBUG
                    qDebug() << "numSizes fixed: " << numSizes;
#endif
                    for (size=0; size<numSizes; size++)
                    {
#if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20105
                        double px=face->available_sizes[size].y_ppem>>6;
#else
                        double px=face->available_sizes[size].width;
#endif
#ifdef KFI_FC_DEBUG
                        qDebug() << "px: " << px;
#endif
                        itsSizes.push_back((int)px);

                        if (px<=alphaSize)
                            itsAlphaSizeIndex=size;
                    }
                }
                XftUnlockFace(f);
            }
        }

        closeFont(f);
    }

    if(itsScalable)
    {
        itsSizes.reserve(sizeof(constScalableSizes)/sizeof(int));

        for (int i=0; constScalableSizes[i]; ++i)
        {
            int px=point2Pixel(constScalableSizes[i]);

            if (px<=alphaSize)
                itsAlphaSizeIndex=i;
            itsSizes.push_back(px);
        }
    }

#ifdef KFI_FC_DEBUG
    qDebug() << "end";
#endif
}

void CFcEngine::drawName(int x, int &y, int h)
{
    QString title(itsDescriptiveName.isEmpty()
                    ? i18n("ERROR: Could not determine font's name.")
                    : itsDescriptiveName);

    if(1==itsSizes.size())
        title=i18np("%2 [1 pixel]", "%2 [%1 pixels]", itsSizes[0], title);

    xft()->drawString(title, x, y, h);
}

void CFcEngine::addFontFile(const QString &file)
{
    if(!itsAddedFiles.contains(file))
    {
        FcInitReinitialize();
        FcConfigAppFontAddFile(FcConfigGetCurrent(), (const FcChar8 *)(QFile::encodeName(file).data()));
        itsAddedFiles.append(file);
    }
}

void CFcEngine::reinit()
{
    if(theirFcDirty)
    {
        FcInitReinitialize();
        theirFcDirty=false;
    }
}

CFcEngine::Xft * CFcEngine::xft()
{
    if(!itsXft)
        itsXft=new Xft;
    return itsXft;
}

}
