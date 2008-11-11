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
 *     RCS:  $Id: itclWidgetBuiltin.c,v 1.1.2.3 2008/11/11 11:37:36 wiede Exp $
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

static char* ItclTraceHullVar(ClientData cdata, Tcl_Interp *interp,
        const char *name1, const char *name2, int flags);

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
 *  ItclTraceHullVar()
 *
 *  Invoked to handle read/write traces on "hull" variables
 *
 *  On write, this procedure returns an error as "hull" may not be modfied
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
static char*
ItclTraceHullVar(
    ClientData clientData,  /* object instance data */
    Tcl_Interp *interp,	    /* interpreter managing this variable */
    const char *name1,    /* variable name */
    const char *name2,    /* unused */
    int flags)		    /* flags indicating read/write */
{
    ItclObject *ioPtr;

    ioPtr = (ItclObject *)clientData;
    /*
     *  Handle write traces "itcl_options"
     */
    if ((flags & TCL_TRACE_WRITES) != 0) {
        return "can't set \"itcl_hull\". The itcl_hull component cannot be redefined";
    }
    return NULL;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_BiInstallHullCmd()
 *
 *  Invoked whenever the user issues the "installhull" method for an object.
 *  Handles the following syntax:
 *
 *    <objName> installhall using <widgetType> ?arg ...?
 *    <objName> installhall name
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
    Tcl_HashEntry *hPtr;
    Tcl_Obj *namePtr;
    Tcl_Var varPtr;
    Tcl_HashSearch place;
    Tcl_DString buffer;
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
    char *token;
    int shortForm;
    int result;

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
            "improper usage: should be \"", 
	    "object installhull using <widgetType> ?arg ...?\"",
            (char*)NULL);
        return TCL_ERROR;
    }
    if (objc < 5) {
	if (objc != 2) {
            token = Tcl_GetString(objv[0]);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "wrong # args: should be \"object ", token,
	        "name|using <widgetType> ?arg ...?\"", (char*)NULL);
            return TCL_ERROR;
        } 
    }
    shortForm = 0;
    widgetName = Tcl_GetString(contextIoPtr->namePtr);
    if (objc == 2) {
        shortForm = 1;
        widgetName = Tcl_GetString(objv[1]);
    }
    Tcl_DStringInit(&buffer);
    if (!shortForm) {
        widgetType = Tcl_GetString(objv[2]);
        if (strcmp(Tcl_GetString(objv[3]), "-class") != 0) {
            token = Tcl_GetString(objv[0]);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "wrong # args: should be \"object ", token,
                " using <widgetType> ?arg ...?\"", (char*)NULL);
            return TCL_ERROR;
        }
        className = Tcl_GetString(objv[4]);
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
    }

    /* initialize the options array */
    tkMainWin = Tk_MainWindow(interp);
    tkWin = Tk_NameToWindow(interp, widgetName,
            tkMainWin);
    if (tkWin == NULL) {
        Tcl_AppendResult(interp, "cannot find window \"",
	        Tcl_GetString(contextIoPtr->namePtr), "\"", NULL);
	return TCL_ERROR;
    }
    hPtr = Tcl_FirstHashEntry(&contextIclsPtr->options, &place);
    while (hPtr) {
	ioptPtr = (ItclOption*)Tcl_GetHashValue(hPtr);
        val = Tk_GetOption(tkWin, Tcl_GetString(ioptPtr->resourceNamePtr),
	        Tcl_GetString(ioptPtr->classNamePtr));
	if (val != NULL) {
            val = ItclSetInstanceVar(interp, "itcl_options",
	            Tcl_GetString(ioptPtr->namePtr), val,
                    contextIoPtr, contextIoPtr->iclsPtr);
	} else {
	    if (ioptPtr->defaultValuePtr != NULL) {
                val = ItclSetInstanceVar(interp, "itcl_options",
	                Tcl_GetString(ioptPtr->namePtr),
		        Tcl_GetString(ioptPtr->defaultValuePtr),
                        contextIoPtr, contextIoPtr->iclsPtr);
	    }
	}
        hPtr = Tcl_NextHashEntry(&place);
    }

    /* initialize the itcl_hull variable */
    Tcl_DStringAppend(&buffer, "::itcl::widget::internal::hull", -1);
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
    contextIoPtr->hullWindowNamePtr = Tcl_NewStringObj(widgetName, -1);
    Itcl_RenameCommand(interp, widgetName,
            Tcl_DStringValue(&buffer));

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
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectVariables, (char *)ivPtr);
    varPtr = Tcl_GetHashValue(hPtr);
    val = ItclSetInstanceVar(interp, "itcl_hull", NULL,
            Tcl_DStringValue(&buffer), contextIoPtr, contextIoPtr->iclsPtr);
    if (val == NULL) {
        Tcl_AppendResult(interp, "cannot set itcl_hull for object \"",
            Tcl_GetString(contextIoPtr->namePtr), "\"", NULL);
        Tcl_DStringFree(&buffer);
        return TCL_ERROR;
    }
    /* now set the write trace on the itcl_hull variable */
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, Tcl_GetString(contextIoPtr->varNsNamePtr), -1);
    Tcl_DStringAppend(&buffer, Tcl_GetString(contextIclsPtr->fullNamePtr), -1);
    Tcl_DStringAppend(&buffer, "::", -1);
    Tcl_DStringAppend(&buffer, Tcl_GetString(ivPtr->namePtr), -1);
    Tcl_TraceVar2(interp, Tcl_DStringValue(&buffer), NULL,
             TCL_TRACE_WRITES, ItclTraceHullVar, contextIoPtr);
    Tcl_DStringFree(&buffer);
    return result;
}
