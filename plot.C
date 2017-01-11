#include "cpgplot.h"
#include "plot.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

data_array::data_array(){}
datum::datum(){}

results::results(int ncols, int nrows, char * input_label){

  FILE * fptr;
  // Get the title
  label = new char[strlen(input_label)+1];
  strcpy(label, input_label);
  
  popdata = new data_array[nrows];

  for (int i=0;i<ncols;i++) {
    popdata[i].vals = new datum[nrows];
    popdata[i].ndatum = nrows;
    popdata[i].name=new char[100];
  }
  // First line is the titles
  fptr = fopen(input_label,"r");
  if (fptr==NULL) {
    fprintf(stderr,"results::results(,,) Error opening file %s\n",
	    input_label);
    exit(-1);
  }
  char a_line[1000];
  fgets(a_line,999,fptr);

  unsigned int chars_read = 0;

  int popnum=0;
  while (chars_read < strlen(a_line)){
    int nc;
    sscanf(&a_line[chars_read],"%s%n",popdata[popnum].name,&nc);
    while (*popdata[popnum].name==' ') popdata[popnum].name++;
    chars_read+=nc;
    popnum++;
  }

  fclose(fptr);
}

results::results(char * fname){
  FILE * fptr;

  fptr = fopen(fname,"r");
  if (fptr==NULL) {
    fprintf(stderr,"Error opening file %s\n",fname);
    exit(-1);
  }

  char a_line[1000];
  int linecount;
  linecount= 0;

  while (fgets(a_line,999,fptr)!='\0')linecount++;
  fclose(fptr);
  
  int words=0;
  char command[1000];
  sprintf(command,"head -1 %s | wc -w",fname);
  FILE * fptr2 = popen(command,"r");
  fscanf(fptr2,"%d",&words);
  fclose(fptr2);

  nparms = words;

  // OK we know the number of fields and the number of rows.

  ndatum = linecount - 1;

  label = new char[strlen(fname)+1];
  strcpy(label, fname);

  selected = new int[ndatum];
  for (int i=0;i<ndatum;i++) selected[i]=0;  // Deselected
  ci = new int[ndatum];
  for (int i=0;i<ndatum;i++) ci[i]=1;  // White the default
  symbol = new int[ndatum];
  for (int i=0;i<ndatum;i++) symbol[i]=1;  // A point
  plotted = new int[ndatum];
  for (int i=0;i<ndatum;i++) plotted[i]=0;  // Not plotted as yet
  deleted = new int[ndatum];
  for (int i=0;i<ndatum;i++) deleted[i]=0;  // Not deleted as yet
  
  popdata = new data_array[words];
  for (int i=0;i<words;i++) {
    popdata[i].vals = new datum[ndatum];
    popdata[i].ndatum = ndatum;
    popdata[i].name=new char[100];
  }
  // First line is the titles
  fptr = fopen(fname,"r");
  if (fptr==NULL) {
    fprintf(stderr,"results::results(char *) Error opening file %s\n",
	    fname);
    exit(-1);
  }

  fgets(a_line,999,fptr);
  unsigned int chars_read = 0;

  int popnum=0;
  while (chars_read < strlen(a_line) && popnum < words){
    int nc;
    sscanf(&a_line[chars_read],"%s%n",popdata[popnum].name,&nc);
    chars_read+=nc;
    popnum++;
  }

  // have the labels, now read the data.

  for (int i=0;i<ndatum;i++){
    chars_read = 0;
    fgets(a_line,999,fptr);          // Grab the line
    popnum=0;
    while (chars_read < strlen(a_line) && popnum < words){
      int nc;
      char thestring[100];
      sscanf(&a_line[chars_read],"%s%n",thestring,&nc);
      int ok = sscanf(thestring,"%f",&popdata[popnum].vals[i].value);
      if (ok==1) popdata[popnum].vals[i].defined = 1; else
	popdata[popnum].vals[i].defined=0;
      popdata[popnum].vals[i].ascii = new char[nc+1];
      strncpy(popdata[popnum].vals[i].ascii,&a_line[chars_read],nc);
      chars_read+=nc;
      popnum++;
    }
  }
  fclose(fptr);
}

void results::show(){

  for (int i=0;i<ndatum;i++)
    {
      for (int j=0;j<nparms;j++) {
	printf("%s ",popdata[j].vals[i].ascii);
	printf(" %d %d",selected[i], ci[i]);
      }
      printf("\n");
    } 
}

void cpgbutton(float x, float y, char * label){
  cpgtext(x,y,label);
  cpgsfs(0);
  float xl,yl;
  cpglen(4,label,&xl,&yl);
  cpgrect(x-0.005,x+xl+0.005,y-0.005,y+0.015+0.005);
}

int results::pgplot(){

  cpgbeg(0,"?",1,1);
  cpgswin(0.0,1.0,0.0,1.0);
  cpgsvp(0.0,1.0,0.0,1.0);
  cpgtext(0.5,0.95,label);

  cpgsfs(0);
  for (int i=0;i<nparms;i++){
    float dx = 0.02;
    float dy = 0.02;
    float x = 0.01;
    float y=0.9-i*0.05;
    cpgrect(x,x+dx,y,y+dy);
    x+=2*dx;
    cpgrect(x,x+dx,y,y+dy);
    x+=2*dx;
    cpgtext(x,y,popdata[i].name);
  }

  cpgbutton(0.05,0.05,(char *)"Plot");
	    cpgbutton(0.15,0.05,(char *)"Histogram");
	    cpgbutton(0.1,0.2,(char *)"Chunglee Kim"); 

  float x=0.5,y=0.5;
  char ans[20];
  while (ans[0]!='q'){
  cpgcurs(&x,&y,ans);
    }
  cpgend();
  return 0;
}

