# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <string.h>
# include "huffman.h"
# include "commun.h"
# include "scan_head.h"
# include "memory.h"


/*par exemple en utilisant fread au debut d'image on lira 0xff puis 0xd8,
cela stockera 0xd8 ff au lieu de 0xff d8, cette fontion règle le problème*/
void inverse_octets(uint16_t *octets_cible) {
  *octets_cible = (((*octets_cible & 0xff) << 8) + (*octets_cible >> 8));
}



/*affiche sur la stdout les données de l'entête (option -h)*/
void test_header(struct HEADER *header) {
  printf("\n\n");
  printf("APPO_lenght : %u\n", header->ptr_APP0->APP0_lenght);
  printf("APPO_image_type : %s\n", header->ptr_APP0->APP0_image_type);
  printf("APPO_version : %s\n", header->ptr_APP0->APP0_version);
  printf("----------------------\n");




  if (header->ptr_COM) {
    printf("COM_lenght : %u\n", header->ptr_COM->COM_lenght);
    printf("COM_commentary : %s\n", header->ptr_COM->COM_commentary);
    printf("\n");
    printf("----------------------");
    printf("\n");
    printf("----------------------\n");
  }




  for (uint8_t i = 0; i < 4; i++) {
    if (header->ptr_DQT[i]) {
      printf("DQT_%u lenght : %u\n", i, header->ptr_DQT[i]->DQT_lenght);
      printf("DQT_%u precision : %u\n", i, header->ptr_DQT[i]->DQT_precision);
      printf("DQT_%u index : %u\n", i, header->ptr_DQT[i]->DQT_index);
      printf("DQT_%u coefficients_zigzag : \n", i);
      afficher_vecteur_uint8(header->ptr_DQT[i]->DQT_coefficients_zigzag);
      printf("----------------------\n");
    } else {
      printf("----------------------\n");
      printf("DQT_%u est vide\n", i);
    }
  }
  printf("----------------------\n");
  printf("\n");
  printf(">>> There are %u DQT_TABLES\n", header->dqt_number);
  printf("\n");
  printf("----------------------");
  printf("\n");




  printf("SOF0_lenght : %u\n", header->ptr_SOF0->SOF0_lenght);
  printf("SOF0_precision : %u\n", header->ptr_SOF0->SOF0_precision);
  printf("SOF0_height : %u\n", header->ptr_SOF0->SOF0_height);
  printf("SOF0_width : %u\n", header->ptr_SOF0->SOF0_width);
  printf("SOF0_nb_components : %u\n", header->ptr_SOF0->SOF0_nb_components);
  char * components[3] = {"Y", "Cb", "Cr"};
  for (uint8_t i = 0; i < header->ptr_SOF0->SOF0_nb_components; i++) {
    printf("----------------------\n");
    printf("SOF0_%s index_C : %u\n", components[i],
           header->ptr_SOF0->SOF0_data_components[i]->index_C);
    printf(
        "SOF0_%s horizontal_sampling_factor : %u\n", components[i],
        header->ptr_SOF0->SOF0_data_components[i]->horizontal_sampling_factor);
    printf("SOF0_%s vertical_sampling_factor : %u\n", components[i],
           header->ptr_SOF0->SOF0_data_components[i]->vertical_sampling_factor);
    printf("SOF0_%s index_Q : %u\n", components[i],
           header->ptr_SOF0->SOF0_data_components[i]->index_Q);
  }
  printf("\n");
  printf("----------------------");
  printf("\n");




  for (uint8_t i = 0; i < 4; i++) {
    if (header->ptr_HUFFMAN_TABLE[i]) {
      printf("HUFFMAN_%u lenght : %u\n", i,
             header->ptr_HUFFMAN_TABLE[i]->HUFFMAN_TABLE_lenght);
      char type[3];
      type[2] = 0;
      type[1] = 67;
      if (header->ptr_HUFFMAN_TABLE[i]->HUFFMAN_TABLE_type == 0) {
        type[0] = 68;
      } else {
        type[0] = 65;
      }
      printf("HUFFMAN_%u type : %s\n", i,
             type);
      printf("HUFFMAN_%u index : %u\n", i,
             header->ptr_HUFFMAN_TABLE[i]->HUFFMAN_TABLE_index);
      printf("Used symbols :\n");
      print_tas_huffman(header->ptr_HUFFMAN_TABLE[i]->HUFFMAN_TABLE_TREE);
      printf("----------------------\n");
    } else {
      printf("----------------------\n");
      printf("HUFFMAN_TABLE_%u est vide\n", i);
    }
  }
  printf("----------------------\n");
  printf("\n");
  printf(">>> There are %u HUFFMAN_TABLES\n", header->dht_number);
  printf("\n");
  printf("----------------------");
  printf("\n");




  printf("SOS_lenght : %u\n", header->ptr_SOS->SOS_lenght);
  printf("SOS_ss : %u\n", header->ptr_SOS->SOS_ss);
  printf("SOS_se : %u\n", header->ptr_SOS->SOS_se);
  printf("SOS_ah : %u\n", header->ptr_SOS->SOS_ah);
  printf("SOS_al : %u\n", header->ptr_SOS->SOS_al);
  printf("SOS_nb_components : %u\n", header->ptr_SOS->SOS_nb_components);
  for (uint8_t i = 0; i < header->ptr_SOS->SOS_nb_components; i++) {
    printf("----------------------\n");
    printf("SOS_%s index_C : %u\n", components[i],
           header->ptr_SOS->SOS_data_components[i]->index_C);
    printf("SOS_%s DC : %u\n", components[i],
           header->ptr_SOS->SOS_data_components[i]->index_H_DC);
    printf("SOS_%s AC : %u\n", components[i],
           header->ptr_SOS->SOS_data_components[i]->index_H_AC);
  }
  printf("----------------------\n");
}



