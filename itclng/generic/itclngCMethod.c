/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  overhauled version author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclngCMethod.c,v 1.1.2.1 2008/02/10 18:35:20 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "tclInt.h"
#include "tclOOInt.h"
#include "itclngCMethod.h"

/*
 * Structure used to help delay computing names of objects or classes for
 * [info frame] until needed, making invokation faster in the normal case.
 */

struct PNI {
    Tcl_Interp *interp;		/* Interpreter in which to compute the name of
				 * a method. */
    Tcl_Method method;		/* Method to compute the name of. */
};

/*
 * Structure used to contain all the information needed about a call frame
 * used in a procedure-like method.
 */

typedef struct {
    CallFrame *framePtr;	/* Reference to the call frame itself (it's
				 * actually allocated on the Tcl stack). */
    ProcErrorProc errProc;	/* The error handler for the body. */
    Tcl_Obj *nameObj;		/* The "name" of the command. */
    Command cmd;		/* The command structure. Mostly bogus. */
    ExtraFrameInfo efi;		/* Extra information used for [info frame]. */
    struct PNI pni;		/* Specialist information used in the efi
				 * field for this type of call. */
} PMFrameData;


static int InvokeCMethod(ClientData clientData,
        Tcl_Interp *interp, Tcl_ObjectContext context,
        int objc, Tcl_Obj *const *objv);

static void DeleteCMethod(ClientData clientData);
static int  CloneCMethod(Tcl_Interp *interp,
        ClientData clientData, ClientData *newClientData);
#ifdef NOTDEF
static int PushMethodCallFrame(Tcl_Interp *interp,
        CallContext *contextPtr, CMethod *pmPtr,
        int objc, Tcl_Obj *const *objv, PMFrameData *fdPtr);
static void MethodErrorHandler(Tcl_Interp *interp, Tcl_Obj *procNameObj);
static void ConstructorErrorHandler(Tcl_Interp *interp, Tcl_Obj *procNameObj);
static void DestructorErrorHandler(Tcl_Interp *interp, Tcl_Obj *procNameObj);
static Tcl_Obj *RenderDeclarerName(ClientData clientData);
#endif


static const Tcl_MethodType CMethodType = {
    TCL_OO_METHOD_VERSION_CURRENT, "C method",
    InvokeCMethod, DeleteCMethod, CloneCMethod
};



