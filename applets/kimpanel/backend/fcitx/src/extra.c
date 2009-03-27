/*!
 * \file extra.c
 * \author dgod
 */

#include "extra.h"
#include "InputWindow.h"

#include <dlfcn.h>
#include <limits.h>
#include <iconv.h>

#define EIM_MAX		4
extern Display *dpy;

iconv_t convGB=(iconv_t)-1;

static EXTRA_IM *EIMS[EIM_MAX];
static void *EIM_handle[EIM_MAX];
static char EIM_file[EIM_MAX][PATH_MAX];
static INT8 EIM_index[EIM_MAX];

static char CandTableEngine[10][MAX_CAND_LEN+1];
static char CodeTipsEngine[10][MAX_TIPS_LEN+1];
static char StringGetEngine[MAX_CAND_LEN+1];

extern INT8 iIMCount;
extern INT8 iIMIndex;
extern INT8 iInCap;

extern MESSAGE  messageUp[];
extern uint     uMessageUp;
extern MESSAGE  messageDown[];
extern uint     uMessageDown;
extern Bool	bPointAfterNumber;
extern int      iCursorPos;

extern int      iMaxCandWord;
extern int      iCandWordCount;
extern int      iCandPageCount;
extern int	iCurrentCandPage;
extern char     strStringGet[];
extern char     strCodeInput[];
extern int      iCodeInputCount;
extern Bool     bShowCursor;
extern Bool	bCursorAuto;

static void ResetAll(void)
{
	int i;
	for(i=0;i<EIM_MAX;i++)
	{
		if(EIMS[i])
		{
			if(EIMS[i]->Destroy)
				EIMS[i]->Destroy();
			EIMS[i]=NULL;
		}
		if(EIM_handle[i])
		{
			dlclose(EIM_handle[i]);
			EIM_handle[i]=NULL;
		}
		if(EIM_file[i][0])
		{
			EIM_file[i][0]=0;
		}
		EIM_index[i]=0;
	}
}

static void ExtraReset(void)
{
	int i;
	EXTRA_IM *eim=NULL;
	for(i=0;i<EIM_MAX;i++)
	{
		if(EIM_index[i]==iIMIndex)
		{
			eim=EIMS[i];
			break;
		}
	}
	bShowCursor=False;
	bCursorAuto=False;
	if(!eim) return;
	if(eim->CandWordMax) eim->CandWordMax=iMaxCandWord;
	if(eim->Reset) eim->Reset();
	eim->StringGet[0]=0;
	eim->CodeInput[0]=0;
	eim->CaretPos=-1;
	eim->CandWordMax=iMaxCandWord;
	eim->Reset();
}

static void DisplayEIM(EXTRA_IM *im)
{
	int i;
	char strTemp[3];

	if ( bPointAfterNumber )
	{
		strTemp[1] = '.';
		strTemp[2] = '\0';
	}
	else strTemp[1]='\0';

	uMessageDown = 0;
	for (i = 0; i < im->CandWordCount; i++)
	{
		strTemp[0] = i + 1 + '0';
		if (i == 9) strTemp[0] = '0';
		strcpy(messageDown[uMessageDown].strMsg, strTemp);
		messageDown[uMessageDown++].type = MSG_INDEX;	
		
		strcpy(messageDown[uMessageDown].strMsg, im->CandTable[i]);
		messageDown[uMessageDown].type = (i!=im->SelectIndex)? MSG_OTHER:MSG_FIRSTCAND;
		if(im->CodeTips && im->CodeTips[i] && im->CodeTips[i][0])
		{
			uMessageDown++;
			strcpy(messageDown[uMessageDown].strMsg,im->CodeTips[i]);
			messageDown[uMessageDown].type=MSG_CODE;
		}
		if (i != 9)
			strcat (messageDown[uMessageDown].strMsg, " ");
		uMessageDown++;
	}    

	uMessageUp=0;   
	if(im->StringGet[0] || im->CodeInput[0])
	{ 
		uMessageUp = 1;	
		strcpy (messageUp[0].strMsg, im->StringGet);
		strcat (messageUp[0].strMsg, im->CodeInput);
		messageUp[0].type = MSG_INPUT;

		bShowCursor=True;
		iCodeInputCount=strlen(im->CodeInput);
		if(im->CaretPos!=-1)
			iCursorPos=strlen(im->StringGet)+im->CaretPos;
		else
			iCursorPos=strlen(im->StringGet)+iCodeInputCount;
		bCursorAuto=True;
	}

	iCandWordCount=im->CandWordCount;
	iCandPageCount=im->CandPageCount;
	iCurrentCandPage=im->CurCandPage;
	if(iCandPageCount) iCandPageCount--;
}

