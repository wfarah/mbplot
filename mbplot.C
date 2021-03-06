#include "cpgplot.h"
#include "plot.h"
#include "dialog.h"
#include "string.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#if defined(S2PLOT)
extern "C" {
#include "s2plot.h"
}
#else
typedef struct {
  float x, y, z;
} XYZ;
#endif

XYZ CrossProduct(XYZ,XYZ);
float DotProduct(XYZ,XYZ);
XYZ VectorSub(XYZ,XYZ);
int sameSide(XYZ p1, XYZ p2, XYZ a, XYZ b);
int inTriangle(XYZ P, XYZ A, XYZ B, XYZ C);

class labels 
{
public:
  float x[100];
  float y[100];
  char * ascii[100];
  int nlabels;
  labels(int nl);
};

labels::labels(int nl){
  nlabels=0;
}

int running_median(float *, float *, int, int);
int local_medians(int nsub, int nlocal, float * newdata, float * median);

void min_means_min(float * min, float * max){
  if (*min>*max) {
    float temp = *max;
    *max=*min;
    *min = temp;
  }
}

int main(int argc, char ** argv){

  int gptno = -1; /* store last point queried */
  results * r = new results(argv[1]);

  printf("number of parameters in %s is %d\n",argv[1],r->nparms);

  labels * l = new labels(0);

  cpgbeg(0,"/xs",1,1);

#if defined(S2PLOT)
  s2opend("/S2MONO", argc, argv);
#endif

  /* create the dialog */
  dialog * d = new dialog();

  /* add the "action" buttons */
  int QUIT         = d->addbutton(0.232,0.05,"Quit");
#if defined(S2PLOT)
  int PLOT3D       = d->addbutton(0.232,0.1,"3D Plot");
#endif
  int PLOT         = d->addbutton(0.232,0.15,"Plot");
  int HISTOGRAM    = d->addbutton(0.232,0.2,"Hist");
  int CHANGECOLOUR = d->addbutton(0.232,0.25,"Colour");
  int SYMBOL       = d->addbutton(0.232,0.3,"Symbol");
  int UNZOOM       = d->addbutton(0.232,0.35,"Unzoom");
  //int ZAPLASTQ     = d->addbutton(0.232,0.4, "Zaplast(d)");
  int TRIDELETE    = d->addbutton(0.232,0.45,"Tri-delete");
  int DELETE       = d->addbutton(0.232,0.5,"Delete");
  int UNDELETE     = d->addbutton(0.232,0.55,"UnDelete");
  int SELECT       = d->addbutton(0.232,0.60,"Select");
  int VIEWALL      = d->addbutton(0.232,0.65,"Viewall");
  int RESET        = d->addbutton(0.232,0.70,"Reset");
  int SAVESELN     = d->addbutton(0.232,0.75, "Save seln");

/*  int CMD1LASTQ     = d->addbutton(0.05,0.75,"CMD1 lastQ");
  int CMD2LASTQ    = d->addbutton(0.05,0.70,"CMD2 lastQ");
  int CMD3LASTQ    = d->addbutton(0.05,0.65,"CMD2 lastQ");
*/
  //int COLLASTQ     = d->addbutton(0.05,0.60,"Col last Q");
  //int COLVERTQ     = d->addbutton(0.05,0.50,"Col vert Q");
  //int ZAPVERTQ     = d->addbutton(0.05,0.45,"Zap vert Q");

  int MEDIAN       = d->addbutton(0.25,0.85,"MEDIAN");

  /* add the plot regions */
  d->addplotregion(0.4,0.99,0.1,0.75);
  d->addplotregion(0.4,0.99,0.8,0.95);

  for (int i=0;i<r->nparms;i++){
    int d1,d2,d3;
    if (i==0) d1=1; else d1=0;
    if (i==1) d2=1; else d2=0;
    if (i==2) d3=1; else d3=0;
    d->addradio(0.010,0.735-0.025*i," ",d3,0); //0.385
    d->addradio(0.035,0.735-0.025*i," ",d1,1); //0.385
    d->addradio(0.060,0.735-0.025*i," ",d2,2); //0.385
  }
 
  //d->addcheck(0.025,0.05+r->nparms*0.025," ",0);
  //d->addcheck(0.050,0.05+r->nparms*0.025,"log?",0);

  d->addcheck(0.025,0.76," ",0); //0.41
  d->addcheck(0.050,0.76,"log?",0); //0.41


  d->addradio(0.025,0.8,"query (q)",1,3);
  d->addradio(0.025,0.825,"zoomx (x)",0,3);
  d->addradio(0.025,0.85,"zoomy (y)",0,3);
  d->addradio(0.025,0.875,"zoombox (z)",0,3);
  d->addradio(0.025,0.9,"annotate (a)",0,3);
  d->addradio(0.025,0.925,"command (c)",0,3);

  d->addradio(0.2,0.825,"Points",1,4);
  d->addradio(0.2,0.8,"Line",0,4);

  d->addcheck(0.025,0.95,"Hardcopy",0);
  d->addcheck(0.2,0.95,"Custom Labels",0);
  int ERRORS = d->addcheck(0.2,0.9,"Errors?",0);
  int LOGZAP = d->addcheck(0.45,0.95,"Log ZAP", 0);
  for (int i=0;i<r->nparms;i++){
	d->addcheck(0.075,0.725-0.025*i," ",0); //0.375
	}
  for (int i=0;i<r->nparms;i++){
        d->addcheck(0.1,0.725-0.025*i,r->popdata[i].name,0); //0.375
	}
  d->draw();

  float x,y;
  char ans;
  int button=-1; int plotno=-1;
  int seek_bounds = 1;
  float x_bound_min, x_bound_max, y_bound_min, y_bound_max;
  int npts;
  int * id = 0;
  float * xd = 0;
  float * yd = 0;
  float * zd = 0;
  float * erx = 0;
  float * ery = 0;
#if defined(S2PLOT)
  float *zd;
  float z_bound_min, z_bound_max;
#endif

  while (button!=QUIT){
    button=d->manage(&x,&y,&ans,&plotno);
    if (button==HISTOGRAM) {
      int xhisto = d->groupon(1);
      xd = new float[r->popdata[0].ndatum];
      xd=r->return1array(xhisto,&npts,d->checks[0].on);
      if (d->checks[2].on) {
	cpgend();
	cpgbeg(0,"?",1,1);
      }
      d->plotregions[1].erase();
      d->plotregions[1].set(x_bound_min,x_bound_max,0.0,(float)npts/4.0);
      cpghist(npts,xd,x_bound_min,x_bound_max,25,1);
      cpgbox("BCNST",0.0,0,"BCNST",0.0,0);
      if (d->checks[2].on){
	cpgend();
	cpgbeg(0,"/xs",1,1);
	cpgscf(1);
	d->checks[2].on=0;
	d->draw();
	button = PLOT; plotno=-1; seek_bounds=0;
      }
      button = plotno = -1;
    }
    if (button==MEDIAN) {
      printf("median\n");
      float * run_median = new float[r->popdata[0].ndatum*2];
      float * input = new float[r->popdata[0].ndatum*2];
      local_medians(r->popdata[0].ndatum, 25, yd, run_median);
      //      running_median(yd, run_median, 
      //	     r->popdata[0].ndatum*2, 25);
      printf("ZAP ");
      for (int i=0;i<r->popdata[0].ndatum;i++) {
	if (
	    pow(10.0,yd[i])-pow(10.0,run_median[i])>4.0e-5
	    ) printf("%d ",i+1);
      }
      printf("\n");
      //      for (int i=0;i<r->popdata[0].ndatum;i++)
      //	printf("GREP %d %f %f %f\n",i,pow(10.0,yd[i]),pow(10.0,run_median[i]),
      //      pow(10.0,yd[i])-pow(10.0,run_median[i]));
      d->plotregions[0].reset();
      cpgsci(2);
      cpgpt(r->popdata[0].ndatum,xd,run_median,17);
      cpgsci(1);
      button = plotno =-1;
    }
    if (button==CHANGECOLOUR) {  // Change Colour
      //for (int i=0;i<r->popdata[0].ndatum;i++) {
      printf("changing color for %d points\n", npts);
      for (int i=0;i<npts;i++) {
	if (xd[i]>=x_bound_min && xd[i]<=x_bound_max &&
	    yd[i]>=y_bound_min && yd[i]<=y_bound_max &&
	    r->deleted[id[i]]!=1){
	  printf(".");
	    r->ci[id[i]]++;
	    if (r->ci[id[i]]>15 || r->ci[id[i]]<1) r->ci[id[i]]=1;
	}
      }
      printf("\n");
      fflush(stdout);
      button = PLOT; seek_bounds=0; plotno = -1;
    }
    if (button==SYMBOL) {  // Change Symbol
      //for (int i=0;i<r->popdata[0].ndatum;i++) {
      for (int i=0;i<npts;i++) {
	if (xd[i]>=x_bound_min && xd[i]<=x_bound_max &&
	    yd[i]>=y_bound_min && yd[i]<=y_bound_max &&
	    r->deleted[id[i]]!=1){
	    r->symbol[id[i]]++;
	    if (r->symbol[id[i]]>17) r->symbol[id[i]]=1;
	}
      }
      button = PLOT; seek_bounds=0; plotno = -1;
    }
    if (button==DELETE) {  // Delete
      //for (int i=0;i<r->popdata[0].ndatum;i++) {
      for (int i=0;i<npts;i++) {
	if (xd[i]>=x_bound_min && xd[i]<=x_bound_max &&
	    yd[i]>=y_bound_min && yd[i]<=y_bound_max){
	    r->deleted[id[i]]=1;
	}
      }
      if (d->checks[LOGZAP].on) {
	printf("zaprect: (%f,%f)-(%f,%f)\n", x_bound_min,y_bound_min,
	       x_bound_max,y_bound_max);
	fflush(stdout);
      }
      button = PLOT; seek_bounds=1; plotno = -1;
    }
    if (button==SELECT) {  // Delete all but these
      for (int i=0;i<r->popdata[0].ndatum;i++) 
	r->deleted[i]=1;
      //for (int i=0;i<r->popdata[0].ndatum;i++)
      for (int i=0;i<npts;i++)
      {
	if (xd[i]>=x_bound_min && xd[i]<=x_bound_max &&
	    yd[i]>=y_bound_min && yd[i]<=y_bound_max){
	    r->deleted[id[i]]=0;
	}
      }
      button = PLOT; seek_bounds=1; plotno = -1;
    }
    if (button==RESET) {  //Reset things
      //for (int i=0;i<r->popdata[0].ndatum;i++) {
      for (int i=0;i<npts;i++) {
	if (xd[i]>=x_bound_min && xd[i]<=x_bound_max &&
	    yd[i]>=y_bound_min && yd[i]<=y_bound_max){
	    r->ci[id[i]]=1;
	    r->symbol[id[i]]=1;
	}
      }
      button = PLOT; seek_bounds=1; plotno = -1;
    }
    if (button==UNZOOM || ans=='u') {  //Reset things
      button = PLOT; seek_bounds=1; plotno = -1;
    }
    if (button==UNDELETE) {  // UnDelete
      for (int i=0;i<r->popdata[0].ndatum;i++) r->deleted[i]=0;
      button = PLOT; seek_bounds=1; plotno = -1;
    }
    if (button==VIEWALL) {  // Viewall
      d->plotregions[0].reset();
      for (int i=0;i<npts;i++){
	if (xd[i]>=x_bound_min && xd[i]<=x_bound_max &&
	    yd[i]>=y_bound_min && yd[i]<=y_bound_max &&
	    r->deleted[i]!=1){
	char text_to_save[1000];
	std::ofstream outfile;
	outfile.open("tmp.dat", std::ios_base::app);
	sprintf(text_to_save,"%s %s %s %03d\n",
                r->popdata[1].vals[i].ascii,
                r->popdata[3].vals[i].ascii,
                r->popdata[5].vals[i].ascii,
                (int)r->popdata[12].vals[i].value);
	outfile << text_to_save;
	cpgsci(2);
	cpgpt1(xd[i],yd[i],17);
	cpgsch(3);
	cpgpt1(xd[i],yd[i],5);
        cpgsch(1);
	}
      }
    char command[1000];
    sprintf(command,"csh ~wfarah/soft/mbplot/mbplot_viewall.csh %s",
    	    argv[2]);
    printf("COMMAND:%s:COMMAND\n",command);
    system(command);
    }
    if (plotno==0 && (ans=='x' || (ans=='A' && d->groupon(3)==1))){
        d->plotregions[plotno].reset();
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
        cpgsci(8);
        cpgmove(x,y1);
        cpgdraw(x,y2);
	x_bound_min = x;
	cpgcurs(&x,&y,&ans);
	x_bound_max = x;
	min_means_min(&x_bound_min,&x_bound_max);
	seek_bounds = 0;
	button=PLOT; plotno=-1; // Plot later in loop
    }
    if (plotno==0 && (ans=='y' || (ans=='A' && d->groupon(3)==2))){
        d->plotregions[plotno].reset();
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
        cpgsci(8);
        cpgmove(x1,y);
        cpgdraw(x2,y);
	y_bound_min = y;
	cpgcurs(&x,&y,&ans);
	y_bound_max = y;
	min_means_min(&y_bound_min,&y_bound_max);
	seek_bounds = 0;
	button=PLOT; plotno=-1; // Plot later in loop
    }
    // Annotate Plot
    if (plotno==0 && (ans=='a' || (ans=='A' && d->groupon(3)==4))){
        d->plotregions[plotno].reset();
        cpgsci(1);
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
	int ptno=r->closest(d->groupon(1),d->groupon(2),x,y,x1,x2,y1,y2,
			    d->checks[0].on,
			    d->checks[1].on);
	l->x[l->nlabels]=x;
	l->y[l->nlabels]=y;
	l->ascii[l->nlabels]=r->popdata[d->groupon(0)].vals[ptno].ascii;
	cpgtext(x,y,l->ascii[l->nlabels]);
        l->nlabels++;
	button=-1; plotno=-1; // Plot later in loop
    }
    // Launch Command 1
    /*if (plotno==0 && (ans=='c' || (ans=='A' && d->groupon(3)==5))){
        d->plotregions[plotno].reset();
        cpgsci(1);
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
	int ptno=r->closest(d->groupon(1),d->groupon(2),x,y,x1,x2,y1,y2,
			    d->checks[0].on,
			    d->checks[1].on);
	gptno = ptno;
	char command[1000];
	sprintf(command,"csh ~wfarah/soft/mbplot/mbplot_c.csh %s %s %s %s %03d",
		argv[2],
		r->popdata[1].vals[ptno].ascii,
		r->popdata[3].vals[ptno].ascii,
		r->popdata[5].vals[ptno].ascii,
		(int)r->popdata[12].vals[ptno].value
		);
	//sprintf(command,"csh mbplot.csh %s",argv[2]);
        //std::ofstream outfile;
        //outfile.open("tmp.dat",std::ios_base::app);
	//for (int i=0;i<r->nparms;i++){
	//    char * t = r->popdata[i].vals[ptno].ascii;
	//    outfile << t;
	//}
	cpgsci(2);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,17);
	cpgsch(3);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,5);
	cpgsch(1);
	printf("COMMAND:%s:COMMAND\n",command);
	system(command);
	button=-1; plotno=-1; // Plot later in loop
    }*/
    // Launch Command 2
    if (plotno==0 && (ans=='f' || (ans=='A' && d->groupon(3)==5))){
        d->plotregions[plotno].reset();
        cpgsci(1);
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
	int ptno=r->closest(d->groupon(1),d->groupon(2),x,y,x1,x2,y1,y2,
			    d->checks[0].on,
			    d->checks[1].on);
	gptno = ptno;
	char command[1000];
	sprintf(command,"~wfarah/fil_extractf %s %s %s %s %s &",
		r->popdata[13].vals[ptno].ascii,
		r->popdata[0].vals[ptno].ascii,
		r->popdata[1].vals[ptno].ascii,
		r->popdata[2].vals[ptno].ascii,
		r->popdata[6].vals[ptno].ascii
		);
	//sprintf(command,"csh mbplot.csh %s",argv[2]);
        //std::ofstream outfile;
        //outfile.open("tmp.dat",std::ios_base::app);
	//for (int i=0;i<r->nparms;i++){
	//    char * t = r->popdata[i].vals[ptno].ascii;
	//    outfile << t;
	//}
	cpgsci(2);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,17);
	cpgsch(3);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,5);
	cpgsch(1);
	printf("COMMAND:%s:COMMAND\n",command);
	system(command);
	button=-1; plotno=-1; // Plot later in loop
    }
    // Launch Command 3
    if (plotno==0 && (ans=='h' || (ans=='A' && d->groupon(3)==5))){
        d->plotregions[plotno].reset();
        cpgsci(1);
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
	int ptno=r->closest(d->groupon(1),d->groupon(2),x,y,x1,x2,y1,y2,
			    d->checks[0].on,
			    d->checks[1].on);
	gptno = ptno;
	char command[1000];
	sprintf(command,"~wfarah/fil_extractf %s %s %s %s %s &",
		r->popdata[5].vals[ptno].ascii,
		r->popdata[0].vals[ptno].ascii,
		r->popdata[1].vals[ptno].ascii,
		r->popdata[2].vals[ptno].ascii,
		r->popdata[6].vals[ptno].ascii
		);
	//sprintf(command,"csh mbplot.csh %s",argv[2]);
        //std::ofstream outfile;
        //outfile.open("tmp.dat",std::ios_base::app);
	//for (int i=0;i<r->nparms;i++){
	//    char * t = r->popdata[i].vals[ptno].ascii;
	//    outfile << t;
	//}
	cpgsci(2);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,17);
	cpgsch(3);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,5);
	cpgsch(1);
	printf("COMMAND:%s:COMMAND\n",command);
	system(command);
	button=-1; plotno=-1; // Plot later in loop
    }
    // Launch Command 4 (general command)
    if (plotno==0 && (ans=='c' || (ans=='A' && d->groupon(3)==5))){
        d->plotregions[plotno].reset();
        cpgsci(1);
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
	int ptno=r->closest(d->groupon(1),d->groupon(2),x,y,x1,x2,y1,y2,
			    d->checks[0].on,
			    d->checks[1].on);
	gptno = ptno;
	char command[1000];
	char *command_pnt = NULL; 
	FILE * fcmd = fopen(CONFIG_DIR"mbplot_general.cmd","r"); //CHECK makeit for CONFIG_DIR
	if (fcmd == NULL){
		printf("Command file doesn't exist\n");
		exit(-1);
	}
	size_t len = 0;
	getline(&command_pnt,&len,fcmd);
    sprintf(command,"%s",command_pnt);
    free(command_pnt);
    command_pnt = command + strlen(command) -1; //-1 for the endline	


	//command_pnt += sprintf(command_pnt,"~wfarah/mbplot_general");
	command_pnt += sprintf(command_pnt," --cand_file %s",argv[1]);
	for (int i=0;i<r->nparms;i++){
		command_pnt += sprintf(command_pnt," --%s %s",
				r->popdata[i].name,
				r->popdata[i].vals[ptno].ascii);
	}
	command_pnt += sprintf(command_pnt," &");
	cpgsci(2);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,17);
	cpgsch(3);
	cpgpt1(r->popdata[d->groupon(1)].vals[ptno].value,
	       r->popdata[d->groupon(2)].vals[ptno].value,5);
	cpgsch(1);
	printf("COMMAND:%s:COMMAND\n",command);
	system(command);
	button=-1; plotno=-1; // Plot later in loop
    }
    if (plotno==0 && (ans=='g' || (ans=='A' && d->groupon(3)==5))) { // do CMD of last queried point
        if (gptno >= 0) {
	    char command[1000];
	    sprintf(command,"~wfarah/fil_extract %s %s %s %s %s &",
		r->popdata[13].vals[gptno].ascii,
		r->popdata[0].vals[gptno].ascii,
		r->popdata[1].vals[gptno].ascii,
		r->popdata[2].vals[gptno].ascii,
		r->popdata[6].vals[gptno].ascii
		);
	    printf("COMMAND:%s:COMMAND\n",command);
	    system(command);
	    r->profile(gptno);
	}
    }
    if (plotno==0 && (ans=='z' || (ans=='A' && d->groupon(3)==3))){
        d->plotregions[plotno].reset();
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
        cpgsci(8);
	cpgband(2,1,x,y,&x2,&y2,&ans);
	x_bound_min = x;
	x_bound_max = x2;
	y_bound_min = y;
	y_bound_max = y2;
	min_means_min(&x_bound_min,&x_bound_max);
	min_means_min(&y_bound_min,&y_bound_max);
	seek_bounds = 0;
	button=PLOT; plotno=-1; // Plot later in loop
    }
    if (plotno==0 && (ans=='q' || (ans=='A' && d->groupon(3)==0))){
        float x1,x2,y1,y2;
        d->plotregions[plotno].query(&x1,&x2,&y1,&y2);
	int ptno=r->closest(d->groupon(1),d->groupon(2),x,y,x1,x2,y1,y2,
			    d->checks[0].on,
			    d->checks[1].on);
	gptno = ptno;
	//if (d->checks[DOPDM].on) {
	//  printf("query result start\n");
	//}
	r->profile(ptno);
	//if (d->checks[DOPDM].on) {
	//  printf("query result end\n");
	//}
	fflush(stdout);
	button = -1; plotno = -1;
    }

    if (button==TRIDELETE) {  // tri-delete
      /* allow user to draw a triangle, then delete all points in 
       * the triangle */
      float trix[3], triy[3];
      int ntri = 0;
      cpgolin(3, &ntri, trix, triy, 1);

      /* numbers below are from setup of plot region 0 at top of file */
      /* 0.35,0.99,0.1,0.75 */

      XYZ a,b,c,p;
      a.x = x_bound_min + (trix[0] - 0.40) / (0.99 - 0.40) * 
	(x_bound_max - x_bound_min);
      a.y = y_bound_min + (triy[0] - 0.10) / (0.75 - 0.10) * 
	(y_bound_max - y_bound_min);
      a.z = 0.;

      b.x = x_bound_min + (trix[1] - 0.40) / (0.99 - 0.40) * 
	(x_bound_max - x_bound_min);
      b.y = y_bound_min + (triy[1] - 0.10) / (0.75 - 0.10) * 
	(y_bound_max - y_bound_min);
      b.z = 0.;

      c.x = x_bound_min + (trix[2] - 0.40) / (0.99 - 0.40) * 
	(x_bound_max - x_bound_min);
      c.y = y_bound_min + (triy[2] - 0.10) / (0.75 - 0.10) * 
	(y_bound_max - y_bound_min);
      c.z = 0.;
      p.z = 0.;
      

      for (int i=0;i<r->popdata[0].ndatum;i++) {
	p.x = xd[i];
	p.y = yd[i];
	if (inTriangle(p, a,b,c)) {
	  r->deleted[id[i]]=1;
	}
      }	

      if (d->checks[LOGZAP].on) {
	printf("zaptri: (%f,%f),(%f,%f),(%f,%f)\n", a.x, a.y, b.x, b.y,
	       c.x, c.y);
	fflush(stdout);
      }

      button = PLOT; seek_bounds=1; plotno = -1;
    }

    //if (button==ZAPLASTQ || ans=='d') { // zap last queried point
    if (ans=='d') { // zap last queried point
      if (gptno >= 0) {
	//printf("deleting ptno = %d\n", gptno);
	r->deleted[gptno]=1;
	/*
	if (d->checks[LOGZAP].on) {
	  printf("zapsingle: 
	  fflush(stdout);
	}
	*/
	printf("\n\n...Deleting last query point...\n\n");
	button=PLOT; seek_bounds=0; plotno=-1;
	gptno = 0;
      }
    }

    /*if (button==COLLASTQ) { // Colour last queried point
      if (gptno >= 0) {
	r->ci[id[gptno]] = (r->ci[id[gptno]] % 15) + 1;
	button=PLOT; seek_bounds=0; plotno=-1;
      }
    }
    
    if (button == COLVERTQ) {
      if (gptno >= 0) {
	float xtar = r->getsinglevalue(d->groupon(1), gptno, d->checks[0].on);
	for (int i=0;i<npts;i++) {
	  if (xd[i]>=xtar * 0.9995 && xd[i]<=xtar*1.0005 &&
	      yd[i]>=y_bound_min && yd[i]<=y_bound_max &&
	      r->deleted[id[i]]!=1){
	    r->ci[id[i]]++;
	    if (r->ci[id[i]]>15 || r->ci[id[i]]<1) r->ci[id[i]]=1;
	  }
	}
	button=PLOT; seek_bounds=0; plotno=-1;
      }
    }

    if ((button==ZAPVERTQ) || ((plotno==0) && (ans=='v'))) { 
      // zap vertical near last queried point
      if (gptno >= 0) {
	float xtar = r->getsinglevalue(d->groupon(1), gptno, d->checks[0].on);
	for (int i=0;i<npts;i++) {
	  if ((xd[i] > xtar*0.9995) && (xd[i] < xtar * 1.0005)) {
	    r->deleted[id[i]]=1;
	  }
	}
	if (d->checks[LOGZAP].on) {
	  printf("zapvert: %f\n", xtar);
	  fflush(stdout);
	}
	button=PLOT; seek_bounds=0; plotno=-1;
	gptno = 0;
      }
    }
*/
    /*if (button==CMD1LASTQ) { // do CMD of last queried point
      if (gptno >= 0) {
	printf("CMD1 data start\n");
	r->profile(gptno);
	printf("CMD1 data end\n");
	fflush(stdout);
      }
    }
    if (button==CMD2LASTQ) { // do CMD of last queried point
      if (gptno >= 0) {
	printf("CMD2 data start\n");
	r->profile(gptno);
	printf("CMD2 data end\n");
	fflush(stdout);
      }
    }
    if (button==CMD3LASTQ) { // do CMD of last queried point
      if (gptno >= 0) {
	printf("CMD3 data start\n");
	r->profile(gptno);
	printf("CMD3 data end\n");
	fflush(stdout);
      }
    }
*/

    if (button==SAVESELN) { // save the selected points
      printf("saving points...\n");
      char outfname[300];
      sprintf(outfname, "%s.sel", argv[1]);
      r->write(outfname);
      printf("...done!\n");
      fflush(stdout);
    }

    if (button==PLOT || (plotno==0 && ans==' ')){  // Plot
      printf("There are %d points\n",r->popdata[0].ndatum);
      int xp = d->groupon(1);
      int yp = d->groupon(2);
      int zp = d->groupon(0);
      if (zp < 0 ) zp = 0;
      xd = new float[r->popdata[0].ndatum];
      yd = new float[r->popdata[0].ndatum];
      zd = new float[r->popdata[0].ndatum];
      id = new int[r->popdata[0].ndatum];
      erx = new float[r->popdata[0].ndatum];
      ery = new float[r->popdata[0].ndatum];
      bool erx_bool = false;
      bool ery_bool = false;
      for (int i=0; i<r->nparms;i++){
	  if (d->checks[6+i].on){
	     erx_bool = true;
	     erx=r->return1array(i,&npts,0);
	     }
	  if (d->checks[6+i+r->nparms].on){
	     ery_bool = true;
	     ery=r->return1array(i,&npts,0);
	     }
	  }
      printf("returning arrays 0\n");
      r->return2arrays(xp,zp,xd,zd,id,&npts,d->checks[0].on,d->checks[1].on);
      printf("returning arrays 1\n");
      r->return2arrays(xp,yp,xd,yd,id,&npts,d->checks[0].on,d->checks[1].on);
      printf("returned\n");
      d->plotregions[0].erase();
      if (seek_bounds==1 && !d->checks[2].on){
	l->nlabels=0;
        d->plotregions[0].set(themin5(xd,npts),themax5(xd,npts),
			      themin5(yd,npts),themax5(yd,npts));
	x_bound_min = themin5(xd,npts);
	x_bound_max = themax5(xd,npts);
	y_bound_min = themin5(yd,npts);
	y_bound_max = themax5(yd,npts);
      } else
	d->plotregions[0].set(x_bound_min,x_bound_max,
			      y_bound_min,y_bound_max);
      char xstr[10]; char ystr[10];
      if (d->checks[0].on) strcpy(xstr,"BCNSTL"); else strcpy(xstr,"BCNST");
      if (d->checks[1].on) strcpy(ystr,"BCNSTL"); else strcpy(ystr,"BCNST");
      if (d->checks[2].on) {
	cpgend();
	cpgbeg(0,"mbplot.ps/cps",1,1);
	cpgswin(x_bound_min,x_bound_max,y_bound_min,y_bound_max);
	cpgsch(1.0);
	cpgsci(1);
	cpgscf(2);
	for (int i=0;i<l->nlabels;i++) cpgtext(l->x[i],l->y[i],l->ascii[i]);
      }
      //      	cpgmove(-3.0,-1.0);
      //	cpgdraw(0.0,2.0);
      //	cpgtext(-2.8,0.2,"P = 0.01 (M\\dc\\u/M\\d\\(2281)\\u) s");
      cpgbox(xstr,0.0,0,ystr,0.0,0);
      for (int i=0;i<npts;i++){
	int ci=r->ci[id[i]];
	cpgsci(ci);
	if (d->groupon(4)==1) {
	  if (i==0) cpgmove(xd[0],yd[0]);
	  else
	    cpgdraw(xd[i],yd[i]);
	}
        cpgpt1(xd[i],yd[i],r->symbol[id[i]]);
	//	if (d->checks[ERRORS].on) {
	//	cpgmove(xd[i],yd[i]-zd[i]);
	//cpgdraw(xd[i],yd[i]+zd[i]);
	//}
      }
      if (erx_bool) cpgerrb(5,npts,xd,yd,erx,1);
      if (ery_bool) cpgerrb(6,npts,xd,yd,ery,1);
      cpgsci(1);
      if (d->checks[3].on){
	char xlab[100],ylab[100],toplabel[100];
	printf("Enter Custom labels\n Enter x axis label >");
	fgets(xlab, 99, stdin);
	/* replace new line with null */
	xlab[strlen(xlab)-1] = '\0';
	printf(" Enter y axis label >");
	fgets(ylab, 99, stdin);
	ylab[strlen(ylab)-1] = '\0';
	printf(" Enter Top label >");
	fgets(toplabel, 99, stdin);
	toplabel[strlen(toplabel)-1] = '\0';
	cpglab(xlab,ylab,toplabel);
      } else
      cpglab(r->popdata[d->groupon(1)].name,r->popdata[d->groupon(2)].name," ");
      // replot interactive
      if (d->checks[2].on) {
	cpgend();
	cpgbeg(0,"/xs",1,1);
	cpgscf(1);
	d->checks[2].on=0;
	d->draw();
	button = PLOT; plotno=-1; seek_bounds=0;
      } else{
        button =   -1; plotno=-1; seek_bounds=1;
      }
    }

