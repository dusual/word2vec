//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "distance.h"

const char sp = ' ';

model_data processModel(char *file_name){
	printf("Beginning process model...");
    model_data result;
	int a, b;
	float len;
	FILE *f = fopen(file_name, "rb");
 	if (f == NULL) {
 		printf("Input file not found\n");
 		return result;
 	}
 	fscanf(f, "%lld", &result.words);
 	fscanf(f, "%lld", &result.size);
    printf("assigning memory to vocab and M datastructures...");
    result.vocab = (char *)malloc((long long)result.words * max_w * sizeof(char));
    for (a = 0; a < N; a++) result.bestw[a] = (char *)malloc(max_size * sizeof(char));
 	for (a = 0; a < N; a++) result.bestw[a] = (char *)malloc(max_size * sizeof(char));
 	result.M = (float *)malloc((long long)result.words * (long long)result.size * sizeof(float));
 	if (result.M == NULL) {
 		printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)result.words * result.size * sizeof(float) / 1048576, result.words, result.size);
 		return result;
 	}
    printf("processing result.words...");
	for (b = 0; b < result.words; b++) {
		a = 0;
		while (1) {
			result.vocab[b * max_w + a] = fgetc(f);
			if (feof(f) || (result.vocab[b * max_w + a] == ' ')) break;
			if ((a < max_w) && (result.vocab[b * max_w + a] != '\n')) a++;
	     	}
	     	result.vocab[b * max_w + a] = 0;
	     	for (a = 0; a < result.size; a++) fread(&result.M[a + b * result.size], sizeof(float), 1, f);
	     	len = 0;
	     	for (a = 0; a < result.size; a++) len += result.M[a + b * result.size] * result.M[a + b * result.size];
	     	len = sqrt(len);
	     	for (a = 0; a < result.size; a++) result.M[a + b * result.size] /= len;
	}
   	fclose(f);
	result.a = a;
	result.b = b;
	result.len = len;
	return result;
}

int countTillNonNull(char** arg){
    int count = 0;
    while (arg[count++] != '\0');
    return count - 1;
}

char** tokenize(char* word, const char ch){
    char* result[5];
    char** tmp;
    char* token = strtok(word, &ch);
    int count = 0;
    while (token) {
        result[count++] = token;
        token = strtok(NULL, " ");
    }
    result[count] = NULL;
    return result;
}

int* findPositionInVocab(char *vocab, char *word, long long words){
    char** st = tokenize(word, sp);
    int cn = countTillNonNull(st);
    
    int a, b;
    int bi[5];
    for (a = 0; a < cn; a++) {
      for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
      if (b == words) b = -1;
      bi[a] = b;	// bi[i] contains the position of the word in vocabulary.
      printf("\nWord: %s  Position in vocabulary: %d\n", st[a], bi[a]);
      if (b == -1) {
        printf("Out of dictionary word!\n");
        break;
      }
    }
    return bi;
}

float findDistance(model_data *modelData, char *word1, char *word2){
	float dist = 0.0;
	long long words = modelData->words;
	int c;
	int a = modelData->a;
	int b = modelData->b;
    char** word1_split = tokenize(word1, sp);
	char** word2_split = tokenize(word2, sp);
	int cn1 = countTillNonNull(word1_split);
	int cn2 = countTillNonNull(word2_split);
	int* tmp = findPositionInVocab(modelData->vocab, word1, words);
    int bi1[cn1];
    for (b = 0; b < cn1; b++) bi1[b] = tmp[b];
	int* bi2 = findPositionInVocab(modelData->vocab, word2, words);
	float *M = modelData->M;
	long long size = modelData->size;
	float* vec = modelData->vec;
    float len = modelData->len;
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn1; b++) {
      		if (bi1[b] == -1) continue;
      		for (a = 0; a < size; a++) vec[a] += M[a + bi1[b] * size];	
    }
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];	// len is dot produt.
    len = sqrt(len);
    for (a = 0; a < size; a++) vec[a] /= len;
    dist = 0.0;
	for (b = 0; b < cn2; b++) {
      		if (bi2[b] == -1) continue;
      		for (a = 0; a < size; a++) dist += vec[a] * M[a + bi2[b] * size];	
    }
	return dist;
}

similar_words findSimilarWords(model_data *modelData, char* word1){
    long long words = modelData->words;
    int c;
    int a = modelData->a;
    int b = modelData->b;
    char *vocab = modelData->vocab;
    char **word1_split = tokenize(word1, sp);
    int cn = countTillNonNull(word1_split);
    int* bi = findPositionInVocab(vocab, word1, words);
    float *M = modelData->M;
    long long size = modelData->size;
    float* vec = modelData->vec;
    similar_words result;
    float bestd[N];
	char* bestw[N];
    float len = modelData->len;
    float dist;
    int d;
    for (a = 0; a < N; a++) {bestd[a] = -1;}
    for (a = 0; a < N; a++) {bestw[a] = modelData->bestw[a]; bestw[a][0] = 0;}
    
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn; b++) {
      		if (bi[b] == -1) continue;
      		for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size]; 
    }
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];	// len is dot produt.
    len = sqrt(len);
    // calculate vec
    for (a = 0; a < size; a++) vec[a] /= len;
    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    // calculate distance betweent two words: which words?
    for (c = 0; c < words; c++) {
      a = 0;
      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
      if (a == 1) continue;
      dist = 0;
      for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
      for (a = 0; a < N; a++) {
        if (dist > bestd[a]) {
          for (d = N - 1; d > a; d--) {
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          bestd[a] = dist;
          strcpy(bestw[a], &vocab[c * max_w]);
          break;
        }
      }
    }
    for (a = 0; a < N; a++) { result.bestd[a] = bestd[a]; result.bestw[a] = bestw[a];}
    return result;
}

typedef struct {
	char st1[max_w];
	int a;
} inputData;

inputData takeInput(){
	 // init the bestd and bestw. bestd => distance, bestw => word
	inputData res;
	printf("Enter word or sentence (EXIT to break): ");
	res.a = 0;
	while (1) {
		res.st1[res.a] = fgetc(stdin);            //st1 contains input text
		if ((res.st1[res.a] == '\n') || (res.a >= max_size - 1)) {
			res.st1[res.a] = 0;
			break;
		}
		res.a++;
	}
	return res;
}


int main(int argc, char **argv) {
  printf("Just in main...");
  char *file_name;
  int a;
  model_data res;
  inputData input;
  similar_words sim;
    float distance;
  if (argc < 2) {
    printf("Usage: ./distance <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }
  printf("Before copying %s", argv[1]);
  file_name = argv[1];
  res = processModel(file_name);
  if (res.M == NULL){
      return -1;
  }
  while (1) {
    input = takeInput();
    if (!strcmp(input.st1, "EXIT")) break;
    
    printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
//    sim = findSimilarWords(&res, input.st1);
//    for (a = 0; a < N; a++) printf("%50s\t\t%f\n", sim.bestw[a], sim.bestd[a]);
    distance = findDistance(&res, input.st1, "india");
    printf("%50s\t\t%f\n", input.st1, distance);
  }
  return 0;
}
