# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <math.h>
# include "idct_dqtz.h"
# include "commun.h"
# include "memory.h"
# include "scan_head.h"
# include "traitement_MCUs.h"
# include "huffman.h"


/*Zigzag inverse*/
void zig_zag(int32_t *zig_zag, int8_t size, int32_t *vecteur) {
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t vect_indice = 1;
  zig_zag[i * size + j] = vecteur[0];
  while (true) {
    if (i == 0) {
      j++;
      zig_zag[i * size + j] = vecteur[vect_indice];
      vect_indice++;
      while (j != 0) {
        j--;
        i++;
        zig_zag[i * size + j] = vecteur[vect_indice];
        vect_indice++;
      }
      if (i != size - 1) {
        i++;
        zig_zag[i * size + j] = vecteur[vect_indice];
        vect_indice++;
        while (i != 0) {
          j++;
          i--;
          zig_zag[i * size + j] = vecteur[vect_indice];
          vect_indice++;
        }
      } else {
        continue;
      }
    } else {
      j++;
      zig_zag[i * size + j] = vecteur[vect_indice];
      vect_indice++;
      if (j != size - 1) {
        while (j != size - 1) {
          j++;
          i--;
          zig_zag[i * size + j] = vecteur[vect_indice];
          vect_indice++;
        }
        i++;
        zig_zag[i * size + j] = vecteur[vect_indice];
        vect_indice++;
        while (i != size - 1) {
          j--;
          i++;
          zig_zag[i * size + j] = vecteur[vect_indice];
          vect_indice++;
        }
      } else {
        break;
      }
    }
  }
}



/*met à zero les elements d'un vecteur de taille 64*/
void vide_buffer_int32(int32_t *buffer) {
  for (uint8_t i = 0; i < 64; i++) {
    buffer[i] = 0;
  }
}



/*affiche sur la stdout les MCU de l'image (option -m)
- le bool hex_or_dec sert à choisir le mode d'affichage en hex ou en dec
*/
void test_group_MCU(struct HEADER *header, struct group_MCU *group_MCU,
                    bool hex_or_dec) {
  int8_t vert_sampY = get_sampling_vert(header, 0);
  int8_t horz_sampY = get_sampling_horz(header, 0);
  for (uint32_t i = 0; i < group_MCU->nb_MCU_height; i++) {
    for (uint32_t j = 0; j < group_MCU->nb_MCU_width; j++){
      printf("---------------------\n");
      printf("Debut MCU (%u, %u) :\n", i, j);
      for (uint8_t v = 0; v < vert_sampY; v++) {
        for (uint8_t h = 0; h < horz_sampY; h++) {
          printf("Y_(%u,%u) : \n", v, h);
          print_result_decoded(
              (group_MCU->group_MCU[i][j]->MCU_Y_blocs[v * horz_sampY + h]),
              hex_or_dec);
          printf("\n");
        } 
      }
      if (header->ptr_SOF0->SOF0_nb_components == 3) {
          printf("Cb : \n");
          print_result_decoded(
              (&group_MCU->group_MCU[i][j]->MCU_Cb_bloc),
              hex_or_dec);
          printf("\n");


          printf("Cr : \n");
          print_result_decoded(
              (&group_MCU->group_MCU[i][j]->MCU_Cr_bloc),
              hex_or_dec);
          printf("\n");
      }
    }
  }
}



/*calcule l'indice d'un element dans sa classe de magnitude*/
uint16_t decode_indice_magnitude(uint8_t poid_faible_huff_symbole,
                                 uint8_t *cour, uint8_t *suiv,
                                 uint8_t *remember_index, FILE **fptr) {
  if (!poid_faible_huff_symbole) {
    return 0;
  }
  uint16_t return_value = 0;
  uint8_t current_bit;
  for (uint8_t i = 0; i < poid_faible_huff_symbole - 1; i++) {
    current_bit = return_bit(cour, suiv, remember_index, fptr);
    return_value = ((return_value + current_bit) << 1);
  }
  current_bit = return_bit(cour, suiv, remember_index, fptr);
  return_value += current_bit;
  return return_value;
}



