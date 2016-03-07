	
/* c206.c **********************************************************}
{* T�ma: Dvousm�rn� v�zan� line�rn� seznam
**
**                   N�vrh a referen�n� implementace: Bohuslav K�ena, ��jen 2001
**                            P�epracovan� do jazyka C: Martin Tu�ek, ��jen 2004
**                                            �pravy: Bohuslav K�ena, ��jen 2015
**
** Implementujte abstraktn� datov� typ dvousm�rn� v�zan� line�rn� seznam.
** U�ite�n�m obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datov� abstrakce reprezentov�n prom�nnou
** typu tDLList (DL znamen� Double-Linked a slou�� pro odli�en�
** jmen konstant, typ� a funkc� od jmen u jednosm�rn� v�zan�ho line�rn�ho
** seznamu). Definici konstant a typ� naleznete v hlavi�kov�m souboru c206.h.
**
** Va��m �kolem je implementovat n�sleduj�c� operace, kter� spolu
** s v��e uvedenou datovou ��st� abstrakce tvo�� abstraktn� datov� typ
** obousm�rn� v�zan� line�rn� seznam:
**
**      DLInitList ...... inicializace seznamu p�ed prvn�m pou�it�m,
**      DLDisposeList ... zru�en� v�ech prvk� seznamu,
**      DLInsertFirst ... vlo�en� prvku na za��tek seznamu,
**      DLInsertLast .... vlo�en� prvku na konec seznamu, 
**      DLFirst ......... nastaven� aktivity na prvn� prvek,
**      DLLast .......... nastaven� aktivity na posledn� prvek, 
**      DLCopyFirst ..... vrac� hodnotu prvn�ho prvku,
**      DLCopyLast ...... vrac� hodnotu posledn�ho prvku, 
**      DLDeleteFirst ... zru�� prvn� prvek seznamu,
**      DLDeleteLast .... zru�� posledn� prvek seznamu, 
**      DLPostDelete .... ru�� prvek za aktivn�m prvkem,
**      DLPreDelete ..... ru�� prvek p�ed aktivn�m prvkem, 
**      DLPostInsert .... vlo�� nov� prvek za aktivn� prvek seznamu,
**      DLPreInsert ..... vlo�� nov� prvek p�ed aktivn� prvek seznamu,
**      DLCopy .......... vrac� hodnotu aktivn�ho prvku,
**      DLActualize ..... p�ep�e obsah aktivn�ho prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na dal�� prvek seznamu,
**      DLPred .......... posune aktivitu na p�edchoz� prvek seznamu, 
**      DLActive ........ zji��uje aktivitu seznamu.
**
** P�i implementaci jednotliv�ch funkc� nevolejte ��dnou z funkc�
** implementovan�ch v r�mci tohoto p��kladu, nen�-li u funkce
** explicitn� uvedeno n�co jin�ho.
**
** Nemus�te o�et�ovat situaci, kdy m�sto leg�ln�ho ukazatele na seznam 
** p�ed� n�kdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodn� komentujte!
**
** Terminologick� pozn�mka: Jazyk C nepou��v� pojem procedura.
** Proto zde pou��v�me pojem funkce i pro operace, kter� by byly
** v algoritmick�m jazyce Pascalovsk�ho typu implemenov�ny jako
** procedury (v jazyce C procedur�m odpov�daj� funkce vracej�c� typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozorn�n� na to, �e do�lo k chyb�.
** Tato funkce bude vol�na z n�kter�ch d�le implementovan�ch operac�.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* glob�ln� prom�nn� -- p��znak o�et�en� chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L p�ed jeho prvn�m pou�it�m (tzn. ��dn�
** z n�sleduj�c�ch funkc� nebude vol�na nad neinicializovan�m seznamem).
** Tato inicializace se nikdy nebude prov�d�t nad ji� inicializovan�m
** seznamem, a proto tuto mo�nost neo�et�ujte. V�dy p�edpokl�dejte,
** �e neinicializovan� prom�nn� maj� nedefinovanou hodnotu.
**/

    L->First = L->Act = L->Last = NULL;    
}

void DLDisposeList (tDLList *L) {
/*
** Zru�� v�echny prvky seznamu L a uvede seznam do stavu, v jak�m
** se nach�zel po inicializaci. Ru�en� prvky seznamu budou korektn�
** uvoln�ny vol�n�m operace free. 
**/

    tDLElemPtr tmp = L->First;

    // Until temp. var. is not NULL...
    while(tmp != NULL) {
        // ...check if we are not at the end of the list...
        if(L->First != L->Last)
            L->First = tmp->rptr;
        else
            L->First = L->Last = NULL;
        
        // ...deallocate memory for current item...
        free(tmp);
        // ...and move to the next one.
        tmp = L->First;
    }

    L->Act = NULL;
}

