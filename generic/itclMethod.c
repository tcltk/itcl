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
 *  These procedures handle commands available within a class scope.
 *  In [incr Tcl], the term "method" is used for a procedure that has
 *  access to object-specific data, while the term "proc" is used for
 *  a procedure that has access only to common class data.
 *
 * ========================================================================
 *  AUTHOR:  Michael J. McLennan
 *           Bell Labs Innovations for Lucent Technologies
 *           mmclennan@lucent.com
 *           http://www.tcltk.com/itcl
 *
 *  overhauled version author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclMethod.c,v 1.1.2.20 2008/09/28 10:41:38 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"

static int EquivArgLists(Tcl_Interp *interp, ItclArgList *origArgs,
        ItclArgList *realArgs);
static void DeleteArgList(ItclArgList *arglistPtr);

/*
 * ------------------------------------------------------------------------
 *  Itcl_BodyCmd()
 *
 *  Invoked by Tcl whenever the user issues an "itcl::body" command to
 *  define or redefine the implementation for a class method/proc.
 *  Handles the following syntax:
 *
 *    itcl::body <class>::<func> <arglist> <body>
 *
 *  Looks for an existing class member function with the name <func>,
 *  and if found, tries to assign the implementation.  If an argument
 *  list was specified in the original declaration, it must match
 *  <arglist> or an error is flagged.  If <body> has the form "@name"
 *  then it is treated as a reference to a C handling procedure;
 *  otherwise, it is taken as a body of Tcl statements.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
static int
NRBodyCmd(
    ClientData clientData,   /*  */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *const *objv)    /* argument objects */
{
    int status = TCL_OK;

    char *head;
    char *tail;
    char *token;
    char *arglist;
    char *body;
    ItclClass *iclsPtr;
    ItclMemberFunc *imPtr;
    Tcl_HashEntry *entry;
    Tcl_DString buffer;

    ItclShowArgs(2, "Itcl_BodyCmd", objc, objv);
    if (objc != 4) {
        token = Tcl_GetString(objv[0]);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "wrong # args: should be \"",
            token, " class::func arglist body\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Parse the member name "namesp::namesp::class::func".
     *  Make sure that a class name was specified, and that the
     *  class exists.
     */
    token = Tcl_GetString(objv[1]);
    Itcl_ParseNamespPath(token, &buffer, &head, &tail);

    if (!head || *head == '\0') {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "missing class specifier for body declaration \"", token, "\"",
            (char*)NULL);
        status = TCL_ERROR;
        goto bodyCmdDone;
    }

    iclsPtr = Itcl_FindClass(interp, head, /* autoload */ 1);
    if (iclsPtr == NULL) {
        status = TCL_ERROR;
        goto bodyCmdDone;
    }

    /*
     *  Find the function and try to change its implementation.
     *  Note that command resolution table contains *all* functions,
     *  even those in a base class.  Make sure that the class
     *  containing the method definition is the requested class.
     */

    imPtr = NULL;
    entry = Tcl_FindHashEntry(&iclsPtr->resolveCmds, tail);
    if (entry) {
        imPtr = (ItclMemberFunc*)Tcl_GetHashValue(entry);
        if (imPtr->iclsPtr != iclsPtr) {
            imPtr = NULL;
        }
    }

    if (imPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "function \"", tail, "\" is not defined in class \"",
            Tcl_GetString(iclsPtr->fullNamePtr), "\"",
            (char*)NULL);
        status = TCL_ERROR;
        goto bodyCmdDone;
    }

    arglist = Tcl_GetString(objv[2]);
    body    = Tcl_GetString(objv[3]);

    if (Itcl_ChangeMemberFunc(interp, imPtr, arglist, body) != TCL_OK) {
        status = TCL_ERROR;
        goto bodyCmdDone;
    }

bodyCmdDone:
    Tcl_DStringFree(&buffer);
    return status;
}

/* ARGSUSED */
int
Itcl_BodyCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    return Itcl_NRCallObjProc(clientData, interp, NRBodyCmd, objc, objv);
}



/*
 * ------------------------------------------------------------------------
 *  Itcl_ConfigBodyCmd()
 *
 *  Invoked by Tcl whenever the user issues an "itcl::configbody" command
 *  to define or redefine the configuration code associated with a
 *  public variable.  Handles the following syntax:
 *
 *    itcl::configbody <class>::<publicVar> <body>
 *
 *  Looks for an existing public variable with the name <publicVar>,
 *  and if found, tries to assign the implementation.  If <body> has
 *  the form "@name" then it is treated as a reference to a C handling
 *  procedure; otherwise, it is taken as a body of Tcl statements.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
NRConfigBodyCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int status = TCL_OK;

    char *head;
    char *tail;
    char *token;
    Tcl_DString buffer;
    ItclClass *iclsPtr;
    ItclVarLookup *vlookup;
    ItclVariable *ivPtr;
    ItclMemberCode *mcode;
    Tcl_HashEntry *entry;

    ItclShowArgs(2, "Itcl_ConfigBodyCmd", objc, objv);
    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "class::option body");
        return TCL_ERROR;
    }

    /*
     *  Parse the member name "namesp::namesp::class::option".
     *  Make sure that a class name was specified, and that the
     *  class exists.
     */
    token = Tcl_GetString(objv[1]);
    Itcl_ParseNamespPath(token, &buffer, &head, &tail);

    if ((head == NULL) || (*head == '\0')) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "missing class specifier for body declaration \"", token, "\"",
            (char*)NULL);
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }

    iclsPtr = Itcl_FindClass(interp, head, /* autoload */ 1);
    if (iclsPtr == NULL) {
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }

    /*
     *  Find the variable and change its implementation.
     *  Note that variable resolution table has *all* variables,
     *  even those in a base class.  Make sure that the class
     *  containing the variable definition is the requested class.
     */
    vlookup = NULL;
    entry = Tcl_FindHashEntry(&iclsPtr->resolveVars, tail);
    if (entry) {
        vlookup = (ItclVarLookup*)Tcl_GetHashValue(entry);
        if (vlookup->ivPtr->iclsPtr != iclsPtr) {
            vlookup = NULL;
        }
    }

    if (vlookup == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "option \"", tail, "\" is not defined in class \"",
            Tcl_GetString(iclsPtr->fullNamePtr), "\"",
            (char*)NULL);
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }
    ivPtr = vlookup->ivPtr;

    if (ivPtr->protection != ITCL_PUBLIC) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "option \"", Tcl_GetString(ivPtr->fullNamePtr),
                "\" is not a public configuration option",
                (char*)NULL);
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }

    token = Tcl_GetString(objv[2]);

    if (Itcl_CreateMemberCode(interp, iclsPtr, (char*)NULL, token,
            &mcode) != TCL_OK) {
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }

    Itcl_PreserveData((ClientData)mcode);
    Itcl_EventuallyFree((ClientData)mcode, Itcl_DeleteMemberCode);

    if (ivPtr->codePtr) {
        Itcl_ReleaseData((ClientData)ivPtr->codePtr);
    }
    ivPtr->codePtr = mcode;

configBodyCmdDone:
    Tcl_DStringFree(&buffer);
    return status;
}

/* ARGSUSED */
int
Itcl_ConfigBodyCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    return Itcl_NRCallObjProc(clientData, interp, NRConfigBodyCmd, objc, objv);
}



/*
 * ------------------------------------------------------------------------
 *  Itcl_CreateMethod()
 *
 *  Installs a method into the namespace associated with a class.
 *  If another command with the same name is already installed, then
 *  it is overwritten.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error message
 *  in the specified interp) if anything goes wrong.
 * ------------------------------------------------------------------------
 */
int
Itcl_CreateMethod(
    Tcl_Interp* interp,  /* interpreter managing this action */
    ItclClass *iclsPtr,  /* class definition */
    Tcl_Obj *namePtr,    /* name of new method */
    CONST char* arglist, /* space-separated list of arg names */
    CONST char* body)    /* body of commands for the method */
{
    ItclMemberFunc *imPtr;

    return ItclCreateMethod(interp, iclsPtr, namePtr, arglist, body, &imPtr);
}

/*
 * ------------------------------------------------------------------------
 *  ItclCreateMethod()
 *
 *  Installs a method into the namespace associated with a class.
 *  If another command with the same name is already installed, then
 *  it is overwritten.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error message
 *  in the specified interp) if anything goes wrong.
 * ------------------------------------------------------------------------
 */
