/*
 * sha1.c
 *
 * Description:
 * Ce fichier met en œuvre l’algorithme 1 de hachage sécurisé tel que défini dans la norme FIPS PUB 180-1 publiée le
 * 17 avril 1995. SHA-1 produit un résumé de message de 160 bits pour un certain flux de données. Il faudra environ
 * 2**n étapes pour trouver un message avec le même résumé que celui de ce message et 2**(n/2) pour trouver deux
 * messages avec le même résumé, lorsque n est la taille en bits du résumé. Cet algorithme peut donc servir de moyen pour
 * donner une "empreinte digitale" d’un message.
 *
 * Questions de portabilité :
 * SHA-1 est défini en termes de "mots" de 32 bits. Ce code utilise <stdint.h> (inclus dans "sha1.h" pour définir des types
 * d’entier non signé de 32 et 8 bits. Si votre compilateur de langage C n’accepte pas les entiers non signés de 32 bits,
 * ce code n’est pas approprié.
 *
 * Avertissements :
 * SHA-1 est conçu pour fonctionner avec des messages de moins de 2^64 bits. Bien que SHA-1 permette que soit généré
 * un résumé de message pour des messages de tout nombre de bits inférieur à 2^64 bits, cette mise en œuvre ne
 * fonctionne qu’avec les messages d’une longueur multiple de la taille d’un caractère de 8 bits.
 *
 */

#include "../include/sha1.h"

/*
 * Definit la macro SHA1 permutation circulaire à gauche
 */

#define SHA1CircularShift(bits,word)  (((word) << (bits)) | ((word) >> (32-(bits))))

/* Prototyptes de fonction locale */
void SHA1PadMessage(SHA1Context *);
void SHA1ProcessMessageBlock(SHA1Context *);

/*
 * SHA1Reset
 *
 * Description :
 * Cette fonction va initialiser le SHA1Context pour préparer le calcul d’un nouveau résumé de message SHA1.
 *
 * Paramètres : context: [in/out]
 *
 * Résultat : Code d’erreur sha.
 *
 */

int SHA1Reset(SHA1Context *context)
{
    if (!context)
    {
        return shaNull;
    }

    context->Length_Low             = 0;
    context->Length_High            = 0;
    context->Message_Block_Index    = 0;

    context->Intermediate_Hash[0]   = 0x67452301;
    context->Intermediate_Hash[1]   = 0xEFCDAB89;
    context->Intermediate_Hash[2]   = 0x98BADCFE;
    context->Intermediate_Hash[3]   = 0x10325476;
    context->Intermediate_Hash[4]   = 0xC3D2E1F0;

    context->Computed   = 0;
    context->Corrupted  = 0;

    return shaSuccess;
}

/*
 *  SHA1Result
 *
 * Description:
 * Cette fonction va retourner le résumé de message de 160 bits dans la matrice Message_Digest fournie par l’appelant.
 * Noter que le premier octet du hachage est mémorisé dans l’élément 0, le dernier octet du hachage dans l’élément 19.
 *
 * Paramètres :
 *      context: [in/out]
 * C’est le contexte à utiliser pour calculer le hachage SHA-1.
 *      Message_Digest: [out]
 * Où le résumé sera renvoyé.
 *
 * Résultat : Code d’erreur sha.
 *
 */

int SHA1Result( SHA1Context *context, uint8_t Message_Digest[SHA1HashSize])
{
    int i;

    if (!context || !Message_Digest)
    {
        return shaNull;
    }

    if (context->Corrupted)
    {
        return context->Corrupted;
    }

    if (!context->Computed)
    {
        SHA1PadMessage(context);
        for(i=0; i<64; ++i)
        {
            /* le message peut être sensible, l’éliminer */
            context->Message_Block[i] = 0;
        }

        context->Length_Low = 0;    /* et éliminer la longueur */
        context->Length_High = 0;
        context->Computed = 1;
    }

    for(i = 0; i < SHA1HashSize; ++i)
    {
        Message_Digest[i] = context->Intermediate_Hash[i>>2] >> 8 * ( 3 - ( i & 0x03 ) );
    }

    return shaSuccess;
}

