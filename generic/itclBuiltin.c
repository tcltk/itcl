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
 *  These procedures handle built-in class methods, including the
 *  "isa" method (to query hierarchy info) and the "info" method
 *  (to query class/object data).
 *
 * ========================================================================
 *  AUTHOR:  Michael J. McLennan
 *           Bell Labs Innovations for Lucent Technologies
 *           mmclennan@lucent.com
 *           http://www.tcltk.com/itcl
 *
 *  overhauled version author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclBuiltin.c,v 1.1.2.35 2008/10/29 19:59:00 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"

Tcl_ObjCmdProc Itcl_BiComponentInstallCmd;
Tcl_ObjCmdProc Itcl_BiDestroyCmd;
Tcl_ObjCmdProc ItclExtendedConfigure;
Tcl_ObjCmdProc ItclExtendedCget;
Tcl_ObjCmdProc ItclExtendedSetGet;

/*
 *  FORWARD DECLARATIONS
 */
static Tcl_Obj* ItclReportPublicOpt _ANSI_ARGS_((Tcl_Interp *interp,
    ItclVariable *ivPtr, ItclObject *contextIoPtr));

static Tcl_ObjCmdProc ItclBiObjectUnknownCmd;
static Tcl_ObjCmdProc ItclBiClassUnknownCmd;
/*
 *  Standard list of built-in methods for all objects.
 */
typedef struct BiMethod {
    char* name;              /* method name */
    char* usage;             /* string describing usage */
    char* registration;      /* registration name for C proc */
    Tcl_ObjCmdProc *proc;    /* implementation C proc */
    int flags;               /* flag for which type of class to be used */
} BiMethod;

static BiMethod BiMethodList[] = {
    { "cget",
        "-option",
        "@itcl-builtin-cget",
        Itcl_BiCgetCmd,
	ITCL_CLASS|ITCL_ECLASS|ITCL_TYPE|ITCL_WIDGET
    },
    { "configure",
        "?-option? ?value -option value...?",
        "@itcl-builtin-configure",
        Itcl_BiConfigureCmd,
	ITCL_CLASS|ITCL_ECLASS|ITCL_TYPE|ITCL_WIDGET
    },
    { "componentinstall",
        "<component> using <classname> <winpath> ?-option value...?",
        "@itcl-builtin-componentinstall",
        Itcl_BiComponentInstallCmd,
	ITCL_WIDGET
    },
    { "destroy",
        "",
        "@itcl-builtin-destroy",
        Itcl_BiDestroyCmd,
	ITCL_TYPE|ITCL_WIDGET
    },
    { "info",
        "???",
        "@itcl-builtin-info",
	Itcl_BiInfoCmd,
	ITCL_CLASS|ITCL_ECLASS|ITCL_TYPE|ITCL_WIDGET
    },
    { "isa",
        "className",
        "@itcl-builtin-isa",
        Itcl_BiIsaCmd,
	ITCL_CLASS|ITCL_ECLASS|ITCL_TYPE|ITCL_WIDGET
    },
    { "setget",
        "varName ?value?",
        "@itcl-builtin-setget",
        ItclExtendedSetGet,
	ITCL_ECLASS
    },
    { "unknown",
        "",
        "@itcl-builtin-classunknown",
        ItclBiClassUnknownCmd,
	ITCL_TYPE
    },
};
static int BiMethodListLen = sizeof(BiMethodList)/sizeof(BiMethod);


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInit()
 *
 *  Creates a namespace full of built-in methods/procs for [incr Tcl]
 *  classes.  This includes things like the "isa" method and "info"
 *  for querying class info.  Usually invoked by Itcl_Init() when
 *  [incr Tcl] is first installed into an interpreter.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
