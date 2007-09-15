/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  These procedures handle built-in class methods, including the
 *  "installhull" method 
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetBuiltin.c,v 1.1.2.1 2007/09/15 11:58:35 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007 Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"
#include <tk.h>

/*
 *  Standard list of built-in methods for all objects.
 */
typedef struct BiMethod {
    char* name;              /* method name */
    char* usage;             /* string describing usage */
    char* registration;      /* registration name for C proc */
    Tcl_ObjCmdProc *proc;    /* implementation C proc */
} BiMethod;

static BiMethod BiMethodList[] = {
    { "installhull", "using widgetType ?arg ...?",
                   "@itcl-builtin-installhull",  Itcl_BiInstallHullCmd },
};
static int BiMethodListLen = sizeof(BiMethodList)/sizeof(BiMethod);


/*
 * ------------------------------------------------------------------------
 *  Itcl_WidgetBiInit()
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
Itcl_WidgetBiInit(
    Tcl_Interp *interp)      /* current interpreter */
{
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
	        BiMethodList[i].proc, (ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
    }
    Tcl_DStringFree(&buffer);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_InstallWidgetBiMethods()
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
Itcl_InstallWidgetBiMethods(
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
            result = Itcl_CreateMethod(interp, iclsPtr,
	        Tcl_NewStringObj(BiMethodList[i].name, -1),
                BiMethodList[i].usage, BiMethodList[i].registration);

            if (result != TCL_OK) {
                break;
            }
        }
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInstallHullCmd()
 *
 *  Invoked whenever the user issues the "installhull" method for an object.
 *  Handles the following syntax:
 *
 *    <objName> installhull using <widgetType> ?arg ...?
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInstallHullCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_HashEntry *hPtr;
    Tcl_Obj *namePtr;
    Tcl_Var varPtr;
    Tcl_HashSearch place;
    Tk_Window tkMainWin;
    Tk_Window tkWin;
    ItclClass *iclsPtr;
    ItclVariable *ivPtr;
    ItclOption *ioptPtr;
    const char *val;
    const char *widgetType;
    const char *className;
    char *token;
    int result;

    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    ItclShowArgs(0, "Itcl_BiInstallHullCmd", objc, objv);
    iclsPtr = (ItclClass *)clientData;
    if (iclsPtr->infoPtr->buildingWidget) {
        contextIclsPtr = iclsPtr;
	contextIoPtr = iclsPtr->infoPtr->currIoPtr;
    } else {
        /*
         *  Make sure that this command is being invoked in the proper
         *  context.
         */
        contextIclsPtr = NULL;
        if (Itcl_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
            return TCL_ERROR;
        }
    }

    if (contextIoPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "improper usage: should be \"", 
	    "object installhull using <widgetType> ?arg ...?\"",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (objc < 5) {
        token = Tcl_GetString(objv[0]);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"object ", token,
	    " using <widgetType> ?arg ...?\"", (char*)NULL);
        return TCL_ERROR;
    }
    widgetType = Tcl_GetString(objv[2]);
    if (strcmp(Tcl_GetString(objv[3]), "-class") != 0) {
        token = Tcl_GetString(objv[0]);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"object ", token,
            " using <widgetType> ?arg ...?\"", (char*)NULL);
        return TCL_ERROR;
    }
    className = Tcl_GetString(objv[4]);
    Tcl_DString buffer;
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, widgetType, -1);

    Tcl_DStringAppend(&buffer, " ", 1);
    Tcl_DStringAppend(&buffer, Tcl_GetString(contextIoPtr->namePtr), -1);
    Tcl_DStringAppend(&buffer, " -class ", 8);
    Tcl_DStringAppend(&buffer, className, -1);
    result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
    Tcl_DStringFree(&buffer);
    if (result != TCL_OK) {
        return result;
    }

    /* initialize the options array */
    tkMainWin = Tk_MainWindow(interp);
    tkWin = Tk_NameToWindow(interp, Tcl_GetString(contextIoPtr->namePtr),
            tkMainWin);
    hPtr = Tcl_FirstHashEntry(&iclsPtr->options, &place);
    while (hPtr) {
	ioptPtr = (ItclOption*)Tcl_GetHashValue(hPtr);
        val = Tk_GetOption(tkWin, Tcl_GetString(ioptPtr->resourceNamePtr),
	        Tcl_GetString(ioptPtr->classNamePtr));
	if (val != NULL) {
            val = Itcl_SetInstanceVar(interp, "options",
	            Tcl_GetString(ioptPtr->namePtr), val,
                    contextIoPtr, contextIoPtr->iclsPtr);
	}
	if (ioptPtr->init != NULL) {
            val = Itcl_SetInstanceVar(interp, "options",
	            Tcl_GetString(ioptPtr->namePtr),
		    Tcl_GetString(ioptPtr->init),
                    contextIoPtr, contextIoPtr->iclsPtr);
	}
        hPtr = Tcl_NextHashEntry(&place);
    }

    /* initialize the hull variable */
    Tcl_DStringAppend(&buffer, "::itclwidget::internal::hull", -1);
    int lgth = strlen(Tcl_DStringValue(&buffer));
    int i;
    i = 0;
    while (1) {
	Tcl_DStringSetLength(&buffer, lgth);
	i++;
	char buf[10];
	sprintf(buf, "%d", i);
	Tcl_DStringAppend(&buffer, buf, -1);
        Tcl_DStringAppend(&buffer, Tcl_GetString(contextIoPtr->namePtr), -1);
	if (Tcl_FindCommand(interp, Tcl_DStringValue(&buffer), NULL, 0)
	        == NULL) {
            break;
	}
    }
    TclRenameCommand(interp, Tcl_GetString(contextIoPtr->namePtr),
            Tcl_DStringValue(&buffer));

    namePtr = Tcl_NewStringObj("hull", -1);
    Tcl_IncrRefCount(namePtr);
    hPtr = Tcl_FindHashEntry(&contextIoPtr->iclsPtr->variables,
            (char *)namePtr);
    Tcl_DecrRefCount(namePtr);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "cannot find class variable hull", NULL);
        return TCL_ERROR;
    }
    ivPtr =Tcl_GetHashValue(hPtr);
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectVariables, (char *)ivPtr);
    varPtr = Tcl_GetHashValue(hPtr);
    val = Itcl_SetInstanceVar(interp, "hull", NULL, Tcl_DStringValue(&buffer),
            contextIoPtr, contextIoPtr->iclsPtr);
    Tcl_DStringFree(&buffer);
    if (val == NULL) {
        Tcl_AppendResult(interp, "cannot set hull for object \"",
            Tcl_GetString(contextIoPtr->namePtr), "\"", NULL);
        return TCL_ERROR;
    }

