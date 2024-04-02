# include <stdlib.h>
# include <stdint.h>
# include <stdio.h>
# include "scan_head.h"
# include "traitement_MCUs.h"



/*verifie après chaque malloc/calloc si tout s'est bien passé*/
void malloc_verify(char *error_message, void *ptr) {
  if (ptr == NULL) {
    printf("%s\n", error_message);
    exit(EXIT_FAILURE);
  }
}



/*free la structure APP0*/
void free_APP0(struct APP0 *APP0) { free(APP0); }



/*free la structure COM*/
void free_COM(struct COM *COM) { free(COM); }



/*free la structure DQT*/
void free_DQT(struct DQT *DQT) { free(DQT); }



/*free la structure HUFFMAN_TABLE*/
void free_HUFFMAN(struct HUFFMAN_TABLE *HUFFMAN_TABLE) {
  if (HUFFMAN_TABLE->HUFFMAN_TABLE_TREE->option_h) {
    free(HUFFMAN_TABLE->HUFFMAN_TABLE_TREE->huff_symbols);
    for (uint16_t i = 0; i < HUFFMAN_TABLE->HUFFMAN_TABLE_TREE->huff_codes->lenght; i++) {
      free(HUFFMAN_TABLE->HUFFMAN_TABLE_TREE->huff_codes->codes_list[i]);
    }
    free(HUFFMAN_TABLE->HUFFMAN_TABLE_TREE->huff_codes);
  }
  free(HUFFMAN_TABLE -> HUFFMAN_TABLE_TREE); 
  free(HUFFMAN_TABLE); 
}



/*free la structure SOF0*/
void free_SOF0(struct SOF0 *SOF0) {
  for (uint8_t i = 0; i < SOF0->SOF0_nb_components; i++) {
    free(SOF0->SOF0_data_components[i]);
  }
  free(SOF0);
}



/*free la structure SOS*/
void free_SOS(struct SOS *SOS) {
  for (uint8_t i = 0; i < SOS->SOS_nb_components; i++) {
    free(SOS->SOS_data_components[i]);
  }
  free(SOS);
}



/*free la structure header*/
void free_header(struct HEADER *header) {
  free_APP0(header->ptr_APP0);
  if (header->is_COM_present) {
    free_COM(header->ptr_COM);
  }
  free_SOF0(header->ptr_SOF0);
  free_SOS(header->ptr_SOS);
  for (uint8_t i = 0; i < header->dqt_number; i++) {
    if (header->did_I_malloc_this_dqt_table[i]) {
      free_DQT(header->ptr_DQT[i]);
    }
  }
  for (uint8_t i = 0; i < 4; i++) {
    if (header->did_I_malloc_this_dht_table[i]) {
      free_HUFFMAN(header->ptr_HUFFMAN_TABLE[i]);
    }
  }
  free(header);
}



/*free la structure group_MCU*/
void free_group_MCU(struct group_MCU * group_MCU) {
  uint8_t nb_Y_succ = group_MCU->group_MCU[0][0]->nb_Y_succ;
  for (uint32_t i = 0; i < group_MCU -> nb_MCU_height; i ++) {
    for (uint32_t j = 0; j < group_MCU -> nb_MCU_width; j ++) {
      for (uint8_t k = 0; k < nb_Y_succ; k++) {
        free(group_MCU -> group_MCU[i][j] ->MCU_Y_blocs[k]);
      }
      free(group_MCU->group_MCU[i][j]);
    }
    free(group_MCU -> group_MCU[i]);
  }
  free(group_MCU);
}



/*free la structure group_RGB*/
void free_group_RGB(struct group_RGB * group_RGB) {
  uint8_t fin = group_RGB ->nb_RGB_succ;
  for (uint32_t i = 0; i < group_RGB -> nb_RGB_height; i ++) {
    for (uint32_t j = 0; j < group_RGB -> nb_RGB_width; j ++) {
      for (uint8_t k = 0; k < fin; k++) {
        free(group_RGB -> group_RGB[i][j]->RGB_RGB_blocs[k]);
      }
      free(group_RGB -> group_RGB[i][j]);
    }
    free(group_RGB -> group_RGB[i]);
  }
  free(group_RGB);
}