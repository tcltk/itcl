/*
 * itclBase.c --
 *
 * This file contains the C-implemeted part of a
 * Itcl implemenatation
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclBase.c,v 1.1.2.1 2007/09/07 21:19:41 wiede Exp $
 */

#include <stdlib.h>
#include "itclInt.h"

extern struct ItclStubAPI itclStubAPI;

static int Initialize _ANSI_ARGS_((Tcl_Interp *interp));

static char initScript[] = "\n\
namespace eval ::itcl {\n\
    proc _find_init {} {\n\
        global env tcl_library\n\
        variable arnulf\n\
        variable library\n\
        variable version\n\
        rename _find_init {}\n\
        if {[info exists library]} {\n\
            lappend dirs $library\n\
        } else {\n\
            if {[catch {uplevel #0 source -rsrc Itcl}] == 0} {\n\
                return\n\
            }\n\
            set dirs {}\n\
            if {[info exists env(ITCL_LIBRARY)]} {\n\
                lappend dirs $env(ITCL_LIBRARY)\n\
            }\n\
            lappend dirs [file join [file dirname $tcl_library] Itcl$version]\n\
            set bindir [file dirname [info nameofexecutable]]\n\
            lappend dirs [file join $bindir .. lib Itcl$version]\n\
            lappend dirs [file join $bindir .. library]\n\
            lappend dirs [file join $bindir .. .. library]\n\
            lappend dirs [file join $bindir .. .. Itcl library]\n\
            lappend dirs [file join $bindir .. .. .. Itcl library]\n\
            lappend dirs [file join $bindir .. .. itcloo library]\n\
            # On MacOSX, check the directories in the tcl_pkgPath\n\
            if {[string equal $::tcl_platform(platform) \"unix\"] && \
                    [string equal $::tcl_platform(os) \"Darwin\"]} {\n\
                foreach d $::tcl_pkgPath {\n\
                    lappend dirs [file join $d Itcl$version]\n\
                }\n\
            }\n\
        }\n\
        foreach i $dirs {\n\
            set library $i\n\
            set itclfile [file join $i itcl.tcl]\n\
            if {![catch {uplevel #0 [list source $itclfile]} msg]} {\n\
                return\n\
            }\n\
        }\n\
        set msg \"Can't find a usable itcl.tcl in the following directories:\n\"\n\
        append msg \"    $dirs\n\"\n\
        append msg \"This probably means that Itcl/Tcl weren't installed properly.\n\"\n\
        append msg \"If you know where the Itcl library directory was installed,\n\"\n\
        append msg \"you can set the environment variable ITCL_LIBRARY to point\n\"\n\
        append msg \"to the library directory.\n\"\n\
        error $msg\n\
    }\n\
    _find_init\n\
}";

/*
 * The following script is used to initialize Itcl in a safe interpreter.
 */

static char safeInitScript[] =
"proc ::itcl::local {class name args} {\n\
    set ptr [uplevel [list $class $name] $args]\n\
    uplevel [list set itcl-local-$ptr $ptr]\n\
    set cmd [uplevel namespace which -command $ptr]\n\
    uplevel [list trace variable itcl-local-$ptr u \"::itcl::delete object $cmd; list\"]\n\
    return $ptr\n\
}";



/*
 * ------------------------------------------------------------------------
 *  Itcl_DeleteObjectMetadata()
 *
 *  Delete the metadata data if any
 *-------------------------------------------------------------------------
 */
void
Itcl_DeleteObjectMetadata(
    ClientData clientData)
{
/* FIX ME !!! NEED CODE HERE !! */
}

/*
 * ------------------------------------------------------------------------
 *  Initialize()
 *
 *      that is the starting point when loading the library
 *      it initializes all internal stuff
 *
 * ------------------------------------------------------------------------
 */

int DLLEXPORT Tcloo_Init(Tcl_Interp *interp);

