/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  Itclng
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  This file defines the C-API calls for creating classes, class methods
 *  procs, variables, commons, options etc. 
 *
 *  overhauled version author: Arnulf Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * ------------------------------------------------------------------------
 */
#include "itclngInt.h"

Tcl_ObjCmdProc Itclng_CreateClassCmd;
Tcl_ObjCmdProc Itclng_CreateClassFinishCmd;
Tcl_ObjCmdProc Itclng_CreateClassMethodCmd;
Tcl_ObjCmdProc Itclng_CreateClassProcCmd;
Tcl_ObjCmdProc Itclng_CreateClassCommonCmd;
Tcl_ObjCmdProc Itclng_CreateClassVariableCmd;
Tcl_ObjCmdProc Itclng_CreateClassOptionCmd;
Tcl_ObjCmdProc Itclng_CreateClassMethodVariableCmd;
Tcl_ObjCmdProc Itclng_CreateClassInheritCmd;
Tcl_ObjCmdProc Itclng_CreateClassConstructorCmd;
Tcl_ObjCmdProc Itclng_CreateClassDestructorCmd;
Tcl_ObjCmdProc Itclng_CreateClassConstructorInitCmd;
Tcl_ObjCmdProc Itclng_CreateObjectCmd;
Tcl_ObjCmdProc Itclng_ConfigureCmd;
Tcl_ObjCmdProc Itclng_CgetCmd;
Tcl_ObjCmdProc Itclng_GetContextCmd;

typedef struct InfoMethod {
    char* commandName;       /* method name */
    char* usage;             /* string describing usage */
    Tcl_ObjCmdProc *proc;    /* implementation C proc */
} InfoMethod;

static InfoMethod ItclngMethodList[] = {
    { "createClass",
      "fullClassName baseClassName",
      Itclng_CreateClassCmd },
    { "createClassFinish",
      "fullClassName resultValue",
      Itclng_CreateClassFinishCmd },
    { "createClassMethod",
      "fullClassName methodName",
      Itclng_CreateClassMethodCmd },
    { "createClassProc",
      "fullClassName procName",
      Itclng_CreateClassProcCmd },
    { "createClassCommon",
      "fullClassName commonName",
      Itclng_CreateClassCommonCmd },
    { "createClassVariable",
      "fullClassName variableName",
      Itclng_CreateClassVariableCmd },
    { "createClassOption",
      "fullClassName variableName",
      Itclng_CreateClassOptionCmd },
    { "createClassMethodVariable",
      "fullClassName methodVariableName",
      Itclng_CreateClassMethodVariableCmd },
    { "createClassInherit",
      "fullClassName className ?className ...?",
      Itclng_CreateClassInheritCmd },
    { "createObject",
      "fullClassName objectName ?arg arg ... ?",
      Itclng_CreateObjectCmd },
    { "configure",
      "fullClassName ?arg arg ... ?",
      Itclng_ConfigureCmd },
    { "cget",
      "fullClassName ?arg arg ... ?",
      Itclng_CgetCmd },
    { "createClassConstructor",
      "fullClassName constructor",
      Itclng_CreateClassConstructorCmd },
    { "createClassConstructorInit",
      "fullClassName ___constructor_init",
      Itclng_CreateClassConstructorInitCmd},
    { "createClassDestructor",
      "fullClassName destructor",
      Itclng_CreateClassDestructorCmd },
    { "getContext",
      "",
      Itclng_GetContextCmd },
    { NULL, NULL, NULL }
};

/*
 * ------------------------------------------------------------------------
 *  Itclng_InitCommands()
 *
 *      that is the starting point when loading the library
 *      it initializes all internal stuff
 *
 * ------------------------------------------------------------------------
 */

