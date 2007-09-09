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
 *     RCS:  $Id: itclMigrate2TclCore.c,v 1.1.2.3 2007/09/09 11:04:16 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include <tcl.h>
#include <tclInt.h>
#include "itclMigrate2TclCore.h"

Tcl_Command
_Tcl_GetOriginalCommand(
    Tcl_Command command) 
{
    return TclGetOriginalCommand(command);
}

int
Itcl_SetCallFrameResolver(
    Tcl_Interp *interp,
    Tcl_Resolve *resolvePtr)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr != NULL) {
        framePtr->isProcCallFrame |= FRAME_HAS_RESOLVER;
	framePtr->resolvePtr = resolvePtr;
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
_Tcl_CreateProc(
    Tcl_Interp *interp,         /* Interpreter containing proc. */
    Tcl_Namespace *nsPtr,       /* Namespace containing this proc. */
    CONST char *procName,       /* Unqualified name of this proc. */
    Tcl_Obj *argsPtr,           /* Description of arguments. */
    Tcl_Obj *bodyPtr,           /* Command body. */
    Tcl_Proc *procPtrPtr)       /* Returns: pointer to proc data. */
{
    return TclCreateProc(interp, (Namespace *)nsPtr, procName, argsPtr,
            bodyPtr, (Proc **)procPtrPtr);
}

void *
_Tcl_GetObjInterpProc(
    void)
{
    return (void *)TclGetObjInterpProc();
}

int
_Tcl_SetNamespaceResolver(
    Tcl_Namespace *nsPtr,
    Tcl_Resolve *resolvePtr)
{
    if (nsPtr == NULL) {
        return TCL_ERROR;
    }
    ((Namespace *)nsPtr)->resolvePtr = resolvePtr;
    return TCL_OK;
}

void
_Tcl_ProcDeleteProc(
    ClientData clientData)
{
    TclProcDeleteProc(clientData);
}

static void
ItclMethodErrorHandler(
    Tcl_Interp *interp,
    Tcl_Obj *methodNameObj)
{
fprintf(stderr, "ItclMethodErrorHandler called!%s\n", Tcl_GetString(methodNameObj));
}

int
_Tcl_InvokeNamespaceProc(
    Tcl_Interp *interp,
    Tcl_Proc proc,
    Tcl_Namespace *nsPtr,
    Tcl_Obj *namePtr,
    int objc,
    Tcl_Obj *const *objv)
{
    CallFrame frame;
    CallFrame *framePtr = &frame;
    CallFrame **framePtrPtr1 = &framePtr;
    Tcl_CallFrame **framePtrPtr = (Tcl_CallFrame **)framePtrPtr1;
    Command cmd;
    Proc *procPtr = (Proc *)proc;
    int flags = 0;;
    int result;

    memset(&cmd, 0, sizeof(Command));
    cmd.nsPtr = (Namespace *) nsPtr;
    cmd.clientData = NULL;
    procPtr->cmdPtr = &cmd;

    result = TclProcCompileProc(interp, procPtr,
            procPtr->bodyPtr, (Namespace *) nsPtr, "body of method",
            Tcl_GetString(namePtr));
    if (result != TCL_OK) {
        return result;
    }

    /*
     * Make the stack frame and fill it out with information about this call.
     * This operation may fail.
     */


    flags |= FRAME_IS_PROC;
    result = TclPushStackFrame(interp, framePtrPtr, nsPtr, flags);
    if (result != TCL_OK) {
        return result;
    }

    framePtr->clientData = NULL;
    framePtr->objc = objc;
    framePtr->objv = objv;
    framePtr->procPtr = procPtr;

    /*
     * Now invoke the body of the method. Note that we need to take special
     * action when doing unknown processing to ensure that the missing method
     * name is passed as an argument.
     */

    result = TclObjInterpProcCore(interp, namePtr, 1, ItclMethodErrorHandler);

    return result;

}

int
Tcl_NewNamespaceVars(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    int argc,
    char *argv[])
{
    const char *tail;
    Var *varPtr = NULL;
    int i;
    int new;

    if (nsPtr == NULL) {
        return TCL_ERROR;
    }
    for (i=0;i<argc;i++) {
        tail = argv[i];
        if (tail == NULL) {
            continue;
        }
	varPtr = TclVarHashCreateVar(&((Namespace *)nsPtr)->varTable,
	    tail, &new);
        TclSetVarNamespaceVar(varPtr);
    }
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
Itcl_GetUplevelNamespace(
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
Itcl_GetCallFrameClientData(
    Tcl_Interp *interp)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr == NULL) {
        return NULL;
    }
    return framePtr->clientData;
}

Tcl_Proc
Itcl_GetCallFrameProc(
    Tcl_Interp *interp)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr == NULL) {
        return NULL;
    }
    return (Tcl_Proc)framePtr->procPtr;
}

int
Itcl_SetCallFrameNamespace(
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
Itcl_GetCallFrameObjc(
    Tcl_Interp *interp)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr == NULL) {
        return 0;
    }
    return ((Interp *)interp)->framePtr->objc;
}

Tcl_Obj * const *
Itcl_GetCallFrameObjv(
    Tcl_Interp *interp)
{
    CallFrame *framePtr = ((Interp *)interp)->framePtr;
    if (framePtr == NULL) {
        return NULL;
    }
    return ((Interp *)interp)->framePtr->objv;
}

int
Itcl_IsCallFrameArgument(
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

int
Itcl_ProcessReturn(
    Tcl_Interp *interp,
    int code,
    int level,
    Tcl_Obj *returnOpts)
{
    Interp *iPtr = (Interp *) interp;
    if (level != 0) {
        iPtr->returnLevel = level;
        iPtr->returnCode = code;
        return TCL_RETURN;
    }
    return TCL_ERROR;
}

int
ItclGetInterpErrorLine(
    Tcl_Interp *interp)
{
    Interp *iPtr = (Interp *) interp;
    return iPtr->errorLine;
}
