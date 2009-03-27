/******************************************************************
Copyright 1993, 1994 by Digital Equipment Corporation, Maynard, Massachusetts,
 
                        All Rights Reserved
 
Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  
 
DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
 
  Author: Hiroyuki Miyamoto  Digital Equipment Corporation
                             miyamoto@jrd.dec.com

    This version tidied and debugged by Steve Underwood May 1999

******************************************************************/

#include <X11/Xlibint.h>
#include <stdlib.h>
#include "FrameMgr.h"

/* Convenient macro */

#define _UNIT(n)   ((int)(n) & 0xFF)
#define _NUMBER(n) (((int)(n) >> 8) & 0xFF)

/* For byte swapping */

#define Swap16(p, n) ((p)->byte_swap ?       \
(((n) << 8 & 0xFF00) | \
 ((n) >> 8 & 0xFF)     \
) : n)
#define Swap32(p, n) ((p)->byte_swap ?            \
        (((n) << 24 & 0xFF000000) | \
         ((n) <<  8 & 0xFF0000) |   \
         ((n) >>  8 & 0xFF00) |     \
         ((n) >> 24 & 0xFF)         \
        ) : n)
#define Swap64(p, n) ((p)->byte_swap ?            \
        (((n) << 56 & 0xFF00000000000000) | \
         ((n) << 40 & 0xFF000000000000) |   \
         ((n) << 24 & 0xFF0000000000) |     \
         ((n) <<  8 & 0xFF00000000) |       \
         ((n) >>  8 & 0xFF000000) |         \
         ((n) >> 24 & 0xFF0000) |           \
         ((n) >> 40 & 0xFF00) |             \
         ((n) >> 56 & 0xFF)                 \
        ) : n)

/* Type definition */

typedef struct _Iter *Iter;

typedef struct _FrameInst *FrameInst;

typedef union
{
    int num; 		/* For BARRAY */
    FrameInst fi; 	/* For POINTER */
    Iter iter; 		/* For ITER */
} ExtraDataRec, *ExtraData;

typedef struct _Chain
{
    ExtraDataRec d;
    int frame_no;
    struct _Chain *next;
} ChainRec, *Chain;

typedef struct _ChainMgr
{
    Chain top;
    Chain tail;
} ChainMgrRec, *ChainMgr;

typedef struct _ChainIter
{
    Chain cur;
} ChainIterRec, *ChainIter;

typedef struct _FrameIter
{
    Iter iter;
    Bool counting;
    unsigned int counter;
    int end;
    struct _FrameIter* next;
} FrameIterRec, *FrameIter;

typedef struct _FrameInst
{
    XimFrame _template;
    ChainMgrRec cm;
    int cur_no;
} FrameInstRec;

typedef void (*IterStartWatchProc) (Iter it, void *client_data);

typedef struct _Iter
{
    XimFrame _template;
    int max_count;
    Bool allow_expansion;
    ChainMgrRec cm;
    int cur_no;
    IterStartWatchProc start_watch_proc;
    void *client_data;
    Bool start_counter;
} IterRec;

typedef struct _FrameMgr
{
    XimFrame frame;
    FrameInst fi;
    char *area;
    int idx;
    Bool byte_swap;
    int total_size;
    FrameIter iters;
} FrameMgrRec;

typedef union
{
    int num;           /* For BARRAY and PAD */
    struct
    {          /* For COUNTER_* */
        Iter iter;
        Bool is_byte_len;
    } counter;
} XimFrameTypeInfoRec, *XimFrameTypeInfo;

/* Special values */
#define NO_VALUE -1
#define NO_VALID_FIELD -2

static FrameInst FrameInstInit(XimFrame frame);
static void FrameInstFree(FrameInst fi);
static XimFrameType FrameInstGetNextType(FrameInst fi, XimFrameTypeInfo info);
static XimFrameType FrameInstPeekNextType(FrameInst fi, XimFrameTypeInfo info);
static FmStatus FrameInstSetSize(FrameInst fi, int num);
static FmStatus FrameInstSetIterCount(FrameInst fi, int num);
static int FrameInstGetTotalSize(FrameInst fi);
static void FrameInstReset(FrameInst fi);

static Iter IterInit(XimFrame frame, int count);
static void IterFree(Iter it);
static int FrameInstGetSize(FrameInst fi);
static int IterGetSize(Iter it);
static XimFrameType IterGetNextType(Iter it, XimFrameTypeInfo info);
static XimFrameType IterPeekNextType(Iter it, XimFrameTypeInfo info);
static FmStatus IterSetSize(Iter it, int num);
static FmStatus IterSetIterCount(Iter it, int num);
static int IterGetTotalSize(Iter it);
static void IterReset(Iter it);
static Bool IterIsLoopEnd(Iter it, Bool* myself);
static void IterSetStartWatch(Iter it, IterStartWatchProc proc, void* client_data);
static void _IterStartWatch(Iter it, void* client_data);

static ExtraData ChainMgrGetExtraData(ChainMgr cm, int frame_no);
static ExtraData ChainMgrSetData(ChainMgr cm, int frame_no,
                                 ExtraDataRec data);
static Bool ChainIterGetNext(ChainIter ci, int* frame_no, ExtraData d);
static int _FrameInstIncrement(XimFrame frame, int count);
static int _FrameInstDecrement(XimFrame frame, int count);
static int _FrameInstGetItemSize(FrameInst fi, int cur_no);
static Bool FrameInstIsIterLoopEnd(FrameInst fi);

static FrameIter _FrameMgrAppendIter(FrameMgr fm, Iter it, int end);
static FrameIter _FrameIterCounterIncr(FrameIter fitr, int i);
static void _FrameMgrRemoveIter(FrameMgr fm, FrameIter it);
static Bool _FrameMgrIsIterLoopEnd(FrameMgr fm);
static Bool _FrameMgrProcessPadding(FrameMgr fm, FmStatus* status);

#define IterGetIterCount(it) ((it)->allow_expansion ? \
NO_VALUE : (it)->max_count)

#define IterFixIteration(it) ((it)->allow_expansion = False)

#define IterSetStarter(it) ((it)->start_counter = True)

#define ChainMgrInit(cm) (cm)->top = (cm)->tail = NULL
#define ChainMgrFree(cm)                \
{                                       \
    Chain tmp;                          \
    Chain cur = (cm)->top;              \
					\
    while (cur)                         \
    {                                   \
        tmp = cur->next;                \
        Xfree (cur);                    \
	cur = tmp;                      \
    }                                   \
}

#define ChainIterInit(ci, cm)           \
{                                       \
    (ci)->cur = (cm)->top;              \
}

/* ChainIterFree has nothing to do. */
#define ChainIterFree(ci)

#define FrameInstIsEnd(fi) ((fi)->_template[(fi)->cur_no].type == EOL)

FrameMgr FrameMgrInit (XimFrame frame, char* area, Bool byte_swap)
{
    FrameMgr fm;

    fm = (FrameMgr) Xmalloc (sizeof (FrameMgrRec));

    fm->frame = frame;
    fm->fi = FrameInstInit (frame);
    fm->area = (char *) area;
    fm->idx = 0;
    fm->byte_swap = byte_swap;
    fm->total_size = NO_VALUE;
    fm->iters = NULL;

    return fm;
}

void FrameMgrInitWithData (FrameMgr fm,
                           XimFrame frame,
                           void * area,
                           Bool byte_swap)
{
    fm->frame = frame;
    fm->fi = FrameInstInit (frame);
    fm->area = (char *) area;
    fm->idx = 0;
    fm->byte_swap = byte_swap;
    fm->total_size = NO_VALUE;
}

void FrameMgrFree (FrameMgr fm)
{
    FrameInstFree (fm->fi);
    Xfree (fm);
}

