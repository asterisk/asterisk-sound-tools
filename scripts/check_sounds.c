/* 
	Copyright 2010 by Steve Murphy, ParseTree Corporation, all rights reserved.
The Author places this file in the PUBLIC DOMAIN. Do with it as you wish.
No warranties. No guarantees. No promises. Use at your own risk, etc.
Give me credit for writing this, please! 

*/



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

struct file_ent
{
	char *name;
	char *text;
	int present;
};

void usage(void)
{
	printf("Usage:  check_sounds <scriptfile> <file-extension>\n\nWhere <script file> is a file with lines of format: <filename>: <script>\n<file-extension> is one of: wav,g722,g729,ulaw,alaw,gsm,etc.\n");
	printf("        This program should be run in the directory containing the sound files and digits,phonetic,etc subdirs.\n");
	printf("        Once running, hit ? and Enter, if you are completely mystified as to what it is waiting for.\n");
}


int file_filter(const struct dirent *x)
{
	if (x->d_type == DT_REG)
		return 1;
	return 0;
}


int dir_filter(const struct dirent *x)
{
	if (x->d_type == DT_DIR && strcmp(x->d_name, ".") != 0 && strcmp(x->d_name, "..") != 0)
		return 1;
	return 0;
}

char charbuf[512] = "";
char *cbufptr = charbuf;

int mygetc(void)
{
	if (*cbufptr == 0) {
		again:
		fgets(charbuf,sizeof(charbuf),stdin);
		charbuf[strlen(charbuf)-1] = 0; /* get rid of the trailing \n */
		cbufptr=charbuf;
		if (charbuf[0] == 0)
			goto again;
		return *cbufptr++;
	} else {
		return *cbufptr++;
	}
}

int main(int argc, char **argv)
{
	FILE *f;
	int numents = 0;
	int lineno = 0;
	int ind = 0;
	struct file_ent *files;
	char buf[80960];
	char nambuf[1024];
	struct dirent **total_namelist;
	struct dirent **dir_namelist;
	struct dirent **subtotal_namelist;
	int total_size;
	int dir_size;
	int sub_size;
	int i;
	int leave = 0;
	
	if (!(f=fopen(argv[1], "r"))) {
		printf("Can't open the file '%s'\n", argv[1]);
		usage();
		return -1;
	}

	while(fgets(buf,sizeof(buf),f))  /* get a count of the number of files */ {
		numents++;
	}
	fclose(f);
	printf("There are %d entries in the file\n", numents);

	files = (struct file_ent *)calloc(numents,sizeof(struct file_ent));	

	if (!(f=fopen(argv[1], "r")))
		return -1;

	while(fgets(buf,sizeof(buf),f))  /* get a count of the number of files */ {
		char *p = strchr(buf,':');
		lineno++;
		if (buf[0] == ';')
			continue;
		if (!p) {
			printf("file '%s' on line %d: Incorrect format. Ignored.\n", argv[1], lineno);
			continue;
		}
		*p = 0;
		snprintf(nambuf,sizeof(nambuf),"%s.%s", buf, argv[2]);
		files[ind].name = strdup(nambuf);
		if(strlen(p+1)) {
			files[ind].text = strdup(p+1);
			files[ind].text[strlen(files[ind].text)-1] = 0; /* remove \n at the end */
		} else {
			files[ind].text = strdup("X");
			files[ind].text[0] = 0;
		}
		if (access(nambuf,F_OK)==0) {
			files[ind].present = 1;
		} else {
			files[ind].present = 0;
			printf("The file '%s' does not exist!\n", nambuf);
		}
		ind++;
	}
	fclose(f);
	/* now, check files we actually have, and report on any files we have, that are not in the scripts. */
	/* SUBDIRS: letters dictate phonetic digits silence followme (wx, ha, ... ) */
	total_size= scandir(".", &total_namelist, file_filter, alphasort);
	if(total_size <= 0) 
	{
		printf("total_size < 0\n");
		exit(-1);
	}
	dir_size = scandir(".", &dir_namelist, dir_filter, alphasort);
	if(dir_size <= 0) 
	{
		printf("dir_size < 0\n");
		exit(-1);
	}

	for (i=0; i< dir_size; i++) /* build the total_namelist from the subdirs */
	{
		int j;
		subtotal_namelist = NULL;
		sub_size = scandir(dir_namelist[i]->d_name, &subtotal_namelist, file_filter, alphasort);
		total_namelist = realloc(total_namelist, (total_size + sub_size) * sizeof(struct dirent*));
		for (j=total_size; j<total_size+sub_size; j++) {
			struct dirent *x = calloc(1,sizeof(struct dirent));
			snprintf(x->d_name, sizeof(x->d_name), "%s/%s", dir_namelist[i]->d_name, subtotal_namelist[j-total_size]->d_name);
			x->d_ino = subtotal_namelist[j-total_size]->d_ino;
			x->d_type = subtotal_namelist[j-total_size]->d_type;
			total_namelist[j] = x;
			free(subtotal_namelist[j-total_size]);
		}
		total_size += sub_size;
		free(subtotal_namelist);
	}

	/* do the check */
	for (i=0; i < total_size; i++) {
		int found;

		found = 0;
		int j;

		for(j=0; j<ind; j++) {
			if (strncmp(total_namelist[i]->d_name,  files[j].name, strlen(files[j].name)) == 0 ) {
				found=1;
				break;
			}
		}
		if (!found && strstr(total_namelist[i]->d_name,".wav")) {
			printf("Could not find the file '%s' in the script list!\n", total_namelist[i]->d_name);
		}
	}
	printf("There are %d files in this dir tree.\n", ind);
	i = 0;
	while (!leave) {
		int c;
		char cbuf[1023];
		char *d;
		int start, end, j;
		while (i< ind && !files[i].present)
			i++;
		if (i >= ind)
			break;
		printf("File: #%d, name=%s, text=%s\n", i, files[i].name, files[i].text);
	no_outp:
		c = mygetc();
		switch (c) {
		case 'p':
		    playit:
			sprintf(cbuf,"play -q %s > /dev/null 2>&1", files[i].name);
			system(cbuf);
			i++;
			if(i>= ind)
				leave=1;
			break;
		case '\n':
			goto no_outp;
		case '\r':
			goto no_outp;
		case 'q':
			leave=1;
			break;
		case 'j':
			printf("\ngoto file #");
			fgets(cbuf,sizeof(cbuf),stdin);
			i =atoi(cbuf);
			break;
		case 'l':
			printf("\nlist entries in range (return to list all) (num <space> num): ");
			fgets(cbuf,sizeof(cbuf),stdin);
			if (strlen(cbuf) == 1) {
			 	start = 0;
				end = ind;
			} else {
				start = strtol(cbuf, &d, 10);
				end = strtol(d, NULL, 10);
				if (end == 0 && end < start)
					end = ind;
			}
				 
			for (j=start; j<end; j++) {
				printf("#%d  %s: %s\n", j, files[j].name, files[j].text);
			}
		case 'r':
			i--;
			goto playit;
			break;
		case '?':
		case 'h':
			printf("\nEnter:  'p' for play; \n\
  'j' jump to a particular file by number (you will be prompted for the number); \n\
  'l' to list files (you will be prompted); \n\
  'r' to repeat playing the previous soundfile; \n\
  'q' to quit; or...  \n\
  'h' or '?' for this message.\n\
Each command must be followed by a return to register.\n\
Multiple characters can be input, followed by 'Enter'\n\
Example:  ppppppppp <return/enter>\n\
 will play the next 9 files without stopping\n");
			goto no_outp;
		default:
			goto no_outp;
		}
	}
}

