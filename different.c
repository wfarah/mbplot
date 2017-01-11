#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nr.h"

main(int argc, char ** argv){
float * dat1, * dat2,p,f;
int npts,npts2,i;
FILE * fptr;
char s[100];
char line[100];
char * cptr;
int istart;

if (argc<3) {fprintf(stderr,"Usage: different filename1 filename2\n"); 
             fprintf(stderr,"       Program takes input from two files\n");
	     fprintf(stderr,"       and works out the probability that the\n");
             fprintf(stderr,"       two distributions are different\n");
             exit(-1);}

 if ((fptr = fopen(argv[1],"r"))!=0) {
	while ((cptr=fgets(line,100,fptr))!='\0') npts++;
	fclose(fptr);

	dat1 = (float *) malloc(sizeof (float) * npts);
        fptr=fopen(argv[1],"r");
        npts = 0;
        
        while ((cptr=fgets(line,300,fptr))!='\0') {
             sscanf(line,"%f",&dat1[npts]);
             npts++;
             }
        fclose(fptr);
 }
 else 
 {
	fprintf(stderr,"Error opening %s\n",argv[1]);
	exit(-1);
 }

 npts2=0;
 if ((fptr = fopen(argv[2],"r"))!=0) {
	while ((cptr=fgets(line,100,fptr))!='\0') npts2++;
	fclose(fptr);

	dat2 = (float *) malloc(sizeof (float) * npts2);
        fptr=fopen(argv[2],"r");
        npts2 = 0;
        
        while ((cptr=fgets(line,300,fptr))!='\0') {
             sscanf(line,"%f",&dat2[npts2]);
             npts2++;
             }
	fclose(fptr);
 }
 else 
 {
	fprintf(stderr,"Error opening %s\n",argv[2]);
	exit(-1);
 }

printf("File %s has %d pts\n",argv[1],npts);
printf("File %s has %d pts\n",argv[2],npts2);

kstwo(&dat1[-1],npts,&dat2[-1],npts2,&f,&p);

printf("Probability that they are different\n");
printf("is %f\n",1.0-p);

}




























