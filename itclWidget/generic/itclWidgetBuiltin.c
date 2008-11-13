/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  These procedures handle built-in class methods, including the
 *  "installhull" method for package ItclWidget
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetBuiltin.c,v 1.1.2.6 2008/11/13 19:56:13 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007 Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclWidgetInt.h"
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

Tcl_CommandTraceProc ItclHullContentsDeleted;



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
void ItclHullContentsDeleted(
    ClientData clientData,
    Tcl_Interp *interp,
    const char *oldName,
    const char *newName,
    int flags)
{
    ItclObject *ioPtr;
    int result;

    ioPtr = (ItclObject *)clientData;
    if (newName == NULL) {
        /* delete the object which has this as a itcl_hull contents */
/*
fprintf(stderr, " DELETE OBJECT!%s!%s!\n", Tcl_GetString(ioPtr->namePtr), Tcl_GetString(ioPtr->origNamePtr));
*/
        result = Itcl_RenameCommand(ioPtr->iclsPtr->interp,
	        Tcl_GetString(ioPtr->origNamePtr), NULL);
/*
fprintf(stderr, "RES!%d!%s!\n", result, Tcl_GetStringResult(ioPtr->iclsPtr->interp));
*/
    }
}

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
    Tcl_Interp *interp,      /* current interpreter */
    ItclObjectInfo *infoPtr)
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
	        BiMethodList[i].proc, (ClientData)infoPtr,
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
 *    installhall using <widgetType> ?arg ...?
 *    installhall name
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_BiInstallHullCmd(
    ClientData clientData,   /* ItclObjectInfo *Ptr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *const objv[])   /* argument objects */
{
    FOREACH_HASH_DECLS;
    Tcl_Obj *namePtr;
    Tcl_Obj *classNamePtr;
    Tcl_Obj *widgetNamePtr;
    Tcl_Var varPtr;
    Tcl_DString buffer;
    Tcl_Obj **newObjv;
    Tk_Window tkMainWin;
    Tk_Window tkWin;
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;
    ItclObjectInfo *infoPtr;
    ItclVariable *ivPtr;
    ItclOption *ioptPtr;
    const char *val;
    const char *widgetType;
    const char *className;
    const char *widgetName;
    const char *origWidgetName;
    char *token;
    int newObjc;
    int lgth;
    int i;
    int shortForm;
    int numOptArgs;
    int optsStartIdx;
    int result;

    result = TCL_OK;
    ItclShowArgs(1, "Itcl_BiInstallHullCmd", objc, objv);
    infoPtr = (ItclObjectInfo *)clientData;
    if (infoPtr->buildingWidget) {
	contextIoPtr = infoPtr->currIoPtr;
        contextIclsPtr = contextIoPtr->iclsPtr;
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
            "cannot installhull without an object context", 
            (char*)NULL);
        return TCL_ERROR;
    }
    if (objc < 3) {
	if (objc != 2) {
            token = Tcl_GetString(objv[0]);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "wrong # args: should be \"", token,
	        "\" name|using <widgetType> ?arg ...?\"", (char*)NULL);
            return TCL_ERROR;
	}
    }
    shortForm = 0;
    widgetName = Tcl_GetString(contextIoPtr->namePtr);
    origWidgetName = widgetName;
    if (objc == 2) {
        shortForm = 1;
        widgetName = Tcl_GetString(objv[1]);
    }
    const char *wName;
    wName = strstr(widgetName, "::");
    if (wName != NULL) {
        widgetName = wName + 2;
    }

    if (!shortForm) {
	widgetNamePtr = Tcl_NewStringObj(widgetName, -1);
	if (contextIclsPtr->flags & ITCL_WIDGETADAPTOR) {
	/* FIXME that code is only temporary until hijacking of hull works */
	    Tcl_AppendToObj(widgetNamePtr, "___", -1);
	}
	widgetName = Tcl_GetString(widgetNamePtr);
        widgetType = Tcl_GetString(objv[2]);
	classNamePtr = NULL;
	className = NULL;
	optsStartIdx = 3;
	if (objc > 3) {
            if (strcmp(Tcl_GetString(objv[3]), "-class") == 0) {
                className = Tcl_GetString(objv[4]);
	        optsStartIdx += 2;
	    }
	}
	if (className == NULL) {
	    classNamePtr = ItclCapitalize(widgetType);
	    className = Tcl_GetString(classNamePtr);
        }
	numOptArgs = objc - optsStartIdx;
	newObjc = 4;
	newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *) *
	        (newObjc + numOptArgs));
	newObjv[0] = Tcl_NewStringObj(widgetType, -1);
	Tcl_IncrRefCount(newObjv[0]);
	newObjv[1] = widgetNamePtr;
	Tcl_IncrRefCount(newObjv[1]);
	newObjv[2] = Tcl_NewStringObj("-class", -1);
	Tcl_IncrRefCount(newObjv[2]);
	newObjv[3] = Tcl_NewStringObj(className, -1);
	Tcl_IncrRefCount(newObjv[3]);
	i = 4;
	for (; optsStartIdx < objc; optsStartIdx++, i++) {
	    newObjv[i] = objv[optsStartIdx];
	    Tcl_IncrRefCount(newObjv[i]);
	}
	ItclShowArgs(1, "HullCreate", newObjc + numOptArgs, newObjv);
        result = Tcl_EvalObjv(interp, newObjc + numOptArgs, newObjv, 0);
	for (i = newObjc + numOptArgs - 1; i > 3; i--) {
	    Tcl_DecrRefCount(newObjv[i]);
	}
	Tcl_IncrRefCount(newObjv[3]);
	Tcl_IncrRefCount(newObjv[2]);
	Tcl_IncrRefCount(newObjv[1]);
	Tcl_IncrRefCount(newObjv[0]);
	ckfree((char *)newObjv);
	if (classNamePtr != NULL) {
	    Tcl_DecrRefCount(classNamePtr);
	}

	/* now initialize the itcl_options array */
        tkMainWin = Tk_MainWindow(interp);
        tkWin = Tk_NameToWindow(interp, origWidgetName, tkMainWin);
        if (tkWin != NULL) {
	    const char *val2;
            FOREACH_HASH_VALUE(ioptPtr, &contextIclsPtr->options) {
                val = Tk_GetOption(tkWin, Tcl_GetString(
		        ioptPtr->resourceNamePtr),
	                Tcl_GetString(ioptPtr->classNamePtr));
	        if (val != NULL) {
                    val = ItclSetInstanceVar(interp, "itcl_options",
	                    Tcl_GetString(ioptPtr->namePtr), val,
                            contextIoPtr, contextIoPtr->iclsPtr);
                    val2 = ItclGetInstanceVar(interp, "itcl_options",
	                    Tcl_GetString(ioptPtr->namePtr),
                            contextIoPtr, contextIoPtr->iclsPtr);
	        } else {
	            if (ioptPtr->defaultValuePtr != NULL) {
                        val = ItclSetInstanceVar(interp, "itcl_options",
	                        Tcl_GetString(ioptPtr->namePtr),
		                Tcl_GetString(ioptPtr->defaultValuePtr),
                                contextIoPtr, contextIoPtr->iclsPtr);
	            }
		}
	    }
        }
        Tcl_DecrRefCount(widgetNamePtr);
    }

    /* initialize the itcl_hull variable */
    i = 0;
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, ITCL_WIDGETS_NAMESPACE"::hull", -1);
    lgth = strlen(Tcl_DStringValue(&buffer));
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
    contextIoPtr->hullWindowNamePtr = Tcl_NewStringObj(widgetName, -1);