int
Itcl_BiInit(
    Tcl_Interp *interp,      /* current interpreter */
    ItclObjectInfo *infoPtr)
{
    Tcl_Namespace *itclBiNs;
    Tcl_DString buffer;
    int i;

    /*
     *  "::itcl::builtin" commands.
     *  These commands are imported into each class
     *  just before the class definition is parsed.
     */
    Tcl_DStringInit(&buffer);
    for (i=0; i < BiMethodListLen; i++) {
	Tcl_DStringSetLength(&buffer, 0);
	Tcl_DStringAppend(&buffer, "::itcl::builtin::", -1);
	Tcl_DStringAppend(&buffer, BiMethodList[i].name, -1);
        Tcl_CreateObjCommand(interp, Tcl_DStringValue(&buffer),
	        BiMethodList[i].proc, (ClientData)infoPtr,
		(Tcl_CmdDeleteProc*)NULL);
    }
    Tcl_DStringFree(&buffer);

    Tcl_CreateObjCommand(interp, "::itcl::builtin::chain", Itcl_BiChainCmd,
            NULL, (Tcl_CmdDeleteProc*)NULL);

    Tcl_CreateObjCommand(interp, "::itcl::builtin::objectunknown",
            ItclBiObjectUnknownCmd, infoPtr, (Tcl_CmdDeleteProc*)NULL);

    Tcl_CreateObjCommand(interp, "::itcl::builtin::classunknown",
            ItclBiClassUnknownCmd, infoPtr, (Tcl_CmdDeleteProc*)NULL);

    ItclInfoInit(interp);
    /*
     *  Export all commands in the built-in namespace so we can
     *  import them later on.
     */
    itclBiNs = Tcl_FindNamespace(interp, "::itcl::builtin",
        (Tcl_Namespace*)NULL, TCL_LEAVE_ERR_MSG);

    if ((itclBiNs == NULL) ||
        Tcl_Export(interp, itclBiNs, "*", /* resetListFirst */ 1) != TCL_OK) {
        return TCL_ERROR;
    }
    /*
     * Install into the master [info] ensemble.
     */

    Tcl_Command infoCmd;
    infoCmd = Tcl_FindCommand(interp, "info", NULL, TCL_GLOBAL_ONLY);
    if (infoCmd != NULL && Tcl_IsEnsemble(infoCmd)) {
        Tcl_Obj *mapDict;

        Tcl_GetEnsembleMappingDict(NULL, infoCmd, &mapDict);
        if (mapDict != NULL) {
            Tcl_DictObjPut(NULL, mapDict, Tcl_NewStringObj("itclinfo", -1),
                    Tcl_NewStringObj("::itcl::builtin::Info", -1));
/* FIXME !!! need to restore ::tcl::Info_vars if Itcl is unloaded !! */
            Tcl_DictObjPut(NULL, mapDict, Tcl_NewStringObj("vars", -1),
                    Tcl_NewStringObj("::itcl::builtin::Info::vars", -1));
            Tcl_SetEnsembleMappingDict(interp, infoCmd, mapDict);
        }
    }

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_InstallBiMethods()
 *
 *  Invoked when a class is first created, just after the class
 *  definition has been parsed, to add definitions for built-in
 *  methods to the class.  If a method already exists in the class
 *  with the same name as the built-in, then the built-in is skipped.
 *  Otherwise, a method definition for the built-in method is added.
 *
 *  Returns TCL_OK if successful, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */
int
Itcl_InstallBiMethods(
    Tcl_Interp *interp,      /* current interpreter */
    ItclClass *iclsPtr)      /* class definition to be updated */
{
    int result = TCL_OK;
    Tcl_HashEntry *hPtr = NULL;

    int i;
    ItclHierIter hier;
    ItclClass *superPtr;

    /*
     *  Scan through all of the built-in methods and see if
     *  that method already exists in the class.  If not, add
     *  it in.
     *
     *  TRICKY NOTE:  The virtual tables haven't been built yet,
     *    so look for existing methods the hard way--by scanning
     *    through all classes.
     */
    Tcl_Obj *objPtr = Tcl_NewStringObj("", 0);
    for (i=0; i < BiMethodListLen; i++) {
        Itcl_InitHierIter(&hier, iclsPtr);
	Tcl_SetStringObj(objPtr, BiMethodList[i].name, -1);
        superPtr = Itcl_AdvanceHierIter(&hier);
        while (superPtr) {
            hPtr = Tcl_FindHashEntry(&superPtr->functions, (char *)objPtr);
            if (hPtr) {
                break;
            }
            superPtr = Itcl_AdvanceHierIter(&hier);
        }
        Itcl_DeleteHierIter(&hier);

        if (!hPtr) {
	    if (iclsPtr->flags & BiMethodList[i].flags) {
                result = Itcl_CreateMethod(interp, iclsPtr,
	            Tcl_NewStringObj(BiMethodList[i].name, -1),
                    BiMethodList[i].usage, BiMethodList[i].registration);

                if (result != TCL_OK) {
                    break;
                }
	    }
        }
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiIsaCmd()
 *
 *  Invoked whenever the user issues the "isa" method for an object.
 *  Handles the following syntax:
 *
 *    <objName> isa <className>
 *
 *  Checks to see if the object has the given <className> anywhere
 *  in its heritage.  Returns 1 if so, and 0 otherwise.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiIsaCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *iclsPtr;
    char *token;

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    if (contextIoPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "improper usage: should be \"object isa className\"",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (objc != 2) {
        token = Tcl_GetString(objv[0]);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"object ", token, " className\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Look for the requested class.  If it is not found, then
     *  try to autoload it.  If it absolutely cannot be found,
     *  signal an error.
     */
    token = Tcl_GetString(objv[1]);
    iclsPtr = Itcl_FindClass(interp, token, /* autoload */ 1);
    if (iclsPtr == NULL) {
        return TCL_ERROR;
    }

    if (Itcl_ObjectIsa(contextIoPtr, iclsPtr)) {
        Tcl_SetIntObj(Tcl_GetObjResult(interp), 1);
    } else {
        Tcl_SetIntObj(Tcl_GetObjResult(interp), 0);
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiConfigureCmd()
 *
 *  Invoked whenever the user issues the "configure" method for an object.
 *  Handles the following syntax:
 *
 *    <objName> configure ?-<option>? ?<value> -<option> <value>...?
 *
 *  Allows access to public variables as if they were configuration
 *  options.  With no arguments, this command returns the current
 *  list of public variable options.  If -<option> is specified,
 *  this returns the information for just one option:
 *
 *    -<optionName> <initVal> <currentVal>
 *
 *  Otherwise, the list of arguments is parsed, and values are
 *  assigned to the various public variable options.  When each
 *  option changes, a big of "config" code associated with the option
 *  is executed, to bring the object up to date.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiConfigureCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    Tcl_Obj *resultPtr;
    Tcl_Obj *objPtr;
    Tcl_DString buffer;
    Tcl_DString buffer2;
    Tcl_HashSearch place;
    Tcl_HashEntry *hPtr;
    Tcl_Namespace *saveNsPtr;
    Tcl_Obj * const *unparsedObjv;
    ItclClass *iclsPtr;
    ItclVariable *ivPtr;
    ItclVarLookup *vlookup;
    ItclMemberCode *mcode;
    ItclHierIter hier;
    ItclObjectInfo *infoPtr;
    CONST char *lastval;
    char *token;
    char *varName;
    int i;
    int unparsedObjc;
    int result;

    ItclShowArgs(1, "Itcl_BiConfigureCmd", objc, objv);
    vlookup = NULL;
    token = NULL;
    hPtr = NULL;
    unparsedObjc = objc;
    unparsedObjv = objv;
    Tcl_DStringInit(&buffer);
    Tcl_DStringInit(&buffer2);

    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    if (contextIoPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "improper usage: should be ",
            "\"object configure ?-option? ?value -option value...?\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  BE CAREFUL:  work in the virtual scope!
     */
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }

    infoPtr = contextIclsPtr->infoPtr;
    if (!(contextIclsPtr->flags & ITCL_CLASS)) {
	/* first check if it is an option */
	if (objc > 1) {
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->options,
	            (char *) objv[1]);
	}
        result = ItclExtendedConfigure(contextIclsPtr, interp, objc, objv);
        if (result != TCL_CONTINUE) {
            return result;
        }
        if (infoPtr->unparsedObjc > 0) {
            unparsedObjc = infoPtr->unparsedObjc;
            unparsedObjv = infoPtr->unparsedObjv;
	} else {
	    unparsedObjc = 0;
	}
    }
    /*
     *  HANDLE:  configure
     */
    if (unparsedObjc == 1) {
        resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);

        Itcl_InitHierIter(&hier, contextIclsPtr);
        while ((iclsPtr=Itcl_AdvanceHierIter(&hier)) != NULL) {
            hPtr = Tcl_FirstHashEntry(&iclsPtr->variables, &place);
            while (hPtr) {
                ivPtr = (ItclVariable*)Tcl_GetHashValue(hPtr);
                if (ivPtr->protection == ITCL_PUBLIC) {
                    objPtr = ItclReportPublicOpt(interp, ivPtr, contextIoPtr);

                    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr,
                        objPtr);
                }
                hPtr = Tcl_NextHashEntry(&place);
            }
        }
        Itcl_DeleteHierIter(&hier);

        Tcl_SetObjResult(interp, resultPtr);
        return TCL_OK;
    } else {

        /*
         *  HANDLE:  configure -option
         */
        if (unparsedObjc == 2) {
            token = Tcl_GetStringFromObj(unparsedObjv[1], (int*)NULL);
            if (*token != '-') {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "improper usage: should be ",
                    "\"object configure ?-option? ?value -option value...?\"",
                    (char*)NULL);
                return TCL_ERROR;
            }

            vlookup = NULL;
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, token+1);
            if (hPtr) {
                vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);

                if (vlookup->ivPtr->protection != ITCL_PUBLIC) {
                    vlookup = NULL;
                }
            }
            if (!vlookup) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "unknown option \"", token, "\"",
                    (char*)NULL);
                return TCL_ERROR;
            }
            resultPtr = ItclReportPublicOpt(interp,
	            vlookup->ivPtr, contextIoPtr);
            Tcl_SetObjResult(interp, resultPtr);
            return TCL_OK;
        }
    }

    /*
     *  HANDLE:  configure -option value -option value...
     *
     *  Be careful to work in the virtual scope.  If this "configure"
     *  method was defined in a base class, the current namespace
     *  (from Itcl_ExecMethod()) will be that base class.  Activate
     *  the derived class namespace here, so that instance variables
     *  are accessed properly.
     */
    result = TCL_OK;

    for (i=1; i < unparsedObjc; i+=2) {
	if (i+1 >= unparsedObjc) {
	    Tcl_AppendResult(interp, "need option value pair", NULL);
	    result = TCL_ERROR;
            goto configureDone;
	}
        vlookup = NULL;
        token = Tcl_GetString(unparsedObjv[i]);
        if (*token == '-') {
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, token+1);
            if (hPtr == NULL) {
                hPtr = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, token);
	    }
            if (hPtr) {
                vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
            }
        }

        if (!vlookup || (vlookup->ivPtr->protection != ITCL_PUBLIC)) {
            Tcl_AppendResult(interp, "unknown option \"", token, "\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto configureDone;
        }
        if (i == unparsedObjc-1) {
            Tcl_AppendResult(interp, "value for \"", token, "\" missing",
                (char*)NULL);
            result = TCL_ERROR;
            goto configureDone;
        }

        ivPtr = vlookup->ivPtr;
        Tcl_DStringSetLength(&buffer2, 0);
        Tcl_DStringAppend(&buffer2,
	        Tcl_GetString(contextIoPtr->varNsNamePtr), -1);
        Tcl_DStringAppend(&buffer2,
	        Tcl_GetString(ivPtr->iclsPtr->fullNamePtr), -1);
        Tcl_DStringAppend(&buffer2, "::", 2);
        Tcl_DStringAppend(&buffer2,
	        Tcl_GetString(ivPtr->namePtr), -1);
	varName = Tcl_DStringValue(&buffer2);
        lastval = Tcl_GetVar2(interp, varName, (char*)NULL, 0);
        Tcl_DStringSetLength(&buffer, 0);
        Tcl_DStringAppend(&buffer, (lastval) ? lastval : "", -1);

        token = Tcl_GetString(unparsedObjv[i+1]);
        if (Tcl_SetVar2(interp, varName, (char*)NULL, token,
                TCL_LEAVE_ERR_MSG) == NULL) {

            char msg[256];
            sprintf(msg,
	        "\n    (error in configuration of public variable \"%.100s\")",
	            Tcl_GetString(ivPtr->fullNamePtr));
            Tcl_AddErrorInfo(interp, msg);
            result = TCL_ERROR;
            goto configureDone;
        }

        /*
         *  If this variable has some "config" code, invoke it now.
         *
         *  TRICKY NOTE:  Be careful to evaluate the code one level
         *    up in the call stack, so that it's executed in the
         *    calling context, and not in the context that we've
         *    set up for public variable access.
         */
        mcode = ivPtr->codePtr;
        if (mcode && Itcl_IsMemberCodeImplemented(mcode)) {
	    if (!ivPtr->iclsPtr->infoPtr->useOldResolvers) {
                Itcl_SetCallFrameResolver(interp, contextIoPtr->resolvePtr);
            }
	    saveNsPtr = Tcl_GetCurrentNamespace(interp);
	    Itcl_SetCallFrameNamespace(interp, ivPtr->iclsPtr->nsPtr);
	    result = Tcl_EvalObjEx(interp, mcode->bodyPtr, 0);
	    Itcl_SetCallFrameNamespace(interp, saveNsPtr);
            if (result == TCL_OK) {
                Tcl_ResetResult(interp);
            } else {
                char msg[256];
                sprintf(msg,
		"\n    (error in configuration of public variable \"%.100s\")",
		        Tcl_GetString(ivPtr->fullNamePtr));
                Tcl_AddErrorInfo(interp, msg);

                Tcl_SetVar2(interp, varName,(char*)NULL,
                    Tcl_DStringValue(&buffer), 0);

                goto configureDone;
            }
        }
    }

