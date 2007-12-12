/*
 * itclNeededFromTclOO.c --
 *
 *	This file contains code to create and manage methods.
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclNeededFromTclOO.c,v 1.1.2.8 2007/12/12 15:31:04 wiede Exp $
 */

#include <tclOOInt.h>
#include <dlfcn.h>

typedef void (*Tcl_ProcErrorProc)(Tcl_Interp *interp, Tcl_Obj *procNameObj);
typedef void (*AddToMixinSubs)(Class *subPtr, Class *superPtr);
typedef void (*RemoveFromMixinSubs)(Class *subPtr, Class *superPtr);
typedef Method *(*NewForwardMethod)(Tcl_Interp *interp, Object *oPtr,
	int flags, Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
typedef Method *(*NewForwardClassMethod)(Tcl_Interp *interp, Class *clsPtr,
	int flags, Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
typedef void(*SetMapMethodNameProc)(Tcl_Object oPtr,
        TclOO_MapMethodNameProc mapMethodNameProc);

static struct tcloo_fcn_ptrs {
    AddToMixinSubs addToMixinSubs;
    RemoveFromMixinSubs removeFromMixinSubs;
    NewForwardMethod newForwardMethod;
    NewForwardClassMethod newForwardClassMethod;
    SetMapMethodNameProc setMapMethodNameProc;
} tcloo_fcn_ptrs = { NULL, NULL, NULL, NULL, NULL };

/*
 * ----------------------------------------------------------------------
 *
 * InitTclOOFunctionPointers --
 *
 *	Create a new procedure-like method for an object for Itcl.
 *
 * ----------------------------------------------------------------------
 */

int
InitTclOOFunctionPointers(
    Tcl_Interp *interp)
{
    void *dlhandle;

    dlhandle = dlopen(NULL, RTLD_NOW);
#ifdef NOTDEF
    tcloo_fcn_ptrs.addToMixinSubs =
            dlsym(dlhandle, "TclOOAddToMixinSubs");
    if (tcloo_fcn_ptrs.addToMixinSubs == NULL) {
	Tcl_AppendResult(interp,
	    "cannot find symbol TclOOAddToMixinSubs for package TclOO", NULL);
        return TCL_ERROR;
    }
    tcloo_fcn_ptrs.removeFromMixinSubs =
            dlsym(dlhandle, "TclOORemoveFromMixinSubs");
    if (tcloo_fcn_ptrs.removeFromMixinSubs == NULL) {
	Tcl_AppendResult(interp,
	    "cannot find symbol TclOOAddRemoveFromSubs for package TclOO", NULL);
        return TCL_ERROR;
    }
#endif
#ifdef NOTDEF
    tcloo_fcn_ptrs.newForwardMethod =
            dlsym(dlhandle, "TclOONewForwardMethod");
    if (tcloo_fcn_ptrs.newForwardMethod == NULL) {
	Tcl_AppendResult(interp,
	    "cannot find symbol TclOONewForwardMethod for package TclOO", NULL);
        return TCL_ERROR;
    }
#endif
    tcloo_fcn_ptrs.newForwardClassMethod =
            dlsym(dlhandle, "TclOONewForwardClassMethod");
    if (tcloo_fcn_ptrs.newForwardClassMethod == NULL) {
	Tcl_AppendResult(interp,
	    "cannot find symbol TclOONewForwardClassMethod for package TclOO", NULL);
        return TCL_ERROR;
    }
    tcloo_fcn_ptrs.setMapMethodNameProc =
            dlsym(dlhandle, "Tcl_ObjectSetMapMethodNameProc");
    if (tcloo_fcn_ptrs.setMapMethodNameProc == NULL) {
	Tcl_AppendResult(interp,
	    "cannot find symbol Tcl_ObjectSetMapMethodNameProc for package TclOO", NULL);
        return TCL_ERROR;
    }
    dlclose(dlhandle);
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * _Tcl_NewProcMethod --
 *
 *	Create a new procedure-like method for an object for Itcl.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Method
_Tcl_NewProcMethod(
    Tcl_Interp *interp,		/* The interpreter containing the object. */
    Tcl_Object oPtr,		/* The object to modify. */
    TclOO_PreCallProc preCallPtr,
    TclOO_PostCallProc postCallPtr,
    Tcl_ProcErrorProc errProc,
    ClientData clientData,
    Tcl_Obj *nameObj,		/* The name of the method, which must not be
				 * NULL. */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which must not be NULL. */
    Tcl_Obj *bodyObj,		/* The body of the method, which must not be
				 * NULL. */
    int flags,                  /* Whether this is a public method. */
    ClientData *clientData2)
{
    ProcedureMethod *pmPtr;
    Tcl_Method method;

    method = (Tcl_Method)TclOONewProcMethod(interp, (Object *)oPtr, flags,
            nameObj, argsObj, bodyObj, &pmPtr);
//    method = (Tcl_Method)tcloo_fcn_ptrs.newProcMethod(interp, (Object *)oPtr, flags,
//            nameObj, argsObj, bodyObj, &pmPtr);
    pmPtr->flags = flags & USE_DECLARER_NS;
    pmPtr->preCallProc = preCallPtr;
    pmPtr->postCallProc = postCallPtr;
    pmPtr->errProc = errProc;
    pmPtr->clientData = clientData;
    if (clientData2 != NULL) {
        *clientData2 = pmPtr;
    }
    return method;
}

/*
 * ----------------------------------------------------------------------
 *
 * _Tcl_NewProcClassMethod --
 *
 *	Create a new procedure-like method for a class for Itcl.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Method
_Tcl_NewProcClassMethod(
    Tcl_Interp *interp,		/* The interpreter containing the class. */
    Tcl_Class clsPtr,		/* The class to modify. */
    TclOO_PreCallProc preCallPtr,
    TclOO_PostCallProc postCallPtr,
    Tcl_ProcErrorProc errProc,
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
    int flags,                  /* Whether this is a public method. */
    ClientData *clientData2)
{
    ProcedureMethod *pmPtr;
    Method *method;

    method = TclOONewProcClassMethod(interp, (Class *)clsPtr, flags,
            nameObj, argsObj, bodyObj, &pmPtr);
    pmPtr->flags = flags & USE_DECLARER_NS;
    pmPtr->preCallProc = preCallPtr;
    pmPtr->postCallProc = postCallPtr;
    pmPtr->errProc = errProc;
    pmPtr->clientData = clientData;
    if (clientData2 != NULL) {
        *clientData2 = pmPtr;
    }
    return (Tcl_Method)method;
}
Tcl_Method
_Tcl_NewForwardMethod(
    Tcl_Interp *interp,
    Tcl_Object oPtr,
    int flags,
    Tcl_Obj *nameObj,
    Tcl_Obj *prefixObj)
{
//    return (Tcl_Method)TclOONewForwardMethod(interp, (Object *)oPtr,
//            flags, nameObj, prefixObj);
    return (Tcl_Method)tcloo_fcn_ptrs.newForwardMethod(interp, (Object *)oPtr,
            flags, nameObj, prefixObj);
return NULL;
}

Tcl_Method
_Tcl_NewForwardClassMethod(
    Tcl_Interp *interp,
    Tcl_Class clsPtr,
    int flags,
    Tcl_Obj *nameObj,
    Tcl_Obj *prefixObj)
{
//    return (Tcl_Method)TclOONewForwardClassMethod(interp, (Class *)clsPtr,
//            flags, nameObj, prefixObj);
    return (Tcl_Method)tcloo_fcn_ptrs.newForwardClassMethod(interp, (Class *)clsPtr,
            flags, nameObj, prefixObj);
return NULL;
}

void
_Tcl_AddToMixinSubs(
    Tcl_Class subPtr,
    Tcl_Class superPtr)
{
//    TclOOAddToMixinSubs((Class *)subPtr, (Class *)superPtr);
    tcloo_fcn_ptrs.addToMixinSubs((Class *)subPtr, (Class *)superPtr);
}

void
_Tcl_RemoveFromMixinSubs(
    Tcl_Class subPtr,
    Tcl_Class superPtr)
{
//    TclOORemoveFromMixinSubs((Class *)subPtr, (Class *)superPtr);
    tcloo_fcn_ptrs.removeFromMixinSubs((Class *)subPtr, (Class *)superPtr);
}

void
_Tcl_ObjectSetMapMethodNameProc(
    Tcl_Object oPtr,
    TclOO_MapMethodNameProc mapMethodNameProc)
{
    tcloo_fcn_ptrs.setMapMethodNameProc(oPtr, mapMethodNameProc);
}
