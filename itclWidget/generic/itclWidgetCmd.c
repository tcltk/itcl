/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 * Implementation of commands for package ItclWidget
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 * ========================================================================
 *           Copyright (c) 2007 Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclWidgetInt.h"


/*
 * ------------------------------------------------------------------------
 *  Itcl_WidgetCmd()
 *
 *  Used to build an [incr Tcl] widget
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */

int
_Itcl_WidgetCmd(
    void *clientData,        /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *const objv[])   /* argument objects */
{
    Tcl_Obj *objPtr;
    ItclClass *iclsPtr;
    int result;

    ItclShowArgs(1, "Itcl_WidgetCmd", objc-1, objv);
    result = ItclClassBaseCmd(clientData, interp, ITCL_WIDGET, objc, objv,
	    &iclsPtr);
    if (result != TCL_OK) {
	return result;
    }

    /* we handle create by owerselfs !! allow classunknown to handle that */
    objPtr = Tcl_NewStringObj("oo::objdefine ", TCL_INDEX_NONE);
    Tcl_AppendToObj(objPtr, iclsPtr->nsPtr->fullName, TCL_INDEX_NONE);
    Tcl_AppendToObj(objPtr, " unexport create", TCL_INDEX_NONE);
    Tcl_IncrRefCount(objPtr);
    result = Tcl_EvalObjEx(interp, objPtr, 0);
    Tcl_DecrRefCount(objPtr);
    objPtr = Tcl_GetObjResult(interp);
    Tcl_AppendToObj(objPtr, iclsPtr->nsPtr->fullName, TCL_INDEX_NONE);
    Tcl_SetObjResult(interp, objPtr);
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_WidgetAdaptorCmd()
 *
 *  Used to an [incr Tcl] widgetadaptor
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */

int
_Itcl_WidgetAdaptorCmd(
    void *clientData,        /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *const objv[])   /* argument objects */
{
    Tcl_Obj *namePtr;
    Tcl_Obj *objPtr;
    ItclClass *iclsPtr;
    ItclComponent *icPtr;
    int result;

    ItclShowArgs(1, "Itcl_WidgetAdaptorCmd", objc-1, objv);
    result = ItclClassBaseCmd(clientData, interp, ITCL_WIDGETADAPTOR,
	    objc, objv, &iclsPtr);
    if (result != TCL_OK) {
	return result;
    }
    /* create the itcl_hull variable */
    namePtr = Tcl_NewStringObj("itcl_hull", TCL_INDEX_NONE);
    if (ItclCreateComponent(interp, iclsPtr, namePtr, ITCL_COMMON, &icPtr) !=
	    TCL_OK) {
	return TCL_ERROR;
    }
    iclsPtr->numVariables++;
    Itcl_BuildVirtualTables(iclsPtr);

    /* we handle create by owerselfs !! allow classunknown to handle that */
    objPtr = Tcl_NewStringObj("oo::objdefine ", TCL_INDEX_NONE);
    Tcl_AppendToObj(objPtr, iclsPtr->nsPtr->fullName, TCL_INDEX_NONE);
    Tcl_AppendToObj(objPtr, " unexport create", TCL_INDEX_NONE);
    Tcl_IncrRefCount(objPtr);
    result = Tcl_EvalObjEx(interp, objPtr, 0);
    Tcl_DecrRefCount(objPtr);
    objPtr = Tcl_GetObjResult(interp);
    Tcl_AppendToObj(objPtr, iclsPtr->nsPtr->fullName, TCL_INDEX_NONE);
    Tcl_SetObjResult(interp, objPtr);
    return result;
}