/*calcule à partir de la classe et l'indice, l'element associé*/
int32_t decode_magnitude(uint8_t classe_magnitude, uint16_t indice_magnitude) {
  if (indice_magnitude >= (1 << (classe_magnitude - 1))) {
    return indice_magnitude;
  } else {
    return -((1 << classe_magnitude) - 1 - indice_magnitude);
  }
}



/*calcule à partir du poid faible d'un certain symbole de huffman
l'element associé dans la représentation par magnitude */
int32_t decode_coeff_mcu(uint8_t poid_faible_huff_symbole, uint8_t *cour,
                         uint8_t *suiv, uint8_t *remember_index, FILE **fptr) {
  uint16_t magnitude_index = decode_indice_magnitude(
      poid_faible_huff_symbole, cour, suiv, remember_index, fptr);
  return decode_magnitude(poid_faible_huff_symbole, magnitude_index);
}



/*decode un bloc de 64 coefficient*/
void bloc_decoding(struct bloc_frequ *result, int32_t *difference_DC,
                   uint8_t *remember_index, uint8_t *cour, uint8_t *suiv,
                   struct TREE * tree_DC, struct TREE * tree_AC, FILE **fptr) {
  uint8_t result_current_index = 1;
  uint8_t return_value = 0;
  int32_t coeff_mcu = 0;
  uint8_t poid_faible_huff_symbole = 0;
  uint8_t poid_fort_huff_symbole = 0;
  return_value = decode_byte_huffman(tree_DC, cour, suiv, remember_index, fptr);
  poid_faible_huff_symbole = return_value & 0x0f;
  coeff_mcu = decode_coeff_mcu(poid_faible_huff_symbole, cour, suiv,
                              remember_index, fptr);
  result->bloc_data[0] = coeff_mcu + *difference_DC;
  *difference_DC = result->bloc_data[0];
  while (true) {
    if (result_current_index >= 64) {
      break;
    }
    return_value =
        decode_byte_huffman(tree_AC, cour, suiv, remember_index, fptr);
    if (return_value == 0x00) {
      break;
    } else if (return_value == 0xf0) {
      result_current_index += 16;
    } else if (return_value && 0x0f != 0) {
      poid_fort_huff_symbole = (return_value & 0xf0) >> 4;
      poid_faible_huff_symbole = return_value & 0x0f;
      result_current_index += poid_fort_huff_symbole;
      coeff_mcu = decode_coeff_mcu(poid_faible_huff_symbole, cour, suiv,
                                  remember_index, fptr);
      result->bloc_data[result_current_index] = coeff_mcu;
      result_current_index++;
    } else {
      fprintf(stderr, "Decoding Problem : found invalid symbol : %x\n",return_value);
      exit(EXIT_FAILURE);
    }
  }
}



/*decode un bloc de 64 coefficient et calcule en même temps 
les mots de code de huffman des symbols de huffman utilisés
(option -h)
*/
void bloc_decoding_h(struct bloc_frequ *result, int32_t *difference_DC,
                   uint8_t *remember_index, uint8_t *cour, uint8_t *suiv,
                   struct TREE * tree_DC, struct TREE * tree_AC, FILE **fptr) {
  uint8_t result_current_index = 1;
  uint8_t return_value = 0;
  int32_t coeff_mcu = 0;
  uint8_t poid_faible_huff_symbole = 0;
  uint8_t poid_fort_huff_symbole = 0;
  return_value = decode_byte_huffman_h(tree_DC, cour, suiv, remember_index, fptr, tree_DC->huff_symbols->symbols, tree_DC->huff_symbols->lenght, tree_DC->huff_codes->codes_list);
  poid_faible_huff_symbole = return_value & 0x0f;
  coeff_mcu = decode_coeff_mcu(poid_faible_huff_symbole, cour, suiv, remember_index, fptr);
  result->bloc_data[0] = coeff_mcu + *difference_DC;
  *difference_DC = result->bloc_data[0];
  while (true) {
    if (result_current_index >= 64) {
      break;
    }
    return_value =
        decode_byte_huffman_h(tree_AC, cour, suiv, remember_index, fptr,  tree_AC->huff_symbols->symbols, tree_AC->huff_symbols->lenght, tree_AC->huff_codes->codes_list);
    if (return_value == 0x00) {
      break;
    } else if (return_value == 0xf0) {
      result_current_index += 16;
    } else if (return_value && 0x0f != 0) {
      poid_fort_huff_symbole = (return_value & 0xf0) >> 4;
      poid_faible_huff_symbole = return_value & 0x0f;
      result_current_index += poid_fort_huff_symbole;
      coeff_mcu = decode_coeff_mcu(poid_faible_huff_symbole, cour, suiv,
                                  remember_index, fptr);
      result->bloc_data[result_current_index] = coeff_mcu;
      result_current_index++;
    } else {
      exit(EXIT_FAILURE);
    }
  }
}



