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
 *     RCS:  $Id: itclInfo.c,v 1.1.2.7 2007/10/14 17:19:06 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"


typedef struct InfoMethod {
    char* name;              /* method name */
    char* usage;             /* string describing usage */
    Tcl_ObjCmdProc *proc;    /* implementation C proc */
} InfoMethod;

static InfoMethod InfoMethodList[] = {
    { "args", "procname", Itcl_BiInfoArgsCmd },
    { "body", "procname", Itcl_BiInfoBodyCmd },
    { "class", "", Itcl_BiInfoClassCmd },
    { "component",
        "?name? ?-inherit? ?-value?",
        Itcl_BiInfoComponentCmd },
    { "function",
        "?name? ?-protection? ?-type? ?-name? ?-args? ?-body?",
        Itcl_BiInfoFunctionCmd },
    { "heritage", "", Itcl_BiInfoHeritageCmd },
    { "inherit", "", Itcl_BiInfoInheritCmd },
    { "option",
        "?name? ?-protection? ?-resource? ?-class? ?-name? ?-default? \
?-cgetmethod? ?-configuremethod? ?-validatemethod? ?-value?",
        Itcl_BiInfoOptionCmd },
    { "variable",
        "?name? ?-protection? ?-type? ?-name? ?-init? ?-value? ?-config?",
         Itcl_BiInfoVariableCmd },
    { "vars", "?pattern?", Itcl_BiInfoVarsCmd },
    /*
     *  Add an error handler to support all of the usual inquiries
     *  for the "info" command in the global namespace.
     */
    { "@error", "", Itcl_DefaultInfoCmd },
    { NULL, NULL, NULL }
};

struct NameProcMap { const char *name; Tcl_ObjCmdProc *proc; };

/*
 * List of commands that are used to implement the [info object] subcommands.
 */

static const struct NameProcMap infoCmds2[] = {
    { "::itcl::builtin::Info::args", Itcl_BiInfoArgsCmd },
    { "::itcl::builtin::Info::body", Itcl_BiInfoBodyCmd },
    { "::itcl::builtin::Info::class", Itcl_BiInfoClassCmd },
    { "::itcl::builtin::Info::component", Itcl_BiInfoComponentCmd },
    { "::itcl::builtin::Info::function", Itcl_BiInfoFunctionCmd },
    { "::itcl::builtin::Info::heritage", Itcl_BiInfoHeritageCmd },
    { "::itcl::builtin::Info::inherit", Itcl_BiInfoInheritCmd },
    { "::itcl::builtin::Info::option", Itcl_BiInfoOptionCmd },
    { "::itcl::builtin::Info::variable", Itcl_BiInfoVariableCmd },
    { "::itcl::builtin::Info::vars", Itcl_BiInfoVarsCmd },
    { "::itcl::builtin::Info::unknown", Itcl_BiInfoUnknownCmd },
    /*
     *  Add an error handler to support all of the usual inquiries
     *  for the "info" command in the global namespace.
     */
    { "::itcl::builtin::Info::@error", Itcl_DefaultInfoCmd },
    { NULL, NULL }
};


