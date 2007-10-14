/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  [incr Tcl] provides object-oriented extensions to Tcl, much as
 *  C++ provides object-oriented extensions to C.  It provides a means
 *  of encapsulating related procedures together with their shared data
 *  in a local namespace that is hidden from the outside world.  It
 *  promotes code re-use through inheritance.  More than anything else,
 *  it encourages better organization of Tcl applications through the
 *  object-oriented paradigm, leading to code that is easier to
 *  understand and maintain.
 *
 *  Procedures in this file support the new syntax for [incr Tcl]
 *  class definitions:
 *
 *    itcl_class <className> {
 *        inherit <base-class>...
 *
 *        constructor {<arglist>} ?{<init>}? {<body>}
 *        destructor {<body>}
 *
 *        method <name> {<arglist>} {<body>}
 *        proc <name> {<arglist>} {<body>}
 *        variable <name> ?<init>? ?<config>?
 *        common <name> ?<init>?
 *
 *        public <thing> ?<args>...?
 *        protected <thing> ?<args>...?
 *        private <thing> ?<args>...?
 *    }
 *
 * ========================================================================
 *  AUTHOR:  Michael J. McLennan
 *           Bell Labs Innovations for Lucent Technologies
 *           mmclennan@lucent.com
 *           http://www.tcltk.com/itcl
 *
 *  overhauled version author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclParse.c,v 1.1.2.16 2007/10/14 17:19:08 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"

/*
 *  Info needed for public/protected/private commands:
 */
typedef struct ProtectionCmdInfo {
    int pLevel;               /* protection level */
    ItclObjectInfo *infoPtr;  /* info regarding all known objects */
} ProtectionCmdInfo;

/*
 *  FORWARD DECLARATIONS
 */
static Tcl_CmdDeleteProc ItclFreeParserCommandData;
static void ItclDelObjectInfo(char* cdata);

Tcl_ObjCmdProc Itcl_ClassCommonCmd;
Tcl_ObjCmdProc Itcl_ClassConstructorCmd;
Tcl_ObjCmdProc Itcl_ClassDestructorCmd;
Tcl_ObjCmdProc Itcl_HandleClass;
Tcl_ObjCmdProc Itcl_ClassInheritCmd;
Tcl_ObjCmdProc Itcl_ClassMethodCmd;
Tcl_ObjCmdProc Itcl_ClassProcCmd;
Tcl_ObjCmdProc Itcl_ClassVariableCmd;
Tcl_ObjCmdProc Itcl_ClassProtectionCmd;
Tcl_ObjCmdProc Itcl_ClassFilterCmd;
Tcl_ObjCmdProc Itcl_ClassMixinCmd;
Tcl_ObjCmdProc Itcl_TypeCmdStart;
Tcl_ObjCmdProc Itcl_WidgetCmdStart;
Tcl_ObjCmdProc Itcl_WidgetAdaptorCmdStart;
Tcl_ObjCmdProc Itcl_ClassOptionCmd;
Tcl_ObjCmdProc Itcl_NWidgetCmd;
Tcl_ObjCmdProc Itcl_EClassCmd;
Tcl_ObjCmdProc Itcl_AddOptionCmd;
Tcl_ObjCmdProc Itcl_AddDelegatedOptionCmd;
Tcl_ObjCmdProc Itcl_AddComponentCmd;
Tcl_ObjCmdProc Itcl_SetComponentCmd;
Tcl_ObjCmdProc Itcl_ClassComponentCmd;
Tcl_ObjCmdProc Itcl_ClassDelegateMethodCmd;
Tcl_ObjCmdProc Itcl_ClassDelegateOptionCmd;
Tcl_ObjCmdProc Itcl_ClassDelegateProcCmd;
Tcl_ObjCmdProc Itcl_ClassForwardCmd;

static const struct {
    const char *name;
    Tcl_ObjCmdProc *objProc;
} parseCmds[] = {
    {"common", Itcl_ClassCommonCmd},
    {"component", Itcl_ClassComponentCmd},
    {"constructor", Itcl_ClassConstructorCmd},
    {"destructor", Itcl_ClassDestructorCmd},
    {"filter", Itcl_ClassFilterCmd},
    {"forward", Itcl_ClassForwardCmd},
    {"mixin", Itcl_ClassMixinCmd},
    {"inherit", Itcl_ClassInheritCmd},
    {"method", Itcl_ClassMethodCmd},
    {"option", Itcl_ClassOptionCmd},
    {"proc", Itcl_ClassProcCmd},
    {"variable", Itcl_ClassVariableCmd},
    {"handleClass", Itcl_HandleClass},
    {NULL, NULL}
};

static const struct {
    const char *name;
    Tcl_ObjCmdProc *objProc;
    int protection;
} protectionCmds[] = {
    {"private", Itcl_ClassProtectionCmd, ITCL_PRIVATE},
    {"protected", Itcl_ClassProtectionCmd, ITCL_PROTECTED},
    {"public", Itcl_ClassProtectionCmd, ITCL_PUBLIC},
    {NULL, NULL, 0}
};

/*
 * ------------------------------------------------------------------------
 *  Itcl_ParseInit()
 *
 *  Invoked by Itcl_Init() whenever a new interpeter is created to add
 *  [incr Tcl] facilities.  Adds the commands needed to parse class
 *  definitions.
 * ------------------------------------------------------------------------
 */