#if defined(S2PLOT)
    if (button==PLOT3D) {
      int xp = d->groupon(1);
      int yp = d->groupon(2);
      int zp = d->groupon(0);
      xd = new float[r->popdata[0].ndatum];
      yd = new float[r->popdata[0].ndatum];
      zd = new float[r->popdata[0].ndatum];
      id = new int[r->popdata[0].ndatum];
      r->return2arrays(xp,zp,xd,zd,id,&npts,0,0);
      r->return2arrays(xp,yp,xd,yd,id,&npts,d->checks[0].on,d->checks[1].on);

      x_bound_min = themin5(xd,npts);
      x_bound_max = themax5(xd,npts);
      y_bound_min = themin5(yd,npts);
      y_bound_max = themax5(yd,npts);
      z_bound_min = themin5(zd,npts);
      z_bound_max = themax5(zd,npts);

      s2eras();
      s2swin(x_bound_min,x_bound_max,y_bound_min,y_bound_max,
      	     z_bound_min,z_bound_max);
      s2sci(S2_PG_WHITE);
      s2sch(1.);
      s2slw(1);
      s2iden(" - mbplot");

      char xstr[16]; char ystr[16]; char zstr[16];
      if (d->checks[0].on) strcpy(xstr,"BCDEQGMNSTL"); else strcpy(xstr,"BCDEQGMNST");
      if (d->checks[1].on) strcpy(ystr,"BCDEQGMNSTL"); else strcpy(ystr,"BCDEQGMNST");
      strcpy(zstr, "BCDEQGMNST");
      s2box(xstr, 0., 0, ystr, 0., 0, zstr, 0., 0);
      s2lab(r->popdata[xp].name,r->popdata[yp].name, 
	    r->popdata[zp].name,"");

      for (int i=0;i<npts;i++){
	int ci=r->ci[id[i]]; 
	s2sci(ci); 
	s2pt1(xd[i],yd[i],zd[i],1);
      }

      s2disp(-1,1); /* never timeout */

    }