int
Itclng_InitCommands (
    Tcl_Interp *interp,
    ItclngObjectInfo *infoPtr)
{
    Tcl_Obj *cmdNamePtr;
    Tcl_Obj *unkObjPtr;
    Tcl_Obj *ensObjPtr;
    Tcl_Namespace *nsPtr;
    Tcl_Command cmd;
    int i;

    /*
     * Build the ensemble used to implement internal commands.
     */

    nsPtr = Tcl_FindNamespace(interp, Tcl_GetString(infoPtr->internalCmds),
            NULL, 0);
    if (nsPtr == NULL) {
        Tcl_Panic("ITCLNG: error in getting namespace for internal commands");
    }
    cmd = Tcl_CreateEnsemble(interp, nsPtr->fullName, nsPtr,
        TCL_ENSEMBLE_PREFIX);
    Tcl_Export(interp, nsPtr, "[a-z]*", 1);
    for (i=0; ItclngMethodList[i].commandName!=NULL; i++) {
	cmdNamePtr = Tcl_NewStringObj(Tcl_GetString(infoPtr->internalCmds), -1);
	Tcl_AppendToObj(cmdNamePtr, "::", 2);
	Tcl_AppendToObj(cmdNamePtr, ItclngMethodList[i].commandName, -1);
        Tcl_CreateObjCommand(interp, Tcl_GetString(cmdNamePtr),
                ItclngMethodList[i].proc, infoPtr, NULL);
        Tcl_DecrRefCount(cmdNamePtr);
    }
    ensObjPtr = infoPtr->internalCmds;
    Tcl_IncrRefCount(ensObjPtr);
    unkObjPtr = Tcl_NewStringObj(Tcl_GetString(infoPtr->internalCmds), -1);
    Tcl_AppendToObj(unkObjPtr, "::unknown", -1);
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
 *  ItclngGetUsage()
 *
 * ------------------------------------------------------------------------
  */
void
ItclngGetUsage(
    Tcl_Interp *interp,
    ItclngObjectInfo *infoPtr,
    Tcl_Obj *objPtr)       /* returns: summary of usage info */
{
    char *spaces = "  ";
    int i;

    for (i=0; ItclngMethodList[i].commandName != NULL; i++) {
	if (strcmp(ItclngMethodList[i].commandName,
	        "unknown") == 0) {
	    continue;
	}
        Tcl_AppendToObj(objPtr, spaces, -1);
        Tcl_AppendToObj(objPtr, Tcl_GetString(infoPtr->internalCmds), -1);
        Tcl_AppendToObj(objPtr, " ", 1);
        Tcl_AppendToObj(objPtr, ItclngMethodList[i].commandName, -1);
        if (strlen(ItclngMethodList[i].usage) > 0) {
            Tcl_AppendToObj(objPtr, " ", -1);
            Tcl_AppendToObj(objPtr, ItclngMethodList[i].usage, -1);
	}
        spaces = "\n  ";
    }
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_UnknownCmd()
 *
 *  the unknown handler for the ::ntk::widget::Widget ensemble
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itclng_UnknownCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngObjectInfo *infoPtr;
    int result;

    infoPtr = (ItclngObjectInfo *)clientData;
    ItclngShowArgs(1, "Itclng_UnknownCmd", objc, objv);
    result = TCL_ERROR;
    /* produce usage message */
    Tcl_Obj *objPtr = Tcl_NewStringObj("unknown command: \"", -1);
    Tcl_AppendToObj(objPtr, Tcl_GetString(objv[2]), -1);
    Tcl_AppendToObj(objPtr, "\" should be one of...\n", -1);
    ItclngGetUsage(interp, infoPtr, objPtr);
    Tcl_SetResult(interp, Tcl_GetString(objPtr), TCL_DYNAMIC);
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngCheckNumCmdParams()
 *
 *  check if number of params is correct and return error message if not
 *  check if function is loadable if not yet available
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */

int
ItclngCheckNumCmdParams(
    Tcl_Interp *interp,
    ItclngObjectInfo *infoPtr,
    const char *funcName,
    int objc,
    int numParams,
    int maxParams)
{
    int i;
    
    if ((objc < numParams+1) || ((objc > maxParams+1) && (maxParams != -1))) {
        for (i=0; ItclngMethodList[i].commandName != NULL; i++) {
	    if (strcmp(ItclngMethodList[i].commandName, funcName) == 0) {
                Tcl_AppendResult(interp,
                        "wrong # args: should be \"",
			Tcl_GetString(infoPtr->internalCmds), " ",
		        funcName, " ", 
		        ItclngMethodList[i].usage, "\"", NULL);
                return TCL_ERROR;
            }
        }
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  CreateOOMethod()
 *
 *  Creates a class method in TclOO
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
static int
CreateOOMethod(
    ItclngMemberFunc *imPtr,
    Tcl_Obj *argumentPtr,
    Tcl_Obj *bodyPtr)
{
    Tcl_HashEntry *hPtr2;
    int isNewEntry;

    ClientData pmPtr;
    imPtr->tmPtr = (ClientData)Itclng_NewProcClassMethod(
            imPtr->iclsPtr->interp, imPtr->iclsPtr->clsPtr,
	    ItclngCheckCallMethod, ItclngAfterCallMethod,
            ItclngProcErrorProc, imPtr, imPtr->namePtr, argumentPtr,
            bodyPtr, &pmPtr);
    hPtr2 = Tcl_CreateHashEntry(&imPtr->iclsPtr->infoPtr->procMethods,
            (char *)imPtr->tmPtr, &isNewEntry);
    if (isNewEntry) {
        Tcl_SetHashValue(hPtr2, imPtr);
    }
    if ((imPtr->flags & ITCLNG_COMMON) == 0) {
        imPtr->accessCmd = Tcl_CreateObjCommand(imPtr->iclsPtr->interp,
	        Tcl_GetString(imPtr->fullNamePtr),
	        Itclng_ExecMethod, imPtr, Tcl_Release);
    } else {
        imPtr->accessCmd = Tcl_CreateObjCommand(imPtr->iclsPtr->interp,
	        Tcl_GetString(imPtr->fullNamePtr),
		Itclng_ExecProc, imPtr, Tcl_Release);
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassFinishCmd()
 *
 *  Creates a class method
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassFinishCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_DString buffer;
    Tcl_Obj *argumentPtr;
    Tcl_Obj *bodyPtr;
    FOREACH_HASH_DECLS;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngClass *iclsPtr2;
    int newEntry;

    ItclngShowArgs(1, "Itclng_CreateClassFinishCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassFinish", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    iclsPtr = Itclng_PopStack(&infoPtr->clsStack);

    /*
     *  At this point, parsing of the class definition has succeeded.
     *  Add built-in methods such as "configure" and "cget"--as long
     *  as they don't conflict with those defined in the class.
     */
    if (Itclng_FirstListElem(&iclsPtr->bases) == NULL) {
        /* no inheritance at all if it is the root class, then make it known */
	if (infoPtr->rootClassIclsPtr == NULL) {
	    /* must be the the root class */
	    infoPtr->rootClassIclsPtr = iclsPtr;
	} else {
        /* no inheritance at all so add the root to the inheritance */
	    iclsPtr2 = infoPtr->rootClassIclsPtr;
            (void) Tcl_CreateHashEntry(&iclsPtr->heritage, (char*)iclsPtr2,
	            &newEntry);
            Itclng_AppendList(&iclsPtr->bases, (ClientData)iclsPtr2);
	    Tcl_Preserve((ClientData)iclsPtr2);
        }
    }

    /*
     *  Build the name resolution tables for all data members.
     */
    Itclng_BuildVirtualTables(iclsPtr);

    /* make the methods and procs known to TclOO */
    ItclngMemberFunc *imPtr;
    Tcl_DStringInit(&buffer);
    FOREACH_HASH_VALUE(imPtr, &iclsPtr->functions) {
        if (!(imPtr->flags & ITCLNG_IMPLEMENT_NONE)) {
	    argumentPtr = ItclngGetArgumentString(iclsPtr,
                    Tcl_GetString(imPtr->namePtr));
	    bodyPtr = ItclngGetBodyString(iclsPtr,
	            Tcl_GetString(imPtr->namePtr));;
	    (void)CreateOOMethod(imPtr, argumentPtr, bodyPtr);
            Tcl_DStringInit(&buffer);
        }
    }
    Tcl_DStringFree(&buffer);
    Tcl_ResetResult(interp);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassMethodCmd()
 *
 *  Creates a class method
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassMethodCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_CreateClassMethodCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassMethod", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(objv[1]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    if (ItclngCreateMethodOrProc(interp, iclsPtr, objv[2],
            /* is common (proc) */ 0, &imPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassProcCmd()
 *
 *  Creates a class proc
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full proc name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassProcCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_CreateClassProcCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassProc", objc, 2, 2)
            != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(objv[1]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    if (ItclngCreateMethodOrProc(interp, iclsPtr, objv[2],
            ITCLNG_COMMON, &imPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassConstructorCmd()
 *
 *  Creates a class constructor
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassConstructorCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_CreateClassConstructorCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassConstructor", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(objv[1]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    if (ItclngCreateMethodOrProc(interp, iclsPtr, objv[2],
            /* is common (proc) */ 0, &imPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    imPtr->flags &= ITCLNG_CONSTRUCTOR;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassDestructorCmd()
 *
 *  Creates a class destructor
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassDestructorCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_CreateClassDestructorCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassDestructor", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(objv[1]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    if (ItclngCreateMethodOrProc(interp, iclsPtr, objv[2],
            /* is common (proc) */ 0, &imPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    imPtr->flags &= ITCLNG_DESTRUCTOR;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassConstructorInitCmd()
 *
 *  Creates a class ___constructor_init
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassConstructorInitCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_CreateClassConstructorInitCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassConstructorInit",
            objc, 2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(objv[1]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    if (ItclngCreateMethodOrProc(interp, iclsPtr, objv[2],
            /* is common (proc) */ 0, &imPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    imPtr->flags &= ITCLNG_CONINIT;
    iclsPtr->initCode = ItclngGetBodyString(iclsPtr, Tcl_GetString(objv[2]));
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassCommonCmd()
 *
 *  Creates a class common
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full common name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassCommonCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const objv[])	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;

    ItclngShowArgs(1, "Itclng_CreateClassCommonCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassMethod", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(objv[1]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    if (ItclngCreateCommonOrVariable(interp, iclsPtr, objv[2],
            ITCLNG_COMMON) != TCL_OK) {
	return TCL_ERROR;
    }

    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassVariableCmd()
 *
 *  Creates a class Variable
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full variable name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassVariableCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const objv[])	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;

    ItclngShowArgs(1, "Itclng_CreateClassVariableCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassVariable", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(objv[1]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    if (ItclngCreateCommonOrVariable(interp, iclsPtr, objv[2],
            /* common */0) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassOptionCmd()
 *
 *  Creates a class option
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full option name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassOptionCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *CONST objv[])	/* argument objects */
{
    ItclngObjectInfo *infoPtr;

    ItclngShowArgs(0, "Itclng_CreateClassOptionCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassOption", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;

    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassMethodVariableCmd()
 *
 *  Creates a class method variable
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method variable name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassMethodVariableCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter that will contain new class */
    int objc,		        /* number of arguments */
    Tcl_Obj *const objv[])	/* arguments */
{
    ItclngObjectInfo *infoPtr;

    ItclngShowArgs(0, "Itclng_CreateClassMethodVariableCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassMethodVariable",
            objc, 2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;

    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassInheritCmd()
 *
 *  Creates a class inheritance
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassInheritCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *CONST objv[])	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngClass *iclsPtr2;
    ItclngHierIter hier;
    int newEntry;

    ItclngShowArgs(1, "Itclng_CreateClassInheritCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassInherit", objc,
            2, -1) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    iclsPtr = (ItclngClass*)Itclng_PeekStack(&infoPtr->clsStack);
    objc--;
    objc--;
    objv++;
    objv++;
    for(;objc > 0;objc--) {
	hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)(*objv));
	/* FIX ME !!! eventually need autoload here !! */
	if (hPtr == NULL) {
            Tcl_AppendResult(interp, "no such class \"", Tcl_GetString(*objv),
	            "\"", NULL);
	    return TCL_ERROR;
	}
        iclsPtr2 = Tcl_GetHashValue(hPtr);
	Itclng_AppendList(&iclsPtr->bases, (ClientData)iclsPtr2);
	Tcl_Preserve((ClientData)iclsPtr2);
        objv++;
    }
    /*
     *  Add each base class and all of its base classes into
     *  the heritage for the current class.  Along the way, make
     *  sure that no class appears twice in the heritage.
     */
    Itclng_InitHierIter(&hier, iclsPtr);
    iclsPtr2 = Itclng_AdvanceHierIter(&hier);  /* skip the class itself */
    iclsPtr2 = Itclng_AdvanceHierIter(&hier);
    while (iclsPtr2 != NULL) {
        (void) Tcl_CreateHashEntry(&iclsPtr->heritage, (char*)iclsPtr2,
	        &newEntry);
        if (!newEntry) {
            break;
        }
        iclsPtr2 = Itclng_AdvanceHierIter(&hier);
    }
    Itclng_DeleteHierIter(&hier);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_GetContextCmd()
 *
 *  Creates a class destructor
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_GetContextCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_Obj *resultPtr;
    Tcl_Obj *objPtr;
    ItclngObjectInfo *infoPtr;
    ItclngObject *ioPtr;
    ItclngClass *iclsPtr;

    ItclngShowArgs(0, "Itclng_GetContextCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "getContext", objc,
            0, 0) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    iclsPtr = NULL;
    ioPtr = NULL;
    if (Itclng_GetContext(interp, &iclsPtr, &ioPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
    if (iclsPtr != NULL) {
        objPtr = Tcl_NewStringObj(iclsPtr->nsPtr->fullName, -1);
    } else {
	objPtr = Tcl_NewStringObj("", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
    if (ioPtr != NULL) {
        objPtr = Tcl_NewObj();
	Tcl_GetCommandFullName(interp, ioPtr->accessCmd, objPtr);
    } else {
	objPtr = Tcl_NewStringObj("", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
    if (ioPtr != NULL) {
        objPtr = Tcl_NewStringObj(ioPtr->iclsPtr->nsPtr->fullName, -1);
    } else {
	objPtr = Tcl_NewStringObj("", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
fprintf(stderr, "CONTEXT!%p!%p!\n", iclsPtr, ioPtr);
    Tcl_SetObjResult(interp, resultPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngReportPublicOpt()
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
ItclngReportPublicOpt(
    Tcl_Interp *interp,      /* interpreter containing the object */
    ItclngVariable *ivPtr,     /* public variable to be reported */
    ItclngObject *contextIoPtr) /* object containing this variable */
{
    CONST char *val;
    ItclngClass *iclsPtr;
    Tcl_HashEntry *hPtr;
    ItclngVarLookup *vlookup;
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

    iclsPtr = (ItclngClass*)contextIoPtr->iclsPtr;
    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveVars,
            Tcl_GetString(ivPtr->fullNamePtr));
    assert(hPtr != NULL);
    vlookup = (ItclngVarLookup*)Tcl_GetHashValue(hPtr);
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

    val = Itclng_GetInstanceVar(interp, Tcl_GetString(ivPtr->namePtr),
            contextIoPtr, ivPtr->iclsPtr);

    if (val) {
        objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);

    return listPtr;
}
#ifdef NOTDEF

/*
 * ------------------------------------------------------------------------
 *  ItclngReportOption()
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
ItclngReportOption(
    Tcl_Interp *interp,      /* interpreter containing the object */
    ItclngOption *ioptPtr,     /* option to be reported */
    ItclngObject *contextIoPtr) /* object containing this variable */
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
    val = ItclngGetInstanceVar(interp, "itcl_options", Tcl_GetString(ioptPtr->namePtr),
            contextIoPtr, ioptPtr->iclsPtr);
    if (val) {
        objPtr = Tcl_NewStringObj((CONST84 char *)val, -1);
    } else {
        objPtr = Tcl_NewStringObj("<undefined>", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);
    return listPtr;
}
#endif

/*
 * ------------------------------------------------------------------------
 *  Itclng_ConfigureCmd()
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
Itclng_ConfigureCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;

    Tcl_Obj *resultPtr;
    Tcl_Obj *objPtr;
    Tcl_DString buffer;
    Tcl_DString buffer2;
    Tcl_HashSearch place;
    Tcl_HashEntry *hPtr;
    Tcl_Namespace *saveNsPtr;
    Tcl_Obj * const *unparsedObjv;
    ItclngClass *iclsPtr;
    ItclngVariable *ivPtr;
    ItclngVarLookup *vlookup;
    ItclngMemberCode *mcode;
    ItclngHierIter hier;
    ItclngObjectInfo *infoPtr;
    CONST char *lastval;
    char *token;
    char *varName;
    int i;
    int unparsedObjc;
    int result;

    ItclngShowArgs(0, "Itclng_BiConfigureCmd", objc, objv);
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
fprintf(stderr, "NS!%s!\n", Tcl_GetCurrentNamespace(interp)->fullName);
    contextIclsPtr = NULL;
    if (Itclng_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
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
#ifdef NOTDEF
    if (!(contextIclsPtr->flags & ITCL_CLASS)) {
	/* first check if it is an option */
	if (objc > 1) {
            hPtr = Tcl_FindHashEntry(&contextIclsPtr->options,
	            (char *) objv[1]);
	}
        result = ItclngExtendedConfigure(contextIclsPtr, interp, objc, objv);
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
#endif
    /*
     *  HANDLE:  configure
     */
    if (unparsedObjc == 1) {
        resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);

        Itclng_InitHierIter(&hier, contextIclsPtr);
        while ((iclsPtr=Itclng_AdvanceHierIter(&hier)) != NULL) {
            hPtr = Tcl_FirstHashEntry(&iclsPtr->variables, &place);
            while (hPtr) {
                ivPtr = (ItclngVariable*)Tcl_GetHashValue(hPtr);
                if (ivPtr->protection == ITCLNG_PUBLIC) {
                    objPtr = ItclngReportPublicOpt(interp, ivPtr, contextIoPtr);

                    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr,
                        objPtr);
                }
                hPtr = Tcl_NextHashEntry(&place);
            }
        }
        Itclng_DeleteHierIter(&hier);

        Tcl_SetObjResult(interp, resultPtr);
        return TCL_OK;
    } else {

        /*
         *  HANDLE:  configure -option
         */
        if (unparsedObjc == 2) {
            token = Tcl_GetString(unparsedObjv[1]);
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
                vlookup = (ItclngVarLookup*)Tcl_GetHashValue(hPtr);

                if (vlookup->ivPtr->protection != ITCLNG_PUBLIC) {
                    vlookup = NULL;
                }
            }
            if (!vlookup) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "unknown option \"", token, "\"",
                    (char*)NULL);
                return TCL_ERROR;
            }
            resultPtr = ItclngReportPublicOpt(interp,
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
                vlookup = (ItclngVarLookup*)Tcl_GetHashValue(hPtr);
            }
        }

        if (!vlookup || (vlookup->ivPtr->protection != ITCLNG_PUBLIC)) {
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
        if (mcode && Itclng_IsMemberCodeImplemented(mcode)) {
#ifdef NOTDEF
	    if (!ivPtr->iclsPtr->infoPtr->useOldResolvers) {
                Itclng_SetCallFrameResolver(interp, contextIoPtr->resolvePtr);
            }
#endif
	    saveNsPtr = Tcl_GetCurrentNamespace(interp);
	    Itclng_SetCallFrameNamespace(interp, ivPtr->iclsPtr->nsPtr);
#ifdef NOTDEF
	    result = Tcl_EvalObjEx(interp, mcode->bodyPtr, 0);
#endif
	    Itclng_SetCallFrameNamespace(interp, saveNsPtr);
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
 *  Itclng_CgetCmd()
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
Itclng_CgetCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    ItclngVarLookup *vlookup;
    CONST char *name;
    CONST char *val;
//    int result;

    ItclngShowArgs(1,"Itclng_BiCgetCmd", objc, objv);
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itclng_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
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

#ifdef NOTDEF
    if (!(contextIclsPtr->flags & ITCL_CLASS)) {
        result = ItclngExtendedCget(contextIclsPtr, interp, objc, objv);
        if (result != TCL_CONTINUE) {
            return result;
        }
    }
#endif
    name = Tcl_GetString(objv[1]);

    vlookup = NULL;
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, name+1);
    if (hPtr) {
        vlookup = (ItclngVarLookup*)Tcl_GetHashValue(hPtr);
    }

    if ((vlookup == NULL) || (vlookup->ivPtr->protection != ITCLNG_PUBLIC)) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "unknown option \"", name, "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    val = Itclng_GetInstanceVar(interp,
            Tcl_GetString(vlookup->ivPtr->namePtr),
            contextIoPtr, vlookup->ivPtr->iclsPtr);

    if (val) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(val, -1));
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("<undefined>", -1));
    }
    return TCL_OK;
}



#ifdef NOTDEF

/*
 * ------------------------------------------------------------------------
 *  Itcl_FindClassesCmd()
 *
 *  Invoked by Tcl whenever the user issues an "itcl::find classes"
 *  command to query the list of known classes.  Handles the following
 *  syntax:
 *
 *    find classes ?<pattern>?
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_FindClassesCmd(
    ClientData clientData,   /* class/object info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);
    Tcl_Namespace *globalNs = Tcl_GetGlobalNamespace(interp);
    int forceFullNames = 0;

    char *pattern;
    CONST char *cmdName;
    int newEntry, handledActiveNs;
    Tcl_HashTable unique;
    Tcl_HashEntry *entry;
    Tcl_HashSearch place;
    Itcl_Stack search;
    Tcl_Command cmd;
    Tcl_Command originalCmd;
    Tcl_Namespace *nsPtr;
    Tcl_Obj *objPtr;

    ItclShowArgs(2, "Itcl_FindClassesCmd", objc, objv);
    if (objc > 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "?pattern?");
        return TCL_ERROR;
    }

    if (objc == 2) {
        pattern = Tcl_GetString(objv[1]);
        forceFullNames = (strstr(pattern, "::") != NULL);
    } else {
        pattern = NULL;
    }

    /*
     *  Search through all commands in the current namespace first,
     *  in the global namespace next, then in all child namespaces
     *  in this interpreter.  If we find any commands that
     *  represent classes, report them.
     */

    Itcl_InitStack(&search);
    Itcl_PushStack((ClientData)globalNs, &search);
    Itcl_PushStack((ClientData)activeNs, &search);  /* last in, first out! */

    Tcl_InitHashTable(&unique, TCL_ONE_WORD_KEYS);

    handledActiveNs = 0;
    while (Itcl_GetStackSize(&search) > 0) {
        nsPtr = Itcl_PopStack(&search);
        if (nsPtr == activeNs && handledActiveNs) {
            continue;
        }

        entry = Tcl_FirstHashEntry(Tcl_GetNamespaceCommandTable(nsPtr),
	        &place);
        while (entry) {
            cmd = (Tcl_Command)Tcl_GetHashValue(entry);
            if (Itcl_IsClass(cmd)) {
                originalCmd = Tcl_GetOriginalCommand(cmd);

                /*
                 *  Report full names if:
                 *  - the pattern has namespace qualifiers
                 *  - the class namespace is not in the current namespace
                 *  - the class's object creation command is imported from
                 *      another namespace.
                 *
                 *  Otherwise, report short names.
                 */
                if (forceFullNames || nsPtr != activeNs ||
                    originalCmd != NULL) {

                    objPtr = Tcl_NewStringObj((char*)NULL, 0);
                    Tcl_GetCommandFullName(interp, cmd, objPtr);
                    cmdName = Tcl_GetString(objPtr);
                } else {
                    cmdName = Tcl_GetCommandName(interp, cmd);
                    objPtr = Tcl_NewStringObj((CONST84 char *)cmdName, -1);
                }

                if (originalCmd) {
                    cmd = originalCmd;
                }
                Tcl_CreateHashEntry(&unique, (char*)cmd, &newEntry);

                if (newEntry &&
			(!pattern || Tcl_StringMatch((CONST84 char *)cmdName,
			pattern))) {
                    Tcl_ListObjAppendElement((Tcl_Interp*)NULL,
			    Tcl_GetObjResult(interp), objPtr);
                } else {
		    /* if not appended to the result, free objPtr. */
		    Tcl_DecrRefCount(objPtr);
		}

            }
            entry = Tcl_NextHashEntry(&place);
        }
        handledActiveNs = 1;  /* don't process the active namespace twice */

        /*
         *  Push any child namespaces onto the stack and continue
         *  the search in those namespaces.
         */
        entry = Tcl_FirstHashEntry(Tcl_GetNamespaceChildTable(nsPtr), &place);
        while (entry != NULL) {
            Itcl_PushStack(Tcl_GetHashValue(entry), &search);
            entry = Tcl_NextHashEntry(&place);
        }
    }
    Tcl_DeleteHashTable(&unique);
    Itcl_DeleteStack(&search);

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_FindObjectsCmd()
 *
 *  Invoked by Tcl whenever the user issues an "itcl::find objects"
 *  command to query the list of known objects.  Handles the following
 *  syntax:
 *
 *    find objects ?-class <className>? ?-isa <className>? ?<pattern>?
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
int
Itcl_FindObjectsCmd(
    ClientData clientData,   /* class/object info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);
    Tcl_Namespace *globalNs = Tcl_GetGlobalNamespace(interp);
    int forceFullNames = 0;

    char *pattern = NULL;
    ItclClass *iclsPtr = NULL;
    ItclClass *isaDefn = NULL;

    char *name = NULL;
    char *token = NULL;
    CONST char *cmdName = NULL;
    int pos;
    int newEntry;
    int match;
    int handledActiveNs;
    ItclObject *contextIoPtr;
    Tcl_HashTable unique;
    Tcl_HashEntry *entry;
    Tcl_HashSearch place;
    Itcl_Stack search;
    Tcl_Command cmd;
    Tcl_Command originalCmd;
    Tcl_CmdInfo cmdInfo;
    Tcl_Namespace *nsPtr;
    Tcl_Obj *objPtr;

    /*
     *  Parse arguments:
     *  objects ?-class <className>? ?-isa <className>? ?<pattern>?
     */
    pos = 0;
    while (++pos < objc) {
        token = Tcl_GetString(objv[pos]);
        if (*token != '-') {
            if (!pattern) {
                pattern = token;
                forceFullNames = (strstr(pattern, "::") != NULL);
            } else {
                break;
            }
        }
        else if ((pos+1 < objc) && (strcmp(token,"-class") == 0)) {
            name = Tcl_GetString(objv[pos+1]);
            iclsPtr = Itcl_FindClass(interp, name, /* autoload */ 1);
            if (iclsPtr == NULL) {
                return TCL_ERROR;
            }
            pos++;
        }
        else if ((pos+1 < objc) && (strcmp(token,"-isa") == 0)) {
            name = Tcl_GetString(objv[pos+1]);
            isaDefn = Itcl_FindClass(interp, name, /* autoload */ 1);
            if (isaDefn == NULL) {
                return TCL_ERROR;
            }
            pos++;
        } else {

            /*
             * Last token? Take it as the pattern, even if it starts
             * with a "-".  This allows us to match object names that
             * start with "-".
             */
            if (pos == objc-1 && !pattern) {
                pattern = token;
                forceFullNames = (strstr(pattern, "::") != NULL);
            } else {
                break;
	    }
        }
    }

    if (pos < objc) {
        Tcl_WrongNumArgs(interp, 1, objv,
            "?-class className? ?-isa className? ?pattern?");
        return TCL_ERROR;
    }

    /*
     *  Search through all commands in the current namespace first,
     *  in the global namespace next, then in all child namespaces
     *  in this interpreter.  If we find any commands that
     *  represent objects, report them.
     */

    Itcl_InitStack(&search);
    Itcl_PushStack((ClientData)globalNs, &search);
    Itcl_PushStack((ClientData)activeNs, &search);  /* last in, first out! */

    Tcl_InitHashTable(&unique, TCL_ONE_WORD_KEYS);

    handledActiveNs = 0;
    while (Itcl_GetStackSize(&search) > 0) {
        nsPtr = Itcl_PopStack(&search);
        if (nsPtr == activeNs && handledActiveNs) {
            continue;
        }

        entry = Tcl_FirstHashEntry(Tcl_GetNamespaceCommandTable(nsPtr), &place);
        while (entry) {
            cmd = (Tcl_Command)Tcl_GetHashValue(entry);
            if (Itcl_IsObject(cmd)) {
                originalCmd = Tcl_GetOriginalCommand(cmd);
                if (originalCmd) {
                    cmd = originalCmd;
                }
		Tcl_GetCommandInfoFromToken(cmd, &cmdInfo);
                contextIoPtr = (ItclObject*)cmdInfo.deleteData;

                /*
                 *  Report full names if:
                 *  - the pattern has namespace qualifiers
                 *  - the class namespace is not in the current namespace
                 *  - the class's object creation command is imported from
                 *      another namespace.
                 *
                 *  Otherwise, report short names.
                 */
                if (forceFullNames || nsPtr != activeNs ||
                    originalCmd != NULL) {

                    objPtr = Tcl_NewStringObj((char*)NULL, 0);
                    Tcl_GetCommandFullName(interp, cmd, objPtr);
		    cmdName = Tcl_GetString(objPtr);
                } else {
                    cmdName = Tcl_GetCommandName(interp, cmd);
                    objPtr = Tcl_NewStringObj((CONST84 char *)cmdName, -1);
                }

                Tcl_CreateHashEntry(&unique, (char*)cmd, &newEntry);

                match = 0;
		if (newEntry &&
			(!pattern || Tcl_StringMatch((CONST84 char *)cmdName,
			pattern))) {
                    if ((iclsPtr == NULL) ||
		            (contextIoPtr->iclsPtr == iclsPtr)) {
                        if (isaDefn == NULL) {
                            match = 1;
                        } else {
                            entry = Tcl_FindHashEntry(
                                &contextIoPtr->iclsPtr->heritage,
                                (char*)isaDefn);

                            if (entry) {
                                match = 1;
                            }
                        }
                    }
                }

                if (match) {
                    Tcl_ListObjAppendElement((Tcl_Interp*)NULL,
                        Tcl_GetObjResult(interp), objPtr);
                } else {
                    Tcl_DecrRefCount(objPtr);  /* throw away the name */
                }
            }
            entry = Tcl_NextHashEntry(&place);
        }
        handledActiveNs = 1;  /* don't process the active namespace twice */

        /*
         *  Push any child namespaces onto the stack and continue
         *  the search in those namespaces.
         */
        entry = Tcl_FirstHashEntry(Tcl_GetNamespaceChildTable(nsPtr), &place);
        while (entry != NULL) {
            Itcl_PushStack(Tcl_GetHashValue(entry), &search);
            entry = Tcl_NextHashEntry(&place);
        }
    }
    Tcl_DeleteHashTable(&unique);
    Itcl_DeleteStack(&search);

    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_DelClassCmd()
 *
 *  Part of the "delete" ensemble.  Invoked by Tcl whenever the
 *  user issues a "delete class" command to delete classes.
 *  Handles the following syntax:
 *
 *    delete class <name> ?<name>...?
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_DelClassCmd(
    ClientData clientData,   /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int i;
    char *name;
    ItclClass *iclsPtr;

    ItclShowArgs(2, "Itcl_DelClassCmd", objc, objv);
    /*
     *  Since destroying a base class will destroy all derived
     *  classes, calls like "destroy class Base Derived" could
     *  fail.  Break this into two passes:  first check to make
     *  sure that all classes on the command line are valid,
     *  then delete them.
     */
    for (i=1; i < objc; i++) {
        name = Tcl_GetString(objv[i]);
        iclsPtr = Itcl_FindClass(interp, name, /* autoload */ 1);
        if (iclsPtr == NULL) {
            return TCL_ERROR;
        }
    }

    for (i=1; i < objc; i++) {
        name = Tcl_GetString(objv[i]);
        iclsPtr = Itcl_FindClass(interp, name, /* autoload */ 0);

        if (iclsPtr) {
            Tcl_ResetResult(interp);
            if (Itcl_DeleteClass(interp, iclsPtr) != TCL_OK) {
                return TCL_ERROR;
            }
        }
    }
    Tcl_ResetResult(interp);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_DelObjectCmd()
 *
 *  Part of the "delete" ensemble.  Invoked by Tcl whenever the user
 *  issues a "delete object" command to delete [incr Tcl] objects.
 *  Handles the following syntax:
 *
 *    delete object <name> ?<name>...?
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
int
Itcl_DelObjectCmd(
    ClientData clientData,   /* object management info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int i;
    char *name;
    ItclObject *contextIoPtr;

    ItclShowArgs(2, "Itcl_DelObjectCmd", objc, objv);
    /*
     *  Scan through the list of objects and attempt to delete them.
     *  If anything goes wrong (i.e., destructors fail), then
     *  abort with an error.
     */
    for (i=1; i < objc; i++) {
        name = Tcl_GetStringFromObj(objv[i], (int*)NULL);
        if (Itcl_FindObject(interp, name, &contextIoPtr) != TCL_OK) {
            return TCL_ERROR;
        }

        if (contextIoPtr == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "object \"", name, "\" not found",
                (char*)NULL);
            return TCL_ERROR;
        }

        if (Itcl_DeleteObject(interp, contextIoPtr) != TCL_OK) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ScopeCmd()
 *
 *  Invoked by Tcl whenever the user issues a "scope" command to
 *  create a fully qualified variable name.  Handles the following
 *  syntax:
 *
 *    scope <variable>
 *
 *  If the input string is already fully qualified (starts with "::"),
 *  then this procedure does nothing.  Otherwise, it looks for a
 *  data member called <variable> and returns its fully qualified
 *  name.  If the <variable> is a common data member, this procedure
 *  returns a name of the form:
 *
 *    ::namesp::namesp::class::variable
 *
 *  If the <variable> is an instance variable, this procedure returns
 *  a name of the form:
 *
 *    @itcl ::namesp::namesp::object variable
 *
 *  This kind of scoped value is recognized by the Itcl_ScopedVarResolver
 *  proc, which handles variable resolution for the entire interpreter.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_ScopeCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int result = TCL_OK;
    Tcl_Namespace *contextNsPtr = Tcl_GetCurrentNamespace(interp);
    char *openParen = NULL;

    register char *p;
    char *token;
    Tcl_HashEntry *hPtr;
    Tcl_Object oPtr;
    Tcl_InterpDeleteProc *procPtr;
    Tcl_Obj *objPtr;
    Tcl_Obj *objPtr2;
    Tcl_Var var;
    ItclClass *contextIclsPtr;
    ItclObject *contextIoPtr;
    Tcl_HashEntry *entry;
    ItclObjectInfo *infoPtr;
    ItclVarLookup *vlookup;
    int doAppend;

    ItclShowArgs(2, "Itcl_ScopeCmd", objc, objv);
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "varname");
        return TCL_ERROR;
    }

    /*
     *  If this looks like a fully qualified name already,
     *  then return it as is.
     */
    token = Tcl_GetStringFromObj(objv[1], (int*)NULL);
    if (*token == ':' && *(token+1) == ':') {
        Tcl_SetObjResult(interp, objv[1]);
        return TCL_OK;
    }

    /*
     *  If the variable name is an array reference, pick out
     *  the array name and use that for the lookup operations
     *  below.
     */
    for (p=token; *p != '\0'; p++) {
        if (*p == '(') {
            openParen = p;
        }
        else if (*p == ')' && openParen) {
            *openParen = '\0';
            break;
        }
    }

    /*
     *  Figure out what context we're in.  If this is a class,
     *  then look up the variable in the class definition.
     *  If this is a namespace, then look up the variable in its
     *  varTable.  Note that the normal Itcl_GetContext function
     *  returns an error if we're not in a class context, so we
     *  perform a similar function here, the hard way.
     *
     *  TRICKY NOTE:  If this is an array reference, we'll get
     *    the array variable as the variable name.  We must be
     *    careful to add the index (everything from openParen
     *    onward) as well.
     */
    contextIoPtr = NULL;
    contextIclsPtr = NULL;
    oPtr = NULL;
    infoPtr = Tcl_GetAssocData(interp, ITCL_INTERP_DATA, &procPtr);
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)contextNsPtr);
    if (hPtr != NULL) {
        contextIclsPtr = (ItclClass *)Tcl_GetHashValue(hPtr);
    }
    if (Itcl_IsClassNamespace(contextNsPtr)) {

        entry = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, token);
        if (!entry) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "variable \"", token, "\" not found in class \"",
                Tcl_GetString(contextIclsPtr->fullNamePtr), "\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto scopeCmdDone;
        }
        vlookup = (ItclVarLookup*)Tcl_GetHashValue(entry);

        if (vlookup->ivPtr->flags & ITCL_COMMON) {
            Tcl_Obj *resultPtr = Tcl_GetObjResult(interp);
	    if (vlookup->ivPtr->protection != ITCL_PUBLIC) {
	        Tcl_AppendToObj(resultPtr, ITCL_VARIABLES_NAMESPACE, -1);
	    }
            Tcl_AppendToObj(resultPtr,
	        Tcl_GetString(vlookup->ivPtr->fullNamePtr), -1);
            if (openParen) {
                *openParen = '(';
                Tcl_AppendToObj(resultPtr, openParen, -1);
                openParen = NULL;
            }
            result = TCL_OK;
            goto scopeCmdDone;
        }

        /*
         *  If this is not a common variable, then we better have
         *  an object context.  Return the name as a fully qualified name.
         */
        infoPtr = contextIclsPtr->infoPtr;
	ClientData clientData;
        clientData = Itcl_GetCallFrameClientData(interp);
        if (clientData != NULL) {
            oPtr = Tcl_ObjectContextObject((Tcl_ObjectContext)clientData);
            if (oPtr != NULL) {
                contextIoPtr = (ItclObject*)Tcl_ObjectGetMetadata(
	                oPtr, infoPtr->object_meta_type);
	    }
        }

        if (contextIoPtr == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "can't scope variable \"", token,
                "\": missing object context\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto scopeCmdDone;
        }

        doAppend = 1;
        if (contextIclsPtr->flags & ITCL_ECLASS) {
            if (strcmp(token, "itcl_options") == 0) {
	        doAppend = 0;
            }
        }
        objPtr = Tcl_NewStringObj((char*)NULL, 0);
        Tcl_IncrRefCount(objPtr);
	if (doAppend) {
            Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
	} else {
	    Tcl_AppendToObj(objPtr, "::", -1);
	    Tcl_AppendToObj(objPtr,
	            Tcl_GetCommandName(interp, contextIoPtr->accessCmd), -1);
	}

        objPtr2 = Tcl_NewStringObj((char*)NULL, 0);
        Tcl_IncrRefCount(objPtr2);
	Tcl_AppendToObj(objPtr2, ITCL_VARIABLES_NAMESPACE, -1);
	Tcl_AppendToObj(objPtr2, Tcl_GetString(objPtr), -1);
        if (doAppend) {
            Tcl_AppendToObj(objPtr2,
	            Tcl_GetString(vlookup->ivPtr->fullNamePtr), -1);
        } else {
            Tcl_AppendToObj(objPtr2, "::", -1);
            Tcl_AppendToObj(objPtr2,
	            Tcl_GetString(vlookup->ivPtr->namePtr), -1);
	}

        if (openParen) {
            *openParen = '(';
            Tcl_AppendToObj(objPtr2, openParen, -1);
            openParen = NULL;
        }
        Tcl_AppendElement(interp, Tcl_GetString(objPtr2));
        Tcl_DecrRefCount(objPtr);
        Tcl_DecrRefCount(objPtr2);
    } else {

        /*
         *  We must be in an ordinary namespace context.  Resolve
         *  the variable using Tcl_FindNamespaceVar.
         *
         *  TRICKY NOTE:  If this is an array reference, we'll get
         *    the array variable as the variable name.  We must be
         *    careful to add the index (everything from openParen
         *    onward) as well.
         */
        Tcl_Obj *resultPtr = Tcl_GetObjResult(interp);

        var = Itcl_FindNamespaceVar(interp, token, contextNsPtr,
            TCL_NAMESPACE_ONLY);

        if (!var) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "variable \"", token, "\" not found in namespace \"",
                contextNsPtr->fullName, "\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto scopeCmdDone;
        }

        Itcl_GetVariableFullName(interp, var, resultPtr);
        if (openParen) {
            *openParen = '(';
            Tcl_AppendToObj(resultPtr, openParen, -1);
            openParen = NULL;
        }
    }

scopeCmdDone:
    if (openParen) {
        *openParen = '(';
    }
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_CodeCmd()
 *
 *  Invoked by Tcl whenever the user issues a "code" command to
 *  create a scoped command string.  Handles the following syntax:
 *
 *    code ?-namespace foo? arg ?arg arg ...?
 *
 *  Unlike the scope command, the code command DOES NOT look for
 *  scoping information at the beginning of the command.  So scopes
 *  will nest in the code command.
 *
 *  The code command is similar to the "namespace code" command in
 *  Tcl, but it preserves the list structure of the input arguments,
 *  so it is a lot more useful.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_CodeCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);

    int pos;
    char *token;
    Tcl_Obj *listPtr, *objPtr;

    /*
     *  Handle flags like "-namespace"...
     */
    for (pos=1; pos < objc; pos++) {
        token = Tcl_GetStringFromObj(objv[pos], (int*)NULL);
        if (*token != '-') {
            break;
        }

        if (strcmp(token, "-namespace") == 0) {
            if (objc == 2) {
                Tcl_WrongNumArgs(interp, 1, objv,
                    "?-namespace name? command ?arg arg...?");
                return TCL_ERROR;
            } else {
                token = Tcl_GetString(objv[pos+1]);
                contextNs = Tcl_FindNamespace(interp, token,
                    (Tcl_Namespace*)NULL, TCL_LEAVE_ERR_MSG);

                if (!contextNs) {
                    return TCL_ERROR;
                }
                pos++;
            }
        } else {
	    if (strcmp(token, "--") == 0) {
                pos++;
                break;
            } else {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "bad option \"", token, "\": should be -namespace or --",
                    (char*)NULL);
                return TCL_ERROR;
            }
        }
    }

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv,
            "?-namespace name? command ?arg arg...?");
        return TCL_ERROR;
    }

    /*
     *  Now construct a scoped command by integrating the
     *  current namespace context, and appending the remaining
     *  arguments AS A LIST...
     */
    listPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);

    Tcl_ListObjAppendElement(interp, listPtr,
        Tcl_NewStringObj("namespace", -1));
    Tcl_ListObjAppendElement(interp, listPtr,
        Tcl_NewStringObj("inscope", -1));

    if (contextNs == Tcl_GetGlobalNamespace(interp)) {
        objPtr = Tcl_NewStringObj("::", -1);
    } else {
        objPtr = Tcl_NewStringObj(contextNs->fullName, -1);
    }
    Tcl_ListObjAppendElement(interp, listPtr, objPtr);

    if (objc-pos == 1) {
        objPtr = objv[pos];
    } else {
        objPtr = Tcl_NewListObj(objc-pos, &objv[pos]);
    }
    Tcl_ListObjAppendElement(interp, listPtr, objPtr);
    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_IsObjectCmd()
 *
 *  Invoked by Tcl whenever the user issues an "itcl::is object"
 *  command to test whether the argument is an object or not.
 *  syntax:
 *
 *    itcl::is object ?-class classname? commandname
 *
 *  Returns 1 if it is an object, 0 otherwise
 * ------------------------------------------------------------------------
 */
int
Itcl_IsObjectCmd(
    ClientData clientData,   /* class/object info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{

    int             classFlag = 0;
    int             idx = 0;
    char            *name = NULL;
    char            *cname;
    char            *cmdName;
    char            *token;
    Tcl_Command     cmd;
    Tcl_Namespace   *contextNs = NULL;
    ItclClass       *iclsPtr = NULL;
    ItclObject      *contextObj;

    /*
     *    Handle the arguments.
     *    objc needs to be either:
     *        2    itcl::is object commandname
     *        4    itcl::is object -class classname commandname
     */
    if (objc != 2 && objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "?-class classname? commandname");
        return TCL_ERROR;
    }

    /*
     *    Parse the command args. Look for the -class
     *    keyword.
     */
    for (idx=1; idx < objc; idx++) {
        token = Tcl_GetString(objv[idx]);

        if (strcmp(token,"-class") == 0) {
            cname = Tcl_GetString(objv[idx+1]);
            iclsPtr = Itcl_FindClass(interp, cname, /* no autoload */ 0);

            if (iclsPtr == NULL) {
                    return TCL_ERROR;
            }

            idx++;
            classFlag = 1;
        } else {
            name = Tcl_GetString(objv[idx]);
        }

    } /* end for objc loop */
        

    /*
     *  The object name may be a scoped value of the form
     *  "namespace inscope <namesp> <command>".  If it is,
     *  decode it.
     */
    if (Itcl_DecodeScopedCommand(interp, name, &contextNs, &cmdName)
        != TCL_OK) {
        return TCL_ERROR;
    }

    cmd = Tcl_FindCommand(interp, cmdName, contextNs, /* flags */ 0);

    /*
     *    Need the NULL test, or the test will fail if cmd is NULL
     */
    if (cmd == NULL || ! Itcl_IsObject(cmd)) {
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(0));
        return TCL_OK;
    }

    /*
     *    Handle the case when the -class flag is given
     */
    if (classFlag) {
	Tcl_CmdInfo cmdInfo;
        if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) == 1) {
            contextObj = (ItclObject*)cmdInfo.objClientData;
            if (! Itcl_ObjectIsa(contextObj, iclsPtr)) {
                Tcl_SetObjResult(interp, Tcl_NewBooleanObj(0));
                return TCL_OK;
            }
        }

    }

    /*
     *    Got this far, so assume that it is a valid object
     */
    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(1));
    ckfree(cmdName);

    return TCL_OK;
}