int
Itcl_ParseInit(
    Tcl_Interp *interp,     /* interpreter to be updated */
    ItclObjectInfo *infoPtr) /* info regarding all known objects and classes */
{
    Tcl_Namespace *parserNs;
    ProtectionCmdInfo *pInfoPtr;
    Tcl_DString buffer;
    int i;

    /*
     *  Create the "itcl::parser" namespace used to parse class
     *  definitions.
     */
    parserNs = Tcl_CreateNamespace(interp, "::itcl::parser",
        (ClientData)infoPtr, Itcl_ReleaseData);

    if (!parserNs) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            " (cannot initialize itcl parser)",
            (char*)NULL);
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    /*
     *  Add commands for parsing class definitions.
     */
    Tcl_DStringInit(&buffer);
    for (i=0 ; parseCmds[i].name ; i++) {
        Tcl_DStringAppend(&buffer, "::itcl::parser::", 16);
        Tcl_DStringAppend(&buffer, parseCmds[i].name, -1);
        Tcl_CreateObjCommand(interp, Tcl_DStringValue(&buffer),
                parseCmds[i].objProc, (ClientData) infoPtr, NULL);
        Tcl_DStringFree(&buffer);
    }

    for (i=0 ; protectionCmds[i].name ; i++) {
        Tcl_DStringAppend(&buffer, "::itcl::parser::", 16);
        Tcl_DStringAppend(&buffer, protectionCmds[i].name, -1);
        pInfoPtr = (ProtectionCmdInfo*)ckalloc(sizeof(ProtectionCmdInfo));
        pInfoPtr->pLevel = protectionCmds[i].protection;
        pInfoPtr->infoPtr = infoPtr;
        Tcl_CreateObjCommand(interp, Tcl_DStringValue(&buffer),
                protectionCmds[i].objProc, (ClientData) pInfoPtr,
		(Tcl_CmdDeleteProc*) ItclFreeParserCommandData);
        Tcl_DStringFree(&buffer);
    }

    /*
     *  Set the runtime variable resolver for the parser namespace,
     *  to control access to "common" data members while parsing
     *  the class definition.
     */
    if (infoPtr->useOldResolvers) {
        ItclSetParserResolver(parserNs);
    }
    /*
     *  Install the "class" command for defining new classes.
     */
    Tcl_CreateObjCommand(interp, "::itcl::class", Itcl_ClassCmd,
        (ClientData)infoPtr, Itcl_ReleaseData);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::body", Itcl_BodyCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

    Tcl_CreateObjCommand(interp, "::itcl::configbody", Itcl_ConfigBodyCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

    Itcl_EventuallyFree((ClientData)infoPtr, ItclDelObjectInfo);

    /*
     *  Create the "itcl::find" command for high-level queries.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::find") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::find",
            "classes", "?pattern?",
            Itcl_FindClassesCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::find",
            "objects", "?-class className? ?-isa className? ?pattern?",
            Itcl_FindObjectsCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);


    /*
     *  Create the "itcl::delete" command to delete objects
     *  and classes.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::delete") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::delete",
            "class", "name ?name...?",
            Itcl_DelClassCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::delete",
            "object", "name ?name...?",
            Itcl_DelObjectCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    /*
     *  Create the "itcl::is" command to test object
     *  and classes existence.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::is") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::is",
            "class", "name", Itcl_IsClassCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::is",
            "object", "?-class classname? name", Itcl_IsObjectCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);


    /*
     *  Add "code" and "scope" commands for handling scoped values.
     */
    Tcl_CreateObjCommand(interp, "::itcl::code", Itcl_CodeCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

    Tcl_CreateObjCommand(interp, "::itcl::scope", Itcl_ScopeCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

    /*
     *  Add the "filter" commands (add/delete)
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::filter") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::filter",
            "add", "objectOrClass filter ? ... ?", Itcl_FilterAddCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::filter",
            "delete", "objectOrClass filter ? ... ?", Itcl_FilterDeleteCmd,
            (ClientData)infoPtr, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    /*
     *  Add the "forward" commands (add/delete)
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::forward") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::forward",
            "add", "objectOrClass srcCommand targetCommand ? options ... ?",
	    Itcl_ForwardAddCmd, (ClientData)infoPtr,
	    Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::forward",
            "delete", "objectOrClass targetCommand ? ... ?",
	    Itcl_ForwardDeleteCmd, (ClientData)infoPtr,
	    Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    /*
     *  Add the "mixin" (add/delete) commands.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::mixin") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::mixin",
            "add", "objectOrClass class ? class ... ?",
	    Itcl_MixinAddCmd, (ClientData)infoPtr,
	    Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::mixin",
            "delete", "objectOrClass class ? class ... ?",
	    Itcl_MixinDeleteCmd, (ClientData)infoPtr,
	    Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    /*
     *  Add commands for handling import stubs at the Tcl level.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::import::stub") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::import::stub",
            "create", "name", Itcl_StubCreateCmd,
            (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL) != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::import::stub",
            "exists", "name", Itcl_StubExistsCmd,
            (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL) != TCL_OK) {
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "::itcl::type", Itcl_TypeCmdStart,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::widget", Itcl_WidgetCmdStart,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::widgetadaptor", Itcl_WidgetAdaptorCmdStart,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::nwidget", Itcl_NWidgetCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::addoption", Itcl_AddOptionCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::adddelegatedoption",
        Itcl_AddDelegatedOptionCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::addcomponent", Itcl_AddComponentCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::setcomponent", Itcl_SetComponentCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::eclass", Itcl_EClassCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    /*
     *  Add the "delegate" (method/option) commands.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::parser::delegate") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::parser::delegate",
            "method", "name to targetName as scipt using script",
	    Itcl_ClassDelegateMethodCmd, (ClientData)infoPtr,
	    Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::parser::delegate",
            "proc", "name to targetName as scipt using script",
	    Itcl_ClassDelegateProcCmd, (ClientData)infoPtr,
	    Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    if (Itcl_AddEnsemblePart(interp, "::itcl::parser::delegate",
            "option", "option to targetOption as script",
	    Itcl_ClassDelegateOptionCmd, (ClientData)infoPtr,
	    Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)infoPtr);

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassCmd()
 *
 *  Invoked by Tcl whenever the user issues an "itcl::class" command to
 *  specify a class definition.  Handles the following syntax:
 *
 *    itcl::class <className> {
 *        inherit <base-class>...
 *
 *        constructor {<arglist>} ?{<init>}? {<body>}
 *        destructor {<body>}
 *
 *        method <name> {<arglist>} {<body>}
 *        proc <name> {<arglist>} {<body>}
 *        variable <varname> ?<init>? ?<config>?
 *        common <varname> ?<init>?
 *
 *        public <args>...
 *        protected <args>...
 *        private <args>...
 *    }
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *iclsPtr;

    return ItclClassBaseCmd(clientData, interp, ITCL_CLASS, objc, objv,
            &iclsPtr);
}

/*
 * ------------------------------------------------------------------------
 *  ItclClassBaseCmd()
 *
 * ------------------------------------------------------------------------
 */
int
ItclClassBaseCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int flags,               /* flags: ITCL_CLASS, ITCL_TYPE,
                              * ITCL_WIDGET or ITCL_WIDGETADAPTOR */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[],   /* argument objects */
    ItclClass **iclsPtrPtr)  /* for returning iclsPtr */
{
    ItclObjectInfo* infoPtr = (ItclObjectInfo*)clientData;

    int result;
    char *className;
    Tcl_Namespace *parserNs;
    ItclClass *iclsPtr;
    Tcl_CallFrame frame;
    Tcl_DString buffer;
    Tcl_Obj *argumentPtr;
    Tcl_Obj *bodyPtr;
    FOREACH_HASH_DECLS;
    Tcl_HashEntry *hPtr2;
    int isNewEntry;

    if (iclsPtrPtr != NULL) {
        *iclsPtrPtr = NULL;
    }
    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "name { definition }");
        return TCL_ERROR;
    }
    ItclShowArgs(2, "ItclClassBaseCmd", objc, objv);
    className = Tcl_GetString(objv[1]);

    /*
     *  Find the namespace to use as a parser for the class definition.
     *  If for some reason it is destroyed, bail out here.
     */
    parserNs = Tcl_FindNamespace(interp, "::itcl::parser",
        (Tcl_Namespace*)NULL, TCL_LEAVE_ERR_MSG);

    if (parserNs == NULL) {
        char msg[256];
        sprintf(msg, "\n    (while parsing class definition for \"%.100s\")",
            className);
        Tcl_AddErrorInfo(interp, msg);
        return TCL_ERROR;
    }

    /*
     *  Try to create the specified class and its namespace.
     */
    /* need the workaround with infoPtr->currClassFlags to keep the stubs
     * call interface compatible!
     */
    infoPtr->currClassFlags = flags;
    if (Itcl_CreateClass(interp, className, infoPtr, &iclsPtr) != TCL_OK) {
        infoPtr->currClassFlags = 0;
        return TCL_ERROR;
    }
    infoPtr->currClassFlags = 0;
    iclsPtr->flags = flags;

    /*
     *  Import the built-in commands from the itcl::builtin namespace.
     *  Do this before parsing the class definition, so methods/procs
     *  can override the built-in commands.
     */
    result = Tcl_Import(interp, iclsPtr->nsPtr, "::itcl::builtin::*",
        /* allowOverwrite */ 1);

    if (result != TCL_OK) {
        char msg[256];
        sprintf(msg, "\n    (while installing built-in commands for class \"%.100s\")", className);
        Tcl_AddErrorInfo(interp, msg);

        Tcl_DeleteNamespace(iclsPtr->nsPtr);
        return TCL_ERROR;
    }

    /*
     *  Push this class onto the class definition stack so that it
     *  becomes the current context for all commands in the parser.
     *  Activate the parser and evaluate the class definition.
     */
    Itcl_PushStack((ClientData)iclsPtr, &infoPtr->clsStack);

    result = Tcl_PushCallFrame(interp, &frame, parserNs,
        /* isProcCallFrame */ 0);

    Itcl_SetCallFrameResolver(interp, iclsPtr->resolvePtr);
    if (result == TCL_OK) {
        result = Tcl_EvalObj(interp, objv[2]);
        Tcl_PopCallFrame(interp);
    }
    Itcl_PopStack(&infoPtr->clsStack);

    if (result != TCL_OK) {
        char msg[256];
        sprintf(msg, "\n    (class \"%.200s\" body line %d)",
            className, interp->errorLine);
        Tcl_AddErrorInfo(interp, msg);

        Tcl_DeleteNamespace(iclsPtr->nsPtr);
        return TCL_ERROR;
    }

    /*
     *  At this point, parsing of the class definition has succeeded.
     *  Add built-in methods such as "configure" and "cget"--as long
     *  as they don't conflict with those defined in the class.
     */
    if (Itcl_InstallBiMethods(interp, iclsPtr) != TCL_OK) {
        Tcl_DeleteNamespace(iclsPtr->nsPtr);
        return TCL_ERROR;
    }

    /*
     *  Build the name resolution tables for all data members.
     */
    Itcl_BuildVirtualTables(iclsPtr);

    /* make the methods and procs known to TclOO */
    ItclMemberFunc *imPtr;
    Tcl_DStringInit(&buffer);
    FOREACH_HASH_VALUE(imPtr, &iclsPtr->functions) {
        if (!(imPtr->flags & ITCL_IMPLEMENT_NONE)) {
	    argumentPtr = imPtr->codePtr->argumentPtr;
	    bodyPtr = imPtr->codePtr->bodyPtr;
	    if (imPtr->codePtr->flags & ITCL_BUILTIN) {
//FIX ME MEMORY leak!!
	        argumentPtr = Tcl_NewStringObj("args", -1);
		int isDone;
		isDone = 0;
	        bodyPtr = Tcl_NewStringObj("return [uplevel 0 ", -1);
		if (strcmp(Tcl_GetString(imPtr->codePtr->bodyPtr),
		        "@itcl-builtin-cget") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::cget", -1);
		    isDone = 1;
		}
		if (strcmp(Tcl_GetString(imPtr->codePtr->bodyPtr),
		        "@itcl-builtin-configure") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::configure", -1);
		    isDone = 1;
		}
		if (strcmp(Tcl_GetString(imPtr->codePtr->bodyPtr),
		        "@itcl-builtin-info") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::Info", -1);
		    isDone = 1;
		}
		if (strcmp(Tcl_GetString(imPtr->codePtr->bodyPtr),
		        "@itcl-builtin-isa") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::isa", -1);
		    isDone = 1;
		}
		if (!isDone) {
		    Tcl_AppendToObj(bodyPtr,
		            Tcl_GetString(imPtr->codePtr->bodyPtr), -1);
                }
	        Tcl_AppendToObj(bodyPtr, " {*}[list $args]]", -1);
	    }
	    ClientData pmPtr;
	    imPtr->tmPtr = (ClientData)Itcl_NewProcClassMethod(interp,
	        iclsPtr->clsPtr, ItclCheckCallMethod, ItclAfterCallMethod,
                ItclProcErrorProc, imPtr, imPtr->namePtr, argumentPtr,
		bodyPtr, &pmPtr);
	    hPtr2 = Tcl_CreateHashEntry(&iclsPtr->infoPtr->procMethods,
	            (char *)imPtr->tmPtr, &isNewEntry);
	    if (isNewEntry) {
	        Tcl_SetHashValue(hPtr2, imPtr);
	    }
	    if ((imPtr->flags & ITCL_COMMON) == 0) {
	        imPtr->accessCmd = Tcl_CreateObjCommand(interp,
		        Tcl_GetString(imPtr->fullNamePtr),
		        Itcl_ExecMethod, imPtr, Itcl_ReleaseData);
	    } else {
	        imPtr->accessCmd = Tcl_CreateObjCommand(interp,
		        Tcl_GetString(imPtr->fullNamePtr),
			Itcl_ExecProc, imPtr, Itcl_ReleaseData);
	    }
            Tcl_DStringInit(&buffer);
        }
    }
    Tcl_DStringFree(&buffer);

    Tcl_ResetResult(interp);
    if (iclsPtrPtr != NULL) {
        *iclsPtrPtr = iclsPtr;
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassInheritCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "inherit" command is invoked to define one or more base classes.
 *  Handles the following syntax:
 *
 *      inherit <baseclass> ?<baseclass>...?
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassInheritCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_InheritCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    int result;
    int i;
    int newEntry;
    int haveClasses;
    char *token;
    Itcl_ListElem *elem;
    Itcl_ListElem *elem2;
    ItclClass *cdPtr;
    ItclClass *baseClsPtr;
    ItclClass *badCdPtr;
    ItclHierIter hier;
    Itcl_Stack stack;
    Tcl_CallFrame frame;
    Tcl_DString buffer;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "class ?class...?");
        return TCL_ERROR;
    }

    /*
     *  An "inherit" statement can only be included once in a
     *  class definition.
     */
    elem = Itcl_FirstListElem(&iclsPtr->bases);
    if (elem != NULL) {
        Tcl_AppendToObj(Tcl_GetObjResult(interp), "inheritance \"", -1);

        while (elem) {
            cdPtr = (ItclClass*)Itcl_GetListValue(elem);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                Tcl_GetString(cdPtr->namePtr), " ", (char*)NULL);

            elem = Itcl_NextListElem(elem);
        }

        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\" already defined for class \"",
	    Tcl_GetString(iclsPtr->fullNamePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Validate each base class and add it to the "bases" list.
     */
    result = Tcl_PushCallFrame(interp, &frame, iclsPtr->nsPtr->parentPtr,
        /* isProcCallFrame */ 0);

    if (result != TCL_OK) {
        return TCL_ERROR;
    }

    for (objc--,objv++; objc > 0; objc--,objv++) {

        /*
         *  Make sure that the base class name is known in the
         *  parent namespace (currently active).  If not, try
         *  to autoload its definition.
         */
        token = Tcl_GetString(*objv);
        baseClsPtr = Itcl_FindClass(interp, token, /* autoload */ 1);
        if (!baseClsPtr) {
            Tcl_Obj *resultPtr = Tcl_GetObjResult(interp);
            int errlen;
            char *errmsg;

            Tcl_IncrRefCount(resultPtr);
            errmsg = Tcl_GetStringFromObj(resultPtr, &errlen);

            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "cannot inherit from \"", token, "\"",
                (char*)NULL);

            if (errlen > 0) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    " (", errmsg, ")", (char*)NULL);
            }
            Tcl_DecrRefCount(resultPtr);
            goto inheritError;
        }

        /*
         *  Make sure that the base class is not the same as the
         *  class that is being built.
         */
        if (baseClsPtr == iclsPtr) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "class \"", Tcl_GetString(iclsPtr->namePtr),
		"\" cannot inherit from itself",
                (char*)NULL);
            goto inheritError;
        }

        Itcl_AppendList(&iclsPtr->bases, (ClientData)baseClsPtr);
        Itcl_PreserveData((ClientData)baseClsPtr);
    }

    /*
     *  Scan through the inheritance list to make sure that no
     *  class appears twice.
     */
    elem = Itcl_FirstListElem(&iclsPtr->bases);
    while (elem) {
        elem2 = Itcl_NextListElem(elem);
        while (elem2) {
            if (Itcl_GetListValue(elem) == Itcl_GetListValue(elem2)) {
                cdPtr = (ItclClass*)Itcl_GetListValue(elem);
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "class \"", iclsPtr->fullNamePtr,
                    "\" cannot inherit base class \"",
                    cdPtr->fullNamePtr, "\" more than once",
                    (char*)NULL);
                goto inheritError;
            }
            elem2 = Itcl_NextListElem(elem2);
        }
        elem = Itcl_NextListElem(elem);
    }

    /*
     *  Add each base class and all of its base classes into
     *  the heritage for the current class.  Along the way, make
     *  sure that no class appears twice in the heritage.
     */
    Itcl_InitHierIter(&hier, iclsPtr);
    cdPtr = Itcl_AdvanceHierIter(&hier);  /* skip the class itself */
    cdPtr = Itcl_AdvanceHierIter(&hier);
    while (cdPtr != NULL) {
        (void) Tcl_CreateHashEntry(&iclsPtr->heritage,
            (char*)cdPtr, &newEntry);

        if (!newEntry) {
            break;
        }
        cdPtr = Itcl_AdvanceHierIter(&hier);
    }
    Itcl_DeleteHierIter(&hier);

    /*
     *  Same base class found twice in the hierarchy?
     *  Then flag error.  Show the list of multiple paths
     *  leading to the same base class.
     */
    if (!newEntry) {
        Tcl_Obj *resultPtr = Tcl_GetObjResult(interp);

        badCdPtr = cdPtr;
        Tcl_AppendStringsToObj(resultPtr,
            "class \"", Tcl_GetString(iclsPtr->fullNamePtr),
	    "\" inherits base class \"",
            Tcl_GetString(badCdPtr->fullNamePtr), "\" more than once:",
            (char*)NULL);

        cdPtr = iclsPtr;
        Itcl_InitStack(&stack);
        Itcl_PushStack((ClientData)cdPtr, &stack);

        /*
         *  Show paths leading to bad base class
         */
        while (Itcl_GetStackSize(&stack) > 0) {
            cdPtr = (ItclClass*)Itcl_PopStack(&stack);

            if (cdPtr == badCdPtr) {
                Tcl_AppendToObj(resultPtr, "\n  ", -1);
                for (i=0; i < Itcl_GetStackSize(&stack); i++) {
                    if (Itcl_GetStackValue(&stack, i) == NULL) {
                        cdPtr = (ItclClass*)Itcl_GetStackValue(&stack, i-1);
                        Tcl_AppendStringsToObj(resultPtr,
                            Tcl_GetString(cdPtr->namePtr), "->",
                            (char*)NULL);
                    }
                }
                Tcl_AppendToObj(resultPtr,
		        Tcl_GetString(badCdPtr->namePtr), -1);
            }
            else if (!cdPtr) {
                (void)Itcl_PopStack(&stack);
            }
            else {
                elem = Itcl_LastListElem(&cdPtr->bases);
                if (elem) {
                    Itcl_PushStack((ClientData)cdPtr, &stack);
                    Itcl_PushStack((ClientData)NULL, &stack);
                    while (elem) {
                        Itcl_PushStack(Itcl_GetListValue(elem), &stack);
                        elem = Itcl_PrevListElem(elem);
                    }
                }
            }
        }
        Itcl_DeleteStack(&stack);
        goto inheritError;
    }

    /*
     *  At this point, everything looks good.
     *  Finish the installation of the base classes.  Update
     *  each base class to recognize the current class as a
     *  derived class.
     */
    Tcl_DStringInit(&buffer);
    haveClasses = 0;
    elem = Itcl_FirstListElem(&iclsPtr->bases);
    Tcl_DStringAppend(&buffer, "::oo::define ", -1);
    Tcl_DStringAppend(&buffer, Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_DStringAppend(&buffer, " superclass", -1);
    while (elem) {
        baseClsPtr = (ItclClass*)Itcl_GetListValue(elem);
        haveClasses++;
        Tcl_DStringAppend(&buffer, " ", -1);
        Tcl_DStringAppend(&buffer, Tcl_GetString(baseClsPtr->fullNamePtr), -1);

        Itcl_AppendList(&baseClsPtr->derived, (ClientData)iclsPtr);
        Itcl_PreserveData((ClientData)iclsPtr);

        elem = Itcl_NextListElem(elem);
    }
    Tcl_PopCallFrame(interp);
    if (haveClasses) {
        result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
    }

    return result;


    /*
     *  If the "inherit" list cannot be built properly, tear it
     *  down and return an error.
     */
inheritError:
    Tcl_PopCallFrame(interp);

    elem = Itcl_FirstListElem(&iclsPtr->bases);
    while (elem) {
        Itcl_ReleaseData( Itcl_GetListValue(elem) );
        elem = Itcl_DeleteListElem(elem);
    }
    return TCL_ERROR;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassProtectionCmd()
 *
 *  Invoked by Tcl whenever the user issues a protection setting
 *  command like "public" or "private".  Creates commands and
 *  variables, and assigns a protection level to them.  Protection
 *  levels are defined as follows:
 *
 *    public    => accessible from any namespace
 *    protected => accessible from selected namespaces
 *    private   => accessible only in the namespace where it was defined
 *
 *  Handles the following syntax:
 *
 *    public <command> ?<arg> <arg>...?
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassProtectionCmd(
    ClientData clientData,   /* protection level (public/protected/private) */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_ClassProtectionCmd", objc, objv);
    ProtectionCmdInfo *pInfo = (ProtectionCmdInfo*)clientData;

    int result;
    int oldLevel;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "command ?arg arg...?");
        return TCL_ERROR;
    }

    oldLevel = Itcl_Protection(interp, pInfo->pLevel);

    if (objc == 2) {
	/* something like: public { variable a; variable b } */
        result = Tcl_EvalObj(interp, objv[1]);
    } else {
	/* something like: public variable a 123 456 */
        result = Itcl_EvalArgs(interp, objc-1, objv+1);
    }

    if (result == TCL_BREAK) {
        Tcl_SetResult(interp, "invoked \"break\" outside of a loop",
            TCL_STATIC);
        result = TCL_ERROR;
    } else {
        if (result == TCL_CONTINUE) {
            Tcl_SetResult(interp, "invoked \"continue\" outside of a loop",
                    TCL_STATIC);
            result = TCL_ERROR;
        } else {
	    if (result != TCL_OK) {
                char mesg[256], *token;
                token = Tcl_GetString(objv[0]);
                sprintf(mesg, "\n    (%.100s body line %d)", token,
		        interp->errorLine);
                Tcl_AddErrorInfo(interp, mesg);
            }
        }
    }

    Itcl_Protection(interp, oldLevel);
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassConstructorCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "constructor" command is invoked to define the constructor
 *  for an object.  Handles the following syntax:
 *
 *      constructor <arglist> ?<init>? <body>
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassConstructorCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_ClassConstructorCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    Tcl_Obj *namePtr;
    char *arglist;
    char *body;

    if (objc < 3 || objc > 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "args ?init? body");
        return TCL_ERROR;
    }

    namePtr = objv[0];
    if (Tcl_FindHashEntry(&iclsPtr->functions, (char *)objv[0])) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\"", Tcl_GetString(namePtr), "\" already defined in class \"",
            iclsPtr->fullNamePtr, "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  If there is an object initialization statement, pick this
     *  out and take the last argument as the constructor body.
     */
    arglist = Tcl_GetString(objv[1]);
    if (objc == 3) {
        body = Tcl_GetString(objv[2]);
    } else {
        iclsPtr->initCode = objv[2];
        Tcl_IncrRefCount(iclsPtr->initCode);
        body = Tcl_GetString(objv[3]);
    }
    if (iclsPtr->initCode != NULL) {
	Tcl_Obj *initNamePtr;
	initNamePtr = Tcl_NewStringObj("___constructor_init", -1);
        if (Itcl_CreateMethod(interp, iclsPtr, initNamePtr, arglist,
	        Tcl_GetString(iclsPtr->initCode)) != TCL_OK) {
            return TCL_ERROR;
        }
    }

    if (Itcl_CreateMethod(interp, iclsPtr, namePtr, arglist, body) != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassDestructorCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "destructor" command is invoked to define the destructor
 *  for an object.  Handles the following syntax:
 *
 *      destructor <body>
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassDestructorCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_ClassDestructorCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    Tcl_Obj *namePtr;
    char *body;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "body");
        return TCL_ERROR;
    }

    namePtr = objv[0];
    body = Tcl_GetString(objv[1]);

    if (Tcl_FindHashEntry(&iclsPtr->functions, (char *)namePtr)) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\"", Tcl_GetString(namePtr), "\" already defined in class \"",
            iclsPtr->fullNamePtr, "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    if (Itcl_CreateMethod(interp, iclsPtr, namePtr, (char*)NULL, body)
        != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassMethodCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "method" command is invoked to define an object method.
 *  Handles the following syntax:
 *
 *      method <name> ?<arglist>? ?<body>?
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassMethodCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_ClassMethodCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    Tcl_Obj *namePtr;
    char *arglist;
    char *body;

    if (objc < 2 || objc > 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "name ?args? ?body?");
        return TCL_ERROR;
    }

    namePtr = objv[1];

    arglist = NULL;
    body = NULL;
    if (objc >= 3) {
        arglist = Tcl_GetString(objv[2]);
    }
    if (objc >= 4) {
        body = Tcl_GetString(objv[3]);
    }

    if (Itcl_CreateMethod(interp, iclsPtr, namePtr, arglist, body) != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassProcCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "proc" command is invoked to define a common class proc.
 *  A "proc" is like a "method", but only has access to "common"
 *  class variables.  Handles the following syntax:
 *
 *      proc <name> ?<arglist>? ?<body>?
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassProcCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_ClassProcCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    Tcl_Obj *namePtr;
    char *arglist;
    char *body;

    if (objc < 2 || objc > 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "name ?args? ?body?");
        return TCL_ERROR;
    }

    namePtr = objv[1];

    arglist = NULL;
    body = NULL;
    if (objc >= 3) {
        arglist = Tcl_GetString(objv[2]);
    }
    if (objc >= 4) {
        body = Tcl_GetString(objv[3]);
    }

    if (Itcl_CreateProc(interp, iclsPtr, namePtr, arglist, body) != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassVariableCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "variable" command is invoked to define an instance variable.
 *  Handles the following syntax:
 *
 *      variable <varname> ?<init>? ?<config>?
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassVariableCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_ClassVariableCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    int pLevel;
    ItclVariable *ivPtr;
    Tcl_Obj *namePtr;
    char *init;
    char *config;

    pLevel = Itcl_Protection(interp, 0);

    if (pLevel == ITCL_PUBLIC) {
        if (objc < 2 || objc > 4) {
            Tcl_WrongNumArgs(interp, 1, objv, "name ?init? ?config?");
            return TCL_ERROR;
        }
    } else {
        if ((objc < 2) || (objc > 3)) {
            Tcl_WrongNumArgs(interp, 1, objv, "name ?init?");
            return TCL_ERROR;
        }
    }

    /*
     *  Make sure that the variable name does not contain anything
     *  goofy like a "::" scope qualifier.
     */
    namePtr = objv[1];
    if (strstr(Tcl_GetString(namePtr), "::")) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "bad variable name \"", Tcl_GetString(namePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    init   = NULL;
    config = NULL;
    if (objc >= 3) {
        init = Tcl_GetString(objv[2]);
    }
    if (objc >= 4) {
        config = Tcl_GetString(objv[3]);
    }

    if (Itcl_CreateVariable(interp, iclsPtr, namePtr, init, config,
            &ivPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    iclsPtr->numVariables++;

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassCommonCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "common" command is invoked to define a variable that is
 *  common to all objects in the class.  Handles the following syntax:
 *
 *      common <varname> ?<init>?
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassCommonCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_ClassCommonCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    Tcl_Namespace *commonNsPtr;
    Tcl_DString buffer;

    Tcl_CallFrame frame;
    Tcl_Obj *namePtr;
    Tcl_HashEntry *hPtr;
    Tcl_Var varPtr;
    char *init;
    ItclVariable *ivPtr;
    int result;
    int isNew;

    if ((objc < 2) || (objc > 3)) {
        Tcl_WrongNumArgs(interp, 1, objv, "varname ?init?");
        return TCL_ERROR;
    }

    /*
     *  Make sure that the variable name does not contain anything
     *  goofy like a "::" scope qualifier.
     */
    namePtr = objv[1];
    if (strstr(Tcl_GetString(namePtr), "::")) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "bad variable name \"", Tcl_GetString(namePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    init = NULL;
    if (objc >= 3) {
        init = Tcl_GetString(objv[2]);
    }

    if (Itcl_CreateVariable(interp, iclsPtr, namePtr, init, (char*)NULL,
            &ivPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    ivPtr->flags |= ITCL_COMMON;
    iclsPtr->numCommons++;

    /*
     *  Create the variable in the namespace associated with the
     *  class.  Do this the hard way, to avoid the variable resolver
     *  procedures.  These procedures won't work until we rebuild
     *  the virtual tables below.
     */
    Tcl_DStringInit(&buffer);
    if (ivPtr->protection != ITCL_PUBLIC) {
        /* public commons go to the class namespace directly the others
	 * go to the variables namespace of the class */
        Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
    }
    Tcl_DStringAppend(&buffer, Tcl_GetString(ivPtr->iclsPtr->fullNamePtr), -1);
    commonNsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer), NULL, 0);
    if (commonNsPtr == NULL) {
        Tcl_AppendResult(interp, "ITCL: cannot find common variables namespace",
	        " for class \"", Tcl_GetString(ivPtr->iclsPtr->fullNamePtr),
		"\"", NULL);
	return TCL_ERROR;
    }
    varPtr = Tcl_NewNamespaceVar(interp, commonNsPtr,
            Tcl_GetString(ivPtr->namePtr));
    hPtr = Tcl_CreateHashEntry(&iclsPtr->classCommons, (char *)ivPtr,
            &isNew);
    if (isNew) {
        Tcl_SetHashValue(hPtr, varPtr);
    }
    result = Tcl_PushCallFrame(interp, &frame, commonNsPtr,
        /* isProcCallFrame */ 0);
    IctlVarTraceInfo *traceInfoPtr;
    traceInfoPtr = (IctlVarTraceInfo *)ckalloc(sizeof(IctlVarTraceInfo));
    memset (traceInfoPtr, 0, sizeof(IctlVarTraceInfo));
    traceInfoPtr->flags = ITCL_TRACE_CLASS;
    traceInfoPtr->ioPtr = NULL;
    traceInfoPtr->iclsPtr = ivPtr->iclsPtr;
    traceInfoPtr->ivPtr = ivPtr;
    Tcl_TraceVar2(interp, Tcl_GetString(ivPtr->namePtr), NULL,
           TCL_TRACE_UNSETS, ItclTraceUnsetVar,
           (ClientData)traceInfoPtr);
    Tcl_PopCallFrame(interp);

    /*
     *  TRICKY NOTE:  Make sure to rebuild the virtual tables for this
     *    class so that this variable is ready to access.  The variable
     *    resolver for the parser namespace needs this info to find the
     *    variable if the developer tries to set it within the class
     *    definition.
     *
     *  If an initialization value was specified, then initialize
     *  the variable now.
     */
    Itcl_BuildVirtualTables(iclsPtr);

    if (init) {
        Tcl_DStringAppend(&buffer, "::", -1);
        Tcl_DStringAppend(&buffer, Tcl_GetString(ivPtr->namePtr), -1);
        CONST char *val = Tcl_SetVar(interp,
	        Tcl_DStringValue(&buffer), init,
                TCL_NAMESPACE_ONLY);

        if (!val) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "cannot initialize common variable \"",
                Tcl_GetString(ivPtr->namePtr), "\"",
                (char*)NULL);
            return TCL_ERROR;
        }
    }
    Tcl_DStringFree(&buffer);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  ItclFreeParserCommandData()
 *
 *  This callback will free() up memory dynamically allocated
 *  and passed as the ClientData argument to Tcl_CreateObjCommand.
 *  This callback is required because one can not simply pass
 *  a pointer to the free() or ckfree() to Tcl_CreateObjCommand.
 * ------------------------------------------------------------------------
 */
static void
ItclFreeParserCommandData(
    ClientData cdata)  /* client data to be destroyed */
{
    ckfree(cdata);
}

/*
 * ------------------------------------------------------------------------
 *  ItclDelObjectInfo()
 *
 *  Invoked when the management info for [incr Tcl] is no longer being
 *  used in an interpreter.  This will only occur when all class
 *  manipulation commands are removed from the interpreter.
 * ------------------------------------------------------------------------
 */
static void
ItclDelObjectInfo(
    char* cdata)    /* client data for class command */
{
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)cdata;

    ItclObject *contextObj;
    Tcl_HashSearch place;
    Tcl_HashEntry *entry;

    /*
     *  Destroy all known objects by deleting their access
     *  commands.
     */
    entry = Tcl_FirstHashEntry(&infoPtr->objects, &place);
    while (entry) {
        contextObj = (ItclObject*)Tcl_GetHashValue(entry);
        Tcl_DeleteCommandFromToken(infoPtr->interp, contextObj->accessCmd);
	    /*
	     * Fix 227804: Whenever an object to delete was found we
	     * have to reset the search to the beginning as the
	     * current entry in the search was deleted and accessing it
	     * is therefore not allowed anymore.
	     */

	    entry = Tcl_FirstHashEntry(&infoPtr->objects, &place);
	    /*entry = Tcl_NextHashEntry(&place);*/
    }
    Tcl_DeleteHashTable(&infoPtr->objects);

    Itcl_DeleteStack(&infoPtr->clsStack);
// FIX ME !!!
// free class_meta_type and object_meta_type
    ckfree((char*)infoPtr);
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassFilterCmd()
 *
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassFilterCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj **newObjv;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    int result;

    ItclShowArgs(1, "Itcl_ClassFilterCmd", objc, objv);
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_CLASS) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget/::itcl::widgetadaptor/::itcl::type.", 
		" Only these can delegate procs", NULL);
	return TCL_ERROR;
    }