FmStatus FrameMgrSetBuffer (FrameMgr fm, void* area)
{
    if (fm->area)
        return FmBufExist;
    fm->area = (char *) area;
    return FmSuccess;
}

FmStatus _FrameMgrPutToken (FrameMgr fm, void *data, int data_size)
{
    XimFrameType type;
    XimFrameTypeInfoRec info;

    if (fm->total_size != NO_VALUE  &&  fm->idx >= fm->total_size)
        return FmNoMoreData;
    /*endif*/
    
    type = FrameInstGetNextType(fm->fi, &info);

    if (type & COUNTER_MASK)
    {
        unsigned long input_length;

        if (info.counter.is_byte_len)
        {
            if ((input_length = IterGetTotalSize (info.counter.iter))
                    == NO_VALUE)
            {
                return FmCannotCalc;
            }
            /*endif*/
        }
        else
        {
            if ((input_length = IterGetIterCount (info.counter.iter))
                == NO_VALUE)
            {
                return FmCannotCalc;
            }
            /*endif*/
        }
        /*endif*/
        switch (type)
        {
        case COUNTER_BIT8:
            *(CARD8 *) (fm->area + fm->idx) = input_length;
            fm->idx++;
            break;
        
        case COUNTER_BIT16:
            *(CARD16 *) (fm->area + fm->idx) = Swap16 (fm, input_length);
            fm->idx += 2;
            break;
            
        case COUNTER_BIT32:
            *(CARD32 *) (fm->area + fm->idx) = Swap32 (fm, input_length);
            fm->idx += 4;
            break;

#if defined(_NEED64BIT)
        case COUNTER_BIT64:
            *(CARD64 *) (fm->area + fm->idx) = Swap64 (fm, input_length);
            fm->idx += 8;
            break;
#endif
	default:
	    break;
        }
        /*endswitch*/
        _FrameMgrPutToken(fm, data, data_size);
        return FmSuccess;
    }
    /*endif*/

    switch (type)
    {
    case BIT8:
        if (data_size == sizeof (unsigned char))
        {
            unsigned long num = *(unsigned char *) data;
            *(CARD8 *) (fm->area + fm->idx) = num;
        }
        else if (data_size == sizeof (unsigned short))
        {
            unsigned long num = *(unsigned short *) data;
            *(CARD8 *) (fm->area + fm->idx) = num;
        }
        else if (data_size == sizeof (unsigned int))
        {
            unsigned long num = *(unsigned int *) data;
            *(CARD8 *) (fm->area + fm->idx) = num;
        }
        else if (data_size == sizeof (unsigned long))
        {
            unsigned long num = *(unsigned long *) data;
            *(CARD8 *) (fm->area + fm->idx) = num;
        }
        else
        {
            ; /* Should never be reached */
        }
        /*endif*/
        fm->idx++;
        return FmSuccess;

    case BIT16:
        if (data_size == sizeof (unsigned char))
        {
            unsigned long num = *(unsigned char *) data;
            *(CARD16*)(fm->area + fm->idx) = Swap16 (fm, num);
        }
        else if (data_size == sizeof (unsigned short))
        {
            unsigned long num = *(unsigned short *) data;
            *(CARD16 *) (fm->area + fm->idx) = Swap16 (fm, num);
        }
        else if (data_size == sizeof (unsigned int))
        {
            unsigned long num = *(unsigned int *) data;
            *(CARD16 *) (fm->area + fm->idx) = Swap16 (fm, num);
        }
        else if (data_size == sizeof (unsigned long))
        {
            unsigned long num = *(unsigned long *) data;
            *(CARD16 *) (fm->area + fm->idx) = Swap16 (fm, num);
        }
        else
        {
            ; /* Should never reached */
        }
        /*endif*/
        fm->idx += 2;
        return FmSuccess;

    case BIT32:
        if (data_size == sizeof (unsigned char))
        {
            unsigned long num = *(unsigned char *) data;
            *(CARD32 *) (fm->area + fm->idx) = Swap32 (fm, num);
        }
        else if (data_size == sizeof (unsigned short))
        {
            unsigned long num = *(unsigned short *) data;
            *(CARD32 *) (fm->area + fm->idx) = Swap32 (fm, num);
        }
        else if (data_size == sizeof (unsigned int))
        {
            unsigned long num = *(unsigned int *) data;
            *(CARD32 *) (fm->area + fm->idx) = Swap32 (fm, num);
        }
        else if (data_size == sizeof (unsigned long))
        {
            unsigned long num = *(unsigned long *) data;
            *(CARD32 *) (fm->area + fm->idx) = Swap32 (fm, num);
        }
        else
        {
            ; /* Should never reached */
        }
        /*endif*/
        fm->idx += 4;
        return FmSuccess;

#if defined(_NEED64BIT)
    case BIT64:
        if (data_size == sizeof (unsigned char))
        {
            unsigned long num = *(unsigned char *) data;
            *(CARD64 *) (fm->area + fm->idx) = Swap64 (fm, num);
        }
        else if (data_size == sizeof (unsigned short))
        {
            unsigned long num = *(unsigned short *) data;
            *(CARD64 *) (fm->area + fm->idx) = Swap64 (fm, num);
        }
        else if (data_size == sizeof (unsigned int))
        {
            unsigned long num = *(unsigned int *) data;
            *(CARD64 *) (fm->area + fm->idx) = Swap64 (fm, num);
        }
        else if (data_size == sizeof (unsigned long))
        {
            unsigned long num = *(unsigned long *) data;
            *(CARD64 *) (fm->area + fm->idx) = Swap64 (fm, num);
        }
        else
        {
            ; /* Should never reached */
        }
        /*endif*/
        fm->idx += 4;
        return FmSuccess;
#endif

    case BARRAY:
        if (info.num == NO_VALUE)
            return FmInvalidCall;
        /*endif*/
        if (info.num > 0)
        {
            bcopy (*(char **) data, fm->area + fm->idx, info.num);
            fm->idx += info.num;
        }
        /*endif*/
        return FmSuccess;

    case PADDING:
        if (info.num == NO_VALUE)
            return FmInvalidCall;
        /*endif*/
        fm->idx += info.num;
        return _FrameMgrPutToken(fm, data, data_size);

    case ITER:
        return FmInvalidCall;
        
    case EOL:
        return FmEOD;
    default:
	break;
    }
    /*endswitch*/
    return (FmStatus) NULL;  /* Should never be reached */
}

