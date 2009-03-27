#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char strCode[10];
    char strHZ[256];
    int s;
    
    if ( argc!=2 ) {
	fprintf(stderr,"Usage: jd2fcitx <source file>\n");
	return -1;
    }
    
    fp=fopen(argv[1], "rt");
    if (!fp ) {
	    fprintf(stderr,"Cannot open source file.\n");
	    return -2;
    }
    
    s=0;
    while (!feof(fp) ) {
	    fscanf(fp, "%s\n", strHZ );
	    if ( isalpha(strHZ[0]) )
		    strcpy(strCode, strHZ);
	    else {
		if(strlen(strCode)>=10 || strlen(strHZ)>=50)
		    exit(0);
		printf("%s %s\n", strCode,strHZ);
		s++;
	    }
    }
    
    fprintf(stderr,"Total: %d\n",s);
    fclose(fp);
    
    return 0;
}
