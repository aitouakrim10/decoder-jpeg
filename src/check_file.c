# include <stdint.h>
# include <string.h>
# include <stdio.h>
# include <stdbool.h>
# include <stdlib.h>
#include <unistd.h>


/*verifie si l'extension du fichier d'entrée est jp[e]g ou non*/
void check_if_input_valid(const char *filename) {
    char * point;
    bool is_valid = true;
    if((point = strrchr(filename,'.')) != NULL ) {
        if(strcmp(point, ".jpeg")) {
          is_valid = false;
        } else {
          if (point[5]) {
            is_valid = false;
          }
        }
        if (!is_valid) {
          is_valid = true;
          if(strcmp(point, ".jpg")) {
            is_valid = false;
          } else {
            if (point[4]) {
              is_valid = false;
            }
          }
        }
        if (!is_valid) {
          fprintf(stderr, "The input file extension should be .jpeg or .jpg\n");
          exit(EXIT_SUCCESS);
        }
    } else {
      fprintf(stderr, "The input file extension should be .jpeg or .jpg\n");
      exit(EXIT_SUCCESS);
    }
}



/*génére le Path et le nom de l'image*/
void gen_outputFilePath(char * inputFilePath, uint8_t nb_components, char * outputFilePath) {
  getcwd(outputFilePath, 255);
  uint8_t len = strlen(outputFilePath);
  outputFilePath[len] = 47;
  outputFilePath[len+1] = 0;
  strcat(outputFilePath, inputFilePath);
  char *lastDot = strrchr(outputFilePath, '.');
  if (nb_components == 1) {
    strcpy(lastDot, ".pgm");
  } else {
    strcpy(lastDot, ".ppm");
  }
}