static int
Initialize (
    Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    Tcl_Namespace *itclNs;
    ItclObjectInfo *info;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }
    if (Tcloo_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    nsPtr = Tcl_CreateNamespace(interp, ITCL_NAMESPACE, NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("Itcl: cannot create namespace: \"%s\" \n", ITCL_NAMESPACE);
    }
    nsPtr = Tcl_CreateNamespace(interp, ITCL_NAMESPACE"::methodset",
            NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("Itcl: cannot create namespace: \"%s::methodset\" \n",
	        ITCL_NAMESPACE);
    }
    Tcl_CreateObjCommand(interp,
            ITCL_NAMESPACE"::methodset::callCCommand",
            ItclCallCCommand, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            ITCL_NAMESPACE"::methodset::objectUnknownCommand",
            ItclObjectUnknownCommand, NULL, NULL);

    /*
     *  Create the top-level data structure for tracking objects.
     *  Store this as "associated data" for easy access, but link
     *  it to the itcl namespace for ownership.
     */
    info = (ItclObjectInfo*)ckalloc(sizeof(ItclObjectInfo));
    info->interp = interp;
    info->class_meta_type = (Tcl_ObjectMetadataType *)ckalloc(
            sizeof(Tcl_ObjectMetadataType));
    info->class_meta_type->version = TCL_OO_METADATA_VERSION_CURRENT;
    info->class_meta_type->name = "ItclClass";
    info->class_meta_type->deleteProc = Itcl_DeleteObjectMetadata;
    info->class_meta_type->cloneProc = NULL;
    info->object_meta_type = (Tcl_ObjectMetadataType *)ckalloc(
            sizeof(Tcl_ObjectMetadataType));
    info->object_meta_type->version = TCL_OO_METADATA_VERSION_CURRENT;
    info->object_meta_type->name = "ItclObject";
    info->object_meta_type->deleteProc = Itcl_DeleteObjectMetadata;
    info->object_meta_type->cloneProc = NULL;
    Tcl_InitHashTable(&info->objects, TCL_ONE_WORD_KEYS);
    Tcl_InitObjHashTable(&info->classes);
    Tcl_InitHashTable(&info->namespaceClasses, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&info->procMethods, TCL_ONE_WORD_KEYS);
    info->ensembleInfo = (EnsembleInfo *)ckalloc(sizeof(EnsembleInfo));
    memset(info->ensembleInfo, 0, sizeof(EnsembleInfo));
    Tcl_InitHashTable(&info->ensembleInfo->ensembles, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&info->ensembleInfo->subEnsembles, TCL_ONE_WORD_KEYS);
    info->ensembleInfo->numEnsembles = 0;
    info->protection = ITCL_DEFAULT_PROTECT;
    info->currIoPtr = NULL;
char *res_option = getenv("ITCL_USE_OLD_RESOLVERS");
int opt;
if (res_option == NULL) {
opt = 1;
} else {
opt = atoi(res_option);
}
    info->useOldResolvers = opt;
    Itcl_InitStack(&info->clsStack);
    Itcl_InitStack(&info->contextStack);

    Tcl_SetAssocData(interp, ITCL_INTERP_DATA,
        (Tcl_InterpDeleteProc*)NULL, (ClientData)info);

    Itcl_PreserveData((ClientData)info);

    /*
     *  Initialize the ensemble package first, since we need this
     *  for other parts of [incr Tcl].
     */

    if (Itcl_EnsembleInit(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    Itcl_ParseInit(interp, info);

    /*
     *  Create "itcl::builtin" namespace for commands that
     *  are automatically built into class definitions.
     */
    if (Itcl_BiInit(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Export all commands in the "itcl" namespace so that they
     *  can be imported with something like "namespace import itcl::*"
     */
    itclNs = Tcl_FindNamespace(interp, "::itcl", (Tcl_Namespace*)NULL,
        TCL_LEAVE_ERR_MSG);

    /*
     *  This was changed from a glob export (itcl::*) to explicit
     *  command exports, so that the itcl::is command can *not* be
     *  exported. This is done for concern that the itcl::is command
     *  imported might be confusing ("is").
     */
    if (!itclNs ||
            (Tcl_Export(interp, itclNs, "body", /* reset */ 1) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "class", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "code", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "configbody", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "delete", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "delete_helper", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "ensemble", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "find", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "local", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "scope", 0) != TCL_OK)) {
        return TCL_ERROR;
    }

    /*
     *  Set up the variables containing version info.
     */

    Tcl_SetVar(interp, "::itcl::version", ITCL_VERSION, TCL_NAMESPACE_ONLY);
    Tcl_SetVar(interp, "::itcl::patchLevel", ITCL_PATCH_LEVEL,
            TCL_NAMESPACE_ONLY);


    /*
     *  Package is now loaded.
     */

/* next lines are not needed in production environment !! */
#define ITCL_DEBUG_C_INTERFACE 1
#ifdef ITCL_DEBUG_C_INTERFACE
    void RegisterDebugCFunctions(Tcl_Interp *interp);
    RegisterDebugCFunctions(interp);
#endif
    return Tcl_PkgProvideEx(interp, "Itcl", ITCL_VERSION, &itclStubAPI);
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_Init()
 *
 *  Invoked whenever a new INTERPRETER is created to install the
 *  [incr Tcl] package.  Usually invoked within Tcl_AppInit() at
 *  the start of execution.
 *
 *  Creates the "::itcl" namespace and installs access commands for
 *  creating classes and querying info.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */

int
Itcl_Init (
    Tcl_Interp *interp)
{
    if (Initialize(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return  Tcl_Eval(interp, initScript);
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_SafeInit()
 *
 *  Invoked whenever a new SAFE INTERPRETER is created to install
 *  the [incr Tcl] package.
 *
 *  Creates the "::itcl" namespace and installs access commands for
 *  creating classes and querying info.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */

int
Itcl_SafeInit (
    Tcl_Interp *interp)
{
    if (Initialize(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return Tcl_Eval(interp, safeInitScript);
}

/*
 * ------------------------------------------------------------------------
 *  ItclCallCCommand()
 *  syntax: is
 *  objv[0]    command name of myself (::itcl::methodset::callCCommand)
 * ------------------------------------------------------------------------
 */

int
ItclCallCCommand(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Tcl_CmdProc *argProc;
    Tcl_ObjCmdProc *objProc;
    ClientData cData;
    int result;

    ItclShowArgs(2, "ItclCallCCommand", objc, objv);
    if (!Itcl_FindC(interp, Tcl_GetString(objv[0])+1, &argProc,
            &objProc, &cData)) {
	Tcl_AppendResult(interp, "no such registered C command 1: \"",
	        Tcl_GetString(objv[1]), "\"", NULL);
        return TCL_ERROR;
    }
    if ((argProc == NULL) && (objProc == NULL)) {
	Tcl_AppendResult(interp, "no such registered C command 2: \"",
	        Tcl_GetString(objv[1]), "\"", NULL);
        return TCL_ERROR;
    }
    result = TCL_ERROR;
    if (argProc != NULL) {
	CONST84 char **argv;
	int i;

	argv = (CONST84 char**)ckalloc((unsigned)(objc*sizeof(char*)));
	for (i=1;i<objc;i++) {
	    argv[i-1] = Tcl_GetString(objv[i]);
	}
        result = (*argProc)(cData, interp, objc-1, argv);
        ckfree((char*)argv);
    }
    if (objProc != NULL) {
	Tcl_Namespace *callerNsPtr;
        callerNsPtr = Itcl_GetUplevelNamespace(interp, 1);
        ItclShowArgs(2, "CARGS", Itcl_GetCallFrameObjc(interp),
	        Itcl_GetCallFrameObjv(interp));
        ItclObjectInfo *infoPtr;
        infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
                ITCL_INTERP_DATA, NULL);

/* FIX ME have to use ItclCallContext here !!! */
//	Itcl_PushStack(callerNsPtr, &infoPtr->namespaceStack);
        result = (*objProc)(cData, interp, Itcl_GetCallFrameObjc(interp)-1,
	        Itcl_GetCallFrameObjv(interp)+1);
//	Itcl_PopStack(&infoPtr->namespaceStack);
    }
    return result;
}
