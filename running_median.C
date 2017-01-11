/***************************************************************************
 *
 *   Copyright (C) 1998 by wvanstra Willem van Straten
 *   Licensed under the Academic Free License version 2.1
 *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

/*
   MXB 28-May 1997
   Added escape clause if passband is all zeroes. 

 */ 

int running_median(float * input_array, float * run_median, 
		   int dimension, int range)
{
  /* Compute the running median */
  int i, j;
  float min, max;

  int index, median;
  int sort_size;
  float sort_interval;
  int * sort_array;

  int old_pt, new_pt;

  /* find the min and max of input_array */
  min = max = input_array [0];	
  for(j=1; j<dimension; ++j){
    if(min>input_array[j])
      min = input_array[j];
    if(max<input_array[j])
      max = input_array[j];
  }

  if (max-min == 0.0) {
    fprintf(stderr,"running_median error: max = min = %f\n",min);
    return (-1);
  }

  /* we take the array to have a resolution */
  /* corresponding to an accuracy of .01 percent */

  sort_size = 100000;
  sort_interval = (max-min)/(float)(sort_size-1);

  sort_array = (int*) malloc (sort_size * sizeof(int));
  assert (sort_array != NULL);

  /* initialize */
  index = 0; median = 0;
  for(j=0; j<sort_size; ++j)
    sort_array[j] = 0;

  /* sort the first range elements into the sort_array*/
  for(j=0; j<range; ++j) {
    sort_array[(int)((input_array[j]-min)/sort_interval)]++;
  }

  /* find the median of the first range points */
  while(median<range/2){
    median+=sort_array[index];
    index++;
  }
  
  /* set the first range/2 points equal to this median */
  for(j=0; j<=range/2; ++j)
    run_median[j] = index*sort_interval + min;

  /* now compute the running median for dimension - range/2 points */
  for(j=range/2+1; j<(dimension-range/2); ++j){
    old_pt = (int)((input_array[j-range/2]-min)/sort_interval);
    new_pt = (int)((input_array[j+range/2]-min)/sort_interval);
    sort_array[old_pt]--;
    sort_array[new_pt]++;
    if(index>old_pt && index<new_pt)
      median -= 1;
    if(index<old_pt && index>new_pt)
      median += 1;
    while(median<range/2){
      index++;
      median+=sort_array[index];
    }
    while(median>range/2){
      median-=sort_array[index];
      index--;
    }
    run_median[j] = index*sort_interval + min;
  }

  /* finally, fill in the last range/2 elements of run_median HUH??? MXB APRIL 2011*/
  for(j=(dimension-range/2); j<dimension; ++j)
    run_median[j] = index*sort_interval + min;

  free (sort_array);
  return(0);
}

int running_mean(float * input_array, int dimension, int range){ 

  int i, j;
  float mean, inv_range = 1/(float)range;
  float *running_mean = (float*) malloc (dimension * sizeof(float));
  assert (running_mean != NULL);

  mean = input_array[0];
  running_mean[0] = mean;
  for(i=1; i<=range/2; ++i){
    mean += input_array[2*i-1];
    mean += input_array[2*i];
    running_mean[i] = mean/(float)(2*i+1);
  }
  for(i=range/2+1; i<(dimension-range/2); ++i){
    mean = mean + input_array[i+range/2] - input_array[i-range/2-1];
    running_mean[i] = inv_range*mean;
  }
  for(i=(dimension - range/2); i<dimension; ++i){
    mean -= input_array[2*i-dimension-1];
    mean -= input_array[2*i-dimension];
    running_mean[i] = mean/(float)(2*(dimension-i)-1);
  }

  for(i=0; i<dimension; ++i)
    input_array[i] = running_mean[i];

  free (running_mean);
  return(0);
}

