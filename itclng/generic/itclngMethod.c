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
 *     RCS:  $Id: itclngMethod.c,v 1.1.2.11 2008/02/10 18:40:40 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclngInt.h"

#ifdef NOTDEF
static int EquivArgLists(Tcl_Interp *interp, ItclngArgList *origArgs,
        ItclngArgList *realArgs);
#endif
static void ItclngDeleteFunction(ItclngMemberFunc *imPtr);


/*
 * ------------------------------------------------------------------------
 *  ItclngDeleteFunction()
 *
 *  fre data associated with a function
 * ------------------------------------------------------------------------
 */
static void
ItclngDeleteFunction(
    ItclngMemberFunc *imPtr)
{
    Tcl_DecrRefCount(imPtr->namePtr);
    Tcl_DecrRefCount(imPtr->fullNamePtr);
    if (imPtr->codePtr != NULL) {
       Tcl_Release((ClientData)imPtr->codePtr);
    }
    ckfree((char*)imPtr);
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_BodyCmd()
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
/* ARGSUSED */
int
Itclng_BodyCmd(
    ClientData dummy,        /* unused */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int status = TCL_OK;

    char *head;
    char *tail;
    char *token;
    char *arglist;
    char *body;
    ItclngClass *iclsPtr;
    ItclngMemberFunc *imPtr;
    Tcl_HashEntry *entry;
    Tcl_DString buffer;

    ItclngShowArgs(2, "Itclng_BodyCmd", objc, objv);
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
    Itclng_ParseNamespPath(token, &buffer, &head, &tail);

    if (!head || *head == '\0') {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "missing class specifier for body declaration \"", token, "\"",
            (char*)NULL);
        status = TCL_ERROR;
        goto bodyCmdDone;
    }

    iclsPtr = Itclng_FindClass(interp, head, /* autoload */ 1);
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
        imPtr = (ItclngMemberFunc*)Tcl_GetHashValue(entry);
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

    if (Itclng_ChangeMemberFunc(interp, imPtr, arglist, body) != TCL_OK) {
        status = TCL_ERROR;
        goto bodyCmdDone;
    }

bodyCmdDone:
    Tcl_DStringFree(&buffer);
    return status;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_ConfigBodyCmd()
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
int
Itclng_ConfigBodyCmd(
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
    ItclngClass *iclsPtr;
    ItclngVarLookup *vlookup;
    ItclngVariable *ivPtr;
    ItclngMemberCode *mcode;
    Tcl_HashEntry *entry;

    ItclngShowArgs(2, "Itclng_ConfigBodyCmd", objc, objv);
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
    Itclng_ParseNamespPath(token, &buffer, &head, &tail);

    if ((head == NULL) || (*head == '\0')) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "missing class specifier for body declaration \"", token, "\"",
            (char*)NULL);
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }

    iclsPtr = Itclng_FindClass(interp, head, /* autoload */ 1);
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
        vlookup = (ItclngVarLookup*)Tcl_GetHashValue(entry);
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

    if (ivPtr->protection != ITCLNG_PUBLIC) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "option \"", Tcl_GetString(ivPtr->fullNamePtr),
                "\" is not a public configuration option",
                (char*)NULL);
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }

    token = Tcl_GetString(objv[2]);

    if (ItclngCreateVariableMemberCode(interp, iclsPtr,
            Tcl_GetString(ivPtr->namePtr), token, &mcode) != TCL_OK) {
        status = TCL_ERROR;
        goto configBodyCmdDone;
    }

    Tcl_Preserve((ClientData)mcode);
    Tcl_EventuallyFree((ClientData)mcode, Itclng_DeleteMemberCode);

    if (ivPtr->codePtr) {
        Tcl_Release((ClientData)ivPtr->codePtr);
    }
    ivPtr->codePtr = mcode;

configBodyCmdDone:
    Tcl_DStringFree(&buffer);
    return status;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_DeleteMemberFunc()
 *
 * ------------------------------------------------------------------------
 */

void Itclng_DeleteMemberFunc (char *cdata) {
    /* needed for stubs compatibility */
    ItclngDeleteFunction((ItclngMemberFunc *)cdata);
}


