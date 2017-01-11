#include <stdio.h>
#include <plot.h>
#include <math.h>

int main(int argc, char ** argv){

  results * pulsars = new results(argv[1]);
  results * cands = new results(argv[2]);

  //printf("Nparms for %s is %d\n", argv[1], pulsars->nparms);
  //printf("Ndatum for %s is %d\n", argv[1], pulsars->ndatum);

  //  printf("value of last point is %f\n",pulsars->popdata[2].vals[3].value);

  //printf("Nparms for %s is %d\n", argv[2], cands->nparms);
  //printf("Ndatum for %s is %d\n", argv[2], cands->ndatum);

  for (int i=0;i<cands->ndatum;i++){
    
    //printf("Testing candidate %s with p= %s and dm = %s\n",cands->popdata[0].vals[i].ascii,
    //cands->popdata[11].vals[i].ascii,cands->popdata[12].vals[i].ascii);

    float dm_c = cands->popdata[12].vals[i].value;
    float p_c = cands->popdata[11].vals[i].value;

    // ra is in hours, dec is in deg
    float ra_c = cands->popdata[5].vals[i].value;
    float dec_c = cands->popdata[6].vals[i].value;

    for (int j=0;j<pulsars->ndatum;j++){
      int match = 0;   // guilty until proven innocent
      float dm_p = pulsars->popdata[5].vals[j].value;
      float p_p = pulsars->popdata[3].vals[j].value;
      p_p*=1000.0; // want ms

      // ra is in hours, dec is in deg
      float ra_p = pulsars->popdata[1].vals[j].value;
      ra_p/=15.0; // now hrs
      float dec_p = pulsars->popdata[2].vals[j].value;

      if (
	  (dm_c !=0.0 && dm_p!=0.0)
	  && (fabs((ra_p-ra_c)/cos(dec_c*M_PI/180.0))<1.0)  // within a degree
	  && (fabs(dec_p-dec_c)<1.0) // within a degree
	  && (fabs(dm_p/dm_c)>0.8 && fabs(dm_c/dm_p)>0.8) // dm within 20%
	  && (fabs(p_p/p_c)>0.99 && fabs(p_c/p_p)>0.99) // p within 1%
	  ) 
	{
	  match = 1;
	}
      //      printf("psr %4.4d p %f dm %f\n",j,p_p, dm_p);
      if (match) {
	printf("%s matches %s PSRp %s CANDp %s PSRdm %s CANDdm %s PSRra %f CANDra %s\n",
	       pulsars->popdata[0].vals[j].ascii,cands->popdata[0].vals[i].ascii,
	       pulsars->popdata[3].vals[j].ascii,cands->popdata[11].vals[i].ascii,
	       pulsars->popdata[5].vals[j].ascii,cands->popdata[12].vals[i].ascii,
	       (pulsars->popdata[1].vals[j].value)/15.0,cands->popdata[5].vals[i].ascii
	       );
      }
    }
  }
} // main
