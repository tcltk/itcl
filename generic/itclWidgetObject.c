/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 * Implementation of objects for package ItclWidget
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetObject.c,v 1.1.2.10 2007/10/02 22:43:30 wiede Exp $
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
 *  HullAndOptionsInstall()
 * ------------------------------------------------------------------------
 */

int
HullAndOptionsInstall(
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
    ItclOption *ioptPtr;
    char *token;
    const char *hullType;
    int hullObjc;
    int foundWclass;
    int result;
    int i;

    ItclShowArgs(1, "HullAndOptionsInstall", objc, objv);
    FOREACH_HASH_VALUE(ioptPtr, &iclsPtr->options) {
	if (ioptPtr->init != NULL) {
	    ItclSetInstanceVar(interp, "options",
	            Tcl_GetString(ioptPtr->namePtr),
		    Tcl_GetString(ioptPtr->init), ioPtr, iclsPtr);
	}
        
    }
    widgetClassPtr = iclsPtr->widgetClassPtr;
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
		}
	    }
	}
    }
    if (widgetClassPtr == NULL) {
	char buf[2];
	char *cp;
	cp = Tcl_GetString(iclsPtr->namePtr);
        widgetClassPtr = Tcl_NewStringObj("", -1);
	buf[0] = toupper(*cp);
	buf[1] = '\0';
	Tcl_AppendToObj(widgetClassPtr, buf, -1);
	Tcl_AppendToObj(widgetClassPtr, cp+1, -1);
	Tcl_IncrRefCount(widgetClassPtr);
    }
    hullObjc = 5;
    hullObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*hullObjc);
    hullObjv[0] = Tcl_NewStringObj("hullinstall", -1);
    Tcl_IncrRefCount(hullObjv[0]);
    hullObjv[1] = Tcl_NewStringObj("using", -1);
    Tcl_IncrRefCount(hullObjv[1]);
    if (iclsPtr->flags & ITCL_WIDGET_FRAME) {
        hullType = "frame";
    }
    if (iclsPtr->flags & ITCL_WIDGET_LABEL_FRAME) {
        hullType = "labelframe";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TOPLEVEL) {
        hullType = "toplevel";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TTK_FRAME) {
        hullType = "ttk::frame";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TTK_LABEL_FRAME) {
        hullType = "ttk::labelframe";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TTK_TOPLEVEL) {
        hullType = "ttk::toplevel";
    }
    hullObjv[2] = Tcl_NewStringObj(hullType, -1);
    Tcl_IncrRefCount(hullObjv[2]);
    hullObjv[3] = Tcl_NewStringObj("-class", -1);
    Tcl_IncrRefCount(hullObjv[3]);
    hullObjv[4] = Tcl_NewStringObj(Tcl_GetString(widgetClassPtr), -1);
    Tcl_IncrRefCount(hullObjv[4]);

    result = Itcl_BiHullInstallCmd(iclsPtr, interp, hullObjc, hullObjv);
    Tcl_DecrRefCount(hullObjv[0]);
    Tcl_DecrRefCount(hullObjv[1]);
    Tcl_DecrRefCount(hullObjv[2]);
    Tcl_DecrRefCount(hullObjv[3]);
    Tcl_DecrRefCount(hullObjv[4]);
    iclsPtr->infoPtr->buildingWidget = 0;
    return result;
}
