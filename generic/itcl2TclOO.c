/*
 * itcl2TclOO.c --
 *
 *	This file contains code to create and manage methods.
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itcl2TclOO.c,v 1.1.2.2 2007/09/07 21:49:05 wiede Exp $
 */

#include "../../oo/generic/tclOOInt.h"
#define _TCLOOINT_H
#include "itclInt.h"

#define LIMIT 60
#define ELLIPSIFY(str,len) \
	((len) > LIMIT ? LIMIT : (len)), (str), ((len) > LIMIT ? "..." : "")

static void
MethodErrorHandler(
    Tcl_Interp *interp,
    Tcl_Obj *methodNameObj)
{
    int nameLen, objectNameLen;
    CallContext *contextPtr = ((Interp *) interp)->varFramePtr->clientData;
    Method *mPtr = contextPtr->callChain[contextPtr->index].mPtr;
    const char *objectName, *kindName, *methodName =
	    Tcl_GetStringFromObj(mPtr->namePtr, &nameLen);
    Object *declarerPtr;

    if (mPtr->declaringObjectPtr != NULL) {
	declarerPtr = mPtr->declaringObjectPtr;
	kindName = "object";
    } else {
	if (mPtr->declaringClassPtr == NULL) {
	    Tcl_Panic("method not declared in class or object");
	}
	declarerPtr = mPtr->declaringClassPtr->thisPtr;
	kindName = "class";
    }

    kindName = "while constructing object ";
    objectName = Tcl_GetStringFromObj(TclOOObjectName(interp, declarerPtr),
	    &objectNameLen);
    Tcl_Obj *objPtr;
    objPtr = Tcl_NewObj();
    Tcl_GetCommandFullName(interp, contextPtr->oPtr->command, objPtr);
    Tcl_IncrRefCount(objPtr);
    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
	    "\n    %s\"%s\" in \"%.*s%s::%.*s%s\" (body line %d)",
	    kindName, Tcl_GetString(objPtr),
	    ELLIPSIFY(objectName, objectNameLen),
	    ELLIPSIFY(methodName, nameLen), interp->errorLine));
    Tcl_DecrRefCount(objPtr);
}

static void
ConstructorErrorHandler(
    Tcl_Interp *interp,
    Tcl_Obj *methodNameObj)
{
    CallContext *contextPtr = ((Interp *) interp)->varFramePtr->clientData;
    Method *mPtr = contextPtr->callChain[contextPtr->index].mPtr;
    Object *declarerPtr;
    const char *objectName, *kindName;
    int objectNameLen;

    if (interp->errorLine == 0xDEADBEEF) {
	/*
	 * Horrible hack to deal with certain constructors that must not add
	 * information to the error trace.
	 */

	return;
    }

    if (mPtr->declaringObjectPtr != NULL) {
	declarerPtr = mPtr->declaringObjectPtr;
	kindName = "object";
    } else {
	if (mPtr->declaringClassPtr == NULL) {
	    Tcl_Panic("method not declared in class or object");
	}
	declarerPtr = mPtr->declaringClassPtr->thisPtr;
	kindName = "class";
    }

    kindName = "while constructing object ";
    objectName = Tcl_GetStringFromObj(TclOOObjectName(interp, declarerPtr),
	    &objectNameLen);
    Tcl_Obj *objPtr;
    objPtr = Tcl_NewObj();
    Tcl_GetCommandFullName(interp, contextPtr->oPtr->command, objPtr);
    Tcl_IncrRefCount(objPtr);
    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
	    "\n    %s\"%s\" in %.*s%s::constructor (body line %d)",
	    kindName, Tcl_GetString(objPtr),
	    ELLIPSIFY(objectName, objectNameLen), interp->errorLine));
    Tcl_DecrRefCount(objPtr);
}

static void
DestructorErrorHandler(
    Tcl_Interp *interp,
    Tcl_Obj *methodNameObj)
{
    CallContext *contextPtr = ((Interp *) interp)->varFramePtr->clientData;
    Method *mPtr = contextPtr->callChain[contextPtr->index].mPtr;
    Object *declarerPtr;
    const char *objectName, *kindName;
    int objectNameLen;

    if (mPtr->declaringObjectPtr != NULL) {
	declarerPtr = mPtr->declaringObjectPtr;
	kindName = "object";
    } else {
	if (mPtr->declaringClassPtr == NULL) {
	    Tcl_Panic("method not declared in class or object");
	}
	declarerPtr = mPtr->declaringClassPtr->thisPtr;
	kindName = "class";
    }

    objectName = Tcl_GetStringFromObj(TclOOObjectName(interp, declarerPtr),
	    &objectNameLen);
    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
	    "\n    (%s \"%.*s%s\" destructor line %d)", kindName,
	    ELLIPSIFY(objectName, objectNameLen), interp->errorLine));
}

static void
ItclMethodErrorHandler(
    Tcl_Interp *interp,
    Tcl_Obj *methodNameObj)
{
//fprintf(stderr, "ItclMethodErrorHandler called!%s\n", Tcl_GetString(methodNameObj));
//fprintf(stderr, "RES!%s!\n", Tcl_GetStringResult(interp));
}

Tcl_Obj *
ItclGfivProc(
    ClientData clientData)
{
    Tcl_Obj *objPtr;

    objPtr = Tcl_NewStringObj("ITCLGFVI", -1);
    Tcl_IncrRefCount(objPtr);
    return objPtr;
}

