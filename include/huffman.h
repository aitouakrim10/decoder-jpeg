#ifndef __HUFFMAN__
#define __HUFFMAN__

# include <stdint.h>
# include <stdbool.h>
# include <stdio.h>
# include "scan_head.h"


/*calcule la taille d'un tas représentant un arbre de Huffman*/
extern uint32_t taille_tab(uint8_t profondeur);


/*calcule la profondeur maximale d'un arbre de Huffman*/
extern uint8_t profondeur_max(uint8_t *data);


/*calcule le nombre de symbols de Huffman d'un arbe*/
extern uint32_t nb_of_huffman_symbols(uint8_t * data);


/*lit un bit dans le flux*/
extern uint8_t return_bit(uint8_t *cour, uint8_t *suiv, uint8_t *remember_index, FILE **fptr);


/*stocke les mots de codes calculé lors du decodage de l'image (option -h)*/
extern void code_already_generated(char * code, uint8_t * symbols, uint32_t lenght_symbols, uint8_t current_symbol, char ** code_list, uint32_t * how_many_codes_done);


/*calcule en lisant dans le flux un symbole de Huffman*/
extern uint8_t decode_byte_huffman(struct TREE * tree, uint8_t *cour, uint8_t *suiv, uint8_t *remember_index, FILE **fptr);


/*calcule en lisant dans le flux un symbole de Huffman et stocke
le mot de code associé (option -h)*/
extern uint8_t decode_byte_huffman_h(struct TREE * tree, uint8_t *cour, uint8_t *suiv, uint8_t *remember_index, FILE **fptr, uint8_t * symbols, uint32_t lenght_symbols, char ** code_liste);


#endif