/*
 * SHA1Input
 *
 * Description :
 * Cette fonction accepte une matrice d’octets comme portion suivante du message.
 *
 * Paramètres :
 *      context: [in/out]
 * C’est le contexte SHA à mettre à jour *
 *      message_array: [in]
 * Matrice de caractères qui représentent la prochaine portion du message.
 *
 *      length: [in]
 * Longueur du message dans message_array
 *
 * Retours :
 * Code d’erreur sha.*
 */

int SHA1Input(    SHA1Context    *context,
const uint8_t  *message_array,
unsigned       length)
{
    if (!length)
    {
        return shaSuccess;
    }

    if (!context || !message_array)
    {
        return shaNull;
    }

    if (context->Computed)
    {
        context->Corrupted = shaStateError;
        return shaStateError;
    }

    if (context->Corrupted)
    {
         return context->Corrupted;
    }

    while(length-- && !context->Corrupted)
    {
    context->Message_Block[context->Message_Block_Index++] = (*message_array & 0xFF);

    context->Length_Low += 8;
    if (context->Length_Low == 0)
        {
        context->Length_High++;
        if (context->Length_High == 0)
        {
            /* Message is too long */
            context->Corrupted = 1;
        }
    }
    if (context->Message_Block_Index == 64)
    {
        SHA1ProcessMessageBlock(context);
    }
    message_array++;
    }

    return shaSuccess;
}

/*
 * SHA1ProcessMessageBlock
 *
 * Description :
 * Cette fonction va traiter les 512 prochains bits du message mémorisés dans la matrice Message_Block.
 *
 * Paramètres :
 * Aucun.
 *
 * Retour :
 * Rien.
 *
 * Commentaires :
 * Beaucoup des noms de variables de ce code, en particulier ceux des noms de caractères seuls, ont été utilisés parce que
 * ce sont ceux qui sont utilisés dans la norme publiée.*
 */

void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const uint32_t K[] =    {   /* Constantes définies dans SHA-1   */
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    };

    int t;                      /* compteur de boucle */
    uint32_t temp;              /* valeur de mot temporaire */
    uint32_t W[80];             /* séquence  de mot */
    uint32_t A, B, C, D, E;     /* mémoires tampons de mots */

/*
 * Initialise les 16 premiers mots dans la matrice W
 */

    for(t = 0; t < 16; t++)
    {
        W[t] = (uint32_t)(context->Message_Block[t * 4]) << 24;
        W[t] |= (uint32_t)(context->Message_Block[t * 4 + 1]) << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;
    context->Message_Block_Index = 0;
}

/*
 * SHA1PadMessage
 *

 * Description:
 * Conformément à la norme, le message doit être bourré pour faire 512 bits. Le premier bit de bourrage doit être un '1'.

 * Les 64 derniers bits représentent la longueur du message d’origine. Tous les bits entre devraient être à 0.
 * Cette fonction bourre le message conformément à ces règles en remplissant en conséquence la matrice Message_Block.
 * Elle va aussi invoquer la fonction ProcessMessageBlock fournie de façon appropriée.
 * Quand elle a fini, on peut supposer que le résumé de message a été calculé.
 *
 * Paramètres :
 *      context: [in/out]           Le contexte à bourrer
 *      ProcessMessageBlock: [in]   C’est la fonction SHA*ProcessMessageBlock appropriée
 * Retour : Rien.
 */

void SHA1PadMessage(SHA1Context *context)
{
/*
 * Vérifier si le bloc de message actuel est trop petit pour contenir les bits et la longueur du bourrage initial.
 * Si oui, on va bourrer le bloc, le traiter, puis continuer le bourrage dans un second bloc.
 */

    if (context->Message_Block_Index > 55)
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }

        SHA1ProcessMessageBlock(context);

        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }
    else
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }

/*
 * Mémorise la longueur de message sous les 8 derniers octets
 */

    context->Message_Block[56] = context->Length_High >> 24;
    context->Message_Block[57] = context->Length_High >> 16;
    context->Message_Block[58] = context->Length_High >> 8;
    context->Message_Block[59] = context->Length_High;
    context->Message_Block[60] = context->Length_Low >> 24;
    context->Message_Block[61] = context->Length_Low >> 16;
    context->Message_Block[62] = context->Length_Low >> 8;
    context->Message_Block[63] = context->Length_Low;


    SHA1ProcessMessageBlock(context);
}