/*decode les données brutes*/
struct group_MCU *decodeur_image(struct HEADER *header, uint8_t *cour,
                                 uint8_t *suiv, FILE **fptr) {
  uint8_t *remember_index = malloc(sizeof(uint8_t));
  malloc_verify("Problem while allocating for remember_index", remember_index);
  *remember_index = 0;
  int32_t *difference_DC_Y = malloc(sizeof(int32_t));
  malloc_verify("Problem while allocating for differnece_DC_Y", difference_DC_Y);
  *difference_DC_Y = 0;
  uint8_t nb_COULEURs = header->ptr_SOF0->SOF0_nb_components;
  uint32_t nb_MCU_width;
  uint32_t nb_MCU_height;
  uint16_t nb_bloc_frequ_height = get_nb_bloc_frequ_height(header);
  uint16_t nb_bloc_frequ_width = get_nb_bloc_frequ_width(header);
  int8_t vert_sampY = get_sampling_vert(header, 0);
  int8_t horz_sampY = get_sampling_horz(header, 0);
  uint8_t nb_Y_succ = vert_sampY * horz_sampY;
  if (vert_sampY == 2 && nb_bloc_frequ_height % 2 == 1) {
    nb_bloc_frequ_height++;
  }
  if (horz_sampY == 2 && nb_bloc_frequ_width % 2 == 1) {
    nb_bloc_frequ_width++;
  }
  nb_MCU_width = nb_bloc_frequ_width / horz_sampY;
  nb_MCU_height = nb_bloc_frequ_height / vert_sampY;
  uint8_t iH = 0;
  if (nb_COULEURs == 1) {
    iH = get_index_HUFF_DC(header, 0);
    struct TREE * tree_DC = get_tree(header, iH, 0);
    iH = get_index_HUFF_AC(header, 0);
    struct TREE * tree_AC = get_tree(header, iH, 1);
    struct group_MCU *group_MCU = malloc(
        sizeof(struct group_MCU) + nb_MCU_height * sizeof(struct MCU **));
    malloc_verify("Problem while allocating for group_MCU", group_MCU);
    group_MCU->nb_components = 1;
    group_MCU->nb_MCU_height = nb_MCU_height;
    group_MCU->nb_MCU_width = nb_MCU_width;
    struct bloc_frequ *buffer_bloc_frequ = malloc(sizeof(struct bloc_frequ));
    malloc_verify("Problem while allocating for buffer_bloc_frequ", buffer_bloc_frequ);
    int32_t *buffer_zigzag = calloc(64, sizeof(int32_t));
    malloc_verify("Problem while allocating for buffer_zigzag", buffer_zigzag);
    int32_t *buffer_quantization = calloc(64, sizeof(int32_t));
    malloc_verify("Problem while allocating for buffer_quantization", buffer_quantization);
    for (uint32_t i = 0; i < nb_MCU_height; i++) {
      group_MCU->group_MCU[i] = calloc(nb_MCU_width, sizeof(struct MCU*));
      malloc_verify("Problem while allocating for group_MCU->group_MCU_i", group_MCU->group_MCU[i]);
      for (uint32_t j = 0; j < nb_MCU_width; j++) {
        group_MCU->group_MCU[i][j] = malloc(sizeof(struct MCU) + nb_Y_succ * sizeof(struct bloc_spatial*));
        malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j", group_MCU->group_MCU[i][j]);
        group_MCU->group_MCU[i][j]->nb_Y_succ = nb_Y_succ;
        for (uint8_t v = 0; v < vert_sampY; v++) {
          for (uint8_t h = 0; h < horz_sampY; h++) {
            group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h] = malloc(sizeof(struct bloc_spatial));
            malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j->MCU_Y_blocs_v_h", group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h]);
            vide_buffer_int32(buffer_bloc_frequ->bloc_data);
            bloc_decoding(buffer_bloc_frequ, difference_DC_Y, remember_index, cour,
                          suiv, tree_DC, tree_AC, fptr);
            dequantisation(
                buffer_quantization,
                header->ptr_DQT[get_iQ_dqt(header, 0)]->DQT_coefficients_zigzag,
                buffer_bloc_frequ);
            zig_zag(buffer_zigzag, 8, buffer_quantization);
            fast_iDCT(group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h], buffer_zigzag, 8);
          }
        }
      }
    }
    free(difference_DC_Y);
    free(remember_index);
    free(buffer_bloc_frequ);
    free(buffer_quantization);
    free(buffer_zigzag);
    return group_MCU;
  } else {
      int32_t *difference_DC_Cb = malloc(sizeof(int32_t));
      int32_t *difference_DC_Cr = malloc(sizeof(int32_t));
      malloc_verify("Problem while allocating for differnece_DC_Cb", difference_DC_Cb);
      malloc_verify("Problem while allocating for differnece_DC_Cr", difference_DC_Cr);
      *difference_DC_Cb = 0;
      *difference_DC_Cr = 0;
      iH = get_index_HUFF_DC(header, 0);
      struct TREE * tree_DC_Y = get_tree(header, iH, 0);
      iH = get_index_HUFF_AC(header, 0);
      struct TREE * tree_AC_Y = get_tree(header, iH, 1);
      iH = get_index_HUFF_DC(header, 1);
      struct TREE * tree_DC_Cb = get_tree(header, iH, 0);
      iH = get_index_HUFF_AC(header, 1);
      struct TREE * tree_AC_Cb = get_tree(header, iH, 1);
      iH = get_index_HUFF_DC(header, 2);
      struct TREE * tree_DC_Cr = get_tree(header, iH, 0);
      iH = get_index_HUFF_AC(header, 2);
      struct TREE * tree_AC_Cr = get_tree(header, iH, 1);
      struct group_MCU *group_MCU = malloc(
          sizeof(struct group_MCU) + nb_MCU_height * sizeof(struct MCU **));
      malloc_verify("Problem while allocating for group_MCU", group_MCU);
      group_MCU->nb_components = 3;
      group_MCU->nb_MCU_height = nb_MCU_height;
      group_MCU->nb_MCU_width = nb_MCU_width;
      struct bloc_frequ *buffer_bloc_frequ = malloc(sizeof(struct bloc_frequ));
      malloc_verify("Problem while allocating for buffer_bloc_frequ", buffer_bloc_frequ);
      int32_t *buffer_zigzag = calloc(64, sizeof(int32_t));
      malloc_verify("Problem while allocating for buffer_zigzag", buffer_zigzag);
      int32_t *buffer_quantization = calloc(64, sizeof(int32_t));
      malloc_verify("Problem while allocating for buffer_quantization", buffer_quantization);
    for (uint32_t i = 0; i < nb_MCU_height; i++) {
      group_MCU->group_MCU[i] = calloc(nb_MCU_width, sizeof(struct MCU*));
      malloc_verify("Problem while allocating for group_MCU->group_MCU_i", group_MCU->group_MCU[i]);
      for (uint32_t j = 0; j < nb_MCU_width; j++) {
        group_MCU->group_MCU[i][j] = malloc(sizeof(struct MCU) + nb_Y_succ * sizeof(struct bloc_spatial*));
        malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j", group_MCU->group_MCU[i][j]);
        group_MCU->group_MCU[i][j]->nb_Y_succ = nb_Y_succ;
        for (uint8_t v = 0; v < vert_sampY; v++) {
          for (uint8_t h = 0; h < horz_sampY; h++) {
            group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h] = malloc(sizeof(struct bloc_spatial));
            malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j->MCU_Y_blocs_v_h", group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h]);
            vide_buffer_int32(buffer_bloc_frequ->bloc_data);
            bloc_decoding(buffer_bloc_frequ, difference_DC_Y, remember_index, cour,
                          suiv, tree_DC_Y, tree_AC_Y, fptr);
            dequantisation(
                buffer_quantization,
                header->ptr_DQT[get_iQ_dqt(header, 0)]->DQT_coefficients_zigzag,
                buffer_bloc_frequ);
            zig_zag(buffer_zigzag, 8, buffer_quantization);
            fast_iDCT(group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h], buffer_zigzag, 8);
          }
        }

        vide_buffer_int32(buffer_bloc_frequ->bloc_data);
        bloc_decoding(buffer_bloc_frequ, difference_DC_Cb, remember_index, cour,
                      suiv, tree_DC_Cb, tree_AC_Cb, fptr);
        dequantisation(
            buffer_quantization,
            header->ptr_DQT[get_iQ_dqt(header, 1)]->DQT_coefficients_zigzag,
            buffer_bloc_frequ);
        zig_zag(buffer_zigzag, 8, buffer_quantization);
        fast_iDCT(&group_MCU->group_MCU[i][j]->MCU_Cb_bloc, buffer_zigzag, 8);


        vide_buffer_int32(buffer_bloc_frequ->bloc_data);
        bloc_decoding(buffer_bloc_frequ, difference_DC_Cr, remember_index, cour,
                      suiv, tree_DC_Cr, tree_AC_Cr, fptr);
        dequantisation(
            buffer_quantization,
            header->ptr_DQT[get_iQ_dqt(header, 1)]->DQT_coefficients_zigzag,
            buffer_bloc_frequ);
        zig_zag(buffer_zigzag, 8, buffer_quantization);
        fast_iDCT(&group_MCU->group_MCU[i][j]->MCU_Cr_bloc, buffer_zigzag, 8);
        
      }
    }
    free(difference_DC_Y);
    free(difference_DC_Cb);
    free(difference_DC_Cr);
    free(remember_index);
    free(buffer_bloc_frequ);
    free(buffer_quantization);
    free(buffer_zigzag);
    return group_MCU;
  }
}