static int ExtraKeyConv(int key)
{
	if(key == (XK_BackSpace & 0x00FF))
		key='\b';
	switch(key){
	case LEFT:key=VK_LEFT;break;
	case RIGHT:key=VK_RIGHT;break;
	case UP:key=VK_UP;break;
	case DOWN:key=VK_DOWN;break;
	case HOME:key=VK_HOME;break;
	case END:key=VK_END;break;
	case CTRL_INSERT:key=KEYM_CTRL|VK_INSERT;break;
	case CTRL_DELETE:key=KEYM_CTRL|VK_DELETE;break;
	case CTRL_0...CTRL_9:key=KEYM_CTRL|('0'+key-CTRL_0);break;
	case CTRL_A...CTRL_Z:key=KEYM_CTRL|('A'+key-CTRL_A);break;
	};
	return key;
}

static INPUT_RETURN_VALUE ExtraDoInput(int key)
{
	int i;
	EXTRA_IM *eim=NULL;
	INPUT_RETURN_VALUE ret=IRV_DO_NOTHING;
	for(i=0;i<EIM_MAX;i++)
	{
		if(EIM_index[i]==iIMIndex)
		{
			eim=EIMS[i];
			break;
		}
	}
	if(!eim) return IRV_TO_PROCESS;
	key=ExtraKeyConv(key);
	if(eim->DoInput)
		ret=eim->DoInput(key);
	if(ret==IRV_GET_CANDWORDS||ret==IRV_GET_CANDWORDS_NEXT)
	{
		strcpy(strStringGet,eim->StringGet);
		eim->StringGet[0]=0;
		if(ret==IRV_GET_CANDWORDS_NEXT)
		{
			DisplayEIM(eim);
		}
		else
		{
			uMessageDown=0;
			uMessageUp=0;
		}
	}
	else if(ret==IRV_DO_NOTHING && (eim->CandWordCount ||
		eim->StringGet[0] || eim->CodeInput[0]))
	{
		DisplayEIM(eim);
		ret=IRV_DISPLAY_CANDWORDS;
	}
	else if(ret==IRV_DISPLAY_CANDWORDS)
	{
		DisplayEIM(eim);
	}
	else if(ret==IRV_ENG)
	{
		iCodeInputCount=strlen(strCodeInput);
		iInCap=3;
		ret=IRV_DONOT_PROCESS;
	}
	else if(ret==IRV_TO_PROCESS)
	{
		if(key==ENTER)
		{
			iCodeInputCount=strlen(strCodeInput);
			strcpy(strStringGet,strCodeInput);
			uMessageDown=0;
			uMessageUp=0;
			ret=IRV_GET_CANDWORDS;
		}
		else if(key==' ')
		{
			if(!eim->CodeInput[0])
				return IRV_TO_PROCESS;
			strcpy(strStringGet,eim->GetCandWord(eim->SelectIndex));
			uMessageDown=0;
			uMessageUp=0;
			ret=IRV_GET_CANDWORDS;
		}
		else if(key>='0' && key<= '9')
		{
			int index;
			if(key=='0') index=9;
			else index=key-'0'-1;
			if(index>=eim->CandWordCount)
			{
				if(eim->CodeInput[0])
					return IRV_DO_NOTHING;
				else return IRV_TO_PROCESS;
			}
			strcpy(strStringGet,eim->GetCandWord(index));
			uMessageDown=0;
			uMessageUp=0;
			ret=IRV_GET_CANDWORDS;
		}
		else if(key==VK_UP)
		{
			if(eim->SelectIndex>0)
			{
				eim->SelectIndex=eim->SelectIndex-1;
				DisplayEIM(eim);
			}
			ret=IRV_DISPLAY_CANDWORDS;
		}
		else if(key==VK_DOWN)
		{
			if(eim->SelectIndex<eim->CandWordCount-1)
			{
				eim->SelectIndex=eim->SelectIndex+1;
				DisplayEIM(eim);
			}
			ret=IRV_DISPLAY_CANDWORDS;
		}
	}
 
	return ret;
}

static INPUT_RETURN_VALUE ExtraGetCandWords(SEARCH_MODE sm)
{
	int i;
	EXTRA_IM *eim=NULL;
	INPUT_RETURN_VALUE ret=IRV_DO_NOTHING;
	for(i=0;i<EIM_MAX;i++)
	{
		if(EIM_index[i]==iIMIndex)
		{
			eim=EIMS[i];
			break;
		}
	}
	if(!eim) return IRV_TO_PROCESS;
	if(eim->GetCandWords)
		ret=eim->GetCandWords(sm);
	if(ret==IRV_DISPLAY_CANDWORDS)
		DisplayEIM(eim);
	return ret;
}

static char *ExtraGetCandWord(int index)
{
	int i;
	EXTRA_IM *eim=NULL;
	for(i=0;i<EIM_MAX;i++)
	{
		if(EIM_index[i]==iIMIndex)
		{
			eim=EIMS[i];
			break;
		}
	}
	if(!eim) return 0;
	if(eim->GetCandWord)
	{
		uMessageDown=0;
		uMessageUp=0;
		return eim->GetCandWord(index);
	}
	return 0;
}

