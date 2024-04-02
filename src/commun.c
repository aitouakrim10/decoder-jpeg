# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include "commun.h"
# include "memory.h"
# include "idct_dqtz.h"
# include "scan_head.h"



/*affiche les symbols de Huffman uttilisés et leurs mots de code*/
void print_tas_huffman(struct TREE * tree) {
  uint32_t lenght = tree->huff_codes->lenght;
  for (uint32_t i = 0; i < lenght; i++) {
    if (tree->huff_codes->codes_list[i][0]) {
      printf("Path  :   %s  symbol  :   %x\n", tree->huff_codes->codes_list[i], tree->huff_symbols->symbols[i]);
    }
	}
}



/*affiche les coefficients d'un bloc 8x8 (représenté comme vecteur 64)
-le bool hex_or_dec choisit le mode (hex/dec) d'affichage des coefficients
*/
void print_result_decoded(struct bloc_spatial *tab, bool hex_or_dec) {
  uint16_t i = 0;
  if (hex_or_dec) {
    for (i = 0; i < 64; i++) {
      printf("%x ", tab->bloc_data[i]);
    }
    printf("\n");
  } else {
    for (i = 0; i < 64; i++) {
      printf("%u ", tab->bloc_data[i]);
    }
    printf("\n");
  }
}



/*affiche un vecteur d'octets de taille 64 sous forme 8x8*/
void afficher_vecteur_uint8(uint8_t *vecteur) {
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      printf("%x ", vecteur[(i << 3) + j]);
    }
    printf("\n");
  }
}



/*recupère le Vertical Sampling Factor d'une composante (Y,Cb,Cr)*/
int8_t get_sampling_vert(struct HEADER *header, uint8_t component) {
  return header->ptr_SOF0->SOF0_data_components[component]
      ->vertical_sampling_factor;
}



/*recupère le Horizontal Sampling Factor d'une composante (Y,Cb,Cr)*/
int8_t get_sampling_horz(struct HEADER *header, uint8_t component) {
  return header->ptr_SOF0->SOF0_data_components[component]
      ->horizontal_sampling_factor;
}



/*recupère le nombre de blocs frequentiels 8x8 par ligne*/
uint32_t get_nb_bloc_frequ_width(struct HEADER *header) {
  return (header->ptr_SOF0->SOF0_width + 7) / 8;
}



/*recupère le nombre de blocs frequentiels 8x8 par colonne*/
uint32_t get_nb_bloc_frequ_height(struct HEADER *header) {
  return (header->ptr_SOF0->SOF0_height + 7) / 8;
}



/*recupère l'indice de la table DQT associé à une composante*/
uint8_t get_iQ_dqt(struct HEADER *header, uint8_t component) {
  return header->ptr_SOF0->SOF0_data_components[component]->index_Q;
}



/*recupère l'indice de l'arbre de huffman associé à une composante*/
struct TREE * get_tree(struct HEADER *header, uint8_t iH, uint8_t type) {
  return header -> ptr_HUFFMAN_TABLE[(type << 1) + iH]->HUFFMAN_TABLE_TREE;
}



/*recupère l'indice de l'arbre AC associé à une composante*/
uint8_t get_index_HUFF_AC(struct HEADER *header, uint8_t component) {
  return header->ptr_SOS->SOS_data_components[component]->index_H_AC;
}



/*recupère l'indice de l'arbre DC associé à une composante*/
uint8_t get_index_HUFF_DC(struct HEADER *header, uint8_t component) {
  return header->ptr_SOS->SOS_data_components[component]->index_H_DC;
}
