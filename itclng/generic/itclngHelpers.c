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
 * RCS: @(#) $Id: itclngHelpers.c,v 1.1.2.7 2008/01/27 19:26:22 wiede Exp $
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
 *  ItclngDeleteClassDictInfo()
 *
 *  removes the class dict info variable
 * ------------------------------------------------------------------------
 */
int
ItclngDeleteClassDictInfo(
    ItclngClass *iclsPtr)
{
    Tcl_Obj *objPtr;
    int result;

    objPtr = Tcl_NewStringObj("::namespace delete ", -1);
    Tcl_AppendToObj(objPtr,
            Tcl_GetString(iclsPtr->infoPtr->internalClassInfos), -1);
    Tcl_AppendToObj(objPtr, Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_IncrRefCount(objPtr);
//fprintf(stderr, "DEL!%s!\n", Tcl_GetString(objPtr));
    result = Tcl_EvalObj(iclsPtr->interp, objPtr);
    Tcl_DecrRefCount(objPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngGetClassDictInfo()
 *
 *  Gets info about a function, variable, option of a class
 *  Returns a Tcl_Obj containg the string/dict part.
 * ------------------------------------------------------------------------
 */
Tcl_Obj*
ItclngGetClassDictInfo(
    ItclngClass *iclsPtr,
    const char *what,
    const char *elementName)
{
    Tcl_Obj *objPtr;
    Tcl_Obj *dictPtr;
    Tcl_Obj *keyPtr;
    Tcl_Obj *valuePtr;

    objPtr = Tcl_NewStringObj(
            Tcl_GetString(iclsPtr->infoPtr->internalClassInfos), -1);
    Tcl_AppendToObj(objPtr, Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_AppendToObj(objPtr, "::infos", -1);
    dictPtr = Tcl_ObjGetVar2(iclsPtr->interp, objPtr, NULL, 0);
    if (dictPtr == NULL) {
        return NULL;
    }
    Tcl_DecrRefCount(objPtr);
    keyPtr = Tcl_NewStringObj(what, -1);
    valuePtr = Tcl_NewObj();
    if (Tcl_DictObjGet(iclsPtr->interp, dictPtr, keyPtr, &valuePtr) != TCL_OK) {
fprintf(stderr, "cannot get dict for class");
	Tcl_DecrRefCount(keyPtr);
        return NULL;
    }
    dictPtr = valuePtr;
    Tcl_SetStringObj(keyPtr, elementName, -1);
    if (Tcl_DictObjGet(iclsPtr->interp, dictPtr, keyPtr, &valuePtr) != TCL_OK) {
fprintf(stderr, "cannot get element for class");
	Tcl_DecrRefCount(keyPtr);
        return NULL;
    }
    Tcl_DecrRefCount(keyPtr);
    if (valuePtr != NULL) {
        Tcl_IncrRefCount(valuePtr);
    }
    return valuePtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngGetDictValueInfo()
 *
 *  Gets info about a function, variable, option of a class
 *  Returns a Tcl_Obj containg the string/dict part.
 * ------------------------------------------------------------------------
 */
Tcl_Obj*
ItclngGetDictValueInfo(
    Tcl_Interp *interp,
    Tcl_Obj *dictPtr,
    const char *elementName)
{
    Tcl_Obj *keyPtr;
    Tcl_Obj *valuePtr;

    keyPtr = Tcl_NewStringObj(elementName, -1);
    valuePtr = Tcl_NewObj();
    if (Tcl_DictObjGet(interp, dictPtr, keyPtr, &valuePtr) != TCL_OK) {
fprintf(stderr, "cannot get element part");
	Tcl_DecrRefCount(keyPtr);
        return NULL;
    }
    Tcl_DecrRefCount(keyPtr);
    if (valuePtr != NULL) {
        Tcl_IncrRefCount(valuePtr);
    }
    return valuePtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngGetArgumentString()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Obj *
ItclngGetArgumentString(
    ItclngClass *iclsPtr,
    const char *functionName)
{
    Tcl_Obj *dictPtr;
    Tcl_Obj *valuePtr;

    dictPtr = ItclngGetClassDictInfo(iclsPtr, "functions", functionName);
    if (dictPtr == NULL) {
        return NULL;
    }
    valuePtr = ItclngGetDictValueInfo(iclsPtr->interp, dictPtr,
            "arguments");
    if (valuePtr == NULL) {
        return NULL;
    }
    dictPtr = valuePtr;
    valuePtr = ItclngGetDictValueInfo(iclsPtr->interp, dictPtr,
            "definition");
    return valuePtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngGetBodyString()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Obj *
ItclngGetBodyString(
    ItclngClass *iclsPtr,
    const char *functionName)
{
    Tcl_Obj *dictPtr;
    Tcl_Obj *valuePtr;

    dictPtr = ItclngGetClassDictInfo(iclsPtr, "functions", functionName);
    if (dictPtr == NULL) {
        return NULL;
    }
    valuePtr = ItclngGetDictValueInfo(iclsPtr->interp, dictPtr,
            "body");
    return valuePtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngGetFunctionStateString()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Obj *
ItclngGetFunctionStateString(
    ItclngClass *iclsPtr,
    const char *functionName)
{
    Tcl_Obj *dictPtr;
    Tcl_Obj *valuePtr;

    dictPtr = ItclngGetClassDictInfo(iclsPtr, "functions", functionName);
    if (dictPtr == NULL) {
        return NULL;
    }
    valuePtr = ItclngGetDictValueInfo(iclsPtr->interp, dictPtr,
            "state");
    return valuePtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngGetVariableStateString()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Obj *
ItclngGetVariableStateString(
    ItclngClass *iclsPtr,
    const char *variableName)
{
    Tcl_Obj *dictPtr;
    Tcl_Obj *valuePtr;

    dictPtr = ItclngGetClassDictInfo(iclsPtr, "variables", variableName);
    if (dictPtr == NULL) {
        return NULL;
    }
    valuePtr = ItclngGetDictValueInfo(iclsPtr->interp, dictPtr,
            "state");
    return valuePtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngGetUsageString()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Obj *
ItclngGetUsageString(
    ItclngClass *iclsPtr,
    const char *functionName)
{
    Tcl_Obj *dictPtr;
    Tcl_Obj *valuePtr;

    dictPtr = ItclngGetClassDictInfo(iclsPtr, "functions", functionName);
    if (dictPtr == NULL) {
        return NULL;
    }
    valuePtr = ItclngGetDictValueInfo(iclsPtr->interp, dictPtr,
            "arguments");
    if (valuePtr == NULL) {
        return NULL;
    }
    dictPtr = valuePtr;
    valuePtr = ItclngGetDictValueInfo(iclsPtr->interp, dictPtr,
            "usage");
    return valuePtr;
}

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
    ItclngVarTraceInfo *tracePtr;
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