/*
 * ------------------------------------------------------------------------
 *  ItclngCreateMemberCode()
 *
 *  Creates the data record representing the implementation behind a
 *  class member function.
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
ItclngCreateMemberCode(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclngClass *iclsPtr,          /* class containing this member */
    const char *name,              /* name of function */
    const char *state,             /* state string of function */
    ItclngMemberCode** mcodePtr)   /* returns: pointer to new implementation */
{
    Tcl_Obj *dictPtr;
    Tcl_Obj *valuePtr;
    Tcl_Obj *argPtr;
    ItclngMemberCode *mcode;

    /*
     *  Allocate some space to hold the implementation.
     */
    mcode = (ItclngMemberCode*)ckalloc(sizeof(ItclngMemberCode));
    memset(mcode, 0, sizeof(ItclngMemberCode));

    dictPtr = ItclngGetClassDictInfo(iclsPtr, "functions", name);
    if (dictPtr == NULL) {
	Tcl_AppendResult(interp, "cannot get functions info", NULL);
        return TCL_ERROR;
    }
    argPtr = ItclngGetDictValueInfo(interp, dictPtr, "arguments");
    Tcl_DecrRefCount(dictPtr);
    if (argPtr == NULL) {
	Tcl_AppendResult(interp, "cannot get arguments", NULL);
        return TCL_ERROR;
    }
    valuePtr = ItclngGetDictValueInfo(interp, argPtr, "minargs");
    if (valuePtr == NULL) {
	Tcl_AppendResult(interp, "cannot get minargs string", NULL);
        Tcl_DecrRefCount(argPtr);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, valuePtr, &mcode->argcount) != TCL_OK) {
        Tcl_DecrRefCount(argPtr);
	Tcl_DecrRefCount(valuePtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(valuePtr);
    valuePtr = ItclngGetDictValueInfo(interp, argPtr, "maxargs");
    if (valuePtr == NULL) {
	Tcl_AppendResult(interp, "cannot get maxargs string", NULL);
        Tcl_DecrRefCount(argPtr);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, valuePtr, &mcode->maxargcount) != TCL_OK) {
	Tcl_DecrRefCount(valuePtr);
        Tcl_DecrRefCount(argPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(argPtr);
    Tcl_DecrRefCount(valuePtr);
    if (strcmp(state, "NO_ARGS") != 0) {
        mcode->flags   |= ITCLNG_ARG_SPEC;
    }
    if (strcmp(state, "COMPLETE") == 0) {
        mcode->flags |= ITCLNG_IMPLEMENT_TCL;
    } else {
        mcode->flags |= ITCLNG_IMPLEMENT_NONE;
    }
    *mcodePtr = mcode;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngCreateMemberFunction()
 *
 *  Creates the data record representing a member function. 
 *
 *  If any errors are encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK, and "imPtr" returns a pointer to the new
 *  member function.
 * ------------------------------------------------------------------------
 */
int
ItclngCreateMemberFunction(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclngClass *iclsPtr,            /* class definition */
    Tcl_Obj *namePtr,              /* name of new member */
    ItclngMemberFunc** imPtrPtr)     /* returns: pointer to new method defn */
{
    Tcl_Obj *statePtr;
    Tcl_HashEntry *entry;
    ItclngMemberFunc *imPtr;
    ItclngMemberCode *mcode;
    char *name;
    const char *stateStr;
    int newEntry;

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

    name = Tcl_GetString(namePtr);
    statePtr = ItclngGetFunctionStateString(iclsPtr, name);
    if (statePtr == NULL) {
	Tcl_AppendResult(interp, "cannot get state string", NULL);
        return TCL_ERROR;
    }
    stateStr = Tcl_GetString(statePtr);
    /*
     *  Try to create the implementation for this command member.
     */
    if (ItclngCreateMemberCode(interp, iclsPtr, name, stateStr, &mcode) !=
            TCL_OK) {
        Tcl_DeleteHashEntry(entry);
        return TCL_ERROR;
    }

    Tcl_Preserve((ClientData)mcode);
    Tcl_EventuallyFree((ClientData)mcode, Itclng_DeleteMemberCode);

    /*
     *  Allocate a member function definition and return.
     */
    imPtr = (ItclngMemberFunc*)ckalloc(sizeof(ItclngMemberFunc));
    memset(imPtr, 0, sizeof(ItclngMemberFunc));
    imPtr->iclsPtr    = iclsPtr;
    imPtr->protection = ItclngGetProtection(iclsPtr, "functions", name);
    imPtr->namePtr    = Tcl_DuplicateObj(namePtr);
    Tcl_IncrRefCount(imPtr->namePtr);
    imPtr->fullNamePtr = Tcl_NewStringObj(
            Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_AppendToObj(imPtr->fullNamePtr, "::", 2);
    Tcl_AppendToObj(imPtr->fullNamePtr, Tcl_GetString(namePtr), -1);
    Tcl_IncrRefCount(imPtr->fullNamePtr);
    imPtr->codePtr    = mcode;
    imPtr->declaringClassPtr = iclsPtr;

    if (strcmp(stateStr, "NO_ARGS") == 0) {
        imPtr->flags |= ITCLNG_ARG_SPEC;
    }
    if (strcmp(name, "___constructor_init") == 0) {
        imPtr->flags |= ITCLNG_CONINIT;
        iclsPtr->constructorInit = imPtr;
    }
    if (strcmp(name, "constructor") == 0) {
        imPtr->flags |= ITCLNG_CONSTRUCTOR;
        iclsPtr->constructor = imPtr;
    }
    if (strcmp(name, "destructor") == 0) {
        imPtr->flags |= ITCLNG_DESTRUCTOR;
        iclsPtr->destructor = imPtr;
    }

    Tcl_SetHashValue(entry, (ClientData)imPtr);
    Tcl_Preserve((ClientData)imPtr);
    Tcl_EventuallyFree((ClientData)imPtr, Itclng_DeleteMemberFunc);

    *imPtrPtr = imPtr;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngCreateMethodOrProc()
 *
 *  Installs a method/proc into the namespace associated with a class.
 *  If another command with the same name is already installed, then
 *  it is overwritten.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error message
 *  in the specified interp) if anything goes wrong.
 * ------------------------------------------------------------------------
 */
int
ItclngCreateMethodOrProc(
    Tcl_Interp* interp,    /* interpreter managing this action */
    ItclngClass *iclsPtr,  /* class definition */
    Tcl_Obj *namePtr,      /* name of new method/proc */
    int flags,             /* whether this is a method or proc */
    ItclngMemberFunc **imPtrPtr)
{
    /*
     *  Create the member function definition.
     */
    if (ItclngCreateMemberFunction(interp, iclsPtr, namePtr, imPtrPtr)
        != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Mark procs as "common".  This distinguishes them from methods.
     */
    (*imPtrPtr)->flags |= flags;

    Tcl_Preserve((ClientData)(*imPtrPtr));
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngChangeMemberFunc()
 *
 *  Modifies the data record representing a member function.  This
 *  is usually the body of the function, but can include the argument
 *  list if it was not defined when the member was first created.
 *
 *  If any errors are encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK
 * ------------------------------------------------------------------------
 */
int
ItclngChangeMemberFunc(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclngClass *iclsPtr,          /* the class */
    Tcl_Obj *namePtr,              /* the function name */
    ItclngMemberFunc* imPtr)       /* command member being changed */
{
    Tcl_HashEntry *hPtr;
    Tcl_Obj *statePtr;
    ItclngMemberCode *mcode = NULL;
    int isNewEntry;
    const char *stateStr;
    const char *name;

    name = Tcl_GetString(namePtr);
    statePtr = ItclngGetFunctionStateString(iclsPtr, name);
    if (statePtr == NULL) {
	Tcl_AppendResult(interp, "cannot get state string", NULL);
        return TCL_ERROR;
    }
    stateStr = Tcl_GetString(statePtr);
    /*
     *  Try to create the implementation for this command member.
     */
    if (ItclngCreateMemberCode(interp, imPtr->iclsPtr,
        Tcl_GetString(imPtr->namePtr), stateStr, &mcode) != TCL_OK) {

        return TCL_ERROR;
    }

    /*
     *  Free up the old implementation and install the new one.
     */
    Tcl_Preserve((ClientData)mcode);
    Tcl_EventuallyFree((ClientData)mcode, Itclng_DeleteMemberCode);

    Tcl_Release((ClientData)imPtr->codePtr);
    imPtr->flags |= ITCLNG_BODY_SPEC;
    imPtr->codePtr = mcode;
    if (mcode->flags & ITCLNG_IMPLEMENT_TCL) {
	ClientData pmPtr;
	Tcl_Obj *argumentPtr;
	Tcl_Obj *bodyPtr;

	argumentPtr = ItclngGetArgumentInfo(imPtr->iclsPtr,
	        Tcl_GetString(imPtr->namePtr), "arguments", "definition");
	bodyPtr = ItclngGetBodyString(imPtr->iclsPtr,
	        Tcl_GetString(imPtr->namePtr));
        imPtr->tmPtr = (ClientData)Itclng_NewProcClassMethod(interp,
	    imPtr->iclsPtr->clsPtr, ItclngCheckCallMethod,
	    ItclngAfterCallMethod,
	    ItclngProcErrorProc, imPtr, imPtr->namePtr, argumentPtr,
	    bodyPtr, &pmPtr);
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
 *  ItclngChangeVariableConfig()
 *
 *  Modifies the data record representing a class variable.  This
 *  is usually the body of the function, but can include the argument
 *  list if it was not defined when the member was first created.
 *
 *  If any errors are encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK
 * ------------------------------------------------------------------------
 */
int
ItclngChangeVariableConfig(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclngClass *iclsPtr,          /* the class */
    Tcl_Obj *namePtr,              /* the variable name */
    const char *configPtr,         /* the variable name */
    ItclngVariable* ivPtr)         /* variable being changed */
{
    Tcl_Obj *statePtr;
    ItclngMemberCode *mcode = NULL;
    const char *stateStr;
    const char *name;

    name = Tcl_GetString(namePtr);
    statePtr = ItclngGetVariableInfoString(iclsPtr, name, "state");
    if (statePtr == NULL) {
	Tcl_AppendResult(interp, "cannot get state string", NULL);
        return TCL_ERROR;
    }
    stateStr = Tcl_GetString(statePtr);
    /*
     *  Try to create the implementation for this command member.
     */
    if (ItclngCreateVariableMemberCode(interp, iclsPtr,
            Tcl_GetString(ivPtr->namePtr), configPtr, &mcode) != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Free up the old implementation and install the new one.
     */
    Tcl_Preserve((ClientData)mcode);
    Tcl_EventuallyFree((ClientData)mcode, Itclng_DeleteMemberCode);

    if (ivPtr->codePtr != NULL) {
        Tcl_Release((ClientData)ivPtr->codePtr);
    }
    ivPtr->codePtr = mcode;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_ChangeMemberFunc()
 *
 *  Modifies the data record representing a member function.  This
 *  is usually the body of the function, but can include the argument
 *  list if it was not defined when the member was first created.
 *  If the body is of the form "@name", then it is treated as a label
 *  for a C procedure registered by Itclng_RegisterC().
 *
 *  If any errors are encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK, and "imPtr" returns a pointer to the new
 *  member function.
 * ------------------------------------------------------------------------
 */
int
Itclng_ChangeMemberFunc(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclngMemberFunc* imPtr,         /* command member being changed */
    CONST char* arglist,           /* space-separated list of arg names */
    CONST char* body)              /* body of commands for the method */
{
    Tcl_HashEntry *hPtr;
    ItclngMemberCode *mcode = NULL;
    int isNewEntry;

    /*
     *  Try to create the implementation for this command member.
     */
    if (ItclngCreateMemberCode(interp, imPtr->iclsPtr,
        Tcl_GetString(imPtr->namePtr), "COMPLETE", &mcode) != TCL_OK) {

        return TCL_ERROR;
    }

    /*
     *  If the argument list was defined when the function was
     *  created, compare the arg lists or usage strings to make sure
     *  that the interface is not being redefined.
     */
    if ((imPtr->flags & ITCLNG_ARG_SPEC) != 0
#ifdef NOTDEF
            && (imPtr->argListPtr != NULL) &&
            !EquivArgLists(interp, imPtr->argListPtr, mcode->argListPtr)
#endif
	    ) {
	const char *argsStr;
#ifdef NOTDEF
	if (imPtr->origArgsPtr != NULL) {
	    argsStr = Tcl_GetString(imPtr->origArgsPtr);
	} else {
#endif
	    argsStr = "";
#ifdef NOTDEF
	}
#endif
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "argument list changed for function \"",
            Tcl_GetString(imPtr->fullNamePtr), "\": should be \"",
            argsStr, "\"",
            (char*)NULL);

        Itclng_DeleteMemberCode((char*)mcode);
        return TCL_ERROR;
    }

    /*
     *  Free up the old implementation and install the new one.
     */
    Tcl_Preserve((ClientData)mcode);
    Tcl_EventuallyFree((ClientData)mcode, Itclng_DeleteMemberCode);

    Tcl_Release((ClientData)imPtr->codePtr);
    imPtr->codePtr = mcode;
    if (mcode->flags & ITCLNG_IMPLEMENT_TCL) {
	ClientData pmPtr;
	Tcl_Obj *argumentPtr;
	Tcl_Obj *bodyPtr;

	argumentPtr = ItclngGetArgumentInfo(imPtr->iclsPtr,
	        Tcl_GetString(imPtr->namePtr), "arguments", "definition");
	bodyPtr = ItclngGetBodyString(imPtr->iclsPtr,
	        Tcl_GetString(imPtr->namePtr));
        imPtr->tmPtr = (ClientData)Itclng_NewProcClassMethod(interp,
	    imPtr->iclsPtr->clsPtr, ItclngCheckCallMethod, ItclngAfterCallMethod,
	    ItclngProcErrorProc, imPtr, imPtr->namePtr, argumentPtr,
	    bodyPtr, &pmPtr);
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
 *  Itclng_DeleteMemberCode()
 *
 *  Destroys all data associated with the given command implementation.
 *  Invoked automatically by Tcl_Release() when the implementation
 *  is no longer being used.
 * ------------------------------------------------------------------------
 */
void
Itclng_DeleteMemberCode(
    char* cdata)  /* pointer to member function definition */
{
    ItclngMemberCode* mCodePtr = (ItclngMemberCode*)cdata;

    if (mCodePtr == NULL) {
        return;
    }
    ckfree((char*)mCodePtr);
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_GetMemberCode()
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
Itclng_GetMemberCode(
    Tcl_Interp* interp,        /* interpreter managing this action */
    ItclngMemberFunc* imPtr)     /* member containing code body */
{
    int result;
    ItclngMemberCode *mcode = imPtr->codePtr;
    assert(mcode != NULL);

    /*
     *  If the implementation has not yet been defined, try to
     *  autoload it now.
     */

    if (!Itclng_IsMemberCodeImplemented(mcode)) {
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

    if (!Itclng_IsMemberCodeImplemented(mcode)) {
fprintf(stderr, "CIMPL!%s!0x%08x\n", Tcl_GetString(imPtr->namePtr), mcode->flags);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "member function \"", Tcl_GetString(imPtr->fullNamePtr),
            "\" is not defined and cannot be autoloaded",
            (char*)NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}



/*
 * ------------------------------------------------------------------------
 *  Itclng_EvalMemberCode()
 *
 *  Used to execute an ItclngMemberCode representation of a code
 *  fragment.  This code may be a body of Tcl commands, or a C handler
 *  procedure.
 *
 *  Executes the command with the given arguments (objc,objv) and
 *  returns an integer status code (TCL_OK/TCL_ERROR).  Returns the
 *  result string or an error message in the interpreter.
 * ------------------------------------------------------------------------
 */
int
Itclng_EvalMemberCode(
    Tcl_Interp *interp,       /* current interpreter */
    ItclngMemberFunc *imPtr,    /* member func, or NULL (for error messages) */
    ItclngObject *contextIoPtr,   /* object context, or NULL */
    int objc,                 /* number of arguments */
    Tcl_Obj *CONST objv[])    /* argument objects */
{
    int result = TCL_OK;

    ItclngMemberCode *mcode;

    ItclngShowArgs(1, "Itclng_EvalMemberCode", objc, objv);
    /*
     *  If this code does not have an implementation yet, then
     *  try to autoload one.  Also, if this is Tcl code, make sure
     *  that it's compiled and ready to use.
     */
    if (Itclng_GetMemberCode(interp, imPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    mcode = imPtr->codePtr;

    /*
     *  Bump the reference count on this code, in case it is
     *  redefined or deleted during execution.
     */
    Tcl_Preserve((ClientData)mcode);

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
    if ((imPtr->flags & ITCLNG_CONSTRUCTOR) && (contextIoPtr != NULL) &&
        contextIoPtr->constructed) {

        result = Itclng_ConstructBase(interp, contextIoPtr, imPtr->iclsPtr,
	        objc, objv);

        if (result != TCL_OK) {
            goto evalMemberCodeDone;
        }
    }

    /*
     *  Execute the code body...
     */
    if ((mcode->flags & ITCLNG_IMPLEMENT_TCL) != 0) {
        if (imPtr->flags & (ITCLNG_CONSTRUCTOR|ITCLNG_DESTRUCTOR)) {
            result = ItclngObjectCmd(imPtr, interp, contextIoPtr->oPtr,
                   imPtr->iclsPtr->clsPtr, objc, objv);
        } else {
            result = ItclngObjectCmd(imPtr, interp, NULL, NULL,
                    objc, objv);
        }
     }

evalMemberCodeDone:
    return result;
}
#ifdef NOTDEF

/*
 * ------------------------------------------------------------------------
 *  ItclngEquivArgLists()
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
    ItclngArgList *origArgs,
    ItclngArgList *realArgs)
{
    ItclngArgList *currPtr;
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
#endif

/*
 * ------------------------------------------------------------------------
 *  Itclng_GetContext()
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
Itclng_GetContext(
    Tcl_Interp *interp,           /* current interpreter */
    ItclngClass **iclsPtrPtr,       /* returns:  class definition or NULL */
    ItclngObject **ioPtrPtr)        /* returns:  object data or NULL */
{
    Tcl_Namespace *activeNs = Tcl_GetCurrentNamespace(interp);
    Tcl_HashEntry *hPtr;
    ItclngCallContext *callContextPtr;
    ItclngObjectInfo *infoPtr;

    /*
     *  Return NULL for anything that cannot be found.
     */
    *ioPtrPtr = NULL;

    if (!Itclng_IsClassNamespace(activeNs)) {
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
    infoPtr = (ItclngObjectInfo *)Tcl_GetAssocData(interp,
            ITCLNG_INTERP_DATA, NULL);
    callContextPtr = Itclng_PeekStack(&infoPtr->contextStack);
    if ((callContextPtr != NULL) && (callContextPtr->imPtr != NULL)) {
        *iclsPtrPtr = callContextPtr->imPtr->iclsPtr;
    } else {
        hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses,
                (char *)activeNs);
        if (hPtr != NULL) {
            *iclsPtrPtr = (ItclngClass *)Tcl_GetHashValue(hPtr);
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
 *  Itclng_GetMemberFuncUsage()
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
Itclng_GetMemberFuncUsage(
    ItclngMemberFunc *imPtr,      /* command member being examined */
    ItclngObject *contextIoPtr,   /* invoked with respect to this object */
    Tcl_Obj *objPtr)            /* returns: string showing usage */
{
    Tcl_Obj *arglistPtr;
    Tcl_HashEntry *entry;
    ItclngMemberFunc *mf;
    ItclngClass *iclsPtr;
    char *name;
    char *arglist;
    int argcount;

    /*
     *  If the command is a method and an object context was
     *  specified, then add the object context.  If the method
     *  was a constructor, and if the object is being created,
     *  then report the invocation via the class creation command.
     */
    if ((imPtr->flags & ITCLNG_COMMON) == 0) {
        if ((imPtr->flags & ITCLNG_CONSTRUCTOR) != 0 &&
            contextIoPtr->constructed) {

            iclsPtr = (ItclngClass*)contextIoPtr->iclsPtr;
            mf = NULL;
            entry = Tcl_FindHashEntry(&iclsPtr->resolveCmds, "constructor");
            if (entry) {
                mf = (ItclngMemberFunc*)Tcl_GetHashValue(entry);
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
	arglistPtr = ItclngGetArgumentInfo(imPtr->iclsPtr,
	        Tcl_GetString(imPtr->namePtr), "origArguments", "usage");
	if (arglistPtr == NULL) {
fprintf(stderr, "INTERNAL ERROR: cannot get origArguments usage\n");
	    return;
	}
        arglist = Tcl_GetString(arglistPtr);
        argcount = imPtr->argcount;
    } else {
        arglistPtr = ItclngGetArgumentInfo(imPtr->iclsPtr,
                Tcl_GetString(imPtr->namePtr), "arguments", "usage");
        if (arglistPtr == NULL) {
fprintf(stderr, "INTERNAL ERROR: cannot get arguments usage\n");
            return;
        }
        arglist = Tcl_GetString(arglistPtr);
        argcount = imPtr->argcount;
    }
    if (imPtr->iclsPtr == imPtr->iclsPtr->infoPtr->rootClassIclsPtr) {
        name = Tcl_GetString(imPtr->namePtr);
        if (strcmp(name, "cget") == 0) {
	    arglist = "-option";
	}
        if (strcmp(name, "configure") == 0) {
	    arglist = "?-option? ?value -option value...?";
	}
        if (strcmp(name, "isa") == 0) {
	    arglist = "className";
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
 *  Itclng_ExecMethod()
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
int
Itclng_ExecMethod(
    ClientData clientData,   /* method definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngMemberFunc *imPtr = (ItclngMemberFunc*)clientData;
    int result = TCL_OK;

    char *token;
    Tcl_HashEntry *entry;
    ItclngClass *iclsPtr;
    ItclngObject *ioPtr;

    ItclngShowArgs(1, "Itclng_ExecMethod", objc, objv);

    /*
     *  Make sure that the current namespace context includes an
     *  object that is being manipulated.  Methods can be executed
     *  only if an object context exists.
     */
    iclsPtr = imPtr->iclsPtr;

    if (Itclng_GetContext(interp, &iclsPtr, &ioPtr) != TCL_OK) {
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
                imPtr = (ItclngMemberFunc*)Tcl_GetHashValue(entry);
            }
        }
    }

    /*
     *  Execute the code for the method.  Be careful to protect
     *  the method in case it gets deleted during execution.
     */
    Tcl_Preserve((ClientData)imPtr);
    result = Itclng_EvalMemberCode(interp, imPtr, ioPtr, objc, objv);
    Tcl_Release((ClientData)imPtr);
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_ExecProc()
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
int
Itclng_ExecProc(
    ClientData clientData,   /* proc definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngMemberFunc *imPtr = (ItclngMemberFunc*)clientData;
    int result = TCL_OK;

    ItclngShowArgs(1, "Itclng_ExecProc", objc, objv);

    /*
     *  Make sure that this command member can be accessed from
     *  the current namespace context.
     */
    if (imPtr->protection != ITCLNG_PUBLIC) {
        if (!Itclng_CanAccessFunc(imPtr, Tcl_GetCurrentNamespace(interp))) {
	    ItclngMemberFunc *imPtr2 = NULL;
            Tcl_HashEntry *hPtr;
	    Tcl_ObjectContext context;
	    context = Itclng_GetCallFrameClientData(interp);
            if (context == NULL) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                        "can't access \"", Tcl_GetString(imPtr->fullNamePtr),
			"\": ", Itclng_ProtectionStr(imPtr->protection),
			" function", (char*)NULL);
                return TCL_ERROR;
            }
	    hPtr = Tcl_FindHashEntry(&imPtr->iclsPtr->infoPtr->procMethods,
	            (char *)Tcl_ObjectContextMethod(context));
	    if (hPtr != NULL) {
	        imPtr2 = Tcl_GetHashValue(hPtr);
	    }
	    if ((imPtr->protection & ITCLNG_PRIVATE) && (imPtr2 != NULL) &&
	            (imPtr->iclsPtr->nsPtr != imPtr2->iclsPtr->nsPtr)) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
		        "invalid command name \"",
		        Tcl_GetString(objv[0]),
		        "\"", NULL);
		return TCL_ERROR;
	    }
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "can't access \"", Tcl_GetString(imPtr->fullNamePtr),
		    "\": ", Itclng_ProtectionStr(imPtr->protection),
		    " function", (char*)NULL);
            return TCL_ERROR;
	}
    }

    /*
     *  Execute the code for the proc.  Be careful to protect
     *  the proc in case it gets deleted during execution.
     */
    Tcl_Preserve((ClientData)imPtr);

    result = Itclng_EvalMemberCode(interp, imPtr, (ItclngObject*)NULL,
        objc, objv);
    Tcl_Release((ClientData)imPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_ConstructBase()
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
Itclng_ConstructBase(
    Tcl_Interp *interp,       /* interpreter */
    ItclngObject *contextObj,   /* object being constructed */
    ItclngClass *contextClass,  /* current class being constructed */
    int objc,
    Tcl_Obj *const *objv)
{
    int result;
    Itclng_ListElem *elem;
    ItclngClass *iclsPtr;
    Tcl_HashEntry *entry;
    Tcl_Obj *cmdlinePtr;
    int cmdlinec;
    Tcl_Obj **cmdlinev;

ItclngShowArgs(0, "Itclng_ConstructBase", objc, objv);
    /*
     *  If the class has an "initCode", invoke it in the current context.
     *
     *  TRICKY NOTE:
     *    This context is the call frame containing the arguments
     *    for the constructor.  The "initCode" makes sense right
     *    now--just before the body of the constructor is executed.
     */
    Itclng_PushStack(contextClass, &contextClass->infoPtr->constructorStack);
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
        cmdlinePtr = Itclng_CreateArgs(interp, "___constructor_init",
	        objc-1-incr, objv+1+incr);

        (void) Tcl_ListObjGetElements((Tcl_Interp*)NULL, cmdlinePtr,
            &cmdlinec, &cmdlinev);
        Tcl_Obj **newObjv;
        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+1));
        newObjv[0] = Tcl_NewStringObj(Tcl_GetCommandName(interp,
	        contextObj->accessCmd), -1);
        Tcl_IncrRefCount(newObjv[0]);
        newObjv[1] = Tcl_NewStringObj("", -1);
        Tcl_AppendToObj(newObjv[1], "___constructor_init", -1);
        Tcl_IncrRefCount(newObjv[1]);
	if (objc > 2) {
            memcpy(newObjv+2, objv+2, (objc-2)*sizeof(Tcl_Obj *));
	}
        result = Itclng_PublicObjectCmd(contextClass->infoPtr->currIoPtr->oPtr,
	        interp, contextClass->clsPtr, cmdlinec, cmdlinev);
        Tcl_DecrRefCount(newObjv[1]);
        Tcl_DecrRefCount(newObjv[0]);
        ckfree((char *)newObjv);
        if (result != TCL_OK) {
            return TCL_ERROR;
        }
    }

    /*
     *  Scan through the list of base classes and see if any of these
     *  have not been constructed.  Invoke base class constructors
     *  implicitly, as needed.  Go through the list of base classes
     *  in reverse order, so that least-specific classes are constructed
     *  first.
     */
    elem = Itclng_LastListElem(&contextClass->bases);
    while (elem != NULL) {
        iclsPtr = (ItclngClass*)Itclng_GetListValue(elem);

        if (Tcl_FindHashEntry(contextObj->constructed,
	        (char *)iclsPtr->namePtr) == NULL) {

            result = Itclng_InvokeMethodIfExists(interp, "constructor",
                iclsPtr, contextObj, 0, (Tcl_Obj* CONST*)NULL);

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
                result = Itclng_ConstructBase(interp, contextObj, iclsPtr,
		        objc, objv);
                if (result != TCL_OK) {
                    return TCL_ERROR;
                }
            }
        }
        elem = Itclng_PrevListElem(elem);
    }
    Itclng_PopStack(&contextClass->infoPtr->constructorStack);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_InvokeMethodIfExists()
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
Itclng_InvokeMethodIfExists(
    Tcl_Interp *interp,       /* interpreter */
    CONST char *name,         /* name of desired method */
    ItclngClass *contextClass,  /* current class being constructed */
    ItclngObject *contextObj,   /* object being constructed */
    int objc,                 /* number of arguments */
    Tcl_Obj *CONST objv[])    /* argument objects */
{
    int result = TCL_OK;

    ItclngMemberFunc *imPtr;
    Tcl_HashEntry *entry;
    Tcl_Obj *cmdlinePtr;
    int cmdlinec;
    Tcl_Obj **cmdlinev;

    ItclngShowArgs(1, "Itclng_InvokeMethodIfExists", objc, objv);
    Tcl_Obj *objPtr = Tcl_NewStringObj(name, -1);
    entry = Tcl_FindHashEntry(&contextClass->functions, (char *)objPtr);

    if (entry) {
        imPtr  = (ItclngMemberFunc*)Tcl_GetHashValue(entry);

        /*
         *  Prepend the method name to the list of arguments.
         */
        cmdlinePtr = Itclng_CreateArgs(interp, name, objc, objv);

        (void) Tcl_ListObjGetElements((Tcl_Interp*)NULL, cmdlinePtr,
            &cmdlinec, &cmdlinev);

        /*
         *  Execute the code for the method.  Be careful to protect
         *  the method in case it gets deleted during execution.
         */
        Tcl_Preserve((ClientData)imPtr);

	if (contextObj->oPtr == NULL) {
            return TCL_ERROR;
	}
        result = Itclng_EvalMemberCode(interp, imPtr, contextObj,
	        cmdlinec, cmdlinev);
        Tcl_Release((ClientData)imPtr);
        Tcl_DecrRefCount(cmdlinePtr);
    }
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_ReportFuncErrors()
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
Itclng_ReportFuncErrors(
    Tcl_Interp* interp,        /* interpreter being modified */
    ItclngMemberFunc *imPtr,     /* command member that was invoked */
    ItclngObject *contextObj,    /* object context for this command */
    int result)                /* integer status code from proc body */
{
/* FIX ME !!! */
/* adapt to use of ItclngProcErrorProc for stubs compatibility !! */
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CmdAliasProc()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Command
Itclng_CmdAliasProc(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    CONST char *cmdName,
    ClientData clientData)
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngObject *ioPtr;
    ItclngMemberFunc *imPtr;
    ItclngResolveInfo *resolveInfoPtr;

    resolveInfoPtr = (ItclngResolveInfo *)clientData;
    if (resolveInfoPtr->flags & ITCLNG_RESOLVE_OBJECT) {
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
	            ITCLNG_NAMESPACE"::methodset::callCCommand", NULL, 0);
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
 *  Itclng_VarAliasProc()
 *
 * ------------------------------------------------------------------------
 */
Tcl_Var
Itclng_VarAliasProc(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    CONST char *varName,
    ClientData clientData)
{
    Tcl_HashEntry *hPtr;
    ItclngObjectInfo *infoPtr;
    ItclngClass *iclsPtr;
    ItclngObject *ioPtr;
    ItclngVarLookup *ivlPtr;
    ItclngResolveInfo *resolveInfoPtr;
    ItclngCallContext *callContextPtr;
    Tcl_Var varPtr;

    varPtr = NULL;
    hPtr = NULL;
    callContextPtr = NULL;
    resolveInfoPtr = (ItclngResolveInfo *)clientData;
    if (resolveInfoPtr->flags & ITCLNG_RESOLVE_OBJECT) {
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
 *  ItclngCheckCallProc()
 *
 *
 * ------------------------------------------------------------------------
 */
int
ItclngCheckCallProc(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_ObjectContext contextPtr,
    Tcl_CallFrame *framePtr,
    int *isFinished)
{
    int result;
    ItclngMemberFunc *imPtr;

    imPtr = (ItclngMemberFunc *)clientData;
#ifdef NOTDEF
    if (!imPtr->iclsPtr->infoPtr->useOldResolvers) {
        Itclng_SetCallFrameResolver(interp, imPtr->iclsPtr->resolvePtr);
    }
#endif
    result = TCL_OK;

    if (isFinished != NULL) {
        *isFinished = 0;
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngCheckCallMethod()
 *
 *
 * ------------------------------------------------------------------------
 */
int
ItclngCheckCallMethod(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_ObjectContext contextPtr,
    Tcl_CallFrame *framePtr,
    int *isFinished)
{

    Tcl_Object oPtr;
    Tcl_HashEntry *hPtr;
    Tcl_Namespace *saveNsPtr;
    Tcl_ObjCmdProc *procPtr;
    ItclngObjectInfo *infoPtr;
    ItclngObject *ioPtr;
    ItclngCallContext *callContextPtr;
    ItclngCallContext *callContextPtr2;
    ItclngMemberFunc *imPtr;
    const char *methodName;
    int hadBuiltin;
    int result;
    int isNew;

    oPtr = NULL;
    hPtr = NULL;
    imPtr = (ItclngMemberFunc *)clientData;
fprintf(stderr, "Itclng_CheckCallMethod!%s!\n", Tcl_GetString(imPtr->namePtr));
    infoPtr = imPtr->iclsPtr->infoPtr;
    if (imPtr->flags & ITCLNG_CONSTRUCTOR) {
        ioPtr = imPtr->iclsPtr->infoPtr->currIoPtr;
    } else {
	if (contextPtr == NULL) {
	    if ((imPtr->flags & ITCLNG_COMMON) ||
                    (imPtr->codePtr->flags & ITCLNG_BUILTIN)) {
#ifdef NOTDEF
                if (!imPtr->iclsPtr->infoPtr->useOldResolvers) {
                    Itclng_SetCallFrameResolver(interp,
                            imPtr->iclsPtr->resolvePtr);
                }
#endif
                if (isFinished != NULL) {
                    *isFinished = 0;
                }
                return TCL_OK;
            }
	    Tcl_AppendResult(interp,
	            "ItclngCheckCallMethod cannot get context object (NULL)",
                    " for ", Tcl_GetString(imPtr->fullNamePtr),
		    NULL);
	    return TCL_ERROR;
	}
        oPtr = Tcl_ObjectContextObject(contextPtr);
	ioPtr = Tcl_ObjectGetMetadata(oPtr,
	        imPtr->iclsPtr->infoPtr->object_meta_type);
    }
    if (imPtr->iclsPtr == infoPtr->rootClassIclsPtr) {
        /* this are methods of the root class, check for calls of builtins */
        methodName = Tcl_GetString(imPtr->namePtr);
	hadBuiltin = 0;
        if (strcmp(methodName, "configure") == 0) {
	    hadBuiltin = 1;
	    procPtr = Itclng_ConfigureCmd;
	}
        if (strcmp(methodName, "cget") == 0) {
	    hadBuiltin = 1;
	    procPtr = Itclng_CgetCmd;
	}
	if (hadBuiltin) {
	    saveNsPtr = Tcl_GetCurrentNamespace(interp);
	    Itclng_SetCallFrameNamespace(interp, ioPtr->iclsPtr->nsPtr);
        callContextPtr = (ItclngCallContext *)ckalloc(
                sizeof(ItclngCallContext));
        callContextPtr->objectFlags = ioPtr->flags;
        callContextPtr->nsPtr = saveNsPtr;
        callContextPtr->ioPtr = ioPtr;
        callContextPtr->imPtr = imPtr;
        callContextPtr->refCount = 1;
	    Itclng_PushStack(callContextPtr, &infoPtr->contextStack);
	    result = (* procPtr)(imPtr->iclsPtr, interp,
	            Itclng_GetCallFrameObjc(interp)-1,
	            Itclng_GetCallFrameObjv(interp)+1);
	    Itclng_PopStack(&infoPtr->contextStack);
	    Itclng_SetCallFrameNamespace(interp, saveNsPtr);
	    ckfree((char *)callContextPtr);
            if (isFinished != NULL) {
                *isFinished = 1;
            }
            return result;
        }
    }
    if ((imPtr->codePtr != NULL) &&
            (imPtr->codePtr->flags & ITCLNG_IMPLEMENT_NONE)) {
        Tcl_AppendResult(interp, "member function \"",
	        Tcl_GetString(imPtr->fullNamePtr),
		"\" is not defined and cannot be autoloaded", NULL);
        if (isFinished != NULL) {
            *isFinished = 1;
        }
        return TCL_ERROR;
    }
    int cObjc = Itclng_GetCallFrameObjc(interp);
    Tcl_Obj *const * cObjv = Itclng_GetCallFrameObjv(interp);
ItclngShowArgs(0, "Check", cObjc, cObjv);
fprintf(stderr, "IM!%s!%d!\n", Tcl_GetString(imPtr->namePtr), imPtr->argcount);
    if (cObjc-2 < imPtr->argcount) {
fprintf(stderr, "bad args\n");
	if (strcmp(Tcl_GetString(imPtr->namePtr), "info") == 0) {
            Tcl_Obj *objPtr = Tcl_NewStringObj(
	            "wrong # args: should be one of...\n", -1);
#ifdef NOTDEF
            ItclngGetInfoUsage(interp, objPtr);
#endif
	    Tcl_SetResult(interp, Tcl_GetString(objPtr), TCL_DYNAMIC);
	} else {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
	            Tcl_GetString(cObjv[0]), " ",
	            Tcl_GetString(imPtr->namePtr), " ",
		    ItclngGetUsageString(imPtr->iclsPtr,
		    Tcl_GetString(imPtr->namePtr)),
		    "\"", NULL);
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
	    if ((imPtr->flags & ITCLNG_COMMON) ||
                    (imPtr->codePtr->flags & ITCLNG_BUILTIN)) {
                if (isFinished != NULL) {
                    *isFinished = 0;
                }
                return TCL_OK;
	    }
	    Tcl_AppendResult(interp, "ItclngCheckCallMethod  ioPtr == NULL", NULL);
            if (isFinished != NULL) {
                *isFinished = 1;
            }
	    return TCL_ERROR;
	}
        callContextPtr = (ItclngCallContext *)ckalloc(
                sizeof(ItclngCallContext));
        callContextPtr->objectFlags = ioPtr->flags;
        callContextPtr->nsPtr = Tcl_GetCurrentNamespace(interp);
        callContextPtr->ioPtr = ioPtr;
        callContextPtr->imPtr = imPtr;
        callContextPtr->refCount = 1;
    }
    if (isNew) {
        Tcl_SetHashValue(hPtr, callContextPtr);
    }
    Itclng_PushStack(callContextPtr, &imPtr->iclsPtr->infoPtr->contextStack);

    ioPtr->callRefCount++;
    imPtr->iclsPtr->callRefCount++;
#ifdef NOTDEF
    if (!imPtr->iclsPtr->infoPtr->useOldResolvers) {
        Itclng_SetCallFrameResolver(interp, ioPtr->resolvePtr);
    }
#endif
    result = TCL_OK;

    if (isFinished != NULL) {
        *isFinished = 0;
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngAfterCallMethod()
 *
 *
 * ------------------------------------------------------------------------
 */
int
ItclngAfterCallMethod(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_ObjectContext contextPtr,
    Tcl_Namespace *nsPtr,
    int call_result)
{
    Tcl_Object oPtr;
    Tcl_HashEntry *hPtr;
    ItclngObject *ioPtr;
    ItclngMemberFunc *imPtr;
    ItclngCallContext *callContextPtr;
    int newEntry;

    oPtr = NULL;
    imPtr = (ItclngMemberFunc *)clientData;

    callContextPtr = NULL;
    if (contextPtr != NULL) {
        callContextPtr = Itclng_PopStack(&imPtr->iclsPtr->infoPtr->contextStack);
    }
    if (callContextPtr == NULL) {
        if ((imPtr->flags & ITCLNG_COMMON) ||
                (imPtr->codePtr->flags & ITCLNG_BUILTIN)) {
            return call_result;
        }
	Tcl_AppendResult(interp,
	        "ItclngAfterCallMethod cannot get context object (NULL)",
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
    if (imPtr->flags & (ITCLNG_CONSTRUCTOR | ITCLNG_DESTRUCTOR)) {
        if ((imPtr->flags & ITCLNG_DESTRUCTOR) && ioPtr &&
             ioPtr->destructed) {
            Tcl_CreateHashEntry(ioPtr->destructed,
                (char *)imPtr->iclsPtr->namePtr, &newEntry);
        }
        if ((imPtr->flags & ITCLNG_CONSTRUCTOR) && ioPtr &&
             ioPtr->constructed) {
            Tcl_CreateHashEntry(ioPtr->constructed,
                (char *)imPtr->iclsPtr->namePtr, &newEntry);
        }
    }
    ioPtr->callRefCount--;
    imPtr->iclsPtr->callRefCount--;
if (ioPtr->flags != callContextPtr->objectFlags) {
fprintf(stderr, "IOPTR_FLAGS2!0x%08x!0x%08x!%s!%d!%d!\n", ioPtr->flags, callContextPtr->objectFlags, Tcl_GetString(imPtr->fullNamePtr), ioPtr->callRefCount, imPtr->iclsPtr->callRefCount);
}
    if (ioPtr->flags & ITCLNG_OBJECT_SHOULD_VARNS_DELETE) {
fprintf(stderr, "DELOBJVAR!%s!%d!\n", Tcl_GetCommandName(interp, ioPtr->accessCmd), ioPtr->callRefCount);
        ItclngDeleteObjectVariablesNamespace(interp, ioPtr);
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
ItclngProcErrorProc(
    Tcl_Interp *interp,
    Tcl_Obj *procNameObj)
{
    ItclngObjectInfo *infoPtr;
    ItclngCallContext *callContextPtr;
    ItclngMemberFunc *imPtr;
    ItclngObject *contextIoPtr;
    ItclngClass *currIclsPtr;
    Tcl_Obj *objPtr;
    Tcl_Namespace *upNsPtr;
    char num[20];
    int constructorStackIndex;
    int constructorStackSize;
    int isFirstLoop;
    int loopCnt;

    infoPtr = (ItclngObjectInfo *)Tcl_GetAssocData(interp,
            ITCLNG_INTERP_DATA, NULL);
    callContextPtr = Itclng_PeekStack(&infoPtr->contextStack);
    loopCnt = 1;
    isFirstLoop = 1;
    upNsPtr = Itclng_GetUplevelNamespace(interp, 1);
    constructorStackIndex = -1;
    while ((callContextPtr != NULL) && (loopCnt > 0)) {
	imPtr = callContextPtr->imPtr;
        contextIoPtr = callContextPtr->ioPtr;
        objPtr = Tcl_NewStringObj("\n    ", -1);
        Tcl_IncrRefCount(objPtr);

        if (imPtr->flags & ITCLNG_CONSTRUCTOR) {
	    /* have to look for classes in construction where the constructor
	     * has not yet been called, but only the initCode or the
	     * inherited constructors
	     */
            if (isFirstLoop) {
	        isFirstLoop = 0;
                constructorStackSize = Itclng_GetStackSize(
		        &imPtr->iclsPtr->infoPtr->constructorStack);
	        constructorStackIndex = constructorStackSize;
	        currIclsPtr = imPtr->iclsPtr;
	    } else {
	        currIclsPtr = (ItclngClass *)Itclng_GetStackValue(
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
            if ((imPtr->codePtr->flags & ITCLNG_IMPLEMENT_TCL) != 0) {
                Tcl_AppendToObj(objPtr, " (", -1);
            }
        }
        if (imPtr->flags & ITCLNG_CONINIT) {
            Tcl_AppendToObj(objPtr, "while constructing object \"", -1);
            Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
            Tcl_AppendToObj(objPtr, "\" in ", -1);
            Tcl_AppendToObj(objPtr,
	            Tcl_GetString(imPtr->iclsPtr->fullNamePtr), -1);
            Tcl_AppendToObj(objPtr, "::constructor", -1);
            if ((imPtr->codePtr->flags & ITCLNG_IMPLEMENT_TCL) != 0) {
                Tcl_AppendToObj(objPtr, " (", -1);
            }
        }
	if (imPtr->flags & ITCLNG_DESTRUCTOR) {
	    Tcl_AppendToObj(objPtr, "while deleting object \"", -1);
            Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
            Tcl_AppendToObj(objPtr, "\" in ", -1);
            Tcl_AppendToObj(objPtr, Tcl_GetString(imPtr->fullNamePtr), -1);
            if ((imPtr->codePtr->flags & ITCLNG_IMPLEMENT_TCL) != 0) {
                Tcl_AppendToObj(objPtr, " (", -1);
            }
        }
	if (!(imPtr->flags & (ITCLNG_CONSTRUCTOR|ITCLNG_DESTRUCTOR|ITCLNG_CONINIT))) {
            Tcl_AppendToObj(objPtr, "(", -1);

            if ((contextIoPtr != NULL) && (contextIoPtr->accessCmd)) {
                Tcl_AppendToObj(objPtr, "object \"", -1);
                Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);
                Tcl_AppendToObj(objPtr, "\" ", -1);
            }

            if ((imPtr->flags & ITCLNG_COMMON) != 0) {
                Tcl_AppendToObj(objPtr, "procedure", -1);
            } else {
                Tcl_AppendToObj(objPtr, "method", -1);
            }
            Tcl_AppendToObj(objPtr, " \"", -1);
            Tcl_AppendToObj(objPtr, Tcl_GetString(imPtr->fullNamePtr), -1);
            Tcl_AppendToObj(objPtr, "\" ", -1);
        }

        if ((imPtr->codePtr->flags & ITCLNG_IMPLEMENT_TCL) != 0) {
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