configureDone:
    if (infoPtr->unparsedObjc > 0) {
        for(i=0;i<infoPtr->unparsedObjc;i++) {
            Tcl_DecrRefCount(infoPtr->unparsedObjv[i]);
        }
        ckfree ((char *)infoPtr->unparsedObjv);
        infoPtr->unparsedObjv = NULL;
        infoPtr->unparsedObjc = 0;
    }
    Tcl_DStringFree(&buffer2);
    Tcl_DStringFree(&buffer);

    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiCgetCmd()
 *
 *  Invoked whenever the user issues the "cget" method for an object.
 *  Handles the following syntax:
 *
 *    <objName> cget -<option>
 *
 *  Allows access to public variables as if they were configuration
 *  options.  Mimics the behavior of the usual "cget" method for
 *  Tk widgets.  Returns the current value of the public variable
 *  with name <option>.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiCgetCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    ItclVarLookup *vlookup;
    CONST char *name;
    CONST char *val;
    int result;

    ItclShowArgs(1,"Itcl_BiCgetCmd", objc, objv);
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    if ((contextIoPtr == NULL) || objc != 2) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "improper usage: should be \"object cget -option\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  BE CAREFUL:  work in the virtual scope!
     */
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }

    if (!(contextIclsPtr->flags & ITCL_CLASS)) {
        result = ItclExtendedCget(contextIclsPtr, interp, objc, objv);
        if (result != TCL_CONTINUE) {
            return result;
        }
    }
    name = Tcl_GetString(objv[1]);

    vlookup = NULL;
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, name+1);
    if (hPtr) {
        vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
    }

    if ((vlookup == NULL) || (vlookup->ivPtr->protection != ITCL_PUBLIC)) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "unknown option \"", name, "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    val = Itcl_GetInstanceVar(interp,
            Tcl_GetString(vlookup->ivPtr->namePtr),
            contextIoPtr, vlookup->ivPtr->iclsPtr);

    if (val) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(val, -1));
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("<undefined>", -1));
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  ItclReportPublicOpt()
 *
 *  Returns information about a public variable formatted as a
 *  configuration option:
 *
 *    -<varName> <initVal> <currentVal>
 *
 *  Used by Itcl_BiConfigureCmd() to report configuration options.
 *  Returns a Tcl_Obj containing the information.
 * ------------------------------------------------------------------------
 */
static Tcl_Obj*
ItclReportPublicOpt(
    Tcl_Interp *interp,      /* interpreter containing the object */
    ItclVariable *ivPtr,     /* public variable to be reported */
    ItclObject *contextIoPtr) /* object containing this variable */
{
    CONST char *val;
    ItclClass *iclsPtr;
    Tcl_HashEntry *hPtr;
    ItclVarLookup *vlookup;
    Tcl_DString optName;
    Tcl_Obj *listPtr;
    Tcl_Obj *objPtr;

    listPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);

    /*
     *  Determine how the option name should be reported.
     *  If the simple name can be used to find it in the virtual
     *  data table, then use the simple name.  Otherwise, this
     *  is a shadowed variable; use the full name.
     */
    Tcl_DStringInit(&optName);
    Tcl_DStringAppend(&optName, "-", -1);

    iclsPtr = (ItclClass*)contextIoPtr->iclsPtr;
    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveVars,
            Tcl_GetString(ivPtr->fullNamePtr));
    assert(hPtr != NULL);
    vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
    Tcl_DStringAppend(&optName, vlookup->leastQualName, -1);

    objPtr = Tcl_NewStringObj(Tcl_DStringValue(&optName), -1);
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);
    Tcl_DStringFree(&optName);


    if (ivPtr->init) {
        objPtr = ivPtr->init;
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);

    val = Itcl_GetInstanceVar(interp, Tcl_GetString(ivPtr->namePtr),
            contextIoPtr, ivPtr->iclsPtr);

    if (val) {
        objPtr = Tcl_NewStringObj((const char *)val, -1);
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);

    return listPtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclReportOption()
 *
 *  Returns information about an option formatted as a
 *  configuration option:
 *
 *    <optionName> <initVal> <currentVal>
 *
 *  Used by ItclExtendedConfigure() to report configuration options.
 *  Returns a Tcl_Obj containing the information.
 * ------------------------------------------------------------------------
 */
static Tcl_Obj*
ItclReportOption(
    Tcl_Interp *interp,      /* interpreter containing the object */
    ItclOption *ioptPtr,     /* option to be reported */
    ItclObject *contextIoPtr) /* object containing this variable */
{
    CONST char *val;
    Tcl_Obj *listPtr;
    Tcl_Obj *objPtr;

    listPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);

    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, ioptPtr->namePtr);
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr,
            ioptPtr->resourceNamePtr);
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, ioptPtr->classNamePtr);
    if (ioptPtr->defaultValuePtr) {
        objPtr = ioptPtr->defaultValuePtr;
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);
    val = ItclGetInstanceVar(interp, "itcl_options", Tcl_GetString(ioptPtr->namePtr),
            contextIoPtr, ioptPtr->iclsPtr);
    if (val) {
        objPtr = Tcl_NewStringObj((const char *)val, -1);
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);
    return listPtr;
}



