#include <stdio.h>
#define max_w 50
#define max_size 2000         // max length of strings
#define N 10                  // number of closest words that will be shown


typedef struct {
	// model data
  float *M;
  char *vocab;
  char *bestw[N];
  float bestd[N];
  long long words, size;
  int a, b;
  float len, vec[max_size];

} model_data;

model_data processModel(char *file_name);

float findDistance(model_data *, char *, char *);

typedef struct {
	float bestd[N];
	char *bestw[N];
} similar_words;

similar_words findSimilarWords(model_data *, char *);
