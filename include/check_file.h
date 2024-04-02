#ifndef __CHECK_FILE__
#define __CHECK_FILE__

# include <stdint.h>


/*recupère l'extention du fichier d'entrée*/
extern void check_if_input_valid(const char *filename);


/*génére le Path et le nom de l'image*/
extern void gen_outputFilePath(char * inputFilePath, uint8_t nb_components, char * outputFilePath);

#endif 