/*lit la partie APP0 de l'entête de l'image*/
struct APP0 *read_APP0(FILE **fptr) {
  struct APP0 *APP0 = malloc(sizeof(struct APP0));
  malloc_verify("Problem while allocating for APP0", APP0);
  fread(&(APP0->APP0_lenght), sizeof(uint16_t), 1, *fptr);
  inverse_octets(&(APP0->APP0_lenght));
  fread(APP0->APP0_image_type, sizeof(char), 5, *fptr);
  if (strcmp(APP0->APP0_image_type, "JFIF")) {
    fprintf(stderr, "Only JFIF files supported\n");
    exit(EXIT_FAILURE);
  }
  fread(&((APP0->APP0_version)[0]), sizeof(char), 1, *fptr);
  (APP0->APP0_version)[0] += 48;
  (APP0->APP0_version)[1] = 46;
  fgets(&((APP0->APP0_version)[2]), 2, *fptr);
  (APP0->APP0_version)[2] += 48;
  if (strcmp(APP0->APP0_version, "1.1")) {
    fprintf(stderr, "Only JFIF version 1.1 supported\n");
    exit(EXIT_FAILURE);
  }
  for (uint8_t i = 0; i < 7; i++) {
    fgetc(*fptr);
  }
  return APP0;
}



/*lit la partie COM de l'entête de l'image*/
struct COM *read_COM(FILE **fptr) {
  uint16_t lenght_COM;
  fread(&lenght_COM, sizeof(uint16_t), 1, *fptr);
  inverse_octets(&lenght_COM);
  struct COM *COM = malloc(sizeof(struct COM) + (lenght_COM - 1) * sizeof(char));
  malloc_verify("Problem while allocating for COM", COM);
  COM->COM_lenght = lenght_COM;
  fgets(COM->COM_commentary, COM->COM_lenght - 1, *fptr);
  return COM;
}



