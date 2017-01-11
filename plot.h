
class datum {

 public:
  datum();
  float value;
  bool defined;
  char * ascii;
};

class data_array {

 public:
  data_array();
  //  int * ci;
  //  int * selected;
  //  int * plotted;
  char * name;
  datum * vals;
  int ndatum;
};

class results {
  
 public:
  results();
  results(char * filename);   // Load from a file filename
  results(int ncols, int nlines, char * fname);
  int profile(int ptno);
  int pgplot();
  int closest(int xvar, int yvar, float x, float y,
	      float xmin, float xmax, float ymin, float ymax, int xlog, int ylog);
  char * label;
  data_array * popdata;
  float * return1array(int arrno, int * nd, int xlogged);
  void return2arrays(int arrno1, int arrno2, float * x1, float * y1, 
		     int * id, int * nd,
		     int xlogged, int ylogged);
  float getsinglevalue(int arrno, int whichpt, int logged);
  int nparms;
  int ndatum;
  int * selected;
  int * ci;
  int * plotted;
  int * deleted;
  int * symbol;

  void show();

  void write(char * filename);
};

float themin(float *, int);
float themax(float *, int);
float themin5(float *, int);
float themax5(float *, int);