void DLInsertFirst (tDLList *L, int val) {
/*
** Vlo�� nov� prvek na za��tek seznamu L.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
**/

    // Allocate and initialize new item
    tDLElemPtr n = malloc(sizeof *n);
    if(n == NULL) {
        DLError();
        return;
    }

    n->data = val;
    // Move first item to the right
    n->rptr = L->First;
    // Unset left pointer (we are the leftmost one)
    n->lptr = NULL;
    // If we are not the only one, set left pointer of our neighbor to us
    if(L->First != NULL)
        L->First->lptr = n;
    // If we actually are the first one, we are also the last one
    if(L->Last == NULL)
        L->Last = n; 
    // And finally make us the first one
    L->First = n;
}

void DLInsertLast(tDLList *L, int val) {
/*
** Vlo�� nov� prvek na konec seznamu L (symetrick� operace k DLInsertFirst).
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
**/ 	

    // Allocate and initialize new item
    tDLElemPtr n = malloc(sizeof *n);
    if(n == NULL) {
        DLError();
        return;
    }

    n->data = val;
    // Unset right pointer (we are the rightmost one)
    n->rptr = NULL;
    // Move last item to the left
    n->lptr = L->Last;
    // If we are the only one in the list, we are last and first one
    if(L->First == NULL)
        L->First = n;
    // If not, set right pointer of our neighbor to us
    if(L->Last != NULL)
        L->Last->rptr = n;
    // And make us the last one
    L->Last = n;
}

void DLFirst (tDLList *L) {
/*
** Nastav� aktivitu na prvn� prvek seznamu L.
** Funkci implementujte jako jedin� p��kaz (nepo��t�me-li return),
** ani� byste testovali, zda je seznam L pr�zdn�.
**/
	
    L->Act = L->First;
}

void DLLast (tDLList *L) {
/*
** Nastav� aktivitu na posledn� prvek seznamu L.
** Funkci implementujte jako jedin� p��kaz (nepo��t�me-li return),
** ani� byste testovali, zda je seznam L pr�zdn�.
**/

    L->Act = L->Last;	
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prost�ednictv�m parametru val vr�t� hodnotu prvn�ho prvku seznamu L.
** Pokud je seznam L pr�zdn�, vol� funkci DLError().
**/

    if(L->First == NULL) {
        DLError();
        return;
    }

    *val = L->First->data; 	
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prost�ednictv�m parametru val vr�t� hodnotu posledn�ho prvku seznamu L.
** Pokud je seznam L pr�zdn�, vol� funkci DLError().
**/

    if(L->Last == NULL) {
        DLError();
        return;
    }

    *val = L->Last->data;	
}

void DLDeleteFirst (tDLList *L) {
/*
** Zru�� prvn� prvek seznamu L. Pokud byl prvn� prvek aktivn�, aktivita 
** se ztr�c�. Pokud byl seznam L pr�zdn�, nic se ned�je.
**/

    if(L->First != NULL) {
        tDLElemPtr tmp = L->First;

        // Make our right neighbour the first one
        L->First = L->First->rptr;
        L->First->lptr = NULL;

        // If we are the last one, uset Last pointer
        if(tmp == L->Last)
            L->Last = NULL;

        // And deallocate delisted item
        free(tmp);        
    }
}	

void DLDeleteLast (tDLList *L) {
/*
** Zru�� posledn� prvek seznamu L. Pokud byl posledn� prvek aktivn�,
** aktivita seznamu se ztr�c�. Pokud byl seznam L pr�zdn�, nic se ned�je.
**/ 

    if(L->Last != NULL) {	
	    tDLElemPtr tmp = L->Last;

        // Make our left neighbour the last one
        L->Last = L->Last->lptr;
        L->Last->rptr = NULL;

        // If we are the last one, unset First pointer
        if(tmp == L->First)
            L->First = NULL;

        // And deallocate delisted item
        free(tmp);
    }
}

