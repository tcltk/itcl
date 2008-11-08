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
 * RCS: @(#) $Id: itclNeededFromTclOO.c,v 1.1.2.14 2008/11/08 23:40:12 wiede Exp $
 */

#include <tclOOInt.h>

/* FIXME this has to be changed/adpted, because of changes in TclOO !!!! */

void
_Tcl_AddToMixinSubs(
    Tcl_Class subPtr,
    Tcl_Class superPtr)
{
/* use instead: TclOOClassSetMixins */
fprintf(stderr, "_Tcl_AddToMixinSubs not yet implemented\n");
}

void
_Tcl_RemoveFromMixinSubs(
    Tcl_Class subPtr,
    Tcl_Class superPtr)
{
/* use instead: TclOOClassSetMixins */
fprintf(stderr, "_Tcl_RemoveFromMixinSubs not yet implemented\n");
}