/*
 * ------------------------------------------------------------------------
 *  ItclInfoInit()
 *
 *  Creates a namespace full of built-in methods/procs for [incr Tcl]
 *  classes.  This includes things like the "info"
 *  for querying class info.  Usually invoked by Itcl_Init() when
 *  [incr Tcl] is first installed into an interpreter.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
int
ItclInfoInit(
    Tcl_Interp *interp)      /* current interpreter */
{
    Tcl_Namespace *nsPtr;
    Tcl_Command cmd;
    int i;

    ItclObjectInfo *infoPtr;
    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
            ITCL_INTERP_DATA, NULL);
    /*
     * Build the ensemble used to implement [info].
     */

    nsPtr = Tcl_CreateNamespace(interp, "::itcl::builtin::Info", NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("ITCL: error in creating namespace: ::itcl::builtin::Info \n");
    }
    cmd = Tcl_CreateEnsemble(interp, nsPtr->fullName, nsPtr,
        TCL_ENSEMBLE_PREFIX);
    Tcl_Export(interp, nsPtr, "[a-z]*", 1);
    for (i=0 ; infoCmds2[i].name!=NULL ; i++) {
        Tcl_CreateObjCommand(interp, infoCmds2[i].name,
                infoCmds2[i].proc, infoPtr, NULL);
    }
    Tcl_Obj *ensObjPtr = Tcl_NewStringObj("::itcl::builtin::Info", -1);
    Tcl_IncrRefCount(ensObjPtr);
    Tcl_Obj *unkObjPtr = Tcl_NewStringObj("::itcl::builtin::Info::unknown", -1);
    Tcl_IncrRefCount(unkObjPtr);
    if (Tcl_SetEnsembleUnknownHandler(NULL,
            Tcl_FindEnsemble(interp, ensObjPtr, TCL_LEAVE_ERR_MSG),
	    unkObjPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(ensObjPtr);
    Tcl_DecrRefCount(unkObjPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclGetInfoUsage()
 *
 * ------------------------------------------------------------------------
  */
void
ItclGetInfoUsage(
    Tcl_Interp *interp,
    Tcl_Obj *objPtr)       /* returns: summary of usage info */
{
    char *spaces = "  ";
    int isOpenEnded = 0;

    int i;

    for (i=0; InfoMethodList[i].name != NULL; i++) {
	if (strcmp(InfoMethodList[i].name, "vars") == 0) {
	    /* we don't report that, as it is a special case
	     * it is only adding the protected and private commons
	     * to the ::info vars command */
	    continue;
	}
        if (*InfoMethodList[i].name == '@'
	        && strcmp(InfoMethodList[i].name,"@error") == 0) {
            isOpenEnded = 1;
        } else {
            Tcl_AppendToObj(objPtr, spaces, -1);
            Tcl_AppendToObj(objPtr, "info ", -1);
            Tcl_AppendToObj(objPtr, InfoMethodList[i].name, -1);
	    if (strlen(InfoMethodList[i].usage) > 0) {
              Tcl_AppendToObj(objPtr, " ", -1);
              Tcl_AppendToObj(objPtr, InfoMethodList[i].usage, -1);
	    }
            spaces = "\n  ";
        }
    }
    if (isOpenEnded) {
        Tcl_AppendToObj(objPtr,
            "\n...and others described on the man page", -1);
    }
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoCmd()
 *
 *  Invoked whenever the user issues the "info" method for an object.
 *  Handles the following syntax:
 *
 *    <objName> info 
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(2, "Itcl_BiInfoCmd", objc, objv);
    if (objc == 1) {
        /* produce usage message */
        Tcl_Obj *objPtr = Tcl_NewStringObj(
	        "wrong # args: should be one of...\n", -1);
        ItclGetInfoUsage(interp, objPtr);
	Tcl_SetResult(interp, Tcl_GetString(objPtr), TCL_DYNAMIC);
	return TCL_ERROR;
    }
    return ItclEnsembleSubCmd(clientData, interp, "::info itclinfo",
            objc, objv, "Itcl_BiInfoCmd");
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoClassCmd()
 *
 *  Returns information regarding the class for an object.  This command
 *  can be invoked with or without an object context:
 *
 *    <objName> info class   <= returns most-specific class name
 *    info class             <= returns active namespace name
 *
 *  Returns a status TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoClassCmd(
    ClientData dummy,     /* not used */
    Tcl_Interp *interp,   /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);
    Tcl_Namespace *contextNs = NULL;

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    char *name;

    ItclShowArgs(2, "Itcl_BiInfoClassCmd", objc, objv);
    if (objc != 1) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"info class\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        /* try it the hard way */
	ClientData clientData;
	clientData = Itcl_GetCallFrameClientData(interp);
        ItclObjectInfo *infoPtr;
        infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
                ITCL_INTERP_DATA, NULL);
        Tcl_Object oPtr;
	if (clientData != NULL) {
            oPtr = Tcl_ObjectContextObject(clientData);
            contextIoPtr = Tcl_ObjectGetMetadata(oPtr,
	            infoPtr->object_meta_type);
            contextIclsPtr = contextIoPtr->iclsPtr;
	}
	if ((contextIoPtr == NULL) || (contextIclsPtr == NULL)) {
	    Tcl_Obj *msg = Tcl_NewStringObj("\nget info like this instead 1: " \
		    "\n  namespace eval className { info ", -1);
	    Tcl_AppendStringsToObj(msg, Tcl_GetString(objv[0]), "... }", NULL);
            Tcl_SetObjResult(interp, msg);
            return TCL_ERROR;
        }
    }

    /*
     *  If there is an object context, then return the most-specific
     *  class for the object.  Otherwise, return the class namespace
     *  name.  Use normal class names when possible.
     */
    if (contextIoPtr) {
        contextNs = contextIoPtr->iclsPtr->nsPtr;
    } else {
        assert(contextIclsPtr != NULL);
        assert(contextIclsPtr->nsPtr != NULL);
        if (contextIclsPtr->infoPtr->useOldResolvers) {
            contextNs = Itcl_GetUplevelNamespace(interp, 1);
        } else {
            contextNs = contextIclsPtr->nsPtr;
	}
    }

    if (contextNs == NULL) {
        name = activeNs->fullName;
    } else {
        if (contextNs->parentPtr == activeNs) {
            name = contextNs->name;
        } else {
            name = contextNs->fullName;
        }
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj(name, -1));
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoInheritCmd()
 *
 *  Returns the list of base classes for the current class context.
 *  Returns a status TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoInheritCmd(
    ClientData dummy,      /* not used */
    Tcl_Interp *interp,    /* current interpreter */
    int objc,              /* number of arguments */
    Tcl_Obj *CONST objv[]) /* argument objects */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclCallContext *callContextPtr;
    Itcl_ListElem *elem;
    ItclMemberFunc *imPtr;
    Tcl_Obj *listPtr;
    Tcl_Obj *objPtr;

    ItclShowArgs(2, "Itcl_BiInfoInheritCmd", objc, objv);
    if (objc != 1) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"info inherit\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        char *name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 2: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }

    /*
     *  Return the list of base classes.
     */
    listPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);

    infoPtr = Tcl_GetAssocData(interp, ITCL_INTERP_DATA, NULL);
    callContextPtr = Itcl_PeekStack(&infoPtr->contextStack);
    imPtr = callContextPtr->imPtr;
    contextIclsPtr = imPtr->iclsPtr;
    Tcl_Namespace *upNsPtr;
    upNsPtr = Itcl_GetUplevelNamespace(interp, 1);
    if (imPtr->iclsPtr->infoPtr->useOldResolvers) {
        if (contextIoPtr != NULL) {
            if (upNsPtr != contextIclsPtr->nsPtr) {
		Tcl_HashEntry *hPtr;
		hPtr = Tcl_FindHashEntry(
		        &imPtr->iclsPtr->infoPtr->namespaceClasses,
			(char *)upNsPtr);
		if (hPtr != NULL) {
		    contextIclsPtr = Tcl_GetHashValue(hPtr);
		} else {
                    contextIclsPtr = contextIoPtr->iclsPtr;
	        }
            }
        }
    } else {
        if (strcmp(Tcl_GetString(imPtr->namePtr), "info") == 0) {
            if (contextIoPtr != NULL) {
	        contextIclsPtr = contextIoPtr->iclsPtr;
            }
        }
    }

    elem = Itcl_FirstListElem(&contextIclsPtr->bases);
    while (elem) {
        iclsPtr = (ItclClass*)Itcl_GetListValue(elem);
        if (iclsPtr->nsPtr->parentPtr == activeNs) {
            objPtr = Tcl_NewStringObj(iclsPtr->nsPtr->name, -1);
        } else {
            objPtr = Tcl_NewStringObj(iclsPtr->nsPtr->fullName, -1);
        }
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);
        elem = Itcl_NextListElem(elem);
    }

    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoHeritageCmd()
 *
 *  Returns the entire derivation hierarchy for this class, presented
 *  in the order that classes are traversed for finding data members
 *  and member functions.
 *
 *  Returns a status TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoHeritageCmd(
    ClientData dummy,     /* not used */
    Tcl_Interp *interp,   /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    char *name;
    ItclObjectInfo *infoPtr;
    ItclHierIter hier;
    ItclCallContext *callContextPtr;
    ItclMemberFunc *imPtr;
    Tcl_Obj *listPtr;
    Tcl_Obj *objPtr;
    ItclClass *iclsPtr;

    ItclShowArgs(2, "Itcl_BiInfoHeritageCmd", objc, objv);
    if (objc != 1) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"info heritage\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 3: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Traverse through the derivation hierarchy and return
     *  base class names.
     */
    listPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
    infoPtr = Tcl_GetAssocData(interp, ITCL_INTERP_DATA, NULL);
    callContextPtr = Itcl_PeekStack(&infoPtr->contextStack);
    imPtr = callContextPtr->imPtr;
    contextIclsPtr = imPtr->iclsPtr;
    Tcl_Namespace *upNsPtr;
    upNsPtr = Itcl_GetUplevelNamespace(interp, 1);
    if (contextIclsPtr->infoPtr->useOldResolvers) {
        if (contextIoPtr != NULL) {
            if (upNsPtr != contextIclsPtr->nsPtr) {
	        Tcl_HashEntry *hPtr;
	        hPtr = Tcl_FindHashEntry(&imPtr->iclsPtr->infoPtr->namespaceClasses, (char *)upNsPtr);
	        if (hPtr != NULL) {
	            contextIclsPtr = Tcl_GetHashValue(hPtr);
	        } else {
                    contextIclsPtr = contextIoPtr->iclsPtr;
	        }
            }
        }
    } else {
        if (strcmp(Tcl_GetString(imPtr->namePtr), "info") == 0) {
            if (contextIoPtr != NULL) {
	        contextIclsPtr = contextIoPtr->iclsPtr;
            }
        }
    }

    Itcl_InitHierIter(&hier, contextIclsPtr);
    while ((iclsPtr=Itcl_AdvanceHierIter(&hier)) != NULL) {
/* FIX ME !!! */
if (iclsPtr->nsPtr == NULL) {
fprintf(stderr, "ITCL: iclsPtr->nsPtr == NULL %s 0x%08x\n", Tcl_GetString(iclsPtr->fullNamePtr), iclsPtr->flags);
return TCL_ERROR;
}
        if (iclsPtr->nsPtr->parentPtr == activeNs) {
            objPtr = Tcl_NewStringObj(iclsPtr->nsPtr->name, -1);
        } else {
            objPtr = Tcl_NewStringObj(iclsPtr->nsPtr->fullName, -1);
        }
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);
    }
    Itcl_DeleteHierIter(&hier);

    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoFunctionCmd()
 *
 *  Returns information regarding class member functions (methods/procs).
 *  Handles the following syntax:
 *
 *    info function ?cmdName? ?-protection? ?-type? ?-name? ?-args? ?-body?
 *
 *  If the ?cmdName? is not specified, then a list of all known
 *  command members is returned.  Otherwise, the information for
 *  a specific command is returned.  Returns a status TCL_OK/TCL_ERROR
 *  to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoFunctionCmd(
    ClientData dummy,     /* not used */
    Tcl_Interp *interp,   /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    char *cmdName = NULL;
    Tcl_Obj *resultPtr = NULL;
    Tcl_Obj *objPtr = NULL;

    static const char *options[] = {
        "-args", "-body", "-name", "-protection", "-type",
        (char*)NULL
    };
    enum BIfIdx {
        BIfArgsIdx, BIfBodyIdx, BIfNameIdx, BIfProtectIdx, BIfTypeIdx
    } *iflist, iflistStorage[5];

    static enum BIfIdx DefInfoFunction[5] = {
        BIfProtectIdx,
        BIfTypeIdx,
        BIfNameIdx,
        BIfArgsIdx,
        BIfBodyIdx
    };

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    ItclClass *iclsPtr;
    int i;
    int result;
    char *name;
    char *val;
    Tcl_HashSearch place;
    Tcl_HashEntry *entry;
    ItclMemberFunc *imPtr;
    ItclMemberCode *mcode;
    ItclHierIter hier;

    ItclShowArgs(2, "Itcl_InfoFunctionCmd", objc, objv);
    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 4: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }

    /*
     *  Process args:
     *  ?cmdName? ?-protection? ?-type? ?-name? ?-args? ?-body?
     */
    objv++;  /* skip over command name */
    objc--;

    if (objc > 0) {
        cmdName = Tcl_GetString(*objv);
        objc--; objv++;
    }

    /*
     *  Return info for a specific command.
     */
    if (cmdName) {
        entry = Tcl_FindHashEntry(&contextIclsPtr->resolveCmds, cmdName);
        if (entry == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "\"", cmdName, "\" isn't a member function in class \"",
                contextIclsPtr->nsPtr->fullName, "\"",
                (char*)NULL);
            return TCL_ERROR;
        }

        imPtr = (ItclMemberFunc*)Tcl_GetHashValue(entry);
        mcode = imPtr->codePtr;

        /*
         *  By default, return everything.
         */
        if (objc == 0) {
            objc = 5;
            iflist = DefInfoFunction;
        } else {

            /*
             *  Otherwise, scan through all remaining flags and
             *  figure out what to return.
             */
            iflist = &iflistStorage[0];
            for (i=0 ; i < objc; i++) {
                result = Tcl_GetIndexFromObj(interp, objv[i],
                    options, "option", 0, (int*)(&iflist[i]));
                if (result != TCL_OK) {
                    return TCL_ERROR;
                }
            }
        }

        if (objc > 1) {
            resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
        }

        for (i=0 ; i < objc; i++) {
            switch (iflist[i]) {
                case BIfArgsIdx:
                    if (mcode && mcode->argListPtr) {
			if (imPtr->usagePtr == NULL) {
                            objPtr = mcode->usagePtr;
			} else {
                            objPtr = imPtr->usagePtr;
		        }
                    } else {
		        if ((imPtr->flags & ITCL_ARG_SPEC) != 0) {
			    if (imPtr->usagePtr == NULL) {
                                objPtr = mcode->usagePtr;
			    } else {
			        objPtr = imPtr->usagePtr;
			    }
                        } else {
                            objPtr = Tcl_NewStringObj("<undefined>", -1);
                        }
		    }
                    break;

                case BIfBodyIdx:
                    if (mcode && Itcl_IsMemberCodeImplemented(mcode)) {
                        objPtr = mcode->bodyPtr;
                    } else {
                        objPtr = Tcl_NewStringObj("<undefined>", -1);
                    }
                    break;

                case BIfNameIdx:
                    objPtr = imPtr->fullNamePtr;
                    break;

                case BIfProtectIdx:
                    val = Itcl_ProtectionStr(imPtr->protection);
                    objPtr = Tcl_NewStringObj(val, -1);
                    break;

                case BIfTypeIdx:
                    val = ((imPtr->flags & ITCL_COMMON) != 0)
                        ? "proc" : "method";
                    objPtr = Tcl_NewStringObj(val, -1);
                    break;
            }

            if (objc == 1) {
                resultPtr = objPtr;
            } else {
                Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
            }
        }
        Tcl_SetObjResult(interp, resultPtr);
    } else {

        /*
         *  Return the list of available commands.
         */
        resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);

        Itcl_InitHierIter(&hier, contextIclsPtr);
        while ((iclsPtr=Itcl_AdvanceHierIter(&hier)) != NULL) {
            entry = Tcl_FirstHashEntry(&iclsPtr->functions, &place);
            while (entry) {
	        int useIt = 1;

                imPtr = (ItclMemberFunc*)Tcl_GetHashValue(entry);
		if (imPtr->codePtr && (imPtr->codePtr->flags & ITCL_BUILTIN)) {
		    if (strcmp(Tcl_GetString(imPtr->namePtr), "info") == 0) {
		        useIt = 0;
		    }
		}
		if (useIt) {
                    objPtr = imPtr->fullNamePtr;
                    Tcl_ListObjAppendElement((Tcl_Interp*)NULL,
		            resultPtr, objPtr);
                }

                entry = Tcl_NextHashEntry(&place);
            }
        }
        Itcl_DeleteHierIter(&hier);

        Tcl_SetObjResult(interp, resultPtr);
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoVariableCmd()
 *
 *  Returns information regarding class data members (variables and
 *  commons).  Handles the following syntax:
 *
 *    info variable ?varName? ?-protection? ?-type? ?-name?
 *        ?-init? ?-config? ?-value?
 *
 *  If the ?varName? is not specified, then a list of all known
 *  data members is returned.  Otherwise, the information for a
 *  specific member is returned.  Returns a status TCL_OK/TCL_ERROR
 *  to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoVariableCmd(
    ClientData dummy,        /* not used */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    char *varName = NULL;
    Tcl_Obj *resultPtr = NULL;
    Tcl_Obj *objPtr = NULL;

    static const char *options[] = {
        "-config", "-init", "-name", "-protection", "-type",
        "-value", (char*)NULL
    };
    enum BIvIdx {
        BIvConfigIdx, BIvInitIdx, BIvNameIdx, BIvProtectIdx,
        BIvTypeIdx, BIvValueIdx
    } *ivlist, ivlistStorage[6];

    static enum BIvIdx DefInfoVariable[5] = {
        BIvProtectIdx,
        BIvTypeIdx,
        BIvNameIdx,
        BIvInitIdx,
        BIvValueIdx
    };

    static enum BIvIdx DefInfoPubVariable[6] = {
        BIvProtectIdx,
        BIvTypeIdx,
        BIvNameIdx,
        BIvInitIdx,
        BIvConfigIdx,
        BIvValueIdx
    };

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    int i;
    int result;
    CONST char *val;
    CONST char *name;
    ItclClass *iclsPtr;
    Tcl_HashSearch place;
    Tcl_HashEntry *entry;
    ItclVariable *ivPtr;
    ItclVarLookup *vlookup;
    ItclHierIter hier;

    ItclShowArgs(2, "Itcl_BiInfoVariableCmd", objc, objv);
    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 5: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }

    /*
     *  Process args:
     *  ?varName? ?-protection? ?-type? ?-name? ?-init? ?-config? ?-value?
     */
    objv++;  /* skip over command name */
    objc--;

    if (objc > 0) {
        varName = Tcl_GetString(*objv);
        objc--; objv++;
    }

    /*
     *  Return info for a specific variable.
     */
    if (varName) {
        entry = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, varName);
        if (entry == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "\"", varName, "\" isn't a variable in class \"",
                contextIclsPtr->nsPtr->fullName, "\"",
                (char*)NULL);
            return TCL_ERROR;
        }

        vlookup = (ItclVarLookup*)Tcl_GetHashValue(entry);
        ivPtr = vlookup->ivPtr;

        /*
         *  By default, return everything.
         */
        if (objc == 0) {
            if (ivPtr->protection == ITCL_PUBLIC &&
                    ((ivPtr->flags & ITCL_COMMON) == 0)) {
                ivlist = DefInfoPubVariable;
                objc = 6;
            } else {
                ivlist = DefInfoVariable;
                objc = 5;
            }
        } else {

            /*
             *  Otherwise, scan through all remaining flags and
             *  figure out what to return.
             */
            ivlist = &ivlistStorage[0];
            for (i=0 ; i < objc; i++) {
                result = Tcl_GetIndexFromObj(interp, objv[i],
                    options, "option", 0, (int*)(&ivlist[i]));
                if (result != TCL_OK) {
                    return TCL_ERROR;
                }
            }
        }

        if (objc > 1) {
            resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
        }

        for (i=0 ; i < objc; i++) {
            switch (ivlist[i]) {
                case BIvConfigIdx:
                    if (ivPtr->codePtr && Itcl_IsMemberCodeImplemented(ivPtr->codePtr)) {
                        objPtr = ivPtr->codePtr->bodyPtr;
                    } else {
                        objPtr = Tcl_NewStringObj("", -1);
                    }
                    break;

                case BIvInitIdx:
                    /*
                     *  If this is the built-in "this" variable, then
                     *  report the object name as its initialization string.
                     */
                    if ((ivPtr->flags & ITCL_THIS_VAR) != 0) {
                        if ((contextIoPtr != NULL) && (contextIoPtr->accessCmd != NULL)) {
                            objPtr = Tcl_NewStringObj((char*)NULL, 0);
                            Tcl_GetCommandFullName(
                                contextIoPtr->iclsPtr->interp,
                                contextIoPtr->accessCmd, objPtr);
                        } else {
                            objPtr = Tcl_NewStringObj("<objectName>", -1);
                        }
                    } else {
		        if (vlookup->ivPtr->init) {
			    objPtr = vlookup->ivPtr->init;
                        } else {
                            objPtr = Tcl_NewStringObj("<undefined>", -1);
		        }
                    }
                    break;

                case BIvNameIdx:
                    objPtr = ivPtr->fullNamePtr;
                    break;

                case BIvProtectIdx:
                    val = Itcl_ProtectionStr(ivPtr->protection);
                    objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
                    break;

                case BIvTypeIdx:
                    val = ((ivPtr->flags & ITCL_COMMON) != 0)
                        ? "common" : "variable";
                    objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
                    break;

                case BIvValueIdx:
                    if ((ivPtr->flags & ITCL_COMMON) != 0) {
                        val = Itcl_GetCommonVar(interp,
			        Tcl_GetString(ivPtr->fullNamePtr),
                                ivPtr->iclsPtr);
                    } else {
		        if (contextIoPtr == NULL) {
                            Tcl_ResetResult(interp);
                            Tcl_AppendResult(interp,
                                    "cannot access object-specific info ",
                                    "without an object context",
                                    (char*)NULL);
                            return TCL_ERROR;
                        } else {
                            val = Itcl_GetInstanceVar(interp,
			            Tcl_GetString(ivPtr->namePtr),
                                    contextIoPtr, ivPtr->iclsPtr);
                        }
                    }
                    if (val == NULL) {
                        val = "<undefined>";
                    }
                    objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
		    Tcl_IncrRefCount(objPtr);
                    break;
            }

            if (objc == 1) {
                resultPtr = objPtr;
            } else {
                Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr,
                    objPtr);
            }
        }
        Tcl_SetObjResult(interp, resultPtr);
    } else {

        /*
         *  Return the list of available variables.  Report the built-in
         *  "this" variable only once, for the most-specific class.
         */
        resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
	Tcl_IncrRefCount(resultPtr);
        Itcl_InitHierIter(&hier, contextIclsPtr);
        while ((iclsPtr=Itcl_AdvanceHierIter(&hier)) != NULL) {
            entry = Tcl_FirstHashEntry(&iclsPtr->variables, &place);
            while (entry) {
                ivPtr = (ItclVariable*)Tcl_GetHashValue(entry);
                if ((ivPtr->flags & ITCL_THIS_VAR) != 0) {
                    if (iclsPtr == contextIclsPtr) {
                        objPtr = ivPtr->fullNamePtr;
                        Tcl_ListObjAppendElement((Tcl_Interp*)NULL,
                            resultPtr, objPtr);
                    }
                } else {
                    objPtr = ivPtr->fullNamePtr;
                    Tcl_ListObjAppendElement((Tcl_Interp*)NULL,
                        resultPtr, objPtr);
                }
                entry = Tcl_NextHashEntry(&place);
            }
        }
        Itcl_DeleteHierIter(&hier);

        Tcl_SetObjResult(interp, resultPtr);
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoVarsCmd()
 *
 *  Returns information regarding variables 
 *
 *    info vars ?pattern?
 *  uses ::info vars and adds Itcl common variables!!
 *
 *  Returns a status TCL_OK/TCL_ERROR
 *  to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoVarsCmd(
    ClientData dummy,        /* not used */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj **newObjv;
    FOREACH_HASH_DECLS;
    int result = TCL_OK;

    ItclShowArgs(2, "Itcl_BiInfoVars", objc, objv);
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc));
    newObjv[0] = Tcl_NewStringObj("::tcl::Info_vars", -1);
    Tcl_IncrRefCount(newObjv[0]);
    memcpy(newObjv+1, objv+1, sizeof(Tcl_Obj *)*(objc-1));
    result = Tcl_EvalObjv(interp, objc, newObjv, 0);
    Tcl_DecrRefCount(newObjv[0]);
    if (objc < 2) {
        return result;
    }
    if (result == TCL_OK) {
	Tcl_DString buffer;
	Tcl_Namespace *nsPtr;
	char *head;
	char *tail;
        /* check if the pattern contains a class namespace
	 * and if yes add the common private and protected vars
	 * and remove the ___DO_NOT_DELETE_THIS_VARIABLE var
	 */
	Itcl_ParseNamespPath(Tcl_GetString(objv[1]), &buffer, &head, &tail);
        if (head == NULL) {
	    nsPtr = Tcl_GetCurrentNamespace(interp);
	} else {
            nsPtr = Tcl_FindNamespace(interp, head, NULL, 0);
        }
	if ((nsPtr != NULL) && Itcl_IsClassNamespace(nsPtr)) {
	    ItclObjectInfo *infoPtr;
	    ItclClass *iclsPtr;
	    ItclVariable *ivPtr;
	    infoPtr = Tcl_GetAssocData(interp, ITCL_INTERP_DATA, NULL);
	    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses,
	            (char *)nsPtr);
	    if (hPtr != NULL) {
		Itcl_List varList;
		Tcl_Obj *resultListPtr;
		Tcl_Obj *namePtr;
		int numElems;

		Itcl_InitList(&varList);
		iclsPtr = Tcl_GetHashValue(hPtr);
		resultListPtr = Tcl_GetObjResult(interp);
		numElems = 0;
/* FIX ME !! should perhaps skip ___DO_NOT_DELETE_THIS_VARIABLE here !! */
		FOREACH_HASH_VALUE(ivPtr, &iclsPtr->variables) {
		    if ((ivPtr->flags & ITCL_COMMON) != 0) {
		        if (ivPtr->protection != ITCL_PUBLIC) {
			    if (head != NULL) {
			        namePtr = ivPtr->fullNamePtr;
			    } else {
			        namePtr = ivPtr->namePtr;
			    }
		            Tcl_ListObjAppendElement(interp, resultListPtr,
			            namePtr);
			    numElems++;
		        }
		    }
		}
	    }
	}
    }

    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoUnknownCmd()
 *
 *  the unknown handler for the ::itcl::builtin::Info ensemble
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoUnknownCmd(
    ClientData dummy,        /* not used */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj *listObj;
    Tcl_Obj *objPtr;
    int result;

    ItclShowArgs(2, "Itcl_BiInfoUnknownCmd", objc, objv);
    result = TCL_OK;
    if (objc < 3) {
        /* produce usage message */
        Tcl_Obj *objPtr = Tcl_NewStringObj(
	        "wrong # args: should be one of...\n", -1);
        ItclGetInfoUsage(interp, objPtr);
	Tcl_SetResult(interp, Tcl_GetString(objPtr), TCL_DYNAMIC);
        return TCL_ERROR;
    }
    listObj = Tcl_NewListObj(-1, NULL);
    objPtr = Tcl_NewStringObj("::info", -1);
    Tcl_ListObjAppendElement(interp, listObj, objPtr);
    objPtr = Tcl_NewStringObj(Tcl_GetString(objv[2]), -1);
    Tcl_ListObjAppendElement(interp, listObj, objPtr);
    Tcl_SetObjResult(interp, listObj);
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoBodyCmd()
 *
 *  Handles the usual "info body" request, returning the body for a
 *  specific proc.  Included here for backward compatibility, since
 *  otherwise Tcl would complain that class procs are not real "procs".
 *  Returns a status TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoBodyCmd(
    ClientData dummy,     /* not used */
    Tcl_Interp *interp,   /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    char *name;
    ItclMemberFunc *imPtr;
    ItclMemberCode *mcode;
    Tcl_HashEntry *entry;
    Tcl_Obj *objPtr;

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    if (objc != 2) {
	Tcl_AppendResult(interp,
	        "wrong # args: should be \"info body function\"",
	        NULL);
//        Tcl_WrongNumArgs(interp, 1, objv, "function");
        return TCL_ERROR;
    }

    /*
     *  If this command is not invoked within a class namespace,
     *  then treat the procedure name as a normal Tcl procedure.
     */
    contextIclsPtr = NULL;
    if (!Itcl_IsClassNamespace(Tcl_GetCurrentNamespace(interp))) {
/* FIX ME !!! */
#ifdef NOTDEF
        Proc *procPtr;

        name = Tcl_GetStringFromObj(objv[1], (int*)NULL);
        procPtr = TclFindProc((Interp*)interp, name);
        if (procPtr == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "\"", name, "\" isn't a procedure",
                (char*)NULL);
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, procPtr->bodyPtr);
#endif
    }

    /*
     *  Otherwise, treat the name as a class method/proc.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 6: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }

    name = Tcl_GetString(objv[1]);
    entry = Tcl_FindHashEntry(&contextIclsPtr->resolveCmds, name);
    if (entry == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\"", name, "\" isn't a procedure",
            (char*)NULL);
        return TCL_ERROR;
    }

    imPtr = (ItclMemberFunc*)Tcl_GetHashValue(entry);
    mcode = imPtr->codePtr;

    /*
     *  Return a string describing the implementation.
     */
    if (mcode && Itcl_IsMemberCodeImplemented(mcode)) {
        objPtr = mcode->bodyPtr;
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_SetObjResult(interp, objPtr);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoArgsCmd()
 *
 *  Handles the usual "info args" request, returning the argument list
 *  for a specific proc.  Included here for backward compatibility, since
 *  otherwise Tcl would complain that class procs are not real "procs".
 *  Returns a status TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoArgsCmd(
    ClientData dummy,     /* not used */
    Tcl_Interp *interp,   /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    char *name;
    ItclMemberFunc *imPtr;
    ItclMemberCode *mcode;
    Tcl_HashEntry *entry;
    Tcl_Obj *objPtr;

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    if (objc != 2) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"info args function\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    name = Tcl_GetString(objv[1]);

    /*
     *  If this command is not invoked within a class namespace,
     *  then treat the procedure name as a normal Tcl procedure.
     */
    contextIclsPtr = NULL;
    if (!Itcl_IsClassNamespace(Tcl_GetCurrentNamespace(interp))) {
/* FIX ME !!! */
#ifdef NOTDEF
        Proc *procPtr;
//        CompiledLocal *localPtr;

        procPtr = TclFindProc((Interp*)interp, name);
        if (procPtr == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "\"", name, "\" isn't a procedure",
                (char*)NULL);
            return TCL_ERROR;
        }

        objPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
        for (localPtr = procPtr->firstLocalPtr;
             localPtr != NULL;
             localPtr = localPtr->nextPtr) {
            if (TclIsVarArgument(localPtr)) {
                Tcl_ListObjAppendElement(interp, objPtr,
                    Tcl_NewStringObj(localPtr->name, -1));
            }
        }

        Tcl_SetObjResult(interp, objPtr);
#endif
    }

    /*
     *  Otherwise, treat the name as a class method/proc.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 7: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }

    entry = Tcl_FindHashEntry(&contextIclsPtr->resolveCmds, name);
    if (entry == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\"", name, "\" isn't a procedure",
            (char*)NULL);
        return TCL_ERROR;
    }

    imPtr = (ItclMemberFunc*)Tcl_GetHashValue(entry);
    mcode = imPtr->codePtr;

    /*
     *  Return a string describing the argument list.
     */
    if (mcode && mcode->argListPtr != NULL) {
        objPtr = imPtr->usagePtr;
    } else {
        if ((imPtr->flags & ITCL_ARG_SPEC) != 0) {
            objPtr = imPtr->usagePtr;
        } else {
            objPtr = Tcl_NewStringObj("<undefined>", -1);
        }
    }
    Tcl_SetObjResult(interp, objPtr);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_DefaultInfoCmd()
 *
 *  Handles any unknown options for the "itcl::builtin::info" command
 *  by passing requests on to the usual "::info" command.  If the
 *  option is recognized, then it is handled.  Otherwise, if it is
 *  still unknown, then an error message is returned with the list
 *  of possible options.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_DefaultInfoCmd(
    ClientData dummy,     /* not used */
    Tcl_Interp *interp,   /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int result;
    char *name;
    Tcl_CmdInfo cmdInfo;
    Tcl_Command cmd;
    Tcl_Obj *resultPtr;

    /*
     *  Look for the usual "::info" command, and use it to
     *  evaluate the unknown option.
     */
    cmd = Tcl_FindCommand(interp, "::info", (Tcl_Namespace*)NULL, 0);
    if (cmd == NULL) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);

        resultPtr = Tcl_GetObjResult(interp);
        Tcl_AppendStringsToObj(resultPtr,
            "bad option \"", name, "\" should be one of...\n",
            (char*)NULL);
        Itcl_GetEnsembleUsageForObj(interp, objv[0], resultPtr);

        return TCL_ERROR;
    }

    Tcl_GetCommandInfoFromToken(cmd, &cmdInfo);
    result = (*cmdInfo.objProc)(cmdInfo.objClientData, interp, objc, objv);

    /*
     *  If the option was not recognized by the usual "info" command,
     *  then we got a "bad option" error message.  Add the options
     *  for the current ensemble to the error message.
     */
    if (result != TCL_OK && strncmp(interp->result,"bad option",10) == 0) {
        resultPtr = Tcl_GetObjResult(interp);
        Tcl_AppendToObj(resultPtr, "\nor", -1);
        Itcl_GetEnsembleUsageForObj(interp, objv[0], resultPtr);
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoOptionCmd()
 *
 *  Returns information regarding class options.
 *  Handles the following syntax:
 *
 *    info option ?optionName? ?-protection? ?-name? ?-resource? ?-class?
 *        ?-default? ?-configmethod? ?-cgetmethod? ?-validatemethod? ?-value?
 *
 *  If the ?optionName? is not specified, then a list of all known
 *  data members is returned.  Otherwise, the information for a
 *  specific member is returned.  Returns a status TCL_OK/TCL_ERROR
 *  to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoOptionCmd(
    ClientData dummy,        /* not used */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    char *optionName = NULL;
    Tcl_Obj *resultPtr = NULL;
    Tcl_Obj *objPtr = NULL;
    Tcl_Obj *optionNamePtr;

    static const char *options[] = {
        "-cgetmethod", "-class", "-configuremethod", "-default",
	"-name", "-protection", "-resource", "-validatemethod",
        "-value", (char*)NULL
    };
    enum BOptIdx {
        BOptCgetMethodIdx, BOptClassIdx, BOptConfigureMethodIdx, BOptDefaultIdx,
	BOptNameIdx, BOptProtectIdx, BOptResourceIdx, BOptValidateMethodIdx,
	BOptValueIdx
    } *ioptlist, ioptlistStorage[9];

    static enum BOptIdx DefInfoOption[9] = {
        BOptProtectIdx,
        BOptNameIdx,
        BOptResourceIdx,
        BOptClassIdx,
        BOptDefaultIdx,
        BOptCgetMethodIdx,
        BOptConfigureMethodIdx,
        BOptValidateMethodIdx,
        BOptValueIdx
    };

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;
    ItclObjectInfo *infoPtr;

    Tcl_HashSearch place;
    Tcl_HashEntry *hPtr;
    Tcl_Namespace *nsPtr;
    ItclOption *ioptPtr;
    ItclHierIter hier;
    ItclClass *iclsPtr;
    CONST char *val;
    CONST char *name;
    int i;
    int result;

    ItclShowArgs(1, "Itcl_BiInfoOptionCmd", objc, objv);
    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 5: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }
    nsPtr = Itcl_GetUplevelNamespace(interp, 1);
    infoPtr = contextIclsPtr->infoPtr;
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)nsPtr);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "cannot find class name for namespace \"",
	        nsPtr->fullName, "\"", NULL);
	return TCL_ERROR;
    }
    contextIclsPtr = Tcl_GetHashValue(hPtr);

    /*
     *  Process args:
     *  ?optionName? ?-protection? ?-name? ?-resource? ?-class?
     * ?-default? ?-cgetmethod? ?-configuremethod? ?-validatemethod? ?-value?
     */
    objv++;  /* skip over command name */
    objc--;

    if (objc > 0) {
        optionName = Tcl_GetString(*objv);
        objc--;
	objv++;
    }

    /*
     *  Return info for a specific option.
     */
    if (optionName) {
	optionNamePtr = Tcl_NewStringObj(optionName, -1);
        hPtr = Tcl_FindHashEntry(&contextIclsPtr->options,
	        (char *)optionNamePtr);
        if (hPtr == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "\"", optionName, "\" isn't a option in class \"",
                contextIclsPtr->nsPtr->fullName, "\"",
                (char*)NULL);
            return TCL_ERROR;
        }

        ioptPtr = (ItclOption*)Tcl_GetHashValue(hPtr);

        /*
         *  By default, return everything.
         */
        if (objc == 0) {
            ioptlist = DefInfoOption;
            objc = 9;
        } else {

            /*
             *  Otherwise, scan through all remaining flags and
             *  figure out what to return.
             */
            ioptlist = &ioptlistStorage[0];
            for (i=0 ; i < objc; i++) {
                result = Tcl_GetIndexFromObj(interp, objv[i],
                    options, "option", 0, (int*)(&ioptlist[i]));
                if (result != TCL_OK) {
                    return TCL_ERROR;
                }
            }
        }

        if (objc > 1) {
            resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
        }

        for (i=0 ; i < objc; i++) {
            switch (ioptlist[i]) {
                case BOptCgetMethodIdx:
                    if (ioptPtr->cgetMethodPtr) {
                        objPtr = ioptPtr->cgetMethodPtr;
                    } else {
                        objPtr = Tcl_NewStringObj("", -1);
                    }
                    break;

                case BOptConfigureMethodIdx:
                    if (ioptPtr->configureMethodPtr) {
                        objPtr = ioptPtr->configureMethodPtr;
                    } else {
                        objPtr = Tcl_NewStringObj("", -1);
                    }
                    break;

                case BOptValidateMethodIdx:
                    if (ioptPtr->validateMethodPtr) {
                        objPtr = ioptPtr->validateMethodPtr;
                    } else {
                        objPtr = Tcl_NewStringObj("", -1);
                    }
                    break;

                case BOptResourceIdx:
                    if (ioptPtr->resourceNamePtr) {
                        objPtr = ioptPtr->resourceNamePtr;
                    } else {
                        objPtr = Tcl_NewStringObj("", -1);
                    }
                    break;

                case BOptClassIdx:
                    if (ioptPtr->classNamePtr) {
                        objPtr = ioptPtr->classNamePtr;
                    } else {
                        objPtr = Tcl_NewStringObj("", -1);
                    }
                    break;

                case BOptDefaultIdx:
		    if (ioptPtr->defaultValuePtr) {
		        objPtr = ioptPtr->defaultValuePtr;
                    } else {
                        objPtr = Tcl_NewStringObj("<undefined>", -1);
		    }
                    break;

                case BOptNameIdx:
                    objPtr = ioptPtr->fullNamePtr;
                    break;

                case BOptProtectIdx:
                    val = Itcl_ProtectionStr(ioptPtr->protection);
                    objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
                    break;

                case BOptValueIdx:
		    if (contextIoPtr == NULL) {
                        Tcl_ResetResult(interp);
                        Tcl_AppendResult(interp,
                                "cannot access object-specific info ",
                                "without an object context",
                                (char*)NULL);
                        return TCL_ERROR;
                    } else {
                        val = ItclGetInstanceVar(interp, "itcl_options",
		                Tcl_GetString(ioptPtr->namePtr),
                                    contextIoPtr, ioptPtr->iclsPtr);
                    }
                    if (val == NULL) {
                        val = "<undefined>";
                    }
                    objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
		    Tcl_IncrRefCount(objPtr);
                    break;
            }

            if (objc == 1) {
                resultPtr = objPtr;
            } else {
                Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr,
                    objPtr);
            }
        }
        Tcl_SetObjResult(interp, resultPtr);
    } else {

        /*
         *  Return the list of available options.
         */
        resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
	Tcl_IncrRefCount(resultPtr);
        Itcl_InitHierIter(&hier, contextIclsPtr);
        while ((iclsPtr=Itcl_AdvanceHierIter(&hier)) != NULL) {
            hPtr = Tcl_FirstHashEntry(&iclsPtr->options, &place);
            while (hPtr) {
                ioptPtr = (ItclOption*)Tcl_GetHashValue(hPtr);
                objPtr = ioptPtr->fullNamePtr;
                Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
                hPtr = Tcl_NextHashEntry(&place);
            }
        }
        Itcl_DeleteHierIter(&hier);

        Tcl_SetObjResult(interp, resultPtr);
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInfoComponentCmd()
 *
 *  Returns information regarding class components.
 *  Handles the following syntax:
 *
 *    info component ?componentName? ?-inherit? ?-name? ?-value?
 *
 *  If the ?componentName? is not specified, then a list of all known
 *  data members is returned.  Otherwise, the information for a
 *  specific member is returned.  Returns a status TCL_OK/TCL_ERROR
 *  to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInfoComponentCmd(
    ClientData dummy,        /* not used */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    char *componentName = NULL;
    Tcl_Obj *resultPtr = NULL;
    Tcl_Obj *objPtr = NULL;
    Tcl_Obj *componentNamePtr;

    static const char *components[] = {
	"-name", "-inherit", "-value", (char*)NULL
    };
    enum BCompIdx {
	BCompNameIdx, BCompInheritIdx, BCompValueIdx
    } *icomplist, icomplistStorage[3];

    static enum BCompIdx DefInfoComponent[3] = {
        BCompNameIdx,
        BCompInheritIdx,
        BCompValueIdx
    };

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;
    ItclObjectInfo *infoPtr;

    Tcl_HashSearch place;
    Tcl_HashEntry *hPtr;
    Tcl_Namespace *nsPtr;
    ItclComponent *icPtr;
    ItclHierIter hier;
    ItclClass *iclsPtr;
    CONST char *val;
    CONST char *name;
    int i;
    int result;

    ItclShowArgs(1, "Itcl_BiInfoComponentCmd", objc, objv);
    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
        name = Tcl_GetString(objv[0]);
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\nget info like this instead 5: ",
            "\n  namespace eval className { info ", name, "... }",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (contextIoPtr != NULL) {
        contextIclsPtr = contextIoPtr->iclsPtr;
    }
    nsPtr = Itcl_GetUplevelNamespace(interp, 1);
    if (nsPtr->parentPtr == NULL) {
        /* :: namespace */
	nsPtr = contextIclsPtr->nsPtr;
    }
    infoPtr = contextIclsPtr->infoPtr;
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)nsPtr);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "cannot find class name for namespace \"",
	        nsPtr->fullName, "\"", NULL);
	return TCL_ERROR;
    }
    contextIclsPtr = Tcl_GetHashValue(hPtr);

    /*
     *  Process args:
     *  ?componentName? ?-inherit? ?-name? ?-value?
     */
    objv++;  /* skip over command name */
    objc--;

    if (objc > 0) {
        componentName = Tcl_GetString(*objv);
        objc--;
	objv++;
    }

    /*
     *  Return info for a specific component.
     */
    if (componentName) {
	componentNamePtr = Tcl_NewStringObj(componentName, -1);
	Itcl_InitHierIter(&hier, contextIoPtr->iclsPtr);
	while ((iclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
	    hPtr = Tcl_FindHashEntry(&iclsPtr->components,
	            (char *)componentNamePtr);
	    if (hPtr != NULL) {
	        break;
	    }
	}
	Itcl_DeleteHierIter(&hier);
        if (hPtr == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "\"", componentName, "\" isn't a component in class \"",
                contextIclsPtr->nsPtr->fullName, "\"",
                (char*)NULL);
            return TCL_ERROR;
        }
        icPtr = (ItclComponent *)Tcl_GetHashValue(hPtr);

        /*
         *  By default, return everything.
         */
        if (objc == 0) {
            icomplist = DefInfoComponent;
            objc = 3;
        } else {

            /*
             *  Otherwise, scan through all remaining flags and
             *  figure out what to return.
             */
            icomplist = &icomplistStorage[0];
            for (i=0 ; i < objc; i++) {
                result = Tcl_GetIndexFromObj(interp, objv[i],
                    components, "component", 0, (int*)(&icomplist[i]));
                if (result != TCL_OK) {
                    return TCL_ERROR;
                }
            }
        }

        if (objc > 1) {
            resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
        }

        for (i=0 ; i < objc; i++) {
            switch (icomplist[i]) {
                case BCompNameIdx:
                    objPtr = icPtr->ivPtr->fullNamePtr;
                    break;

                case BCompInheritIdx:
		    if (icPtr->flags & ITCL_COMPONENT_INHERIT) {
                        val = "1";
		    } else {
                        val = "0";
		    }
                    objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
                    break;

                case BCompValueIdx:
		    if (contextIoPtr == NULL) {
                        Tcl_ResetResult(interp);
                        Tcl_AppendResult(interp,
                                "cannot access object-specific info ",
                                "without an object context",
                                (char*)NULL);
                        return TCL_ERROR;
                    } else {
                        val = ItclGetInstanceVar(interp,
			        Tcl_GetString(icPtr->namePtr), NULL,
                                    contextIoPtr, icPtr->ivPtr->iclsPtr);
                    }
                    if (val == NULL) {
                        val = "<undefined>";
                    }
                    objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
		    Tcl_IncrRefCount(objPtr);
                    break;
            }

            if (objc == 1) {
                resultPtr = objPtr;
            } else {
                Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr,
                    objPtr);
            }
        }
        Tcl_SetObjResult(interp, resultPtr);
    } else {

        /*
         *  Return the list of available components.
         */
        resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
	Tcl_IncrRefCount(resultPtr);
        Itcl_InitHierIter(&hier, contextIclsPtr);
        while ((iclsPtr=Itcl_AdvanceHierIter(&hier)) != NULL) {
            hPtr = Tcl_FirstHashEntry(&iclsPtr->components, &place);
            while (hPtr) {
                icPtr = (ItclComponent *)Tcl_GetHashValue(hPtr);
                objPtr = icPtr->ivPtr->fullNamePtr;
                Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
                hPtr = Tcl_NextHashEntry(&place);
            }
        }
        Itcl_DeleteHierIter(&hier);

        Tcl_SetObjResult(interp, resultPtr);
    }
    return TCL_OK;
}
