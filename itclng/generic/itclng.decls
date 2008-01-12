# -*- tcl -*-
# $Id: itclng.decls,v 1.1.2.1 2008/01/12 23:42:29 wiede Exp $

# public API
library itclng
interface itclng
hooks {itclngInt}
epoch 0
scspec ITCLNGAPI

# Declare each of the functions in the public Itclng interface.  Note that
# the an index should never be reused for a different function in order
# to preserve backwards compatibility.

declare 0 current {
    int Itclng_Init(Tcl_Interp *interp);
}
declare 1 current {
    int Itclng_SafeInit(Tcl_Interp *interp)
}
declare 2 current {
    void Itclng_InitStack(Itclng_Stack *stack)
}
declare 3 current {
    void Itclng_DeleteStack(Itclng_Stack *stack)
}
declare 4 current {
    void Itclng_PushStack(ClientData cdata, Itclng_Stack *stack)
}
declare 5 current {
    ClientData Itclng_PopStack(Itclng_Stack *stack)
}
declare 6 current {
    ClientData Itclng_PeekStack(Itclng_Stack *stack)
}
declare 7 current {
    ClientData Itclng_GetStackValue(Itclng_Stack *stack, int pos)
}
declare 8 current {
    void Itclng_InitList(Itclng_List *listPtr)
}
declare 9 current {
    void Itclng_DeleteList(Itclng_List *listPtr)
}
declare 10 current {
    Itclng_ListElem* Itclng_CreateListElem(Itclng_List *listPtr)
}
declare 11 current {
    Itclng_ListElem* Itclng_DeleteListElem(Itclng_ListElem *elemPtr)
}
declare 12 current {
    Itclng_ListElem* Itclng_InsertList(Itclng_List *listPtr, ClientData val)
}
declare 13 current {
    Itclng_ListElem* Itclng_InsertListElem (Itclng_ListElem *pos, ClientData val)
}
declare 14 current {
    Itclng_ListElem* Itclng_AppendList(Itclng_List *listPtr, ClientData val)
}
declare 15 current {
    Itclng_ListElem* Itclng_AppendListElem(Itclng_ListElem *pos, ClientData val)
}
declare 16 current {
    void Itclng_SetListValue(Itclng_ListElem *elemPtr, ClientData val)
}

# private API
interface itclngInt
#
# Functions used within the package, but not considered "public"
#

declare 0 current {
    char* Itclng_ProtectionStr (int pLevel)
}