FmStatus _FrameMgrGetToken (FrameMgr fm , void* data, int data_size)
{
    XimFrameType type;
    static XimFrameTypeInfoRec info;  /* memory */
    FrameIter fitr;

    if (fm->total_size != NO_VALUE  &&  fm->idx >= fm->total_size)
        return FmNoMoreData;
    /*endif*/
    
    type = FrameInstGetNextType(fm->fi, &info);

    if (type & COUNTER_MASK)
    {
        int end=0;
        FrameIter client_data;

        type &= ~COUNTER_MASK;
        switch (type)
        {
        case BIT8:
            end = *(CARD8 *) (fm->area + fm->idx);
            break;
        
        case BIT16:
            end = Swap16 (fm, *(CARD16 *) (fm->area + fm->idx));
            break;
        
        case BIT32:
            end = Swap32 (fm, *(CARD32 *) (fm->area + fm->idx));
            break;

#if defined(_NEED64BIT)        
        case BIT64:
            end = Swap64 (fm, *(CARD64 *) (fm->area + fm->idx));
            break;
#endif
	default:
	    break;
        }
        /*endswitch*/
        
        if ((client_data = _FrameMgrAppendIter (fm, info.counter.iter, end)))
        {
            IterSetStarter (info.counter.iter);
            IterSetStartWatch (info.counter.iter,
                               _IterStartWatch,
                               (void *) client_data);
        }
        /*endif*/
    }
    /*endif*/

    type &= ~COUNTER_MASK;
    switch (type)
    {
    case BIT8:
        if (data_size == sizeof (unsigned char))
        {
            *(unsigned char*) data = *(CARD8 *) (fm->area + fm->idx);
        }
        else if (data_size == sizeof (unsigned short))
        {
            *(unsigned short *) data = *(CARD8 *) (fm->area + fm->idx);
        }
        else if (data_size == sizeof (unsigned int))
        {
            *(unsigned int *) data = *(CARD8 *) (fm->area + fm->idx);
        }
        else if (data_size == sizeof (unsigned long))
        {
            *(unsigned long *) data = *(CARD8 *) (fm->area + fm->idx);
        }
        else
        {
            ; /* Should never reached */
        }
        /*endif*/
        fm->idx++;
        if ((fitr = _FrameIterCounterIncr (fm->iters, 1/*BIT8*/)))
            _FrameMgrRemoveIter (fm, fitr);
        /*endif*/
        return FmSuccess;

    case BIT16:
        if (data_size == sizeof (unsigned char))
        {
            *(unsigned char *) data =
                Swap16 (fm, *(CARD16 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned short))
        {
            *(unsigned short *) data =
                Swap16 (fm, *(CARD16 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned int))
        {
            *(unsigned int *) data =
                Swap16 (fm, *(CARD16 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned long))
        {
            *(unsigned long *) data =
                Swap16 (fm, *(CARD16 *) (fm->area + fm->idx));
        }
        else
        {
            ; /* Should never reached */
        }
        /*endif*/
        fm->idx += 2;
        if ((fitr = _FrameIterCounterIncr (fm->iters, 2/*BIT16*/)))
            _FrameMgrRemoveIter(fm, fitr);
        /*endif*/
        return FmSuccess;

    case BIT32:
        if (data_size == sizeof (unsigned char))
        {
            *(unsigned char *) data =
                Swap32 (fm, *(CARD32 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned short))
        {
            *(unsigned short *) data =
                Swap32 (fm, *(CARD32 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned int))
        {
            *(unsigned int *) data =
                Swap32 (fm, *(CARD32 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned long))
        {
            *(unsigned long *) data =
                Swap32 (fm, *(CARD32 *) (fm->area + fm->idx));
        }
        else
        {
            ; /* Should never reached */
        }
        /*endif*/
        fm->idx += 4;
        if ((fitr = _FrameIterCounterIncr (fm->iters, 4/*BIT32*/)))
            _FrameMgrRemoveIter (fm, fitr);
        /*endif*/
        return FmSuccess;

#if defined(_NEED64BIT)    
    case BIT64:
        if (data_size == sizeof (unsigned char))
        {
            *(unsigned char *) data =
                Swap64 (fm, *(CARD64 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned short))
        {
            *(unsigned short *) data =
                Swap64 (fm, *(CARD64 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned int))
        {
            *(unsigned int *) data =
                Swap64 (fm, *(CARD64 *) (fm->area + fm->idx));
        }
        else if (data_size == sizeof (unsigned long))
        {
            *(unsigned long *) data =
                Swap64 (fm, *(CARD64 *) (fm->area + fm->idx));
        }
        else
        {
            ; /* Should never reached */
        }
        /*endif*/
        fm->idx += 8;
        if ((fitr = _FrameIterCounterIncr (fm->iters, 8/*BIT64*/)))
            _FrameMgrRemoveIter (fm, fitr);
        /*endif*/
        return FmSuccess;
#endif

    case BARRAY:
        if (info.num == NO_VALUE)
            return FmInvalidCall;
        /*endif*/
        if (info.num > 0)
        {
            *(char **) data = fm->area + fm->idx;
            
            fm->idx += info.num;
            if ((fitr = _FrameIterCounterIncr (fm->iters, info.num)))
                _FrameMgrRemoveIter (fm, fitr);
            /*endif*/
        }
        else
        {
            *(char **) data = NULL;
        }
        /*endif*/
        return FmSuccess;

    case PADDING:
        if (info.num == NO_VALUE)
            return FmInvalidCall;
        /*endif*/
        fm->idx += info.num;
        if ((fitr = _FrameIterCounterIncr (fm->iters, info.num)))
            _FrameMgrRemoveIter (fm, fitr);
        /*endif*/
        return _FrameMgrGetToken (fm, data, data_size);

    case ITER:
        return FmInvalidCall; 	/* if comes here, it's a bug! */

    case EOL:
        return FmEOD;
    default:
	break;
    }
    /*endswitch*/
    return (FmStatus) NULL;  /* Should never be reached */
}

FmStatus FrameMgrSetSize (FrameMgr fm, int barray_size)
{
    if (FrameInstSetSize (fm->fi, barray_size) == FmSuccess)
        return FmSuccess;
    /*endif*/
    return FmNoMoreData;
}

FmStatus FrameMgrSetIterCount (FrameMgr fm, int count)
{
    if (FrameInstSetIterCount (fm->fi, count) == FmSuccess)
        return FmSuccess;
    /*endif*/
    return FmNoMoreData;
}

FmStatus FrameMgrSetTotalSize (FrameMgr fm, int total_size)
{
    fm->total_size = total_size;
    return FmSuccess;
}

int FrameMgrGetTotalSize (FrameMgr fm)
{
    return FrameInstGetTotalSize (fm->fi);
}

int FrameMgrGetSize (FrameMgr fm)
{
    register int ret_size;

    ret_size = FrameInstGetSize (fm->fi);
    if (ret_size == NO_VALID_FIELD)
        return NO_VALUE;
    /*endif*/
    return ret_size;
}

FmStatus FrameMgrSkipToken (FrameMgr fm, int skip_count)
{
    XimFrameType type;
    XimFrameTypeInfoRec info;
    register int i;

    if (fm->total_size != NO_VALUE  &&  fm->idx >= fm->total_size)
        return FmNoMoreData;
    /*endif*/
    for (i = 0;  i < skip_count;  i++)
    {
        type = FrameInstGetNextType (fm->fi, &info);
        type &= ~COUNTER_MASK;

        switch (type)
        {
        case BIT8:
            fm->idx++;
            break;
            
        case BIT16:
            fm->idx += 2;
            break;
            
        case BIT32:
            fm->idx += 4;
            break;
            
        case BIT64:
            fm->idx += 8;
            break;
            
        case BARRAY:
            if (info.num == NO_VALUE)
                return FmInvalidCall;
            /*endif*/
            fm->idx += info.num;
            break;
            
        case PADDING:
            if (info.num == NO_VALUE)
                return FmInvalidCall;
            /*endif*/
            fm->idx += info.num;
            return FrameMgrSkipToken (fm, skip_count);
            
        case ITER:
            return FmInvalidCall;
            
        case EOL:
            return FmEOD;
	default:
	    break;
        }
        /*endswitch*/
    }
    /*endfor*/
    return FmSuccess;
}

void FrameMgrReset (FrameMgr fm)
{
    fm->idx = 0;
    FrameInstReset (fm->fi);
}

Bool FrameMgrIsIterLoopEnd (FrameMgr fm, FmStatus* status)
{
    do
    {
        if (_FrameMgrIsIterLoopEnd (fm))
            return  True;
        /*endif*/
    }
    while (_FrameMgrProcessPadding (fm, status));

    return False;
}


/* Internal routines */

static Bool _FrameMgrIsIterLoopEnd (FrameMgr fm)
{
    return FrameInstIsIterLoopEnd (fm->fi);
}

static Bool _FrameMgrProcessPadding (FrameMgr fm, FmStatus* status)
{
    XimFrameTypeInfoRec info;
    XimFrameType next_type = FrameInstPeekNextType (fm->fi, &info);
    FrameIter fitr;

    if (next_type == PADDING)
    {
        if (info.num == NO_VALUE)
        {
            *status = FmInvalidCall;
            return True;
        }
        /*endif*/
        next_type = FrameInstGetNextType (fm->fi, &info);
        fm->idx += info.num;
        if ((fitr = _FrameIterCounterIncr (fm->iters, info.num)))
            _FrameMgrRemoveIter (fm, fitr);
        /*endif*/
        *status = FmSuccess;
        return True;
    }
    /*endif*/
    *status = FmSuccess;
    return False;
}

static FrameInst FrameInstInit (XimFrame frame)
{
    FrameInst fi;

    fi = (FrameInst) Xmalloc (sizeof (FrameInstRec));

    fi->_template = frame;
    fi->cur_no = 0;
    ChainMgrInit (&fi->cm);
    return fi;
}

static void FrameInstFree (FrameInst fi)
{
    ChainIterRec ci;
    int frame_no;
    ExtraDataRec d;

    ChainIterInit (&ci, &fi->cm);

    while (ChainIterGetNext (&ci, &frame_no, &d))
    {
        register XimFrameType type;
        type = fi->_template[frame_no].type;
        if (type == ITER)
        {
            if (d.iter)
                IterFree (d.iter);
            /*endif*/
        }
        else if (type == POINTER)
        {
            if (d.fi)
                FrameInstFree (d.fi);
            /*endif*/
        }
        /*endif*/
    }
    /*endwhile*/
    ChainIterFree (&ci);
    ChainMgrFree (&fi->cm);
    Xfree (fi);
}

static XimFrameType FrameInstGetNextType(FrameInst fi, XimFrameTypeInfo info)
{
    XimFrameType ret_type;

    ret_type = fi->_template[fi->cur_no].type;

    switch (ret_type)
    {
    case BIT8:
    case BIT16:
    case BIT32:
    case BIT64:
    case EOL:
        fi->cur_no = _FrameInstIncrement(fi->_template, fi->cur_no);
        break;

    case COUNTER_BIT8:
    case COUNTER_BIT16:
    case COUNTER_BIT32:
    case COUNTER_BIT64:
        if (info)
        {
            register int offset, iter_idx;

            info->counter.is_byte_len =
                (((long) fi->_template[fi->cur_no].data & 0xFF)) == FmCounterByte;
            offset = ((long) fi->_template[fi->cur_no].data) >> 8;
            iter_idx = fi->cur_no + offset;
            if (fi->_template[iter_idx].type == ITER)
            {
                ExtraData d;
                ExtraDataRec dr;

                if ((d = ChainMgrGetExtraData (&fi->cm, iter_idx)) == NULL)
                {
                    dr.iter = IterInit (&fi->_template[iter_idx + 1], NO_VALUE);
                    d = ChainMgrSetData (&fi->cm, iter_idx, dr);
                }
                /*endif*/
                info->counter.iter = d->iter;
            }
            else
            {
                /* Should never reach here */
            }
            /*endif*/
        }
        /*endif*/
        fi->cur_no = _FrameInstIncrement (fi->_template, fi->cur_no);
        break;

    case BARRAY:
        if (info)
        {
            ExtraData d;

            if ((d = ChainMgrGetExtraData (&fi->cm, fi->cur_no)) == NULL)
                info->num = NO_VALUE;
            else
                info->num = d->num;
            /*endif*/
        }
        /*endif*/
        fi->cur_no = _FrameInstIncrement (fi->_template, fi->cur_no);
        break;

    case PADDING:
        if (info)
        {
            register int unit;
            register int number;
            register int size;
            register int i;

            unit = _UNIT ((long) fi->_template[fi->cur_no].data);
            number = _NUMBER ((long) fi->_template[fi->cur_no].data);

            i = fi->cur_no;
            size = 0;
            while (number > 0)
            {
                i = _FrameInstDecrement (fi->_template, i);
                size += _FrameInstGetItemSize (fi, i);
                number--;
            }
            /*endwhile*/
            info->num = (unit - (size%unit))%unit;
        }
        /*endif*/
        fi->cur_no = _FrameInstIncrement (fi->_template, fi->cur_no);
        break;

    case ITER:
        {
            ExtraData d;
            ExtraDataRec dr;
            XimFrameType sub_type;


            if ((d = ChainMgrGetExtraData (&fi->cm, fi->cur_no)) == NULL)
            {
                dr.iter = IterInit (&fi->_template[fi->cur_no + 1], NO_VALUE);
                d = ChainMgrSetData (&fi->cm, fi->cur_no, dr);
            }
            /*endif*/
            sub_type = IterGetNextType (d->iter, info);
            if (sub_type == EOL)
            {
                fi->cur_no = _FrameInstIncrement (fi->_template, fi->cur_no);
                ret_type = FrameInstGetNextType (fi, info);
            }
            else
            {
                ret_type = sub_type;
            }
            /*endif*/
        }
        break;

    case POINTER:
        {
            ExtraData d;
            ExtraDataRec dr;
            XimFrameType sub_type;

            if ((d = ChainMgrGetExtraData (&fi->cm, fi->cur_no)) == NULL)
            {
                dr.fi = FrameInstInit (fi->_template[fi->cur_no + 1].data);
                d = ChainMgrSetData (&fi->cm, fi->cur_no, dr);
            }
            /*endif*/
            sub_type = FrameInstGetNextType (d->fi, info);
            if (sub_type == EOL)
            {
                fi->cur_no = _FrameInstIncrement (fi->_template, fi->cur_no);
                ret_type = FrameInstGetNextType (fi, info);
            }
            else
            {
                ret_type = sub_type;
            }
            /*endif*/
        }
        break;
    default:
	break;
    }
    /*endswitch*/
    return ret_type;
}

static XimFrameType FrameInstPeekNextType (FrameInst fi, XimFrameTypeInfo info)
{
    XimFrameType ret_type;

    ret_type = fi->_template[fi->cur_no].type;

    switch (ret_type)
    {
    case BIT8:
    case BIT16:
    case BIT32:
    case BIT64:
    case EOL:
        break;

    case COUNTER_BIT8:
    case COUNTER_BIT16:
    case COUNTER_BIT32:
    case COUNTER_BIT64:
        if (info)
        {
            register int offset;
	    register int iter_idx;

            info->counter.is_byte_len =
                (((long) fi->_template[fi->cur_no].data) & 0xFF) == FmCounterByte;
            offset = ((long)fi->_template[fi->cur_no].data) >> 8;
            iter_idx = fi->cur_no + offset;
            if (fi->_template[iter_idx].type == ITER)
            {
                ExtraData d;
                ExtraDataRec dr;

                if ((d = ChainMgrGetExtraData (&fi->cm, iter_idx)) == NULL)
                {
                    dr.iter = IterInit (&fi->_template[iter_idx + 1], NO_VALUE);
                    d = ChainMgrSetData (&fi->cm, iter_idx, dr);
                }
                /*endif*/
                info->counter.iter = d->iter;
            }
            else
            {
                /* Should not be reached here */
            }
            /*endif*/
        }
        /*endif*/
        break;

    case BARRAY:
        if (info)
        {
            ExtraData d;

            if ((d = ChainMgrGetExtraData (&fi->cm, fi->cur_no)) == NULL)
                info->num = NO_VALUE;
            else
                info->num = d->num;
            /*endif*/
        }
        /*endif*/
        break;

    case PADDING:
        if (info)
        {
            register int unit;
            register int number;
            register int size;
            register int i;

            unit = _UNIT ((long) fi->_template[fi->cur_no].data);
            number = _NUMBER ((long) fi->_template[fi->cur_no].data);

            i = fi->cur_no;
            size = 0;
            while (number > 0)
            {
                i = _FrameInstDecrement (fi->_template, i);
                size += _FrameInstGetItemSize (fi, i);
                number--;
            }
            /*endwhile*/
            info->num = (unit - (size%unit))%unit;
        }
        /*endif*/
        break;

    case ITER:
        {
            ExtraData d;
            ExtraDataRec dr;
            XimFrameType sub_type;

            if ((d = ChainMgrGetExtraData (&fi->cm, fi->cur_no)) == NULL)
            {
                dr.iter = IterInit (&fi->_template[fi->cur_no + 1], NO_VALUE);
                d = ChainMgrSetData (&fi->cm, fi->cur_no, dr);
            }
            /*endif*/
            sub_type = IterPeekNextType (d->iter, info);
            if (sub_type == EOL)
                ret_type = FrameInstPeekNextType (fi, info);
            else
                ret_type = sub_type;
            /*endif*/
        }
        break;

    case POINTER:
        {
            ExtraData d;
            ExtraDataRec dr;
            XimFrameType sub_type;

            if ((d = ChainMgrGetExtraData (&fi->cm, fi->cur_no)) == NULL)
            {
                dr.fi = FrameInstInit (fi->_template[fi->cur_no + 1].data);
                d = ChainMgrSetData (&fi->cm, fi->cur_no, dr);
            }
            /*endif*/
            sub_type = FrameInstPeekNextType (d->fi, info);
            if (sub_type == EOL)
                ret_type = FrameInstPeekNextType (fi, info);
            else
                ret_type = sub_type;
            /*endif*/
	default:
	    break;
        }
        break;
    }
    /*endswitch*/
    return ret_type;
}

static Bool FrameInstIsIterLoopEnd (FrameInst fi)
{
    Bool ret = False;

    if (fi->_template[fi->cur_no].type == ITER)
    {
        ExtraData d = ChainMgrGetExtraData (&fi->cm, fi->cur_no);
        Bool yourself;

        if (d)
        {
            ret = IterIsLoopEnd (d->iter, &yourself);
            if (ret  &&  yourself)
                fi->cur_no = _FrameInstIncrement (fi->_template, fi->cur_no);
            /*endif*/
        }
        /*endif*/
    }
    /*endif*/
    return (ret);
}

static FrameIter _FrameMgrAppendIter (FrameMgr fm, Iter it, int end)
{
    FrameIter p = fm->iters;

    while (p  &&  p->next)
        p = p->next;
    /*endwhile*/
    
    if (!p)
    {
        fm->iters =
        p = (FrameIter) Xmalloc (sizeof (FrameIterRec));
    }
    else
    {
        p->next = (FrameIter) Xmalloc (sizeof (FrameIterRec));
        p = p->next;
    }
    /*endif*/
    if (p)
    {
        p->iter = it;
        p->counting = False;
        p->counter = 0;
        p->end = end;
        p->next = NULL;
    }
    /*endif*/
    return (p);
}

static void _FrameMgrRemoveIter (FrameMgr fm, FrameIter it)
{
    FrameIter prev;
    FrameIter p;

    prev = NULL;
    p = fm->iters;
    while (p)
    {
        if (p == it)
        {
            if (prev)
                prev->next = p->next;
            else
                fm->iters = p->next;
            /*endif*/
            Xfree (p);
            break;
        }
        /*endif*/
        prev = p;
        p = p->next;
    }
    /*endwhile*/
}

static FrameIter _FrameIterCounterIncr (FrameIter fitr, int i)
{
    FrameIter p = fitr;

    while (p)
    {
        if (p->counting)
        {
            p->counter += i;
            if (p->counter >= p->end)
            {
                IterFixIteration (p->iter);
                return (p);
            }
            /*endif*/
        }
        /*endif*/
        p = p->next;
    }
    /*endwhile*/
    return (NULL);
}

static void _IterStartWatch (Iter it, void *client_data)
{
    FrameIter p = (FrameIter) client_data;
    p->counting = True;
}

static FmStatus FrameInstSetSize (FrameInst fi, int num)
{
    ExtraData d;
    ExtraDataRec dr;
    XimFrameType type;
    register int i;

    i = 0;
    while ((type = fi->_template[i].type) != EOL)
    {
        switch (type)
        {
        case BARRAY:
            if ((d = ChainMgrGetExtraData (&fi->cm, i)) == NULL)
            {
                dr.num = -1;
                d = ChainMgrSetData (&fi->cm, i, dr);
            }
            /*endif*/
            if (d->num == NO_VALUE)
            {
                d->num = num;
                return FmSuccess;
            }
            /*endif*/
            break;
        case ITER:
            if ((d = ChainMgrGetExtraData (&fi->cm, i)) == NULL)
            {
                dr.iter = IterInit (&fi->_template[i + 1], NO_VALUE);
                d = ChainMgrSetData (&fi->cm, i, dr);
            }
            /*endif*/
            if (IterSetSize (d->iter, num) == FmSuccess)
                return FmSuccess;
            /*endif*/
            break;
            
        case POINTER:
            if ((d = ChainMgrGetExtraData(&fi->cm, i)) == NULL)
            {
                dr.fi = FrameInstInit(fi->_template[i + 1].data);
                d = ChainMgrSetData(&fi->cm, i, dr);
            }
            /*endif*/
            if (FrameInstSetSize(d->fi, num) == FmSuccess)
                return FmSuccess;
            /*endif*/
            break;
	default:
	    break;
        }
        /*endswitch*/
        i = _FrameInstIncrement(fi->_template, i);
    }
    /*endwhile*/
    return FmNoMoreData;
}

static int FrameInstGetSize (FrameInst fi)
{
    XimFrameType type;
    register int i;
    ExtraData d;
    ExtraDataRec dr;
    int ret_size;

    i = fi->cur_no;
    while ((type = fi->_template[i].type) != EOL)
    {
        switch (type)
        {
        case BARRAY:
            if ((d = ChainMgrGetExtraData (&fi->cm, i)) == NULL)
                return NO_VALUE;
            /*endif*/
            return d->num;

        case ITER:
            if ((d = ChainMgrGetExtraData (&fi->cm, i)) == NULL)
            {
                dr.iter = IterInit (&fi->_template[i + 1], NO_VALUE);
                d = ChainMgrSetData (&fi->cm, i, dr);
            }
            /*endif*/
            ret_size = IterGetSize(d->iter);
            if (ret_size != NO_VALID_FIELD)
                return ret_size;
            /*endif*/
            break;
            
        case POINTER:
            if ((d = ChainMgrGetExtraData (&fi->cm, i)) == NULL)
            {
                dr.fi = FrameInstInit (fi->_template[i + 1].data);
                d = ChainMgrSetData (&fi->cm, i, dr);
            }
            /*endif*/
            ret_size = FrameInstGetSize (d->fi);
            if (ret_size != NO_VALID_FIELD)
                return ret_size;
            /*endif*/
            break;
	default:
	    break;
        }
        /*endswitch*/
        i = _FrameInstIncrement (fi->_template, i);
    }
    /*endwhile*/
    return NO_VALID_FIELD;
}

static FmStatus FrameInstSetIterCount (FrameInst fi, int num)
{
    ExtraData d;
    ExtraDataRec dr;
    register int i;
    XimFrameType type;

    i = 0;
    while ((type = fi->_template[i].type) != EOL)
    {
        switch (type)
        {
        case ITER:
            if ((d = ChainMgrGetExtraData (&fi->cm, i)) == NULL)
            {
                dr.iter = IterInit (&fi->_template[i + 1], num);
                (void)ChainMgrSetData (&fi->cm, i, dr);
                return FmSuccess;
            }
            /*endif*/
            if (IterSetIterCount (d->iter, num) == FmSuccess)
                return FmSuccess;
            /*endif*/
            break;
            
        case POINTER:
            if ((d = ChainMgrGetExtraData (&fi->cm, i)) == NULL)
            {
                dr.fi = FrameInstInit (fi->_template[i + 1].data);
                d = ChainMgrSetData (&fi->cm, i, dr);
            }
            /*endif*/
            if (FrameInstSetIterCount (d->fi, num) == FmSuccess)
                return FmSuccess;
            /*endif*/
            break;

	default:
	    break;
        }
        /*endswitch*/
        i = _FrameInstIncrement (fi->_template, i);
    }
    /*endwhile*/
    return FmNoMoreData;
}

static int FrameInstGetTotalSize (FrameInst fi)
{
    register int size;
    register int i;

    size = 0;
    i = 0;

    while (fi->_template[i].type != EOL)
    {
        size += _FrameInstGetItemSize (fi, i);
        i = _FrameInstIncrement (fi->_template, i);
    }
    /*endwhile*/
    return size;
}

static void FrameInstReset (FrameInst fi)
{
    ChainIterRec ci;
    int frame_no;
    ExtraDataRec d;

    ChainIterInit (&ci, &fi->cm);

    while (ChainIterGetNext (&ci, &frame_no, &d))
    {
        register XimFrameType type;
        type = fi->_template[frame_no].type;
        if (type == ITER)
        {
            if (d.iter)
                IterReset (d.iter);
            /*endif*/
        }
        else if (type == POINTER)
        {
            if (d.fi)
                FrameInstReset (d.fi);
            /*endif*/
        }
        /*endif*/
    }
    /*endwhile*/
    ChainIterFree (&ci);

    fi->cur_no = 0;
}

static Iter IterInit (XimFrame frame, int count)
{
    Iter it;
    register XimFrameType type;

    it = (Iter) Xmalloc (sizeof (IterRec));
    it->_template = frame;
    it->max_count = (count == NO_VALUE)  ?  0  :  count;
    it->allow_expansion = (count == NO_VALUE);
    it->cur_no = 0;
    it->start_watch_proc = NULL;
    it->client_data = NULL;
    it->start_counter = False;

    type = frame->type;
    if (type & COUNTER_MASK)
    {
        /* COUNTER_XXX cannot be an item of a ITER */
        Xfree (it);
        return NULL;
    }
    /*endif*/

    switch (type)
    {
    case BIT8:
    case BIT16:
    case BIT32:
    case BIT64:
        /* Do nothing */
        break;
        
    case BARRAY:
    case ITER:
    case POINTER:
        ChainMgrInit (&it->cm);
        break;
        
    default:
        Xfree (it);
        return NULL; /* This should never occur */
    }
    /*endswitch*/
    return it;
}

static void IterFree (Iter it)
{
    switch (it->_template->type)
    {
    case BARRAY:
        ChainMgrFree (&it->cm);
        break;
    
    case ITER:
        {
            ChainIterRec ci;
            int count;
            ExtraDataRec d;

            ChainIterInit (&ci, &it->cm);
            while (ChainIterGetNext (&ci, &count, &d))
                IterFree (d.iter);
            /*endwhile*/
            ChainIterFree (&ci);
            ChainMgrFree (&it->cm);
        }
        break;
    
    case POINTER:
        {
            ChainIterRec ci;
            int count;
            ExtraDataRec dr;
    
            ChainIterInit (&ci, &it->cm);
            while (ChainIterGetNext (&ci, &count, &dr))
                FrameInstFree (dr.fi);
            /*endwhile*/
            ChainIterFree (&ci);
            ChainMgrFree (&it->cm);
        }
        break;

    default:
	break;
    }
    /*endswitch*/
    Xfree (it);
}

static Bool IterIsLoopEnd (Iter it, Bool *myself)
{
    Bool ret = False;
    *myself = False;

    if (!it->allow_expansion  &&  (it->cur_no == it->max_count))
    {
        *myself = True;
        return True;
    }
    /*endif*/
    
    if (it->_template->type == POINTER)
    {
        ExtraData d = ChainMgrGetExtraData (&it->cm, it->cur_no);
        if (d)
        {
            if (FrameInstIsIterLoopEnd (d->fi))
            {
                ret = True;
            }
            else
            {
                if (FrameInstIsEnd (d->fi))
                {
                    it->cur_no++;
                    if (!it->allow_expansion  &&  it->cur_no == it->max_count)
                    {
                        *myself = True;
                        ret = True;
                    }
                    /*endif*/
                }
                /*endif*/
            }
            /*endif*/
        }
        /*endif*/
    }
    else if (it->_template->type == ITER)
    {
        ExtraData d = ChainMgrGetExtraData (&it->cm, it->cur_no);
        if (d)
        {
            Bool yourself;
            
            if (IterIsLoopEnd (d->iter, &yourself))
                ret = True;
            /*endif*/
        }
        /*endif*/
    }
    /*endif*/

    return ret;
}

static XimFrameType IterGetNextType (Iter it, XimFrameTypeInfo info)
{
    XimFrameType type = it->_template->type;

    if (it->start_counter)
    {
        (*it->start_watch_proc) (it, it->client_data);
        it->start_counter = False;
    }
    /*endif*/
    if (it->cur_no >= it->max_count)
    {
        if (it->allow_expansion)
            it->max_count = it->cur_no + 1;
        else
            return EOL;
        /*endif*/
    }
    /*endif*/

    switch (type)
    {
    case BIT8:
    case BIT16:
    case BIT32:
    case BIT64:
        it->cur_no++;
        return type;

    case BARRAY:
        if (info)
        {
            ExtraData d;
    
            if ((d = ChainMgrGetExtraData (&it->cm, it->cur_no)) == NULL)
                info->num = NO_VALUE;
            else
                info->num = d->num;
            /*endif*/
        }
        /*endif*/
        it->cur_no++;
        return BARRAY;

    case ITER:
        {
            XimFrameType ret_type;
            ExtraData d;
            ExtraDataRec dr;

            if ((d = ChainMgrGetExtraData (&it->cm, it->cur_no)) == NULL)
            {
                dr.iter = IterInit (it->_template + 1, NO_VALUE);
                d = ChainMgrSetData (&it->cm, it->cur_no, dr);
            }
            /*endif*/

            ret_type = IterGetNextType (d->iter, info);
            if (ret_type == EOL)
            {
                it->cur_no++;
                ret_type = IterGetNextType (it, info);
            }
            /*endif*/
	    return ret_type;
        }

    case POINTER:
        {
            XimFrameType ret_type;
            ExtraData d;
            ExtraDataRec dr;

            if ((d = ChainMgrGetExtraData (&it->cm, it->cur_no)) == NULL)
            {
                dr.fi = FrameInstInit (it->_template[1].data);
                d = ChainMgrSetData (&it->cm, it->cur_no, dr);
            }
            /*endif*/

            ret_type = FrameInstGetNextType (d->fi, info);
            if (ret_type == EOL)
            {
                it->cur_no++;
                ret_type = IterGetNextType (it, info);
            }
            /*endif*/
	    return ret_type;
        }

    default:
	return (XimFrameType) NULL;
    }
    /*endswitch*/
    return (XimFrameType) NULL;  /* This should never occur */
}

static XimFrameType IterPeekNextType (Iter it, XimFrameTypeInfo info)
{
    XimFrameType type = it->_template->type;

    if (!it->allow_expansion  &&  it->cur_no >= it->max_count)
        return (EOL);
    /*endif*/
    
    switch (type)
    {
    case BIT8:
    case BIT16:
    case BIT32:
    case BIT64:
        return type;

    case BARRAY:
        if (info)
        {
            ExtraData d;

            if ((d = ChainMgrGetExtraData (&it->cm, it->cur_no)) == NULL)
                info->num = NO_VALUE;
            else
                info->num = d->num;
            /*endif*/
        }
        /*endif*/
        return BARRAY;
    
    case ITER:
        {
            XimFrameType ret_type;
            ExtraData d;
            ExtraDataRec dr;

            if ((d = ChainMgrGetExtraData (&it->cm, it->cur_no)) == NULL)
            {
                dr.iter = IterInit (it->_template + 1, NO_VALUE);
                d = ChainMgrSetData (&it->cm, it->cur_no, dr);
            }
            /*endif*/

            ret_type = IterPeekNextType (d->iter, info);
            if (ret_type == EOL)
                ret_type = IterPeekNextType (it, info);
            /*endif*/
            return ret_type;
        }
        
    case POINTER:
        {
            XimFrameType ret_type;
            ExtraData d;
            ExtraDataRec dr;

            if ((d = ChainMgrGetExtraData (&it->cm, it->cur_no)) == NULL)
            {
                dr.fi = FrameInstInit (it->_template[1].data);
                d = ChainMgrSetData (&it->cm, it->cur_no, dr);
            }
            /*endif*/

            ret_type = FrameInstPeekNextType (d->fi, info);
            if (ret_type == EOL)
                ret_type = IterPeekNextType (it, info);
            /*endif*/
            return (ret_type);
        }

    default:
	break;
    }
    /*endswitch*/
    /* Reaching here is a bug! */
    return (XimFrameType) NULL;
}

static FmStatus IterSetSize (Iter it, int num)
{
    XimFrameType type;
    register int i;

    if (!it->allow_expansion  &&  it->max_count == 0)
        return FmNoMoreData;
    /*endif*/
    
    type = it->_template->type;
    switch (type)
    {
    case BARRAY:
        {
            ExtraData d;
            ExtraDataRec dr;

            for (i = 0;  i < it->max_count;  i++)
            {
                if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
                {
                    dr.num = NO_VALUE;
                    d = ChainMgrSetData (&it->cm, i, dr);
                }
                /*endif*/
                if (d->num == NO_VALUE)
                {
                    d->num = num;
                    return FmSuccess;
                }
                /*endif*/
            }
            /*endfor*/
            if (it->allow_expansion)
            {
                ExtraDataRec dr;
                
                dr.num = num;
                ChainMgrSetData (&it->cm, it->max_count, dr);
                it->max_count++;
    
                return FmSuccess;
            }
            /*endif*/
        }   
        return FmNoMoreData;

    case ITER:
        {
            ExtraData d;
            ExtraDataRec dr;

            for (i = 0;  i < it->max_count;  i++)
            {
                if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
                {
                    dr.iter = IterInit (it->_template + 1, NO_VALUE);
                    d = ChainMgrSetData (&it->cm, i, dr);
                }
                /*endif*/
                if (IterSetSize (d->iter, num) == FmSuccess)
                    return FmSuccess;
                /*endif*/
            }
            /*endfor*/
            if (it->allow_expansion)
            {
                ExtraDataRec dr;

                dr.iter = IterInit (it->_template + 1, NO_VALUE);
                ChainMgrSetData (&it->cm, it->max_count, dr);
                it->max_count++;

                if (IterSetSize(dr.iter, num) == FmSuccess)
                    return FmSuccess;
                /*endif*/
            }
            /*endif*/
        }
        return FmNoMoreData;

    case POINTER:
        {
            ExtraData d;
            ExtraDataRec dr;

            for (i = 0;  i < it->max_count;  i++)
            {
                if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
                {
                    dr.fi = FrameInstInit (it->_template[1].data);
                    d = ChainMgrSetData (&it->cm, i, dr);
                }
                /*endif*/
                if (FrameInstSetSize (d->fi, num) == FmSuccess)
                    return FmSuccess;
                /*endif*/
            }
            /*endfor*/
            if (it->allow_expansion)
            {
                ExtraDataRec dr;

                dr.fi = FrameInstInit (it->_template[1].data);
                ChainMgrSetData (&it->cm, it->max_count, dr);
                it->max_count++;

                if (FrameInstSetSize (dr.fi, num) == FmSuccess)
                    return FmSuccess;
                /*endif*/
            }
            /*endif*/
        }
        return FmNoMoreData;

    default:
	break;
    }
    /*endswitch*/
    return FmNoMoreData;
}

static int IterGetSize (Iter it)
{
    register int i;
    ExtraData d;
    ExtraDataRec dr;

    if (it->cur_no >= it->max_count)
        return NO_VALID_FIELD;
    /*endif*/
    
    switch (it->_template->type)
    {
    case BARRAY:
        if ((d = ChainMgrGetExtraData (&it->cm, it->cur_no)) == NULL)
            return NO_VALUE;
        /*endif*/
        return d->num;

    case ITER:
        for (i = it->cur_no; i < it->max_count; i++)
        {
            int ret_size;

            if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
            {
                dr.iter = IterInit (it->_template + 1, NO_VALUE);
                d = ChainMgrSetData (&it->cm, i, dr);
            }
            /*endif*/
            ret_size = IterGetSize (d->iter);
            if (ret_size != NO_VALID_FIELD)
                return ret_size;
            /*endif*/
        }
        /*endfor*/
        return NO_VALID_FIELD;
    
    case POINTER:
        for (i = it->cur_no;  i < it->max_count;  i++)
        {
            int ret_size;
            
            if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
            {
                dr.fi = FrameInstInit (it->_template[1].data);
                d = ChainMgrSetData (&it->cm, i, dr);
            }
            /*endif*/
            ret_size = FrameInstGetSize (d->fi);
            if (ret_size != NO_VALID_FIELD)
                return ret_size;
            /*endif*/
        }
        /*endfor*/
        return NO_VALID_FIELD;

    default:
	break;
    }
    /*endswitch*/
    return NO_VALID_FIELD;
}

static FmStatus IterSetIterCount (Iter it, int num)
{
    register int i;

    if (it->allow_expansion)
    {
        it->max_count = num;
        it->allow_expansion = False;
        return FmSuccess;
    }
    /*endif*/

    if (it->max_count == 0)
        return FmNoMoreData;
    /*endif*/

    switch (it->_template->type)
    {
    case ITER:
        for (i = 0;  i < it->max_count;  i++)
        {
            ExtraData d;
            ExtraDataRec dr;

            if ((d = ChainMgrGetExtraData(&it->cm, i)) == NULL)
            {
                dr.iter = IterInit(it->_template + 1, num);
                (void)ChainMgrSetData(&it->cm, i, dr);
                return FmSuccess;
            }
            /*endif*/
            if (IterSetIterCount(d->iter, num) == FmSuccess)
                return FmSuccess;
            /*endif*/
        }
        /*endfor*/
        if (it->allow_expansion)
        {
            ExtraDataRec dr;

            dr.iter = IterInit (it->_template + 1, num);
            ChainMgrSetData (&it->cm, it->max_count, dr);
            it->max_count++;

            return FmSuccess;
        }
        /*endif*/
        break;
    
    case POINTER:
        for (i = 0;  i < it->max_count;  i++)
        {
            ExtraData d;
            ExtraDataRec dr;
            
            if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
            {
                dr.fi = FrameInstInit (it->_template[1].data);
                d = ChainMgrSetData (&it->cm, i, dr);
            }
            /*endif*/
            if (FrameInstSetIterCount (d->fi, num) == FmSuccess)
                return FmSuccess;
            /*endif*/
        }
        /*endfor*/
        if (it->allow_expansion)
        {
            ExtraDataRec dr;
            
            dr.fi = FrameInstInit (it->_template[1].data);
            ChainMgrSetData (&it->cm, it->max_count, dr);
            it->max_count++;

            if (FrameInstSetIterCount (dr.fi, num) == FmSuccess)
                return FmSuccess;
            /*endif*/
        }
        /*endif*/
        break;

    default:
	break;
    }
    /*endswitch*/
    return FmNoMoreData;
}

static int IterGetTotalSize (Iter it)
{
    register int size, i;
    XimFrameType type;

    if (it->allow_expansion)
        return NO_VALUE;
    /*endif*/
    if (it->max_count == 0)
        return 0;
    /*endif*/

    size = 0;
    type = it->_template->type;

    switch (type)
    {
    case BIT8:
        size = it->max_count;
        break;

    case BIT16:
        size = it->max_count*2;
        break;

    case BIT32:
        size = it->max_count*4;
        break;

    case BIT64:
        size = it->max_count*8;
        break;

    case BARRAY:
        for (i = 0;  i < it->max_count;  i++)
        {
            register int num;
            ExtraData d;
            
            if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
                return  NO_VALUE;
            /*endif*/
            if ((num = d->num) == NO_VALUE)
                return  NO_VALUE;
            /*endif*/
            size += num;
        }
        /*endfor*/
        break;
        
    case ITER:
        for (i = 0;  i < it->max_count;  i++)
        {
            register int num;
            ExtraData d;
            
            if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
                return  NO_VALUE;
            /*endif*/
            if ((num = IterGetTotalSize (d->iter)) == NO_VALUE)
                return  NO_VALUE;
            /*endif*/
            size += num;
        }
        /*endfor*/
        break;
        
    case POINTER:
        for (i = 0;  i < it->max_count;  i++)
        {
            register int num;
            ExtraData d;
            ExtraDataRec dr;
            
            if ((d = ChainMgrGetExtraData (&it->cm, i)) == NULL)
            {
                dr.fi = FrameInstInit (it->_template[1].data);
                d = ChainMgrSetData (&it->cm, i, dr);
            }
            /*endif*/
            if ((num = FrameInstGetTotalSize (d->fi)) == NO_VALUE)
                return NO_VALUE;
            /*endif*/
            size += num;
        }
        /*endfor*/
        break;

    default:
	break;
    }
    /*endswitch*/
    return  size;
}

static void IterReset (Iter it)
{
    ChainIterRec ci;
    int count;
    ExtraDataRec d;

    switch (it->_template->type)
    {
    case ITER:
        ChainIterInit (&ci, &it->cm);
        while (ChainIterGetNext (&ci, &count, &d))
            IterReset (d.iter);
        /*endwhile*/
        ChainIterFree (&ci);
        break;
  
    case POINTER:
        ChainIterInit (&ci, &it->cm);
        while (ChainIterGetNext (&ci, &count, &d))
            FrameInstReset (d.fi);
        /*endwhile*/
        ChainIterFree (&ci);
        break;

    default:
	break;
    }
    /*endswitch*/
    it->cur_no = 0;
}

static void IterSetStartWatch (Iter it,
                               IterStartWatchProc proc,
                               void *client_data)
{
    it->start_watch_proc = proc;
    it->client_data = client_data;
}

static ExtraData ChainMgrSetData (ChainMgr cm,
                                  int frame_no,
                                  ExtraDataRec data)
{
    Chain cur = (Chain) Xmalloc (sizeof (ChainRec));

    cur->frame_no = frame_no;
    cur->d = data;
    cur->next = NULL;

    if (cm->top == NULL)
    {
        cm->top = cm->tail = cur;
    }
    else
    {
        cm->tail->next = cur;
        cm->tail = cur;
    }
    /*endif*/
    return &cur->d;
}

static ExtraData ChainMgrGetExtraData (ChainMgr cm, int frame_no)
{
    Chain cur;

    cur = cm->top;

    while (cur)
    {
        if (cur->frame_no == frame_no)
            return &cur->d;
        /*endif*/
        cur = cur->next;
    }
    /*endwhile*/
    return NULL;
}

static Bool ChainIterGetNext (ChainIter ci, int *frame_no, ExtraData d)
{
    if (ci->cur == NULL)
        return False;
    /*endif*/
    
    *frame_no = ci->cur->frame_no;
    *d = ci->cur->d;

    ci->cur = ci->cur->next;

    return True;
}

static int _FrameInstIncrement (XimFrame frame, int count)
{
    XimFrameType type;

    type = frame[count].type;
    type &= ~COUNTER_MASK;

    switch (type)
    {
    case BIT8:
    case BIT16:
    case BIT32:
    case BIT64:
    case BARRAY:
    case PADDING:
        return count + 1;
        
    case POINTER:
        return count + 2;
        
    case ITER:
        return _FrameInstIncrement (frame, count + 1);
    default:
	break;
    }
    /*endswitch*/
    return - 1;    /* Error */
}

static int _FrameInstDecrement (XimFrame frame, int count)
{
    register int i;
    XimFrameType type;

    if (count == 0)
        return - 1;    /* cannot decrement */
    /*endif*/
    
    if (count == 1)
        return 0;     /* BOGUS - It should check the contents of data */
    /*endif*/
    
    type = frame[count - 2].type;
    type &= ~COUNTER_MASK;

    switch (type)
    {
    case BIT8:
    case BIT16:
    case BIT32:
    case BIT64:
    case BARRAY:
    case PADDING:
    case PTR_ITEM:
        return count - 1;

    case POINTER:
    case ITER:
        i = count - 3;
        while (i >= 0)
        {
            if (frame[i].type != ITER)
                return i + 1;
            /*endif*/
            i--;
        }
        /*endwhile*/
        return 0;
    default:
	break;
    }
    /*enswitch*/
    return - 1;    /* Error */
}

static int _FrameInstGetItemSize (FrameInst fi, int cur_no)
{
    XimFrameType type;

    type = fi->_template[cur_no].type;
    type &= ~COUNTER_MASK;

    switch (type)
    {
    case BIT8:
        return 1;

    case BIT16:
        return 2;

    case BIT32:
        return 4;

    case BIT64:
        return 8;

    case BARRAY:
        {
            ExtraData d;

            if ((d = ChainMgrGetExtraData (&fi->cm, cur_no)) == NULL)
                return NO_VALUE;
            /*endif*/
            if (d->num == NO_VALUE)
                return NO_VALUE;
            /*endif*/
            return d->num;
        }

    case PADDING:
        {
            register int unit;
            register int number;
            register int size;
            register int i;

            unit = _UNIT ((long) fi->_template[cur_no].data);
            number = _NUMBER ((long) fi->_template[cur_no].data);

            i = cur_no;
            size = 0;
            while (number > 0)
            {
                i = _FrameInstDecrement (fi->_template, i);
                size += _FrameInstGetItemSize (fi, i);
                number--;
            }
            /*endwhile*/
            size = (unit - (size%unit))%unit;
            return size;
        }

    case ITER:
        {
            ExtraData d;
            int sub_size;

            if ((d = ChainMgrGetExtraData (&fi->cm, cur_no)) == NULL)
                return NO_VALUE;
            /*endif*/
            sub_size = IterGetTotalSize (d->iter);
            if (sub_size == NO_VALUE)
                return NO_VALUE;
            /*endif*/
            return sub_size;
        }

    case POINTER:
        {
            ExtraData d;
            int sub_size;

            if ((d = ChainMgrGetExtraData (&fi->cm, cur_no)) == NULL)
                return NO_VALUE;
            /*endif*/
            sub_size = FrameInstGetTotalSize (d->fi);
            if (sub_size == NO_VALUE)
                return NO_VALUE;
            /*endif*/
            return sub_size;
        }

    default:
	break;
    }
    /*endswitch*/
    return NO_VALUE;
}
