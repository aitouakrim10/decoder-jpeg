#ifndef __TRAITEMENT_MCUS__
#define __TRAITEMENT_MCUS__

# include <stdint.h>
# include <stdio.h>
# include <stdbool.h>
# include "scan_head.h"
# include "idct_dqtz.h"


/*représente une MCU (Y, Cb, Cr)*/
struct MCU {
  uint8_t nb_Y_succ;
  struct bloc_spatial MCU_Cb_bloc;
  struct bloc_spatial MCU_Cr_bloc;
  struct bloc_spatial *MCU_Y_blocs[];
};


/*contient toutes les MCUs*/
struct group_MCU {
  uint32_t nb_MCU_height;
  uint32_t nb_MCU_width;
  uint8_t nb_components;
  struct MCU **group_MCU[];
};


/*c'est ce qu'on obtient en convertissant une MCU*/
struct RGB {
  uint8_t nb_RGB_succ;
  struct bloc_spatial_x3 *RGB_RGB_blocs[];
};


/*c'est ce qu'on obtient en convertissant toutes les MCUs*/
struct group_RGB {
  uint32_t nb_RGB_height;
  uint32_t nb_RGB_width;
  uint8_t nb_RGB_succ;
  struct RGB **group_RGB[];
};


/*Zigzag inverse*/
extern void zig_zag(int32_t *zig_zag, int8_t size, int32_t *vecteur);


/*met à zero les elements d'un vecteur de taille 64*/
extern void vide_buffer_int32(int32_t *buffer);


/*affiche sur la stdout les MCU de l'image (option -m)
- le bool hex_or_dec sert à choisir le mode d'affichage en hex ou en dec
*/
extern void test_group_MCU(struct HEADER *header, struct group_MCU *group_MCU, bool hex_or_dec);


/*calcule l'indice d'un element dans sa classe de magnitude*/
extern uint16_t decode_indice_magnitude(uint8_t poid_faible_huff_symbole, uint8_t *cour, uint8_t *suiv, uint8_t *remember_index, FILE **fptr);


/*calcule à partir de la classe et l'indice, l'element associé*/
extern int32_t decode_magnitude(uint8_t classe_magnitude, uint16_t indice_magnitude);


/*calcule à partir du poid faible d'un certain symbole de huffman
l'element associé dans la représentation par magnitude */
extern int32_t decode_coeff_mcu(uint8_t poid_faible_huff_symbole, uint8_t *cour, uint8_t *suiv, uint8_t *remember_index, FILE **fptr);


/*decode un bloc de 64 coefficient*/
extern void bloc_decoding(struct bloc_frequ *result, int32_t *difference_DC, uint8_t *remember_index, uint8_t *cour, uint8_t *suiv, struct TREE * tree_DC, struct TREE * tree_AC, FILE **fptr);


/*decode un bloc de 64 coefficient et calcule en même temps 
les mots de code de huffman des symbols de huffman utilisés
(option -h)
*/
extern void bloc_decoding_h(struct bloc_frequ *result, int32_t *difference_DC, uint8_t *remember_index, uint8_t *cour, uint8_t *suiv, struct TREE * tree_DC, struct TREE * tree_AC, FILE **fptr);


/*decode les données brutes*/
extern struct group_MCU *decodeur_image(struct HEADER *header, uint8_t *cour, uint8_t *suiv, FILE **fptr);


/*decode les données brutes et calcule en même temps 
les mots de code de huffman des symbols de huffman utilisés
(option -h)*/
extern struct group_MCU *decodeur_image_h(struct HEADER *header, uint8_t *cour, uint8_t *suiv, FILE **fptr);


/*convertion d'une MCU en une bloc RGB*/
extern void convert_MCU_to_RGB(struct RGB * RGB, struct MCU * MCU, struct HEADER * header);


/*cnvertion des MCUs en RGBs*/
extern struct group_RGB *convert_group_MCU_to_group_RGB(struct group_MCU * group_MCU, struct HEADER * header);


#endif