void DLPostDelete (tDLList *L) {
/*
** Zru�� prvek seznamu L za aktivn�m prvkem.
** Pokud je seznam L neaktivn� nebo pokud je aktivn� prvek
** posledn�m prvkem seznamu, nic se ned�je.
**/
	
    tDLElemPtr tmp = L->Act;

    // If there's an active item and it's not the last one...
    if(tmp != NULL && tmp->rptr != NULL) {
        // ...save the right neighbour of the active item...
        tmp = tmp->rptr;
        // ...remove information about it from the active item...
        L->Act->rptr = tmp->rptr;
        // ...if it's possible, remove the same information from the
        // neighbor on the other side...
        if(tmp->rptr != NULL)
            tmp->rptr->lptr = L->Act;
        // ...if it was the last item, set the active item as the last one...
        if(tmp == L->Last)
            L->Last = L->Act;

        // ...and destroy the saved item.
        free(tmp);
    }		
}

void DLPreDelete (tDLList *L) {
/*
** Zru�� prvek p�ed aktivn�m prvkem seznamu L .
** Pokud je seznam L neaktivn� nebo pokud je aktivn� prvek
** prvn�m prvkem seznamu, nic se ned�je.
**/

    tDLElemPtr tmp = L->Act;

    // If there's an active item and it's not the last one...
    if(tmp != NULL && tmp->lptr != NULL) {
        // ...save the left neigbor of the active item...
        tmp = tmp->lptr;
        // ...remove information about it from the active item...
        L->Act->lptr = tmp->lptr;
        // ...if it's possible, remove the same infomation from the
        // neighbor on the other side...
        if(tmp->lptr != NULL)
            tmp->lptr->rptr = L->Act;
        // ...if it was the first item, set the active item as the first one...
        if(tmp == L->First)
            L->First = L->Act;

        // ...and destroy the saved item.
        free(tmp);       
    }
}

void DLPostInsert (tDLList *L, int val) {
/*
** Vlo�� prvek za aktivn� prvek seznamu L.
** Pokud nebyl seznam L aktivn�, nic se ned�je.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
**/

    // Allocate and initialize new item
    if(L->Act != NULL) {
        tDLElemPtr n = malloc(sizeof *n);
        if(n == NULL) {
            DLError();
            return;
        }

        n->data = val;
        // Left pointer should point to the active item
        n->lptr = L->Act;
        // Right pointer should point to former right neigbor
        // of the active item
        n->rptr = L->Act->rptr;
        // Fix right pointer of the active item
        L->Act->rptr = n;
        // If our right neighbor really exists, tell him about
        // our existence
        if(n->rptr != NULL)
            n->rptr->lptr = n;
        // If not, we are most likely the last one
        else
            L->Last = n;
    }
}

void DLPreInsert (tDLList *L, int val) {
/*
** Vlo�� prvek p�ed aktivn� prvek seznamu L.
** Pokud nebyl seznam L aktivn�, nic se ned�je.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
**/

    // Allocate and initialize new item
    if(L->Act != NULL) {
        tDLElemPtr n = malloc(sizeof *n);
        if(n == NULL) {
            DLError();
            return;
        }

        n->data = val;
        // Righ pointer should point to the active item
        n->rptr = L->Act;
        // Left pointer should point to former left neighbor
        // of the active item
        n->lptr = L->Act->lptr;
        // Fix left pointer of the active item
        L->Act->lptr = n;
        // If our left neighbor really exists, tell him about
        // our existence
        if(n->lptr != NULL)
            n->lptr->rptr = n;
        // If not, we are most likely the first one
        else
            L->First = n;
    }	
}

void DLCopy (tDLList *L, int *val) {
/*
** Prost�ednictv�m parametru val vr�t� hodnotu aktivn�ho prvku seznamu L.
** Pokud seznam L nen� aktivn�, vol� funkci DLError ().
**/

    if(L->Act == NULL) {
        DLError();
        return;
    }

    *val = L->Act->data;
}

void DLActualize (tDLList *L, int val) {
/*
** P�ep�e obsah aktivn�ho prvku seznamu L.
** Pokud seznam L nen� aktivn�, ned�l� nic.
**/

    if(L->Act != NULL)
        L->Act->data = val;
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na n�sleduj�c� prvek seznamu L.
** Nen�-li seznam aktivn�, ned�l� nic.
** V�imn�te si, �e p�i aktivit� na posledn�m prvku se seznam stane neaktivn�m.
**/

    if(L->Act != NULL)
        L->Act = L->Act->rptr;
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na p�edchoz� prvek seznamu L.
** Nen�-li seznam aktivn�, ned�l� nic.
** V�imn�te si, �e p�i aktivit� na prvn�m prvku se seznam stane neaktivn�m.
**/

    if(L->Act != NULL)
        L->Act = L->Act->lptr;
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivn�, vrac� nenulovou hodnotu, jinak vrac� 0.
** Funkci je vhodn� implementovat jedn�m p��kazem return.
**/

    return (L->Act != NULL);
}

/* Konec c206.c*/
