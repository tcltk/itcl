/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  These procedures handle built-in class methods, including the
 *  "hullinstall" method for package ItclWidget
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetBuiltin.c,v 1.1.2.6 2007/09/16 20:12:59 wiede Exp $
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
    { "hullinstall", "using widgetType ?arg ...?",
                   "@itcl-builtin-hullinstall",  Itcl_BiHullInstallCmd },
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
 *  Itcl_BiHullInstallCmd()
 *
 *  Invoked whenever the user issues the "hullinstall" method for an object.
 *  Handles the following syntax:
 *
 *    <objName> hullinstall using <widgetType> ?arg ...?
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiHullInstallCmd(
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

    ItclShowArgs(1, "Itcl_BiHullInstallCmd", objc, objv);
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
	    "object hullinstall using <widgetType> ?arg ...?\"",
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
            val = ItclSetInstanceVar(interp, "options",
	            Tcl_GetString(ioptPtr->namePtr), val,
                    contextIoPtr, contextIoPtr->iclsPtr);
	}
	if (ioptPtr->init != NULL) {
            val = ItclSetInstanceVar(interp, "options",
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
    Tcl_RenameCommand(interp, Tcl_GetString(contextIoPtr->namePtr),
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
    val = ItclSetInstanceVar(interp, "hull", NULL, Tcl_DStringValue(&buffer),
            contextIoPtr, contextIoPtr->iclsPtr);
    Tcl_DStringFree(&buffer);
    if (val == NULL) {
        Tcl_AppendResult(interp, "cannot set hull for object \"",
            Tcl_GetString(contextIoPtr->namePtr), "\"", NULL);
        return TCL_ERROR;
    }
    return result;
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

    Tcl_HashEntry *hPtr;
    Tcl_DString buffer;
    ItclVarLookup *vlookup;
    ItclDelegatedMethod *idmPtr;
    ItclComponent *icPtr;
    ItclOption *ioptPtr;
    const char *val;
    char *token;
    int i;
    int result;

    ItclShowArgs(1, "ItclWidgetConfigure", objc, objv);
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

    hPtr = NULL;
    Tcl_Obj *methodNamePtr;
    methodNamePtr = Tcl_NewStringObj("*", -1);
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedMethods, (char *)
            methodNamePtr);
    if (hPtr != NULL) {
        idmPtr = (ItclDelegatedMethod *)Tcl_GetHashValue(hPtr);
	Tcl_SetStringObj(methodNamePtr, "configure", -1);
        hPtr = Tcl_FindHashEntry(&idmPtr->exceptions, (char *)methodNamePtr);
        if (hPtr == NULL) {
	    icPtr = idmPtr->icPtr;
	    val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
	            NULL, contextIoPtr, contextIclsPtr);
            if (val != NULL) {
	        Tcl_DString buffer;
	        Tcl_DStringInit(&buffer);
	        Tcl_DStringAppend(&buffer, val, -1);
	        Tcl_DStringAppend(&buffer, " configure ", -1);
		for(i=2;i<objc;i++) {
		    Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i]), -1);
		    Tcl_DStringAppend(&buffer, " ", 1);
		}
                result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
                return result;
	    }
	}
    }
    /* now do the hard work */
    if (objc == 1) {
fprintf(stderr, "plain configure not yet implemented\n");
    }
    /* first handle delegated options */
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedMethods, (char *)
            objv[1]);
    if (hPtr != NULL) {
        idmPtr = (ItclDelegatedMethod *)Tcl_GetHashValue(hPtr);
        icPtr = idmPtr->icPtr;
        val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
                NULL, contextIoPtr, contextIclsPtr);
        if (val != NULL) {
            Tcl_DString buffer;
            Tcl_DStringInit(&buffer);
	    Tcl_DStringAppend(&buffer, val, -1);
	    Tcl_DStringAppend(&buffer, " configure ", -1);
	    for(i=2;i<objc;i++) {
	        Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i]), -1);
	        Tcl_DStringAppend(&buffer, " ", 1);
	    }
            result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
            return result;
        }
    }
    /* now look if it is an option at all */
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->options, (char *) objv[1]);
    if (hPtr == NULL) {
	/* no option at all, let the normal configure do the job */
	return TCL_CONTINUE;
    }
    ioptPtr = (ItclOption *)Tcl_GetHashValue(hPtr);
    if (objc == 2) {
        /* return info for an option */
	/* FIX ME temporary !!! */
        return TCL_OK;
    }
    Tcl_DStringInit(&buffer);
    result = TCL_CONTINUE;
    /* set one or more options */
    for (i=1; i < objc; i+=2) {
	if (i+1 > objc) {
	    Tcl_AppendResult(interp, "need option value pair", NULL);
	    result = TCL_ERROR;
	    break;
	}
        hPtr = Tcl_FindHashEntry(&contextIclsPtr->options, (char *) objv[i]);
        if (hPtr == NULL) {
	    /* check if normal public variable/common */
	    /* FIX ME !!! temporary */
	    result = TCL_CONTINUE;
	    break;
        }
        ioptPtr = (ItclOption *)Tcl_GetHashValue(hPtr);
        if (ioptPtr->validateMethodPtr != NULL) {
            Tcl_DStringAppend(&buffer, Tcl_GetString(
	            ioptPtr->validateMethodPtr), -1);
            Tcl_DStringAppend(&buffer, " ", -1);
            Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i]), -1);
            Tcl_DStringAppend(&buffer, " ", -1);
            Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i+1]), -1);
	    result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
	    if (result != TCL_OK) {
	        break;
	    }
	}
        Tcl_DStringFree(&buffer);
        if (ioptPtr->configureMethodPtr != NULL) {
            Tcl_DStringAppend(&buffer, Tcl_GetString(
	            ioptPtr->configureMethodPtr), -1);
            Tcl_DStringAppend(&buffer, " ", -1);
            Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i]), -1);
            Tcl_DStringAppend(&buffer, " ", -1);
            Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i+1]), -1);
	    result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
	    if (result != TCL_OK) {
	        break;
	    }
	} else {
	    if (ItclSetInstanceVar(interp, "options", Tcl_GetString(objv[i]),
	            Tcl_GetString(objv[i+1]), contextIoPtr, contextIclsPtr)
		    == NULL) {
		result = TCL_ERROR;
	        break;
	    }
	}
        Tcl_DStringFree(&buffer);
        result = TCL_OK;
    }
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

    Tcl_DString buffer;
    Tcl_HashEntry *hPtr;
    ItclDelegatedMethod *idmPtr;
    ItclComponent *icPtr;
    ItclOption *ioptPtr;
    const char *val;
    int i;
    int result;

    ItclShowArgs(1,"ItclWidgetCget", objc, objv);
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

    hPtr = NULL;
    Tcl_Obj *methodNamePtr;
    methodNamePtr = Tcl_NewStringObj("*", -1);
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedMethods, (char *)
            methodNamePtr);
    if (hPtr != NULL) {
        idmPtr = (ItclDelegatedMethod *)Tcl_GetHashValue(hPtr);
	Tcl_SetStringObj(methodNamePtr, "cget", -1);
        hPtr = Tcl_FindHashEntry(&idmPtr->exceptions, (char *)methodNamePtr);
        if (hPtr == NULL) {
	    icPtr = idmPtr->icPtr;
	    val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
	            NULL, contextIoPtr, contextIclsPtr);
            if (val != NULL) {
	        Tcl_DString buffer;
	        Tcl_DStringInit(&buffer);
	        Tcl_DStringAppend(&buffer, val, -1);
	        Tcl_DStringAppend(&buffer, " cget ", -1);
		for(i=2;i<objc;i++) {
		    Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i]), -1);
		    Tcl_DStringAppend(&buffer, " ", 1);
		}
                result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
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
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedMethods, (char *)
            objv[1]);
    if (hPtr != NULL) {
        idmPtr = (ItclDelegatedMethod *)Tcl_GetHashValue(hPtr);
        icPtr = idmPtr->icPtr;
        val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
                NULL, contextIoPtr, contextIclsPtr);
        if (val != NULL) {
            Tcl_DString buffer;
            Tcl_DStringInit(&buffer);
	    Tcl_DStringAppend(&buffer, val, -1);
	    Tcl_DStringAppend(&buffer, " cget ", -1);
	    for(i=2;i<objc;i++) {
	        Tcl_DStringAppend(&buffer, Tcl_GetString(objv[i]), -1);
	        Tcl_DStringAppend(&buffer, " ", 1);
	    }
            result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
            return result;
        }
    }
    /* now look if it is an option at all */
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->options, (char *) objv[1]);
    if (hPtr == NULL) {
	/* no option at all, let the normal configure do the job */
	return TCL_CONTINUE;
    }
    ioptPtr = (ItclOption *)Tcl_GetHashValue(hPtr);
    Tcl_DStringInit(&buffer);
    result = TCL_CONTINUE;
    ioptPtr = (ItclOption *)Tcl_GetHashValue(hPtr);
    if (ioptPtr->cgetMethodPtr != NULL) {
        Tcl_DStringAppend(&buffer, Tcl_GetString(
                ioptPtr->cgetMethodPtr), -1);
        Tcl_DStringAppend(&buffer, " ", -1);
        Tcl_DStringAppend(&buffer, Tcl_GetString(objv[1]), -1);
	result = Tcl_Eval(interp, Tcl_DStringValue(&buffer));
        Tcl_DStringFree(&buffer);
    } else {
	if (ItclGetInstanceVar(interp, "options", Tcl_GetString(objv[i]),
	        contextIoPtr, contextIclsPtr) == NULL) {
	    result = TCL_ERROR;
	} else {
            result = TCL_OK;
        }
    }
    Tcl_DStringFree(&buffer);
    return result;
}

