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
 * RCS: @(#) $Id: itclNeededFromTclOO.c,v 1.1.2.2 2007/09/07 21:49:05 wiede Exp $
 */

#include "../../oo/generic/tclOOInt.h"


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
    Tcl_Obj *nameObj,		/* The name of the method, which must not be
				 * NULL. */
    Tcl_Obj *argsObj,		/* The formal argument list for the method,
				 * which must not be NULL. */
    Tcl_Obj *bodyObj,		/* The body of the method, which must not be
				 * NULL. */
    int flags,                  /* Whether this is a public method. */
    ClientData *clientData)
{
    ProcedureMethod *pmPtr;
    Tcl_Method method;

    method = (Tcl_Method)TclOONewProcMethod(interp, (Object *)oPtr, flags,
            nameObj, argsObj, bodyObj, &pmPtr);
    pmPtr->flags = flags & USE_DECLARER_NS;
    pmPtr->clientData = clientData;
    if (clientData != NULL) {
        *clientData = pmPtr;
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
    pmPtr->clientData = clientData;
    if (clientData2 != NULL) {
        *clientData2 = pmPtr;
    }
    return (Tcl_Method)method;
}

/*
 * ----------------------------------------------------------------------
 *
 * _Tcl_ProcPtrFromPM --
 *
 *	Get The ProcPtr from a struct ProcedureMethod
 *
 * ----------------------------------------------------------------------
 */

ClientData
_Tcl_ProcPtrFromPM(
    ClientData *clientData)
{
    return ((ProcedureMethod *)clientData)->procPtr;
}
