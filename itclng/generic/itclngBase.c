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
 * RCS: @(#) $Id: itclngBase.c,v 1.1.2.3 2008/01/14 21:25:53 wiede Exp $
 */

#include <stdlib.h>
#include "itclngInt.h"

extern struct ItclngStubAPI itclngStubAPI;

static int Initialize _ANSI_ARGS_((Tcl_Interp *interp));

static char *clazzClassScript = "set itclngClass [::oo::class create ::itclng::clazz]; \
    ::oo::define $itclngClass superclass ::oo::class";

#ifdef NOTDEF

static char *clazzUnknownBody = "\n\
    set mySelf [::oo::Helpers::self]\n\
    if {[::itcl::is class $mySelf]} {\n\
        set namespace [uplevel 1 namespace current]\n\
        set my_namespace $namespace\n\
        if {$my_namespace ne \"::\"} {\n\
            set my_namespace ${my_namespace}::\n\
        }\n\
        set my_class [::itcl::find classes ${my_namespace}$m]\n\
        if {[string length $my_class] > 0} {\n\
            # class already exists, it is a redefinition, so delete old class first\n\
	    ::itcl::delete class $my_class\n\
        }\n\
        set cmd [uplevel 1 ::info command ${my_namespace}$m]\n\
        if {[string length $cmd] > 0} {\n\
            error \"command \\\"$m\\\" already exists in namespace \\\"$namespace\\\"\"\n\
        }\n\
    } \n\
    set myns [uplevel namespace current]\n\
    if {$myns ne \"::\"} {\n\
       set myns ${myns}::\n\
    }\n\
    set myObj [lindex [::info level 0] 0]\n\
    set cmd [list uplevel 1 ::itcl::parser::handleClass $myObj $mySelf $m {*}[list $args]]\n\
    if {[catch {\n\
        eval $cmd\n\
    } obj errInfo]} {\n\
	return -code error -errorinfo $::errorInfo $obj\n\
    }\n\
    return $obj\n\
";

/*
 * ------------------------------------------------------------------------
 *  AddClassUnknowMethod()
 *
 * ------------------------------------------------------------------------
 */
static int
AddClassUnknowMethod(
    Tcl_Interp *interp,
    ItclngObjectInfo *infoPtr,
    Tcl_Class clsPtr)
{
    ClientData pmPtr;
    Tcl_Obj *bodyPtr = Tcl_NewStringObj(clazzUnknownBody, -1);
    Tcl_Obj *namePtr = Tcl_NewStringObj("unknown", -1);
    Tcl_IncrRefCount(namePtr);
    Tcl_Obj *argumentPtr = Tcl_NewStringObj("m args", -1);
    Tcl_IncrRefCount(argumentPtr);
    ClientData tmPtr = (ClientData)Itcl_NewProcClassMethod(interp,
        clsPtr, NULL, NULL, NULL, NULL, namePtr, argumentPtr, bodyPtr, &pmPtr);
    if (tmPtr == NULL) {
        Tcl_Panic("cannot add class method unknown");
    }
    return TCL_OK;
}
#endif
/*
 * ------------------------------------------------------------------------
 *  Initialize()
 *
 *      that is the starting point when loading the library
 *      it initializes all internal stuff
 *
 * ------------------------------------------------------------------------
 */

const char *TclOOInitializeStubs(Tcl_Interp *interp, const char *version,
        int epoch, int revision);

static int
Initialize (
    Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    ItclngObjectInfo *infoPtr;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }
    const char * ret = TclOOInitializeStubs(interp, "0.1.1", 0, 0);
    if (ret == NULL) {
        return TCL_ERROR;
    }
    nsPtr = Tcl_CreateNamespace(interp, ITCLNG_NAMESPACE, NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("Itclng: cannot create namespace: \"%s\" \n", ITCLNG_NAMESPACE);
    }
    /*
     *  Create the top-level data structure for tracking objects.
     *  Store this as "associated data" for easy access, but link
     *  it to the itcl namespace for ownership.
     */
    infoPtr = (ItclngObjectInfo*)ckalloc(sizeof(ItclngObjectInfo));
    memset(infoPtr, 0, sizeof(ItclngObjectInfo));
    infoPtr->interp = interp;
    infoPtr->class_meta_type = (Tcl_ObjectMetadataType *)ckalloc(
            sizeof(Tcl_ObjectMetadataType));
    infoPtr->class_meta_type->version = TCL_OO_METADATA_VERSION_CURRENT;
    infoPtr->class_meta_type->name = "ItclngClass";
    infoPtr->class_meta_type->deleteProc = ItclngDeleteClassMetadata;
    infoPtr->class_meta_type->cloneProc = NULL;
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

    /* first create the Itclng base class as root of itclng classes */
    if (Tcl_Eval(interp, clazzClassScript) != TCL_OK) {
        Tcl_Panic("cannot create Itclng root class ::itclng::clazz");
    }
    Tcl_Obj *objPtr = Tcl_NewStringObj("::itclng::clazz", -1);
    infoPtr->clazzObjectPtr = Tcl_GetObjectFromObj(interp, objPtr);
    if (infoPtr->clazzObjectPtr == NULL) {
        Tcl_AppendResult(interp,
                "ITCLNG: cannot get Object for ::itclng::clazz for class \"",
                "::itclng::clazz", "\"", NULL);
        return TCL_ERROR;
    }
    infoPtr->clazzClassPtr = Tcl_GetObjectAsClass(infoPtr->clazzObjectPtr);
#ifdef NOTDEF
    AddClassUnknowMethod(interp, infoPtr, infoPtr->clazzClassPtr);
#endif

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
