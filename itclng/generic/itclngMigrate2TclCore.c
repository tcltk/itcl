/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  This file contains procedures that belong in the Tcl/Tk core.
 *  Hopefully, they'll migrate there soon.
 *
 * ========================================================================
 *  AUTHOR:  Arnulf Wiedemann
 *
 *     RCS:  $Id: itclngMigrate2TclCore.c,v 1.1.2.2 2008/02/10 19:58:15 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include <tcl.h>
#include <tclInt.h>
#include "itclngMigrate2TclCore.h"

int
Itclng_SetCallFrameResolver(
    Tcl_Interp *interp,
    Tcl_Resolve *resolvePtr)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr != NULL) {
#ifdef ITCLNG_USE_MODIFIED_TCL_H
        framePtr->isProcCallFrame |= FRAME_HAS_RESOLVER;
	framePtr->resolvePtr = resolvePtr;
#endif
        return TCL_OK;
    }
    return TCL_ERROR;
}

Tcl_HashTable *
_Tcl_GetNamespaceCommandTable(
    Tcl_Namespace *nsPtr)
{
    return &((Namespace *)nsPtr)->cmdTable;
}

Tcl_HashTable *
_Tcl_GetNamespaceChildTable(
    Tcl_Namespace *nsPtr)
{
    return &((Namespace *)nsPtr)->childTable;
}

int
_Tcl_InitRewriteEnsemble(
    Tcl_Interp *interp,
    int numRemoved,
    int numInserted,
    int objc,
    Tcl_Obj *const *objv)
{
    Interp *iPtr = (Interp *) interp;

    int isRootEnsemble = (iPtr->ensembleRewrite.sourceObjs == NULL);

    if (isRootEnsemble) {
        iPtr->ensembleRewrite.sourceObjs = objv;
        iPtr->ensembleRewrite.numRemovedObjs = numRemoved;
        iPtr->ensembleRewrite.numInsertedObjs = numInserted;
    } else {
        int numIns = iPtr->ensembleRewrite.numInsertedObjs;
        if (numIns < numRemoved) {
            iPtr->ensembleRewrite.numRemovedObjs += numRemoved - numIns;
            iPtr->ensembleRewrite.numInsertedObjs += numInserted - 1;
        } else {
            iPtr->ensembleRewrite.numInsertedObjs += numInserted - numRemoved;
        }
    }
    return isRootEnsemble;
}

void
_Tcl_ResetRewriteEnsemble(
    Tcl_Interp *interp,
    int isRootEnsemble)
{
    Interp *iPtr = (Interp *) interp;

    if (isRootEnsemble) {
        iPtr->ensembleRewrite.sourceObjs = NULL;
        iPtr->ensembleRewrite.numRemovedObjs = 0;
        iPtr->ensembleRewrite.numInsertedObjs = 0;
    }
}

int
_Tcl_SetNamespaceResolver(
    Tcl_Namespace *nsPtr,
    Tcl_Resolve *resolvePtr)
{
    if (nsPtr == NULL) {
        return TCL_ERROR;
    }
#ifdef ITCLNG_USE_MODIFIED_TCL_H
    ((Namespace *)nsPtr)->resolvePtr = resolvePtr;
#endif
    return TCL_OK;
}

Tcl_Var
Tcl_NewNamespaceVar(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    const char *varName)
{
    Var *varPtr = NULL;
    int new;

    if ((nsPtr == NULL) || (varName == NULL)) {
        return NULL;
    }

    varPtr = TclVarHashCreateVar(&((Namespace *)nsPtr)->varTable,
            varName, &new);
    TclSetVarNamespaceVar(varPtr);
    VarHashRefCount(varPtr)++;
    return (Tcl_Var)varPtr;
}

Tcl_Namespace *
Itclng_GetUplevelNamespace(
    Tcl_Interp *interp,
    int level)
{
    if (level < 0) {
        return NULL;
    }
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    while ((framePtr != NULL) && (level-- > 0)) {
        framePtr = framePtr->callerVarPtr;
    }
    if (framePtr == NULL) {
        return NULL;
    }
    return (Tcl_Namespace *)framePtr->nsPtr;
}

ClientData
Itclng_GetCallFrameClientData(
    Tcl_Interp *interp,
    int level)
{
    if (level < 0) {
        return NULL;
    }
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    while ((framePtr != NULL) && (level-- > 0)) {
        framePtr = framePtr->callerVarPtr;
    }
    if (framePtr == NULL) {
        return NULL;
    }
    return framePtr->clientData;
}

int
Itclng_SetCallFrameNamespace(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr == NULL) {
        return TCL_ERROR;
    }
    ((Interp *)interp)->framePtr->nsPtr = (Namespace *)nsPtr;
    return TCL_OK;
}

int
Itclng_GetCallFrameObjc(
    Tcl_Interp *interp)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr == NULL) {
        return 0;
    }
    return ((Interp *)interp)->framePtr->objc;
}

Tcl_Obj * const *
Itclng_GetCallFrameObjv(
    Tcl_Interp *interp)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr == NULL) {
        return NULL;
    }
    return ((Interp *)interp)->framePtr->objv;
}

int
Itclng_IsCallFrameArgument(
    Tcl_Interp *interp,
    const char *name)
{
    CallFrame *varFramePtr = ((Interp *)interp)->framePtr;
    if (varFramePtr == NULL) {
        return 0;
    }
    if (!varFramePtr->isProcCallFrame) {
        return 0;
    }
    Proc *procPtr = varFramePtr->procPtr;
    /*
     *  Search through compiled locals first...
     */
    if (procPtr) {
        CompiledLocal *localPtr = procPtr->firstLocalPtr;
        int nameLen = strlen(name);

        for (;localPtr != NULL; localPtr = localPtr->nextPtr) {
            if (TclIsVarArgument(localPtr)) {
                register char *localName = localPtr->name;
                if ((name[0] == localName[0])
                        && (nameLen == localPtr->nameLength)
                        && (strcmp(name, localName) == 0)) {
                    return 1;
                }
            }
        }            
    }
    return 0;
}
