	
/* c206.c **********************************************************}
{* Téma: Dvousmìrnì vázaný lineární seznam
**
**                   Návrh a referenèní implementace: Bohuslav Køena, øíjen 2001
**                            Pøepracované do jazyka C: Martin Tuèek, øíjen 2004
**                                            Úpravy: Bohuslav Køena, øíjen 2015
**
** Implementujte abstraktní datový typ dvousmìrnì vázaný lineární seznam.
** U¾iteèným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován promìnnou
** typu tDLList (DL znamená Double-Linked a slou¾í pro odli¹ení
** jmen konstant, typù a funkcí od jmen u jednosmìrnì vázaného lineárního
** seznamu). Definici konstant a typù naleznete v hlavièkovém souboru c206.h.
**
** Va¹ím úkolem je implementovat následující operace, které spolu
** s vý¹e uvedenou datovou èástí abstrakce tvoøí abstraktní datový typ
** obousmìrnì vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu pøed prvním pou¾itím,
**      DLDisposeList ... zru¹ení v¹ech prvkù seznamu,
**      DLInsertFirst ... vlo¾ení prvku na zaèátek seznamu,
**      DLInsertLast .... vlo¾ení prvku na konec seznamu, 
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek, 
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku, 
**      DLDeleteFirst ... zru¹í první prvek seznamu,
**      DLDeleteLast .... zru¹í poslední prvek seznamu, 
**      DLPostDelete .... ru¹í prvek za aktivním prvkem,
**      DLPreDelete ..... ru¹í prvek pøed aktivním prvkem, 
**      DLPostInsert .... vlo¾í nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vlo¾í nový prvek pøed aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... pøepí¹e obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na dal¹í prvek seznamu,
**      DLPred .......... posune aktivitu na pøedchozí prvek seznamu, 
**      DLActive ........ zji¹»uje aktivitu seznamu.
**
** Pøi implementaci jednotlivých funkcí nevolejte ¾ádnou z funkcí
** implementovaných v rámci tohoto pøíkladu, není-li u funkce
** explicitnì uvedeno nìco jiného.
**
** Nemusíte o¹etøovat situaci, kdy místo legálního ukazatele na seznam 
** pøedá nìkdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodnì komentujte!
**
** Terminologická poznámka: Jazyk C nepou¾ívá pojem procedura.
** Proto zde pou¾íváme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozornìní na to, ¾e do¹lo k chybì.
** Tato funkce bude volána z nìkterých dále implementovaných operací.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální promìnná -- pøíznak o¹etøení chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L pøed jeho prvním pou¾itím (tzn. ¾ádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádìt nad ji¾ inicializovaným
** seznamem, a proto tuto mo¾nost neo¹etøujte. V¾dy pøedpokládejte,
** ¾e neinicializované promìnné mají nedefinovanou hodnotu.
**/

    L->First = L->Act = L->Last = NULL;    
}

void DLDisposeList (tDLList *L) {
/*
** Zru¹í v¹echny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Ru¹ené prvky seznamu budou korektnì
** uvolnìny voláním operace free. 
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
** Vlo¾í nový prvek na zaèátek seznamu L.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
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
** Vlo¾í nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
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
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/
	
    L->Act = L->First;
}

void DLLast (tDLList *L) {
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/

    L->Act = L->Last;	
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/

    if(L->First == NULL) {
        DLError();
        return;
    }

    *val = L->First->data; 	
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/

    if(L->Last == NULL) {
        DLError();
        return;
    }

    *val = L->Last->data;	
}

void DLDeleteFirst (tDLList *L) {
/*
** Zru¹í první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
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
** Zru¹í poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
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
** Zru¹í prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se nedìje.
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
** Zru¹í prvek pøed aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se nedìje.
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
** Vlo¾í prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
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
** Vlo¾í prvek pøed aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
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
** Prostøednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/

    if(L->Act == NULL) {
        DLError();
        return;
    }

    *val = L->Act->data;
}

void DLActualize (tDLList *L, int val) {
/*
** Pøepí¹e obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedìlá nic.
**/

    if(L->Act != NULL)
        L->Act->data = val;
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na posledním prvku se seznam stane neaktivním.
**/

    if(L->Act != NULL)
        L->Act = L->Act->rptr;
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na pøedchozí prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na prvním prvku se seznam stane neaktivním.
**/

    if(L->Act != NULL)
        L->Act = L->Act->lptr;
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním pøíkazem return.
**/

    return (L->Act != NULL);
}

/* Konec c206.c*/
