#include <stdio.h>
#include <time.h>
#include <string.h>

#define FOPEN64 0

int WebTime(char *name);

struct Logfile {
  
  Logfile() {
    in = NULL;
    date = 0;
  }

  ~Logfile() {
    if (in) fclose (in);
    in = NULL;
  }

  void OpenFile(char *name) {
#if FOPEN64
      in = fopen64(name, "r");
#else
    in = fopen(name, "r");
#endif
    date = -1;
    if (in == NULL) date = 0;
    ReadNext();
  } 

  void ReadNext() {
    if (!date) return;

    if (feof(in)) {
      date = 0;
      return;
    }
    
    buf[0] = 0;
    fgets(buf, 4096, in);
    
    // eliminate monitor scripts :)
    if (strstr(buf, "monitor")) {
      ReadNext();
      return;
    }

    char *time = strchr(buf, '[');

    if (!time) {
      ReadNext();
      return;
    }
    
    time ++;
    char *end = strchr(time, ' ');
    if (!end) {
      ReadNext();
      return;
    }

    // copy from time to end into datebuf
    strncpy(datebuf, time, end - time);
    datebuf[end - time] = 0;
    
    date = WebTime(datebuf);
  };

  FILE *in;
  char buf[4096];
  char datebuf[512];
  long date;
};

int WebTime(char *name)
{
  int day, year, h, m, s;
  int month;
  struct tm t;
  char mo[100];
  char *tmp;
  int val;

  while (tmp = strchr(name, '/')) {
    tmp[0] = ' ';
  }
  while (tmp = strchr(name, ':')) {
    tmp[0] = ' ';
  }
  
  val = sscanf(name, "%d %s %d %d %d %d", &day, mo, &year, &h, &m, &s);

  month = 0;
  if (!strcmp(mo, "Feb")) month = 1;
  if (!strcmp(mo, "Mar")) month = 2;
  if (!strcmp(mo, "Apr")) month = 3;
  if (!strcmp(mo, "May")) month = 4;
  if (!strcmp(mo, "Jun")) month = 5;
  if (!strcmp(mo, "Jul")) month = 6;
  if (!strcmp(mo, "Aug")) month = 7;
  if (!strcmp(mo, "Sep")) month = 8;
  if (!strcmp(mo, "Oct")) month = 9;
  if (!strcmp(mo, "Nov")) month = 10;
  if (!strcmp(mo, "Dec")) month = 11;

  memset(&t, 0, sizeof(struct tm));
  t.tm_sec = s;
  t.tm_min = m;
  t.tm_hour = h;
  t.tm_mon = month;
  t.tm_year = year-1900;
  t.tm_mday = day;

  return mktime(&t);
}

int main(int argc, char **argv)
{
  int i;
  int nf = argc - 1;
  Logfile *lf = new Logfile[nf];

  for (i = 0; i < nf; i++) {
    lf[i].OpenFile(argv[i + 1]);
    if (!lf[i].in) {
      fprintf(stderr, "Warning: %s failed to open.\n", argv[i + 1]);
    }
  }

  // now, while there's a valid file, output the best line we find in the set
  long valid;
  do {
    valid = false;
    int minindex = -1; 
    int min = 0x7fffFFFF;
    for (i = 0; i < nf; i++) {
      if (!lf[i].date) continue;
	
      if (lf[i].date > 0) valid = true;

      if (lf[i].date < min) {
	min = lf[i].date;
	minindex = i;
      }

      valid = true;
    }
    if (minindex != -1) {
      fputs(lf[minindex].buf, stdout);
      lf[minindex].ReadNext();
    }

  } while (valid);

  delete[] lf;

  return 0;
}



