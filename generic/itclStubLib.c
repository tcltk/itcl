/* 
 * itclStubLib.c --
 *
 *	Stub object that will be statically linked into extensions that wish
 *	to access Itcl.
 *
 * Copyright (c) 1998-1999 by XXXX
 * Copyright (c) 1998 Paul Duffin.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: 
 */

/*
 * We need to ensure that we use the stub macros so that this file contains
 * no references to any of the stub functions.  This will make it possible
 * to build an extension that references Tcl_InitStubs but doesn't end up
 * including the rest of the stub functions.
 */

#ifndef USE_TCL_STUBS
#define USE_TCL_STUBS
#endif
#undef USE_TCL_STUB_PROCS

#include "itclInt.h"

/*
 * Ensure that Itcl_InitStubs is built as an exported symbol.  The other stub
 * functions should be built as non-exported symbols.
 */

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

ItclStubs *itclStubsPtr;
ItclIntStubs *itclIntStubsPtr;


/*
 *----------------------------------------------------------------------
 *
 * Itcl_InitStubs --
 *
 *	Tries to initialise the stub table pointers and ensures that
 *	the correct version of Itcl is loaded.
 *
 * Results:
 *	The actual version of Itcl that satisfies the request, or
 *	NULL to indicate that an error occurred.
 *
 * Side effects:
 *	Sets the stub table pointers.
 *
 *----------------------------------------------------------------------
 */

char *
Itcl_InitStubs (interp, version, exact)
    Tcl_Interp *interp;
    char *version;
    int exact;
{
    char *actualVersion;
    
    actualVersion = Tcl_PkgRequireEx(interp, "Itcl", version, exact,
        (ClientData *) &itclStubsPtr);

    if (actualVersion == NULL) {
	itclStubsPtr = NULL;
	return NULL;
    }

    if (itclStubsPtr->hooks) {
	itclIntStubsPtr = itclStubsPtr->hooks->itclIntStubs;
    } else {
	itclIntStubsPtr = NULL;
    }
    
    return actualVersion;
}
