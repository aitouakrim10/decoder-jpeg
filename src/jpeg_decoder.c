# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include "check_file.h"
# include "traitement_MCUs.h"
# include "scan_head.h"
# include "ppm.h"
# include "memory.h"


/*fonction main du decodeur JPEG*/
int main(int argc, char *argv[]) {
  bool option_h_present = false;
  bool option_m_present = false;
  char *inputFilePath = NULL;
  bool did_I_take_the_file_input = false;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-' && argv[i][1] == 'h') {
      option_h_present = true;
    } else if (argv[i][0] == '-' && argv[i][1] == 'm') {
      option_m_present = true;
    } else {
      if (!did_I_take_the_file_input) {
        inputFilePath = argv[i];
        did_I_take_the_file_input = true;
      } else {
        fprintf(stderr, "Usage : jpeg2ppm [-m] [-h] <Path_to_image.jp[e]g>\n");
        exit(EXIT_SUCCESS);
      }
    }
  }
  if (inputFilePath == NULL) {
    fprintf(stderr, "Usage : jpeg2ppm [-m] [-h] <Path_to_image.jp[e]g>\n");
    exit(EXIT_SUCCESS);
  }
  check_if_input_valid(inputFilePath);
  FILE *fptr = fopen(inputFilePath, "r");
  if (fptr == NULL) {
    fprintf(stderr, "Fichier introuvable : %s\n", inputFilePath);
    exit(EXIT_FAILURE);
  }
  uint16_t *current = malloc(sizeof(uint16_t));
  malloc_verify("Problem while allocating for current", current);
  struct HEADER *header = scan_header(&fptr, &current, option_h_present);
  uint8_t nb_components = header ->ptr_SOF0 ->SOF0_nb_components;
  uint8_t *suiv = malloc(sizeof(uint8_t));
  malloc_verify("Problem while allocating for suiv", suiv);
  *suiv = (uint8_t)((*current) & 0x00ff);
  uint8_t *cour = malloc(sizeof(uint8_t));
  malloc_verify("Problem while allocating for cour", cour);
  *cour = (uint8_t)(((*current) & 0xff00) >> 8);
  struct group_MCU *group_MCU;
  if (!option_h_present) {
    group_MCU = decodeur_image(header, cour, suiv, &fptr);
  } else {
    group_MCU = decodeur_image_h(header, cour, suiv, &fptr);
  }
  if (option_h_present) {
    test_header(header);
  }
  fclose(fptr);
  if (option_m_present) {
   test_group_MCU(header, group_MCU, true);
  }
  char outputFilePath[256];
  gen_outputFilePath(inputFilePath, nb_components, outputFilePath);
  genere_image(outputFilePath, header, group_MCU);
  free(current);
  free(suiv);
  free(cour);
  return EXIT_SUCCESS;
}