/*decode les données brutes et calcule en même temps 
les mots de code de huffman des symbols de huffman utilisés
(option -h)*/
struct group_MCU *decodeur_image_h(struct HEADER *header, uint8_t *cour,
                                 uint8_t *suiv, FILE **fptr) {
  uint8_t *remember_index = malloc(sizeof(uint8_t));
  malloc_verify("Problem while allocating for remember_index", remember_index);
  *remember_index = 0;
  int32_t *difference_DC_Y = malloc(sizeof(int32_t));
  malloc_verify("Problem while allocating for differnece_DC_Y", difference_DC_Y);
  *difference_DC_Y = 0;
  uint8_t nb_COULEURs = header->ptr_SOF0->SOF0_nb_components;
  uint32_t nb_MCU_width;
  uint32_t nb_MCU_height;
  uint16_t nb_bloc_frequ_height = get_nb_bloc_frequ_height(header);
  uint16_t nb_bloc_frequ_width = get_nb_bloc_frequ_width(header);
  int8_t vert_sampY = get_sampling_vert(header, 0);
  int8_t horz_sampY = get_sampling_horz(header, 0);
  uint8_t nb_Y_succ = vert_sampY * horz_sampY;
  if (vert_sampY == 2 && nb_bloc_frequ_height % 2 == 1) {
    nb_bloc_frequ_height++;
  }
  if (horz_sampY == 2 && nb_bloc_frequ_width % 2 == 1) {
    nb_bloc_frequ_width++;
  }
  nb_MCU_width = nb_bloc_frequ_width / horz_sampY;
  nb_MCU_height = nb_bloc_frequ_height / vert_sampY;
  uint8_t iH = 0;
  if (nb_COULEURs == 1) {
    iH = get_index_HUFF_DC(header, 0);
    struct TREE * tree_DC = get_tree(header, iH, 0);
    iH = get_index_HUFF_AC(header, 0);
    struct TREE * tree_AC = get_tree(header, iH, 1);
    struct group_MCU *group_MCU = malloc(
        sizeof(struct group_MCU) + nb_MCU_height * sizeof(struct MCU **));
    malloc_verify("Problem while allocating for group_MCU", group_MCU);
    group_MCU->nb_components = 1;
    group_MCU->nb_MCU_height = nb_MCU_height;
    group_MCU->nb_MCU_width = nb_MCU_width;
    struct bloc_frequ *buffer_bloc_frequ = malloc(sizeof(struct bloc_frequ));
    malloc_verify("Problem while allocating for buffer_bloc_frequ", buffer_bloc_frequ);
    int32_t *buffer_zigzag = calloc(64, sizeof(int32_t));
    malloc_verify("Problem while allocating for buffer_zigzag", buffer_zigzag);
    int32_t *buffer_quantization = calloc(64, sizeof(int32_t));
    malloc_verify("Problem while allocating for buffer_quantization", buffer_quantization);
    for (uint32_t i = 0; i < nb_MCU_height; i++) {
      group_MCU->group_MCU[i] = calloc(nb_MCU_width, sizeof(struct MCU*));
      malloc_verify("Problem while allocating for group_MCU->group_MCU_i", group_MCU->group_MCU[i]);
      for (uint32_t j = 0; j < nb_MCU_width; j++) {
        group_MCU->group_MCU[i][j] = malloc(sizeof(struct MCU) + nb_Y_succ * sizeof(struct bloc_spatial*));
        malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j", group_MCU->group_MCU[i][j]);
        group_MCU->group_MCU[i][j]->nb_Y_succ = nb_Y_succ;
        for (uint8_t v = 0; v < vert_sampY; v++) {
          for (uint8_t h = 0; h < horz_sampY; h++) {
            group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h] = malloc(sizeof(struct bloc_spatial));
            malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j->MCU_Y_blocs_v_h", group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h]);
            vide_buffer_int32(buffer_bloc_frequ->bloc_data);
            bloc_decoding_h(buffer_bloc_frequ, difference_DC_Y, remember_index, cour,
                          suiv, tree_DC, tree_AC, fptr);
            dequantisation(
                buffer_quantization,
                header->ptr_DQT[get_iQ_dqt(header, 0)]->DQT_coefficients_zigzag,
                buffer_bloc_frequ);
            zig_zag(buffer_zigzag, 8, buffer_quantization);
            fast_iDCT(group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h], buffer_zigzag, 8);
          }
        }
      }
    }
    free(difference_DC_Y);
    free(remember_index);
    free(buffer_bloc_frequ);
    free(buffer_quantization);
    free(buffer_zigzag);
    return group_MCU;
  } else {
      int32_t *difference_DC_Cb = malloc(sizeof(int32_t));
      int32_t *difference_DC_Cr = malloc(sizeof(int32_t));
      malloc_verify("Problem while allocating for differnece_DC_Cb", difference_DC_Cb);
      malloc_verify("Problem while allocating for differnece_DC_Cr", difference_DC_Cr);
      *difference_DC_Cb = 0;
      *difference_DC_Cr = 0;
      iH = get_index_HUFF_DC(header, 0);
      struct TREE * tree_DC_Y = get_tree(header, iH, 0);
      iH = get_index_HUFF_AC(header, 0);
      struct TREE * tree_AC_Y = get_tree(header, iH, 1);
      iH = get_index_HUFF_DC(header, 1);
      struct TREE * tree_DC_Cb = get_tree(header, iH, 0);
      iH = get_index_HUFF_AC(header, 1);
      struct TREE * tree_AC_Cb = get_tree(header, iH, 1);
      iH = get_index_HUFF_DC(header, 2);
      struct TREE * tree_DC_Cr = get_tree(header, iH, 0);
      iH = get_index_HUFF_AC(header, 2);
      struct TREE * tree_AC_Cr = get_tree(header, iH, 1);
      struct group_MCU *group_MCU = malloc(
          sizeof(struct group_MCU) + nb_MCU_height * sizeof(struct MCU **));
      malloc_verify("Problem while allocating for group_MCU", group_MCU);
      group_MCU->nb_components = 3;
      group_MCU->nb_MCU_height = nb_MCU_height;
      group_MCU->nb_MCU_width = nb_MCU_width;
      struct bloc_frequ *buffer_bloc_frequ = malloc(sizeof(struct bloc_frequ));
      malloc_verify("Problem while allocating for buffer_bloc_frequ", buffer_bloc_frequ);
      int32_t *buffer_zigzag = calloc(64, sizeof(int32_t));
      malloc_verify("Problem while allocating for buffer_zigzag", buffer_zigzag);
      int32_t *buffer_quantization = calloc(64, sizeof(int32_t));
      malloc_verify("Problem while allocating for buffer_quantization", buffer_quantization);
    for (uint32_t i = 0; i < nb_MCU_height; i++) {
      group_MCU->group_MCU[i] = calloc(nb_MCU_width, sizeof(struct MCU*));
      malloc_verify("Problem while allocating for group_MCU->group_MCU_i", group_MCU->group_MCU[i]);
      for (uint32_t j = 0; j < nb_MCU_width; j++) {
        group_MCU->group_MCU[i][j] = malloc(sizeof(struct MCU) + nb_Y_succ * sizeof(struct bloc_spatial*));
        malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j", group_MCU->group_MCU[i][j]);
        group_MCU->group_MCU[i][j]->nb_Y_succ = nb_Y_succ;
        for (uint8_t v = 0; v < vert_sampY; v++) {
          for (uint8_t h = 0; h < horz_sampY; h++) {
            group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h] = malloc(sizeof(struct bloc_spatial));
            malloc_verify("Problem while allocating for group_MCU->group_MCU_i_j->MCU_Y_blocs_v_h", group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h]);
            vide_buffer_int32(buffer_bloc_frequ->bloc_data);
            bloc_decoding_h(buffer_bloc_frequ, difference_DC_Y, remember_index, cour,
                          suiv, tree_DC_Y, tree_AC_Y, fptr);
            dequantisation(
                buffer_quantization,
                header->ptr_DQT[get_iQ_dqt(header, 0)]->DQT_coefficients_zigzag,
                buffer_bloc_frequ);
            zig_zag(buffer_zigzag, 8, buffer_quantization);
            fast_iDCT(group_MCU->group_MCU[i][j]->MCU_Y_blocs[v *  horz_sampY + h], buffer_zigzag, 8);
          }
        }


        vide_buffer_int32(buffer_bloc_frequ->bloc_data);
        bloc_decoding_h(buffer_bloc_frequ, difference_DC_Cb, remember_index, cour,
                      suiv, tree_DC_Cb, tree_AC_Cb, fptr);
        dequantisation(
            buffer_quantization,
            header->ptr_DQT[get_iQ_dqt(header, 1)]->DQT_coefficients_zigzag,
            buffer_bloc_frequ);
        zig_zag(buffer_zigzag, 8, buffer_quantization);
        fast_iDCT(&group_MCU->group_MCU[i][j]->MCU_Cb_bloc, buffer_zigzag, 8);


        vide_buffer_int32(buffer_bloc_frequ->bloc_data);
        bloc_decoding_h(buffer_bloc_frequ, difference_DC_Cr, remember_index, cour,
                      suiv, tree_DC_Cr, tree_AC_Cr, fptr);
        dequantisation(
            buffer_quantization,
            header->ptr_DQT[get_iQ_dqt(header, 1)]->DQT_coefficients_zigzag,
            buffer_bloc_frequ);
        zig_zag(buffer_zigzag, 8, buffer_quantization);
        fast_iDCT(&group_MCU->group_MCU[i][j]->MCU_Cr_bloc, buffer_zigzag, 8);
        
      }
    }
    free(difference_DC_Y);
    free(difference_DC_Cb);
    free(difference_DC_Cr);
    free(remember_index);
    free(buffer_bloc_frequ);
    free(buffer_quantization);
    free(buffer_zigzag);
    return group_MCU;
  }
}



