#ifndef _FCITX_H_
#define _FCITX_H_

#define EIM_MAX		4

#define MAX_IM_NAME	15

#define KEYM_MASK	0xff0000
#define KEYM_CTRL	0x010000
#define KEYM_SHIFT	0x020000
#define KEYM_ALT	0x040000
#define KEYM_SUPER	0x080000

#define VK_CODE(x)	((x)&0xffff)

#define VK_ENTER	'\r'
#define VK_BACKSPACE	'\b'
#define VK_TAB		'\t'
#define VK_ESC		0x1b
#define VK_SPACE	0x20
#define VK_LSHIFT	0xe1
#define VK_RSHIFT	0xe2
#define VK_LCTRL	0xe3
#define VK_RCTRL	0xe4
#define VK_CAPSLOCK	0xe5
#define VK_LALT		0xe9
#define VK_RALT		0xea
#define VK_DELETE	0xff

#define VK_HOME		0xff50
#define VK_LEFT		0xff51
#define VK_UP		0xff52
#define VK_RIGHT	0xff53
#define VK_DOWN		0xff54
#define VK_PGUP		0xff55
#define VK_PGDN		0xff56
#define VK_END		0xff57
#define VK_INSERT	0xff63

#define MAX_CODE_LEN	63
#define MAX_CAND_LEN	127
#define MAX_TIPS_LEN	9

typedef struct {
	char Name[MAX_IM_NAME + 1];

	void (*Reset) (void);
	int (*DoInput) (int);
	int (*GetCandWords)(int);
	char *(*GetCandWord) (int);
	int (*Init) (char *arg);
	int (*Destroy) (void);
	void *Bihua;

	char *CodeInput;
	char *StringGet;
	char (*CandTable)[MAX_CAND_LEN+1];
	char (*CodeTips)[MAX_TIPS_LEN+1];
	char *(*GetSelect)(void);
	char *(*GetPath)(char *);
	int CandWordMax;

	int CodeLen;
	int CurCandPage;
	int CandWordCount;
	int CandPageCount;
	int SelectIndex;
	int CaretPos;
}EXTRA_IM;

#endif/*_FCITX_H_*/
