# include <stdint.h>
# include <math.h>
# include "idct_dqtz.h"



/*constantes utilisées en iDCR rapide*/
const float zero_five_x_sqrt_8 = 1.414213;
const float sqrt_8 = 2.828427;
const float zero_five = 0.5;
const float inverse_sqrt_two = 0.707106;
const float cos6 = 0.382683;
const float cos3 = 0.831469;
const float cos1 = 0.980785;
const float sin6 = 0.923879;
const float sin3 = 0.555570;
const float sin1 = 0.195090;



/*fonction C dans la defintion de l'iDCT*/
float fonction_C(float lambda) {
  float l;
  l = (lambda == 0) ? inverse_sqrt_two : 1;
  return l;
}



/*dequantise un bloc*/
void dequantisation(int32_t *result, uint8_t *DQT_coefficients_zigzag,
                    struct bloc_frequ *decoded_huffman_bloc) {
  for (uint8_t i = 0; i < 64; i++) {
    result[i] = decoded_huffman_bloc->bloc_data[i] * DQT_coefficients_zigzag[i];
  }
}



/*ajoute l'offset et fait la saturation*/
uint8_t traite_float_offset(float resultat) {
  resultat += 128;
  if (resultat <= 0) {
    return 0;
  } else if (resultat >= 255) {
    return 255;
  } else {
    return (uint8_t)round(resultat);
  }
}



/*calcule l'iDCT d'un seul coefficient*/
uint8_t iDCT(uint8_t x, uint8_t y, int32_t *matrix, uint8_t n) {
  float resultat = 0;
  float cos_x = (((x << 1) + 1) * M_PI) / (n << 1);
  float cos_y = (((y << 1) + 1) * M_PI) / (n << 1);
  uint8_t n_1 = n - 1;
  for (uint8_t lambda = 0; lambda <= n_1; lambda++) {
    for (uint8_t mu = 0; mu <= n_1; mu++) {
      resultat += fonction_C(lambda) * fonction_C(mu) * cosf(cos_x * lambda) *
                  cosf(cos_y * mu) * matrix[lambda * n + mu];
    }
  }
  resultat = resultat / sqrtf(n << 1);
  return traite_float_offset(resultat);
}



/*calcule l'iDCT (par la définition) d'un bloc 8x8 (représenté en vecteur 64)*/
void traitement_bloc_iDCT(struct bloc_spatial *bloc_traite, int32_t *bloc, uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    for (uint8_t j = 0; j < n; j++) {
      bloc_traite->bloc_data[i * n + j] = iDCT(i, j, bloc, n);
    }
  }
}



