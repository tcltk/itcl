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
 *  This file defines information that tracks classes and objects
 *  at a global level for a given interpreter.
 *
 * ========================================================================
 *  AUTHOR:  Michael J. McLennan
 *           Bell Labs Innovations for Lucent Technologies
 *           mmclennan@lucent.com
 *           http://www.tcltk.com/itcl
 *
 *  overhauled version author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclCmd.c,v 1.1.2.5 2007/09/15 11:56:11 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"

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
                Tcl_GetString(contextIclsPtr->fullname), "\"",
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
         *  an object context.  Return the name "@itcl object variable".
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

        objPtr = Tcl_NewStringObj((char*)NULL, 0);
        Tcl_IncrRefCount(objPtr);
        Tcl_GetCommandFullName(interp, contextIoPtr->accessCmd, objPtr);

        objPtr2 = Tcl_NewStringObj((char*)NULL, 0);
        Tcl_IncrRefCount(objPtr2);
	Tcl_AppendToObj(objPtr2, ITCL_VARIABLES_NAMESPACE, -1);
	Tcl_AppendToObj(objPtr2, Tcl_GetString(objPtr), -1);
        Tcl_AppendToObj(objPtr2,
	    Tcl_GetString(vlookup->ivPtr->fullNamePtr), -1);

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

        var = Tcl_FindNamespaceVar(interp, token, contextNsPtr,
            TCL_NAMESPACE_ONLY);

        if (!var) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "variable \"", token, "\" not found in namespace \"",
                contextNsPtr->fullName, "\"",
                (char*)NULL);
            result = TCL_ERROR;
            goto scopeCmdDone;
        }

        Tcl_GetVariableFullName(interp, var, resultPtr);
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
    Tcl_Obj **newObjv;
    int result;

    ItclShowArgs(1, "Itcl_ForwardAddCmd", objc, objv);
//    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+1));
    newObjv[0] = Tcl_NewStringObj("::oo::define", -1);
    Tcl_IncrRefCount(newObjv[0]);
    newObjv[1] = objv[1];
    newObjv[2] = Tcl_NewStringObj("forward", -1);
    Tcl_IncrRefCount(newObjv[2]);
    memcpy(newObjv+3, objv+2, sizeof(Tcl_Obj *)*(objc-2));
    ItclShowArgs(1, "Itcl_ForwardAddCmd2", objc+1, newObjv);
    result = Tcl_EvalObjv(interp, objc+1, newObjv, 0);
    Tcl_DecrRefCount(newObjv[0]);
    Tcl_DecrRefCount(newObjv[2]);

    return result;
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
    int result;

    ItclShowArgs(1, "Itcl_MixinAddCmd", objc, objv);
//    Tcl_Namespace *contextNs = Tcl_GetCurrentNamespace(interp);
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