char *ExtraGetPath(char *type)
{
	static char ret[256];
	ret[0]=0;
	if(!strcmp(type,"HOME"))
	{
		sprintf(ret,"%s/.fcitx",getenv("HOME"));
	}
	else if(!strcmp(type,"DATA"))
	{
		strcpy(ret,PKGDATADIR"/data");
	}
	else if(!strcmp(type,"LIB"))
	{
		strcpy(ret,PKGDATADIR"/data");
	}
	else if(!strcmp(type,"BIN"))
	{
		strcpy(ret,PKGDATADIR"/../../bin");
	}
	return ret;
}

/* the result is slow, why? */
char *GetClipboardString(Display *disp)
{
	Atom sel;
	Window w;
	int ret;
	Atom target;
	Atom type;
	int fmt;
	unsigned long n,after;
	unsigned char *pret=0;
	static char result[1024];

	sel = XInternAtom(disp, "PRIMARY", 0);
	target = XInternAtom(disp,"UTF8_STRING",0);
	w=XGetSelectionOwner(disp,sel);
	if(w==None)
	{
		//printf("None owner\n");
		return NULL;
	}
	XConvertSelection(disp,sel,target,sel,w,CurrentTime);
	ret=XGetWindowProperty(disp,w,sel,0,1023,False,target,&type,&fmt,&n,&after,&pret);
	if(ret!=Success || !pret || fmt!=8)
	{
		//printf("Error %d\n",ret);
		return NULL;
	}
	memcpy(result,pret,n);
	XFree(pret);
	result[n]=0;
	return result;
}

char *ExtraGetSelect(void)
{
	static char phrase[32];
	char *f;
	size_t l1,l2;
	char *ps=phrase;
	f=GetClipboardString(dpy);
	if(!f) return NULL;
	l1=strlen(f);
	if(l1>=32) return NULL;
	l2=32;
	l1=iconv(convGB,&f,&l1,&ps,&l2);
	*ps=0;
	return phrase;
}

int InitExtraIM(EXTRA_IM *eim,char *arg)
{
	eim->CodeInput=strCodeInput;
	eim->StringGet=StringGetEngine;
	eim->CandTable=CandTableEngine;
	eim->CodeTips=CodeTipsEngine;
	eim->GetSelect=ExtraGetSelect;
	eim->GetPath=ExtraGetPath;
	eim->CandWordMax=iMaxCandWord;
	eim->CaretPos=-1;

	if(eim->Init(arg))
	{
		printf("eim: init fail\n");
		return -1;
	}

	return 0;
}

void LoadExtraIM(char *fn)
{
	void *handle;
	int i;
	EXTRA_IM *eim;
	char path[PATH_MAX];
	char temp[256];
	char *arg;
	char fnr[256];

	if(convGB==(iconv_t)-1)
	{
		convGB=iconv_open("GB18030","UTF-8");
		if(convGB==(iconv_t)-1)
			return;
	}
	
	for(i=0;i<EIM_MAX;i++)
	{
		if(EIM_file[i][0] && !strcmp(EIM_file[i],path))
		{
			ResetAll();
			break;
		}
	}
	for(i=0;i<EIM_MAX;i++)
	{
		if(!EIMS[i])
		{
			break;
		}
	}
	if(i==EIM_MAX)
	{
		printf("eim: no space\n");
		return;
	}

	strcpy(fnr,fn);
	arg=strstr(fnr,".so ");
	if(arg)
	{
		arg[3]=0;
		arg+=4;
	}
	if(fnr[0]=='~' && fnr[1]=='/')
		sprintf(temp,"%s/%s",getenv("HOME"),fnr+2);
	else if(strchr(fnr,'/'))
		strcpy(temp,fnr);
	else
		sprintf(temp,"%s/%s",ExtraGetPath("LIB"),fnr);

	handle=dlopen(temp,RTLD_LAZY);
 	
	if(!handle)
	{
		printf("eim: open %s fail %s\n",temp,dlerror());
		return;
	}
	eim=dlsym(handle,"EIM");
	if(!eim || !eim->Init)
	{
		printf("eim: bad im\n");
		dlclose(handle);
		return;
	}
	if(InitExtraIM(eim,arg))
	{
		dlclose(handle);
		return;
	}
	RegisterNewIM(eim->Name,ExtraReset,ExtraDoInput,ExtraGetCandWords,ExtraGetCandWord,NULL,NULL,NULL,NULL);
	EIMS[i]=eim;
	EIM_handle[i]=handle;
	strcpy(EIM_file[i],fn);
	EIM_index[i]=iIMCount-1;
}