/* fprintf(stderr, "REN!%s!%s!\n", widgetName, Tcl_DStringValue(&buffer)); */
    Itcl_RenameCommand(interp, widgetName,
            Tcl_DStringValue(&buffer));
    result = Tcl_TraceCommand(interp, Tcl_DStringValue(&buffer),
            TCL_TRACE_RENAME|TCL_TRACE_DELETE,
            ItclHullContentsDeleted, contextIoPtr);

    namePtr = Tcl_NewStringObj("itcl_hull", -1);
    Tcl_IncrRefCount(namePtr);
    hPtr = Tcl_FindHashEntry(&contextIoPtr->iclsPtr->variables,
            (char *)namePtr);
    Tcl_DecrRefCount(namePtr);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "cannot find class variable itcl_hull", NULL);
        return TCL_ERROR;
    }
    ivPtr =Tcl_GetHashValue(hPtr);
    if (ivPtr->initted <= 1) {
        ivPtr->initted = 0;
        hPtr = Tcl_FindHashEntry(&contextIoPtr->objectVariables, (char *)ivPtr);
        varPtr = Tcl_GetHashValue(hPtr);
        val = ItclSetInstanceVar(interp, "itcl_hull", NULL,
                Tcl_DStringValue(&buffer), contextIoPtr, contextIoPtr->iclsPtr);
        ivPtr->initted = 2;
        if (val == NULL) {
            Tcl_AppendResult(interp, "cannot set itcl_hull for object \"",
                Tcl_GetString(contextIoPtr->namePtr), "\"", NULL);
            Tcl_DStringFree(&buffer);
            return TCL_ERROR;
        }
    }
    return result;
}
