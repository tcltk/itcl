/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetObject.c,v 1.1.2.1 2007/09/15 11:58:35 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007 Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"


/*
 * ------------------------------------------------------------------------
 *  ItclInitObjectOptions()
 *
 *  Init all instance options 
 *  This is usually invoked automatically
 *  by Itcl_CreateObject(), when an object is created.
 * ------------------------------------------------------------------------
 */
int
ItclInitObjectOptions(
   Tcl_Interp *interp,
   ItclObject *ioPtr,
   ItclClass *iclsPtr,
   const char *name)
{
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  InstallHullAndOptions()
 * ------------------------------------------------------------------------
 */

int
InstallHullAndOptions(
    Tcl_Interp *interp,
    ItclObject *ioPtr,
    ItclClass *iclsPtr,
    int objc,
    Tcl_Obj * const objv[],
    int *newObjc,
    Tcl_Obj **newObjv)
{
    FOREACH_HASH_DECLS;
    Tcl_Obj *widgetClassPtr;
    Tcl_Obj **hullObjv;
    Tcl_DString buffer;
    Tcl_DString buffer2;
    Tcl_Var optionsVar;
    ItclOption *ioptPtr;
    char *token;
    int hullObjc;
    int foundWclass;
    int result;
    int i;

    ItclShowArgs(0, "InstallHullAndOptions", objc, objv);
    
    Tcl_DStringInit(&buffer),
    Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
    Tcl_DStringAppend(&buffer, "::", 2);
    Tcl_DStringAppend(&buffer, Tcl_GetCommandName(interp, ioPtr->accessCmd), -1);
    Tcl_DStringAppend(&buffer, Tcl_GetString(iclsPtr->fullname), -1);
    Tcl_DStringAppend(&buffer, "::options", -1);
    Tcl_DStringInit(&buffer2),
    optionsVar = Tcl_FindNamespaceVar(interp, Tcl_DStringValue(&buffer), NULL, 0);
    FOREACH_HASH_VALUE(ioptPtr, &iclsPtr->options) {
        Tcl_DStringAppend(&buffer2, Tcl_DStringValue(&buffer), -1);
	Tcl_DStringAppend(&buffer2, "::", 1);
        Tcl_DStringAppend(&buffer2, Tcl_GetString(ioptPtr->namePtr)+1, -1);
        
    }
    widgetClassPtr = NULL;
    foundWclass = 0;
    iclsPtr->infoPtr->buildingWidget = 1;
    iclsPtr->infoPtr->currIoPtr = ioPtr;
    for (i = 0; i < objc; i++) {
        token = Tcl_GetString(objv[i]);
	if ((*token == '-') && (strcmp(token, "-class") == 0)) {
	    /* check if it is in the -option position */
	    if (((i % 2) == 0) && (i + 1 <= objc)) {
		widgetClassPtr = objv[i+1];
		foundWclass = 1;
	        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc-1));
		*newObjc = objc - 2;
		memcpy(newObjv, objv, i * sizeof(Tcl_Obj *));
		if (objc-i-2 > 0) {
		    memcpy(newObjv+i, objv+i+2, (objc-i-2)*sizeof(Tcl_Obj *));
ItclShowArgs(0, "InstallHullAndOptions2", *newObjc, newObjv);
		}
	    }
	}
    }
    if (widgetClassPtr == NULL) {
	char buf[2];
	char *cp;
	cp = Tcl_GetString(iclsPtr->name);
        widgetClassPtr = Tcl_NewStringObj("", -1);
	buf[0] = toupper(*cp);
	buf[1] = '\0';
	Tcl_AppendToObj(widgetClassPtr, buf, -1);
	Tcl_AppendToObj(widgetClassPtr, cp+1, -1);
	Tcl_IncrRefCount(widgetClassPtr);
    }
    hullObjc = 5;
    hullObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*hullObjc);
    hullObjv[0] = Tcl_NewStringObj("installhull", -1);
    Tcl_IncrRefCount(hullObjv[0]);
    hullObjv[1] = Tcl_NewStringObj("using", -1);
    Tcl_IncrRefCount(hullObjv[1]);
    if (iclsPtr->flags & ITCL_WIDGET_IS_FRAME) 
    hullObjv[2] = Tcl_NewStringObj((iclsPtr->flags & ITCL_WIDGET_IS_FRAME) ?
            "frame" : "toplevel" , -1);
    Tcl_IncrRefCount(hullObjv[2]);
    hullObjv[3] = Tcl_NewStringObj("-class", -1);
    Tcl_IncrRefCount(hullObjv[3]);
    hullObjv[4] = Tcl_NewStringObj(Tcl_GetString(widgetClassPtr), -1);
    Tcl_IncrRefCount(hullObjv[4]);

    result = Itcl_BiInstallHullCmd(iclsPtr, interp, hullObjc, hullObjv);
    Tcl_DecrRefCount(hullObjv[0]);
    Tcl_DecrRefCount(hullObjv[1]);
    Tcl_DecrRefCount(hullObjv[2]);
    Tcl_DecrRefCount(hullObjv[3]);
    Tcl_DecrRefCount(hullObjv[4]);
    iclsPtr->infoPtr->buildingWidget = 0;
    return result;
}

