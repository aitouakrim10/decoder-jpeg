#ifndef __MEMORY__
#define __MEMORY__

# include "scan_head.h" 
# include "traitement_MCUs.h"



/*verifie après chaque malloc/calloc si tout s'est bien passé*/
extern void malloc_verify(char *error_message, void *ptr);


/*free la structure APP0*/
extern void free_APP0(struct APP0 *APP0);


/*free la structure COM*/
extern void free_COM(struct COM *COM);


/*free la structure DQT*/
extern void free_DQT(struct DQT *DQT);


/*free la structure HUFFMAN_TABLE*/
extern void free_HUFFMAN(struct HUFFMAN_TABLE *HUFFMAN_TABLE);


/*free la structure SOF0*/
extern void free_SOF0(struct SOF0 *SOF0);


/*free la structure SOS*/
extern void free_SOS(struct SOS *SOS);


/*free la structure header*/
extern void free_header(struct HEADER *header);


/*free la structure group_MCU*/
extern void free_group_MCU(struct group_MCU * group_MCU);


/*free la structure group_RGB*/
extern void free_group_RGB(struct group_RGB * group_RGB);


#endif