/*
 * sha1.h
 *
 * Description :
 * Fichier d�en-t�te pour le code qui met en �uvre l�algorithme 1 de hachage s�curis� tel que d�fini dans la norme FIPS
 * PUB 180-1 publi�e le 17 avril 1995. Beaucoup des noms de variables de ce code, en particulier les noms de caract�res
 * seuls, ont �t� utilis�s parce que ces noms sont utilis�s dans la norme publi�e. Pour plus d�informations, pri�re de se
 * reporter � la lecture du fichier sha1.c.
 */

#ifndef SHA1_H_INCLUDED
#define SHA1_H_INCLUDED

#include <stdio.h>
/*
 * Si vous n�avez pas le fichier d�en-t�te stdint.h de la norme ISO, vous devrez alors taper ce qui suit :
 * name             signification
 * uint32_t         entier non sign� de 32 bits
 * uint8_t          entier non sign� de 8 bits (c�est-�-dire, un caract�re non sign�)
 * int_least16_t    entier >= 16 bits
 */

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* param�tre pointeur Nul */
    shaInputTooLong,    /* donn�es d�entr�e trop longues */
    shaStateError       /* appel d�entr�e apr�s r�sultat */
};
#endif
#define SHA1HashSize 20

#include <stdint.h>

/*
 * Cette structure va contenir les informations de contexte pour l�op�ration de hachage SHA-1
 */
typedef struct SHA1Context
{
    uint32_t Intermediate_Hash[SHA1HashSize/4];     /* R�sum� de message */
    uint32_t Length_Low;                            /* Longueur du message en bits */
    uint32_t Length_High;                           /* Longueur du message en bits */

    /* Indice dans la matrice de bloc de message */

    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64];                      /* Blocs de message de 512 bits */
    int Computed;                                   /* Le r�sum� est-il calcul� ? */
    int Corrupted;                                  /* Le r�sum� de message est-il corrompu ? */
} SHA1Context;

/*
 * Prototypes de fonction
 */

int SHA1Reset(SHA1Context *);
int SHA1Input(SHA1Context *, const uint8_t *, unsigned int);
int SHA1Result( SHA1Context *, uint8_t Message_Digest[SHA1HashSize]);

#endif // SHA1_H_INCLUDED
