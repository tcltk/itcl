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
 *     RCS:  $Id: itclParse.c,v 1.1.2.1 2007/09/07 21:19:42 wiede Exp $
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
    ItclObjectInfo *info;     /* info regarding all known objects */
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

static const struct {
    const char *name;
    Tcl_ObjCmdProc *objProc;
} parseCmds[] = {
    {"common", Itcl_ClassCommonCmd},
    {"constructor", Itcl_ClassConstructorCmd},
    {"destructor", Itcl_ClassDestructorCmd},
    {"handleClass", Itcl_HandleClass},
    {"inherit", Itcl_ClassInheritCmd},
    {"method", Itcl_ClassMethodCmd},
    {"proc", Itcl_ClassProcCmd},
    {"variable", Itcl_ClassVariableCmd},
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
Itcl_ParseInit(interp, info)
    Tcl_Interp *interp;     /* interpreter to be updated */
    ItclObjectInfo *info;   /* info regarding all known objects and classes */
{
    Tcl_Namespace *parserNs;
    ProtectionCmdInfo *pInfo;
    Tcl_DString buffer;
    int i;

    /*
     *  Create the "itcl::parser" namespace used to parse class
     *  definitions.
     */
    parserNs = Tcl_CreateNamespace(interp, "::itcl::parser",
        (ClientData)info, Itcl_ReleaseData);

    if (!parserNs) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            " (cannot initialize itcl parser)",
            (char*)NULL);
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)info);

    /*
     *  Add commands for parsing class definitions.
     */
    Tcl_DStringInit(&buffer);
    for (i=0 ; parseCmds[i].name ; i++) {
        Tcl_DStringAppend(&buffer, "::itcl::parser::", 16);
        Tcl_DStringAppend(&buffer, parseCmds[i].name, -1);
        Tcl_CreateObjCommand(interp, Tcl_DStringValue(&buffer),
                parseCmds[i].objProc, (ClientData) info, NULL);
        Tcl_DStringFree(&buffer);
    }

    for (i=0 ; protectionCmds[i].name ; i++) {
        Tcl_DStringAppend(&buffer, "::itcl::parser::", 16);
        Tcl_DStringAppend(&buffer, protectionCmds[i].name, -1);
        pInfo = (ProtectionCmdInfo*)ckalloc(sizeof(ProtectionCmdInfo));
        pInfo->pLevel = protectionCmds[i].protection;
        pInfo->info = info;
        Tcl_CreateObjCommand(interp, Tcl_DStringValue(&buffer),
                protectionCmds[i].objProc, (ClientData) pInfo,
		(Tcl_CmdDeleteProc*) ItclFreeParserCommandData);
        Tcl_DStringFree(&buffer);
    }

    /*
     *  Set the runtime variable resolver for the parser namespace,
     *  to control access to "common" data members while parsing
     *  the class definition.
     */
    if (info->useOldResolvers) {
        ItclSetParserResolver(parserNs);
    }
    /*
     *  Install the "class" command for defining new classes.
     */
    Tcl_CreateObjCommand(interp, "::itcl::class", Itcl_ClassCmd,
        (ClientData)info, Itcl_ReleaseData);
    Itcl_PreserveData((ClientData)info);

    Tcl_CreateObjCommand(interp, "::itcl::body", Itcl_BodyCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

    Tcl_CreateObjCommand(interp, "::itcl::configbody", Itcl_ConfigBodyCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

    Itcl_EventuallyFree((ClientData)info, ItclDelObjectInfo);

    /*
     *  Create the "itcl::find" command for high-level queries.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::find") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::find",
            "classes", "?pattern?",
            Itcl_FindClassesCmd,
            (ClientData)info, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)info);

    if (Itcl_AddEnsemblePart(interp, "::itcl::find",
            "objects", "?-class className? ?-isa className? ?pattern?",
            Itcl_FindObjectsCmd,
            (ClientData)info, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)info);


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
            (ClientData)info, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)info);

    if (Itcl_AddEnsemblePart(interp, "::itcl::delete",
            "object", "name ?name...?",
            Itcl_DelObjectCmd,
            (ClientData)info, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)info);

    /*
     *  Create the "itcl::is" command to test object
     *  and classes existence.
     */
    if (Itcl_CreateEnsemble(interp, "::itcl::is") != TCL_OK) {
        return TCL_ERROR;
    }
    if (Itcl_AddEnsemblePart(interp, "::itcl::is",
            "class", "name", Itcl_IsClassCmd,
            (ClientData)info, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)info);

    if (Itcl_AddEnsemblePart(interp, "::itcl::is",
            "object", "?-class classname? name", Itcl_IsObjectCmd,
            (ClientData)info, Itcl_ReleaseData) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_PreserveData((ClientData)info);


    /*
     *  Add "code" and "scope" commands for handling scoped values.
     */
    Tcl_CreateObjCommand(interp, "::itcl::code", Itcl_CodeCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

    Tcl_CreateObjCommand(interp, "::itcl::scope", Itcl_ScopeCmd,
        (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);

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
    ItclObjectInfo* info = (ItclObjectInfo*)clientData;

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

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "name { definition }");
        return TCL_ERROR;
    }
    ItclShowArgs(2, "Itcl_ClassCmd", objc, objv);
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
    if (Itcl_CreateClass(interp, className, info, &iclsPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Import the built-in commands from the itcl::builtin namespace.
     *  Do this before parsing the class definition, so methods/procs
     *  can override the built-in commands.
     */
    result = Tcl_Import(interp, iclsPtr->namesp, "::itcl::builtin::*",
        /* allowOverwrite */ 1);

    if (result != TCL_OK) {
        char msg[256];
        sprintf(msg, "\n    (while installing built-in commands for class \"%.100s\")", className);
        Tcl_AddErrorInfo(interp, msg);

        Tcl_DeleteNamespace(iclsPtr->namesp);
        return TCL_ERROR;
    }

    /*
     *  Push this class onto the class definition stack so that it
     *  becomes the current context for all commands in the parser.
     *  Activate the parser and evaluate the class definition.
     */
    Itcl_PushStack((ClientData)iclsPtr, &info->clsStack);

    result = Tcl_PushCallFrame(interp, &frame, parserNs,
        /* isProcCallFrame */ 0);

    Itcl_SetCallFrameResolver(interp, iclsPtr->resolvePtr);
    if (result == TCL_OK) {
        result = Tcl_EvalObj(interp, objv[2]);
        Tcl_PopCallFrame(interp);
    }
    Itcl_PopStack(&info->clsStack);

    if (result != TCL_OK) {
        char msg[256];
        sprintf(msg, "\n    (class \"%.200s\" body line %d)",
            className, interp->errorLine);
        Tcl_AddErrorInfo(interp, msg);

        Tcl_DeleteNamespace(iclsPtr->namesp);
        return TCL_ERROR;
    }

    /*
     *  At this point, parsing of the class definition has succeeded.
     *  Add built-in methods such as "configure" and "cget"--as long
     *  as they don't conflict with those defined in the class.
     */
    if (Itcl_InstallBiMethods(interp, iclsPtr) != TCL_OK) {
        Tcl_DeleteNamespace(iclsPtr->namesp);
        return TCL_ERROR;
    }

    /*
     *  Build the name resolution tables for all data members.
     */
    Itcl_BuildVirtualTables(iclsPtr);

    /* make the methods and procs known to TclOO */
    ItclMemberFunc *mPtr;
    Tcl_DStringInit(&buffer);
    FOREACH_HASH_VALUE(mPtr, &iclsPtr->functions) {
        if (!(mPtr->flags & ITCL_IMPLEMENT_NONE)) {
	    argumentPtr = mPtr->codePtr->argumentPtr;
	    bodyPtr = mPtr->codePtr->bodyPtr;
	    if (mPtr->codePtr->flags & ITCL_BUILTIN) {
//FIX ME MEMORY leak!!
	        argumentPtr = Tcl_NewStringObj("args", -1);
		int isDone;
		isDone = 0;
	        bodyPtr = Tcl_NewStringObj("return [uplevel 0 ", -1);
		if (strcmp(Tcl_GetString(mPtr->codePtr->bodyPtr),
		        "@itcl-builtin-cget") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::cget", -1);
		    isDone = 1;
		}
		if (strcmp(Tcl_GetString(mPtr->codePtr->bodyPtr),
		        "@itcl-builtin-configure") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::configure", -1);
		    isDone = 1;
		}
		if (strcmp(Tcl_GetString(mPtr->codePtr->bodyPtr),
		        "@itcl-builtin-info") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::Info", -1);
		    isDone = 1;
		}
		if (strcmp(Tcl_GetString(mPtr->codePtr->bodyPtr),
		        "@itcl-builtin-isa") == 0) {
		    Tcl_AppendToObj(bodyPtr, "::itcl::builtin::isa", -1);
		    isDone = 1;
		}
		if (!isDone) {
		    Tcl_AppendToObj(bodyPtr,
		            Tcl_GetString(mPtr->codePtr->bodyPtr), -1);
                }
	        Tcl_AppendToObj(bodyPtr, " {*}[list $args]]", -1);
	    }
	    ClientData pmPtr;
	    mPtr->tmPtr = (ClientData)Itcl_NewProcClassMethod(interp,
	        iclsPtr->classPtr, ItclCheckCallMethod, ItclAfterCallMethod,
                mPtr, mPtr->namePtr, argumentPtr,
		bodyPtr, &pmPtr);
            Tcl_Proc procPtr;
	    procPtr = Tcl_ProcPtrFromPM(pmPtr);
	    hPtr2 = Tcl_CreateHashEntry(&iclsPtr->info->procMethods,
	            (char *)procPtr, &isNewEntry);
	    if (isNewEntry) {
	        Tcl_SetHashValue(hPtr2, mPtr);
	    }
	    if ((mPtr->flags & ITCL_COMMON) == 0) {
	        mPtr->accessCmd = Tcl_CreateObjCommand(interp,
		        Tcl_GetString(mPtr->fullNamePtr),
		        Itcl_ExecMethod, mPtr, Itcl_ReleaseData);
	    } else {
	        mPtr->accessCmd = Tcl_CreateObjCommand(interp,
		        Tcl_GetString(mPtr->fullNamePtr),
			Itcl_ExecProc, mPtr, Itcl_ReleaseData);
	    }
            Tcl_DStringInit(&buffer);
        }
    }
    Tcl_DStringFree(&buffer);

    Tcl_ResetResult(interp);
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
    ItclObjectInfo *info = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&info->clsStack);

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
                Tcl_GetString(cdPtr->name), " ", (char*)NULL);

            elem = Itcl_NextListElem(elem);
        }

        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\" already defined for class \"",
	    Tcl_GetString(iclsPtr->fullname), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Validate each base class and add it to the "bases" list.
     */
    result = Tcl_PushCallFrame(interp, &frame, iclsPtr->namesp->parentPtr,
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
                "class \"", Tcl_GetString(iclsPtr->name),
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
                    "class \"", iclsPtr->fullname,
                    "\" cannot inherit base class \"",
                    cdPtr->fullname, "\" more than once",
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
            "class \"", Tcl_GetString(iclsPtr->fullname),
	    "\" inherits base class \"",
            Tcl_GetString(badCdPtr->fullname), "\" more than once:",
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
                            Tcl_GetString(cdPtr->name), "->",
                            (char*)NULL);
                    }
                }
                Tcl_AppendToObj(resultPtr, Tcl_GetString(badCdPtr->name), -1);
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
    Tcl_DStringAppend(&buffer, Tcl_GetString(iclsPtr->fullname), -1);
    Tcl_DStringAppend(&buffer, " superclass", -1);
    while (elem) {
        baseClsPtr = (ItclClass*)Itcl_GetListValue(elem);
        haveClasses++;
        Tcl_DStringAppend(&buffer, " ", -1);
        Tcl_DStringAppend(&buffer, Tcl_GetString(baseClsPtr->fullname), -1);

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
    ItclObjectInfo *info = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&info->clsStack);

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
            iclsPtr->fullname, "\"",
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
    ItclObjectInfo *info = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&info->clsStack);

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
            iclsPtr->fullname, "\"",
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
    ItclObjectInfo *info = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&info->clsStack);

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
    ItclObjectInfo *info = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&info->clsStack);

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
    ItclObjectInfo *info = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&info->clsStack);

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
    ItclObjectInfo *info = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&info->clsStack);

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
    Tcl_DStringAppend(&buffer, Tcl_GetString(ivPtr->iclsPtr->fullname), -1);
    commonNsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer), NULL, 0);
    if (commonNsPtr == NULL) {
        Tcl_AppendResult(interp, "ITCL: cannot find common variables namespace",
	        " for class \"", Tcl_GetString(ivPtr->iclsPtr->fullname),
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
    ItclObjectInfo *info = (ItclObjectInfo*)cdata;

    ItclObject *contextObj;
    Tcl_HashSearch place;
    Tcl_HashEntry *entry;

    /*
     *  Destroy all known objects by deleting their access
     *  commands.
     */
    entry = Tcl_FirstHashEntry(&info->objects, &place);
    while (entry) {
        contextObj = (ItclObject*)Tcl_GetHashValue(entry);
        Tcl_DeleteCommandFromToken(info->interp, contextObj->accessCmd);
	    /*
	     * Fix 227804: Whenever an object to delete was found we
	     * have to reset the search to the beginning as the
	     * current entry in the search was deleted and accessing it
	     * is therefore not allowed anymore.
	     */

	    entry = Tcl_FirstHashEntry(&info->objects, &place);
	    /*entry = Tcl_NextHashEntry(&place);*/
    }
    Tcl_DeleteHashTable(&info->objects);

    Itcl_DeleteStack(&info->clsStack);
// FIX ME !!!
// free class_meta_type and object_meta_type
    ckfree((char*)info);
}

