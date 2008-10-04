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
Tcl_ObjCmdProc Itclng_CreateClassCMethodCmd;
Tcl_ObjCmdProc Itclng_CreateClassProcCmd;
Tcl_ObjCmdProc Itclng_CreateClassCProcCmd;
Tcl_ObjCmdProc Itclng_ChangeClassMemberFuncCmd;
Tcl_ObjCmdProc Itclng_ChangeClassVariableConfigCmd;
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
Tcl_ObjCmdProc Itclng_IsaCmd;
Tcl_ObjCmdProc Itclng_ChainCmd;
Tcl_ObjCmdProc Itclng_GetContextCmd;
Tcl_ObjCmdProc Itclng_GetCallContextInfoCmd;
Tcl_ObjCmdProc Itclng_GetInstanceVarValueCmd;
Tcl_ObjCmdProc Itclng_FindClassesCmd;
Tcl_ObjCmdProc Itclng_FindObjectsCmd;
Tcl_ObjCmdProc Itclng_DeleteClassCmd;
Tcl_ObjCmdProc Itclng_DeleteObjectCmd;
Tcl_ObjCmdProc Itclng_IsClassCmd;
Tcl_ObjCmdProc Itclng_IsObjectCmd;
Tcl_ObjCmdProc Itclng_ScopeCmd;
Tcl_ObjCmdProc Itclng_CodeCmd;
Tcl_ObjCmdProc ItclngExtendedConfigure;
Tcl_ObjCmdProc ItclngExtendedCget;
Tcl_ObjCmdProc ItclngExtendedSetGet;

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
    { "createClassCMethod",
      "fullClassName methodName",
      Itclng_CreateClassCMethodCmd },
    { "createClassProc",
      "fullClassName procName",
      Itclng_CreateClassProcCmd },
    { "createClassCProc",
      "fullClassName procName",
      Itclng_CreateClassCProcCmd },
    { "changeClassMemberFunc",
      "fullClassName methodName",
      Itclng_ChangeClassMemberFuncCmd },
    { "changeClassVariableConfig",
      "fullClassName methodName",
      Itclng_ChangeClassVariableConfigCmd },
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
    { "isa",
      "fullClassName ?arg arg ... ?",
      Itclng_IsaCmd },
    { "chain",
      "?arg arg ... ?",
      Itclng_ChainCmd },
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
    { "getCallContextInfo",
      "",
      Itclng_GetCallContextInfoCmd },
    { "getInstanceVarValue",
      "",
      Itclng_GetInstanceVarValueCmd },
    { "findClasses",
      "",
      Itclng_FindClassesCmd },
    { "findObjects",
      "",
      Itclng_FindObjectsCmd },
    { "deleteClass",
      "",
      Itclng_DeleteClassCmd },
    { "deleteObject",
      "object ?object object ...?",
      Itclng_DeleteObjectCmd },
    { "isClass",
      "",
      Itclng_IsClassCmd },
    { "isObject",
      "",
      Itclng_IsObjectCmd },
    { "scope",
      "",
      Itclng_ScopeCmd },
    { "code",
      "",
      Itclng_CodeCmd },
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
    Tcl_ObjCmdProc *objProcPtr;
    Tcl_CmdProc *argProcPtr;
    Tcl_Obj *namePtr;
    ClientData clientData;
    ClientData pmPtr;
    int isNewEntry;

    if (imPtr->flags &ITCLNG_IMPLEMENT_C) {
	namePtr = ItclngGetBodyString(imPtr->iclsPtr,
	        Tcl_GetString(imPtr->namePtr));
        if (!Itclng_FindC(imPtr->iclsPtr->interp, Tcl_GetString(namePtr),
	        &argProcPtr, &objProcPtr, &clientData) != TCL_OK) {
fprintf(stderr, "cannot find C-function %s for %s\n", Tcl_GetString(namePtr), Tcl_GetString(imPtr->namePtr));
	    return TCL_ERROR;
	}
        imPtr->tmPtr = (ClientData)Itclng_NewCClassMethod(
                imPtr->iclsPtr->interp, imPtr->iclsPtr->clsPtr,
	        ItclngCheckCallMethod, ItclngAfterCallMethod,
                ItclngProcErrorProc, imPtr, imPtr->namePtr, argumentPtr,
                objProcPtr, &pmPtr);
    } else {
        imPtr->tmPtr = (ClientData)Itclng_NewProcClassMethod(
                imPtr->iclsPtr->interp, imPtr->iclsPtr->clsPtr,
	        ItclngCheckCallMethod, ItclngAfterCallMethod,
                ItclngProcErrorProc, imPtr, imPtr->namePtr, argumentPtr,
                bodyPtr, &pmPtr);
    }
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
	    argumentPtr = ItclngGetArgumentInfo(iclsPtr,
                    Tcl_GetString(imPtr->namePtr), "arguments", "definition");
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
 *  Itclng_CreateClassCMethodCmd()
 *
 *  Creates a class C-implemented method
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassCMethodCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_CreateClassCMethodCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassCMethod", objc,
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
    imPtr->flags |= ITCLNG_IMPLEMENT_C;
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
 *  Itclng_CreateClassCProcCmd()
 *
 *  Creates a class C-implemented proc
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the full method name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassCProcCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_CreateClassCProcCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClassCProc", objc,
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
            ITCLNG_COMMON, &imPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    imPtr->flags |= ITCLNG_IMPLEMENT_C;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_ChangeClassMemberFuncCmd()
 *
 *  Change a class member func
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK
 * ------------------------------------------------------------------------
 */
