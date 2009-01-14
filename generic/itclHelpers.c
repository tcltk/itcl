/*
 * itclHelpers.c --
 *
 * This file contains the C-implemeted part of 
 * Itcl 
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclHelpers.c,v 1.1.2.12 2009/01/14 22:43:24 davygrvy Exp $
 */

#include "itclInt.h"

void ItclDeleteArgList(ItclArgList *arglistPtr);
#ifdef ITCL_DEBUG
int _itcl_debug_level = 0;

/*
 * ------------------------------------------------------------------------
 *  ItclShowArgs()
 * ------------------------------------------------------------------------
 */

void
ItclShowArgs(
    int level,
    const char *str,
    int objc,
    Tcl_Obj * const* objv)
{
    int i;

    if (level > _itcl_debug_level) {
        return;
    }
    fprintf(stderr, "%s", str);
    for (i = 0; i < objc; i++) {
        fprintf(stderr, "!%s", objv[i] == NULL ? "??" :
                Tcl_GetString(objv[i]));
    }
    fprintf(stderr, "!\n");
}
#endif

/*
 * ------------------------------------------------------------------------
 *  Itcl_ProtectionStr()
 *
 *  Converts an integer protection code (ITCL_PUBLIC, ITCL_PROTECTED,
 *  or ITCL_PRIVATE) into a human-readable character string.  Returns
 *  a pointer to this string.
 * ------------------------------------------------------------------------
 */
char*
Itcl_ProtectionStr(
    int pLevel)     /* protection level */
{
    switch (pLevel) {
    case ITCL_PUBLIC:
        return "public";
    case ITCL_PROTECTED:
        return "protected";
    case ITCL_PRIVATE:
        return "private";
    }
    return "<bad-protection-code>";
}

/*
 * ------------------------------------------------------------------------
 *  ItclCreateArgList()
 * ------------------------------------------------------------------------
 */

