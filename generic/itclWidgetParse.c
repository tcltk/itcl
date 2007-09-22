/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  [incr Tcl] provides object-oriented extensions to Tcl, much as
 *
 *  Procedures in this file support the new syntax for commands
 *  for class definitions of package ItclWidget:
 *
 *    itcl::type <className> {
 *    }
 *    itcl::widgetadaptor <className> {
 *    }
 *    itcl::widget <className> {
 *        inherit <base-class>...
 *
 *        delegate method/option to component as script
 *        delegate method/option to component using script
 *
 *        option {<nameSpec>} ?{value}? ?-cgetmethod {<name>}?
 *                ?-configuremethod {<name>}? ?-validatemethod {<name>}?
 *                ?-readonly {<boolean>}?
 *        component {<componentname>}
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
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetParse.c,v 1.1.2.9 2007/09/22 13:39:23 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007  Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * ------------------------------------------------------------------------
 */

#include "itclInt.h"

Tcl_ObjCmdProc Itcl_ClassComponentCmd;
Tcl_ObjCmdProc Itcl_ClassComponentInstallCmd;
Tcl_ObjCmdProc Itcl_ClassDelegateMethodCmd;
Tcl_ObjCmdProc Itcl_ClassDelegateOptionCmd;
Tcl_ObjCmdProc Itcl_ClassDelegateProcCmd;
Tcl_ObjCmdProc Itcl_ClassWidgetClassCmd;
Tcl_ObjCmdProc Itcl_ClassOptionCmd;
Tcl_ObjCmdProc Itcl_ClassHullTypeCmd;
Tcl_ObjCmdProc Itcl_WidgetCmd;
Tcl_ObjCmdProc Itcl_WidgetAdaptorCmd;
Tcl_ObjCmdProc Itcl_TypeCmd;

static const struct {
    const char *name;
    Tcl_ObjCmdProc *objProc;
} parseCmds[] = {
    {"component", Itcl_ClassComponentCmd},
    {"componentinstall", Itcl_ClassComponentInstallCmd},
    {"hulltype", Itcl_ClassHullTypeCmd},
    {"option", Itcl_ClassOptionCmd},
    {"widgetclass", Itcl_ClassWidgetClassCmd},
    {NULL, NULL}
};


/*
 * ------------------------------------------------------------------------
 *  Itcl_WidgetParseInit()
 *
 *  Invoked by Itcl_Init() whenever a new interpeter is created to add
 *  [incr Tcl] facilities.  Adds the commands needed to parse class
 *  definitions.
 * ------------------------------------------------------------------------
 */
int
Itcl_WidgetParseInit(
    Tcl_Interp *interp,     /* interpreter to be updated */
    ItclObjectInfo *infoPtr) /* info regarding all known objects and classes */
{
    Tcl_DString buffer;
    int i;

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

    Tcl_CreateObjCommand(interp, "::itcl::type", Itcl_TypeCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::widget", Itcl_WidgetCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::widgetadaptor", Itcl_WidgetAdaptorCmd,
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
    if (iclsPtr->flags & ITCL_IS_CLASS) {
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
 *  Itcl_ClassComponentInstallCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "componentinstall" command is invoked to define a componentinstall 
 *  Handles the following syntax:
 *
 *      componentinstall 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassComponentInstallCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
//    Tcl_Obj **newObjv;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
//    ItclComponent *icPtr;
//    const char *usage;
//    int inherit;
//    int newObjc;

    ItclShowArgs(0, "Itcl_ClassComponentInstallCmd", objc, objv);
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
iclsPtr = NULL;
    Tcl_AppendResult(interp, "componentinstall not yet implemented", NULL);
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassHullTypeCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "hulltype" command is invoked to define a hulltype 
 *  Handles the following syntax:
 *
 *      hulltype 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassHullTypeCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int correctArg;

    ItclShowArgs(1, "Itcl_ClassHullTypeCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (!(iclsPtr->flags & ITCL_IS_WIDGET)) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget.", 
		" Only an ::itcl::widget can have a hulltype", NULL);
	return TCL_ERROR;
    }
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, " frame/toplevel");
	return TCL_ERROR;
    }
    correctArg = 0;
    if (strcmp(Tcl_GetString(objv[1]), "frame") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_IS_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "labelframe") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_IS_LABEL_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "toplevel") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_IS_TOPLEVEL;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "ttk::frame") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_IS_TTK_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "ttk::labelframe") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_IS_TTK_LABEL_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "ttk::toplevel") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_IS_TTK_TOPLEVEL;
        correctArg = 1;
    }
    if (!correctArg) {
        Tcl_AppendResult(interp,
	        "syntax: must be hulltype frame or toplevel",
	        NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassWidgetClassCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "widgetclass" command is invoked to define a widgetclass 
 *  Handles the following syntax:
 *
 *      widgetclass 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassWidgetClassCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;

    ItclShowArgs(1, "Itcl_ClassWidgetClassCmd", objc, objv);
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "<widgetclass>");
        return TCL_ERROR;
    }
    if (!(iclsPtr->flags & ITCL_IS_WIDGET)) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget.", 
		" Only an ::itcl::widget can have a widgetclass", NULL);
	return TCL_ERROR;
    }
    iclsPtr->widgetClassPtr = objv[1];
    Tcl_IncrRefCount(iclsPtr->widgetClassPtr);
    /* FIX ME !! */
    /* maybe have to check for valid widget class name ?? */
    Tcl_AppendResult(interp, "not yet completely implemented", NULL);
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
    if (iclsPtr->flags & ITCL_IS_CLASS) {
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
	    Tcl_AppendResult(interp, "bad option\"", token, "\" should be ",
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
    /* FIX ME !!! */
    /* check for already delegated */

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
	    targetPtr = Tcl_NewStringObj(methodName, -1);
	}
        /* check for valid method name */
        // ItclIsValidMethodName(methodName);
        /* check for locally defined option */
        methodNamePtr = Tcl_NewStringObj(methodName, -1);
	Tcl_IncrRefCount(methodNamePtr);
	hPtr = Tcl_FindHashEntry(&iclsPtr->functions, (char *)methodNamePtr);
	if (hPtr != NULL) {
	    Tcl_AppendResult(interp, "method \"", methodName,
	            "\" has been defined locally", NULL);
	    return TCL_ERROR;
	}
        idmPtr->namePtr = methodNamePtr;

    } else {
        methodNamePtr = Tcl_NewStringObj("*", -1);
	Tcl_IncrRefCount(methodNamePtr);
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
	    /* FIX ME !!! */
	    /* set the hash value to theItclMemberFunc value here !! */
	}
    }
    hPtr = Tcl_CreateHashEntry(&iclsPtr->delegatedFunctions,
            (char *)idmPtr->namePtr, &isNew);
    Tcl_SetHashValue(hPtr, idmPtr);
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
    Tcl_Obj *optionNamePtr;
    Tcl_Obj *componentPtr;
    Tcl_Obj *targetPtr;
    Tcl_Obj *exceptionsPtr;
    Tcl_Obj *resourceNamePtr;
    Tcl_Obj *classNamePtr;
    Tcl_HashEntry *hPtr;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclComponent *icPtr;
    ItclDelegatedOption *idoPtr;
    const char *usageStr;
    const char *option;
    const char *component;
    const char *token;
    const char **argv;
    int foundOpt;
    int argc;
    int isNew;
    int i;

    ItclShowArgs(1, "Itcl_ClassDelegateOptionCmd", objc, objv);
    usageStr = "<optionDef> to <targetDef> ?as <script>? ?except <script>?";
    if (objc < 4) {
	Tcl_AppendResult(interp, "wrong # args should be ", usageStr, NULL);
        return TCL_ERROR;
    }
    infoPtr = (ItclObjectInfo *)clientData;
    iclsPtr = (ItclClass *)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_IS_CLASS) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget/::itcl::widgetadaptor/::itcl::type.", 
		" Only these can delegate options", NULL);
	return TCL_ERROR;
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
	    Tcl_AppendResult(interp, "bad option\"", token, "\" should be ",
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
    if ((*option == '*') && (exceptionsPtr != NULL)) {
	Tcl_AppendResult(interp,
	        "cannot specify \"except\" with \"delegate option *\"", NULL);
	return TCL_ERROR;
    }
    /* FIX ME !!! */
    /* check for already delegated */

    if (ItclCreateComponent(interp, iclsPtr, componentPtr, &icPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    idoPtr = (ItclDelegatedOption *)ckalloc(sizeof(ItclDelegatedOption));
    memset(idoPtr, 0, sizeof(ItclDelegatedOption));
    Tcl_InitObjHashTable(&idoPtr->exceptions);
    if (*option != '*') {
        if (targetPtr == NULL) {
	    targetPtr = Tcl_NewStringObj(option, -1);
	}
        if (resourceNamePtr == NULL) {
	    resourceNamePtr = Tcl_NewStringObj(option+1, -1);
	    Tcl_IncrRefCount(resourceNamePtr);
	}
        if (classNamePtr == NULL) {
	    classNamePtr = ItclCapitalize(option+1);
	}
        /* check for valid option name */
        // ItclIsValidOptionName(option);
        /* check for locally defined option */
        optionNamePtr = Tcl_NewStringObj(option, -1);
	Tcl_IncrRefCount(optionNamePtr);
	hPtr = Tcl_FindHashEntry(&iclsPtr->options, (char *)optionNamePtr);
	if (hPtr != NULL) {
	    Tcl_AppendResult(interp, "option \"", option,
	            "\" has been defined locally", NULL);
	    return TCL_ERROR;
	}
        idoPtr->namePtr = optionNamePtr;
        idoPtr->resourceNamePtr = resourceNamePtr;
        idoPtr->classNamePtr = classNamePtr;

    } else {
        optionNamePtr = Tcl_NewStringObj("*", -1);
	Tcl_IncrRefCount(optionNamePtr);
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
	    /* FIX ME !!! */
	    /* set the hash value to the ItclOption value here !! */
	}
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
    if (iclsPtr->flags & ITCL_IS_CLASS) {
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
	    Tcl_AppendResult(interp, "bad option\"", token, "\" should be ",
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
    /* FIX ME !!! */
    /* check for already delegated */

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
	    targetPtr = Tcl_NewStringObj(procName, -1);
	}
        /* check for valid proc name */
        // ItclIsValidMethodName(procName);
        /* check for locally defined option */
        procNamePtr = Tcl_NewStringObj(procName, -1);
	Tcl_IncrRefCount(procNamePtr);
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
	    /* FIX ME !!! */
	    /* set the hash value to theItclMemberFunc value here !! */
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
    int argc;
    int pLevel;
    int readOnly;
    int newObjc;
    int foundOption;
    int i;

    ItclShowArgs(1, "Itcl_ClassOptionCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr->flags & ITCL_IS_CLASS) {
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
    int needCapitalize = 0;
    if (argc > 2) {
        className = argv[2];
    } else {
	/* class name defaults to option name minus hyphen and capitalized */
        className = name+1;
        needCapitalize = 1;
    }
    init = defaultValue;
    if ((newObjc > 1) && (init == NULL)) {
        init = Tcl_GetString(newObjv[1]);
    }

    namePtr = Tcl_NewStringObj(name, -1);
    Tcl_IncrRefCount(namePtr);
    if (Itcl_CreateOption(interp, iclsPtr, namePtr, resourceName, className, 
            init, configureMethod, &ioptPtr) != TCL_OK) {
        return TCL_ERROR;
    }
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

