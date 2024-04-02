#ifndef __PPM__
#define __PPM__

# include "scan_head.h"
# include "traitement_MCUs.h"



/*écrit dans l'image PPM/PGM*/
extern void genere_image(const char * input_file, struct HEADER * header, struct group_MCU * group_MCU);


#endif