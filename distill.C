#include <stdio.h>
#include <math.h>

/*
  distills frequencies into unique ones not harmonically related
*/

// returns the index of the largest value in an array
// and checks that unique!=0

int ibiggest(int n, float * snr, int * unique){
  int biggest = 0;
  while (unique[biggest]!=1) biggest++;
  float largest = snr[biggest];

  for (int i=0;i<n;i++){
    if (snr[i]>largest && unique[i]!=0){
      biggest=i;
      largest=snr[biggest];
    }
  }
  return(biggest);
}

/*
  tests whether test_freq is a multiple of the peak freq
  for all the various harmonics. peak_freq is the SNR of
  the largest amplitude pulse.
*/

int test_harm(float peak_freq, float test_freq, int nh){
  if (nh==0){
    for (int i=1;i<=16;i++) {
      float ratio = test_freq/(i*peak_freq);
      if (ratio>0.999 && ratio< 1.001) return(1);
    }
    return(0);
  }
  // should be another loop in subsequent harms???? 
  if (nh==1){
    for (int i=1;i<=16;i++) {
      for (int j=1;j<=2;j++){
	float ratio = j*test_freq/(i*peak_freq);
	if (ratio>0.999 && ratio< 1.001) return(1);
      }
    }
    return(0);
  }
  if (nh==2){
    for (int i=1;i<=16;i++) {
      for (int j=1;j<=4;j++){
	float ratio = j*test_freq/(i*peak_freq);
	if (ratio>0.999 && ratio< 1.001) return(1);
      }
    }
    return(0);
  }
  if (nh==3){
    for (int i=1;i<16;i++) {
      for (int j=1;j<=8;j++){
      float ratio = j*test_freq/(i*peak_freq);
      if (ratio>0.999 && ratio< 1.001) return(1);
      }
    }
    return(0);
  }
  if (nh==4){
    for (int i=1;i<16;i++) {
      for (int j=1;j<=16;j++){
      float ratio = j*test_freq/(i*peak_freq);
      if (ratio>0.999 && ratio< 1.001) return(1);
      }
    }
    return(0);
  }
}

// unique = 1 means unique frequency, 0 is a harmonic
// Takes four arrays each of dimension n and distills 
// down to unique frequencies, one at a time.
// When a harmonic is found, it sets unique[i] to zero
// returning the number of entities zeroed.

int distill(int n, float * freq, float * snr, int * nh, int * unique, 
	    float test_freq){
  int ikilled=0;
  int kill;
  for (int i=0;i<n;i++){
    kill = 0;
    if (unique[i]!=0) kill = test_harm(test_freq,freq[i], nh[i]);
    if (kill) {ikilled++; unique[i]=0; }
  }
  return(ikilled);
}

void izero(int n, int * u){for (int i=0;i<n;i++)u[i]=0;}
void fzero(int n, float * u){for (int i=0;i<n;i++)u[i]=0;}
void ione(int n, int * u){for (int i=0;i<n;i++)u[i]=1;}
void fone(int n, float * u){for (int i=0;i<n;i++)u[i]=1;}

int main(int argc, char ** argv){

  FILE * fptr = fopen(argv[1],"r");
  int count=0;
  char a_line[128];
  while (fgets(a_line,128,fptr)!=0) count++;
  fclose(fptr);
  printf("File %s contains %d lines\n",argv[1],count);
  float * dms = new float[count];
  float * accs = new float[count];
  float * snrs = new float[count];
  float * periods = new float[count];
  int * nhs = new int[count];
  float * freqs = periods;
  fptr = fopen(argv[1],"r");
  count=0;
  while (fgets(a_line,128,fptr)!=0) {
    sscanf(a_line,"%f %f %f %f %d",&dms[count],&accs[count],
	   &snrs[count],&periods[count],&nhs[count]);
    freqs[count]=1000.0/periods[count];
  count++;
  }
  fclose(fptr);
  int * unique = new int[count];
  ione(count,unique);

  int left = count;

  while (left>0) {
    int ibig = ibiggest(count, snrs, unique);
    if (snrs[ibig]>9.9) {
    printf("largest value is %f %f Hz DM %f acc %4.0f nh %1d ",
	 snrs[ibig], freqs[ibig],dms[ibig],accs[ibig],nhs[ibig]);
    int related = distill(count, freqs, snrs, nhs, unique, 
					 freqs[ibig]);
    printf("%d related ",related);
    left-=related;
    printf("%d now left\n",left);
    unique[ibig]=0; // zero the biggest
    } else left=0;
  }
}
