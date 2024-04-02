#ifndef __IDCT_DQTZ__
#define __IDCT_DQTZ__
# define M_PI 3.1415927
# include <stdint.h>



/*représente un bloc fréquetiel (64 octects)*/
struct bloc_frequ {
  int32_t bloc_data[64];
};


/*représente un bloc spatial (64 octects)*/
struct bloc_spatial {
  uint8_t bloc_data[64];
};



/*structure utilsée pour représenter les blocs d'une RGB (64x3 octects) pour accélérer
l'écriture dans l'image (écriture par paquet de 8x3 octects)*/
struct bloc_spatial_x3 {
  uint8_t bloc_data[192];
};


/*fonction C dans la defintion de l'iDCT*/
extern float fonction_C(float lambda);


/*dequantise un bloc*/
extern void dequantisation(int32_t *result, uint8_t *DQT_coefficients_zigzag, struct bloc_frequ *decoded_huffman_bloc);


/*ajoute l'offset et fait la saturation*/
extern uint8_t traite_float_offset(float resultat);


/*calcule l'iDCT d'un seul coefficient*/
extern uint8_t iDCT(uint8_t x, uint8_t y, int32_t *matrix, uint8_t n);


/*calcule l'iDCT (par la définition) d'un bloc 8x8 (représenté en vecteur 64)*/
extern void traitement_bloc_iDCT(struct bloc_spatial *bloc_traite, int32_t *bloc, uint8_t n);


/*calcule l'iDCT (rapide) d'un bloc 8x8 (représenté en vecteur 64)*/
extern void fast_iDCT(struct bloc_spatial * bloc_traite, int32_t *bloc, uint8_t n);

#endif