/*convertion d'une MCU en une bloc RGB*/
void convert_MCU_to_RGB(struct RGB * RGB, struct MCU * MCU, struct HEADER * header) {
  float red;
  float green;
  float blue;
  uint8_t bloc;
  uint8_t cbr_coordinates;
  uint8_t vert_sampY = get_sampling_vert(header, 0);
  uint8_t horz_sampY = get_sampling_horz(header, 0);
  for (uint8_t v = 0; v < vert_sampY; v ++) {
    for (uint8_t h = 0; h < horz_sampY; h ++) {
      bloc = v * horz_sampY + h;
      for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j ++) {
          cbr_coordinates = (((i/vert_sampY) + (v << 2)) << 3) + ((j/horz_sampY) + (h << 2));
          red = ((MCU->MCU_Y_blocs[bloc]->bloc_data[(i << 3) + j]) + 1.402 * ((MCU->MCU_Cr_bloc.bloc_data[cbr_coordinates])-128));
          green = ((MCU->MCU_Y_blocs[bloc]->bloc_data[(i << 3) + j]) - 0.34414 * ((MCU->MCU_Cb_bloc.bloc_data[cbr_coordinates])-128) - 0.71414 * ((MCU->MCU_Cr_bloc.bloc_data[cbr_coordinates])-128));
          blue = ((MCU->MCU_Y_blocs[bloc]->bloc_data[(i << 3) + j]) + 1.772 * ((MCU->MCU_Cb_bloc.bloc_data[cbr_coordinates])-128));
          if (red > 255) red = 255;
          if (green > 255) green = 255;
          if (blue > 255) blue = 255;
          if (red < 0) red = 0;
          if (green < 0) green = 0;
          if (blue < 0) blue = 0;
          red = (uint8_t) round(red);
          green = (uint8_t) round(green);
          blue = (uint8_t) round(blue);
          RGB ->RGB_RGB_blocs[v * horz_sampY + h]->bloc_data[((i << 3) + j)*3] = red;
          RGB ->RGB_RGB_blocs[v * horz_sampY + h]->bloc_data[((i << 3) + j)*3 + 1] = green;
          RGB ->RGB_RGB_blocs[v * horz_sampY + h]->bloc_data[((i << 3) + j)*3 + 2] = blue;
        }
      }
    }      
  }
}



