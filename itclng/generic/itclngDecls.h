/*
 * $Id: itclngDecls.h,v 1.1.2.1 2008/01/12 23:42:29 wiede Exp $
 *
 * This file is (mostly) automatically generated from itcl.decls.
 */


#if defined(USE_ITCLNG_STUBS)

extern const char *Itclng_InitStubs(
        Tcl_Interp *, const char *version, int exact);
#define Itclng_InitStubs(interp, version, exact) Itclng_InitStubs( \
        interp, ITCLNG_VERSION, 1)
#else

#define Itclng_InitStubs(interp, version, exact) Tcl_PkgRequire(interp,"Itclng",version,exact)

#endif



/* !BEGIN!: Do not edit below this line. */

#define ITCLNG_STUBS_EPOCH 0
#define ITCLNG_STUBS_REVISION 18

#if !defined(USE_ITCLNG_STUBS)

/*
 * Exported function declarations:
 */

/* 0 */
ITCLNGAPI int		Itclng_Init (Tcl_Interp * interp);
/* 1 */
ITCLNGAPI int		Itclng_SafeInit (Tcl_Interp * interp);
/* 2 */
ITCLNGAPI void		Itclng_InitStack (Itclng_Stack * stack);
/* 3 */
ITCLNGAPI void		Itclng_DeleteStack (Itclng_Stack * stack);
/* 4 */
ITCLNGAPI void		Itclng_PushStack (ClientData cdata, 
				Itclng_Stack * stack);
/* 5 */
ITCLNGAPI ClientData	Itclng_PopStack (Itclng_Stack * stack);
/* 6 */
ITCLNGAPI ClientData	Itclng_PeekStack (Itclng_Stack * stack);
/* 7 */
ITCLNGAPI ClientData	Itclng_GetStackValue (Itclng_Stack * stack, int pos);
/* 8 */
ITCLNGAPI void		Itclng_InitList (Itclng_List * listPtr);
/* 9 */
ITCLNGAPI void		Itclng_DeleteList (Itclng_List * listPtr);
/* 10 */
ITCLNGAPI Itclng_ListElem* Itclng_CreateListElem (Itclng_List * listPtr);
/* 11 */
ITCLNGAPI Itclng_ListElem* Itclng_DeleteListElem (Itclng_ListElem * elemPtr);
/* 12 */
ITCLNGAPI Itclng_ListElem* Itclng_InsertList (Itclng_List * listPtr, 
				ClientData val);
/* 13 */
ITCLNGAPI Itclng_ListElem* Itclng_InsertListElem (Itclng_ListElem * pos, 
				ClientData val);
/* 14 */
ITCLNGAPI Itclng_ListElem* Itclng_AppendList (Itclng_List * listPtr, 
				ClientData val);
/* 15 */
ITCLNGAPI Itclng_ListElem* Itclng_AppendListElem (Itclng_ListElem * pos, 
				ClientData val);
/* 16 */
ITCLNGAPI void		Itclng_SetListValue (Itclng_ListElem * elemPtr, 
				ClientData val);

#endif /* !defined(USE_ITCLNG_STUBS) */

typedef struct ItclngStubHooks {
    struct ItclngIntStubs *itclngIntStubs;
} ItclngStubHooks;