int
ItclCreateMethod(
    Tcl_Interp* interp,  /* interpreter managing this action */
    ItclClass *iclsPtr,  /* class definition */
    Tcl_Obj *namePtr,    /* name of new method */
    CONST char* arglist, /* space-separated list of arg names */
    CONST char* body,    /* body of commands for the method */
    ItclMemberFunc **imPtrPtr)
{
    ItclMemberFunc *imPtr;

    /*
     *  Make sure that the method name does not contain anything
     *  goofy like a "::" scope qualifier.
     */
    if (strstr(Tcl_GetString(namePtr),"::")) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "bad method name \"", Tcl_GetString(namePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Create the method definition.
     */
    if (Itcl_CreateMemberFunc(interp, iclsPtr, namePtr, arglist, body, &imPtr)
        != TCL_OK) {
        return TCL_ERROR;
    }

    Itcl_PreserveData((ClientData)imPtr);
    if (imPtrPtr != NULL) {
        *imPtrPtr = imPtr;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_CreateProc()
 *
 *  Installs a class proc into the namespace associated with a class.
 *  If another command with the same name is already installed, then
 *  it is overwritten.  Returns TCL_OK on success, or TCL_ERROR  (along
 *  with an error message in the specified interp) if anything goes
 *  wrong.
 * ------------------------------------------------------------------------
 */
int
Itcl_CreateProc(
    Tcl_Interp* interp,  /* interpreter managing this action */
    ItclClass *iclsPtr,  /* class definition */
    Tcl_Obj* namePtr,    /* name of new proc */
    const char *arglist, /* space-separated list of arg names */
    const char *body)    /* body of commands for the proc */
{
    ItclMemberFunc *imPtr;

    /*
     *  Make sure that the proc name does not contain anything
     *  goofy like a "::" scope qualifier.
     */
    if (strstr(Tcl_GetString(namePtr),"::")) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "bad proc name \"", Tcl_GetString(namePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Create the proc definition.
     */
    if (Itcl_CreateMemberFunc(interp, iclsPtr, namePtr, arglist,
            body, &imPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Mark procs as "common".  This distinguishes them from methods.
     */
    imPtr->flags |= ITCL_COMMON;

    Itcl_PreserveData((ClientData)imPtr);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_CreateMemberFunc()
 *
 *  Creates the data record representing a member function.  This
 *  includes the argument list and the body of the function.  If the
 *  body is of the form "@name", then it is treated as a label for
 *  a C procedure registered by Itcl_RegisterC().
 *
 *  If any errors are encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK, and "imPtr" returns a pointer to the new
 *  member function.
 * ------------------------------------------------------------------------
 */
int
Itcl_CreateMemberFunc(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclClass *iclsPtr,            /* class definition */
    Tcl_Obj *namePtr,              /* name of new member */
    CONST char* arglist,           /* space-separated list of arg names */
    CONST char* body,              /* body of commands for the method */
    ItclMemberFunc** imPtrPtr)     /* returns: pointer to new method defn */
{
    int newEntry;
    char *name;
    ItclMemberFunc *imPtr;
    ItclMemberCode *mcode;
    Tcl_HashEntry *entry;

    /*
     *  Add the member function to the list of functions for
     *  the class.  Make sure that a member function with the
     *  same name doesn't already exist.
     */
    entry = Tcl_CreateHashEntry(&iclsPtr->functions, (char *)namePtr, &newEntry);

    if (!newEntry) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "\"", Tcl_GetString(namePtr), "\" already defined in class \"",
            Tcl_GetString(iclsPtr->fullNamePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    /*
     *  Try to create the implementation for this command member.
     */
    if (Itcl_CreateMemberCode(interp, iclsPtr, arglist, body,
        &mcode) != TCL_OK) {

        Tcl_DeleteHashEntry(entry);
        return TCL_ERROR;
    }

    Itcl_PreserveData((ClientData)mcode);
    Itcl_EventuallyFree((ClientData)mcode, Itcl_DeleteMemberCode);

    /*
     *  Allocate a member function definition and return.
     */
    imPtr = (ItclMemberFunc*)ckalloc(sizeof(ItclMemberFunc));
    memset(imPtr, 0, sizeof(ItclMemberFunc));
    imPtr->iclsPtr    = iclsPtr;
    imPtr->protection = Itcl_Protection(interp, 0);
    imPtr->namePtr    = Tcl_NewStringObj(Tcl_GetString(namePtr), -1);
    Tcl_IncrRefCount(imPtr->namePtr);
    imPtr->fullNamePtr = Tcl_NewStringObj(
            Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_AppendToObj(imPtr->fullNamePtr, "::", 2);
    Tcl_AppendToObj(imPtr->fullNamePtr, Tcl_GetString(namePtr), -1);
    Tcl_IncrRefCount(imPtr->fullNamePtr);
    if (arglist != NULL) {
        imPtr->origArgsPtr = Tcl_NewStringObj(arglist, -1);
        Tcl_IncrRefCount(imPtr->origArgsPtr);
    }
    imPtr->codePtr    = mcode;

    if (imPtr->protection == ITCL_DEFAULT_PROTECT) {
        imPtr->protection = ITCL_PUBLIC;
    }

    imPtr->declaringClassPtr = iclsPtr;

    if (arglist) {
        imPtr->flags |= ITCL_ARG_SPEC;
    }
    if (mcode->argListPtr) {
        ItclCreateArgList(interp, arglist, &imPtr->argcount,
	        &imPtr->maxargcount, &imPtr->usagePtr,
		&imPtr->argListPtr, imPtr, NULL);
    }

    name = Tcl_GetString(namePtr);
    if ((body != NULL) && (body[0] == '@')) {
        /* check for builtin cget isa and configure and mark them for
	 * use of a different arglist "args" for TclOO !! */
	if (strcmp(name, "cget") == 0) {
            imPtr->codePtr->flags |= ITCL_BUILTIN;
	}
	if (strcmp(name, "configure") == 0) {
	    imPtr->argcount = 0;
	    imPtr->maxargcount = -1;
            imPtr->codePtr->flags |= ITCL_BUILTIN;
	}
	if (strcmp(name, "isa") == 0) {
            imPtr->codePtr->flags |= ITCL_BUILTIN;
	}
	if (strcmp(name, "info") == 0) {
            imPtr->codePtr->flags |= ITCL_BUILTIN;
	}
    }
    if (strcmp(name, "___constructor_init") == 0) {
        imPtr->flags |= ITCL_CONINIT;
        iclsPtr->constructorInit = imPtr;
    }
    if (strcmp(name, "constructor") == 0) {
        imPtr->flags |= ITCL_CONSTRUCTOR;
        iclsPtr->constructor = imPtr;
    }
    if (strcmp(name, "destructor") == 0) {
        imPtr->flags |= ITCL_DESTRUCTOR;
        iclsPtr->destructor = imPtr;
    }

    Tcl_SetHashValue(entry, (ClientData)imPtr);
    Itcl_PreserveData((ClientData)imPtr);
    Itcl_EventuallyFree((ClientData)imPtr, Itcl_DeleteMemberFunc);

    *imPtrPtr = imPtr;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ChangeMemberFunc()
 *
 *  Modifies the data record representing a member function.  This
 *  is usually the body of the function, but can include the argument
 *  list if it was not defined when the member was first created.
 *  If the body is of the form "@name", then it is treated as a label
 *  for a C procedure registered by Itcl_RegisterC().
 *
 *  If any errors are encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK, and "imPtr" returns a pointer to the new
 *  member function.
 * ------------------------------------------------------------------------
 */
int
Itcl_ChangeMemberFunc(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclMemberFunc* imPtr,         /* command member being changed */
    CONST char* arglist,           /* space-separated list of arg names */
    CONST char* body)              /* body of commands for the method */
{
    Tcl_HashEntry *hPtr;
    ItclMemberCode *mcode = NULL;
    int isNewEntry;

    /*
     *  Try to create the implementation for this command member.
     */
    if (Itcl_CreateMemberCode(interp, imPtr->iclsPtr,
        arglist, body, &mcode) != TCL_OK) {

        return TCL_ERROR;
    }

    /*
     *  If the argument list was defined when the function was
     *  created, compare the arg lists or usage strings to make sure
     *  that the interface is not being redefined.
     */
    if ((imPtr->flags & ITCL_ARG_SPEC) != 0 &&
            (imPtr->argListPtr != NULL) &&
            !EquivArgLists(interp, imPtr->argListPtr, mcode->argListPtr)) {
	const char *argsStr;
	if (imPtr->origArgsPtr != NULL) {
	    argsStr = Tcl_GetString(imPtr->origArgsPtr);
	} else {
	    argsStr = "";
	}
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "argument list changed for function \"",
            Tcl_GetString(imPtr->fullNamePtr), "\": should be \"",
            argsStr, "\"",
            (char*)NULL);

        Itcl_DeleteMemberCode((char*)mcode);
        return TCL_ERROR;
    }

    /*
     *  Free up the old implementation and install the new one.
     */
    Itcl_PreserveData((ClientData)mcode);
    Itcl_EventuallyFree((ClientData)mcode, Itcl_DeleteMemberCode);

    Itcl_ReleaseData((ClientData)imPtr->codePtr);
    imPtr->codePtr = mcode;
    if (mcode->flags & ITCL_IMPLEMENT_TCL) {
	ClientData pmPtr;
        imPtr->tmPtr = (ClientData)Itcl_NewProcClassMethod(interp,
	    imPtr->iclsPtr->clsPtr, ItclCheckCallMethod, ItclAfterCallMethod,
	    ItclProcErrorProc, imPtr, imPtr->namePtr, mcode->argumentPtr,
	    mcode->bodyPtr, &pmPtr);
        hPtr = Tcl_CreateHashEntry(&imPtr->iclsPtr->infoPtr->procMethods,
                (char *)imPtr->tmPtr, &isNewEntry);
        if (isNewEntry) {
            Tcl_SetHashValue(hPtr, imPtr);
        }
    }

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_CreateMemberCode()
 *
 *  Creates the data record representing the implementation behind a
 *  class member function.  This includes the argument list and the body
 *  of the function.  If the body is of the form "@name", then it is
 *  treated as a label for a C procedure registered by Itcl_RegisterC().
 *
 *  The implementation is kept by the member function definition, and
 *  controlled by a preserve/release paradigm.  That way, if it is in
 *  use while it is being redefined, it will stay around long enough
 *  to avoid a core dump.
 *
 *  If any errors are encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK, and "mcodePtr" returns a pointer to the new
 *  implementation.
 * ------------------------------------------------------------------------
 */
int
Itcl_CreateMemberCode(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclClass *iclsPtr,              /* class containing this member */
    CONST char* arglist,           /* space-separated list of arg names */
    CONST char* body,              /* body of commands for the method */
    ItclMemberCode** mcodePtr)     /* returns: pointer to new implementation */
{
    int argc;
    int maxArgc;
    Tcl_Obj *usagePtr;
    ItclArgList *argListPtr;
    ItclMemberCode *mcode;

    /*
     *  Allocate some space to hold the implementation.
     */
    mcode = (ItclMemberCode*)ckalloc(sizeof(ItclMemberCode));
    memset(mcode, 0, sizeof(ItclMemberCode));

    if (arglist) {
        if (ItclCreateArgList(interp, arglist, &argc, &maxArgc, &usagePtr,
	        &argListPtr, NULL, NULL) != TCL_OK) {
            Itcl_DeleteMemberCode((char*)mcode);
            return TCL_ERROR;
        }
        mcode->argcount = argc;
        mcode->maxargcount = maxArgc;
        mcode->argListPtr = argListPtr;
        mcode->usagePtr = usagePtr;
	mcode->argumentPtr = Tcl_NewStringObj((CONST84 char *)arglist, -1);
	Tcl_IncrRefCount(mcode->argumentPtr);
        mcode->flags   |= ITCL_ARG_SPEC;
    } else {
        argc = 0;
        argListPtr = NULL;
    }

    if (body) {
        mcode->bodyPtr = Tcl_NewStringObj((CONST84 char *)body, -1);
    } else {
        mcode->bodyPtr = Tcl_NewStringObj((CONST84 char *)"", -1);
        mcode->flags |= ITCL_IMPLEMENT_NONE;
    }
    Tcl_IncrRefCount(mcode->bodyPtr);

    /*
     *  If the body definition starts with '@', then treat the value
     *  as a symbolic name for a C procedure.
     */
    if (body == NULL) {
        /* No-op */
    } else {
        if (*body == '@') {
            Tcl_CmdProc *argCmdProc;
            Tcl_ObjCmdProc *objCmdProc;
            ClientData cdata;
	    int isDone;

	    isDone = 0;
	    if (strcmp(body, "@itcl-builtin-cget") == 0) {
	        isDone = 1;
	    }
	    if (strcmp(body, "@itcl-builtin-configure") == 0) {
	        isDone = 1;
	    }
	    if (strcmp(body, "@itcl-builtin-info") == 0) {
	        isDone = 1;
	    }
	    if (strcmp(body, "@itcl-builtin-isa") == 0) {
	        isDone = 1;
	    }
	    if (strcmp(body, "@itcl-builtin-hullinstall") == 0) {
	        isDone = 1;
	    }
	    if (strncmp(body, "@itcl-builtin-setget", 20) == 0) {
	        isDone = 1;
	    }
	    if (!isDone) {
                if (!Itcl_FindC(interp, body+1, &argCmdProc, &objCmdProc,
		        &cdata)) {
                    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                            "no registered C procedure with name \"",
			    body+1, "\"", (char*)NULL);
                    Itcl_DeleteMemberCode((char*)mcode);
                    return TCL_ERROR;
                }

                if (objCmdProc != NULL) {
                    mcode->flags |= ITCL_IMPLEMENT_OBJCMD;
                    mcode->cfunc.objCmd = objCmdProc;
                    mcode->clientData = cdata;
                } else {
	            if (argCmdProc != NULL) {
                        mcode->flags |= ITCL_IMPLEMENT_ARGCMD;
                        mcode->cfunc.argCmd = argCmdProc;
                        mcode->clientData = cdata;
                    }
                }
	    } else {
                mcode->flags |= ITCL_IMPLEMENT_TCL|ITCL_BUILTIN;
	    }
        } else {

            /*
             *  Otherwise, treat the body as a chunk of Tcl code.
             */
            mcode->flags |= ITCL_IMPLEMENT_TCL;
	}
    }

    *mcodePtr = mcode;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_DeleteMemberCode()
 *
 *  Destroys all data associated with the given command implementation.
 *  Invoked automatically by Itcl_ReleaseData() when the implementation
 *  is no longer being used.
 * ------------------------------------------------------------------------
 */
void
Itcl_DeleteMemberCode(
    char* cdata)  /* pointer to member function definition */
{
    ItclMemberCode* mCodePtr = (ItclMemberCode*)cdata;

    if (mCodePtr == NULL) {
        return;
    }
    if (mCodePtr->argListPtr != NULL) {
        DeleteArgList(mCodePtr->argListPtr);
    }
    if (mCodePtr->usagePtr != NULL) {
        Tcl_DecrRefCount(mCodePtr->usagePtr);
    }
    if (mCodePtr->argumentPtr != NULL) {
        Tcl_DecrRefCount(mCodePtr->argumentPtr);
    }
    /* do NOT free mCodePtr->bodyPtr here !! that is done in TclOO!! */
    ckfree((char*)mCodePtr);
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_GetMemberCode()
 *
 *  Makes sure that the implementation for an [incr Tcl] code body is
 *  ready to run.  Note that a member function can be declared without
 *  being defined.  The class definition may contain a declaration of
 *  the member function, but its body may be defined in a separate file.
 *  If an undefined function is encountered, this routine automatically
 *  attempts to autoload it.  If the body is implemented via Tcl code,
 *  then it is compiled here as well.
 *
 *  Returns TCL_ERROR (along with an error message in the interpreter)
 *  if an error is encountered, or if the implementation is not defined
 *  and cannot be autoloaded.  Returns TCL_OK if implementation is
 *  ready to use.
 * ------------------------------------------------------------------------
 */
int
Itcl_GetMemberCode(
    Tcl_Interp* interp,        /* interpreter managing this action */
    ItclMemberFunc* imPtr)     /* member containing code body */
{
    int result;
    ItclMemberCode *mcode = imPtr->codePtr;
    assert(mcode != NULL);

    /*
     *  If the implementation has not yet been defined, try to
     *  autoload it now.
     */

    if (!Itcl_IsMemberCodeImplemented(mcode)) {
        result = Tcl_VarEval(interp, "::auto_load ",
	        Tcl_GetString(imPtr->fullNamePtr), (char*)NULL);

        if (result != TCL_OK) {
            char msg[256];
            sprintf(msg, "\n    (while autoloading code for \"%.100s\")",
                Tcl_GetString(imPtr->fullNamePtr));
            Tcl_AddErrorInfo(interp, msg);
            return result;
        }
        Tcl_ResetResult(interp);  /* get rid of 1/0 status */
    }

    /*
     *  If the implementation is still not available, then
     *  autoloading must have failed.
     *
     *  TRICKY NOTE:  If code has been autoloaded, then the
     *    old mcode pointer is probably invalid.  Go back to
     *    the member and look at the current code pointer again.
     */
    mcode = imPtr->codePtr;
    assert(mcode != NULL);

    if (!Itcl_IsMemberCodeImplemented(mcode)) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "member function \"", Tcl_GetString(imPtr->fullNamePtr),
            "\" is not defined and cannot be autoloaded",
            (char*)NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}



static int
CallItclObjectCmd(
    ClientData data[],
    Tcl_Interp *interp,
    int result)
{
    ItclMemberFunc *imPtr = data[0];
    Tcl_Object oPtr = data[1];
    int objc = PTR2INT(data[2]);
    Tcl_Obj **objv = data[3];

    if (oPtr != NULL) {
        return ItclObjectCmd(imPtr, interp, oPtr, imPtr->iclsPtr->clsPtr,
                objc, objv);
    } else {
        return ItclObjectCmd(imPtr, interp, NULL, NULL, objc, objv);
    }
}
/*
 * ------------------------------------------------------------------------
 *  Itcl_EvalMemberCode()
 *
 *  Used to execute an ItclMemberCode representation of a code
 *  fragment.  This code may be a body of Tcl commands, or a C handler
 *  procedure.
 *
 *  Executes the command with the given arguments (objc,objv) and
 *  returns an integer status code (TCL_OK/TCL_ERROR).  Returns the
 *  result string or an error message in the interpreter.
 * ------------------------------------------------------------------------
 */
static int
CallConstructBase(
    ClientData data[],
    Tcl_Interp *interp,
    int result)
{
    ItclMemberFunc *imPtr = data[0];
    ItclObject *contextIoPtr = data[1];
    int objc = PTR2INT(data[2]);
    Tcl_Obj *const* objv = data[3];

    return Itcl_ConstructBase(interp, contextIoPtr, imPtr->iclsPtr,
	        objc, objv);
}
int
Itcl_EvalMemberCode(
    Tcl_Interp *interp,       /* current interpreter */
    ItclMemberFunc *imPtr,    /* member func, or NULL (for error messages) */
    ItclObject *contextIoPtr,   /* object context, or NULL */
    int objc,                 /* number of arguments */
    Tcl_Obj *CONST objv[])    /* argument objects */
{
    ItclMemberCode *mcode;
    void *callbackPtr;
    int result = TCL_OK;
    int i;

    ItclShowArgs(1, "Itcl_EvalMemberCode", objc, objv);
    /*
     *  If this code does not have an implementation yet, then
     *  try to autoload one.  Also, if this is Tcl code, make sure
     *  that it's compiled and ready to use.
     */
    if (Itcl_GetMemberCode(interp, imPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    mcode = imPtr->codePtr;

    /*
     *  Bump the reference count on this code, in case it is
     *  redefined or deleted during execution.
     */
    Itcl_PreserveData((ClientData)mcode);

    /*
     *  If this code is a constructor, and if it is being invoked
     *  when an object is first constructed (i.e., the "constructed"
     *  table is still active within the object), then handle the
     *  "initCode" associated with the constructor and make sure that
     *  all base classes are properly constructed.
     *
     *  TRICKY NOTE:
     *    The "initCode" must be executed here.  This is the only
     *    opportunity where the arguments of the constructor are
     *    available in a call frame.
     */
    if ((imPtr->flags & ITCL_CONSTRUCTOR) && (contextIoPtr != NULL) &&
        contextIoPtr->constructed) {

        callbackPtr = Itcl_GetCurrentCallbackPtr(interp);
        Itcl_NRAddCallback(interp, CallConstructBase, imPtr, contextIoPtr,
	        INT2PTR(objc), objv);
        result = Itcl_NRRunCallbacks(interp, callbackPtr);
        if (result != TCL_OK) {
            goto evalMemberCodeDone;
        }
    }

    /*
     *  Execute the code body...
     */
    if (((mcode->flags & ITCL_IMPLEMENT_OBJCMD) != 0) ||
            ((mcode->flags & ITCL_IMPLEMENT_ARGCMD) != 0)) {
	Tcl_Namespace *callerNsPtr;
	callerNsPtr = Tcl_GetCurrentNamespace(interp);
	Itcl_SetCallFrameNamespace(interp, imPtr->iclsPtr->nsPtr);

        if ((mcode->flags & ITCL_IMPLEMENT_OBJCMD) != 0) {
            result = (*mcode->cfunc.objCmd)(mcode->clientData,
                    interp, objc, objv);
        } else {
            if ((mcode->flags & ITCL_IMPLEMENT_ARGCMD) != 0) {
                char **argv;
                argv = (char**)ckalloc( (unsigned)(objc*sizeof(char*)) );
                for (i=0; i < objc; i++) {
                    argv[i] = Tcl_GetStringFromObj(objv[i], (int*)NULL);
                }
        
                result = (*mcode->cfunc.argCmd)(mcode->clientData,
                    interp, objc, (CONST84 char **)argv);
        
                ckfree((char*)argv);
	    }
        }
    } else {
        if ((mcode->flags & ITCL_IMPLEMENT_TCL) != 0) {
            callbackPtr = Itcl_GetCurrentCallbackPtr(interp);
	    if (imPtr->flags & (ITCL_CONSTRUCTOR|ITCL_DESTRUCTOR)) {
                Itcl_NRAddCallback(interp, CallItclObjectCmd, imPtr,
                        contextIoPtr->oPtr, INT2PTR(objc), (void *)objv);
	    } else {
                Itcl_NRAddCallback(interp, CallItclObjectCmd, imPtr,
                        NULL, INT2PTR(objc), (void *)objv);
            }
            result = Itcl_NRRunCallbacks(interp, callbackPtr);
         }
    }

evalMemberCodeDone:
    return result;
}
/*
 * ------------------------------------------------------------------------
 *  DeleteArgList()
 * ------------------------------------------------------------------------
 */

static void
DeleteArgList(
    ItclArgList *arglistPtr)	/* first argument in arg list chain */
{
    ItclArgList *currPtr;
    ItclArgList *nextPtr;

/* FIX ME !!! */
return;
    currPtr=arglistPtr;
    while (currPtr != NULL) {
	if (currPtr->defaultValuePtr != NULL) {
	    Tcl_DecrRefCount(currPtr->defaultValuePtr);
	}
	Tcl_DecrRefCount(currPtr->namePtr);
        nextPtr = currPtr->nextPtr;
        ckfree((char *)currPtr);
        currPtr=nextPtr;
    }
    return;
}

/*
 * ------------------------------------------------------------------------
 *  ItclEquivArgLists()
 *
 *  Compares two argument lists to see if they are equivalent.  The
 *  first list is treated as a prototype, and the second list must
 *  match it.  Argument names may be different, but they must match in
 *  meaning.  If one argument is optional, the corresponding argument
 *  must also be optional.  If the prototype list ends with the magic
 *  "args" argument, then it matches everything in the other list.
 *
 *  Returns non-zero if the argument lists are equivalent.
 * ------------------------------------------------------------------------
 */

static int
EquivArgLists(
    Tcl_Interp *interp,
    ItclArgList *origArgs,
    ItclArgList *realArgs)
{
    ItclArgList *currPtr;
    char *argName;

    for (currPtr=origArgs; currPtr != NULL; currPtr=currPtr->nextPtr) {
	if ((realArgs != NULL) && (realArgs->namePtr == NULL)) {
            if (currPtr->namePtr != NULL) {
		if (strcmp(Tcl_GetString(currPtr->namePtr), "args") != 0) {
		    /* the definition has more arguments */
	            return 0;
	        }
            }
	}
	if (realArgs == NULL) {
	    if (currPtr->defaultValuePtr != NULL) {
	       /* default args must be there ! */
	       return 0;
	    }
            if (currPtr->namePtr != NULL) {
		if (strcmp(Tcl_GetString(currPtr->namePtr), "args") != 0) {
		    /* the definition has more arguments */
	            return 0;
	        }
	    }
	    return 1;
	}
	if (currPtr->namePtr == NULL) {
	    /* no args defined */
            if (realArgs->namePtr != NULL) {
	        return 0;
	    }
	    return 1;
	}
	argName = Tcl_GetString(currPtr->namePtr);
	if (strcmp(argName, "args") == 0) {
	    if (currPtr->nextPtr == NULL) {
	        /* this is the last arument */
	        return 1;
	    }
	}
	if (currPtr->defaultValuePtr != NULL) {
	    if (realArgs->defaultValuePtr != NULL) {
	        /* default values must be the same */
		if (strcmp(Tcl_GetString(currPtr->defaultValuePtr),
		        Tcl_GetString(realArgs->defaultValuePtr)) != 0) {
		    return 0;
	        }
	    }
	}
        realArgs = realArgs->nextPtr;
    }
    if ((currPtr == NULL) && (realArgs != NULL)) {
       /* new definition has more args then the old one */
       return 0;
    }
    return 1;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_GetContext()
 *
 *  Convenience routine for looking up the current object/class context.
 *  Useful in implementing methods/procs to see what class, and perhaps
 *  what object, is active.
 *
 *  Returns TCL_OK if the current namespace is a class namespace.
 *  Also returns pointers to the class definition, and to object
 *  data if an object context is active.  Returns TCL_ERROR (along
 *  with an error message in the interpreter) if a class namespace
 *  is not active.
 * ------------------------------------------------------------------------
 */
int
Itcl_GetContext(
    Tcl_Interp *interp,           /* current interpreter */
    ItclClass **iclsPtrPtr,       /* returns:  class definition or NULL */
    ItclObject **ioPtrPtr)        /* returns:  object data or NULL */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);
    Tcl_HashEntry *hPtr;
    ItclCallContext *callContextPtr;
    ItclObjectInfo *infoPtr;

    /*
     *  Return NULL for anything that cannot be found.
     */
    *ioPtrPtr = NULL;

    if (!Itcl_IsClassNamespace(activeNs)) {
        /*
         *  If there is no class/object context, return an error message.
         */
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "namespace \"", activeNs->fullName, "\" is not a class namespace",
            (char*)NULL);

        return TCL_ERROR;
    }
    /*
     *  If the active namespace is a class namespace, then return
     *  all known info.  See if the current call frame is a known
     *  object context, and if so, return that context.
     */
    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
            ITCL_INTERP_DATA, NULL);
    callContextPtr = Itcl_PeekStack(&infoPtr->contextStack);
    if ((callContextPtr != NULL) && (callContextPtr->imPtr != NULL)) {
        *iclsPtrPtr = callContextPtr->imPtr->iclsPtr;
    } else {
        hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses,
                (char *)activeNs);
        if (hPtr != NULL) {
            *iclsPtrPtr = (ItclClass *)Tcl_GetHashValue(hPtr);
        }
    }
    if (*iclsPtrPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "namespace \"", activeNs->fullName, "\" is not a class namespace",
            (char*)NULL);

        return TCL_ERROR;
    }

    if (callContextPtr == NULL) {
	/* must be a class namespace without an object */
	*ioPtrPtr = NULL;
	return TCL_OK;
    }
    *ioPtrPtr = callContextPtr->ioPtr;
    if ((*ioPtrPtr == NULL) && ((*iclsPtrPtr)->nsPtr != NULL)) {
        /* maybe we are in a constructor try currIoPtr */
        *ioPtrPtr = (*iclsPtrPtr)->infoPtr->currIoPtr;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_GetMemberFuncUsage()
 *
 *  Returns a string showing how a command member should be invoked.
 *  If the command member is a method, then the specified object name
 *  is reported as part of the invocation path:
 *
 *      obj method arg ?arg arg ...?
 *
 *  Otherwise, the "obj" pointer is ignored, and the class name is
 *  used as the invocation path:
 *
 *      class::proc arg ?arg arg ...?
 *
 *  Returns the string by appending it onto the Tcl_Obj passed in as
 *  an argument.
 * ------------------------------------------------------------------------
 */
void
Itcl_GetMemberFuncUsage(
    ItclMemberFunc *imPtr,      /* command member being examined */
    ItclObject *contextIoPtr,   /* invoked with respect to this object */
    Tcl_Obj *objPtr)            /* returns: string showing usage */
{
    int argcount;
    char *name;
    char *arglist;
    Tcl_HashEntry *entry;
    ItclMemberFunc *mf;
    ItclClass *iclsPtr;

    /*
     *  If the command is a method and an object context was
     *  specified, then add the object context.  If the method
     *  was a constructor, and if the object is being created,
     *  then report the invocation via the class creation command.
     */
    if ((imPtr->flags & ITCL_COMMON) == 0) {
        if ((imPtr->flags & ITCL_CONSTRUCTOR) != 0 &&
            contextIoPtr->constructed) {

            iclsPtr = (ItclClass*)contextIoPtr->iclsPtr;
            mf = NULL;
            entry = Tcl_FindHashEntry(&iclsPtr->resolveCmds, "constructor");
            if (entry) {
                mf = (ItclMemberFunc*)Tcl_GetHashValue(entry);
            }

            if (mf == imPtr) {
                Tcl_GetCommandFullName(contextIoPtr->iclsPtr->interp,
                    contextIoPtr->iclsPtr->accessCmd, objPtr);
                Tcl_AppendToObj(objPtr, " ", -1);
                name = (char *) Tcl_GetCommandName(
		    contextIoPtr->iclsPtr->interp, contextIoPtr->accessCmd);
                Tcl_AppendToObj(objPtr, name, -1);
            } else {
                Tcl_AppendToObj(objPtr, Tcl_GetString(imPtr->fullNamePtr), -1);
            }
        } else {
	    if (contextIoPtr && contextIoPtr->accessCmd) {
                name = (char *) Tcl_GetCommandName(
		    contextIoPtr->iclsPtr->interp, contextIoPtr->accessCmd);
                Tcl_AppendStringsToObj(objPtr, name, " ",
		        Tcl_GetString(imPtr->namePtr), (char*)NULL);
            } else {
                Tcl_AppendStringsToObj(objPtr, "<object> ",
		        Tcl_GetString(imPtr->namePtr), (char*)NULL);
	    }
        }
    } else {
        Tcl_AppendToObj(objPtr, Tcl_GetString(imPtr->fullNamePtr), -1);
    }

    /*
     *  Add the argument usage info.
     */
    if (imPtr->codePtr) {
	if (imPtr->codePtr->usagePtr != NULL) {
            arglist = Tcl_GetString(imPtr->codePtr->usagePtr);
	} else {
	    arglist = NULL;
	}
        argcount = imPtr->argcount;
    } else {
        if (imPtr->argListPtr != NULL) {
            arglist = Tcl_GetString(imPtr->usagePtr);
            argcount = imPtr->argcount;
        } else {
            arglist = NULL;
            argcount = 0;
        }
    }
    if (arglist) {
	if (strlen(arglist) > 0) {
            Tcl_AppendToObj(objPtr, " ", -1);
            Tcl_AppendToObj(objPtr, arglist, -1);
        }
    }
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ExecMethod()
 *
 *  Invoked by Tcl to handle the execution of a user-defined method.
 *  A method is similar to the usual Tcl proc, but has access to
 *  object-specific data.  If for some reason there is no current
 *  object context, then a method call is inappropriate, and an error
 *  is returned.
 *
 *  Methods are implemented either as Tcl code fragments, or as C-coded
 *  procedures.  For Tcl code fragments, command arguments are parsed
 *  according to the argument list, and the body is executed in the
 *  scope of the class where it was defined.  For C procedures, the
 *  arguments are passed in "as-is", and the procedure is executed in
 *  the most-specific class scope.
 * ------------------------------------------------------------------------
 */
static int
NRExecMethod(
    ClientData clientData,   /* method definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *const *objv)    /* argument objects */
{
    ItclMemberFunc *imPtr = (ItclMemberFunc*)clientData;
    int result = TCL_OK;

    char *token;
    Tcl_HashEntry *entry;
    ItclClass *iclsPtr;
    ItclObject *ioPtr;

    ItclShowArgs(1, "Itcl_ExecMethod", objc, objv);

    /*
     *  Make sure that the current namespace context includes an
     *  object that is being manipulated.  Methods can be executed
     *  only if an object context exists.
     */
    iclsPtr = imPtr->iclsPtr;
    if (Itcl_GetContext(interp, &iclsPtr, &ioPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    if (ioPtr == NULL) {
	if (strcmp(Tcl_GetString(imPtr->namePtr), "info") != 0) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "cannot access object-specific info without an object context",
                (char*)NULL);
            return TCL_ERROR;
        }
    }

    /*
     *  Make sure that this command member can be accessed from
     *  the current namespace context.
     *  That is now done in ItclMapMethodNameProc !!
     */

    /*
     *  All methods should be "virtual" unless they are invoked with
     *  a "::" scope qualifier.
     *
     *  To implement the "virtual" behavior, find the most-specific
     *  implementation for the method by looking in the "resolveCmds"
     *  table for this class.
     */
    token = Tcl_GetString(objv[0]);
    if (strstr(token, "::") == NULL) {
	if (ioPtr != NULL) {
            entry = Tcl_FindHashEntry(&ioPtr->iclsPtr->resolveCmds,
                Tcl_GetString(imPtr->namePtr));

            if (entry) {
                imPtr = (ItclMemberFunc*)Tcl_GetHashValue(entry);
            }
        }
    }

    /*
     *  Execute the code for the method.  Be careful to protect
     *  the method in case it gets deleted during execution.
     */
    Itcl_PreserveData((ClientData)imPtr);
    /* next line is VERY UGLY HACK !! to make test xxx run */
    imPtr->flags |= ITCL_CALLED_FROM_EXEC; 

    result = Itcl_EvalMemberCode(interp, imPtr, ioPtr, objc, objv);
    Itcl_ReleaseData((ClientData)imPtr);
    return result;
}

/* ARGSUSED */
int
Itcl_ExecMethod(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    return Itcl_NRCallObjProc(clientData, interp, NRExecMethod, objc, objv);
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ExecProc()
 *
 *  Invoked by Tcl to handle the execution of a user-defined proc.
 *
 *  Procs are implemented either as Tcl code fragments, or as C-coded
 *  procedures.  For Tcl code fragments, command arguments are parsed
 *  according to the argument list, and the body is executed in the
 *  scope of the class where it was defined.  For C procedures, the
 *  arguments are passed in "as-is", and the procedure is executed in
 *  the most-specific class scope.
 * ------------------------------------------------------------------------
 */
static int
NRExecProc(
    ClientData clientData,   /* proc definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclMemberFunc *imPtr = (ItclMemberFunc*)clientData;
    int result = TCL_OK;

    ItclShowArgs(1, "Itcl_ExecProc", objc, objv);

    /*
     *  Make sure that this command member can be accessed from
     *  the current namespace context.
     */
    if (imPtr->protection != ITCL_PUBLIC) {
        if (!Itcl_CanAccessFunc(imPtr, Tcl_GetCurrentNamespace(interp))) {
	    ItclMemberFunc *imPtr2 = NULL;
            Tcl_HashEntry *hPtr;
	    Tcl_ObjectContext context;
	    context = Itcl_GetCallFrameClientData(interp);
            if (context == NULL) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                        "can't access \"", Tcl_GetString(imPtr->fullNamePtr),
			"\": ", Itcl_ProtectionStr(imPtr->protection),
			" function", (char*)NULL);
                return TCL_ERROR;
            }
	    hPtr = Tcl_FindHashEntry(&imPtr->iclsPtr->infoPtr->procMethods,
	            (char *)Tcl_ObjectContextMethod(context));
	    if (hPtr != NULL) {
	        imPtr2 = Tcl_GetHashValue(hPtr);
	    }
	    if ((imPtr->protection & ITCL_PRIVATE) && (imPtr2 != NULL) &&
	            (imPtr->iclsPtr->nsPtr != imPtr2->iclsPtr->nsPtr)) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
	                "invalid command name \"",
		        Tcl_GetString(objv[0]),
		        "\"", NULL);
	        return TCL_ERROR;
	    }
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "can't access \"", Tcl_GetString(imPtr->fullNamePtr),
		    "\": ", Itcl_ProtectionStr(imPtr->protection),
		    " function", (char*)NULL);
            return TCL_ERROR;
        }
    }

    /*
     *  Execute the code for the proc.  Be careful to protect
     *  the proc in case it gets deleted during execution.
     */
    Itcl_PreserveData((ClientData)imPtr);

    result = Itcl_EvalMemberCode(interp, imPtr, (ItclObject*)NULL,
        objc, objv);
    Itcl_ReleaseData((ClientData)imPtr);
    return result;
}

/* ARGSUSED */
int
Itcl_ExecProc(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    return Itcl_NRCallObjProc(clientData, interp, NRExecProc, objc, objv);
}

static int
CallInvokeMethodIfExists(
    ClientData data[],
    Tcl_Interp *interp,
    int result)
{
    ItclClass *iclsPtr = data[0];
    ItclObject *contextObj = data[1];
    int objc = PTR2INT(data[2]);
    Tcl_Obj* const* objv = data[3];

    result = Itcl_InvokeMethodIfExists(interp, "constructor",
            iclsPtr, contextObj, objc, (Tcl_Obj* CONST*)objv);

    if (result != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
CallPublicObjectCmd(
    ClientData data[],
    Tcl_Interp *interp,
    int result)
{
    ItclClass *contextClass = data[0];
    int cmdlinec = PTR2INT(data[1]);
    Tcl_Obj **cmdlinev = data[2];

    result = Itcl_PublicObjectCmd(contextClass->infoPtr->currIoPtr->oPtr,
            interp, contextClass->clsPtr, cmdlinec, cmdlinev);
    if (result != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}
/*
 * ------------------------------------------------------------------------
 *  Itcl_ConstructBase()
 *
 *  Usually invoked just before executing the body of a constructor
 *  when an object is first created.  This procedure makes sure that
 *  all base classes are properly constructed.  If an "initCode" fragment
 *  was defined with the constructor for the class, then it is invoked.
 *  After that, the list of base classes is checked for constructors
 *  that are defined but have not yet been invoked.  Each of these is
 *  invoked implicitly with no arguments.
 *
 *  Assumes that a local call frame is already installed, and that
 *  constructor arguments have already been matched and are sitting in
 *  this frame.  Returns TCL_OK on success; otherwise, this procedure
 *  returns TCL_ERROR, along with an error message in the interpreter.
 * ------------------------------------------------------------------------
 */
int
Itcl_ConstructBase(
    Tcl_Interp *interp,       /* interpreter */
    ItclObject *contextObj,   /* object being constructed */
    ItclClass *contextClass,  /* current class being constructed */
    int objc,
    Tcl_Obj *const *objv)
{
    int result;
    Itcl_ListElem *elem;
    ItclClass *iclsPtr;
    Tcl_HashEntry *entry;
    Tcl_Obj *cmdlinePtr;
    Tcl_Obj **cmdlinev;
    void *callbackPtr;
    int cmdlinec;

    ItclShowArgs(1, "Itcl_ConstructBase", objc, objv);
    /*
     *  If the class has an "initCode", invoke it in the current context.
     *
     *  TRICKY NOTE:
     *    This context is the call frame containing the arguments
     *    for the constructor.  The "initCode" makes sense right
     *    now--just before the body of the constructor is executed.
     */
    Itcl_PushStack(contextClass, &contextClass->infoPtr->constructorStack);
    if (contextClass->initCode) {
        /*
         *  Prepend the method name to the list of arguments.
         */
	int incr = 0;
	if (strcmp(Tcl_GetString(objv[0]), "my") == 0) {
	     /* number of args to skip depends on if we are called from
	      * another constructor or directly */
	     incr = 1;
	}
        cmdlinePtr = Itcl_CreateArgs(interp, "___constructor_init",
	        objc-1-incr, objv+1+incr);

        (void) Tcl_ListObjGetElements((Tcl_Interp*)NULL, cmdlinePtr,
            &cmdlinec, &cmdlinev);
        callbackPtr = Itcl_GetCurrentCallbackPtr(interp);
        Itcl_NRAddCallback(interp, CallPublicObjectCmd, contextClass,
	        INT2PTR(cmdlinec), cmdlinev, NULL);
        result = Itcl_NRRunCallbacks(interp, callbackPtr);
        if (result != TCL_OK) {
	    return result;
	}
    }

    /*
     *  Scan through the list of base classes and see if any of these
     *  have not been constructed.  Invoke base class constructors
     *  implicitly, as needed.  Go through the list of base classes
     *  in reverse order, so that least-specific classes are constructed
     *  first.
     */
    elem = Itcl_LastListElem(&contextClass->bases);
    while (elem != NULL) {
        iclsPtr = (ItclClass*)Itcl_GetListValue(elem);

        if (Tcl_FindHashEntry(contextObj->constructed,
	        (char *)iclsPtr->namePtr) == NULL) {

            callbackPtr = Itcl_GetCurrentCallbackPtr(interp);
            Itcl_NRAddCallback(interp, CallInvokeMethodIfExists, iclsPtr,
	            contextObj, INT2PTR(0), NULL);
            result = Itcl_NRRunCallbacks(interp, callbackPtr);
            if (result != TCL_OK) {
                return TCL_ERROR;
            }
            /*
             *  The base class may not have a constructor, but its
             *  own base classes could have one.  If the constructor
             *  wasn't found in the last step, then other base classes
             *  weren't constructed either.  Make sure that all of its
             *  base classes are properly constructed.
             */
	    Tcl_Obj *objPtr;
	    objPtr = Tcl_NewStringObj("constructor", -1);
	    Tcl_IncrRefCount(objPtr);
            entry = Tcl_FindHashEntry(&iclsPtr->functions, (char *)objPtr);
	    Tcl_DecrRefCount(objPtr);
            if (entry == NULL) {
                result = Itcl_ConstructBase(interp, contextObj, iclsPtr,
		        objc, objv);
                if (result != TCL_OK) {
                    return TCL_ERROR;
                }
            }
        }
        elem = Itcl_PrevListElem(elem);
    }
    Itcl_PopStack(&contextClass->infoPtr->constructorStack);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_InvokeMethodIfExists()
 *
 *  Looks for a particular method in the specified class.  If the
 *  method is found, it is invoked with the given arguments.  Any
 *  protection level (protected/private) for the method is ignored.
 *  If the method does not exist, this procedure does nothing.
 *
 *  This procedure is used primarily to invoke the constructor/destructor
 *  when an object is created/destroyed.
 *
 *  Returns TCL_OK on success; otherwise, this procedure returns
 *  TCL_ERROR along with an error message in the interpreter.
 * ------------------------------------------------------------------------
 */
int
Itcl_InvokeMethodIfExists(
    Tcl_Interp *interp,       /* interpreter */
    CONST char *name,         /* name of desired method */
    ItclClass *contextClass,  /* current class being constructed */
    ItclObject *contextObj,   /* object being constructed */
    int objc,                 /* number of arguments */
    Tcl_Obj *CONST objv[])    /* argument objects */
{
    int result = TCL_OK;

    ItclMemberFunc *imPtr;
    Tcl_HashEntry *entry;
    Tcl_Obj *cmdlinePtr;
    int cmdlinec;
    Tcl_Obj **cmdlinev;

    ItclShowArgs(1, "Itcl_InvokeMethodIfExists", objc, objv);
    Tcl_Obj *objPtr = Tcl_NewStringObj(name, -1);
    entry = Tcl_FindHashEntry(&contextClass->functions, (char *)objPtr);

    if (entry) {
        imPtr  = (ItclMemberFunc*)Tcl_GetHashValue(entry);

        /*
         *  Prepend the method name to the list of arguments.
         */
        cmdlinePtr = Itcl_CreateArgs(interp, name, objc, objv);

        (void) Tcl_ListObjGetElements((Tcl_Interp*)NULL, cmdlinePtr,
            &cmdlinec, &cmdlinev);

        /*
         *  Execute the code for the method.  Be careful to protect
         *  the method in case it gets deleted during execution.
         */
        Itcl_PreserveData((ClientData)imPtr);

	if (contextObj->oPtr == NULL) {
            return TCL_ERROR;
	}
        result = Itcl_EvalMemberCode(interp, imPtr, contextObj,
	        cmdlinec, cmdlinev);
        Itcl_ReleaseData((ClientData)imPtr);
        Tcl_DecrRefCount(cmdlinePtr);
    }
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ReportFuncErrors()
 *
 *  Used to interpret the status code returned when the body of a
 *  Tcl-style proc is executed.  Handles the "errorInfo" and "errorCode"
 *  variables properly, and adds error information into the interpreter
 *  if anything went wrong.  Returns a new status code that should be
 *  treated as the return status code for the command.
 *
 *  This same operation is usually buried in the Tcl InterpProc()
 *  procedure.  It is defined here so that it can be reused more easily.
 * ------------------------------------------------------------------------
 */
int
Itcl_ReportFuncErrors(
    Tcl_Interp* interp,        /* interpreter being modified */
    ItclMemberFunc *imPtr,     /* command member that was invoked */
    ItclObject *contextObj,    /* object context for this command */
    int result)                /* integer status code from proc body */
{
/* FIX ME !!! */
/* adapt to use of ItclProcErrorProc for stubs compatibility !! */
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_CmdAliasProc()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Command
Itcl_CmdAliasProc(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    CONST char *cmdName,
    ClientData clientData)
{
    Tcl_HashEntry *hPtr;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclObject *ioPtr;
    ItclMemberFunc *imPtr;
    ItclResolveInfo *resolveInfoPtr;

    resolveInfoPtr = (ItclResolveInfo *)clientData;
    if (resolveInfoPtr->flags & ITCL_RESOLVE_OBJECT) {
        ioPtr = resolveInfoPtr->ioPtr;
        iclsPtr = ioPtr->iclsPtr;
    } else {
        ioPtr = NULL;
        iclsPtr = resolveInfoPtr->iclsPtr;
    }
    infoPtr = iclsPtr->infoPtr;
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)nsPtr);
    if (hPtr == NULL) {
	return NULL;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveCmds, cmdName);
    if (hPtr == NULL) {
	if (strcmp(cmdName, "info") == 0) {
	    return Tcl_FindCommand(interp, "::itcl::builtin::Info", NULL, 0);
	}
	if (strcmp(cmdName, "@itcl-builtin-info") == 0) {
	    return Tcl_FindCommand(interp, "::itcl::builtin::Info", NULL, 0);
	}
	if (strcmp(cmdName, "@itcl-builtin-cget") == 0) {
	    return Tcl_FindCommand(interp, "::itcl::builtin::cget", NULL, 0);
	}
	if (strcmp(cmdName, "@itcl-builtin-configure") == 0) {
	    return Tcl_FindCommand(interp, "::itcl::builtin::configure", NULL, 0);
	}
	if (strncmp(cmdName, "@itcl-builtin-setget", 20) == 0) {
	    return Tcl_FindCommand(interp, "::itcl::builtin::setget", NULL, 0);
	}
	if (strcmp(cmdName, "@itcl-builtin-isa") == 0) {
	    return Tcl_FindCommand(interp, "::itcl::builtin::isa", NULL, 0);
	}
	if (*cmdName == '@') {
	    return Tcl_FindCommand(interp,
	            ITCL_NAMESPACE"::methodset::callCCommand", NULL, 0);
	}
        return NULL;
    }
    imPtr = Tcl_GetHashValue(hPtr);
	if (strcmp(cmdName, "info") == 0) {
	    return Tcl_FindCommand(interp, "::itcl::builtin::Info", NULL, 0);
	}
    return imPtr->accessCmd;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_VarAliasProc()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Var
Itcl_VarAliasProc(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    CONST char *varName,
    ClientData clientData)
{

    Tcl_HashEntry *hPtr;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclObject *ioPtr;
    ItclVarLookup *ivlPtr;
    ItclResolveInfo *resolveInfoPtr;
    ItclCallContext *callContextPtr;
    Tcl_Var varPtr;

    varPtr = NULL;
    hPtr = NULL;
    callContextPtr = NULL;
    resolveInfoPtr = (ItclResolveInfo *)clientData;
    if (resolveInfoPtr->flags & ITCL_RESOLVE_OBJECT) {
        ioPtr = resolveInfoPtr->ioPtr;
        iclsPtr = ioPtr->iclsPtr;
    } else {
        ioPtr = NULL;
        iclsPtr = resolveInfoPtr->iclsPtr;
    }
    infoPtr = iclsPtr->infoPtr;
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)nsPtr);
    if (hPtr != NULL) {
        iclsPtr = Tcl_GetHashValue(hPtr);
    }
    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveVars, varName);
    if (hPtr == NULL) {
	/* no class/object variable */
        return NULL;
    }
    ivlPtr = Tcl_GetHashValue(hPtr);
    if (ivlPtr == NULL) {
        return NULL;
    }
    if (!ivlPtr->accessible) {
        return NULL;
    }

    if (ioPtr != NULL) {
        hPtr = Tcl_FindHashEntry(&ioPtr->objectVariables,
	        (char *)ivlPtr->ivPtr);
    } else {
        hPtr = Tcl_FindHashEntry(&iclsPtr->classCommons,
	        (char *)ivlPtr->ivPtr);
        if (hPtr == NULL) {
	    if (callContextPtr != NULL) {
	        ioPtr = callContextPtr->ioPtr;
	    }
	    if (ioPtr != NULL) {
                hPtr = Tcl_FindHashEntry(&ioPtr->objectVariables,
	                (char *)ivlPtr->ivPtr);
	    }
	}
    }
    if (hPtr != NULL) {
        varPtr = Tcl_GetHashValue(hPtr);
    }
    return varPtr;
}

/*
 * ------------------------------------------------------------------------
 *  ItclCheckCallProc()
 *
 *
 * ------------------------------------------------------------------------
 */
int
ItclCheckCallProc(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_ObjectContext contextPtr,
    Tcl_CallFrame *framePtr,
    int *isFinished)
{
    int result;
    ItclMemberFunc *imPtr;

    imPtr = (ItclMemberFunc *)clientData;
    if (!imPtr->iclsPtr->infoPtr->useOldResolvers) {
        Itcl_SetCallFrameResolver(interp, imPtr->iclsPtr->resolvePtr);
    }
    result = TCL_OK;

    if (isFinished != NULL) {
        *isFinished = 0;
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclCheckCallMethod()
 *
 *
 * ------------------------------------------------------------------------
 */
int
ItclCheckCallMethod(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_ObjectContext contextPtr,
    Tcl_CallFrame *framePtr,
    int *isFinished)
{

    Tcl_Object oPtr;
    ItclObject *ioPtr;
    Tcl_HashEntry *hPtr;
    ItclCallContext *callContextPtr;
    ItclCallContext *callContextPtr2;
    ItclMemberFunc *imPtr;
    int result;
    int isNew;

    oPtr = NULL;
    hPtr = NULL;
    imPtr = (ItclMemberFunc *)clientData;
    if (imPtr->flags & ITCL_CONSTRUCTOR) {
        ioPtr = imPtr->iclsPtr->infoPtr->currIoPtr;
    } else {
	if (contextPtr == NULL) {
	    if ((imPtr->flags & ITCL_COMMON) ||
                    (imPtr->codePtr->flags & ITCL_BUILTIN)) {
                if (!imPtr->iclsPtr->infoPtr->useOldResolvers) {
                    Itcl_SetCallFrameResolver(interp,
                            imPtr->iclsPtr->resolvePtr);
                }
                if (isFinished != NULL) {
                    *isFinished = 0;
                }
                return TCL_OK;
            }
	    Tcl_AppendResult(interp,
	            "ItclCheckCallMethod cannot get context object (NULL)",
                    " for ", Tcl_GetString(imPtr->fullNamePtr),
		    NULL);
	    return TCL_ERROR;
	}
        oPtr = Tcl_ObjectContextObject(contextPtr);
	ioPtr = Tcl_ObjectGetMetadata(oPtr,
	        imPtr->iclsPtr->infoPtr->object_meta_type);
    }
    if ((imPtr->codePtr != NULL) &&
            (imPtr->codePtr->flags & ITCL_IMPLEMENT_NONE)) {
        Tcl_AppendResult(interp, "member function \"",
	        Tcl_GetString(imPtr->fullNamePtr),
		"\" is not defined and cannot be autoloaded", NULL);
        if (isFinished != NULL) {
            *isFinished = 1;
        }
        return TCL_ERROR;
    }
    int cObjc = Itcl_GetCallFrameObjc(interp);
    Tcl_Obj *const * cObjv = Itcl_GetCallFrameObjv(interp);
    if (cObjc-2 < imPtr->argcount) {
	if (strcmp(Tcl_GetString(imPtr->namePtr), "info") == 0) {
            Tcl_Obj *objPtr = Tcl_NewStringObj(
	            "wrong # args: should be one of...\n", -1);
            ItclGetInfoUsage(interp, objPtr);
	    Tcl_SetResult(interp, Tcl_GetString(objPtr), TCL_DYNAMIC);
	} else {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
	            Tcl_GetString(cObjv[0]), " ",
	            Tcl_GetString(imPtr->namePtr), " ",
		    Tcl_GetString(imPtr->usagePtr), "\"", NULL);
	}
        if (isFinished != NULL) {
            *isFinished = 1;
        }
        return TCL_ERROR;
    }
    isNew = 0;
    callContextPtr = NULL;
    Tcl_Namespace *currNsPtr;
    currNsPtr = Tcl_GetCurrentNamespace(interp);
    if (ioPtr != NULL) {
        hPtr = Tcl_CreateHashEntry(&ioPtr->contextCache, (char *)imPtr, &isNew);
        if (!isNew) {
	    callContextPtr2 = Tcl_GetHashValue(hPtr);
	    if (callContextPtr2->refCount == 0) {
	        callContextPtr = callContextPtr2;
                callContextPtr->objectFlags = ioPtr->flags;
                callContextPtr->nsPtr = Tcl_GetCurrentNamespace(interp);
                callContextPtr->ioPtr = ioPtr;
                callContextPtr->imPtr = imPtr;
                callContextPtr->refCount = 1;
	    } else {
	      if ((callContextPtr2->objectFlags == ioPtr->flags) 
		    && (callContextPtr2->nsPtr == currNsPtr)) {
	        callContextPtr = callContextPtr2;
                callContextPtr->refCount++;
              }
            }
        }
    }
    if (callContextPtr == NULL) {
	if (ioPtr == NULL) {
	    Tcl_AppendResult(interp, "ItclCheckCallMethod  ioPtr == NULL", NULL);
            if (isFinished != NULL) {
                *isFinished = 1;
            }
	    return TCL_ERROR;
	}
        callContextPtr = (ItclCallContext *)ckalloc(
                sizeof(ItclCallContext));
        callContextPtr->objectFlags = ioPtr->flags;
        callContextPtr->nsPtr = Tcl_GetCurrentNamespace(interp);
        callContextPtr->ioPtr = ioPtr;
        callContextPtr->imPtr = imPtr;
        callContextPtr->refCount = 1;
    }
    if (isNew) {
        Tcl_SetHashValue(hPtr, callContextPtr);
    }
    Itcl_PushStack(callContextPtr, &imPtr->iclsPtr->infoPtr->contextStack);

    ioPtr->callRefCount++;
    imPtr->iclsPtr->callRefCount++;
//    ioPtr->flags |= ITCL_OBJECT_NO_VARNS_DELETE;
    if (!imPtr->iclsPtr->infoPtr->useOldResolvers) {
        Itcl_SetCallFrameResolver(interp, ioPtr->resolvePtr);
    }
    result = TCL_OK;

    if (isFinished != NULL) {
        *isFinished = 0;
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclAfterCallMethod()
 *
 *
 * ------------------------------------------------------------------------
 */
int
ItclAfterCallMethod(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_ObjectContext contextPtr,
    Tcl_Namespace *nsPtr,
    int call_result)
{
    Tcl_Object oPtr;
    Tcl_HashEntry *hPtr;
    ItclObject *ioPtr;
    ItclMemberFunc *imPtr;
    ItclCallContext *callContextPtr;
    int newEntry;

    oPtr = NULL;
    imPtr = (ItclMemberFunc *)clientData;

    callContextPtr = NULL;
    if (contextPtr != NULL) {
        callContextPtr = Itcl_PopStack(&imPtr->iclsPtr->infoPtr->contextStack);
    }
    if (callContextPtr == NULL) {
        if ((imPtr->flags & ITCL_COMMON) ||
                (imPtr->codePtr->flags & ITCL_BUILTIN)) {
            return call_result;
        }
	Tcl_AppendResult(interp,
	        "ItclAfterCallMethod cannot get context object (NULL)",
                " for ", Tcl_GetString(imPtr->fullNamePtr), NULL);
        return TCL_ERROR;
    }
    /*
     *  If this is a constructor or destructor, and if it is being
     *  invoked at the appropriate time, keep track of which methods
     *  have been called.  This information is used to implicitly
     *  invoke constructors/destructors as needed.
     */
    ioPtr = callContextPtr->ioPtr;
    if (imPtr->flags & (ITCL_CONSTRUCTOR | ITCL_DESTRUCTOR)) {
        if ((imPtr->flags & ITCL_DESTRUCTOR) && ioPtr &&
             ioPtr->destructed) {
            Tcl_CreateHashEntry(ioPtr->destructed,
                (char *)imPtr->iclsPtr->namePtr, &newEntry);
        }
        if ((imPtr->flags & ITCL_CONSTRUCTOR) && ioPtr &&
             ioPtr->constructed) {
            Tcl_CreateHashEntry(ioPtr->constructed,
                (char *)imPtr->iclsPtr->namePtr, &newEntry);
        }
    }
    ioPtr->callRefCount--;
    imPtr->iclsPtr->callRefCount--;
    if (ioPtr->flags & ITCL_OBJECT_SHOULD_VARNS_DELETE) {
        ItclDeleteObjectVariablesNamespace(interp, ioPtr);
    }
    
    callContextPtr->refCount--;
    if (callContextPtr->refCount == 0) {
        if (callContextPtr->ioPtr != NULL) {
	    hPtr = Tcl_FindHashEntry(&callContextPtr->ioPtr->contextCache,
	            (char *)callContextPtr->imPtr);
            if (hPtr == NULL) {
                ckfree((char *)callContextPtr);
	    }
        } else {
            ckfree((char *)callContextPtr);
        }
    }
    return call_result;
}

void
ItclProcErrorProc(
    Tcl_Interp *interp,
    Tcl_Obj *procNameObj)
{
    ItclObjectInfo *infoPtr;
    ItclCallContext *callContextPtr;
    ItclMemberFunc *imPtr;
    ItclObject *contextIoPtr;
    ItclClass *currIclsPtr;
    Tcl_Obj *objPtr;
    Tcl_Namespace *upNsPtr;
    char num[20];
    int constructorStackIndex;
    int constructorStackSize;
    int isFirstLoop;
    int loopCnt;

    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
            ITCL_INTERP_DATA, NULL);
    callContextPtr = Itcl_PeekStack(&infoPtr->contextStack);
    loopCnt = 1;
    isFirstLoop = 1;
    upNsPtr = Itcl_GetUplevelNamespace(interp, 1);
    constructorStackIndex = -1;
    while ((callContextPtr != NULL) && (loopCnt > 0)) {
	imPtr = callContextPtr->imPtr;
        contextIoPtr = callContextPtr->ioPtr;
        objPtr = Tcl_NewStringObj("\n    ", -1);
        Tcl_IncrRefCount(objPtr);

        if (imPtr->flags & ITCL_CONSTRUCTOR) {
	    /* have to look for classes in construction where the constructor
	     * has not yet been called, but only the initCode or the
	     * inherited constructors
	     */
            if (isFirstLoop) {
	        isFirstLoop = 0;
                constructorStackSize = Itcl_GetStackSize(
		        &imPtr->iclsPtr->infoPtr->constructorStack);
	        constructorStackIndex = constructorStackSize;
	        currIclsPtr = imPtr->iclsPtr;
	    } else {
	        currIclsPtr = (ItclClass *)Itcl_GetStackValue(
	                &imPtr->iclsPtr->infoPtr->constructorStack,
		        constructorStackIndex);
            }
	    if (constructorStackIndex < 0) {
	        break;
	    }
	    if (currIclsPtr == NULL) {
	        break;
	    }
	    if (upNsPtr == currIclsPtr->nsPtr) {
	        break;
	    }
	    constructorStackIndex--;
	    loopCnt++;
            Tcl_AppendToObj(objPtr, "while constructing object \"", -1);
            Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
            Tcl_AppendToObj(objPtr, "\" in ", -1);
            Tcl_AppendToObj(objPtr, currIclsPtr->nsPtr->fullName, -1);
            Tcl_AppendToObj(objPtr, "::constructor", -1);
            if ((imPtr->codePtr->flags & ITCL_IMPLEMENT_TCL) != 0) {
                Tcl_AppendToObj(objPtr, " (", -1);
            }
        }
        if (imPtr->flags & ITCL_CONINIT) {
            Tcl_AppendToObj(objPtr, "while constructing object \"", -1);
            Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
            Tcl_AppendToObj(objPtr, "\" in ", -1);
            Tcl_AppendToObj(objPtr,
	            Tcl_GetString(imPtr->iclsPtr->fullNamePtr), -1);
            Tcl_AppendToObj(objPtr, "::constructor", -1);
            if ((imPtr->codePtr->flags & ITCL_IMPLEMENT_TCL) != 0) {
                Tcl_AppendToObj(objPtr, " (", -1);
            }
        }
	if (imPtr->flags & ITCL_DESTRUCTOR) {
	    Tcl_AppendToObj(objPtr, "while deleting object \"", -1);
            Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
            Tcl_AppendToObj(objPtr, "\" in ", -1);
            Tcl_AppendToObj(objPtr, Tcl_GetString(imPtr->fullNamePtr), -1);
            if ((imPtr->codePtr->flags & ITCL_IMPLEMENT_TCL) != 0) {
                Tcl_AppendToObj(objPtr, " (", -1);
            }
        }
	if (!(imPtr->flags & (ITCL_CONSTRUCTOR|ITCL_DESTRUCTOR|ITCL_CONINIT))) {
            Tcl_AppendToObj(objPtr, "(", -1);

            if ((contextIoPtr != NULL) && (contextIoPtr->accessCmd)) {
                Tcl_AppendToObj(objPtr, "object \"", -1);
                Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
                Tcl_AppendToObj(objPtr, "\" ", -1);
            }

            if ((imPtr->flags & ITCL_COMMON) != 0) {
                Tcl_AppendToObj(objPtr, "procedure", -1);
            } else {
                Tcl_AppendToObj(objPtr, "method", -1);
            }
            Tcl_AppendToObj(objPtr, " \"", -1);
            Tcl_AppendToObj(objPtr, Tcl_GetString(imPtr->fullNamePtr), -1);
            Tcl_AppendToObj(objPtr, "\" ", -1);
        }

        if ((imPtr->codePtr->flags & ITCL_IMPLEMENT_TCL) != 0) {
            Tcl_Obj *dictPtr;
	    Tcl_Obj *keyPtr;
	    Tcl_Obj *valuePtr;
	    int lineNo;

	    keyPtr = Tcl_NewStringObj("-errorline", -1);
            dictPtr = Tcl_GetReturnOptions(interp, TCL_ERROR);
	    if (Tcl_DictObjGet(interp, dictPtr, keyPtr, &valuePtr) != TCL_OK) {
	        /* how should we handle an error ? */
		Tcl_DecrRefCount(keyPtr);
		return;
	    }
            if (valuePtr == NULL) {
	        /* how should we handle an error ? */
		Tcl_DecrRefCount(keyPtr);
		return;
	    }
            if (Tcl_GetIntFromObj(interp, valuePtr, &lineNo) != TCL_OK) {
	        /* how should we handle an error ? */
		Tcl_DecrRefCount(keyPtr);
	        Tcl_DecrRefCount(valuePtr);
		return;
	    }
	    Tcl_DecrRefCount(keyPtr);
	    Tcl_DecrRefCount(valuePtr);
            Tcl_AppendToObj(objPtr, "body line ", -1);
            sprintf(num, "%d", lineNo);
            Tcl_AppendToObj(objPtr, num, -1);
            Tcl_AppendToObj(objPtr, ")", -1);
        } else {
            Tcl_AppendToObj(objPtr, ")", -1);
        }

        Tcl_AddErrorInfo(interp, Tcl_GetString(objPtr));
        Tcl_DecrRefCount(objPtr);
        loopCnt--;
    }
}