#endif

  }
  cpgend();

  printf("Normal execution completed\n");
  return(0);

}



XYZ CrossProduct(XYZ p1,XYZ p2)
{
   XYZ p;

   p.x = p1.y * p2.z - p1.z * p2.y;
   p.y = p1.z * p2.x - p1.x * p2.z;
   p.z = p1.x * p2.y - p1.y * p2.x;

   return(p);
}

float DotProduct(XYZ p1,XYZ p2)
{
   return(p1.x*p2.x + p1.y*p2.y + p1.z*p2.z);
}

XYZ VectorSub(XYZ p1,XYZ p2) {
  XYZ p;

  p.x = p2.x - p1.x;
  p.y = p2.y - p1.y;
  p.z = p2.z - p1.z;
  
  return(p);
}


int sameSide(XYZ p1, XYZ p2, XYZ a, XYZ b) {
  if (DotProduct(CrossProduct(VectorSub(b,a), VectorSub(p1,a)),
		 CrossProduct(VectorSub(b,a), VectorSub(p2,a))) >= 0) {
    return 1;
  }
  return 0;
}

int inTriangle(XYZ p, XYZ A, XYZ B, XYZ C) {
  if (!sameSide(p,A,B,C) || !sameSide(p,B,A,C) || !sameSide(p,C,A,B)) {
    return 0;
  }
  return 1;
}