/*
 * ------------------------------------------------------------------------
 *  Itcl_IsClassCmd()
 *
 *  Invoked by Tcl whenever the user issues an "itcl::is class"
 *  command to test whether the argument is an itcl class or not
 *  syntax:
 *
 *    itcl::is class commandname
 *
 *  Returns 1 if it is a class, 0 otherwise
 * ------------------------------------------------------------------------
 */
int
Itcl_IsClassCmd(
    ClientData clientData,   /* class/object info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{

    char           *cname;
    char           *name;
    ItclClass      *iclsPtr = NULL;
    Tcl_Namespace  *contextNs = NULL;

    /*
     *    Need itcl::is class classname
     */
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "commandname");
        return TCL_ERROR;
    }

    name = Tcl_GetString(objv[1]);

    /*
     *    The object name may be a scoped value of the form
     *    "namespace inscope <namesp> <command>".  If it is,
     *    decode it.
     */
    if (Itcl_DecodeScopedCommand(interp, name, &contextNs, &cname) != TCL_OK) {
        return TCL_ERROR;
    }

    iclsPtr = Itcl_FindClass(interp, cname, /* no autoload */ 0);

    /*
     *    If classDefn is NULL, then it wasn't found, hence it
     *    isn't a class
     */
    if (iclsPtr != NULL) {
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(1));
    } else {
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(0));
    }

    ckfree(cname);

    return TCL_OK;

} /* end Itcl_IsClassCmd function */

