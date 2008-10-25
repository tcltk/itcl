/*
 * itclInt.h --
 *
 * This file contains internal definitions for the C-implemented part of a
 * Itcl
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclWidgetInt.h,v 1.1.2.1 2008/10/25 19:43:04 wiede Exp $
 */

#include <string.h>
#include <ctype.h>
#include <tcl.h>
#include <tclOO.h>
#include "itclInt.h"

MODULE_SCOPE int HullAndOptionsInstall(Tcl_Interp *interp, ItclObject *ioPtr,
        ItclClass *iclsPtr, int objc, Tcl_Obj * const objv[],
	int *newObjc, Tcl_Obj **newObjv);
MODULE_SCOPE int ComponentInstall(Tcl_Interp *interp, ItclObject *ioPtr,
        ItclClass *iclsPtr, int objc, Tcl_Obj * const objv[]);
MODULE_SCOPE int ItclWidgetConfigure(ClientData clientData, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[]);
MODULE_SCOPE int ItclWidgetCget(ClientData clientData, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[]);
MODULE_SCOPE int Itcl_WidgetParseInit(Tcl_Interp *interp,
        ItclObjectInfo *infoPtr);
MODULE_SCOPE int Itcl_WidgetBiInit(Tcl_Interp *interp);
MODULE_SCOPE int ItclWidgetInfoInit(Tcl_Interp *interp);
MODULE_SCOPE int ItclWidgetInitObjectOptions(Tcl_Interp *interp,
        ItclObject *ioPtr, ItclClass *iclsPtr, const char *name);
