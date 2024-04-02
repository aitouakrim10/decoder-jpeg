#ifndef __SCAN_HEAD__
#define __SCAN_HEAD__

# include <stdint.h>
# include <stdio.h>
# include <stdbool.h>


/*représente les données de l'entête*/
struct HEADER {
  struct APP0 *ptr_APP0;
  struct COM *ptr_COM;
  bool is_COM_present;
  struct DQT *ptr_DQT[4];
  bool did_I_malloc_this_dqt_table[4];
  uint8_t dqt_number;
  struct SOF0 *ptr_SOF0;
  struct HUFFMAN_TABLE *ptr_HUFFMAN_TABLE[4];
  bool did_I_malloc_this_dht_table[4];
  uint8_t dht_number;
  struct SOS *ptr_SOS;
};

/*représente les données de la partie APP0 de l'entête*/
struct APP0 {
  uint16_t APP0_lenght;
  char APP0_image_type[5];
  char APP0_version[4];
  char APP0_7bytes_trash[8];
};

/*représente les données de la partie COM de l'entête*/
struct COM {
  uint16_t COM_lenght;
  char COM_commentary[];
};

/*représente les données d'une partie DQT de l'entête*/
struct DQT {
  uint16_t DQT_lenght;
  uint8_t DQT_precision;
  uint8_t DQT_index;
  uint8_t DQT_coefficients_zigzag[64];
};

/*représente les données de la partie SOF0 de l'entête*/
struct SOF0 {
  uint16_t SOF0_lenght;
  uint8_t SOF0_precision;
  uint16_t SOF0_height;
  uint16_t SOF0_width;
  uint8_t SOF0_nb_components;
  struct SOF0_data_component *SOF0_data_components[];
};

/*représente les données des composante de la partie SOF0*/
struct SOF0_data_component {
  uint8_t index_C;
  int8_t vertical_sampling_factor;
  int8_t horizontal_sampling_factor;
  uint8_t index_Q;
};

/*représente un arbre de Huffman*/
struct TREE {
  uint8_t index;
  uint32_t lenght;
  uint32_t profondeur;
  bool option_h;
  struct huff_symbols * huff_symbols;
  struct huff_codes * huff_codes;
  uint32_t how_many_codes_done;
  uint16_t tree[];
};

/*représente un arbre d'une partie DHT de l'entête*/
struct HUFFMAN_TABLE {
  uint16_t HUFFMAN_TABLE_lenght;
  uint8_t HUFFMAN_TABLE_type;
  uint8_t HUFFMAN_TABLE_index;
  struct TREE * HUFFMAN_TABLE_TREE;
};

/*stocke les symbols de huffman d'un arbre*/
struct huff_symbols {
  uint32_t lenght;
  uint8_t symbols[];
};

/*stocke les mots de codes d'un arbre de huffman*/
struct huff_codes {
  uint32_t lenght;
  char * codes_list[];
};

/*représente les données d'une partie SOS de l'entête*/
struct SOS {
  uint16_t SOS_lenght;
  uint8_t SOS_nb_components;
  uint8_t SOS_ss;
  uint8_t SOS_se;
  uint8_t SOS_ah;
  uint8_t SOS_al;
  struct SOS_data_component *SOS_data_components[];
};

/*représente les données des composante de la partie SOS*/
struct SOS_data_component {
  uint8_t index_C;
  uint8_t index_H_DC;
  uint8_t index_H_AC;
};

/*par exemple en utilisant fread au debut d'image on lira 0xff puis 0xd8,
cela stockera 0xd8 ff au lieu de 0xff d8, cette fontion règle le problème*/
extern void inverse_octets(uint16_t *octets_cible);


/*affiche sur la stdout les données de l'entête (option -h)*/
extern void test_header(struct HEADER *header);


/*lit la partie APP0 de l'entête de l'image*/
extern struct APP0 *read_APP0(FILE **fptr);


/*lit la partie COM de l'entête de l'image*/
extern struct COM *read_COM(FILE **fptr);


/*lit la partie DQT de l'entête de l'image*/
extern void read_DQT(FILE **fptr, struct HEADER * header);


/*lit la partie SOF0 de l'entête de l'image*/
extern struct SOF0 *read_SOF0(FILE **fptr);


/*lit la partie DHT de l'entête de l'image*/
extern void read_DHT(FILE **fptr, struct HEADER * header, bool option_h);


/*lit la partie SOS de l'entête de l'image*/
extern struct SOS *read_SOS(FILE **fptr);


/*lit l'entête de l'image*/
extern struct HEADER *scan_header(FILE **fptr, uint16_t **current, bool option_h);


#endif