void
ItclSetErrProc(
    ClientData *clientData,
    ItclMemberFunc *mPtr)
{
    ProcedureMethod *pmPtr = *((ProcedureMethod **)clientData);

    if (mPtr->flags & ITCL_CONSTRUCTOR) {
        pmPtr->errProc = ConstructorErrorHandler;
    }
    if (mPtr->flags & ITCL_DESTRUCTOR) {
        pmPtr->errProc = DestructorErrorHandler;
    }
    if ((mPtr->flags & (ITCL_CONSTRUCTOR | ITCL_DESTRUCTOR)) == 0) {
        pmPtr->errProc = MethodErrorHandler;
    }
    pmPtr->gfivProc = ItclGfivProc;
}

int
Itcl_InvokeProcedureMethod(
    ClientData clientData,	/* Pointer to some per-method context. */
    Tcl_Interp *interp,
    TclOO_PreCallProc preCallProc,
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Arguments as actually seen. */
{
    Method *mPtr = clientData;
    Tcl_Namespace *nsPtr = mPtr->declaringClassPtr->thisPtr->namespacePtr;
    ProcedureMethod *pmPtr = mPtr->clientData;
    Proc *procPtr = pmPtr->procPtr;
    int flags = FRAME_IS_METHOD;
    CallFrame frame;
    CallFrame *framePtr = &frame;
    CallFrame **framePtrPtr1 = &framePtr;
    Tcl_CallFrame **framePtrPtr = (Tcl_CallFrame **)framePtrPtr1;
    Command cmd;
    int result;

    memset(&cmd, 0, sizeof(Command));
    cmd.nsPtr = (Namespace *) nsPtr;
    cmd.clientData = NULL;
    pmPtr->procPtr->cmdPtr = &cmd;

    result = TclProcCompileProc(interp, pmPtr->procPtr,
	    pmPtr->procPtr->bodyPtr, (Namespace *) nsPtr, "body of method",
	    Tcl_GetString(mPtr->namePtr));
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
     * Give the pre-call callback a chance to do some setup and, possibly,
     * veto the call.
     */

    if (preCallProc != NULL) {
	int isFinished;

	result = preCallProc(pmPtr->clientData, interp, NULL,
		(Tcl_CallFrame *) framePtr, &isFinished);
	if (isFinished || result != TCL_OK) {
	    Tcl_PopCallFrame(interp);
	    goto done;
	}
    }

    /*
     * Now invoke the body of the method. Note that we need to take special
     * action when doing unknown processing to ensure that the missing method
     * name is passed as an argument.
     */

    result = TclObjInterpProcCore(interp, mPtr->namePtr, 1,
            ItclMethodErrorHandler);

done:
//fprintf(stderr, "IIPM!%s!%s!\n", Tcl_GetString(mPtr->namePtr), Tcl_GetStringResult(interp));
    return result;
}


/*
 * ----------------------------------------------------------------------
 *
 * Itcl_PublicObjectCmd, Itcl_PrivateObjectCmd --
 *
 *	Main entry point for object invokations. The Public* and Private*
 *	wrapper functions are just thin wrappers round the main ObjectCmd
 *	function that does call chain creation, management and invokation.
 *
 * ----------------------------------------------------------------------
 */

int
Itcl_PublicObjectCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_Class clsPtr,
    int objc,
    Tcl_Obj *const *objv)
{
    Object *oPtr = (Object *)clientData;
    int result;

    result = TclOOObjectCmdCore(oPtr, interp, objc, objv, PUBLIC_METHOD,
	    &oPtr->publicContextCache, (Class *)clsPtr);
    return result;
}

int
Itcl_PrivateObjectCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_Class clsPtr,
    int objc,
    Tcl_Obj *const *objv)
{
    Object *oPtr = (Object *)clientData;
    int result;

    result = TclOOObjectCmdCore(oPtr, interp, objc, objv, PRIVATE_METHOD,
	    &oPtr->publicContextCache, (Class *)clsPtr);
    return result;
}

/*
 * ----------------------------------------------------------------------
 *
 * Itcl_NewProcClassMethod --
 *
 *	Create a new procedure-like method for a class for Itcl.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Method
Itcl_NewProcClassMethod(
    Tcl_Interp *interp,		/* The interpreter containing the class. */
    Tcl_Class clsPtr,		/* The class to modify. */
    TclOO_PreCallProc preCallPtr,
    TclOO_PostCallProc postCallPtr,
    ClientData clientData,
    Tcl_Obj *nameObj,		/* The name of the method, which may be NULL;
				 * if so, up to caller to manage storage
				 * (e.g., because it is a constructor or
				 * destructor). */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which may be NULL; if so, it is equivalent
				 * to an empty list. */
    Tcl_Obj *bodyObj,		/* The body of the method, which must not be
				 * NULL. */
    ClientData *clientData2)
{
    Tcl_Method result;

    result = Tcl_NewProcClassMethod(interp, clsPtr, preCallPtr, postCallPtr,
           clientData, nameObj, argsObj, bodyObj,
           PUBLIC_METHOD | USE_DECLARER_NS, clientData2);
    ItclSetErrProc(clientData2, clientData);
    return result;
}

/*
 * ----------------------------------------------------------------------
 *
 * Itcl_NewProcMethod --
 *
 *	Create a new procedure-like method for an object for Itcl.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Method
Itcl_NewProcMethod(
    Tcl_Interp *interp,		/* The interpreter containing the object. */
    Tcl_Object oPtr,		/* The object to modify. */
    Tcl_Obj *nameObj,		/* The name of the method, which must not be
				 * NULL. */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which must not be NULL. */
    Tcl_Obj *bodyObj,		/* The body of the method, which must not be
				 * NULL. */
    ClientData *clientData)
{
    return Tcl_NewProcMethod(interp, oPtr, nameObj, argsObj, bodyObj,
           PUBLIC_METHOD | USE_DECLARER_NS, clientData);
}


