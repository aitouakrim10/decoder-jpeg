#ifndef __COMMUN__
#define __COMMUN__


# include "idct_dqtz.h"
# include "scan_head.h"
#include <stdint.h>
#include <stdbool.h>




/*affiche les symbols de Huffman uttilisés et leurs mots de code*/
extern void print_tas_huffman(struct TREE * tree);


/*affiche les coefficients d'un bloc 8x8 (représenté comme vecteur 64)
-le bool hex_or_dec choisit le mode (hex/dec) d'affichage des coefficients
*/
extern void print_result_decoded(struct bloc_spatial *tab, bool hex_or_dec);


/*affiche un vecteur d'octets de taille 64 sous forme 8x8*/
extern void afficher_vecteur_uint8(uint8_t *vecteur);


/*recupère le Vertical Sampling Factor d'une composante (Y,Cb,Cr)*/
extern int8_t get_sampling_vert(struct HEADER *header, uint8_t component);


/*recupère le Horizontal Sampling Factor d'une composante (Y,Cb,Cr)*/
extern int8_t get_sampling_horz(struct HEADER *header, uint8_t component);


/*recupère le nombre de blocs frequentiels 8x8 par ligne*/
extern uint32_t get_nb_bloc_frequ_width(struct HEADER *header);


/*recupère le nombre de blocs frequentiels 8x8 par colonne*/
extern uint32_t get_nb_bloc_frequ_height(struct HEADER *header);


/*recupère l'indice de la table DQT associé à une composante*/
extern uint8_t get_iQ_dqt(struct HEADER *header, uint8_t component);


/*recupère l'indice de l'arbre de huffman associé à une composante*/
extern struct TREE * get_tree(struct HEADER *header, uint8_t iH, uint8_t type);


/*recupère l'indice de l'arbre AC associé à une composante*/
extern uint8_t get_index_HUFF_AC(struct HEADER *header, uint8_t component);


/*recupère l'indice de l'arbre DC associé à une composante*/
extern uint8_t get_index_HUFF_DC(struct HEADER *header, uint8_t component);


#endif 