/*cnvertion des MCUs en RGBs*/
struct group_RGB *convert_group_MCU_to_group_RGB(struct group_MCU * group_MCU, struct HEADER * header) {
  uint32_t nb_MCU_height = group_MCU ->nb_MCU_height;
  uint32_t nb_MCU_width = group_MCU ->nb_MCU_width;
  uint8_t nb_Y_succ = group_MCU -> group_MCU[0][0]->nb_Y_succ;
  struct group_RGB *group_RGB = malloc(sizeof(struct group_RGB) + nb_MCU_height * sizeof(struct RGB **));
  malloc_verify("Problem while allocating for group_RGB", group_RGB);
  group_RGB->nb_RGB_succ = nb_Y_succ;
  group_RGB->nb_RGB_height = nb_MCU_height;
  group_RGB->nb_RGB_width = nb_MCU_width;
  uint8_t vert_sampY = get_sampling_vert(header, 0);
  uint8_t horz_sampY = get_sampling_horz(header, 0);
  for (uint32_t i = 0; i < nb_MCU_height; i++) {
    group_RGB->group_RGB[i] = calloc(nb_MCU_width, sizeof(struct RGB*));
    malloc_verify("Problem while allocating for group_RGB->group_RGB", group_RGB->group_RGB[i]);
    for (uint32_t j = 0; j < nb_MCU_width; j++) {
      group_RGB->group_RGB[i][j] = malloc(sizeof(struct RGB) + nb_Y_succ * sizeof(struct bloc_spatial_x3*));
      malloc_verify("Problem while allocating for group_RGB->group_RGB_i_j", group_RGB->group_RGB[i][j]);
      for (uint8_t v = 0; v < vert_sampY; v++) {
        for (uint8_t h = 0; h < horz_sampY; h++) {
          group_RGB->group_RGB[i][j]->RGB_RGB_blocs[v * horz_sampY + h] = malloc(sizeof(struct bloc_spatial_x3));
          malloc_verify("Problem while allocating for group_RGB->group_RGB[i][j]->group_bloc_RGB->RGB_RGB_blocs_k", group_RGB->group_RGB[i][j]->RGB_RGB_blocs[v * horz_sampY + h]);
        }
      }
      convert_MCU_to_RGB(group_RGB -> group_RGB[i][j],group_MCU -> group_MCU[i][j], header);
    }
  }
  return group_RGB;
}