/*lit la partie DQT de l'entête de l'image*/
void read_DQT(FILE **fptr, struct HEADER * header){
  uint16_t lenght_DQT;
  fread(&lenght_DQT, sizeof(uint16_t), 1, *fptr);
  inverse_octets(&lenght_DQT);
  uint8_t nb_DQT_tables_in_this_section = ((lenght_DQT - 2)/65);
  for (uint8_t i = 0; i < nb_DQT_tables_in_this_section; i++) {
    if (header->dqt_number > 3) {
      fprintf(stderr, "ERROR more than 4 DQT tables were found\n");
      exit(EXIT_FAILURE);
    }
    struct DQT *DQT = malloc(sizeof(struct DQT));
    malloc_verify("Problem while allocating for DQT", DQT);
    DQT -> DQT_lenght = lenght_DQT;
    int8_t precision_indice_DQT;
    fread(&precision_indice_DQT, sizeof(int8_t), 1, *fptr);
    if ((precision_indice_DQT >> 4) == 1) {
      DQT->DQT_precision = 16;
    } else if ((precision_indice_DQT >> 4) == 0) {
      DQT->DQT_precision = 8;
    } else {
      fprintf(stderr, "Invalid precision\n");
      exit(EXIT_FAILURE);
    }
    DQT->DQT_index = precision_indice_DQT & 0x0f;
    fread(&(DQT->DQT_coefficients_zigzag), sizeof(uint8_t), 64, *fptr);
    header -> ptr_DQT[DQT->DQT_index] = DQT;
    header->did_I_malloc_this_dqt_table[DQT->DQT_index] = true;
    header -> dqt_number++;
  }
}



/*lit la partie SOF0 de l'entête de l'image*/
struct SOF0 *read_SOF0(FILE **fptr) {
  uint16_t lenght_SOF0;
  uint8_t precision_SOF0;
  uint16_t height_SOF0;
  uint16_t width_SOF0;
  uint8_t nb_components_SOF0;
  fread(&lenght_SOF0, sizeof(uint16_t), 1, *fptr);
  fread(&precision_SOF0, sizeof(uint8_t), 1, *fptr);
  fread(&height_SOF0, sizeof(uint16_t), 1, *fptr);
  fread(&width_SOF0, sizeof(uint16_t), 1, *fptr);
  fread(&nb_components_SOF0, sizeof(uint8_t), 1, *fptr);
  inverse_octets(&lenght_SOF0);
  inverse_octets(&height_SOF0);
  inverse_octets(&width_SOF0);
  if (nb_components_SOF0 != 1 && nb_components_SOF0 != 3) {
    fprintf(stderr, "Only 1 (gray) or 3 (YCbCr) components images are supported\n");
    exit(EXIT_FAILURE);
  }
  struct SOF0 *SOF0 =
      malloc(sizeof(struct SOF0) +
             nb_components_SOF0 * sizeof(struct SOF0_data_component *));
  malloc_verify("Problem while allocating for SOF0", SOF0);
  SOF0->SOF0_lenght = lenght_SOF0;
  SOF0->SOF0_precision = precision_SOF0;
  SOF0->SOF0_height = height_SOF0;
  SOF0->SOF0_width = width_SOF0;
  SOF0->SOF0_nb_components = nb_components_SOF0;
  int8_t nb_8bits_factor;
  for (uint8_t i = 0; i < nb_components_SOF0; i++) {
    SOF0->SOF0_data_components[i] = malloc(sizeof(struct SOF0_data_component));
    malloc_verify("Problem while allocating for SOF0_data_components", SOF0->SOF0_data_components[i]);
    fread(&((SOF0->SOF0_data_components)[i]->index_C), sizeof(uint8_t), 1,
          *fptr);
    fread(&nb_8bits_factor, sizeof(int8_t), 1, *fptr);
    SOF0->SOF0_data_components[i]->vertical_sampling_factor =
        nb_8bits_factor & 0b00001111;
    SOF0->SOF0_data_components[i]->horizontal_sampling_factor =
        (nb_8bits_factor & 0b11110000) >> 4;
    fread(&((SOF0->SOF0_data_components)[i]->index_Q), sizeof(uint8_t), 1,
          *fptr);
  }
  return SOF0;
}



