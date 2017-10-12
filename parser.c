#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct File {
    FILE *fp;
    char *filename;
} files[4];

void open(struct File *file) {
    if ((file->fp = fopen(file->filename, "r")) == NULL) {
        perror(file->filename);
        exit(EXIT_FAILURE);
    }
}

void close(struct File *file) {
    if (fclose(file->fp) != 0) {
        perror("closing file");
        exit(EXIT_FAILURE);
    }
}

void consumeLine(FILE *fp) {
    while ((char) fgetc(fp) != '\n' && !feof(fp)) {}
}

void readStrWithSpace(FILE *fp, char *str) {
    char tmp[20], *tmpPtr = tmp;
    char c;
    fscanf(fp, "%s", str);

    c = (char) fgetc(fp);
    while (c == ' ' && c != '\t') {
        strcat(str, " ");
        fscanf(fp, "%s", tmpPtr);
        strcat(str, tmp);
        c = (char) fgetc(fp);
    }
}

int consDrugArr(FILE *fp, char **drugs) {
    int id;
    char name[50];
    char *namePtr = name;

    while (!feof(fp)) {
        fscanf(fp, "%d", &id);
        readStrWithSpace(fp, namePtr);

        if ((char) fgetc(fp) != '\n') {
            consumeLine(fp);
        }

        char *drug = calloc(50, sizeof(char));
        if (drug == NULL) {
            return id;
        }
        strcpy(drug, name);
        *(drugs + (id-1)) = drug;
    }

    return id;
}

void parseRes1(FILE *fp, char **drugs) {
    char base[20], *basePtr = base;
    int id, baseId;
    float or;

    FILE *out;
    if ((out = fopen("r1.txt", "w")) == NULL) {
        perror("r1");
        return;
    }

    while (!feof(fp)) {
        fscanf(fp, "%s", basePtr);
        fscanf(fp, "%d", &baseId);
        fscanf(fp, "%f", &or);

        base[strlen(base)-4] = 0;

        id = atoi(base + 4);
        fprintf(out, "%s\t0\t%.4f\n", drugs[id-1], or);
        consumeLine(fp);
    }

    if (fclose(out) != 0) {
        perror("r1 close");
        return;
    }
}

void parseRes2(FILE *fp, char **drugs) {
    int addedList[2], baseList[3];
    char added[20], *addedPtr = added;
    int id, baseId;
    float or;

    FILE *out;
    if ((out = fopen("r2.txt", "w")) == NULL) {
        perror("r2");
        return;
    }

    while (!feof(fp)) {
      char baselineExists = 0;
      int base;

        fscanf(fp, "%s", addedPtr);

        if (added[strlen(added)-1] == '}') {
          baselineExists = 1;
          int start = strlen(added)-2;
          char startStr[4];

          while (added[start] != '{') {
            start--;
          }

          int len = strlen(added) - start - 1 > 3 ? 4 : 3;
          strncpy(startStr, added+start+1, len);
          base = atoi(startStr);
        } else {
          fscanf(fp, "%d", &baseId);
        }

        fscanf(fp, "%f", &or);

        added[strcspn(added, "}")] = 0;
        char *addedStrList = strtok(added + 4, ",");
        addedList[0] = atoi(addedStrList);
        addedStrList = strtok(NULL, ",");
        addedList[1] = atoi(addedStrList);

        if (baselineExists) {
          if (addedList[0] == base) {
            fprintf(out, "%s\t%s\t%.4f\n", drugs[addedList[1]-1], drugs[base-1], or);
          } else {
            fprintf(out, "%s\t%s\t%.4f\n", drugs[addedList[0]-1], drugs[base-1], or);
          }
          continue;
        }

        fprintf(out, "%s,%s\t0\t%.4f\n", drugs[addedList[0]-1], drugs[addedList[1]-1], or);
    }

    if (fclose(out) != 0) {
        perror("r2 close");
        return;
    }
}

void parseRes3(FILE *fp, char **drugs) {
    int addedList[3], baseList[3];
    char added[30], *addedPtr = added;
    int id, baseId;
    float or;

    FILE *out;
    if ((out = fopen("r3.txt", "w")) == NULL) {
        perror("r3");
        return;
    }

    while (!feof(fp)) {
	char baselineExists = 0;
    	int base[2], baseLen;

        fscanf(fp, "%s", addedPtr);

        if (added[strlen(added)-1] == '}') {
          baselineExists = 1;
          int start = strlen(added)-2;

          while (added[start] != '{') {
            start--;
          }

          int currId = 0;
          int j = 0;

          for (int i = start+1; i < strlen(added); i++) {
            if (added[i] == ',' || added[i] == '}') {
              base[j] = currId;
              currId = 0;
              j++;
              continue;
            }

            currId *= 10;
            currId += (int) added[i] - '0';
          }
          baseLen = j;
        } else {
          fscanf(fp, "%d", &baseId);
        }

        fscanf(fp, "%f", &or);

        added[strcspn(added, "}")] = 0;
        char *addedStrList = strtok(added + 4, ",");
        addedList[0] = atoi(addedStrList);
        addedStrList = strtok(NULL, ",");
        addedList[1] = atoi(addedStrList);
        addedStrList = strtok(NULL, ",");
        addedList[2] = atoi(addedStrList);

	char baseCount = 0;
        if (baselineExists) {
          for (int i = 0; i < 3; i++) {
	    char isInBase = 0;

	    for (int j = 0; j < baseLen; j++) {
	      if (addedList[i] == base[j]) {
		baseCount++;
		isInBase = 1;
		break;
	      }
	    }

            if (!isInBase) {
              fprintf(out, "%s", drugs[addedList[i]-1]);
              if (i == 2 + baseCount - baseLen) {
                fprintf(out, "\t");
              } else {
                fprintf(out, ",");
              }
            }
          }

          for (int i = 0; i < baseLen-1; i++) {
            fprintf(out, "%s,", drugs[base[i]-1]);
          }

          fprintf(out, "%s\t%.4f\n", drugs[base[baseLen-1]-1], or);
          continue;
        }

        fprintf(out, "%s,%s\t0\t%.4f\n", drugs[addedList[0]-1], drugs[addedList[1]-1], or);
    }

    if (fclose(out) != 0) {
        perror("r3 close");
        return;
    }
}

int main() {
    void (*parseFuncs[3])(FILE *fp, char **drugs) = {parseRes1, parseRes2, parseRes3};
    char *filenames[4] = {"drugs.txt", "res1.txt", "res2.txt", "res3.txt"};
    int drugLength;

    char **drugs = malloc(2000 * sizeof(char *));
    if (drugs == NULL) {
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        files[i].filename = filenames[i];
        open(&files[i]);

        if (i == 0) {
            drugLength = consDrugArr(files[i].fp, drugs);
        } else {
            parseFuncs[i-1](files[i].fp, drugs);
        }

        close(&files[i]);
    }

    free(drugs);
    return 0;
}