/* FIX ME need to change the chain command to do the same here as the TclOO next command !! */
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "<filterName> ?<filterName> ...?");
        return TCL_ERROR;
    }
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+2));
    newObjv[0] = Tcl_NewStringObj("::oo::define", -1);
    Tcl_IncrRefCount(newObjv[0]);
    newObjv[1] = Tcl_NewStringObj(Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_IncrRefCount(newObjv[1]);
    newObjv[2] = Tcl_NewStringObj("filter", -1);
    Tcl_IncrRefCount(newObjv[2]);
    memcpy(newObjv+3, objv+1, sizeof(Tcl_Obj *)*(objc-1));
ItclShowArgs(1, "Itcl_ClassFilterCmd2", objc+2, newObjv);
    result = Tcl_EvalObjv(interp, objc+2, newObjv, 0);
    Tcl_DecrRefCount(newObjv[0]);
    Tcl_DecrRefCount(newObjv[1]);
    Tcl_DecrRefCount(newObjv[2]);
    ckfree((char *)newObjv);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassMixinCmd()
 *
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassMixinCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "Itcl_ClassMixinCmd", objc, objv);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_TypeCmdStart()
 *
 *  that is just a dummy command to load package ItclWidget
 *  and then to resend the command and execute it in that package
 *  package ItclWidget is renaming the Tcl command!!
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_TypeCmdStart(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(1, "Itcl_TypeCmdStart", objc-1, objv);
    const char *res = Tcl_PkgRequire(interp, "ItclWidget", "4.0", 0);
    if (res == NULL) {
        return TCL_ERROR;
    }
    return Tcl_EvalObjv(interp, objc, objv, 0);
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_WidgetCmdStart()
 *
 *  that is just a dummy command to load package ItclWidget
 *  and then to resend the command and execute it in that package
 *  package ItclWidget is renaming the Tcl command!!
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_WidgetCmdStart(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(1, "Itcl_WidgetCmdStart", objc-1, objv);
    const char *res = Tcl_PkgRequire(interp, "ItclWidget", "4.0", 0);
    if (res == NULL) {
        return TCL_ERROR;
    }
    return Tcl_EvalObjv(interp, objc, objv, 0);
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_WidgetadaptorCmdStart()
 *
 *  that is just a dummy command to load package ItclWidget
 *  and then to resend the command and execute it in that package
 *  package ItclWidget is renaming the Tcl command!!
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_WidgetAdaptorCmdStart(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(1, "Itcl_WidgetAdaptorCmdStart", objc-1, objv);
    const char *res = Tcl_PkgRequire(interp, "ItclWidget", "4.0", 0);
    if (res == NULL) {
        return TCL_ERROR;
    }
    return Tcl_EvalObjv(interp, objc, objv, 0);
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassOptionCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "option" command is invoked to define an option 
 *  Handles the following syntax:
 *
 *      option 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassOptionCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclOption *ioptPtr;
    Tcl_Obj *namePtr;
    Tcl_Obj *classNamePtr;
    Tcl_Obj *nameSpecPtr;
    Tcl_Obj **newObjv;
    char *init;
    char *defaultValue;
    char *cgetMethod;
    char *configureMethod;
    char *validateMethod;
    char *token;
    char *usage;
    const char **argv;
    const char *name;
    const char *resourceName;
    const char *className;
    char ch;
    int argc;
    int pLevel;
    int readOnly;
    int newObjc;
    int foundOption;
    int i;

    ItclShowArgs(1, "Itcl_ClassOptionCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_CLASS) {
        Tcl_AppendResult(interp, "a \"class\" cannot have options", NULL);
	return TCL_ERROR;
    }
    pLevel = Itcl_Protection(interp, 0);

    usage = "namespec ?init? ?-default value? ?-readonly? ?-cgetmethod methodName? ?-configuremethod methodName? ?-validatemethod methodName?";
    if (pLevel == ITCL_PUBLIC) {
        if (objc < 2 || objc > 11) {
            Tcl_WrongNumArgs(interp, 1, objv, usage);
            return TCL_ERROR;
        }
    } else {
        if ((objc < 2) || (objc > 12)) {
            Tcl_WrongNumArgs(interp, 1, objv, usage);
            return TCL_ERROR;
	}
    }

    defaultValue = NULL;
    cgetMethod = NULL;
    configureMethod = NULL;
    validateMethod = NULL;
    readOnly = 0;
    newObjc = 0;
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*newObjc);
    for (i=1; i<objc; i++) {
        token = Tcl_GetString(objv[i]);
	foundOption = 0;
	if (*token == '-') {
	    if (objc < i+1) {
                Tcl_WrongNumArgs(interp, 1, objv, usage);
	        return TCL_ERROR;
	    }
	    if (strcmp(token, "-default") == 0) {
	        foundOption = 1;
		i++;
	        defaultValue = Tcl_GetString(objv[i]);
	    } else {
	      if (strcmp(token, "-readonly") == 0) {
	        foundOption = 1;
		readOnly = 1;
	      } else {
	        if (strcmp(token, "-cgetmethod") == 0) {
	            foundOption = 1;
		    i++;
	            cgetMethod = Tcl_GetString(objv[i]);
		} else {
	          if (strcmp(token, "-configuremethod") == 0) {
	              foundOption = 1;
		      i++;
	              configureMethod = Tcl_GetString(objv[i]);
		  } else {
	            if (strcmp(token, "-validatemethod") == 0) {
	                foundOption = 1;
		        i++;
	                validateMethod = Tcl_GetString(objv[i]);
		    }
	          }
	        }
	      }
	    }
	}
	if (!foundOption) {
	    newObjv[newObjc] = objv[i];
	    newObjc++;
	}
    }
    
    if (newObjc < 1) {
        Tcl_AppendResult(interp, usage, NULL);
	return TCL_ERROR;
    }
    resourceName = NULL;
    className = NULL;

    /*
     *  Make sure that the variable name does not contain anything
     *  goofy like a "::" scope qualifier.
     */
    nameSpecPtr = newObjv[0];
    token = Tcl_GetString(nameSpecPtr);
    if (Tcl_SplitList(interp, (CONST84 char *)token, &argc, &argv) != TCL_OK) {
        return TCL_ERROR;
    }
    name = argv[0];
    if (*name != '-') {
	Tcl_AppendResult(interp, "options must start with a '-'", NULL);
        return TCL_ERROR;
    }
    if (strstr(name, "::")) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "bad option name \"", name, "\"", (char*)NULL);
        return TCL_ERROR;
    }
    const char *cp;
    cp = name;
    while (*cp) {
        if (isupper(*cp)) {
	    Tcl_AppendResult(interp,
	            "options must not contain uppercase characters", NULL);
            return TCL_ERROR;
	}
	cp++;
    }
    if (argc > 1) {
        resourceName = argv[1];
    } else {
	/* resource name defaults to option name minus hyphen */
        resourceName = name+1;
    }
    if (argc > 2) {
        className = argv[2];
    } else {
	/* class name defaults to option name minus hyphen and capitalized */
        className = name+1;
    }
    ch = toupper(*className);
    char buf[2];
    sprintf(buf, "%c", ch);
    classNamePtr = Tcl_NewStringObj(buf, -1);
    Tcl_AppendToObj(classNamePtr, className+1, -1);
    Tcl_IncrRefCount(classNamePtr);
    init = defaultValue;
    if ((newObjc > 1) && (init == NULL)) {
        init = Tcl_GetString(newObjv[1]);
    }

    namePtr = Tcl_NewStringObj(name, -1);
    Tcl_IncrRefCount(namePtr);
    if (Itcl_CreateOption(interp, iclsPtr, namePtr, resourceName,
            Tcl_GetString(classNamePtr), init, configureMethod,
	    &ioptPtr) != TCL_OK) {
        Tcl_DecrRefCount(classNamePtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(classNamePtr);
    if (cgetMethod != NULL) {
        ioptPtr->cgetMethodPtr = Tcl_NewStringObj(cgetMethod, -1);
    }
    if (configureMethod != NULL) {
        ioptPtr->configureMethodPtr = Tcl_NewStringObj(configureMethod, -1);
    }
    if (validateMethod != NULL) {
        ioptPtr->validateMethodPtr = Tcl_NewStringObj(validateMethod, -1);
    }
    if (readOnly != 0) {
        ioptPtr->flags |= ITCL_OPTION_READONLY;
    }

    ckfree((char *)newObjv);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclCreateComponent()
 *
 *
 * ------------------------------------------------------------------------
 */
int
ItclCreateComponent(
    Tcl_Interp *interp,
    ItclClass *iclsPtr,
    Tcl_Obj *componentPtr,
    ItclComponent **icPtrPtr)
{
    Tcl_Obj *bodyPtr;
    Tcl_HashEntry *hPtr;
    ItclComponent *icPtr;
    ItclVariable *ivPtr;
    ItclMemberFunc *imPtr;
    int isNew;

    hPtr = Tcl_CreateHashEntry(&iclsPtr->components, (char *)componentPtr,
            &isNew);
    if (isNew) {
        if (Itcl_CreateVariable(interp, iclsPtr, componentPtr, NULL, NULL,
                &ivPtr) != TCL_OK) {
            return TCL_ERROR;
        }
        ivPtr->flags |= ITCL_COMPONENT;
	bodyPtr = Tcl_NewStringObj("return [$", -1);
	Tcl_AppendToObj(bodyPtr, Tcl_GetString(componentPtr), -1);
	Tcl_AppendToObj(bodyPtr, " {*}$args]", -1);
        if (ItclCreateMethod(interp, iclsPtr, componentPtr, "args",
	        Tcl_GetString(bodyPtr), &imPtr) != TCL_OK) {
            return TCL_ERROR;
        }
        imPtr->flags |= ITCL_COMPONENT;
        icPtr = (ItclComponent *)ckalloc(sizeof(ItclComponent));
        memset(icPtr, 0, sizeof(ItclComponent));
        icPtr->namePtr = componentPtr;
        Tcl_IncrRefCount(icPtr->namePtr);
        icPtr->ivPtr = ivPtr;
	Tcl_SetHashValue(hPtr, icPtr);
        /* FIX ME !!! */
        /* need write trace ! */
    } else {
        icPtr =Tcl_GetHashValue(hPtr);
    }
    *icPtrPtr = icPtr;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassComponentCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "component" command is invoked to define a component 
 *  Handles the following syntax:
 *
 *      component 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassComponentCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj **newObjv;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclComponent *icPtr;
    const char *usage;
    int inherit;
    int newObjc;

    ItclShowArgs(1, "Itcl_ClassComponentCmd", objc, objv);
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    usage = "component ?-inherit?";
    if (iclsPtr->flags & ITCL_CLASS) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget/::itcl::widgetadaptor/::itcl::type.", 
		" Only these can have components", NULL);
	return TCL_ERROR;
    }
    if ((objc != 2) && (objc != 3)) {
        Tcl_AppendResult(interp, "wrong # args should be: ", usage, NULL);
        return TCL_ERROR;
    }
    inherit = 0;
    if (objc == 3) {
        if (strcmp(Tcl_GetString(objv[2]), "-inherit") != 0) {
            Tcl_AppendResult(interp, "wrong syntax should be: ", usage, NULL);
            return TCL_ERROR;
	}
        inherit = 1;
    }
    if (ItclCreateComponent(interp, iclsPtr, objv[1], &icPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    if (inherit) {
        icPtr->flags |= ITCL_COMPONENT_INHERIT;
	newObjc = 4;
	newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj*)*newObjc);
	newObjv[0] = Tcl_NewStringObj("delegate::option", -1);
	Tcl_IncrRefCount(newObjv[0]);
	newObjv[1] = Tcl_NewStringObj("*", -1);
	Tcl_IncrRefCount(newObjv[1]);
	newObjv[2] = Tcl_NewStringObj("to", -1);
	Tcl_IncrRefCount(newObjv[2]);
	newObjv[3] = objv[1];
	Tcl_IncrRefCount(newObjv[3]);
        if (Itcl_ClassDelegateOptionCmd(infoPtr, interp, newObjc, newObjv)
	        != TCL_OK) {
	    return TCL_ERROR;
	}
	Tcl_SetStringObj(newObjv[1] , "method", -1);
        if (Itcl_ClassDelegateMethodCmd(infoPtr, interp, newObjc, newObjv)
	        != TCL_OK) {
	    return TCL_ERROR;
	}
	Tcl_DecrRefCount(newObjv[0]);
	Tcl_DecrRefCount(newObjv[1]);
	Tcl_DecrRefCount(newObjv[2]);
	Tcl_DecrRefCount(newObjv[3]);
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassDelegateMethodCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "delegate method" command is invoked to define a 
 *  Handles the following syntax:
 *
 *      delegate method
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassDelegateMethodCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj *methodNamePtr;
    Tcl_Obj *componentPtr;
    Tcl_Obj *targetPtr;
    Tcl_Obj *usingPtr;
    Tcl_Obj *exceptionsPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashEntry *hPtr2;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclComponent *icPtr;
    ItclDelegatedFunction *idmPtr;
    const char *usageStr;
    const char *methodName;
    const char *component;
    const char *token;
    const char **argv;
    int argc;
    int foundOpt;
    int isNew;
    int i;

    ItclShowArgs(1, "Itcl_ClassDelegateMethodCmd", objc, objv);
    usageStr = "delegate method <methodName> to <componentName> ?as <targetName>?\n\
delegate method <methodName> ?to <componentName>? using <pattern>\n\
delegate method * ?to <componentName>? ?using <pattern>? ?except <methods>?";
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_CLASS) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget/::itcl::widgetadaptor/::itcl::type.", 
		" Only these can delegate methods", NULL);
	return TCL_ERROR;
    }
    if (objc < 4) {
	Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
        return TCL_ERROR;
    }
    methodName = Tcl_GetString(objv[1]);
    component = NULL;
    targetPtr = NULL;
    usingPtr = NULL;
    exceptionsPtr = NULL;
    for(i=2;i<objc;i++) {
        token = Tcl_GetString(objv[i]);
	if (i+1 == objc) {
	    Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
	    return TCL_ERROR;
	}
	foundOpt = 0;
	if (strcmp(token, "to") == 0) {
	    i++;
	    component = Tcl_GetString(objv[i]);
	    componentPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "as") == 0) {
	    i++;
	    targetPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "except") == 0) {
	    i++;
	    exceptionsPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "using") == 0) {
	    i++;
	    usingPtr = objv[i];
	    foundOpt++;
        }
        if (!foundOpt) {
	    Tcl_AppendResult(interp, "bad option \"", token, "\" should be ",
	            usageStr, NULL);
	    return TCL_ERROR;
	}
    }
    if ((component == NULL) && (usingPtr == NULL)) {
	Tcl_AppendResult(interp, "missing to should be: ", usageStr, NULL);
	return TCL_ERROR;
    }
    if ((*methodName == '*') && (targetPtr != NULL)) {
	Tcl_AppendResult(interp,
	        "cannot specify \"as\" with \"delegate option *\"", NULL);
	return TCL_ERROR;
    }
    /* check for already delegated */
    methodNamePtr = Tcl_NewStringObj(methodName, -1);
    Tcl_IncrRefCount(methodNamePtr);
    hPtr = Tcl_FindHashEntry(&iclsPtr->delegatedFunctions, (char *)
            methodNamePtr);
    if (hPtr != NULL) {
        Tcl_AppendResult(interp, "method \"", methodName,
	        "\" is already delegated", NULL);
        return TCL_ERROR;
    }

    if (componentPtr != NULL) {
        if (ItclCreateComponent(interp, iclsPtr, componentPtr, &icPtr)
	        != TCL_OK) {
            return TCL_ERROR;
        }
    } else {
        icPtr = NULL;
    }
    idmPtr = (ItclDelegatedFunction *)ckalloc(sizeof(ItclDelegatedFunction));
    memset(idmPtr, 0, sizeof(ItclDelegatedFunction));
    Tcl_InitObjHashTable(&idmPtr->exceptions);
    if (*methodName != '*') {
        if ((targetPtr == NULL) && (usingPtr == NULL)) {
	    targetPtr = methodNamePtr;
	}
	/* FIX ME !!! */
        /* check for locally defined method */
	hPtr = Tcl_FindHashEntry(&iclsPtr->functions, (char *)methodNamePtr);
	if (hPtr != NULL) {
	    Tcl_AppendResult(interp, "method \"", methodName,
	            "\" has been defined locally", NULL);
	    return TCL_ERROR;
	}
        idmPtr->namePtr = methodNamePtr;

    } else {
        idmPtr->namePtr = methodNamePtr;
    }
    idmPtr->icPtr = icPtr;
    idmPtr->asPtr = targetPtr;
    if (idmPtr->asPtr != NULL) {
        Tcl_IncrRefCount(idmPtr->asPtr);
    }
    idmPtr->usingPtr = usingPtr;
    if (idmPtr->usingPtr != NULL) {
        Tcl_IncrRefCount(idmPtr->usingPtr);
    }
    if (exceptionsPtr != NULL) {
        if (Tcl_SplitList(interp, Tcl_GetString(exceptionsPtr), &argc, &argv)
	        != TCL_OK) {
	    return TCL_ERROR;
	}
        for(i=0;i<argc;i++) {
	    Tcl_Obj *objPtr;
	    objPtr = Tcl_NewStringObj(argv[i], -1);
	    Tcl_IncrRefCount(objPtr);
	    hPtr = Tcl_CreateHashEntry(&idmPtr->exceptions, (char *)objPtr,
	            &isNew);
	    hPtr2 = Tcl_FindHashEntry(&iclsPtr->functions, (char *)objPtr);
/* FIX ME !!! can only be done after a class/widget has been parsed completely !! */
#ifdef NOTDEF
	    if (hPtr2 == NULL) {
	        Tcl_AppendResult(interp, "no such method: \"",
		        Tcl_GetString(objPtr), "\" found for delegation", NULL);
	        return TCL_ERROR;
	    }
	    Tcl_SetHashValue(hPtr, Tcl_GetHashValue(hPtr2));
#endif
	}
    }
    hPtr = Tcl_CreateHashEntry(&iclsPtr->delegatedFunctions,
            (char *)idmPtr->namePtr, &isNew);
    Tcl_SetHashValue(hPtr, idmPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_HandleDelegateOptionCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "delegate option" command is invoked to define a 
 *  or if ::itcl::adddelegateoption with an itcl object
 *  Handles the following syntax:
 *
 *      delegate option ...
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_HandleDelegateOptionCmd(
    Tcl_Interp *interp,      /* current interpreter */
    ItclObject *ioPtr,       /* != NULL for ::itcl::adddelgatedoption 
                                otherwise NULL */
    ItclClass *iclsPtr,      /* != NULL for delegate option otherwise NULL */
    ItclDelegatedOption **idoPtrPtr,
                             /* where to return idoPtr */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */

{
    Tcl_Obj *allOptionNamePtr;
    Tcl_Obj *optionNamePtr;
    Tcl_Obj *componentPtr;
    Tcl_Obj *targetPtr;
    Tcl_Obj *exceptionsPtr;
    Tcl_Obj *resourceNamePtr;
    Tcl_Obj *classNamePtr;
    Tcl_HashEntry *hPtr;
    ItclComponent *icPtr;
    ItclDelegatedOption *idoPtr;
    ItclHierIter hier;
    const char *usageStr;
    const char *option;
    const char *component;
    const char *token;
    const char **argv;
    char *what;
    const char *whatName;
    int foundOpt;
    int argc;
    int isNew;
    int i;

    ItclShowArgs(1, "Itcl_HandleDelegatedOptionCmd", objc, objv);
    usageStr = "<optionDef> to <targetDef> ?as <script>? ?except <script>?";
    if (objc < 4) {
	Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
        return TCL_ERROR;
    }
    if (ioPtr != NULL) {
        what = "object";
	whatName = Tcl_GetCommandName(interp, ioPtr->accessCmd);
    } else {
	whatName = iclsPtr->nsPtr->fullName;
        what = "class";
    }
    token = Tcl_GetString(objv[1]);
    if (Tcl_SplitList(interp, (CONST84 char *)token, &argc, &argv) != TCL_OK) {
        return TCL_ERROR;
    }
    option = argv[0];
    if ((argc < 1) || ((*option == '*') && (argc > 1))) {
        Tcl_AppendResult(interp, "<optionDef> must be either \"*\" or ",
	       "\"<optionName> <resourceName> <className>\"", NULL);
	return TCL_ERROR;
    }
    if ((*option != '*') && (argc > 3)) {
        Tcl_AppendResult(interp, "<optionDef> syntax should be: ",
	       "\"<optionName> <resourceName> <className>\"", NULL);
	return TCL_ERROR;
    }
    optionNamePtr = Tcl_NewStringObj(option, -1);
    Tcl_IncrRefCount(optionNamePtr);
    resourceNamePtr = NULL;
    classNamePtr = NULL;
    if (argc > 1) {
       resourceNamePtr = Tcl_NewStringObj(argv[1], -1);
       Tcl_IncrRefCount(resourceNamePtr);
    }
    if (argc > 2) {
       classNamePtr = Tcl_NewStringObj(argv[2], -1);
       Tcl_IncrRefCount(classNamePtr);
    }
    component = NULL;
    targetPtr = NULL;
    exceptionsPtr = NULL;
    for(i=2;i<objc;i++) {
        token = Tcl_GetString(objv[i]);
	if (i+1 == objc) {
	    Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
	    return TCL_ERROR;
	}
	foundOpt = 0;
	if (strcmp(token, "to") == 0) {
	    i++;
	    component = Tcl_GetString(objv[i]);
	    componentPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "as") == 0) {
	    i++;
	    targetPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "except") == 0) {
	    i++;
	    exceptionsPtr = objv[i];
	    foundOpt++;
        }
        if (!foundOpt) {
	    Tcl_AppendResult(interp, "bad option \"", token, "\" should be ",
	            usageStr, NULL);
	    return TCL_ERROR;
	}
    }
    if (component == NULL) {
	Tcl_AppendResult(interp, "missing to should be: ", usageStr, NULL);
	return TCL_ERROR;
    }
    if ((*option == '*') && (targetPtr != NULL)) {
	Tcl_AppendResult(interp,
	        "cannot specify \"as\" with \"delegate option *\"", NULL);
	return TCL_ERROR;
    }
#ifdef NOTDEF
    if ((*option == '*') && (exceptionsPtr != NULL)) {
	Tcl_AppendResult(interp,
	        "cannot specify \"except\" with \"delegate option *\"", NULL);
	return TCL_ERROR;
    }
#endif
    /* check for already delegated */
    allOptionNamePtr = Tcl_NewStringObj("*", -1);
    Tcl_IncrRefCount(allOptionNamePtr);
    if (ioPtr != NULL) {
        hPtr = Tcl_FindHashEntry(&ioPtr->objectDelegatedOptions, (char *)
                allOptionNamePtr);
    } else {
        hPtr = Tcl_FindHashEntry(&iclsPtr->delegatedOptions, (char *)
                allOptionNamePtr);
    }
    Tcl_DecrRefCount(allOptionNamePtr);
    if (hPtr != NULL) {
        Tcl_AppendResult(interp, "option \"", option,
	        "\" is already delegated", NULL);
        return TCL_ERROR;
    }

    if (ioPtr != NULL) {
        Itcl_InitHierIter(&hier, ioPtr->iclsPtr);
	while ((iclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
	    hPtr = Tcl_FindHashEntry(&iclsPtr->components,
	            (char *)componentPtr);
            if (hPtr != NULL) {
	        break;
	    }
	}
	Itcl_DeleteHierIter(&hier);
    } else {
        hPtr = Tcl_FindHashEntry(&iclsPtr->components, (char *)componentPtr);
    }
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, what, " \"", whatName,
	        "\" has no component \"", Tcl_GetString(componentPtr), "\"",
		NULL);
        return TCL_ERROR;
    }
    icPtr = Tcl_GetHashValue(hPtr);
    if (*option != '*') {
	/* FIX ME !!! */
	/* check for locally defined option */
        /* check for valid option name */
        // ItclIsValidOptionName(option);
	if (ioPtr != NULL) {
	    hPtr = Tcl_FindHashEntry(&ioPtr->objectOptions,
	            (char *)optionNamePtr);
	} else {
	    hPtr = Tcl_FindHashEntry(&iclsPtr->options, (char *)optionNamePtr);
	}
	if (hPtr != NULL) {
	    Tcl_AppendResult(interp, "option \"", option,
	            "\" has been defined locally", NULL);
	    return TCL_ERROR;
	}
    }
    idoPtr = (ItclDelegatedOption *)ckalloc(sizeof(ItclDelegatedOption));
    memset(idoPtr, 0, sizeof(ItclDelegatedOption));
    Tcl_InitObjHashTable(&idoPtr->exceptions);
    if (*option != '*') {
        if (targetPtr == NULL) {
	    targetPtr = optionNamePtr;
	}
        if (resourceNamePtr == NULL) {
	    resourceNamePtr = Tcl_NewStringObj(option+1, -1);
	    Tcl_IncrRefCount(resourceNamePtr);
	}
        if (classNamePtr == NULL) {
	    classNamePtr = ItclCapitalize(option+1);
	}
        idoPtr->namePtr = optionNamePtr;
        idoPtr->resourceNamePtr = resourceNamePtr;
        idoPtr->classNamePtr = classNamePtr;

    } else {
        idoPtr->namePtr = optionNamePtr;
    }
    idoPtr->icPtr = icPtr;
    idoPtr->asPtr = targetPtr;
    if (idoPtr->asPtr != NULL) {
        Tcl_IncrRefCount(idoPtr->asPtr);
    }
    if (exceptionsPtr != NULL) {
        if (Tcl_SplitList(interp, Tcl_GetString(exceptionsPtr), &argc, &argv)
	        != TCL_OK) {
	    return TCL_ERROR;
	}
        for(i=0;i<argc;i++) {
	    Tcl_Obj *objPtr;
	    objPtr = Tcl_NewStringObj(argv[i], -1);
	    Tcl_IncrRefCount(objPtr);
	    hPtr = Tcl_CreateHashEntry(&idoPtr->exceptions, (char *)objPtr,
	            &isNew);
#ifdef NOTDEF
	    hPtr2 = Tcl_FindHashEntry(&iclsPtr->options, (char *)objPtr);
/* FIX ME !!! can only be done after a class/widget has been parsed completely !! */
	    if (hPtr2 == NULL) {
	        Tcl_AppendResult(interp, "no such option: \"",
		        Tcl_GetString(objPtr), "\" found for delegation", NULL);
	        return TCL_ERROR;
	    }
	    Tcl_SetHashValue(hPtr, Tcl_GetHashValue(hPtr2));
#endif
	}
    }
    if (idoPtrPtr != NULL) {
        *idoPtrPtr = idoPtr;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassDelegateOptionCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "delegate option" command is invoked to define a 
 *  Handles the following syntax:
 *
 *      delegate option
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassDelegateOptionCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclDelegatedOption *idoPtr;
    const char *usageStr;
    int isNew;
    int result;

    ItclShowArgs(1, "Itcl_ClassDelegateOptionCmd", objc, objv);
    usageStr = "<optionDef> to <targetDef> ?as <script>? ?except <script>?";
    if (objc < 4) {
	Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
        return TCL_ERROR;
    }
    infoPtr = (ItclObjectInfo *)clientData;
    iclsPtr = (ItclClass *)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_CLASS) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget/::itcl::widgetadaptor/::itcl::type/::itcl::struct.", 
		" Only these can delegate options", NULL);
	return TCL_ERROR;
    }
    result = Itcl_HandleDelegateOptionCmd(interp, NULL, iclsPtr, &idoPtr,
             objc, objv);
    if (result != TCL_OK) {
        return result;
    }
    hPtr = Tcl_CreateHashEntry(&iclsPtr->delegatedOptions,
            (char *)idoPtr->namePtr, &isNew);
    Tcl_SetHashValue(hPtr, idoPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassDelegateProcCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "delegate proc" command is invoked to define a 
 *  Handles the following syntax:
 *
 *      delegate proc
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassDelegateProcCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj *procNamePtr;
    Tcl_Obj *componentPtr;
    Tcl_Obj *targetPtr;
    Tcl_Obj *usingPtr;
    Tcl_Obj *exceptionsPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashEntry *hPtr2;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclComponent *icPtr;
    ItclDelegatedFunction *idmPtr;
    const char *usageStr;
    const char *procName;
    const char *component;
    const char *token;
    const char **argv;
    int argc;
    int foundOpt;
    int isNew;
    int i;

    ItclShowArgs(0, "Itcl_ClassDelegateProcCmd", objc, objv);
    usageStr = "delegate proc <procName> to <componentName> ?as <targetName>?\n\
delegate proc <procName> ?to <componentName>? using <pattern>\n\
delegate proc * ?to <componentName>? ?using <pattern>? ?except <procs>?";
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_CLASS) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget/::itcl::widgetadaptor/::itcl::type.", 
		" Only these can delegate procs", NULL);
	return TCL_ERROR;
    }

    if (objc < 4) {
	Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
        return TCL_ERROR;
    }
    procName = Tcl_GetString(objv[1]);
    component = NULL;
    targetPtr = NULL;
    usingPtr = NULL;
    exceptionsPtr = NULL;
    for(i=2;i<objc;i++) {
        token = Tcl_GetString(objv[i]);
	if (i+1 == objc) {
	    Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
	    return TCL_ERROR;
	}
	foundOpt = 0;
	if (strcmp(token, "to") == 0) {
	    i++;
	    component = Tcl_GetString(objv[i]);
	    componentPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "as") == 0) {
	    i++;
	    targetPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "except") == 0) {
	    i++;
	    exceptionsPtr = objv[i];
	    foundOpt++;
        }
	if (strcmp(token, "using") == 0) {
	    i++;
	    usingPtr = objv[i];
	    foundOpt++;
        }
        if (!foundOpt) {
	    Tcl_AppendResult(interp, "bad option \"", token, "\" should be ",
	            usageStr, NULL);
	    return TCL_ERROR;
	}
    }
    if ((component == NULL) && (usingPtr == NULL)) {
	Tcl_AppendResult(interp, "missing to should be: ", usageStr, NULL);
	return TCL_ERROR;
    }
    if ((*procName == '*') && (targetPtr != NULL)) {
	Tcl_AppendResult(interp,
	        "cannot specify \"as\" with \"delegate option *\"", NULL);
	return TCL_ERROR;
    }
    /* check for already delegated */
    procNamePtr = Tcl_NewStringObj(procName, -1);
    Tcl_IncrRefCount(procNamePtr);
    hPtr = Tcl_FindHashEntry(&iclsPtr->delegatedFunctions, (char *)
            procNamePtr);
    if (hPtr != NULL) {
        Tcl_AppendResult(interp, "proc \"", procName,
	        "\" is already delegated", NULL);
        return TCL_ERROR;
    }


    if (componentPtr != NULL) {
        if (ItclCreateComponent(interp, iclsPtr, componentPtr, &icPtr)
	        != TCL_OK) {
            return TCL_ERROR;
        }
    } else {
        icPtr = NULL;
    }
    idmPtr = (ItclDelegatedFunction *)ckalloc(sizeof(ItclDelegatedFunction));
    memset(idmPtr, 0, sizeof(ItclDelegatedFunction));
    Tcl_InitObjHashTable(&idmPtr->exceptions);
    if (*procName != '*') {
        if ((targetPtr == NULL) && (usingPtr == NULL)) {
	    targetPtr = procNamePtr;
	}
	/* FIX ME !!! */
        /* check for locally defined proc */
	hPtr = Tcl_FindHashEntry(&iclsPtr->functions, (char *)procNamePtr);
	if (hPtr != NULL) {
	    Tcl_AppendResult(interp, "proc \"", procName,
	            "\" has been defined locally", NULL);
	    return TCL_ERROR;
	}
        idmPtr->namePtr = procNamePtr;

    } else {
        procNamePtr = Tcl_NewStringObj("*", -1);
	Tcl_IncrRefCount(procNamePtr);
        idmPtr->namePtr = procNamePtr;
    }
    idmPtr->icPtr = icPtr;
    idmPtr->asPtr = targetPtr;
    if (idmPtr->asPtr != NULL) {
        Tcl_IncrRefCount(idmPtr->asPtr);
    }
    idmPtr->usingPtr = usingPtr;
    if (idmPtr->usingPtr != NULL) {
        Tcl_IncrRefCount(idmPtr->usingPtr);
    }
    if (exceptionsPtr != NULL) {
        if (Tcl_SplitList(interp, Tcl_GetString(exceptionsPtr), &argc, &argv)
	        != TCL_OK) {
	    return TCL_ERROR;
	}
        for(i=0;i<argc;i++) {
	    Tcl_Obj *objPtr;
	    objPtr = Tcl_NewStringObj(argv[i], -1);
	    Tcl_IncrRefCount(objPtr);
	    hPtr = Tcl_CreateHashEntry(&idmPtr->exceptions, (char *)objPtr,
	            &isNew);
	    hPtr2 = Tcl_FindHashEntry(&iclsPtr->functions, (char *)objPtr);
/* FIX ME !!! can only be done after a class/widget has been parsed completely !! */
#ifdef NOTDEF
	    if (hPtr2 == NULL) {
	        Tcl_AppendResult(interp, "no such method: \"",
		        Tcl_GetString(objPtr), "\" found for delegation", NULL);
	        return TCL_ERROR;
	    }
	    Tcl_SetHashValue(hPtr, Tcl_GetHashValue(hPtr2));
#endif
	}
    }
    idmPtr->flags = ITCL_COMMON;
    hPtr = Tcl_CreateHashEntry(&iclsPtr->delegatedFunctions,
            (char *)idmPtr->namePtr, &isNew);
    Tcl_SetHashValue(hPtr, idmPtr);
    return TCL_ERROR;
}
/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassForwardCmd()
 *
 *  Used to similar to iterp alias to forward the call of a method 
 *  to another method within the class
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_ClassForwardCmd(
    ClientData clientData,   /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj *prefixObj;
    Tcl_Method mPtr;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;

    ItclShowArgs(1, "Itcl_ClassForwardCmd", objc, objv);
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_CLASS) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget/::itcl::widgetadaptor/",
		"::itcl::type/::itcl::eclass.", 
		" Only these can delegate procs", NULL);
	return TCL_ERROR;
    }
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "<forwardName> <targetName> ?<arg> ...?");
        return TCL_ERROR;
    }
    prefixObj = Tcl_NewListObj(objc-2, objv+2);
    mPtr = Itcl_NewForwardClassMethod(interp, iclsPtr->clsPtr, 1,
            objv[1], prefixObj);
    if (mPtr == NULL) {
        return TCL_ERROR;
    }
    return TCL_OK;
}