/*lit la partie DHT de l'entête de l'image*/
void read_DHT(FILE **fptr, struct HEADER * header, bool option_h) {
  uint16_t lenght_HUFF;
  fread(&lenght_HUFF, sizeof(uint16_t), 1, *fptr);
  inverse_octets(&lenght_HUFF);
  uint32_t end_of_section = lenght_HUFF - 2;
  while (end_of_section) {
    if (header->dht_number > 3) {
      fprintf(stderr, "ERROR more than 4 (AC/DC) DHT tables were found\n");
      exit(EXIT_FAILURE);
    }
    int8_t precision_indice_DHT;
    fread(&precision_indice_DHT, sizeof(int8_t), 1, *fptr);
    int8_t if_not0_error = precision_indice_DHT & 0b11100000;
    if (if_not0_error != 0) {
      fprintf(stderr, "ERROR\n");
      exit(EXIT_SUCCESS);
    }
    uint8_t type_huff = (precision_indice_DHT & 0b00010000) >> 4;
    uint8_t index_huff = precision_indice_DHT & 0b00001111;
    uint8_t data_symbols[16];
    fread(data_symbols, sizeof(uint8_t), 16, *fptr);
    uint32_t nb_huffman_symbols = nb_of_huffman_symbols(data_symbols);
    struct huff_symbols * huff_symbols = malloc(sizeof(struct huff_symbols) + nb_huffman_symbols*sizeof(uint8_t));
    malloc_verify("Problem while allocating for huff_symbols", huff_symbols);
    huff_symbols -> lenght = nb_huffman_symbols;
    fread(huff_symbols->symbols, sizeof(uint8_t), nb_huffman_symbols, *fptr);
    uint8_t profondeur = profondeur_max(data_symbols);
    uint32_t lenght_tree_tab = taille_tab(profondeur);
    struct TREE * TREE = malloc(sizeof(struct TREE) + lenght_tree_tab * sizeof(uint16_t));
    malloc_verify("Problem while allocating for TREE", TREE);
    TREE->index = (type_huff << 1) + index_huff;
    TREE ->lenght = lenght_tree_tab;
    TREE ->profondeur = profondeur;
    for (uint32_t i = 0; i < lenght_tree_tab ; i++) {
      TREE->tree[i] = 0xffff;
    }
    uint32_t nb_noeuds_insaturés_avant = 0;
    uint32_t current_index;
    uint8_t lenght_courant = 0;
    uint32_t index_current_symbol = 0;
    for (uint8_t profondeur_courante = 0; profondeur_courante < profondeur;
        profondeur_courante++) {
      current_index = (1 << (profondeur_courante + 1)) - 1;
      current_index += nb_noeuds_insaturés_avant;
      lenght_courant = data_symbols[profondeur_courante];
      for (uint8_t j = 0; j < lenght_courant; j++) {
        TREE->tree[current_index + j] = huff_symbols->symbols[index_current_symbol];
        index_current_symbol++;
      }
      nb_noeuds_insaturés_avant= 2 * (nb_noeuds_insaturés_avant + lenght_courant);
    }

    struct HUFFMAN_TABLE *HUFFMAN_TABLE = malloc(sizeof(struct HUFFMAN_TABLE));
    malloc_verify("Problem while allocating for HUFFMAN_TABLE", HUFFMAN_TABLE);
    HUFFMAN_TABLE->HUFFMAN_TABLE_lenght = nb_huffman_symbols;
    HUFFMAN_TABLE->HUFFMAN_TABLE_index = precision_indice_DHT & 0b00001111;
    if (HUFFMAN_TABLE->HUFFMAN_TABLE_index > 3) {
      fprintf(stderr, "ERROR\n");
      exit(EXIT_SUCCESS);
    }
    HUFFMAN_TABLE->HUFFMAN_TABLE_type = (precision_indice_DHT & 0b00010000) >> 4;
    HUFFMAN_TABLE->HUFFMAN_TABLE_TREE = TREE;
    TREE->option_h = option_h;
    if (option_h) {
      TREE->huff_symbols = huff_symbols;
      struct huff_codes * huff_codes = malloc(sizeof(struct huff_codes) + nb_huffman_symbols*sizeof(char *));
      malloc_verify("Problem while allocating for huff_codes", huff_codes);
      huff_codes -> lenght = nb_huffman_symbols;
      for (uint32_t i = 0; i < nb_huffman_symbols; i++) {
        huff_codes -> codes_list[i] = calloc(17, sizeof(char)); 
      }
      TREE->huff_codes = huff_codes;
    } else {
      free(huff_symbols);
    }
    header -> ptr_HUFFMAN_TABLE[(type_huff << 1) + index_huff] = HUFFMAN_TABLE;
    header->did_I_malloc_this_dht_table[(type_huff << 1) + index_huff] = true;
    header->dht_number++;
    end_of_section -= (17 + nb_huffman_symbols);
  }
}