int
Itclng_ChangeClassMemberFuncCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;

    ItclngShowArgs(1, "Itclng_ChangeClassMemberFuncCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "changeClassMemberFunc", objc,
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
    hPtr = Tcl_FindHashEntry(&iclsPtr->functions, (char *)objv[2]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such function \"", Tcl_GetString(objv[2]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    imPtr = Tcl_GetHashValue(hPtr);
    if (ItclngChangeMemberFunc(interp, iclsPtr, objv[2], imPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_ChangeClassVariableConfigCmd()
 *
 *  Change a class variable config code
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK
 * ------------------------------------------------------------------------
 */
int
Itclng_ChangeClassVariableConfigCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngVariable *ivPtr;

    ItclngShowArgs(1, "Itclng_ChangeClassVariableConfigCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "changeClassVariableConfig",
            objc, 3, 3) != TCL_OK) {
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
    hPtr = Tcl_FindHashEntry(&iclsPtr->variables, (char *)objv[2]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such variable \"", Tcl_GetString(objv[2]),
	        "\"", NULL);
        return TCL_ERROR;
    }
    ivPtr = Tcl_GetHashValue(hPtr);
//fprintf(stderr, "CALL!ItclngChangeVariableConfig!\n");
    if (ItclngChangeVariableConfig(interp, iclsPtr, objv[2],
            Tcl_GetString(objv[3]), ivPtr) != TCL_OK) {
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

    ItclngShowArgs(1, "Itclng_CreateClassOptionCmd", objc, objv);
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

    ItclngShowArgs(1, "Itclng_CreateClassMethodVariableCmd", objc, objv);
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
	    if (iclsPtr2 != iclsPtr->infoPtr->rootClassIclsPtr) {
                break;
	    }
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
 *  gets context info concerning class and object
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the info
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
    ItclngCallContext *callContextPtr;

    ItclngShowArgs(1, "Itclng_GetContextCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "getContext", objc,
            0, 0) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    iclsPtr = NULL;
    ioPtr = NULL;
    callContextPtr = Itclng_PeekStack(&infoPtr->contextStack);
    if (callContextPtr != NULL) {
        objPtr = Tcl_NewStringObj(callContextPtr->ioPtr->iclsPtr->nsPtr->fullName, -1);
    } else {
	objPtr = Tcl_NewStringObj("", -1);
    }
//fprintf(stderr, "CALLNS!%s!\n", Tcl_GetString(objPtr));
//    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
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
    Tcl_SetObjResult(interp, resultPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_GetCallContextInfoCmd()
 *
 *  get call context info
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the info
 * ------------------------------------------------------------------------
 */
int
Itclng_GetCallContextInfoCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_Obj *resultPtr;
    Tcl_Obj *objPtr;
    ItclngObjectInfo *infoPtr;
    ItclngCallContext *callContextPtr;

    ItclngShowArgs(1, "Itclng_GetCallContextInfoCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "getCallContextInfo", objc,
            0, 0) != TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    resultPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
    callContextPtr = Itclng_PeekStack(&infoPtr->contextStack);
    if (callContextPtr != NULL) {
	objPtr = Tcl_NewObj();
	Tcl_GetCommandFullName(interp, callContextPtr->ioPtr->accessCmd,
	        objPtr);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
        objPtr = Tcl_NewStringObj(callContextPtr->imPtr->iclsPtr->nsPtr->fullName, -1);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
        objPtr = Tcl_NewStringObj(callContextPtr->ioPtr->iclsPtr->nsPtr->fullName, -1);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
        objPtr = Tcl_NewStringObj(callContextPtr->nsPtr->fullName, -1);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
        objPtr = Tcl_NewStringObj(Tcl_GetString(
	        callContextPtr->imPtr->fullNamePtr), -1);
    } else {
	objPtr = Tcl_NewStringObj("", -1);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
	objPtr = Tcl_NewStringObj("", -1);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
	objPtr = Tcl_NewStringObj("", -1);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
	objPtr = Tcl_NewStringObj("", -1);
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
	objPtr = Tcl_NewStringObj("", -1);
    }
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, resultPtr, objPtr);
    Tcl_SetObjResult(interp, resultPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_InstanceVarValueCmd()
 *
 *  get value of the instance variable
 *  On failure returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the info
 * ------------------------------------------------------------------------
 */
int
Itclng_GetInstanceVarValueCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter */
    int objc,		        /* number of arguments */
    Tcl_Obj *const*objv)	/* argument objects */
{
    Tcl_Obj *objPtr;
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngCallContext *callContextPtr;
    ItclngVariable *ivPtr;
    const char *name1;
    const char *name2;
    const char *cp;

    ItclngShowArgs(1, "Itclng_GetInstanceVarValueCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "getInstanceVarValue", objc,
            2, 2) != TCL_OK) {
        return TCL_ERROR;
    }
    name1 = Tcl_GetString(objv[1]);
    name2 = Tcl_GetString(objv[2]);
    if (strlen(name2) == 0) {
        name2 = NULL;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    callContextPtr = Itclng_PeekStack(&infoPtr->contextStack);
    if (callContextPtr != NULL) {
//fprintf(stderr, "COCL!%s!\n", callContextPtr->ioPtr->iclsPtr->nsPtr->fullName);
        hPtr = Tcl_FindHashEntry(&callContextPtr->ioPtr->iclsPtr->variables,
	        (char *)objv[1]);
        if (hPtr == NULL) {
            hPtr = Tcl_FindHashEntry(
	            &callContextPtr->ioPtr->iclsPtr->resolveVars,
	            Tcl_GetString(objv[1]));
	    Tcl_AppendResult(interp, "no such variable 1 \"", name1, NULL);
	    return TCL_ERROR;
	}
	ivPtr = Tcl_GetHashValue(hPtr);
	cp = ItclngGetInstanceVar(interp, name1, name2, callContextPtr->ioPtr,
	        ivPtr->iclsPtr);
        if (cp == NULL) {
	    Tcl_AppendResult(interp, "no such variable 2 \"", name1, NULL);
	    return TCL_ERROR;
	}
	objPtr = Tcl_NewStringObj(cp, -1);
    } else {
	objPtr = Tcl_NewStringObj("", -1);
    }
    Tcl_SetObjResult(interp, objPtr);
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
 *  Used by Itcl_ConfigureCmd() to report configuration options.
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
    Tcl_Obj *configPtr;
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

    ItclngShowArgs(1, "Itclng_ConfigureCmd", objc, objv);
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
//    if (!(contextIclsPtr->flags & ITCL_CLASS)) {
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
	    if (infoPtr->unparsedObjc == -1) {
	        infoPtr->unparsedObjc = 0;
	    } else {
	        unparsedObjc = 0;
	    }
	}
//    }
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
            configPtr = ItclngGetVariableInfoString(ivPtr->iclsPtr,
	            Tcl_GetString(ivPtr->namePtr), "config");
	    result = Tcl_EvalObjEx(interp, configPtr, 0);
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

/*
 * ------------------------------------------------------------------------
 *  Itclng_BiIsaCmd()
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
Itclng_IsaCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngClass *iclsPtr;
    char *token;

    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;

    ItclngShowArgs(1, "Itclng_IsaCmd", objc, objv);
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itclng_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
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
    iclsPtr = Itclng_FindClass(interp, token, /* autoload */ 1);
    if (iclsPtr == NULL) {
        return TCL_ERROR;
    }

//fprintf(stderr, "ICLS!%s!\n", iclsPtr->nsPtr->fullName);
    if (Itclng_ObjectIsa(contextIoPtr, iclsPtr)) {
        Tcl_SetIntObj(Tcl_GetObjResult(interp), 1);
    } else {
        Tcl_SetIntObj(Tcl_GetObjResult(interp), 0);
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_ChainCmd()
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
 *
 *  Returns TCL_OK or TCL_ERROR on failure
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itclng_ChainCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int result = TCL_OK;

    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    Tcl_Obj *cmdlinePtr;
    Tcl_Obj **newobjv;
    Tcl_DString buffer;
    ItclngMemberFunc *imPtr;
    ItclngClass *iclsPtr;
    ItclngHierIter hier;
    char *cmd;
    char *cmd2;
    char *head;

    ItclngShowArgs(1, "Itclng_ChainCmd", objc, objv);
    /*
     *  If this command is not invoked within a class namespace,
     *  signal an error.
     */
    contextIclsPtr = NULL;
    if (Itclng_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
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
    Tcl_Obj * const *cObjv;
    cObjv = Itclng_GetCallFrameObjv(interp);
    if (cObjv == NULL) {
            return TCL_OK;
    }
    if (Itclng_GetCallFrameClientData(interp, 0) == NULL) {
        /* that has been a direct call, so no object in front !! */
	cmd = Tcl_GetString(cObjv[0]);
    } else {
        cmd = Tcl_GetString(cObjv[1]);
    }
//fprintf(stderr, "CHAIN1!%s!%p!%s!%s!%s!\n", cmd, Itclng_GetCallFrameClientData(interp, 0), Tcl_GetCurrentNamespace(interp)->fullName, Itclng_GetUplevelNamespace(interp, 1)->fullName, contextIclsPtr->nsPtr->fullName);
    result = TCL_OK;
    Itclng_ParseNamespPath(cmd, &buffer, &head, &cmd2);
fprintf(stderr, "C!%s!%s!\n", cmd, cmd2);
    if (strcmp(cmd2, "___constructor_init") == 0) {
        cmd2 = "constructor";
    }
//fprintf(stderr, "HEAD!%s!\n", head == NULL ? "(nil)" : head);
    Tcl_DStringFree(&buffer);
#ifndef NOTDEF
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->infoPtr->namespaceClasses,
            (char *)Tcl_GetCurrentNamespace(interp));
    if (hPtr != NULL) {
        contextIclsPtr = Tcl_GetHashValue(hPtr);
//fprintf(stderr, "NC!%s!\n", contextIclsPtr->nsPtr->fullName);
    } else {
        /* must be a direct call from the object, so use the object's
	 * class */
	contextIclsPtr = contextIoPtr->iclsPtr;
    }
//fprintf(stderr, "CMD2!%s!%s!\n", cmd, contextIclsPtr->nsPtr->fullName);
#endif

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
        Itclng_InitHierIter(&hier, contextIoPtr->iclsPtr);
        while ((iclsPtr = Itclng_AdvanceHierIter(&hier)) != NULL) {
//fprintf(stderr, "LOICLS!%s!%s!\n", iclsPtr->nsPtr->fullName, contextIclsPtr->nsPtr->fullName);
            if (iclsPtr == contextIclsPtr) {
                break;
            }
        }
    } else {
        Itclng_InitHierIter(&hier, contextIclsPtr);
        Itclng_AdvanceHierIter(&hier);    /* skip the current class */
    }

    /*
     *  Now search up the class hierarchy for the next implementation.
     *  If found, execute it.  Otherwise, do nothing.
     */
    Tcl_Obj *objPtr;
    objPtr = Tcl_NewStringObj(cmd2, -1);
    Tcl_IncrRefCount(objPtr);
    while ((iclsPtr = Itclng_AdvanceHierIter(&hier)) != NULL) {
        hPtr = Tcl_FindHashEntry(&iclsPtr->functions, (char *)objPtr);
fprintf(stderr, "H!%s!%p!%s!\n", iclsPtr->nsPtr->fullName, hPtr, cmd2);
        if (hPtr) {
            imPtr = (ItclngMemberFunc*)Tcl_GetHashValue(hPtr);

            /*
             *  NOTE:  Avoid the usual "virtual" behavior of
             *         methods by passing the full name as
             *         the command argument.
             */
            cmdlinePtr = Itclng_CreateArgs(interp, Tcl_GetString(imPtr->fullNamePtr),
                objc-1, objv+1);

            (void) Tcl_ListObjGetElements((Tcl_Interp*)NULL, cmdlinePtr,
                &objc, &newobjv);

ItclngShowArgs(1, "Itclng_ChainCmd2", objc-1, newobjv+1);
            Itclng_SetCallFrameNamespace(interp, imPtr->iclsPtr->nsPtr);
	    if (imPtr->flags & ITCLNG_CONSTRUCTOR) {
	        Tcl_SetStringObj(newobjv[0], Tcl_GetCommandName(interp,
		        contextIclsPtr->infoPtr->currIoPtr->accessCmd), -1);
	        result = Itclng_EvalMemberCode(interp, imPtr,
		        imPtr->iclsPtr->infoPtr->currIoPtr, objc-1, newobjv+1);
	    } else {
	        result = Itclng_EvalMemberCode(interp, imPtr, contextIoPtr,
		        objc-1, newobjv+1);
            }

            Tcl_DecrRefCount(cmdlinePtr);
            break;
	}
    }
    Tcl_DecrRefCount(objPtr);

    Itclng_DeleteHierIter(&hier);
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_FindClassesCmd()
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
Itclng_FindClassesCmd(
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
    Itclng_Stack search;
    Tcl_Command cmd;
    Tcl_Command originalCmd;
    Tcl_Namespace *nsPtr;
    Tcl_Obj *objPtr;

    ItclngShowArgs(2, "Itclng_FindClassesCmd", objc, objv);
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

    Itclng_InitStack(&search);
    Itclng_PushStack((ClientData)globalNs, &search);
    Itclng_PushStack((ClientData)activeNs, &search);  /* last in, first out! */

    Tcl_InitHashTable(&unique, TCL_ONE_WORD_KEYS);

    handledActiveNs = 0;
    while (Itclng_GetStackSize(&search) > 0) {
        nsPtr = Itclng_PopStack(&search);
        if (nsPtr == activeNs && handledActiveNs) {
            continue;
        }

        entry = Tcl_FirstHashEntry(Tcl_GetNamespaceCommandTable(nsPtr),
	        &place);
        while (entry) {
            cmd = (Tcl_Command)Tcl_GetHashValue(entry);
            if (Itclng_IsClass(cmd)) {
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
            Itclng_PushStack(Tcl_GetHashValue(entry), &search);
            entry = Tcl_NextHashEntry(&place);
        }
    }
    Tcl_DeleteHashTable(&unique);
    Itclng_DeleteStack(&search);

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_FindObjectsCmd()
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
Itclng_FindObjectsCmd(
    ClientData clientData,   /* class/object info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);
    Tcl_Namespace *globalNs = Tcl_GetGlobalNamespace(interp);
    int forceFullNames = 0;

    char *pattern = NULL;
    ItclngClass *iclsPtr = NULL;
    ItclngClass *isaDefn = NULL;

    char *name = NULL;
    char *token = NULL;
    CONST char *cmdName = NULL;
    int pos;
    int newEntry;
    int match;
    int handledActiveNs;
    ItclngObject *contextIoPtr;
    Tcl_HashTable unique;
    Tcl_HashEntry *entry;
    Tcl_HashSearch place;
    Itclng_Stack search;
    Tcl_Command cmd;
    Tcl_Command originalCmd;
    Tcl_CmdInfo cmdInfo;
    Tcl_Namespace *nsPtr;
    Tcl_Obj *objPtr;

    ItclngShowArgs(1, "Itclng_FindObjects", objc, objv);
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
            iclsPtr = Itclng_FindClass(interp, name, /* autoload */ 1);
            if (iclsPtr == NULL) {
                return TCL_ERROR;
            }
            pos++;
        }
        else if ((pos+1 < objc) && (strcmp(token,"-isa") == 0)) {
            name = Tcl_GetString(objv[pos+1]);
            isaDefn = Itclng_FindClass(interp, name, /* autoload */ 1);
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

    Itclng_InitStack(&search);
    Itclng_PushStack((ClientData)globalNs, &search);
    Itclng_PushStack((ClientData)activeNs, &search);  /* last in, first out! */

    Tcl_InitHashTable(&unique, TCL_ONE_WORD_KEYS);

    handledActiveNs = 0;
    while (Itclng_GetStackSize(&search) > 0) {
        nsPtr = Itclng_PopStack(&search);
        if (nsPtr == activeNs && handledActiveNs) {
            continue;
        }

        entry = Tcl_FirstHashEntry(Tcl_GetNamespaceCommandTable(nsPtr), &place);
        while (entry) {
            cmd = (Tcl_Command)Tcl_GetHashValue(entry);
            if (Itclng_IsObject(cmd)) {
                originalCmd = Tcl_GetOriginalCommand(cmd);
                if (originalCmd) {
                    cmd = originalCmd;
                }
		Tcl_GetCommandInfoFromToken(cmd, &cmdInfo);
                contextIoPtr = (ItclngObject*)cmdInfo.deleteData;

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
            Itclng_PushStack(Tcl_GetHashValue(entry), &search);
            entry = Tcl_NextHashEntry(&place);
        }
    }
    Tcl_DeleteHashTable(&unique);
    Itclng_DeleteStack(&search);

    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_DeleteClassCmd()
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
Itclng_DeleteClassCmd(
    ClientData clientData,   /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int i;
    char *name;
    ItclngClass *iclsPtr;

    ItclngShowArgs(1, "Itclng_DeleteClassCmd", objc, objv);
    /*
     *  Since destroying a base class will destroy all derived
     *  classes, calls like "destroy class Base Derived" could
     *  fail.  Break this into two passes:  first check to make
     *  sure that all classes on the command line are valid,
     *  then delete them.
     */
    for (i=1; i < objc; i++) {
        name = Tcl_GetString(objv[i]);
        iclsPtr = Itclng_FindClass(interp, name, /* autoload */ 1);
        if (iclsPtr == NULL) {
            return TCL_ERROR;
        }
    }

    for (i=1; i < objc; i++) {
        name = Tcl_GetString(objv[i]);
        iclsPtr = Itclng_FindClass(interp, name, /* autoload */ 0);

        if (iclsPtr) {
            Tcl_ResetResult(interp);
            if (Itclng_DeleteClass(interp, iclsPtr) != TCL_OK) {
                return TCL_ERROR;
            }
        }
    }
    Tcl_ResetResult(interp);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_DeleteObjectCmd()
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
Itclng_DeleteObjectCmd(
    ClientData clientData,   /* object management info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngObject *contextIoPtr;
    char *name;
    int i;

    ItclngShowArgs(1, "Itclng_DeleteObjectCmd", objc, objv);
    /*
     *  Scan through the list of objects and attempt to delete them.
     *  If anything goes wrong (i.e., destructors fail), then
     *  abort with an error.
     */
    for (i=1; i < objc; i++) {
        name = Tcl_GetStringFromObj(objv[i], (int*)NULL);
        if (Itclng_FindObject(interp, name, &contextIoPtr) != TCL_OK) {
	    return TCL_ERROR;
        }

        if (contextIoPtr == NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "object \"", name, "\" not found",
                (char*)NULL);
	    return TCL_ERROR;
        }

        if (Itclng_DeleteObject(interp, contextIoPtr) != TCL_OK) {
	    return TCL_ERROR;
        }
    }
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_ScopeCmd()
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
Itclng_ScopeCmd(
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
    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;
    Tcl_HashEntry *entry;
    ItclngObjectInfo *infoPtr;
    ItclngVarLookup *vlookup;
    int doAppend;

    ItclngShowArgs(1, "Itclng_ScopeCmd", objc, objv);
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
    infoPtr = Tcl_GetAssocData(interp, ITCLNG_INTERP_DATA, &procPtr);
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)contextNsPtr);
    if (hPtr != NULL) {
        contextIclsPtr = (ItclngClass *)Tcl_GetHashValue(hPtr);
    }
    if (Itclng_IsClassNamespace(contextNsPtr)) {

        entry = Tcl_FindHashEntry(&contextIclsPtr->resolveVars, token);
        if (!entry) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "variable \"", token, "\" not found in class \"",
                Tcl_GetString(contextIclsPtr->fullNamePtr), "\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto scopeCmdDone;
        }
        vlookup = (ItclngVarLookup*)Tcl_GetHashValue(entry);

        if (vlookup->ivPtr->flags & ITCLNG_COMMON) {
            Tcl_Obj *resultPtr = Tcl_GetObjResult(interp);
	    if (vlookup->ivPtr->protection != ITCLNG_PUBLIC) {
	        Tcl_AppendToObj(resultPtr, Tcl_GetString(
		        vlookup->ivPtr->iclsPtr->infoPtr->internalVars), -1);
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
        clientData = Itclng_GetCallFrameClientData(interp, 1);
        if (clientData != NULL) {
            oPtr = Tcl_ObjectContextObject((Tcl_ObjectContext)clientData);
            if (oPtr != NULL) {
                contextIoPtr = (ItclngObject*)Tcl_ObjectGetMetadata(
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
        if (strcmp(token, "itcl_options") == 0) {
	    doAppend = 0;
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
	Tcl_AppendToObj(objPtr2, Tcl_GetString(
	        vlookup->ivPtr->iclsPtr->infoPtr->internalVars), -1);
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

        var = Itclng_FindNamespaceVar(interp, token, contextNsPtr,
            TCL_NAMESPACE_ONLY);

        if (!var) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "variable \"", token, "\" not found in namespace \"",
                contextNsPtr->fullName, "\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto scopeCmdDone;
        }

        Itclng_GetVariableFullName(interp, var, resultPtr);
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
 *  Itclng_CodeCmd()
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
Itclng_CodeCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);

    int pos;
    char *token;
    Tcl_Obj *listPtr, *objPtr;

    ItclngShowArgs(1, "Itclng_CodeCmd", objc, objv);
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
 *  Itclng_IsObjectCmd()
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
Itclng_IsObjectCmd(
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
    ItclngClass       *iclsPtr = NULL;
    ItclngObject      *contextObj;

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
            iclsPtr = Itclng_FindClass(interp, cname, /* no autoload */ 0);

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
    if (Itclng_DecodeScopedCommand(interp, name, &contextNs, &cmdName)
        != TCL_OK) {
        return TCL_ERROR;
    }

    cmd = Tcl_FindCommand(interp, cmdName, contextNs, /* flags */ 0);

    /*
     *    Need the NULL test, or the test will fail if cmd is NULL
     */
    if (cmd == NULL || ! Itclng_IsObject(cmd)) {
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(0));
        return TCL_OK;
    }

    /*
     *    Handle the case when the -class flag is given
     */
    if (classFlag) {
	Tcl_CmdInfo cmdInfo;
        if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) == 1) {
            contextObj = (ItclngObject*)cmdInfo.objClientData;
            if (! Itclng_ObjectIsa(contextObj, iclsPtr)) {
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
 *  Itclng_IsClassCmd()
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
Itclng_IsClassCmd(
    ClientData clientData,   /* class/object info */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{

    char           *cname;
    char           *name;
    ItclngClass      *iclsPtr = NULL;
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
    if (Itclng_DecodeScopedCommand(interp, name, &contextNs, &cname) != TCL_OK) {
        return TCL_ERROR;
    }

    iclsPtr = Itclng_FindClass(interp, cname, /* no autoload */ 0);

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

#ifdef NOTDEF

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

    ItclShowArgs(1, "Itcl_ForwardAddCmd", objc, objv);
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
    ItclShowArgs(1, "Itcl_NWidgetCmd", objc-1, objv);
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
    ItclShowArgs(1, "Itcl_AddOptionCmd", objc, objv);
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

/*
 * ------------------------------------------------------------------------
 *  ItclngExtendedConfigure()
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
ItclngExtendedConfigure(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    Tcl_Object oPtr;
    Tcl_Obj *resultPtr;
    Tcl_Obj *objPtr;
    Tcl_Obj *methodNamePtr;
    Tcl_Obj *configureMethodPtr;
    Tcl_Namespace *saveNsPtr;
    Tcl_Namespace *evalNsPtr;
    Tcl_Obj **newObjv;
    ItclngVarLookup *vlookup;
    ItclngDelegatedFunction *idmPtr;
    ItclngDelegatedOption *idoPtr;
    ItclngObject *ioPtr;
    ItclngClass *iclsPtr;
    ItclngComponent *icPtr;
    ItclngOption *ioptPtr;
    ItclngObjectInfo *infoPtr;
    const char *val;
    char *token;
    int i;
    int result;

    ItclngShowArgs(1, "ItclngExtendedConfigure", objc, objv);
    vlookup = NULL;
    token = NULL;
    ioptPtr = NULL;
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
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
    if (infoPtr->currContextIclsPtr != NULL) {
        contextIclsPtr = infoPtr->currContextIclsPtr;
    }

    hPtr = NULL;
    /* first check if method configure is delegated */
    methodNamePtr = Tcl_NewStringObj("*", -1);
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->delegatedFunctions, (char *)
            methodNamePtr);
    if (hPtr != NULL) {
        idmPtr = (ItclngDelegatedFunction *)Tcl_GetHashValue(hPtr);
	Tcl_SetStringObj(methodNamePtr, "configure", -1);
        hPtr = Tcl_FindHashEntry(&idmPtr->exceptions, (char *)methodNamePtr);
        if (hPtr == NULL) {
	    icPtr = idmPtr->icPtr;
	    val = ItclngGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
	            NULL, contextIoPtr, iclsPtr);
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
                    ioPtr = (ItclngObject *)Tcl_ObjectGetMetadata(oPtr,
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
	infoPtr->unparsedObjc = -1;
        return TCL_CONTINUE;
fprintf(stderr, "plain configure for options not yet implemented\n");
        return TCL_ERROR;
    }
    /* first handle delegated options */
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectDelegatedOptions, (char *)
            objv[1]);
    if (hPtr != NULL) {
	/* the option is delegated */
        idoPtr = (ItclngDelegatedOption *)Tcl_GetHashValue(hPtr);
        icPtr = idoPtr->icPtr;
        val = ItclngGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
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
                ioPtr = (ItclngObject *)Tcl_ObjectGetMetadata(oPtr,
                        infoPtr->object_meta_type);
	        infoPtr->currContextIclsPtr = ioPtr->iclsPtr;
	    }
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
	    infoPtr->unparsedObjc = -1;
	    /* no option at all, let the normal configure do the job */
	    return TCL_CONTINUE;
        }
        ioptPtr = (ItclngOption *)Tcl_GetHashValue(hPtr);
        resultPtr = ItclngReportOption(interp, ioptPtr, contextIoPtr);
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
	    /* FIX ME !!! temporary */
	    continue;
        }
        ioptPtr = (ItclngOption *)Tcl_GetHashValue(hPtr);
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
	if (ioptPtr->configureMethodPtr != NULL) {
	    configureMethodPtr = ioptPtr->configureMethodPtr;
	    Tcl_IncrRefCount(configureMethodPtr);
	    evalNsPtr = ioptPtr->iclsPtr->nsPtr;
	}
	if (ioptPtr->configureMethodVarPtr != NULL) {
	    val = ItclngGetInstanceVar(interp,
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
		ItclngMemberFunc *imPtr;
		imPtr = (ItclngMemberFunc *)Tcl_GetHashValue(hPtr);
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
	    Itclng_SetCallFrameNamespace(interp, evalNsPtr);
            result = Tcl_EvalObjv(interp, 3, newObjv, TCL_EVAL_DIRECT);
	    Tcl_DecrRefCount(newObjv[0]);
	    Tcl_DecrRefCount(newObjv[1]);
	    Tcl_DecrRefCount(newObjv[2]);
            ckfree((char *)newObjv);
	    Itclng_SetCallFrameNamespace(interp, saveNsPtr);
	    Tcl_DecrRefCount(configureMethodPtr);
	    if (result != TCL_OK) {
	        break;
	    }
	} else {
	    if (ItclngSetInstanceVar(interp, "itcl_options",
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
 *  ItclngExtendedCget()
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
ItclngExtendedCget(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    Tcl_Obj *objPtr;
    Tcl_Object oPtr;
    Tcl_Obj *methodNamePtr;
    Tcl_Obj **newObjv;
    ItclngDelegatedFunction *idmPtr;
    ItclngDelegatedOption *idoPtr;
    ItclngComponent *icPtr;
    ItclngObjectInfo *infoPtr;
    ItclngOption *ioptPtr;
    ItclngObject *ioPtr;
    const char *val;
    int i;
    int result;

    ItclngShowArgs(1,"ItclngExtendedCget", objc, objv);
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
        idmPtr = (ItclngDelegatedFunction *)Tcl_GetHashValue(hPtr);
	Tcl_SetStringObj(methodNamePtr, "cget", -1);
        hPtr = Tcl_FindHashEntry(&idmPtr->exceptions, (char *)methodNamePtr);
        if (hPtr == NULL) {
	    icPtr = idmPtr->icPtr;
	    val = ItclngGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
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
                    ioPtr = (ItclngObject *)Tcl_ObjectGetMetadata(oPtr,
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
        idoPtr = (ItclngDelegatedOption *)Tcl_GetHashValue(hPtr);
        icPtr = idoPtr->icPtr;
        val = ItclngGetInstanceVar(interp, Tcl_GetString(icPtr->namePtr),
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
                ioPtr = (ItclngObject *)Tcl_ObjectGetMetadata(oPtr,
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
    ioptPtr = (ItclngOption *)Tcl_GetHashValue(hPtr);
    result = TCL_CONTINUE;
    if (ioptPtr->cgetMethodPtr != NULL) {
        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*2);
        newObjv[0] = ioptPtr->cgetMethodPtr;
        newObjv[1] = objv[1];
        result = Tcl_EvalObjv(interp, objc, newObjv, TCL_EVAL_DIRECT);
    } else {
        val = ItclngGetInstanceVar(interp, "itcl_options",
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
 *  ItclngExtendedSetGet()
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
ItclngExtendedSetGet(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngClass *contextIclsPtr;
    ItclngObject *contextIoPtr;

    Tcl_HashEntry *hPtr;
    Tcl_Obj **newObjv;
    ItclngMethodVariable *imvPtr;
    ItclngObjectInfo *infoPtr;
    const char *usageStr;
    const char *val;
    char *token;
    int result;
    int setValue;

    ItclngShowArgs(1, "ItclngExtendedSetGet", objc, objv);
    token = NULL;
    imvPtr = NULL;
    result = TCL_OK;
    /*
     *  Make sure that this command is being invoked in the proper
     *  context.
     */
    contextIclsPtr = NULL;
    if (Itclng_GetContext(interp, &contextIclsPtr, &contextIoPtr) != TCL_OK) {
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
    imvPtr = (ItclngMethodVariable *)Tcl_GetHashValue(hPtr);
    if (objc == 2) {
        val = ItclngGetInstanceVar(interp, Tcl_GetString(objv[1]), NULL, 
	        contextIoPtr, imvPtr->iclsPtr);
        if (val == NULL) {
            result = TCL_ERROR;
        } else {
	   Tcl_SetResult(interp, (char *)val, TCL_VOLATILE);
	}
        return result;
    }
    imvPtr = (ItclngMethodVariable *)Tcl_GetHashValue(hPtr);
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
            if (ItclngSetInstanceVar(interp, Tcl_GetString(objv[1]), NULL, 
	            Tcl_GetString(objv[2]), contextIoPtr,
		    imvPtr->iclsPtr) == NULL) {
                result = TCL_ERROR;
            }
        }
    }
    return result;
}