#ifdef NOTDEF
    if (varPtr != NULL) {
	Tcl_AppendResult(interp, "hull is already set for object \"", 
	        Tcl_GetString(contextIoPtr->namePtr), "\"", NULL);
        return TCL_ERROR;
    }
#endif
//fprintf(stderr, "Itcl_BiInstallHullCmd END!%s!\n", Tcl_GetStringResult(interp));
    return result;
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
        objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);

    return listPtr;
}



/*
 * ------------------------------------------------------------------------
 *  ItclWidgetConfigure()
 *
 *  Invoked whenever the user issues the "configure" method for an object.
 *  If the class is not ITCL_IS CLASS
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
ItclWidgetConfigure(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    Tcl_HashSearch place;
    Tcl_HashEntry *hPtr;
    Tcl_Obj *resultPtr;
    Tcl_Obj *objPtr;
    Tcl_DString buffer;
    Tcl_DString buffer2;
    ItclClass *iclsPtr;
    ItclVariable *ivPtr;
    ItclVarLookup *vlookup;
    ItclMemberCode *mcode;
    ItclHierIter hier;
    CONST char *lastval;
    char *token;
    char *varName;
    int i;
    int result;

    ItclShowArgs(0, "ItclWidgetConfigure", objc, objv);
    vlookup = NULL;
    token = NULL;
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

    /*
     *  HANDLE:  configure
     */
    if (objc == 1) {
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
        if (objc == 2) {
            token = Tcl_GetStringFromObj(objv[1], (int*)NULL);
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
	    Tcl_Obj *optionNamePtr;
	    optionNamePtr = Tcl_NewStringObj("*", -1);
	    Tcl_IncrRefCount(optionNamePtr);
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedOptions,
	            (char *)optionNamePtr);
            if (hPtr != NULL) {
                ItclDelegatedOption *idoPtr;
                ItclComponent *icPtr;
	        const char *val;
		idoPtr = Tcl_GetHashValue(hPtr);
		icPtr = idoPtr->icPtr;
		val = Itcl_GetInstanceVar(interp,
		        Tcl_GetString(icPtr->namePtr), contextIoPtr,
			contextIclsPtr);
	        if (val != NULL) {
		    Tcl_DString buffer;
		    Tcl_DStringInit(&buffer);
		    Tcl_DStringAppend(&buffer, val, -1);
		    Tcl_DStringAppend(&buffer, " configure ", -1);
		    Tcl_DStringAppend(&buffer, token, -1);
                    result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
                    return result;
		}
	    }
	    Tcl_SetStringObj(optionNamePtr, token, -1);
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->options,
	            (char *)optionNamePtr);
            if (hPtr != NULL) {
/* FIX ME to be implemented */
fprintf(stderr, "hPtr1!%p!%s!\n", hPtr, Tcl_GetString(optionNamePtr));
	    }
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedOptions,
	            (char *)optionNamePtr);
            if (hPtr != NULL) {
/* FIX ME to be implemented */
fprintf(stderr, "hPtr2!%p!\n", hPtr);
	    }
	    Tcl_DecrRefCount(optionNamePtr);
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

    Tcl_DStringInit(&buffer);
    Tcl_DStringInit(&buffer2);

    for (i=1; i < objc; i+=2) {
        vlookup = NULL;
        token = Tcl_GetString(objv[i]);
        if (*token == '-') {
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, token+1);
            if (hPtr) {
                vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
            }
        }

        if (!vlookup || vlookup->ivPtr->protection != ITCL_PUBLIC) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "unknown option \"", token, "\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto configureDone;
        }
        if (i == objc-1) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "value for \"", token, "\" missing",
                (char*)NULL);
            result = TCL_ERROR;
            goto configureDone;
        }

        ivPtr = vlookup->ivPtr;
        Tcl_DStringSetLength(&buffer2, 0);
        Tcl_DStringAppend(&buffer2,
	        Tcl_GetString(contextIoPtr->varNsNamePtr), -1);
        Tcl_DStringAppend(&buffer2,
	        Tcl_GetString(ivPtr->iclsPtr->fullname), -1);
        Tcl_DStringAppend(&buffer2, "::", 2);
        Tcl_DStringAppend(&buffer2,
	        Tcl_GetString(ivPtr->namePtr), -1);
	varName = Tcl_DStringValue(&buffer2);
        lastval = Tcl_GetVar2(interp, varName, (char*)NULL, 0);
        Tcl_DStringSetLength(&buffer, 0);
        Tcl_DStringAppend(&buffer, (lastval) ? lastval : "", -1);

        token = Tcl_GetStringFromObj(objv[i+1], (int*)NULL);

        if (Tcl_SetVar2(interp, varName, (char*)NULL, token,
                TCL_LEAVE_ERR_MSG) == NULL) {

            char msg[256];
            sprintf(msg, "\n    (error in configuration of public variable \"%.100s\")", Tcl_GetString(ivPtr->fullNamePtr));
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
	    Tcl_Namespace *saveNsPtr = Tcl_GetCurrentNamespace(interp);
	    Itcl_SetCallFrameNamespace(interp, ivPtr->iclsPtr->namesp);
	    result = Tcl_EvalObjEx(interp, mcode->bodyPtr, 0);
	    Itcl_SetCallFrameNamespace(interp, saveNsPtr);
            if (result == TCL_OK) {
                Tcl_ResetResult(interp);
            } else {
                char msg[256];
                sprintf(msg, "\n    (error in configuration of public variable \"%.100s\")", Tcl_GetString(ivPtr->fullNamePtr));
                Tcl_AddErrorInfo(interp, msg);

                Tcl_SetVar2(interp, varName,(char*)NULL,
                    Tcl_DStringValue(&buffer), 0);

                goto configureDone;
            }
        }
    }

configureDone:
    Tcl_DStringFree(&buffer2);
    Tcl_DStringFree(&buffer);

    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclWidgetCget()
 *
 *  Invoked whenever the user issues the "cget" method for an object.
 *  If the class is NOT ITCL_IS_CLASS
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
ItclWidgetCget(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;

    CONST char *name;
    CONST char *val;
    ItclVarLookup *vlookup;
    Tcl_HashEntry *hPtr;

    ItclShowArgs(0,"ItclWidgetCget", objc, objv);
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