/*
 * ------------------------------------------------------------------------
 *  Itcl_BiChainCmd()
 *
 *  Invoked to handle the "chain" command, to access the version of
 *  a method or proc that exists in a base class.  Handles the
 *  following syntax:
 *
 *    chain ?<arg> <arg>...?
 *
 *  Looks up the inheritance hierarchy for another implementation
 *  of the method/proc that is currently executing.  If another
 *  implementation is found, it is invoked with the specified
 *  <arg> arguments.  If it is not found, this command does nothing.
 *  This allows a base class method to be called out in a generic way,
 *  so the code will not have to change if the base class changes.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
NRBiChainCmd(
    ClientData dummy,        /* not used */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int result = TCL_OK;

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    char *cmd;
    char *cmd1;
    char *head;
    ItclClass *iclsPtr;
    ItclHierIter hier;
    Tcl_HashEntry *hPtr;
    ItclMemberFunc *imPtr;
    Tcl_DString buffer;
    Tcl_Obj *cmdlinePtr;
    Tcl_Obj **newobjv;

    ItclShowArgs(1, "Itcl_BiChainCmd", objc, objv);
    Tcl_Obj * const *cObjv;
    int freeCmd;
    int idx;

    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "cannot chain functions outside of a class context",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Try to get the command name from the current call frame.
     *  If it cannot be determined, do nothing.  Otherwise, trim
     *  off any leading path names.
     */
    cObjv = Itcl_GetCallFrameObjv(interp);
    if (cObjv == NULL) {
        return TCL_OK;
    }

    if ((Itcl_GetCallFrameClientData(interp) == NULL) || (objc == 1)) {
        /* that has been a direct call, so no object in front !! */
	idx = 0;
    } else {
	idx = 1;
    }
    cmd1 = (char *)ckalloc(strlen(Tcl_GetString(cObjv[idx]))+1);
    freeCmd = 1;
    strcpy(cmd1, Tcl_GetString(cObjv[idx]));
    Itcl_ParseNamespPath(cmd1, &buffer, &head, &cmd);
    if (strcmp(cmd, "___constructor_init") == 0) {
	ckfree(cmd1);
	freeCmd = 0;
        cmd = "constructor";
    }

    /*
     *  Look for the specified command in one of the base classes.
     *  If we have an object context, then start from the most-specific
     *  class and walk up the hierarchy to the current context.  If
     *  there is multiple inheritance, having the entire inheritance
     *  hierarchy will allow us to jump over to another branch of
     *  the inheritance tree.
     *
     *  If there is no object context, just start with the current
     *  class context.
     */
    if (contextIoPtr != NULL) {
        Itcl_InitHierIter(&hier, contextIoPtr->iclsPtr);
        while ((iclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
            if (iclsPtr == contextIclsPtr) {
                break;
            }
        }
    } else {
        Itcl_InitHierIter(&hier, contextIclsPtr);
        Itcl_AdvanceHierIter(&hier);    /* skip the current class */
    }

    /*
     *  Now search up the class hierarchy for the next implementation.
     *  If found, execute it.  Otherwise, do nothing.
     */
    Tcl_Obj *objPtr;
    objPtr = Tcl_NewStringObj(cmd, -1);
    if (freeCmd) {
        ckfree(cmd1);
    }
    Tcl_IncrRefCount(objPtr);
    while ((iclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
        hPtr = Tcl_FindHashEntry(&iclsPtr->functions, (char *)objPtr);
        if (hPtr) {
            imPtr = (ItclMemberFunc*)Tcl_GetHashValue(hPtr);

            /*
             *  NOTE:  Avoid the usual "virtual" behavior of
             *         methods by passing the full name as
             *         the command argument.
             */
            cmdlinePtr = Itcl_CreateArgs(interp, Tcl_GetString(imPtr->fullNamePtr),
                objc-1, objv+1);

	    int my_objc;
            (void) Tcl_ListObjGetElements((Tcl_Interp*)NULL, cmdlinePtr,
                &my_objc, &newobjv);

	    if (imPtr->flags & ITCL_CONSTRUCTOR) {
		Tcl_DecrRefCount(newobjv[0]);
		newobjv[0] = Tcl_NewStringObj(Tcl_GetCommandName(interp,
                        contextIclsPtr->infoPtr->currIoPtr->accessCmd), -1);
		Tcl_IncrRefCount(newobjv[0]);
		contextIoPtr = imPtr->iclsPtr->infoPtr->currIoPtr;
            }
//ItclShowArgs(0, "___chain", objc-1, newobjv+1);
            result = Itcl_EvalMemberCode(interp, imPtr, contextIoPtr,
	            my_objc-1, newobjv+1);
            Tcl_DecrRefCount(cmdlinePtr);
            break;
        }
    }
    Tcl_DecrRefCount(objPtr);

    Tcl_DStringFree(&buffer);
    Itcl_DeleteHierIter(&hier);
    return result;
}
/* ARGSUSED */
int
Itcl_BiChainCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    return Itcl_NRCallObjProc(clientData, interp, NRBiChainCmd, objc, objv);
}


/*
 * ------------------------------------------------------------------------
 *  ItclBiObjectUnknownCmd()
 *
 *  Invoked to handle the "objectunknown" command
 *  this is called whenever an object is called with an unknown method/proc
 *  following syntax:
 *
 *    unkownobject <object> <methodname> ?<arg> <arg>...?
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
ItclBiObjectUnknownCmd(
    ClientData clientData,   /* ItclObjectInfo Ptr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    FOREACH_HASH_DECLS;
    Tcl_Object oPtr;
    Tcl_Command cmd;
    Tcl_CmdInfo cmdInfo;
    Tcl_Obj **newObjv;
    ItclObject *ioPtr;
    ItclObjectInfo *infoPtr;
    ItclComponent *icPtr;
    const char *val;
    int result;

    ItclShowArgs(1, "ItclBiObjectUnknownCmd", objc, objv);
    cmd = Tcl_GetCommandFromObj(interp, objv[1]);
    if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) != 1) {
    }
    oPtr = cmdInfo.objClientData;
    infoPtr = (ItclObjectInfo *)clientData;
    ioPtr = (ItclObject *)Tcl_ObjectGetMetadata(oPtr,
            infoPtr->object_meta_type);
    FOREACH_HASH_VALUE(icPtr, &ioPtr->objectComponents) {
        if (icPtr->flags & ITCL_COMPONENT_INHERIT) {
	    val = Itcl_GetInstanceVar(interp,
	            Tcl_GetString(icPtr->namePtr), ioPtr,
		    icPtr->ivPtr->iclsPtr);
	    if (val != NULL) {
                newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *) * (objc -1));
		newObjv[0] = Tcl_NewStringObj(val, -1);
		Tcl_IncrRefCount(newObjv[0]);
		memcpy(newObjv+1, objv+2, sizeof(Tcl_Obj *) * (objc-2));
                result = Tcl_EvalObjv(interp, objc-1, newObjv, 0);
		Tcl_DecrRefCount(newObjv[0]);
	        return result;
	    }
	}
    }
    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "bad option \"", Tcl_GetString(objv[2]), "\": should be one of...",
	    (char*)NULL);
    ItclReportObjectUsage(interp, ioPtr, NULL, NULL);
    return TCL_ERROR;
}

static int
CallCreateObject(
    ClientData data[],
    Tcl_Interp *interp,
    int result)
{
    ItclClass *iclsPtr = data[0];
    int objc = PTR2INT(data[1]);
    Tcl_Obj *const *objv = data[2];

    if (result != TCL_OK) {
        return result;
    }
    result = Itcl_HandleClass(iclsPtr->infoPtr, interp, objc, objv);
    Tcl_DecrRefCount(objv[2]);
    Tcl_DecrRefCount(objv[1]);
    Tcl_DecrRefCount(objv[0]);
    return result;
}
/*
 * ------------------------------------------------------------------------
 *  ItclBiClassUnknownCmd()
 *
 *  Invoked to handle the "classunknown" command
 *  this is called whenever an object is called with an unknown method/proc
 *  following syntax:
 *
 *    classunknown <object> <methodname> ?<arg> <arg>...?
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
ItclBiClassUnknownCmd(
    ClientData clientData,   /* ItclObjectInfo Ptr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    FOREACH_HASH_DECLS;
    Tcl_Obj **newObjv;
    Tcl_Obj **lObjv;
    Tcl_Obj *listPtr;
    ItclClass *iclsPtr;
    ItclObjectInfo *infoPtr;
    ItclComponent *icPtr;
    ItclDelegatedFunction *idmPtr;
    const char *val;
    const char *funcName;
    int lObjc;
    int result;
    int offset;
    int useComponent;
    int idx;

    ItclShowArgs(1, "ItclBiClassUnknownCmd", objc, objv);
    listPtr = NULL;
    useComponent = 1;
    infoPtr = (ItclObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses,
            (char *)Tcl_GetCurrentNamespace(interp));
    if (hPtr == NULL) {
fprintf(stderr, "ItclBiClassUnknownCmd cannot find class\n");
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    FOREACH_HASH_VALUE(icPtr, &iclsPtr->components) {
        if (icPtr->flags & ITCL_COMPONENT_INHERIT) {
	    val = Tcl_GetVar2(interp, Tcl_GetString(icPtr->namePtr), NULL, 0);
	    if (val != NULL) {
                newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *) * (objc));
		newObjv[0] = Tcl_NewStringObj(val, -1);
		Tcl_IncrRefCount(newObjv[0]);
		memcpy(newObjv+1, objv+1, sizeof(Tcl_Obj *) * (objc-1));
                result = Tcl_EvalObjv(interp, objc, newObjv, 0);
		Tcl_DecrRefCount(newObjv[0]);
	        return result;
	    }
	}
    }
    funcName = Tcl_GetString(objv[1]);
    FOREACH_HASH_VALUE(idmPtr, &iclsPtr->delegatedFunctions) {
        if ((strcmp(Tcl_GetString(idmPtr->namePtr), funcName) == 0) ||
	        (strcmp(Tcl_GetString(idmPtr->namePtr), "*") == 0)) {
	    val = NULL;
	    if (idmPtr->icPtr != NULL) {
	        val = Tcl_GetVar2(interp, Tcl_GetString(idmPtr->icPtr->namePtr),
	                NULL, 0);
	        if (val == NULL) {
fprintf(stderr, "contents of component == NULL\n");
	            return TCL_ERROR;
	        }
	    }
	    offset = 1;
	    if (strcmp(Tcl_GetString(idmPtr->namePtr), "*") == 0) {
	        hPtr = Tcl_FindHashEntry(&idmPtr->exceptions, (char *)objv[1]);
		/* we have no method name in that case in the caller */
		if (hPtr != NULL) {
		    Tcl_AppendResult(interp, "unknown subcommand \"",
		            funcName, "\": must be ", NULL);
	            return TCL_ERROR;
		}
                Tcl_IncrRefCount(idmPtr->namePtr);
                /* and now for the argument */
                Tcl_IncrRefCount(idmPtr->namePtr);
	    }
	    lObjc = 0;
	    if ((idmPtr->asPtr != NULL) || (idmPtr->usingPtr != NULL)) {
		offset++;
                listPtr = Tcl_NewListObj(0, NULL);
                result = ExpandDelegateAs(interp, NULL, iclsPtr,
			idmPtr, funcName, listPtr);
                if (result != TCL_OK) {
                    return result;
                }
	        result = Tcl_ListObjGetElements(interp, listPtr,
		        &lObjc, &lObjv);
	        if (result != TCL_OK) {
		    return result;
		}
		if (idmPtr->usingPtr != NULL) {
                    useComponent = 0;
		}
	    }
