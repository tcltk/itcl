/*
 * itclngHelpers.c --
 *
 * This file contains the C-implemeted part of 
 * Itcl 
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclngHelpers.c,v 1.1.2.3 2008/01/13 11:56:24 wiede Exp $
 */

#include "itclngInt.h"

void ItclngDeleteArgList(ItclngArgList *arglistPtr);
#ifdef ITCLNG_DEBUG
int _itclng_debug_level = 0;

/*
 * ------------------------------------------------------------------------
 *  ItclngShowArgs()
 * ------------------------------------------------------------------------
 */

void
ItclngShowArgs(
    int level,
    const char *str,
    int objc,
    Tcl_Obj * const* objv)
{
    int i;

    if (level > _itclng_debug_level) {
        return;
    }
    fprintf(stderr, "%s", str);
    for (i = 0; i < objc; i++) {
        fprintf(stderr, "!%s", objv[i] == NULL ? "??" :
                Tcl_GetString(objv[i]));
    }
    fprintf(stderr, "!\n");
}
#endif

/*
 * ------------------------------------------------------------------------
 *  Itclng_ProtectionStr()
 *
 *  Converts an integer protection code (ITCL_PUBLIC, ITCL_PROTECTED,
 *  or ITCL_PRIVATE) into a human-readable character string.  Returns
 *  a pointer to this string.
 * ------------------------------------------------------------------------
 */
char*
Itclng_ProtectionStr(
    int pLevel)     /* protection level */
{
    switch (pLevel) {
    case ITCLNG_PUBLIC:
        return "public";
    case ITCLNG_PROTECTED:
        return "protected";
    case ITCLNG_PRIVATE:
        return "private";
    }
    return "<bad-protection-code>";
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_CreateArgs()
 *
 *  This procedure takes a string and a list of (objc,objv) arguments,
 *  and glues them together in a single list.  This is useful when
 *  a command word needs to be prepended or substituted into a command
 *  line before it is executed.  The arguments are returned in a single
 *  list object, and they can be retrieved by calling
 *  Tcl_ListObjGetElements.  When the arguments are no longer needed,
 *  they should be discarded by decrementing the reference count for
 *  the list object.
 *
 *  Returns a pointer to the list object containing the arguments.
 * ------------------------------------------------------------------------
 */
Tcl_Obj*
Itclng_CreateArgs(
    Tcl_Interp *interp,      /* current interpreter */
    CONST char *string,      /* first command word */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int i;
    Tcl_Obj *listPtr;

    listPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr,
            Tcl_NewStringObj("my", -1));
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr,
        Tcl_NewStringObj((CONST84 char *)string, -1));

    for (i=0; i < objc; i++) {
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objv[i]);
    }

    Tcl_IncrRefCount(listPtr);
    return listPtr;
}

#ifdef NOTDEF

/*
 * ------------------------------------------------------------------------
 *  ItclTraceUnsetVar()
 * ------------------------------------------------------------------------
 */

char *
ItclngTraceUnsetVar(
    ClientData clientData,
    Tcl_Interp *interp,
    const char *name1,
    const char *name2,
    int flags)
{
    IctlngVarTraceInfo *tracePtr;
    Tcl_HashEntry *hPtr;

    if (name2 != NULL) {
        /* unsetting of an array element nothing to do */
	return NULL;
    }
    tracePtr = (ItclngVarTraceInfo *)clientData;
    if (tracePtr->flags & ITCLNG_TRACE_CLASS) {
        hPtr = Tcl_FindHashEntry(&tracePtr->iclsPtr->classCommons,
	        (char *)tracePtr->ivPtr);
	if (hPtr != NULL) {
	    Tcl_DeleteHashEntry(hPtr);
	}
    }
    if (tracePtr->flags & ITCLNG_TRACE_OBJECT) {
        hPtr = Tcl_FindHashEntry(&tracePtr->ioPtr->objectVariables,
	        (char *)tracePtr->ivPtr);
	if (hPtr != NULL) {
	    Tcl_DeleteHashEntry(hPtr);
	}
    }
    return NULL;
}
#endif

/*
 * ------------------------------------------------------------------------
 *  ItclngCapitalize()
 * ------------------------------------------------------------------------
 */

Tcl_Obj *
ItclngCapitalize(
    const char *str)
{
    Tcl_Obj *objPtr;
    char buf[2];
    
    sprintf(buf, "%c", toupper(*str));
    buf[1] = '\0';
    objPtr = Tcl_NewStringObj(buf, -1);
    Tcl_AppendToObj(objPtr, str+1, -1);
    Tcl_IncrRefCount(objPtr);
    return objPtr;
}
