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
 * RCS: @(#) $Id: itclNeededFromTclOO.c,v 1.1.2.11 2008/01/06 19:24:32 wiede Exp $
 */

#include <tclOOInt.h>
#include <dlfcn.h>

//typedef void (*Tcl_ProcErrorProc)(Tcl_Interp *interp, Tcl_Obj *procNameObj);
typedef void (*AddToMixinSubs)(Class *subPtr, Class *superPtr);
typedef void (*RemoveFromMixinSubs)(Class *subPtr, Class *superPtr);

static struct tcloo_fcn_ptrs {
    AddToMixinSubs addToMixinSubs;
    RemoveFromMixinSubs removeFromMixinSubs;
} tcloo_fcn_ptrs = { NULL, NULL };

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
    dlclose(dlhandle);
    return TCL_OK;
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