int
ItclCreateArgList(
    Tcl_Interp *interp,		/* interpreter managing this function */
    const char *str,		/* string representing argument list */
    int *argcPtr,		/* number of mandatory arguments */
    int *maxArgcPtr,		/* number of arguments parsed */
    Tcl_Obj **usagePtr,         /* store usage message for arguments here */
    ItclArgList **arglistPtrPtr,
    				/* returns pointer to parsed argument list */
    ItclMemberFunc *mPtr,
    const char *commandName)
{
    int argc;
    int defaultArgc;
    const char **argv;
    const char **defaultArgv;
    ItclArgList *arglistPtr;
    ItclArgList *lastArglistPtr;
    int i;
    int hadArgsArgument;
    int result;

    *arglistPtrPtr = NULL;
    lastArglistPtr = NULL;
    argc = 0;
    hadArgsArgument = 0;
    result = TCL_OK;
    *maxArgcPtr = 0;
    *argcPtr = 0;
    *usagePtr = Tcl_NewStringObj("", -1);
    if (str) {
        if (Tcl_SplitList(interp, (const char *)str, &argc, &argv)
	        != TCL_OK) {
	    return TCL_ERROR;
	}
	i = 0;
	if (argc == 0) {
	   /* signal there are 0 arguments */
            arglistPtr = (ItclArgList *)ckalloc(sizeof(ItclArgList));
	    memset(arglistPtr, 0, sizeof(ItclArgList));
	    *arglistPtrPtr = arglistPtr;
	}
        while (i < argc) {
            if (Tcl_SplitList(interp, argv[i], &defaultArgc, &defaultArgv)
	            != TCL_OK) {
	        result = TCL_ERROR;
	        break;
	    }
	    arglistPtr = NULL;
	    if (defaultArgc == 0 || defaultArgv[0][0] == '\0') {
		if (commandName != NULL) {
	            Tcl_AppendResult(interp, "procedure \"",
		            commandName,
			    "\" has argument with no name", NULL);
		} else {
	            char buf[10];
		    sprintf(buf, "%d", i);
		    Tcl_AppendResult(interp, "argument #", buf,
		            " has no name", NULL);
		}
	        result = TCL_ERROR;
	        break;
	    }
	    if (defaultArgc > 2) {
	        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
	            "too many fields in argument specifier \"",
		    argv[i], "\"",
		    (char*)NULL);
	        result = TCL_ERROR;
	        break;
	    }
	    if (strstr(defaultArgv[0],"::")) {
	        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
		        "bad argument name \"", defaultArgv[0], "\"",
			(char*)NULL);
		result = TCL_ERROR;
		break;
	    }
            arglistPtr = (ItclArgList *)ckalloc(sizeof(ItclArgList));
	    memset(arglistPtr, 0, sizeof(ItclArgList));
            if (*arglistPtrPtr == NULL) {
	         *arglistPtrPtr = arglistPtr;
	    } else {
	        lastArglistPtr->nextPtr = arglistPtr;
	        Tcl_AppendToObj(*usagePtr, " ", 1);
	    }
	    arglistPtr->namePtr = 
	            Tcl_NewStringObj(defaultArgv[0], -1);
	    (*maxArgcPtr)++;
	    if (defaultArgc == 1) {
		(*argcPtr)++;
	        arglistPtr->defaultValuePtr = NULL;
		if ((strcmp(defaultArgv[0], "args") == 0) && (i == argc-1)) {
		    hadArgsArgument = 1;
		    (*argcPtr)--;
	            Tcl_AppendToObj(*usagePtr, "?arg arg ...?", -1);
		} else {
	            Tcl_AppendToObj(*usagePtr, defaultArgv[0], -1);
	        }
	    } else {
	        arglistPtr->defaultValuePtr = 
		        Tcl_NewStringObj(defaultArgv[1], -1);
	        Tcl_AppendToObj(*usagePtr, "?", 1);
	        Tcl_AppendToObj(*usagePtr, defaultArgv[0], -1);
	        Tcl_AppendToObj(*usagePtr, "?", 1);
	    }
            lastArglistPtr = arglistPtr;
	    i++;
        }
    }
    /*
     *  If anything went wrong, destroy whatever arguments were
     *  created and return an error.
     */
    if (result != TCL_OK) {
        ItclDeleteArgList(*arglistPtrPtr);
        *arglistPtrPtr = NULL;
    }
    if (hadArgsArgument) {
        *maxArgcPtr = -1;
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclDeleteArgList()
 * ------------------------------------------------------------------------
 */

void
ItclDeleteArgList(
    ItclArgList *arglistPtr)	/* first argument in arg list chain */
{
    ItclArgList *currPtr;
    ItclArgList *nextPtr;

    for (currPtr=arglistPtr; currPtr; currPtr=nextPtr) {
	if (currPtr->defaultValuePtr != NULL) {
	    Tcl_DecrRefCount(currPtr->defaultValuePtr);
	}
	if (currPtr->namePtr != NULL) {
	    Tcl_DecrRefCount(currPtr->namePtr);
	}
        nextPtr = currPtr->nextPtr;
        ckfree((char *)currPtr);
    }
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_EvalArgs()
 *
 *  This procedure invokes a list of (objc,objv) arguments as a
 *  single command.  It is similar to Tcl_EvalObj, but it doesn't
 *  do any parsing or compilation.  It simply treats the first
 *  argument as a command and invokes that command in the current
 *  context.
 *
 *  Returns TCL_OK if successful.  Otherwise, this procedure returns
 *  TCL_ERROR along with an error message in the interpreter.
 * ------------------------------------------------------------------------
 */
int
Itcl_EvalArgs(
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int result;
    Tcl_Command cmd;
    int cmdlinec;
    Tcl_Obj **cmdlinev;
    Tcl_Obj *cmdlinePtr = NULL;
    Tcl_CmdInfo infoPtr;

    /*
     * Resolve the command by converting it to a CmdName object.
     * This caches a pointer to the Command structure for the
     * command, so if we need it again, it's ready to use.
     */
    cmd = Tcl_GetCommandFromObj(interp, objv[0]);

    cmdlinec = objc;
    cmdlinev = (Tcl_Obj	**) objv;

    /*
     * If the command is still not found, handle it with the
     * "unknown" proc.
     */
    if (cmd == NULL) {
        cmd = Tcl_FindCommand(interp, "unknown",
            (Tcl_Namespace *) NULL, /*flags*/ TCL_GLOBAL_ONLY);

        if (cmd == NULL) {
            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "invalid command name \"",
                Tcl_GetStringFromObj(objv[0], NULL), "\"", NULL);
            return TCL_ERROR;
        }

        cmdlinePtr = Itcl_CreateArgs(interp, "unknown", objc, objv);
        Tcl_ListObjGetElements(NULL, cmdlinePtr, &cmdlinec, &cmdlinev);
    }

    /*
     *  Finally, invoke the command's Tcl_ObjCmdProc.  Be careful
     *  to pass in the proper client data.
     */
    Tcl_ResetResult(interp);
    result = Tcl_GetCommandInfoFromToken(cmd, &infoPtr);
    if (result == 1) {
        result = (infoPtr.objProc)(infoPtr.objClientData, interp,
                cmdlinec, cmdlinev);
    }

    if (cmdlinePtr) {
        Tcl_DecrRefCount(cmdlinePtr);
    }
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_CreateArgs()
 *
 *  This procedure takes a string and a list of (objc,objv) arguments,
 *  and glues them together in a single list.  This is useful when
 *  a command word needs to be prepended or substituted into a command
 *  line before it is executed.  The arguments are returned in a single
 *  list object, and they can be retrieved by calling
 *  Tcl_ListObjGetElements.  When the arguments are no longer needed,
 *  they should be discarded by decrementing the reference count for
 *  the list object.
 *
 *  Returns a pointer to the list object containing the arguments.
 * ------------------------------------------------------------------------
 */
Tcl_Obj*
Itcl_CreateArgs(
    Tcl_Interp *interp,      /* current interpreter */
    const char *string,      /* first command word */
    int objc,                /* number of arguments */
    Tcl_Obj *const objv[])   /* argument objects */
{
    int i;
    Tcl_Obj *listPtr;
    Tcl_Obj *objPtr;

    ItclShowArgs(1, "Itcl_CreateArgs", objc, objv);
    listPtr = Tcl_NewListObj(0, (Tcl_Obj**)NULL);
    objPtr = Tcl_NewStringObj("my", -1);
    Tcl_IncrRefCount(objPtr);
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);
    objPtr = Tcl_NewStringObj(string, -1);
    Tcl_IncrRefCount(objPtr);
    Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objPtr);

    for (i=0; i < objc; i++) {
        Tcl_ListObjAppendElement((Tcl_Interp*)NULL, listPtr, objv[i]);
    }
    return listPtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclEnsembleSubCmd()
 * ------------------------------------------------------------------------
 */

int
ItclEnsembleSubCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    const char *ensembleName,
    int objc,
    Tcl_Obj *const *objv,
    const char *functionName)
{
    int result;
    Tcl_Obj **newObjv;
    int isRootEnsemble;
    ItclShowArgs(2, functionName, objc, objv);

    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+1));
    isRootEnsemble = Tcl_InitRewriteEnsemble(interp, 1, 2, objc, objv);
    newObjv[0] = Tcl_NewStringObj("::info", -1);
    Tcl_IncrRefCount(newObjv[0]);
    newObjv[1] = Tcl_NewStringObj("itclinfo", -1);
    Tcl_IncrRefCount(newObjv[1]);
    if (objc > 1) {
        memcpy(newObjv+2, objv+1, sizeof(Tcl_Obj *) * (objc-1));
    }
    result = Tcl_EvalObjv(interp, objc+1, newObjv, TCL_EVAL_INVOKE);
    Tcl_DecrRefCount(newObjv[0]);
    Tcl_DecrRefCount(newObjv[1]);
    ckfree((char *)newObjv);
    Tcl_ResetRewriteEnsemble(interp, isRootEnsemble);
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  ItclTraceUnsetVar()
 * ------------------------------------------------------------------------
 */

