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
 */

#include <string.h>
#include <ctype.h>
#include <tcl.h>
#include <tclOO.h>
#include <itclInt.h>

#define ITCL_WIDGETS_NAMESPACE "::itcl::internal::widgets"

typedef int (*HullAndOptionsInst)(Tcl_Interp *interp,
        struct ItclObject *ioPtr, struct ItclClass *iclsPtr, int objc,
        Tcl_Obj *const *objv, int *newObjc, Tcl_Obj **newObjv);
typedef int (*InitObjectOptions)(Tcl_Interp *interp,
        struct ItclObject *ioPtr, struct ItclClass *iclsPtr, const char *name);
typedef int (*DelegationInst)(Tcl_Interp *interp,
        struct ItclObject *ioPtr, struct ItclClass *iclsPtr);


typedef struct ItclWidgetInfo {
    InitObjectOptions initObjectOpts;
    HullAndOptionsInst hullAndOptsInst;
    DelegationInst delegationInst;
    Tcl_ObjCmdProc *widgetConfigure;
    Tcl_ObjCmdProc *widgetCget;
} ItclWidgetInfo;


MODULE_SCOPE int HullAndOptionsInstall(Tcl_Interp *interp, ItclObject *ioPtr,
        ItclClass *iclsPtr, int objc, Tcl_Obj * const objv[],
	int *newObjc, Tcl_Obj **newObjv);
MODULE_SCOPE int InstallComponent(Tcl_Interp *interp, ItclObject *ioPtr,
        ItclClass *iclsPtr, int objc, Tcl_Obj * const objv[]);
MODULE_SCOPE int Itcl_BiInstallHullCmd (ClientData clientData,
        Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
MODULE_SCOPE int ItclWidgetConfigure(ClientData clientData, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[]);
MODULE_SCOPE int ItclWidgetCget(ClientData clientData, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[]);
MODULE_SCOPE int Itcl_WidgetParseInit(Tcl_Interp *interp,
        ItclObjectInfo *infoPtr);
MODULE_SCOPE int Itcl_WidgetBiInit(Tcl_Interp *interp, ItclObjectInfo *infoPtr);
MODULE_SCOPE int ItclWidgetInfoInit(Tcl_Interp *interp,
        ItclObjectInfo *infoPtr);
MODULE_SCOPE int ItclWidgetInitObjectOptions(Tcl_Interp *interp,
        ItclObject *ioPtr, ItclClass *iclsPtr, const char *name);