/*
 * ------------------------------------------------------------------------
 *  Itcl_FilterCmd()
 *
 *  Used to add a filter command to an object which is called just before
 *  a method/proc of a class is executed
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_FilterAddCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj **newObjv;
    int result;

    ItclShowArgs(1, "Itcl_FilterCmd", objc, objv);
//    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);
/* FIX ME need to change the chain command to do the same here as the TclOO next command !! */
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "<className> <filterName> ?<filterName> ...?");
        return TCL_ERROR;
    }
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+1));
    newObjv[0] = Tcl_NewStringObj("::oo::define", -1);
    Tcl_IncrRefCount(newObjv[0]);
    newObjv[1] = objv[1];
    newObjv[2] = Tcl_NewStringObj("filter", -1);
    Tcl_IncrRefCount(newObjv[2]);
    memcpy(newObjv+3, objv+2, sizeof(Tcl_Obj *)*(objc-2));
    ItclShowArgs(1, "Itcl_FilterAddCmd2", objc+1, newObjv);
    result = Tcl_EvalObjv(interp, objc+1, newObjv, 0);
    Tcl_DecrRefCount(newObjv[0]);
    Tcl_DecrRefCount(newObjv[2]);

    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_FilterDeleteCmd()
 *
 *  used to delete filter commands of a class or object
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_FilterDeleteCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(1, "Itcl_FilterDeleteCmd", objc, objv);
//    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);

    Tcl_AppendResult(interp, "::itcl::filter delete command not yet implemented", NULL);
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ForwardAddCmd()
 *
 *  Used to similar to iterp alias to forward the call of a method 
 *  to another method within the class
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_ForwardAddCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj *prefixObj;
    Tcl_Method mPtr;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;

    ItclShowArgs(0, "Itcl_ForwardAddCmd", objc, objv);
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "<forwardName> <targetName> ?<arg> ...?");
        return TCL_ERROR;
    }
    infoPtr = Tcl_GetAssocData(interp, ITCL_INTERP_DATA, NULL);
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (iclsPtr == NULL) {
	Tcl_HashEntry *hPtr;
        hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
	if (hPtr == NULL) {
	    Tcl_AppendResult(interp, "class: \"", Tcl_GetString(objv[1]),
	            "\" not found", NULL);
	    return TCL_ERROR;
	}
        iclsPtr = Tcl_GetHashValue(hPtr);
    }
    prefixObj = Tcl_NewListObj(objc-2, objv+2);
    mPtr = Itcl_NewForwardClassMethod(interp, iclsPtr->clsPtr, 1,
            objv[1], prefixObj);
    if (mPtr == NULL) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ForwardDeleteCmd()
 *
 *  used to delete forwarded commands of a class or object
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_ForwardDeleteCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(1, "Itcl_ForwardDeleteCmd", objc, objv);
//    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);

    Tcl_AppendResult(interp, "::itcl::forward delete command not yet implemented", NULL);
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_MixinAddCmd()
 *
 *  Used to add the methods of a class to another class without heritance
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_MixinAddCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Obj **newObjv;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    int result;

    ItclShowArgs(1, "Itcl_MixinAddCmd", objc, objv);
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "<className> <mixinName> ?<mixinName> ...?");
        return TCL_ERROR;
    }
    infoPtr = Tcl_GetAssocData(interp, ITCL_INTERP_DATA, NULL);
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+1));
    newObjv[0] = Tcl_NewStringObj("::oo::define", -1);
    Tcl_IncrRefCount(newObjv[0]);
    newObjv[1] = objv[1];
    newObjv[2] = Tcl_NewStringObj("mixin", -1);
    Tcl_IncrRefCount(newObjv[2]);
    memcpy(newObjv+3, objv+2, sizeof(Tcl_Obj *)*(objc-2));
    ItclShowArgs(1, "Itcl_MixinAddCmd2", objc+1, newObjv);
    result = Tcl_EvalObjv(interp, objc+1, newObjv, 0);
    Tcl_DecrRefCount(newObjv[0]);
    Tcl_DecrRefCount(newObjv[2]);

    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_MixinDeleteCmd()
 *
 *  Used to delete the methods of a class to another class without heritance
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_MixinDeleteCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(1, "Itcl_MixinDeleteCmd", objc, objv);
//    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);

    Tcl_AppendResult(interp, "::itcl::mixin delete command not yet implemented", NULL);
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_NWidgetCmd()
 *
 *  Used to build an [incr Tcl] nwidget
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_NWidgetCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *iclsPtr;
    ItclObjectInfo *infoPtr;
    int result;

    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(0, "Itcl_NWidgetCmd", objc-1, objv);
    result = ItclClassBaseCmd(clientData, interp, ITCL_NWIDGET, objc, objv,
            &iclsPtr);
    if (result != TCL_OK) {
        return result;
    }
    if (iclsPtr == NULL) {
fprintf(stderr, "Itcl_NWidgetCmd!iclsPtr == NULL\n");
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_AddOptionCmd()
 *
 *  Used to build an option to an [incr Tcl] nwidget/eclass
 *
 *  Syntax: ::itcl::addoption <nwidget class> <optionName> <defaultValue>
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_AddOptionCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    const char *protectionStr;
    int pLevel;
    int result;

    result = TCL_OK;
    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(0, "Itcl_AddOptionCmd", objc, objv);
    if (objc < 4) {
        Tcl_WrongNumArgs(interp, 1, objv, 
	        "className protection option optionName ...");
	return TCL_ERROR;
    }
    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "class \"", Tcl_GetString(objv[1]),
	        "\" not found", NULL);
        return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    protectionStr = Tcl_GetString(objv[2]);
    pLevel = -1;
    if (strcmp(protectionStr, "public") == 0) {
        pLevel = ITCL_PUBLIC;
    }
    if (strcmp(protectionStr, "protected") == 0) {
        pLevel = ITCL_PROTECTED;
    }
    if (strcmp(protectionStr, "private") == 0) {
        pLevel = ITCL_PRIVATE;
    }
    if (pLevel == -1) {
	Tcl_AppendResult(interp, "bad protection \"", protectionStr, "\"",
	        NULL);
        return TCL_ERROR;
    }
    Itcl_PushStack((ClientData)iclsPtr, &infoPtr->clsStack);
    result = Itcl_ClassOptionCmd(clientData, interp, objc-1, objv+1);
    Itcl_PopStack(&infoPtr->clsStack);
    if (result != TCL_OK) {
        return result;
    }
    result = DelegatedOptionsInstall(interp, iclsPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_AddObjectOptionCmd()
 *
 *  Used to build an option for an [incr Tcl] object
 *
 *  Syntax: ::itcl::addobjectoption <object> <protection> option <optionSpec> 
 *     ?-default <defaultValue>?
 *     ?-configuremethod <configuremethod>?
 *     ?-validatemethod <validatemethod>?
 *     ?-cgetmethod <cgetmethod>?
 *     ?-configuremethodvar <configuremethodvar>?
 *     ?-validatemethodvar <validatemethodvar>?
 *     ?-cgetmethodvar <cgetmethodvar>?
 *     ?-readonly?
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_AddObjectOptionCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_HashEntry *hPtr;
    Tcl_Command cmd;
    Tcl_Obj *objPtr;
    ItclObjectInfo *infoPtr;
    ItclObject *ioPtr;
    ItclOption *ioptPtr;
    const char *protectionStr;
    int pLevel;
    int isNew;

    ioptPtr = NULL;
    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(1, "Itcl_AddObjectOptionCmd", objc, objv);
    if (objc < 4) {
        Tcl_WrongNumArgs(interp, 1, objv, 
	        "objectName protection option optionName ...");
	return TCL_ERROR;
    }
    
    cmd = Tcl_FindCommand(interp, Tcl_GetString(objv[1]), NULL, 0);
    if (cmd == NULL) {
	Tcl_AppendResult(interp, "object \"", Tcl_GetString(objv[1]),
	        "\" not found", NULL);
        return TCL_ERROR;
    }
    hPtr = Tcl_FindHashEntry(&infoPtr->objects, (char *)cmd);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "object \"", Tcl_GetString(objv[1]),
	        "\" not found", NULL);
        return TCL_ERROR;
    }
    ioPtr = Tcl_GetHashValue(hPtr);
    protectionStr = Tcl_GetString(objv[2]);
    pLevel = -1;
    if (strcmp(protectionStr, "public") == 0) {
        pLevel = ITCL_PUBLIC;
    }
    if (strcmp(protectionStr, "protected") == 0) {
        pLevel = ITCL_PROTECTED;
    }
    if (strcmp(protectionStr, "private") == 0) {
        pLevel = ITCL_PRIVATE;
    }
    if (pLevel == -1) {
	Tcl_AppendResult(interp, "bad protection \"", protectionStr, "\"",
	        NULL);
        return TCL_ERROR;
    }
    infoPtr->protection = pLevel;
    if (ItclParseOption(infoPtr, interp, objc-3, objv+3, &ioptPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    objPtr = Tcl_NewObj();
    Tcl_GetCommandFullName(interp, ioPtr->accessCmd, objPtr);
    ioptPtr->fullNamePtr = Tcl_NewStringObj(
            Tcl_GetString(ioPtr->namePtr), -1);
    Tcl_AppendToObj(ioptPtr->fullNamePtr, "::", 2);
    Tcl_AppendToObj(ioptPtr->fullNamePtr, Tcl_GetString(ioptPtr->namePtr), -1);
    Tcl_IncrRefCount(ioptPtr->fullNamePtr);
    hPtr = Tcl_CreateHashEntry(&ioPtr->objectOptions,
            (char *)ioptPtr->namePtr, &isNew);
    Tcl_SetHashValue(hPtr, ioptPtr);
    ItclSetInstanceVar(interp, "itcl_options",
            Tcl_GetString(ioptPtr->namePtr),
            Tcl_GetString(ioptPtr->defaultValuePtr), ioPtr, NULL);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_AddDelegatedOptionCmd()
 *
 *  Used to build an option to an [incr Tcl] nwidget/eclass
 *
 *  Syntax: ::itcl::adddelegatedoption <nwidget object> <optionName> <defaultValue>
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_AddDelegatedOptionCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_HashEntry *hPtr;
    Tcl_Command cmd;
    ItclObjectInfo *infoPtr;
    ItclObject *ioPtr;
    ItclDelegatedOption *idoPtr;
    int isNew;
    int result;

    result = TCL_OK;
    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(1, "Itcl_AddDelegatedOptionCmd", objc, objv);
    if (objc < 4) {
        Tcl_WrongNumArgs(interp, 1, objv, 
	        "className protection option optionName ...");
	return TCL_ERROR;
    }
    
    cmd = Tcl_FindCommand(interp, Tcl_GetString(objv[1]), NULL, 0);
    if (cmd == NULL) {
	Tcl_AppendResult(interp, "object \"", Tcl_GetString(objv[1]),
	        "\" not found", NULL);
        return TCL_ERROR;
    }
    hPtr = Tcl_FindHashEntry(&infoPtr->objects, (char *)cmd);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "object \"", Tcl_GetString(objv[1]),
	        "\" not found", NULL);
        return TCL_ERROR;
    }
    ioPtr = Tcl_GetHashValue(hPtr);
    result = Itcl_HandleDelegateOptionCmd(interp, ioPtr, NULL, &idoPtr,
            objc-3, objv+3);
    if (result != TCL_OK) {
        return result;
    }
    hPtr = Tcl_CreateHashEntry(&ioPtr->objectDelegatedOptions,
            (char *)idoPtr->namePtr, &isNew);
    Tcl_SetHashValue(hPtr, idoPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_AddDelegatedFunctionCmd()
 *
 *  Used to build an function to an [incr Tcl] nwidget/eclass
 *
 *  Syntax: ::itcl::adddelegatedfunction <nwidget object> <fucntionName> ...
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_AddDelegatedFunctionCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_HashEntry *hPtr;
    Tcl_Command cmd;
    Tcl_Obj *componentNamePtr;
    ItclObjectInfo *infoPtr;
    ItclObject *ioPtr;
    ItclClass *iclsPtr;
    ItclDelegatedFunction *idmPtr;
    ItclHierIter hier;
    const char *val;
    int isNew;
    int result;

    result = TCL_OK;
    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(1, "Itcl_AddDelegatedFunctionCmd", objc, objv);
    if (objc < 4) {
        Tcl_WrongNumArgs(interp, 1, objv, 
	        "className protection method/proc functionName ...");
	return TCL_ERROR;
    }
    
    cmd = Tcl_FindCommand(interp, Tcl_GetString(objv[1]), NULL, 0);
    if (cmd == NULL) {
	Tcl_AppendResult(interp, "object \"", Tcl_GetString(objv[1]),
	        "\" not found", NULL);
        return TCL_ERROR;
    }
    hPtr = Tcl_FindHashEntry(&infoPtr->objects, (char *)cmd);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "object \"", Tcl_GetString(objv[1]),
	        "\" not found", NULL);
        return TCL_ERROR;
    }
    ioPtr = Tcl_GetHashValue(hPtr);
    result = Itcl_HandleDelegateMethodCmd(interp, ioPtr, NULL, &idmPtr,
            objc-3, objv+3);
    if (result != TCL_OK) {
        return result;
    }
    componentNamePtr = idmPtr->icPtr->namePtr;
    Itcl_InitHierIter(&hier, ioPtr->iclsPtr);
    while ((iclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
        hPtr = Tcl_FindHashEntry(&iclsPtr->components, (char *)
                componentNamePtr);
	if (hPtr != NULL) {
	    break;
	}
    }
    Itcl_DeleteHierIter(&hier);
    val = Itcl_GetInstanceVar(interp,
            Tcl_GetString(componentNamePtr), ioPtr, iclsPtr);
    componentNamePtr = Tcl_NewStringObj(val, -1);
    Tcl_IncrRefCount(componentNamePtr);
    DelegateFunction(interp, ioPtr, ioPtr->iclsPtr, componentNamePtr, idmPtr);
    hPtr = Tcl_CreateHashEntry(&ioPtr->objectDelegatedFunctions,
            (char *)idmPtr->namePtr, &isNew);
    Tcl_DecrRefCount(componentNamePtr);
    Tcl_SetHashValue(hPtr, idmPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_AddComponentCmd()
 *
 *  Used to add a component to an [incr Tcl] nwidget/eclass
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_AddComponentCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclObjectInfo *infoPtr;
    int result;

    result = TCL_OK;
    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(1, "Itcl_AddComponentCmd", objc, objv);
fprintf(stderr, "not yet implemented\n");
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_SetComponentCmd()
 *
 *  Used to set a component for an [incr Tcl] nwidget/eclass
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_SetComponentCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    FOREACH_HASH_DECLS;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclObject *contextIoPtr;
    ItclClass *contextIclsPtr;
    ItclComponent *icPtr;
    ItclDelegatedOption *idoPtr;
    ItclHierIter hier;
    const char *name;
    const char *val;
    int result;

    result = TCL_OK;
    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(1, "Itcl_SetComponentCmd", objc, objv);
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, 
	        "className componentName");
	return TCL_ERROR;
    }
    name = Tcl_GetStringFromObj(objv[1], (int*)NULL);
    if (Itcl_FindObject(interp, name, &contextIoPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    Itcl_InitHierIter(&hier, contextIoPtr->iclsPtr);
    while ((contextIclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
        hPtr = Tcl_FindHashEntry(&contextIclsPtr->components, (char *)objv[2]);
        if (hPtr != NULL) {
	    break;
	}
    }
    Itcl_DeleteHierIter(&hier);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "object \"", Tcl_GetString(objv[1]),
	        "\" has no component \"", Tcl_GetString(objv[2]), "\"", NULL);
        return TCL_ERROR;
    }
    icPtr = Tcl_GetHashValue(hPtr);
    val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr), NULL,
            contextIoPtr, contextIclsPtr);
    if ((val != NULL) && (strlen(val) != 0)) {
        /* delete delegated options to the old component here !! */
fprintf(stderr, "deleting old delegated options\n");
        Itcl_InitHierIter(&hier, contextIoPtr->iclsPtr);
        while ((iclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
            FOREACH_HASH_VALUE(idoPtr, &iclsPtr->delegatedOptions) {
	        if (strcmp(Tcl_GetString(idoPtr->icPtr->namePtr),
		        Tcl_GetString(objv[2])) == 0) {
		    Tcl_DeleteHashEntry(hPtr);
	        }
	    }
        }
        Itcl_DeleteHierIter(&hier);
    }
    if (ItclSetInstanceVar(interp, Tcl_GetString(icPtr->namePtr), NULL,
             Tcl_GetString(objv[3]), contextIoPtr, contextIclsPtr) == NULL) {
	return TCL_ERROR;
    }
    val = ItclGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr), NULL,
            contextIoPtr, contextIclsPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ExtendedClassCmd()
 *
 *  Used to an [incr Tcl] struct
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_ExtendedClassCmd(
    ClientData clientData,   /* infoPtr */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclClass *iclsPtr;
    ItclObjectInfo *infoPtr;
    int result;

    infoPtr = (ItclObjectInfo *)clientData;
    ItclShowArgs(1, "Itcl_ExtendedClassCmd", objc-1, objv);
    result = ItclClassBaseCmd(clientData, interp, ITCL_ECLASS, objc, objv,
            &iclsPtr);
    if (iclsPtr == NULL) {
        ItclShowArgs(0, "Itcl_ExtendedClassCmd", objc-1, objv);
fprintf(stderr, "Itcl_ExtendedClassCmd!iclsPtr == NULL\n");
        return TCL_ERROR;
    }
    return result;
}
#endif