char *
ItclTraceUnsetVar(
    ClientData clientData,
    Tcl_Interp *interp,
    const char *name1,
    const char *name2,
    int flags)
{
#ifdef NOTDEF
    IctlVarTraceInfo *tracePtr;
    Tcl_HashEntry *hPtr;
#endif

    if (name2 != NULL) {
        /* unsetting of an array element nothing to do */
	return NULL;
    }
    /* also when unsetting variables, they stay alive until the class
     * or object is teared down!!
     */
#ifdef NOTDEF
    tracePtr = (IctlVarTraceInfo *)clientData;
    if (tracePtr->flags & ITCL_TRACE_CLASS) {
        hPtr = Tcl_FindHashEntry(&tracePtr->iclsPtr->classCommons,
	        (char *)tracePtr->ivPtr);
	if (hPtr != NULL) {
	    Tcl_DeleteHashEntry(hPtr);
	}
    }
    if (tracePtr->flags & ITCL_TRACE_OBJECT) {
        hPtr = Tcl_FindHashEntry(&tracePtr->ioPtr->objectVariables,
	        (char *)tracePtr->ivPtr);
	if (hPtr != NULL) {
	    Tcl_DeleteHashEntry(hPtr);
	}
    }
    ckfree((char *)tracePtr);
#endif
    return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  ItclCapitalize()
 * ------------------------------------------------------------------------
 */

Tcl_Obj *
ItclCapitalize(
    const char *str)
{
    Tcl_Obj *objPtr;
    char buf[2];
    
    sprintf(buf, "%c", toupper(*str));
    buf[1] = '\0';
    objPtr = Tcl_NewStringObj(buf, -1);
    Tcl_AppendToObj(objPtr, str+1, -1);
    return objPtr;
}