/*
 * ----------------------------------------------------------------------
 *
 * TclOONewCMethod --
 *
 *	Create a new C-inplemented method for an object.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Method
TclOONewCMethod(
    Tcl_Interp *interp,		/* The interpreter containing the object. */
    Tcl_Object oPtr,		/* The object to modify. */
    int flags,			/* Whether this is a public method. */
    Tcl_Obj *nameObj,		/* The name of the method, which must not be
				 * NULL. */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which must not be NULL. */
    Tcl_ObjCmdProc *cMethod,	/* The C-implemented method, which must not be
				 * NULL. */
    CMethod **cmPtrPtr)	        /* Place to write pointer to C-implemented
                                 * method structure to allow for deeper tuning
				 * of the structure's contents. NULL if caller
				 * is not interested. */
{
    int argsLen;
    register CMethod *cmPtr;
    Tcl_Method method;

    if (Tcl_ListObjLength(interp, argsObj, &argsLen) != TCL_OK) {
	return NULL;
    }
    cmPtr = (CMethod *) ckalloc(sizeof(CMethod));
    memset(cmPtr, 0, sizeof(CMethod));
    cmPtr->version = TCLOO_C_METHOD_VERSION;
    cmPtr->flags = flags & USE_DECLARER_NS;
    cmPtr->cMethodPtr = cMethod;
    method = Tcl_NewMethod(interp, oPtr, nameObj, flags,
            &CMethodType, cmPtr);
    if (method == NULL) {
	ckfree((char *) cmPtr);
    } else if (cmPtrPtr != NULL) {
	*cmPtrPtr = cmPtr;
    }
    return method;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOONewCClassMethod --
 *
 *	Create a new procedure-like method for a class.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Method
TclOONewCClassMethod(
    Tcl_Interp *interp,		/* The interpreter containing the class. */
    Tcl_Class clsPtr,		/* The class to modify. */
    int flags,			/* Whether this is a public method. */
    Tcl_Obj *nameObj,		/* The name of the method, which may be NULL;
				 * if so, up to caller to manage storage
				 * (e.g., because it is a constructor or
				 * destructor). */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which may be NULL; if so, it is equivalent
				 * to an empty list. */
    Tcl_ObjCmdProc *cMethod,	/* The C-implemented method, which must not be
				 * NULL. */
    CMethod **cmPtrPtr)	        /* Place to write pointer to C-implemented
                                 * method structure to allow for deeper tuning
				 * of the structure's contents. NULL if caller
				 * is not interested. */
{
    int argsLen;		/* -1 => delete argsObj before exit */
    register CMethod *cmPtr;
    const char *procName;
    Tcl_Method method;

    if (argsObj == NULL) {
	argsLen = -1;
	argsObj = Tcl_NewObj();
	Tcl_IncrRefCount(argsObj);
	procName = "<destructor>";
    } else if (Tcl_ListObjLength(interp, argsObj, &argsLen) != TCL_OK) {
	return NULL;
    } else {
	procName = (nameObj==NULL ? "<constructor>" : TclGetString(nameObj));
    }
    cmPtr = (CMethod *) ckalloc(sizeof(CMethod));
    memset(cmPtr, 0, sizeof(CMethod));
    cmPtr->version = TCLOO_C_METHOD_VERSION;
    cmPtr->flags = flags & USE_DECLARER_NS;
    cmPtr->cMethodPtr = cMethod;
    method = Tcl_NewClassMethod(interp, clsPtr, nameObj, flags,
            &CMethodType, cmPtr);
    if (argsLen == -1) {
	Tcl_DecrRefCount(argsObj);
    }
    if (method == NULL) {
	ckfree((char *) cmPtr);
    } else if (cmPtrPtr != NULL) {
	*cmPtrPtr = cmPtr;
    }
    return method;
}

/*
 * ----------------------------------------------------------------------
 *
 * InvokeCMethod --
 *
 *	How to invoke a C-implemented method.
 *
 * ----------------------------------------------------------------------
 */

static int
InvokeCMethod(
    ClientData clientData,	/* Pointer to some per-method context. */
    Tcl_Interp *interp,
    Tcl_ObjectContext context,	/* The method calling context. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Arguments as actually seen. */
{
    CMethod *cmPtr = clientData;
    int result;
    CallFrame frame;
    CallFrame *framePtr = &frame;
    Tcl_Namespace *nsPtr;
    register int skip;
    CallContext *contextPtr = (CallContext *)context;
    Object *oPtr;

fprintf(stderr, "ICM!%d", objc);
int j;
for (j=0;j<objc;j++) {
fprintf(stderr, "!%s", Tcl_GetString(objv[j]));
}
fprintf(stderr, "\n");

    result = TCL_OK;
    oPtr = contextPtr->oPtr;
    nsPtr = oPtr->namespacePtr;

#ifdef NOTDEF
    if (cmPtr->flags & USE_DECLARER_NS) {
	register Method *mPtr = contextPtr->callChain[contextPtr->index].mPtr;

	if (mPtr->declaringClassPtr != NULL) {
	    nsPtr = mPtr->declaringClassPtr->thisPtr->namespacePtr;
	} else {
	    nsPtr = mPtr->declaringObjectPtr->namespacePtr;
	}
    }
#endif

    /*
     * Give the pre-call callback a chance to do some setup and, possibly,
     * veto the call.
     */

    if (cmPtr->preCallProc != NULL) {
	int isFinished;

	result = cmPtr->preCallProc(cmPtr->clientData, interp, context,
		(Tcl_CallFrame *)framePtr, &isFinished);
	if (isFinished || result != TCL_OK) {
	    goto done;
	}
    }

    /*
     * Now invoke the body of the method. Note that we need to take special
     * action when doing unknown processing to ensure that the missing method
     * name is passed as an argument.
     */

    skip = Tcl_ObjectContextSkippedArgs(context);
    skip--;
    if (((CallContext *) context)->flags & OO_UNKNOWN_METHOD) {
	skip--;
    }
fprintf(stderr, "skip!%d!\n", skip);
    result = (cmPtr->cMethodPtr)(cmPtr->clientData, interp, objc-skip,
            objv+skip);

    /*
     * Give the post-call callback a chance to do some cleanup. Note that at
     * this point the call frame itself is invalid; it's already been popped.
     */

    if (cmPtr->postCallProc) {
	result = cmPtr->postCallProc(cmPtr->clientData, interp, context,
		Tcl_GetObjectNamespace(Tcl_ObjectContextObject(context)),
		result);
    }

    /*
     * Scrap the special frame data now that we're done with it.
     */

  done:
    return result;
}

#ifdef NOTDEF
static int
PushMethodCallFrame(
    Tcl_Interp *interp,		/* Current interpreter. */
    CallContext *contextPtr,	/* Current method call context. */
    CMethod *cmPtr,	        /* Information about this C-implemented
				 * method. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv,	/* Array of arguments. */
    PMFrameData *fdPtr)		/* Place to store information about the call
				 * frame. */
{
    Object *oPtr = contextPtr->oPtr;
    Tcl_Namespace *nsPtr = oPtr->namespacePtr;
    int flags = FRAME_IS_METHOD, result;
    const char *namePtr;
    CallFrame **framePtrPtr = &fdPtr->framePtr;

    /*
     * Compute basic information on the basis of the type of method it is.
     */

#ifdef NOTDEF
    if (contextPtr->flags & CONSTRUCTOR) {
	namePtr = "<constructor>";
	flags |= FRAME_IS_CONSTRUCTOR;
	fdPtr->nameObj = fPtr->constructorName;
	fdPtr->errProc = ConstructorErrorHandler;
    } else if (contextPtr->flags & DESTRUCTOR) {
	namePtr = "<destructor>";
	flags |= FRAME_IS_DESTRUCTOR;
	fdPtr->nameObj = fPtr->destructorName;
	fdPtr->errProc = DestructorErrorHandler;
    } else {
#endif
	fdPtr->nameObj = Tcl_MethodName(
		Tcl_ObjectContextMethod((Tcl_ObjectContext) contextPtr));
	namePtr = TclGetString(fdPtr->nameObj);
	fdPtr->errProc = MethodErrorHandler;
#ifdef NOTDEF
    }
#endif
    if (cmPtr->errProc != NULL) {
	fdPtr->errProc = cmPtr->errProc;
    }

    /*
     * Magic to enable things like [incr Tcl], which wants methods to run in
     * their class's namespace.
     */

    if (cmPtr->flags & USE_DECLARER_NS) {
	register Method *mPtr = contextPtr->callChain[contextPtr->index].mPtr;

	if (mPtr->declaringClassPtr != NULL) {
	    nsPtr = mPtr->declaringClassPtr->thisPtr->namespacePtr;
	} else {
	    nsPtr = mPtr->declaringObjectPtr->namespacePtr;
	}
    }

    /*
     * Compile the body. This operation may fail.
     */

    fdPtr->efi.length = 2;
    memset(&fdPtr->cmd, 0, sizeof(Command));
    fdPtr->cmd.nsPtr = (Namespace *) nsPtr;
    fdPtr->cmd.clientData = &fdPtr->efi;

    /*
     * Make the stack frame and fill it out with information about this call.
     * This operation may fail.
     */

    flags |= FRAME_IS_PROC;
    result = TclPushStackFrame(interp, (Tcl_CallFrame **) framePtrPtr, nsPtr,
	    flags);
    if (result != TCL_OK) {
	return result;
    }

    fdPtr->framePtr->clientData = contextPtr;
    fdPtr->framePtr->objc = objc;
    fdPtr->framePtr->objv = objv;

    /*
     * Finish filling out the extra frame info so that [info frame] works.
     */

    fdPtr->efi.fields[0].name = "method";
    fdPtr->efi.fields[0].proc = NULL;
    fdPtr->efi.fields[0].clientData = fdPtr->nameObj;
    if (cmPtr->gfivProc != NULL) {
	fdPtr->efi.fields[1].proc = cmPtr->gfivProc;
	fdPtr->efi.fields[1].clientData = cmPtr;
    } else {
	register Tcl_Method method =
		Tcl_ObjectContextMethod((Tcl_ObjectContext) contextPtr);

	if (Tcl_MethodDeclarerObject(method) != NULL) {
	    fdPtr->efi.fields[1].name = "object";
	} else {
	    fdPtr->efi.fields[1].name = "class";
	}
	fdPtr->efi.fields[1].proc = RenderDeclarerName;
	fdPtr->efi.fields[1].clientData = &fdPtr->pni;
	fdPtr->pni.interp = interp;
	fdPtr->pni.method = method;
    }

    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * RenderDeclarerName --
 *
 *	Returns the name of the entity (object or class) which declared a
 *	method. Used for producing information for [info frame] in such a way
 *	that the expensive part of this (generating the object or class name
 *	itself) isn't done until it is needed.
 *
 * ----------------------------------------------------------------------
 */

static Tcl_Obj *
RenderDeclarerName(
    ClientData clientData)
{
    struct PNI *pni = clientData;
    Tcl_Object object = Tcl_MethodDeclarerObject(pni->method);

    if (object == NULL) {
	object = Tcl_GetClassAsObject(Tcl_MethodDeclarerClass(pni->method));
    }
    return TclOOObjectName(pni->interp, (Object *) object);
}
#endif

/*
 * ----------------------------------------------------------------------
 *
 * TclOOObjectName --
 *
 *	Utility function that returns the name of the object. Note that this
 *	simplifies cache management by keeping the code to do it in one place
 *	and not sprayed all over. The value returned always has a reference
 *	count of at least one.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Obj *
TclOOObjectName(
    Tcl_Interp *interp,
    Object *oPtr)
{
    Tcl_Obj *namePtr;

    if (oPtr->cachedNameObj) {
	return oPtr->cachedNameObj;
    }
    namePtr = Tcl_NewObj();
    Tcl_GetCommandFullName(interp, oPtr->command, namePtr);
    Tcl_IncrRefCount(namePtr);
    oPtr->cachedNameObj = namePtr;
    return namePtr;
}

#ifdef NOTDEF

/*
 * ----------------------------------------------------------------------
 *
 * MethodErrorHandler, ConstructorErrorHandler, DestructorErrorHandler --
 *
 *	How to fill in the stack trace correctly upon error in various forms
 *	of procedure-like methods. LIMIT is how long the inserted strings in
 *	the error traces should get before being converted to have ellipses,
 *	and ELLIPSIFY is a macro to do the conversion (with the help of a
 *	%.*s%s format field). Note that ELLIPSIFY is only safe for use in
 *	suitable formatting contexts.
 *
 * ----------------------------------------------------------------------
 */

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

    objectName = Tcl_GetStringFromObj(TclOOObjectName(interp, declarerPtr),
	    &objectNameLen);
    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
	    "\n    (%s \"%.*s%s\" method \"%.*s%s\" line %d)",
	    kindName, ELLIPSIFY(objectName, objectNameLen),
	    ELLIPSIFY(methodName, nameLen), interp->errorLine));
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

    objectName = Tcl_GetStringFromObj(TclOOObjectName(interp, declarerPtr),
	    &objectNameLen);
    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
	    "\n    (%s \"%.*s%s\" constructor line %d)", kindName,
	    ELLIPSIFY(objectName, objectNameLen), interp->errorLine));
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
#endif

