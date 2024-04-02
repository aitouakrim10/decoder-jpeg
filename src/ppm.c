# include <stdint.h>
# include <stdio.h>
# include "commun.h"
# include "scan_head.h"
# include "memory.h"
# include "traitement_MCUs.h"


/*écrit dans l'image PPM/PGM*/
void genere_image(const char * input_file, struct HEADER * header, struct group_MCU * group_MCU) {
  uint32_t nb_MCU_height = group_MCU -> nb_MCU_height;
  uint32_t nb_MCU_width = group_MCU -> nb_MCU_width;
  uint8_t vert_sampY = get_sampling_vert(header, 0);
  uint8_t horz_sampY = get_sampling_horz(header, 0);
  uint32_t height = header->ptr_SOF0->SOF0_height;
  uint32_t width = header->ptr_SOF0->SOF0_width;
  uint32_t width_limits = (horz_sampY << 3) - ((nb_MCU_width << 3) * horz_sampY - width);
  uint32_t height_limits = (vert_sampY << 3) - ((nb_MCU_height << 3) * vert_sampY - height);
  uint8_t nb_components = header->ptr_SOF0->SOF0_nb_components;

  FILE *fptr = fopen(input_file, "wb");
  char * image_type = "P6";
  if (nb_components == 1) {
    image_type = "P5";
    fprintf(fptr, "%s\n%u %u\n255\n",image_type, width, height);
    for (uint32_t i = 0; i < nb_MCU_height-1; i++) {
      for (uint8_t k = 0; k < 8; k++) {
        for (uint32_t j = 0; j < nb_MCU_width-1; j++) {
          fwrite(&group_MCU->group_MCU[i][j]->MCU_Y_blocs[0]->bloc_data[(k << 3)], sizeof(uint8_t), 8, fptr);
        }
        fwrite(&group_MCU->group_MCU[i][nb_MCU_width-1]->MCU_Y_blocs[0]->bloc_data[(k << 3)], sizeof(uint8_t),width_limits, fptr);
      }
    }
    for (uint8_t k = 0; k < height_limits; k++) {
      for (uint32_t j = 0; j < nb_MCU_width-1; j++) {
        fwrite(&group_MCU->group_MCU[nb_MCU_height-1][j]->MCU_Y_blocs[0]->bloc_data[(k << 3)], sizeof(uint8_t), 8, fptr);
      }
      fwrite(&group_MCU->group_MCU[nb_MCU_height-1][nb_MCU_width-1]->MCU_Y_blocs[0]->bloc_data[(k << 3)], sizeof(uint8_t), width_limits, fptr);
    }
    fclose(fptr);
    free_header(header);
    free_group_MCU(group_MCU);
  } else {
    uint16_t width_tripled = width_limits*3;
    fprintf(fptr, "%s\n%u %u\n255\n",image_type, width, height);
    struct group_RGB * group_RGB = convert_group_MCU_to_group_RGB(group_MCU, header);
    uint32_t temp = width_tripled;
    uint8_t temp_index = 0;
    uint32_t temp_h = height_limits;
    uint8_t temp_h_index = 0;
    for (uint32_t i = 0; i < nb_MCU_height-1; i++) {
      for (uint8_t line = 0; line < vert_sampY; line++) {
        for (uint8_t k = 0; k < 8; k++) {
          for (uint32_t j = 0; j < nb_MCU_width-1; j++) {
            for (uint8_t col = 0; col < horz_sampY; col++) {
              fwrite(&(group_RGB->group_RGB[i][j]->RGB_RGB_blocs[line * horz_sampY + col])->bloc_data[(k << 3)*3], sizeof(uint8_t),24, fptr);
            }
          }

          temp = width_tripled;
          temp_index = 0;
          while (true) {
            if (temp <= 24) {
              fwrite(&(group_RGB->group_RGB[i][nb_MCU_width-1]->RGB_RGB_blocs[line * horz_sampY + temp_index])->bloc_data[(k << 3)*3], sizeof(uint8_t), temp, fptr);
              break;
            } else {
              fwrite(&(group_RGB->group_RGB[i][nb_MCU_width-1]->RGB_RGB_blocs[line * horz_sampY + temp_index])->bloc_data[(k << 3)*3], sizeof(uint8_t), 24, fptr);
              temp-=24;
              temp_index++;
            }
          }
        }
      }
    }
    while (true) {
      if (temp_h <= 8) {
        for (uint8_t k = 0; k < temp_h; k++) {
          for (uint32_t j = 0; j < nb_MCU_width-1; j++) {
            for (uint8_t col = 0; col < horz_sampY; col++) {
              fwrite(&(group_RGB->group_RGB[nb_MCU_height-1][j]->RGB_RGB_blocs[temp_h_index * horz_sampY + col])->bloc_data[(k << 3)*3], sizeof(uint8_t), 24, fptr);
            }
          }
          temp = width_tripled;
          temp_index = 0;
          while (true) {
            if (temp <= 24) {
              fwrite(&(group_RGB->group_RGB[nb_MCU_height-1][nb_MCU_width-1]->RGB_RGB_blocs[temp_h_index * horz_sampY + temp_index])->bloc_data[(k << 3)*3], sizeof(uint8_t), temp, fptr);
              break;
            } else {
              fwrite(&(group_RGB->group_RGB[nb_MCU_height-1][nb_MCU_width-1]->RGB_RGB_blocs[temp_h_index * horz_sampY + temp_index])->bloc_data[(k << 3)*3], sizeof(uint8_t), 24, fptr);
              temp-=24;
              temp_index++;
            }
          }
        }
      break;
      } else {
        for (uint8_t k = 0; k < 8; k++) {
          for (uint32_t j = 0; j < nb_MCU_width-1; j++) {
            for (uint8_t col = 0; col < horz_sampY; col++) {
              fwrite(&(group_RGB->group_RGB[nb_MCU_height-1][j]->RGB_RGB_blocs[temp_h_index * horz_sampY + col])->bloc_data[(k << 3)*3], sizeof(uint8_t), 24, fptr);
            }
          }
          temp = width_tripled;
          temp_index = 0;
          while (true) {
            if (temp <= 24) {
              fwrite(&(group_RGB->group_RGB[nb_MCU_height-1][nb_MCU_width-1]->RGB_RGB_blocs[temp_h_index * horz_sampY + temp_index])->bloc_data[(k << 3)*3], sizeof(uint8_t), temp, fptr);
              break;
            } else {
              fwrite(&(group_RGB->group_RGB[nb_MCU_height-1][nb_MCU_width-1]->RGB_RGB_blocs[temp_h_index * horz_sampY + temp_index])->bloc_data[(k << 3)*3], sizeof(uint8_t), 24, fptr);
              temp-=24;
              temp_index++;
            }
          }
        }
        temp_h -= 8;
        temp_h_index++;
      }
    }
  fclose(fptr);
  free_header(header);
  free_group_MCU(group_MCU);
  free_group_RGB(group_RGB);
  }
}
