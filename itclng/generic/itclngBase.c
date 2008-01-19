/*
 * itclngBase.c --
 *
 * This file contains the C-implemented startup part of a
 * Itcl implemenatation
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclngBase.c,v 1.1.2.4 2008/01/19 17:29:13 wiede Exp $
 */

#include <stdlib.h>
#include "itclngInt.h"

extern struct ItclngStubAPI itclngStubAPI;

Tcl_ObjCmdProc Itclng_CreateClassFinishCmd;
const char *TclOOInitializeStubs(Tcl_Interp *interp, const char *version,
        int epoch, int revision);

static int Initialize _ANSI_ARGS_((Tcl_Interp *interp));

/*
 * ------------------------------------------------------------------------
 *  Initialize()
 *
 *      that is the starting point when loading the library
 *      it initializes all internal stuff
 *
 * ------------------------------------------------------------------------
 */


static int
Initialize (
    Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    ItclngObjectInfo *infoPtr;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }
    const char * ret = TclOOInitializeStubs(interp, "0.1.2", 0, 0);
    if (ret == NULL) {
        return TCL_ERROR;
    }
    nsPtr = Tcl_CreateNamespace(interp, ITCLNG_NAMESPACE, NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("Itclng: cannot create namespace: \"%s\" \n",
	        ITCLNG_NAMESPACE);
    }
    /*
     *  Create the top-level data structure for tracking objects.
     *  Store this as "associated data" for easy access, but link
     *  it to the itcl namespace for ownership.
     */
    infoPtr = (ItclngObjectInfo*)ckalloc(sizeof(ItclngObjectInfo));
    memset(infoPtr, 0, sizeof(ItclngObjectInfo));
    infoPtr->interp = interp;

    /* initialize the class meta data type for TclOO */
    infoPtr->class_meta_type = (Tcl_ObjectMetadataType *)ckalloc(
            sizeof(Tcl_ObjectMetadataType));
    infoPtr->class_meta_type->version = TCL_OO_METADATA_VERSION_CURRENT;
    infoPtr->class_meta_type->name = "ItclngClass";
    infoPtr->class_meta_type->deleteProc = ItclngDeleteClassMetadata;
    infoPtr->class_meta_type->cloneProc = NULL;

    /* initialize the object meta data type for TclOO */
    infoPtr->object_meta_type = (Tcl_ObjectMetadataType *)ckalloc(
            sizeof(Tcl_ObjectMetadataType));
    infoPtr->object_meta_type->version = TCL_OO_METADATA_VERSION_CURRENT;
    infoPtr->object_meta_type->name = "ItclngObject";
    infoPtr->object_meta_type->deleteProc = ItclngDeleteObjectMetadata;
    infoPtr->object_meta_type->cloneProc = NULL;

    Tcl_InitHashTable(&infoPtr->objects, TCL_ONE_WORD_KEYS);
    Tcl_InitObjHashTable(&infoPtr->classes);
    Tcl_InitHashTable(&infoPtr->namespaceClasses, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&infoPtr->procMethods, TCL_ONE_WORD_KEYS);
    infoPtr->protection = ITCLNG_PUBLIC;
    infoPtr->currIoPtr = NULL;
    infoPtr->currContextIclsPtr = NULL;
    infoPtr->currClassFlags = 0;
    Itclng_InitStack(&infoPtr->clsStack);
    Itclng_InitStack(&infoPtr->contextStack);
    Itclng_InitStack(&infoPtr->constructorStack);

    Tcl_SetAssocData(interp, ITCLNG_INTERP_DATA,
        (Tcl_InterpDeleteProc*)NULL, (ClientData)infoPtr);

    Tcl_Preserve((ClientData)infoPtr);

    if (Itclng_InitCommands(interp, infoPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    /*
     *  Set up the variables containing version info.
     */

    Tcl_SetVar(interp, "::itclng::version", ITCLNG_VERSION, TCL_NAMESPACE_ONLY);
    Tcl_SetVar(interp, "::itclng::patchLevel", ITCLNG_PATCH_LEVEL,
            TCL_NAMESPACE_ONLY);


    /*
     *  Package is now loaded.
     */

    return Tcl_PkgProvideEx(interp, "Itclng", ITCLNG_VERSION, &itclngStubAPI);
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_Init()
 *
 *  Invoked whenever a new INTERPRETER is created to install the
 *  [incr Tcl] package.  Usually invoked within Tcl_AppInit() at
 *  the start of execution.
 *
 *  Creates the "::itclng" namespace and installs access commands for
 *  creating classes and querying info.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */

int
Itclng_Init (
    Tcl_Interp *interp)
{
    if (Initialize(interp) != TCL_OK) {
        return TCL_ERROR;
    }

//    return  Tcl_Eval(interp, initScript);
return TCL_OK; 
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_SafeInit()
 *
 *  Invoked whenever a new SAFE INTERPRETER is created to install
 *  the [incr Tcl] package.
 *
 *  Creates the "::itclng" namespace and installs access commands for
 *  creating classes and querying info.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */

int
Itclng_SafeInit (
    Tcl_Interp *interp)
{
    if (Initialize(interp) != TCL_OK) {
        return TCL_ERROR;
    }
//    return Tcl_Eval(interp, safeInitScript);
return TCL_OK;
}