/*lit la partie SOS de l'entête de l'image*/
struct SOS *read_SOS(FILE **fptr) {
  uint16_t lenght_SOS;
  uint8_t nb_components_SOS;
  fread(&lenght_SOS, sizeof(uint16_t), 1, *fptr);
  fread(&nb_components_SOS, sizeof(uint8_t), 1, *fptr);
  inverse_octets(&lenght_SOS);
  struct SOS *SOS =
      malloc(sizeof(struct SOS) +
             nb_components_SOS * sizeof(struct SOS_data_component *));
  malloc_verify("Problem while allocating for SOS", SOS);
  SOS->SOS_lenght = lenght_SOS;
  SOS->SOS_nb_components = nb_components_SOS;
  int8_t index_H_DC_AC;
  for (uint8_t i = 0; i < nb_components_SOS; i++) {
    SOS->SOS_data_components[i] = malloc(sizeof(struct SOS_data_component));
    malloc_verify("Problem while allocating for SOS_data_components", SOS->SOS_data_components[i]);
    fread(&(SOS->SOS_data_components[i]->index_C), sizeof(uint8_t), 1, *fptr);
    fread(&index_H_DC_AC, sizeof(int8_t), 1, *fptr);
    SOS->SOS_data_components[i]->index_H_AC = index_H_DC_AC & 0b00001111;
    SOS->SOS_data_components[i]->index_H_DC = (index_H_DC_AC & 0b11110000) >> 4;
  }
  fread(&(SOS->SOS_ss), sizeof(uint8_t), 1, *fptr);
  fread(&(SOS->SOS_se), sizeof(uint8_t), 1, *fptr);
  int8_t approx_ah_al;
  fread(&approx_ah_al, sizeof(int8_t), 1, *fptr);
  SOS->SOS_al = approx_ah_al & 0b00001111;
  SOS->SOS_ah = (approx_ah_al & 0b11110000) >> 4;
  if ((SOS->SOS_ss != 0) || (SOS->SOS_se != 63) || (SOS->SOS_ah != 0) || (SOS->SOS_al != 0)) {
    fprintf(stderr, "Only baseline mode supported\n");
    exit(EXIT_SUCCESS);
  }
  return SOS;
}



/*lit l'entête de l'image*/
struct HEADER *scan_header(FILE **fptr, uint16_t **current, bool option_h) {
  fread(*current, sizeof(uint16_t), 1, *fptr);
  inverse_octets(*current);
  struct HEADER *header = malloc(sizeof(struct HEADER));
  malloc_verify("Problem while allocating for header", header);
  header->dqt_number = 0;
  header->dht_number = 0;
  for (uint8_t i = 0; i < 4; i++) {
    header->did_I_malloc_this_dht_table[i] = false;
    header->did_I_malloc_this_dqt_table[i] = false;
  }
  header->is_COM_present = false;
  bool detect_fin = true;
  while (detect_fin) {
    switch (**current) {

    // SOI_SECTION
    case 0xffd8:
      break;

    // APPX_SECTION
    case 0xffe0:
      header->ptr_APP0 = read_APP0(fptr);
      break;

    // COM_SECTION
    case 0xfffe:
      header->is_COM_present = true;
      header->ptr_COM = read_COM(fptr);
      break;

    // DQT_SECTION
    case 0xffdb:
      read_DQT(fptr, header);
      break;

    // SOF0_SECTION
    case 0xffc0:
      header->ptr_SOF0 = read_SOF0(fptr);
      break;

    // DHT_SECTION
    case 0xffc4:
      read_DHT(fptr, header, option_h);
      break;

    // SOS_SECTION
    case 0xffda:
      header->ptr_SOS = read_SOS(fptr);
      detect_fin = false;
      break;

    default:
      fprintf(stderr, "ERROR while reading header : found invalid marker %x\n", **current);
      exit(EXIT_FAILURE);
    }

    fread(*current, sizeof(uint16_t), 1, *fptr);
    inverse_octets(*current);
  }
  return header;
}
