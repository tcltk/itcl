/*
 * $Id: itclngStubInit.c,v 1.1.2.1 2008/01/12 23:42:30 wiede Exp $
 *
 * This file is (mostly) automatically generated from itcl.decls.
 * It is compiled and linked in with the itcl package proper.
 */

//#include "itcl.h"
#include "itclngInt.h"

/* !BEGIN!: Do not edit below this line. */

ItclngIntStubs itclngIntStubs = {
    TCL_STUB_MAGIC,
    ITCLNGINT_STUBS_EPOCH,
    ITCLNGINT_STUBS_REVISION,
    0,
    Itclng_ProtectionStr, /* 0 */
};

static ItclngStubHooks itclngStubHooks = {
    &itclngIntStubs
};

ItclngStubs itclngStubs = {
    TCL_STUB_MAGIC,
    ITCLNG_STUBS_EPOCH,
    ITCLNG_STUBS_REVISION,
    &itclngStubHooks,
    Itclng_Init, /* 0 */
    Itclng_SafeInit, /* 1 */
    Itclng_InitStack, /* 2 */
    Itclng_DeleteStack, /* 3 */
    Itclng_PushStack, /* 4 */
    Itclng_PopStack, /* 5 */
    Itclng_PeekStack, /* 6 */
    Itclng_GetStackValue, /* 7 */
    Itclng_InitList, /* 8 */
    Itclng_DeleteList, /* 9 */
    Itclng_CreateListElem, /* 10 */
    Itclng_DeleteListElem, /* 11 */
    Itclng_InsertList, /* 12 */
    Itclng_InsertListElem, /* 13 */
    Itclng_AppendList, /* 14 */
    Itclng_AppendListElem, /* 15 */
    Itclng_SetListValue, /* 16 */
};

/* !END!: Do not edit above this line. */

struct ItclngStubAPI itclngStubAPI = {
    &itclngStubs,
    &itclngIntStubs
};
