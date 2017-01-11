/*local_medians.c, a program to calculate the running median of a 1d data array
**
**ahughes Feb 97
**
*/

#include <stdio.h>
#include <string.h>

float give_median(int npts, float * newdata);
void sort(int n, float * ra);

int local_medians(int nsub, int nlocal, float * newdata, float * median){

/* given the input array newdata, this routine fills in the median array with
the "local" median value, which is computed from nlocal points at each point. */

  // do middle points
  for (int i=nlocal;i<=nsub;i++) 
    median[i-nlocal/2-1]=give_median(nlocal,&newdata[i-nlocal]);
    
 
  // Set the edge values to the median nlocal/2 away from the edge.
  for (int i=0;i<nlocal/2;i++) median[i] = median[nlocal/2];
  for (int i = nsub-nlocal/2;i<nsub; i++) median[i]=median[nsub-1-nlocal/2];
  
   return 0;

} 


float give_median(int npts, float * newdata){

// returns median of input array

  float * work_array = new float[npts];
  float median;
 
  // copy into working array so that original data is not overwritten
  for (int i=0;i<npts;i++) work_array[i]=newdata[i];
  sort(npts,work_array);
  median=work_array[npts/2];
  delete [] work_array;

  return(median);
}

void sort(int n,float *ra)
{
  int l,j,ir,i;
  float rra;

  if(n<2) return;
  l=(n >> 1)+1;
  ir=n;
  for (;;) {
     if (l > 1)	
	rra=ra[--l-1];
     else {
	rra=ra[ir-1];
	ra[ir-1]=ra[0];
	if (--ir == 0) {
		ra[0]=rra;
		return;
		}
	}
     i=l;
     j=l<<1;
     while (j <= ir) {
	if (j < ir && ra[j-1] < ra[j]) j++;
	if (rra < ra[j-1]) {
		ra[i-1]=ra[j-1];
		i=j;
		j = i<<1;
		}
	else j=ir+1;
        }
     ra[i-1]=rra;
     }
     return;
}

















