/*
 * itclBase.c --
 *
 * This file contains the C-implemented startup part of an
 * Itcl implemenatation
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclBase.c,v 1.1.2.25 2008/11/23 20:49:30 wiede Exp $
 */

#include <stdlib.h>
#include "itclInt.h"
#include <tclOODecls.h>

Tcl_ObjCmdProc ItclFinishCmd;

#ifdef OBJ_REF_COUNT_DEBUG
Tcl_ObjCmdProc ItclDumpRefCountInfo;
#endif

#ifdef ITCL_PRESERVE_DEBUG
Tcl_ObjCmdProc ItclDumpPreserveInfo;
#endif

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
	    lappend dirs [file join . library]\n\
            lappend dirs [file join $bindir .. lib Itcl$version]\n\
            lappend dirs [file join $bindir .. library]\n\
            lappend dirs [file join $bindir .. .. library]\n\
            lappend dirs [file join $bindir .. .. Itcl library]\n\
            lappend dirs [file join $bindir .. .. .. Itcl library]\n\
            lappend dirs [file join $bindir .. .. itcl-ng itcl library]\n\
            # On MacOSX, check the directories in the tcl_pkgPath\n\
            if {[string equal $::tcl_platform(platform) \"unix\"] && \
                    [string equal $::tcl_platform(os) \"Darwin\"]} {\n\
                foreach d $::tcl_pkgPath {\n\
                    lappend dirs [file join $d Itcl$version]\n\
                }\n\
            }\n\
            # On *nix, check the directories in the tcl_pkgPath\n\
            if {[string equal $::tcl_platform(platform) \"unix\"]} {\n\
                foreach d $::tcl_pkgPath {\n\
                    lappend dirs $d\n\
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

static char *clazzClassScript = "set itclClass [::oo::class create ::itcl::clazz]; \
    ::oo::define $itclClass superclass ::oo::class";


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
    set ::errorInfo {}\n\
    set obj {}\n\
    if {[catch {\n\
        eval $cmd\n\
    } obj errInfo]} {\n\
	return -code error -errorinfo $::errorInfo $obj\n\
    }\n\
    return $obj\n\
";

#define ITCL_IS_ENSEMBLE 0x1

typedef struct ItclCmdsInfo {
    const char *name;
    int flags;
} ItclCmdsInfo;
static ItclCmdsInfo itclCmds [] = {
    { "::itcl::class", 0},
    { "::itcl::find", ITCL_IS_ENSEMBLE},
    { "::itcl::delete", ITCL_IS_ENSEMBLE},
    { "::itcl::is", ITCL_IS_ENSEMBLE},
    { "::itcl::filter", ITCL_IS_ENSEMBLE},
    { "::itcl::forward", ITCL_IS_ENSEMBLE},
    { "::itcl::mixin", ITCL_IS_ENSEMBLE},
    { "::itcl::type", 0},
    { "::itcl::widget", 0},
    { "::itcl::widgetadaptor", 0},
    { "::itcl::nwidget", 0},
    { "::itcl::addoption", 0},
    { "::itcl::addobjectoption", 0},
    { "::itcl::adddelegatedoption", 0},
    { "::itcl::adddelegatedmethod", 0},
    { "::itcl::addcomponent", 0},
    { "::itcl::setcomponent", 0},
    { "::itcl::extendedclass", 0},
    { "::itcl::parser::delegate", ITCL_IS_ENSEMBLE},
    { NULL, 0},
};

/*
 * ------------------------------------------------------------------------
 *  AddClassUnknowMethod()
 *
 * ------------------------------------------------------------------------
 */
static int
AddClassUnknowMethod(
    Tcl_Interp *interp,
    ItclObjectInfo *infoPtr,
    Tcl_Class clsPtr)
{
    ClientData pmPtr;
    Tcl_Obj *namePtr = Tcl_NewStringObj("unknown", -1);
    Tcl_IncrRefCount(namePtr);
    Tcl_Obj *argumentPtr = Tcl_NewStringObj("m args", -1);
    Tcl_IncrRefCount(argumentPtr);
    Tcl_Obj *bodyPtr = Tcl_NewStringObj(clazzUnknownBody, -1);
    ClientData tmPtr = (ClientData)Itcl_NewProcClassMethod(interp,
        clsPtr, NULL, NULL, NULL, NULL, namePtr, argumentPtr, bodyPtr, &pmPtr);
    if (tmPtr == NULL) {
        Tcl_Panic("cannot add class method unknown");
    }
    return TCL_OK;
}
void
FreeItclObjectInfo(
    ClientData clientData)
{
    ItclObjectInfo *infoPtr;

    infoPtr = (ItclObjectInfo *)clientData;
    /* need somehow to determine the interpreter and use a per interp
     * ItclObjectInfo structure !!
     * then can call ItclFinishCmd here
     */
/*
fprintf(stderr, "@@@@ FreeItclObjectInfo called\n");
*/
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

int ItclVarsAndCommandResolveInit(Tcl_Interp *interp);

static int
Initialize (
    Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    Tcl_Namespace *itclNs;
    ItclObjectInfo *infoPtr;
    const char * ret;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }

    ret = Tcl_OOInitStubs(interp);
    if (ret == NULL) {
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
            ITCL_NAMESPACE"::finish",
            ItclFinishCmd, NULL, NULL);
    /* for debugging only !!! */
#ifdef OBJ_REF_COUNT_DEBUG
    Tcl_CreateObjCommand(interp,
            ITCL_NAMESPACE"::dumprefcountinfo",
            ItclDumpRefCountInfo, NULL, NULL);
#endif

#ifdef ITCL_PRESERVE_DEBUG
    Tcl_CreateObjCommand(interp,
            ITCL_NAMESPACE"::dumppreserveinfo",
            ItclDumpPreserveInfo, NULL, NULL);
#endif
    /* END for debugging only !!! */

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
    infoPtr = (ItclObjectInfo*)ckalloc(sizeof(ItclObjectInfo));
    memset(infoPtr, 0, sizeof(ItclObjectInfo));
    infoPtr->interp = interp;
    infoPtr->class_meta_type = (Tcl_ObjectMetadataType *)ckalloc(
            sizeof(Tcl_ObjectMetadataType));
    infoPtr->class_meta_type->version = TCL_OO_METADATA_VERSION_CURRENT;
    infoPtr->class_meta_type->name = "ItclClass";
    infoPtr->class_meta_type->deleteProc = ItclDeleteClassMetadata;
    infoPtr->class_meta_type->cloneProc = NULL;
    infoPtr->object_meta_type = (Tcl_ObjectMetadataType *)ckalloc(
            sizeof(Tcl_ObjectMetadataType));
    infoPtr->object_meta_type->version = TCL_OO_METADATA_VERSION_CURRENT;
    infoPtr->object_meta_type->name = "ItclObject";
    infoPtr->object_meta_type->deleteProc = ItclDeleteObjectMetadata;
    infoPtr->object_meta_type->cloneProc = NULL;
    Tcl_InitHashTable(&infoPtr->objects, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&infoPtr->objectCmds, TCL_ONE_WORD_KEYS);
    Tcl_InitObjHashTable(&infoPtr->objectNames);
    Tcl_InitHashTable(&infoPtr->classes, TCL_ONE_WORD_KEYS);
    Tcl_InitObjHashTable(&infoPtr->nameClasses);
    Tcl_InitHashTable(&infoPtr->namespaceClasses, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&infoPtr->procMethods, TCL_ONE_WORD_KEYS);
    Tcl_InitObjHashTable(&infoPtr->instances);
    Tcl_InitHashTable(&infoPtr->objectInstances, TCL_ONE_WORD_KEYS);
    Tcl_InitObjHashTable(&infoPtr->myEnsembles);
    infoPtr->ensembleInfo = (EnsembleInfo *)ckalloc(sizeof(EnsembleInfo));
    memset(infoPtr->ensembleInfo, 0, sizeof(EnsembleInfo));
    Tcl_InitHashTable(&infoPtr->ensembleInfo->ensembles, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&infoPtr->ensembleInfo->subEnsembles, TCL_ONE_WORD_KEYS);
    infoPtr->ensembleInfo->numEnsembles = 0;
    infoPtr->protection = ITCL_DEFAULT_PROTECT;
    infoPtr->currIoPtr = NULL;
    infoPtr->windgetInfoPtr = NULL;
    infoPtr->currContextIclsPtr = NULL;
    infoPtr->currClassFlags = 0;
    infoPtr->buildingWidget = 0;
char *res_option = getenv("ITCL_USE_OLD_RESOLVERS");
int opt;
if (res_option == NULL) {
opt = 1;
} else {
opt = atoi(res_option);
}
    infoPtr->useOldResolvers = opt;
    Itcl_InitStack(&infoPtr->clsStack);
    Itcl_InitStack(&infoPtr->contextStack);
    Itcl_InitStack(&infoPtr->constructorStack);

    Tcl_SetAssocData(interp, ITCL_INTERP_DATA,
        (Tcl_InterpDeleteProc*)FreeItclObjectInfo, (ClientData)infoPtr);

    Itcl_PreserveData((ClientData)infoPtr);

    ItclVarsAndCommandResolveInit(interp);

    /* first create the Itcl base class as root of itcl classes */
    if (Tcl_Eval(interp, clazzClassScript) != TCL_OK) {
        Tcl_Panic("cannot create Itcl root class ::itcl::clazz");
    }
    Tcl_Obj *objPtr = Tcl_NewStringObj("::itcl::clazz", -1);
    infoPtr->clazzObjectPtr = Tcl_GetObjectFromObj(interp, objPtr);
    if (infoPtr->clazzObjectPtr == NULL) {
        Tcl_AppendResult(interp,
                "ITCL: cannot get Object for ::itcl::clazz for class \"",
                "::itcl::clazz", "\"", NULL);
        return TCL_ERROR;
    }
    infoPtr->clazzClassPtr = Tcl_GetObjectAsClass(infoPtr->clazzObjectPtr);
    AddClassUnknowMethod(interp, infoPtr, infoPtr->clazzClassPtr);

    /*
     *  Initialize the ensemble package first, since we need this
     *  for other parts of [incr Tcl].
     */

    if (Itcl_EnsembleInit(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    Itcl_ParseInit(interp, infoPtr);

    /*
     *  Create "itcl::builtin" namespace for commands that
     *  are automatically built into class definitions.
     */
    if (Itcl_BiInit(interp, infoPtr) != TCL_OK) {
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
            (Tcl_Export(interp, itclNs, "filter", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "find", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "forward", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "local", 0) != TCL_OK) ||
            (Tcl_Export(interp, itclNs, "mixin", 0) != TCL_OK) ||
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
	const char **argv;
	int i;

	argv = (const char**)ckalloc((unsigned)(objc*sizeof(char*)));
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

/* FIXME have to use ItclCallContext here !!! */
/*	Itcl_PushStack(callerNsPtr, &infoPtr->namespaceStack); */
        result = (*objProc)(cData, interp, Itcl_GetCallFrameObjc(interp)-1,
	        Itcl_GetCallFrameObjv(interp)+1);
/*	Itcl_PopStack(&infoPtr->namespaceStack); */
    }
    return result;
}

int
ItclFinishCmd(
    ClientData clientData,   /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    FOREACH_HASH_DECLS;
    Tcl_Namespace *nsPtr;
    Tcl_Obj *objPtr;
    ItclObjectInfo *infoPtr;
    ItclCmdsInfo *iciPtr;
    int i;
    int result;

    ItclShowArgs(0, "ItclFinishCmd", objc, objv);
    infoPtr = Tcl_GetAssocData(interp, ITCL_INTERP_DATA, NULL);
    for (i = 0; ;i++) {
        iciPtr = &itclCmds[i];
        if (iciPtr->name == NULL) {
	    break;
	}
        result = Itcl_RenameCommand(interp, iciPtr->name, "");
        iciPtr++;
    }
    FOREACH_HASH_VALUE(objPtr, &infoPtr->myEnsembles) {
	nsPtr = Tcl_FindNamespace(interp, Tcl_GetString(objPtr), NULL, 0);
	if (nsPtr != NULL) {
            Tcl_DeleteNamespace(nsPtr);
	}
    }
    nsPtr = Tcl_FindNamespace(interp, "::itcl::parser", NULL, 0);
    if (nsPtr != NULL) {
        Tcl_DeleteNamespace(nsPtr);
    }
    Itcl_ReleaseData((ClientData)infoPtr);
    return TCL_OK;
}

#ifdef OBJ_REF_COUNT_DEBUG
void Tcl_DbDumpRefCountInfo(const char *fileName);

int
ItclDumpRefCountInfo(
    ClientData clientData,   /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(1, "ItclDumpRefCountInfo", objc, objv);
    Tcl_DbDumpRefCountInfo(NULL);
    return TCL_OK;
}
#endif

#ifdef ITCL_PRESERVE_DEBUG
void Itcl_DbDumpPreserveInfo(const char *fileName);

int
ItclDumpPreserveInfo(
    ClientData clientData,   /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclDumpPreserveInfo", objc, objv);
    Itcl_DbDumpPreserveInfo(NULL);
    return TCL_OK;
}
#endif