//fprintf(stderr, "OBJC!%d!%d!%d!%d!%d!\n", objc, lObjc, offset, useComponent, (objc + lObjc - offset));
            newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *) *
	            (objc + lObjc - offset + useComponent));
	    if (useComponent) {
		if ((val == NULL) || (strlen(val) == 0)) {
		    Tcl_AppendResult(interp, "component \"", 
			    Tcl_GetString(idmPtr->icPtr->namePtr),
			    "\" is not initialized", NULL);
		    return TCL_ERROR;
		}
	        newObjv[0] = Tcl_NewStringObj(val, -1);
	        Tcl_IncrRefCount(newObjv[0]);
	    }
	    for (idx = 0; idx < lObjc; idx++) {
		newObjv[useComponent+idx] = lObjv[idx];
	        Tcl_IncrRefCount(newObjv[useComponent+idx]);
	    }
	    if (objc-offset > 0) {
                memcpy(newObjv+useComponent+lObjc, objv+offset,
	                sizeof(Tcl_Obj *) * (objc-offset));
            }
ItclShowArgs(1, "EVAL", objc+lObjc-offset+useComponent, newObjv);
            result = Tcl_EvalObjv(interp, objc+lObjc-offset+useComponent,
	            newObjv, 0);
	    if (useComponent) {
	        Tcl_DecrRefCount(newObjv[0]);
	    }
	    for (idx = 0; idx < lObjc; idx++) {
	        Tcl_DecrRefCount(newObjv[useComponent+idx]);
	    }
            /* FIXME need to free listPtr ?? */
	    if (result == TCL_ERROR) {
	        const char *resStr;
		Tcl_Obj *resPtr = Tcl_NewStringObj("", -1);
		resStr = Tcl_GetStringResult(interp);
		/* FIXME ugly hack at the moment !! */
		if (strncmp(resStr, "wrong # args: should be ", 24) == 0) {
		   Tcl_AppendToObj(resPtr, resStr, 25);
                   resStr += 25;
		   Tcl_AppendToObj(resPtr, Tcl_GetString(iclsPtr->namePtr), -1);
                   resStr += strlen(val);
		   Tcl_AppendToObj(resPtr, resStr, -1);
		   Tcl_ResetResult(interp);
		   Tcl_SetObjResult(interp, resPtr);
		}
	    }
	    return result;
	}
    }
    offset = 1;
    if (strcmp(Tcl_GetString(objv[1]), "create") == 0) {
	/* allow typeClassName create objectName */
        offset++;
    } else {
	/* allow typeClassName objectName */
    }
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *) * (objc+3-offset));
    newObjv[0] = objv[0];
    Tcl_IncrRefCount(newObjv[0]);
    newObjv[1] = iclsPtr->namePtr;
    Tcl_IncrRefCount(newObjv[1]);
    newObjv[2] = Tcl_NewStringObj(iclsPtr->nsPtr->fullName, -1);
    Tcl_IncrRefCount(newObjv[2]);
    memcpy(newObjv+3, objv+offset, (objc-offset) * sizeof(Tcl_Obj *));
        void *callbackPtr;
        callbackPtr = Itcl_GetCurrentCallbackPtr(interp);
    Itcl_NRAddCallback(interp, CallCreateObject, iclsPtr,
            INT2PTR(objc+3-offset), (ClientData)newObjv, NULL);
    return Itcl_NRRunCallbacks(interp, callbackPtr);
}