/*
 * ----------------------------------------------------------------------
 *
 * DeleteProcedureMethod, CloneProcedureMethod --
 *
 *	How to delete and clone procedure-like methods.
 *
 * ----------------------------------------------------------------------
 */

static void
DeleteCMethod(
    ClientData clientData)
{
    register CMethod *cmPtr = clientData;

    if (cmPtr->deleteClientdataProc) {
	cmPtr->deleteClientdataProc(cmPtr->clientData);
    }
    ckfree((char *) cmPtr);
}

static int
CloneCMethod(
    Tcl_Interp *interp,
    ClientData clientData,
    ClientData *newClientData)
{
    CMethod *cmPtr = clientData;
    CMethod *cm2Ptr = (CMethod *)ckalloc(sizeof(CMethod));

    memcpy(cm2Ptr, cmPtr, sizeof(CMethod));
    if (cmPtr->cloneClientdataProc) {
	cm2Ptr->clientData = cmPtr->cloneClientdataProc(cmPtr->clientData);
    }
    *newClientData = cm2Ptr;
    return TCL_OK;
}

/*
 * Extended method construction for itcl-ng.
 */

Tcl_Method
TclOONewCInstanceMethodEx(
    Tcl_Interp *interp,		/* The interpreter containing the object. */
    Tcl_Object oPtr,		/* The object to modify. */
    TclOO_PreCallProc preCallPtr,
    TclOO_PostCallProc postCallPtr,
    ProcErrorProc errProc,
    ClientData clientData,
    Tcl_Obj *nameObj,		/* The name of the method, which must not be
				 * NULL. */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which must not be NULL. */
    Tcl_ObjCmdProc *cMethod,	/* The C-implemented method, which must not be
				 * NULL. */
    int flags,			/* Whether this is a public method. */
    void **internalTokenPtr)	/* If non-NULL, points to a variable that gets
				 * the reference to the CMethod
				 * structure. */
{
    CMethod *cmPtr;
    Tcl_Method method = (Tcl_Method) TclOONewCMethod(interp,
	    oPtr, flags, nameObj, argsObj, cMethod, &cmPtr);

    if (method == NULL) {
	return NULL;
    }
    cmPtr->flags = flags & USE_DECLARER_NS;
    cmPtr->preCallProc = preCallPtr;
    cmPtr->postCallProc = postCallPtr;
    cmPtr->errProc = errProc;
    cmPtr->clientData = clientData;
    if (internalTokenPtr != NULL) {
	*internalTokenPtr = cmPtr;
    }
    return method;
}