float * results::return1array(int popno, int * ndata, int xlogged){

  int maxnd = popdata[popno].ndatum;

  float * array = new float[maxnd];

  int totalvals;

  totalvals=0;
  for (int i=0;i<maxnd;i++){
    if (popdata[popno].vals[i].defined && !deleted[i]){
      if (xlogged){
	if (popdata[popno].vals[i].value>0.0){
          array[totalvals]=log10(popdata[popno].vals[i].value);
          totalvals++;
	}
      }
      else
      {
       array[totalvals]=popdata[popno].vals[i].value;
       totalvals++;
      }
    }
  }
  *ndata=totalvals;
  return(array);
}

void results::return2arrays(int popno1, int popno2, float * arrayx, float * arrayy, 
			    int * id, int * ndata, int xlogged, int ylogged){

  int maxnd = popdata[popno1].ndatum;

  //  arrayx = new float[maxnd];
  //  arrayy = new float[maxnd];

  int totalvals;

  totalvals=0;
  for (int i=0;i<maxnd;i++){

    if (popdata[popno1].vals[i].defined && popdata[popno2].vals[i].defined
	&& !deleted[i]){
      // Both logged
      if (xlogged && ylogged){
	if (popdata[popno1].vals[i].value>0.0 && popdata[popno2].vals[i].value>0.0){
        arrayx[totalvals]=log10(popdata[popno1].vals[i].value);
        arrayy[totalvals]=log10(popdata[popno2].vals[i].value);
	id[totalvals]=i;
        totalvals++;
	}
      }
      // Just xlogged
      if (xlogged && !ylogged){
	if (popdata[popno1].vals[i].value>0.0){
        arrayx[totalvals]=log10(popdata[popno1].vals[i].value);
        arrayy[totalvals]=popdata[popno2].vals[i].value;
	id[totalvals]=i;
        totalvals++;
	}
      }
      // Just ylogged
      if (!xlogged && ylogged){
	if (popdata[popno2].vals[i].value>0.0){
        arrayx[totalvals]=popdata[popno1].vals[i].value;
        arrayy[totalvals]=log10(popdata[popno2].vals[i].value);
	id[totalvals]=i;
        totalvals++;
	}
      }
      // Neither logged
      if (!xlogged && !ylogged){
        arrayx[totalvals]=popdata[popno1].vals[i].value;
        arrayy[totalvals]=popdata[popno2].vals[i].value;
	id[totalvals]=i;
        totalvals++;
      }
    }
  }
  *ndata=totalvals;
}

float results::getsinglevalue(int arrno, int whichpt, int logged) {
  float res = popdata[arrno].vals[whichpt].value;
  if (logged) {
    if (res > 0.0) {
      return log10(res);
    } else {
      return -100.0;
    }
  } else {
    return res;
  }
}
    
float themin(float * x, int n){
  float m = x[0];
  for (int i=0;i<n;i++) if (m>x[i]) m=x[i];
  return(m);
}

float themax(float * x, int n){
  float m = x[0];
  for (int i=0;i<n;i++) if (m<x[i]) m=x[i];
  return(m);
}

// The min minus 5%
float themin5(float * x, int n){
  float m = themin(x,n)- 0.05*(themax(x,n)-themin(x,n));
  return(m);
}

// The max plus 5%
float themax5(float * x, int n){
  float m = themax(x,n)+0.05*(themax(x,n)-themin(x,n));
  return(m);
}

int results::profile(int ptno){
  for (int i=0;i<nparms;i++)
  printf("%s %s \n",popdata[i].name,popdata[i].vals[ptno].ascii);
  return(ptno);
}


int results::closest(int xvar, int yvar, float x, float y,
		     float xmin, float xmax, float ymin, float ymax,
		     int xlog, int ylog){
  
  float distmin = 2.0;
  int index=0;

  for (int i=0;i<popdata[0].ndatum;i++){
    if (popdata[xvar].vals[i].defined && popdata[yvar].vals[i].defined){
      float dx, dy;
      if (xlog ) {
	if (popdata[xvar].vals[i].value>0.0)
         dx = (log10(popdata[xvar].vals[i].value)-x)/(xmax-xmin);
	else
	  dx=10.0;
      }
      if (!xlog ) {
         dx = (popdata[xvar].vals[i].value-x)/(xmax-xmin);
      }
      if (ylog ) {
	if (popdata[yvar].vals[i].value>0.0)
         dy = (log10(popdata[yvar].vals[i].value)-y)/(ymax-ymin);
	else
	  dy=10.0;
      }
      if (!ylog ) {
       dy = (popdata[yvar].vals[i].value-y)/(ymax-ymin);
      }
      float dist = sqrt(dx*dx+dy*dy);
      if (dist<distmin && !deleted[i]){
	distmin = dist;
	index = i;
      }
    }
  }
  return(index);
}

void results::write(char *fname) {
  FILE *fptr;
  fptr = fopen(fname, "w");
  if (fptr == NULL) {
    fprintf(stderr, "Error opening file %s\n", fname);
    return;
  }

  int i,j;

  /* write first line: titles of columns */
  for (i = 0; i < nparms; i++) {
    fprintf(fptr, "%s ", popdata[i].name);
  }
  fprintf(fptr, "\n");

  /* write the data line by line only the non-deleted points */
  for (i = 0; i < ndatum; i++) {
    if (deleted[i]) {
      continue;
    }
    
    for (j = 0; j < nparms; j++) {
      fprintf(fptr, "%s ", popdata[j].vals[i].ascii);
    }
    fprintf(fptr, "\n");
  }
}  