/*
 * ------------------------------------------------------------------------
 *  ItclExtendedConfigure()
 *
 *  Invoked whenever the user issues the "configure" method for an object.
 *  If the class is not ITCL_CLASS
 *  Handles the following syntax:
 *
 *    <objName> configure ?-<option>? ?<value> -<option> <value>...?
 *
 *  Allows access to public variables as if they were configuration
 *  options.  With no arguments, this command returns the current
 *  list of public variable options.  If -<option> is specified,
 *  this returns the information for just one option:
 *
 *    -<optionName> <initVal> <currentVal>
 *
 *  Otherwise, the list of arguments is parsed, and values are
 *  assigned to the various public variable options.  When each
 *  option changes, a big of "config" code associated with the option
 *  is executed, to bring the object up to date.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
ItclExtendedConfigure(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    Tcl_HashEntry *hPtr2;
    Tcl_Object oPtr;
    Tcl_Obj *resultPtr;
    Tcl_Obj *objPtr;
    Tcl_Obj *methodNamePtr;
    Tcl_Obj *configureMethodPtr;
    Tcl_Namespace *saveNsPtr;
    Tcl_Namespace *evalNsPtr;
    Tcl_Obj **newObjv;
    ItclVarLookup *vlookup;
    ItclDelegatedFunction *idmPtr;
    ItclDelegatedOption *idoPtr;
    ItclObject *ioPtr;
    ItclComponent *icPtr;
    ItclOption *ioptPtr;
    ItclObjectInfo *infoPtr;
    const char *val;
    char *token;
    int i;
    int result;

    ItclShowArgs(1, "ItclExtendedConfigure", objc, objv);
    vlookup = NULL;
    token = NULL;
    ioptPtr = NULL;
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    if (contextIoPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "improper usage: should be ",
            "\"object configure ?-option? ?value -option value...?\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  BE CAREFUL:  work in the virtual scope!
     */
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }
    infoPtr = contextIclsPtr->infoPtr;
    if (infoPtr->currContextIclsPtr != NULL) {
        contextIclsPtr = infoPtr->currContextIclsPtr;
    }

    hPtr = NULL;
    /* first check if method configure is delegated */
    methodNamePtr = Tcl_NewStringObj("*", -1);
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedFunctions, (char *)
            methodNamePtr);
    if (hPtr != NULL) {
        idmPtr = (ItclDelegatedFunction *)Tcl_GetHashValue(hPtr);
	Tcl_SetStringObj(methodNamePtr, "configure", -1);
        hPtr = Tcl_FindHashEntry(&idmPtr->exceptions, (char *)methodNamePtr);
        if (hPtr == NULL) {
	    icPtr = idmPtr->icPtr;
	    val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
	            NULL, contextIoPtr, contextIclsPtr);
            if (val != NULL) {
	        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+2));
	        newObjv[0] = Tcl_NewStringObj(val, -1);
	        Tcl_IncrRefCount(newObjv[0]);
	        newObjv[1] = Tcl_NewStringObj("configure", 9);
	        Tcl_IncrRefCount(newObjv[1]);
	        for(i=2;i<objc;i++) {
	            newObjv[i+1] = objv[i];
                }
		objPtr = Tcl_NewStringObj(val, -1);
	        Tcl_IncrRefCount(objPtr);
	        oPtr = Tcl_GetObjectFromObj(interp, objPtr);
	        if (oPtr != NULL) {
                    ioPtr = (ItclObject *)Tcl_ObjectGetMetadata(oPtr,
                            infoPtr->object_meta_type);
	            infoPtr->currContextIclsPtr = ioPtr->iclsPtr;
	        }
                result = Tcl_EvalObjv(interp, objc, newObjv, TCL_EVAL_DIRECT);
                Tcl_DecrRefCount(newObjv[0]);
                Tcl_DecrRefCount(newObjv[1]);
                ckfree((char *)newObjv);
	        Tcl_DecrRefCount(objPtr);
	        if (oPtr != NULL) {
	            infoPtr->currContextIclsPtr = NULL;
	        }
                return result;
	    }
	}
    }
    /* now do the hard work */
    if (objc == 1) {
fprintf(stderr, "plain configure not yet implemented\n");
        return TCL_ERROR;
    }
    /* first handle delegated options */
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectDelegatedOptions, (char *)
            objv[1]);
    if (hPtr == NULL) {
	Tcl_Obj *objPtr;
	objPtr = Tcl_NewStringObj("*",1);
	Tcl_IncrRefCount(objPtr);
        /* check if all options are delegated */
        hPtr = Tcl_FindHashEntry(&contextIoPtr->objectDelegatedOptions, (char *)
                objPtr);
	Tcl_DecrRefCount(objPtr);
        if (hPtr != NULL) {
	    /* now check the exceptions */
            idoPtr = (ItclDelegatedOption *)Tcl_GetHashValue(hPtr);
	    hPtr2 = Tcl_FindHashEntry(&idoPtr->exceptions, (char *)objv[1]);
	    if (hPtr2 != NULL) {
		/* found in exceptions, so no delegation for this option */
	        hPtr = NULL;
	    }
        }
    }
    if (hPtr != NULL) {
	/* the option is delegated */
        idoPtr = (ItclDelegatedOption *)Tcl_GetHashValue(hPtr);
        icPtr = idoPtr->icPtr;
        val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
                NULL, contextIoPtr, icPtr->ivPtr->iclsPtr);
        if (val != NULL) {
	    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+2));
	    newObjv[0] = Tcl_NewStringObj(val, -1);
	    Tcl_IncrRefCount(newObjv[0]);
	    newObjv[1] = Tcl_NewStringObj("configure", 9);
	    Tcl_IncrRefCount(newObjv[1]);
	    for(i=1;i<objc;i++) {
	        newObjv[i+1] = objv[i];
            }
	    objPtr = Tcl_NewStringObj(val, -1);
	    Tcl_IncrRefCount(objPtr);
	    oPtr = Tcl_GetObjectFromObj(interp, objPtr);
	    if (oPtr != NULL) {
                ioPtr = (ItclObject *)Tcl_ObjectGetMetadata(oPtr,
                        infoPtr->object_meta_type);
	        infoPtr->currContextIclsPtr = ioPtr->iclsPtr;
	    }
            ItclShowArgs(1, "delegated eval", objc+1, newObjv);
            result = Tcl_EvalObjv(interp, objc+1, newObjv, TCL_EVAL_DIRECT);
            Tcl_DecrRefCount(newObjv[0]);
            Tcl_DecrRefCount(newObjv[1]);
            ckfree((char *)newObjv);
	    Tcl_DecrRefCount(objPtr);
	    if (oPtr != NULL) {
	        infoPtr->currContextIclsPtr = NULL;
	    }
            return result;
        }
    }

    if (objc == 2) {
        /* now look if it is an option at all */
        hPtr = Tcl_FindHashEntry(&contextIoPtr->objectOptions,
	        (char *) objv[1]);
        if (hPtr == NULL) {
	    /* no option at all, let the normal configure do the job */
	    return TCL_CONTINUE;
        }
        ioptPtr = (ItclOption *)Tcl_GetHashValue(hPtr);
        resultPtr = ItclReportOption(interp, ioptPtr, contextIoPtr);
        Tcl_SetObjResult(interp, resultPtr);
        return TCL_OK;
    }
    result = TCL_OK;
    /* set one or more options */
    for (i=1; i < objc; i+=2) {
	if (i+1 >= objc) {
	    Tcl_AppendResult(interp, "need option value pair", NULL);
	    result = TCL_ERROR;
	    break;
	}
        hPtr = Tcl_FindHashEntry(&contextIoPtr->objectOptions,
	        (char *) objv[i]);
        if (hPtr == NULL) {
	    infoPtr->unparsedObjc += 2;
	    if (infoPtr->unparsedObjv == NULL) {
	        infoPtr->unparsedObjc++; /* keep the first slot for 
		                            correct working !! */
	        infoPtr->unparsedObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)
	                *(infoPtr->unparsedObjc));
	        infoPtr->unparsedObjv[0] = objv[0];
	        Tcl_IncrRefCount(infoPtr->unparsedObjv[0]);
	    } else {
	        infoPtr->unparsedObjv = (Tcl_Obj **)ckrealloc(
	                (char *)infoPtr->unparsedObjv, sizeof(Tcl_Obj *)
	                *(infoPtr->unparsedObjc));
	    }
	    infoPtr->unparsedObjv[infoPtr->unparsedObjc-2] = objv[i];
	    Tcl_IncrRefCount(infoPtr->unparsedObjv[infoPtr->unparsedObjc-2]);
	    infoPtr->unparsedObjv[infoPtr->unparsedObjc-1] = objv[i+1];
	    Tcl_IncrRefCount(infoPtr->unparsedObjv[infoPtr->unparsedObjc-1]);
	    /* check if normal public variable/common ? */
	    /* FIXME !!! temporary */
	    continue;
        }
        ioptPtr = (ItclOption *)Tcl_GetHashValue(hPtr);
        if (ioptPtr->validateMethodPtr != NULL) {
	    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*3);
	    newObjv[0] = ioptPtr->validateMethodPtr;
	    newObjv[1] = objv[i];
	    newObjv[2] = objv[i+1];
            result = Tcl_EvalObjv(interp, 3, newObjv, TCL_EVAL_DIRECT);
            ckfree((char *)newObjv);
	    if (result != TCL_OK) {
	        break;
	    }
	}
	configureMethodPtr = NULL;
	evalNsPtr = NULL;
	if (ioptPtr->configureMethodPtr != NULL) {
	    configureMethodPtr = ioptPtr->configureMethodPtr;
	    Tcl_IncrRefCount(configureMethodPtr);
	    evalNsPtr = ioptPtr->iclsPtr->nsPtr;
	}
	if (ioptPtr->configureMethodVarPtr != NULL) {
	    val = ItclGetInstanceVar(interp,
	            Tcl_GetString(ioptPtr->configureMethodVarPtr), NULL,
		    contextIoPtr, ioptPtr->iclsPtr);
	    if (val == NULL) {
	        Tcl_AppendResult(interp, "configure cannot get value for",
		        " configuremethodvar \"",
			Tcl_GetString(ioptPtr->configureMethodVarPtr),
			"\"", NULL);
		return TCL_ERROR;
	    }
	    hPtr = Tcl_FindHashEntry(&contextIoPtr->iclsPtr->resolveCmds,
	        (char *)val);
            if (hPtr != NULL) {
		ItclMemberFunc *imPtr;
		ItclCmdLookup *clookup;
		clookup = (ItclCmdLookup *)Tcl_GetHashValue(hPtr);
		imPtr = clookup->imPtr;
	        evalNsPtr = imPtr->iclsPtr->nsPtr;
	    } else {
		Tcl_AppendResult(interp, "cannot find method \"",
		        val, "\" found in configuremethodvar", NULL);
		return TCL_ERROR;
	    }
	    configureMethodPtr = Tcl_NewStringObj(val, -1);
	    Tcl_IncrRefCount(configureMethodPtr);
	}
        if (configureMethodPtr != NULL) {
	    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*3);
	    newObjv[0] = configureMethodPtr;
	    Tcl_IncrRefCount(newObjv[0]);
	    newObjv[1] = objv[i];
	    Tcl_IncrRefCount(newObjv[1]);
	    newObjv[2] = objv[i+1];
	    Tcl_IncrRefCount(newObjv[2]);
	    saveNsPtr = Tcl_GetCurrentNamespace(interp);
	    Itcl_SetCallFrameNamespace(interp, evalNsPtr);
            result = Tcl_EvalObjv(interp, 3, newObjv, TCL_EVAL_DIRECT);
	    Tcl_DecrRefCount(newObjv[0]);
	    Tcl_DecrRefCount(newObjv[1]);
	    Tcl_DecrRefCount(newObjv[2]);
            ckfree((char *)newObjv);
	    Itcl_SetCallFrameNamespace(interp, saveNsPtr);
	    Tcl_DecrRefCount(configureMethodPtr);
	    if (result != TCL_OK) {
	        break;
	    }
	} else {
	    if (ItclSetInstanceVar(interp, "itcl_options",
	            Tcl_GetString(objv[i]),
	            Tcl_GetString(objv[i+1]), contextIoPtr, ioptPtr->iclsPtr)
		    == NULL) {
		result = TCL_ERROR;
fprintf(stderr, "BRK3!%s!\n", Tcl_GetStringResult(interp));
	        break;
	    }
	}
        result = TCL_OK;
    }
    if (infoPtr->unparsedObjc > 0) {
	if (result == TCL_OK) {
            return TCL_CONTINUE;
        }
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclExtendedCget()
 *
 *  Invoked whenever the user issues the "cget" method for an object.
 *  If the class is NOT ITCL_CLASS
 *  Handles the following syntax:
 *
 *    <objName> cget -<option>
 *
 *  Allows access to public variables as if they were configuration
 *  options.  Mimics the behavior of the usual "cget" method for
 *  Tk widgets.  Returns the current value of the public variable
 *  with name <option>.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
ItclExtendedCget(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    Tcl_Obj *objPtr;
    Tcl_Object oPtr;
    Tcl_Obj *methodNamePtr;
    Tcl_Obj **newObjv;
    ItclDelegatedFunction *idmPtr;
    ItclDelegatedOption *idoPtr;
    ItclComponent *icPtr;
    ItclObjectInfo *infoPtr;
    ItclOption *ioptPtr;
    ItclObject *ioPtr;
    const char *val;
    int i;
    int result;

    ItclShowArgs(1,"ItclExtendedCget", objc, objv);
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    if ((contextIoPtr == NULL) || objc != 2) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "improper usage: should be \"object cget -option\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  BE CAREFUL:  work in the virtual scope!
     */
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }
    infoPtr = contextIclsPtr->infoPtr;
    if (infoPtr->currContextIclsPtr != NULL) {
        contextIclsPtr = infoPtr->currContextIclsPtr;
    }

    hPtr = NULL;
    /* first check if method cget is delegated */
    methodNamePtr = Tcl_NewStringObj("*", -1);
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedFunctions, (char *)
            methodNamePtr);
    if (hPtr != NULL) {
        idmPtr = (ItclDelegatedFunction *)Tcl_GetHashValue(hPtr);
	Tcl_SetStringObj(methodNamePtr, "cget", -1);
        hPtr = Tcl_FindHashEntry(&idmPtr->exceptions, (char *)methodNamePtr);
        if (hPtr == NULL) {
	    icPtr = idmPtr->icPtr;
	    val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
	            NULL, contextIoPtr, contextIclsPtr);
            if (val != NULL) {
	        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+1));
	        newObjv[0] = Tcl_NewStringObj(val, -1);
	        Tcl_IncrRefCount(newObjv[0]);
	        newObjv[1] = Tcl_NewStringObj("cget", 4);
	        Tcl_IncrRefCount(newObjv[1]);
		for(i=1;i<objc;i++) {
		    newObjv[i+1] = objv[i];
		}
		objPtr = Tcl_NewStringObj(val, -1);
	        Tcl_IncrRefCount(objPtr);
	        oPtr = Tcl_GetObjectFromObj(interp, objPtr);
	        if (oPtr != NULL) {
                    ioPtr = (ItclObject *)Tcl_ObjectGetMetadata(oPtr,
                            infoPtr->object_meta_type);
	            infoPtr->currContextIclsPtr = ioPtr->iclsPtr;
	        }
		ItclShowArgs(1, "DELEGATED EVAL", objc+1, newObjv);
                result = Tcl_EvalObjv(interp, objc+1, newObjv, TCL_EVAL_DIRECT);
	        Tcl_DecrRefCount(newObjv[0]);
	        Tcl_DecrRefCount(newObjv[1]);
	        Tcl_DecrRefCount(objPtr);
	        if (oPtr != NULL) {
	            infoPtr->currContextIclsPtr = NULL;
	        }
                return result;
	    }
	}
    }
    if (objc == 1) {
        Tcl_WrongNumArgs(interp, 1, objv, "option");
        return TCL_ERROR;
    }
    /* now do the hard work */
    /* first handle delegated options */
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectDelegatedOptions, (char *)
            objv[1]);
    if (hPtr != NULL) {
	/* the option is delegated */
        idoPtr = (ItclDelegatedOption *)Tcl_GetHashValue(hPtr);
        icPtr = idoPtr->icPtr;
        val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
                NULL, contextIoPtr, icPtr->ivPtr->iclsPtr);
        if (val != NULL) {
	    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+1));
	    newObjv[0] = Tcl_NewStringObj(val, -1);
	    Tcl_IncrRefCount(newObjv[0]);
	    newObjv[1] = Tcl_NewStringObj("cget", 4);
	    Tcl_IncrRefCount(newObjv[1]);
	    for(i=1;i<objc;i++) {
	        newObjv[i+1] = objv[i];
	    }
	    objPtr = Tcl_NewStringObj(val, -1);
	    Tcl_IncrRefCount(objPtr);
	    oPtr = Tcl_GetObjectFromObj(interp, objPtr);
	    if (oPtr != NULL) {
                ioPtr = (ItclObject *)Tcl_ObjectGetMetadata(oPtr,
                        infoPtr->object_meta_type);
	        infoPtr->currContextIclsPtr = ioPtr->iclsPtr;
	    }
            result = Tcl_EvalObjv(interp, objc+1, newObjv, TCL_EVAL_DIRECT);
	    Tcl_DecrRefCount(newObjv[0]);
	    Tcl_DecrRefCount(newObjv[1]);
	    Tcl_DecrRefCount(objPtr);
	    if (oPtr != NULL) {
	        infoPtr->currContextIclsPtr = NULL;
	    }
            return result;
        } else {
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "component \"",
	            Tcl_GetString(icPtr->namePtr),
	            "\" is not set, needed for option \"",
		    Tcl_GetString(objv[1]),
	            "\"", NULL);
	    return TCL_ERROR;
	}
    }

    /* now look if it is an option at all */
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectOptions, (char *) objv[1]);
    if (hPtr == NULL) {
	/* no option at all, let the normal configure do the job */
	return TCL_CONTINUE;
    }
    ioptPtr = (ItclOption *)Tcl_GetHashValue(hPtr);
    result = TCL_CONTINUE;
    if (ioptPtr->cgetMethodPtr != NULL) {
        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*2);
        newObjv[0] = ioptPtr->cgetMethodPtr;
        newObjv[1] = objv[1];
        result = Tcl_EvalObjv(interp, objc, newObjv, TCL_EVAL_DIRECT);
    } else {
        val = ItclGetInstanceVar(interp, "itcl_options",
                Tcl_GetString(ioptPtr->namePtr),
		contextIoPtr, ioptPtr->iclsPtr);
        if (val) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(val, -1));
        } else {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("<undefined>", -1));
        }
        result = TCL_OK;
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclExtendedSetGet()
 *
 *  Invoked whenever the user writes to a methodvariable or calls the method
 *  with the same name as the variable.
 *  only for not ITCL_CLASS classes
 *  Handles the following syntax:
 *
 *    <objName> setget varName ?<value>?
 *
 *  Allows access to methodvariables as if they hat a setter and getter
 *  method
 *  With no arguments, this command returns the current
 *  value of the variable.  If <value> is specified,
 *  this sets the variable to the value calling a callback if exists:
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
ItclExtendedSetGet(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    Tcl_Obj **newObjv;
    ItclMethodVariable *imvPtr;
    ItclObjectInfo *infoPtr;
    const char *usageStr;
    const char *val;
    char *token;
    int result;
    int setValue;

    ItclShowArgs(1, "ItclExtendedSetGet", objc, objv);
    token = NULL;
    imvPtr = NULL;
    result = TCL_OK;
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    usageStr = "improper usage: should be \"object setget varName ?value?\"";
    if (contextIoPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                usageStr, (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  BE CAREFUL:  work in the virtual scope!
     */
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }
    infoPtr = contextIclsPtr->infoPtr;
    if (infoPtr->currContextIclsPtr != NULL) {
        contextIclsPtr = infoPtr->currContextIclsPtr;
    }

    hPtr = NULL;
    if (objc < 2) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                usageStr, (char*)NULL);
        return TCL_ERROR;
    }
    /* look if it is an methodvariable at all */
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectMethodVariables,
            (char *) objv[1]);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "no such methodvariable \"",
	        Tcl_GetString(objv[1]), "\"", NULL);
	return TCL_ERROR;
    }
    imvPtr = (ItclMethodVariable *)Tcl_GetHashValue(hPtr);
    if (objc == 2) {
        val = ItclGetInstanceVar(interp, Tcl_GetString(objv[1]), NULL, 
	        contextIoPtr, imvPtr->iclsPtr);
        if (val == NULL) {
            result = TCL_ERROR;
        } else {
	   Tcl_SetResult(interp, (char *)val, TCL_VOLATILE);
	}
        return result;
    }
    imvPtr = (ItclMethodVariable *)Tcl_GetHashValue(hPtr);
    result = TCL_OK;
    setValue = 1;
    if (imvPtr->callbackPtr != NULL) {
        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*3);
        newObjv[0] = imvPtr->callbackPtr;
        Tcl_IncrRefCount(newObjv[0]);
        newObjv[1] = objv[1];
        Tcl_IncrRefCount(newObjv[1]);
        newObjv[2] = objv[2];
        Tcl_IncrRefCount(newObjv[2]);
        result = Tcl_EvalObjv(interp, 3, newObjv, TCL_EVAL_DIRECT);
        Tcl_DecrRefCount(newObjv[0]);
        Tcl_DecrRefCount(newObjv[1]);
        Tcl_DecrRefCount(newObjv[2]);
        ckfree((char *)newObjv);
    }
    if (result == TCL_OK) {
        Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &setValue);
	/* if setValue != 0 set the new value of the variable here */
	if (setValue) {
            if (ItclSetInstanceVar(interp, Tcl_GetString(objv[1]), NULL, 
	            Tcl_GetString(objv[2]), contextIoPtr,
		    imvPtr->iclsPtr) == NULL) {
                result = TCL_ERROR;
            }
        }
    }
    return result;
}
/*
 * ------------------------------------------------------------------------
 *  Itcl_BiComponentInstallCmd()
 *
 *  Invoked whenever the user issues the "componentinstall" method for an
 *  object.
 *  Handles the following syntax:
 *
 *    componentinstall <componentName> using <widgetClassName> <widgetPathName>
 *      ?-option value -option value ...?
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiComponentInstallCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_HashEntry *hPtr;
    char *token;

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    ItclShowArgs(1, "Itcl_BiComponentInstallCmd", objc, objv);
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    if (contextIoPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "improper usage: should be \"object componentinstall \"",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (objc < 5) {
        token = Tcl_GetString(objv[0]);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"", token, " <componentName> using",
	    " <widgetClassName> <widgetPathName>",
	    " ?-option value -option value ...?\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /* get component name and check, if it exists */
    token = Tcl_GetString(objv[1]);
    if (contextIclsPtr == NULL) {
        Tcl_AppendResult(interp, "cannot find context class for object \"",
	        Tcl_GetCommandName(interp, contextIoPtr->accessCmd), "\"",
		NULL);
        return TCL_ERROR;
    }
    if (!contextIclsPtr->flags & (ITCL_WIDGET|ITCL_WIDGETADAPTOR)) {
        Tcl_AppendResult(interp, "no such method \"componentinstall\"", NULL);
	return TCL_ERROR;
    }
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->components, (char *)objv[1]);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "class \"",
	        Tcl_GetString(contextIclsPtr->namePtr),
	        "\" has no component \"",
		Tcl_GetString(objv[1]), "\"", NULL);
        return TCL_ERROR;
    }
    if (contextIclsPtr->infoPtr->windgetInfoPtr != NULL) {
        if (contextIclsPtr->infoPtr->windgetInfoPtr->componentInst != NULL) {
            if (contextIclsPtr->infoPtr->windgetInfoPtr->componentInst(
	            interp, contextIoPtr, contextIclsPtr,
		    objc, objv) != TCL_OK) {
	        return TCL_ERROR;
	    }
	}
    }
    return TCL_OK;
}
/*
 * ------------------------------------------------------------------------
 *  Itcl_BiDestroyCmd()
 *
 *  Invoked whenever the user issues the "destroy" method for an
 *  object.
 *  Handles the following syntax:
 *
 *    destroy
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiDestroyCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    ItclShowArgs(1, "Itcl_BiDestroyCmd", objc, objv);
    contextIoPtr = NULL;
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    if (objc != 1) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"", Tcl_GetString(objv[0]), (char*)NULL);
        return TCL_ERROR;
    }

    if (contextIclsPtr == NULL) {
        Tcl_AppendResult(interp, "cannot find context class for object \"",
	        Tcl_GetCommandName(interp, contextIoPtr->accessCmd), "\"",
		NULL);
        return TCL_ERROR;
    }
    if (!contextIclsPtr->flags & (ITCL_TYPE|ITCL_WIDGET|ITCL_WIDGETADAPTOR)) {
        Tcl_AppendResult(interp, "no such method \"destroy\"", NULL);
	return TCL_ERROR;
    }
    ItclDestroyClassNamesp(contextIclsPtr);
    return TCL_OK;
}

