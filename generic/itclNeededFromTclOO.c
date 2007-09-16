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
 * RCS: @(#) $Id: itclNeededFromTclOO.c,v 1.1.2.6 2007/09/16 17:16:29 wiede Exp $
 */

#include <tclOOInt.h>

typedef void (*Tcl_ProcErrorProc)(Tcl_Interp *interp, Tcl_Obj *procNameObj);


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
    return (Tcl_Method)TclOONewForwardMethod(interp, (Object *)oPtr,
            flags, nameObj, prefixObj);
}

Tcl_Method
_Tcl_NewForwardClassMethod(
    Tcl_Interp *interp,
    Tcl_Class clsPtr,
    int flags,
    Tcl_Obj *nameObj,
    Tcl_Obj *prefixObj)
{
    return (Tcl_Method)TclOONewForwardClassMethod(interp, (Class *)clsPtr,
            flags, nameObj, prefixObj);
}

void
_Tcl_AddToMixinSubs(
    Tcl_Class subPtr,
    Tcl_Class superPtr)
{
    TclOOAddToMixinSubs((Class *)subPtr, (Class *)superPtr);
}

void
_Tcl_RemoveFromMixinSubs(
    Tcl_Class subPtr,
    Tcl_Class superPtr)
{
    TclOORemoveFromMixinSubs((Class *)subPtr, (Class *)superPtr);
}