Tcl_Method
TclOONewCClassMethodEx(
    Tcl_Interp *interp,		/* The interpreter containing the class. */
    Tcl_Class clsPtr,		/* The class to modify. */
    TclOO_PreCallProc preCallPtr,
    TclOO_PostCallProc postCallPtr,
    ProcErrorProc errProc,
    ClientData clientData,
    Tcl_Obj *nameObj,		/* The name of the method, which may be NULL;
				 * if so, up to caller to manage storage
				 * (e.g., because it is a constructor or
				 * destructor). */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which may be NULL; if so, it is equivalent
				 * to an empty list. */
    Tcl_ObjCmdProc *cMethod,	/* The C-implemented method, which must not be
				 * NULL. */
    int flags,			/* Whether this is a public method. */
    void **internalTokenPtr)	/* If non-NULL, points to a variable that gets
				 * the reference to the ProcedureMethod
				 * structure. */
{
    CMethod *cmPtr;
    Tcl_Method method = (Tcl_Method) TclOONewCClassMethod(interp,
	    clsPtr, flags, nameObj, argsObj, cMethod, &cmPtr);

    if (method == NULL) {
	return NULL;
    }
    cmPtr->flags = flags & USE_DECLARER_NS;
    cmPtr->preCallProc = preCallPtr;
    cmPtr->postCallProc = postCallPtr;
    cmPtr->errProc = errProc;
    cmPtr->clientData = clientData;
    if (internalTokenPtr != NULL) {
	*internalTokenPtr = cmPtr;
    }
    return method;
}
