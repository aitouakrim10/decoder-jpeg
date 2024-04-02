# include <stdint.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include "scan_head.h"



/*calcule la taille d'un tas représentant un arbre de Huffman*/
uint32_t taille_tab(uint8_t profondeur) {
  uint32_t taille = 0;
  for (uint8_t i = 0; i <= profondeur; i++) {
    taille += (1 << i);
  }
  return taille;
}



/*calcule la profondeur maximale d'un arbre de Huffman*/
uint8_t profondeur_max(uint8_t *data) {
  uint8_t profondeur = 0;
  for (uint8_t i = 0; i < 16; i++) {
    if (data[i] != 0) {
      profondeur = i;
    }
  }
  profondeur++;
  return profondeur;
}



/*calcule le nombre de symbols de Huffman d'un arbe*/
uint32_t nb_of_huffman_symbols(uint8_t * data) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < 16; i++) {
    sum += data[i];
  }
  return sum;
}



/*lit un bit dans le flux*/
uint8_t return_bit(uint8_t *cour, uint8_t *suiv, uint8_t *remember_index, FILE **fptr) {
  if (*remember_index >= 8) {
    if (*cour == 0xff) {
      fread(suiv, sizeof(uint8_t), 1, *fptr);
      *cour = *suiv;
      fread(suiv, sizeof(uint8_t), 1, *fptr);
      *remember_index = 0;
    } else {
      *cour = *suiv;
      fread(suiv, sizeof(uint8_t), 1, *fptr);
      *remember_index = 0;
    }
  }
  uint8_t res = (((*cour << (*remember_index)) & 0x80) >> 7);
  *remember_index = ((*remember_index) + 1);
  return res;
}



/*stocke les mots de codes calculé lors du decodage de l'image (option -h)*/
void code_already_generated(char * code, uint8_t * symbols, uint32_t lenght_symbols, uint8_t current_symbol, char ** code_list, uint32_t * how_many_codes_done) {
  uint32_t code_index = 0;
  for (uint32_t i = 0; i < lenght_symbols; i++) {
    if (symbols[i] == current_symbol) {
      code_index = i;
      break;
    }
  }
  if (strcmp(code, code_list[code_index])) {
    *how_many_codes_done = (*how_many_codes_done + 1);
    strcpy(code_list[code_index], code);
  }
}



/*calcule en lisant dans le flux un symbole de Huffman*/
uint8_t decode_byte_huffman(struct TREE * tree, uint8_t *cour, uint8_t *suiv, uint8_t *remember_index, FILE **fptr) {
  uint32_t tree_index = 0;
  uint8_t current_bit;
  while (true) {
    current_bit = return_bit(cour, suiv, remember_index, fptr);
    if (current_bit == 0) {
      tree_index = (tree_index << 1) + 1;
    } else {
      tree_index = (tree_index << 1) + 2;
    }
    if (tree->tree[tree_index] != 0xffff) {
      return (uint8_t)(tree->tree[tree_index]);
    }
  }
}



/*calcule en lisant dans le flux un symbole de Huffman et stocke
le mot de code associé (option -h)*/
uint8_t decode_byte_huffman_h(struct TREE * tree, uint8_t *cour, uint8_t *suiv,
                            uint8_t *remember_index, FILE **fptr, uint8_t * symbols, uint32_t lenght_symbols, char ** code_list) {
  uint32_t tree_index = 0;
  uint8_t current_bit;
  if (tree->how_many_codes_done == lenght_symbols) {
    while (true) {
    current_bit = return_bit(cour, suiv, remember_index, fptr);
    if (current_bit == 0) {
      tree_index = (tree_index << 1) + 1;
    } else {
      tree_index = (tree_index << 1) + 2;
    }
    if (tree->tree[tree_index] != 0xffff) {
      return (uint8_t)(tree->tree[tree_index]);
    }
  }
  } else {
    char code[17];
    code[16] = 0;
    uint8_t code_index = 0;
    while (true) {
      current_bit = return_bit(cour, suiv, remember_index, fptr);
      code[code_index] = (current_bit + 48);
      code_index++;
      if (current_bit == 0) {
        tree_index = (tree_index << 1) + 1;
      } else {
        tree_index = (tree_index << 1) + 2;
      }
      if (tree->tree[tree_index] != 0xffff) {
        code[code_index] = 0;
        code_already_generated(code, symbols, lenght_symbols, (uint8_t)tree->tree[tree_index], code_list, &(tree->how_many_codes_done));
        return (uint8_t)(tree->tree[tree_index]);
      }
    }
  }
}