/*calcule l'iDCT (rapide) d'un bloc 8x8 (représenté en vecteur 64)*/
void fast_iDCT(struct bloc_spatial * bloc_traite, int32_t *bloc, uint8_t n) {
  float buffer[64];
  for (uint8_t i = 0; i < n; ++i) {
    const float a0 = bloc[(0 << 3) + i] * sqrt_8;
    const float a1 = bloc[(4 << 3) + i] * sqrt_8;
    const float a2 = bloc[(2 << 3) + i] * sqrt_8;
    const float a3 = bloc[(6 << 3) + i] * sqrt_8;
    const float a4 = (bloc[(1 << 3) + i] - bloc[(7 << 3) + i]) * zero_five_x_sqrt_8;
    const float a5 = bloc[(3 << 3) + i] * 2;
    const float a6 = bloc[(5 << 3) + i] * 2;
    const float a7 = (bloc[(1 << 3) + i] + bloc[(7 << 3) + i]) * zero_five_x_sqrt_8;

    const float b0 = (a0 + a1) * zero_five;
    const float b1 = (a0 - a1) * zero_five;
    const float b2 = inverse_sqrt_two * (a2 * cos6 - a3 * sin6);
    const float b3 = inverse_sqrt_two * (a3 *  cos6 + a2 * sin6);
    const float b4 = (a4 + a6) * zero_five;
    const float b5 = (a7 - a5) * zero_five;
    const float b6 = (a4 - a6) * zero_five;
    const float b7 = (a5 + a7) * zero_five;

    const float c0 = (b0 + b3) * zero_five;
    const float c1 = (b1 + b2) * zero_five;
    const float c2 = (b1 - b2) * zero_five;
    const float c3 = (b0 - b3) * zero_five;
    const float c4 = b4 * cos3 - b7 * sin3;
    const float c5 = b5 * cos1 - b6 * sin1;
    const float c6 = b6 * cos1 + b5 * sin1;
    const float c7 = b7 * cos3 + b4 * sin3;

    buffer[(0 << 3) + i] = (c0 + c7) * zero_five;
    buffer[(1 << 3) + i] = (c1 + c6) * zero_five;
    buffer[(2 << 3) + i] = (c2 + c5) * zero_five;
    buffer[(3 << 3) + i] = (c3 + c4) * zero_five;
    buffer[(4 << 3) + i] = (c3 - c4) * zero_five;
    buffer[(5 << 3) + i] = (c2 - c5) * zero_five;
    buffer[(6 << 3) + i] = (c1 - c6) * zero_five;
    buffer[(7 << 3) + i] = (c0 - c7) * zero_five;

  }
  for (uint8_t i = 0; i < n; ++i) {
    const float a0 = buffer[0 + (i << 3)] * sqrt_8;
    const float a1 = buffer[4 + (i << 3)] * sqrt_8;
    const float a2 = buffer[2 + (i << 3)] * sqrt_8;
    const float a3 = buffer[6 + (i << 3)] * sqrt_8;
    const float a4 = (buffer[1 + (i << 3)] - buffer[7 + (i << 3)]) * zero_five_x_sqrt_8;
    const float a5 = buffer[3 + (i << 3)] * 2;
    const float a6 = buffer[5 + (i << 3)] * 2;
    const float a7 = (buffer[1 + (i << 3)] + buffer[7 + (i << 3)]) * zero_five_x_sqrt_8;

    const float b0 = (a0 + a1) * zero_five;
    const float b1 = (a0 - a1) * zero_five;
    const float b2 = inverse_sqrt_two * (a2 * cos6 - a3 * sin6);
    const float b3 = inverse_sqrt_two * (a3 *  cos6 + a2 * sin6);
    const float b4 = (a4 + a6) * zero_five;
    const float b5 = (a7 - a5) * zero_five;
    const float b6 = (a4 - a6) * zero_five;
    const float b7 = (a5 + a7) * zero_five;

    const float c0 = (b0 + b3) * zero_five;
    const float c1 = (b1 + b2) * zero_five;
    const float c2 = (b1 - b2) * zero_five;
    const float c3 = (b0 - b3) * zero_five;
    const float c4 = b4 * cos3 - b7 * sin3;
    const float c5 = b5 * cos1 - b6 * sin1;
    const float c6 = b6 * cos1 + b5 * sin1;
    const float c7 = b7 * cos3 + b4 * sin3;
    
    buffer[0 + (i << 3)] = (c0 + c7) * zero_five;
    buffer[1 + (i << 3)] = (c1 + c6) * zero_five;
    buffer[2 + (i << 3)] = (c2 + c5) * zero_five;
    buffer[3 + (i << 3)] = (c3 + c4) * zero_five;
    buffer[4 + (i << 3)] = (c3 - c4) * zero_five;
    buffer[5 + (i << 3)] = (c2 - c5) * zero_five;
    buffer[6 + (i << 3)] = (c1 - c6) * zero_five;
    buffer[7 + (i << 3)] = (c0 - c7) * zero_five;
  }

  for (uint8_t i = 0; i < n ; i++) {
    for (uint8_t j = 0; j < n; j++) {
      bloc_traite->bloc_data[(i << 3) + j] = traite_float_offset(buffer[(i << 3) + j]);
    }
  }

}