typedef struct ItclngStubs {
    int magic;
    int epoch;
    int revision;
    struct ItclngStubHooks *hooks;

    int (*itclng_Init) (Tcl_Interp * interp); /* 0 */
    int (*itclng_SafeInit) (Tcl_Interp * interp); /* 1 */
    void (*itclng_InitStack) (Itclng_Stack * stack); /* 2 */
    void (*itclng_DeleteStack) (Itclng_Stack * stack); /* 3 */
    void (*itclng_PushStack) (ClientData cdata, Itclng_Stack * stack); /* 4 */
    ClientData (*itclng_PopStack) (Itclng_Stack * stack); /* 5 */
    ClientData (*itclng_PeekStack) (Itclng_Stack * stack); /* 6 */
    ClientData (*itclng_GetStackValue) (Itclng_Stack * stack, int pos); /* 7 */
    void (*itclng_InitList) (Itclng_List * listPtr); /* 8 */
    void (*itclng_DeleteList) (Itclng_List * listPtr); /* 9 */
    Itclng_ListElem* (*itclng_CreateListElem) (Itclng_List * listPtr); /* 10 */
    Itclng_ListElem* (*itclng_DeleteListElem) (Itclng_ListElem * elemPtr); /* 11 */
    Itclng_ListElem* (*itclng_InsertList) (Itclng_List * listPtr, ClientData val); /* 12 */
    Itclng_ListElem* (*itclng_InsertListElem) (Itclng_ListElem * pos, ClientData val); /* 13 */
    Itclng_ListElem* (*itclng_AppendList) (Itclng_List * listPtr, ClientData val); /* 14 */
    Itclng_ListElem* (*itclng_AppendListElem) (Itclng_ListElem * pos, ClientData val); /* 15 */
    void (*itclng_SetListValue) (Itclng_ListElem * elemPtr, ClientData val); /* 16 */
} ItclngStubs;

#ifdef __cplusplus
extern "C" {
#endif
extern const ItclngStubs *itclngStubsPtr;
#ifdef __cplusplus
}
#endif

#if defined(USE_ITCLNG_STUBS)

/*
 * Inline function declarations:
 */

#ifndef Itclng_Init
#define Itclng_Init \
	(itclngStubsPtr->itclng_Init) /* 0 */
#endif
#ifndef Itclng_SafeInit
#define Itclng_SafeInit \
	(itclngStubsPtr->itclng_SafeInit) /* 1 */
#endif
#ifndef Itclng_InitStack
#define Itclng_InitStack \
	(itclngStubsPtr->itclng_InitStack) /* 2 */
#endif
#ifndef Itclng_DeleteStack
#define Itclng_DeleteStack \
	(itclngStubsPtr->itclng_DeleteStack) /* 3 */
#endif
#ifndef Itclng_PushStack
#define Itclng_PushStack \
	(itclngStubsPtr->itclng_PushStack) /* 4 */
#endif
#ifndef Itclng_PopStack
#define Itclng_PopStack \
	(itclngStubsPtr->itclng_PopStack) /* 5 */
#endif
#ifndef Itclng_PeekStack
#define Itclng_PeekStack \
	(itclngStubsPtr->itclng_PeekStack) /* 6 */
#endif
#ifndef Itclng_GetStackValue
#define Itclng_GetStackValue \
	(itclngStubsPtr->itclng_GetStackValue) /* 7 */
#endif
#ifndef Itclng_InitList
#define Itclng_InitList \
	(itclngStubsPtr->itclng_InitList) /* 8 */
#endif
#ifndef Itclng_DeleteList
#define Itclng_DeleteList \
	(itclngStubsPtr->itclng_DeleteList) /* 9 */
#endif
#ifndef Itclng_CreateListElem
#define Itclng_CreateListElem \
	(itclngStubsPtr->itclng_CreateListElem) /* 10 */
#endif
#ifndef Itclng_DeleteListElem
#define Itclng_DeleteListElem \
	(itclngStubsPtr->itclng_DeleteListElem) /* 11 */
#endif
#ifndef Itclng_InsertList
#define Itclng_InsertList \
	(itclngStubsPtr->itclng_InsertList) /* 12 */
#endif
#ifndef Itclng_InsertListElem
#define Itclng_InsertListElem \
	(itclngStubsPtr->itclng_InsertListElem) /* 13 */
#endif
#ifndef Itclng_AppendList
#define Itclng_AppendList \
	(itclngStubsPtr->itclng_AppendList) /* 14 */
#endif
#ifndef Itclng_AppendListElem
#define Itclng_AppendListElem \
	(itclngStubsPtr->itclng_AppendListElem) /* 15 */
#endif
#ifndef Itclng_SetListValue
#define Itclng_SetListValue \
	(itclngStubsPtr->itclng_SetListValue) /* 16 */
#endif

#endif /* defined(USE_ITCLNG_STUBS) */

/* !END!: Do not edit above this line. */
