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
 *  These procedures handle command and variable resolution
 *
 * ========================================================================
 *  AUTHOR:  Michael J. McLennan
 *           Bell Labs Innovations for Lucent Technologies
 *           mmclennan@lucent.com
 *           http://www.tcltk.com/itcl
 *
 *     RCS:  $Id: itclResolve.c,v 1.1.2.16 2008/10/29 20:28:47 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"

#ifdef NOTDEF
struct Tcl_ResolvedVarInfo;

typedef Tcl_Var (Tcl_ResolveRuntimeVarProc)(Tcl_Interp *interp,
	struct Tcl_ResolvedVarInfo *vinfoPtr);

typedef void (Tcl_ResolveVarDeleteProc)(struct Tcl_ResolvedVarInfo *vinfoPtr);

/*
 * The following structure encapsulates the routines needed to resolve a
 * variable reference at runtime. Any variable specific state will typically
 * be appended to this structure.
 */

typedef struct Tcl_ResolvedVarInfo {
    Tcl_ResolveRuntimeVarProc *fetchProc;
    Tcl_ResolveVarDeleteProc *deleteProc;
} Tcl_ResolvedVarInfo;

typedef int (Tcl_ResolveCompiledVarProc) (Tcl_Interp *interp,
	const char *name, int length, Tcl_Namespace *context,
	Tcl_ResolvedVarInfo **rPtr);

typedef int (Tcl_ResolveVarProc) (Tcl_Interp *interp, const char *name,
	Tcl_Namespace *context, int flags, Tcl_Var *rPtr);

typedef int (Tcl_ResolveCmdProc) (Tcl_Interp *interp, const char *name,
	Tcl_Namespace *context, int flags, Tcl_Command *rPtr);

typedef struct Tcl_ResolverInfo {
    Tcl_ResolveCmdProc *cmdResProc;
				/* Procedure handling command name
				 * resolution. */
    Tcl_ResolveVarProc *varResProc;
				/* Procedure handling variable name resolution
				 * for variables that can only be handled at
				 * runtime. */
    Tcl_ResolveCompiledVarProc *compiledVarResProc;
				/* Procedure handling variable name resolution
				 * at compile time. */
} Tcl_ResolverInfo;
#endif

/*
 * This structure is a subclass of Tcl_ResolvedVarInfo that contains the
 * ItclVarLookup info needed at runtime.
 */
typedef struct ItclResolvedVarInfo {
    Tcl_ResolvedVarInfo vinfo;        /* This must be the first element. */
    ItclVarLookup *vlookup;           /* Pointer to lookup info. */
} ItclResolvedVarInfo;

static Tcl_Var ItclClassRuntimeVarResolver _ANSI_ARGS_((
    Tcl_Interp *interp, Tcl_ResolvedVarInfo *vinfoPtr));


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassCmdResolver()
 *
 *  Used by the class namespaces to handle name resolution for all
 *  commands.  This procedure looks for references to class methods
 *  and procs, and returns TCL_OK along with the appropriate Tcl
 *  command in the rPtr argument.  If a particular command is private,
 *  this procedure returns TCL_ERROR and access to the command is
 *  denied.  If a command is not recognized, this procedure returns
 *  TCL_CONTINUE, and lookup continues via the normal Tcl name
 *  resolution rules.
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassCmdResolver(
    Tcl_Interp *interp,		/* current interpreter */
    CONST char* name,		/* name of the command being accessed */
    Tcl_Namespace *nsPtr,	/* namespace performing the resolution */
    int flags,			/* TCL_LEAVE_ERR_MSG => leave error messages
				 *   in interp if anything goes wrong */
    Tcl_Command *rPtr)		/* returns: resolved command */
{
    ItclClass *iclsPtr;
    ItclObjectInfo *infoPtr;

    Tcl_HashEntry *hPtr;
    ItclMemberFunc *imPtr;
    int isCmdDeleted;

    if ((name[0] == 't') && (strcmp(name, "this") == 0)) {
        return TCL_CONTINUE;
    }
    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
                ITCL_INTERP_DATA, NULL);
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)nsPtr);
    if (hPtr == NULL) {
        return TCL_CONTINUE;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    /*
     *  If the command is a member function
     */
    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveCmds, name);
    if (hPtr == NULL) {
#ifdef NOTDEF
	if (!(iclsPtr->flags & ITCL_CLASS)) {
	    namePtr = Tcl_NewStringObj(name, -1);
	    Tcl_IncrRefCount(namePtr);
	    hPtr = Tcl_FindHashEntry(&iclsPtr->components, (char *)namePtr);
	    Tcl_DecrRefCount(namePtr);
            if (hPtr != NULL) {
	    }
        }
        if (hPtr == NULL) {
#endif
            return TCL_CONTINUE;
#ifdef NOTDEF
        }
        imPtr = (ItclMemberFunc*)Tcl_GetHashValue(hPtr);
#endif
    } else {
        ItclCmdLookup *clookup;
        clookup = (ItclCmdLookup *)Tcl_GetHashValue(hPtr);
        imPtr = clookup->imPtr;
    }

    /*
     *  Looks like we found an accessible member function.
     *
     *  TRICKY NOTE:  Check to make sure that the command handle
     *    is still valid.  If someone has deleted or renamed the
     *    command, it may not be.  This is just the time to catch
     *    it--as it is being resolved again by the compiler.
     */
    
    /*
     * The following #if is needed so itcl can be compiled with
     * all versions of Tcl.  The integer "deleted" was renamed to
     * "flags" in tcl8.4a2.  This #if is also found in itcl_ensemble.c .
     * We're using a runtime check with itclCompatFlags to adjust for
     * the behavior of this change, too.
     *
     */
/* FIXME !!! */
isCmdDeleted = 0;
//    isCmdDeleted = (!imPtr->accessCmd || imPtr->accessCmd->flags);

    if (isCmdDeleted) {
	imPtr->accessCmd = NULL;

	if ((flags & TCL_LEAVE_ERR_MSG) != 0) {
	    Tcl_AppendResult(interp,
		"can't access \"", name, "\": deleted or redefined\n",
		"(use the \"body\" command to redefine methods/procs)",
		(char*)NULL);
	}
	return TCL_ERROR;   /* disallow access! */
    }
    *rPtr = imPtr->accessCmd;
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassVarResolver()
 *
 *  Used by the class namespaces to handle name resolution for runtime
 *  variable accesses.  This procedure looks for references to both
 *  common variables and instance variables at runtime.  It is used as
 *  a second line of defense, to handle references that could not be
 *  resolved as compiled locals.
 *
 *  If a variable is found, this procedure returns TCL_OK along with
 *  the appropriate Tcl variable in the rPtr argument.  If a particular
 *  variable is private, this procedure returns TCL_ERROR and access
 *  to the variable is denied.  If a variable is not recognized, this
 *  procedure returns TCL_CONTINUE, and lookup continues via the normal
 *  Tcl name resolution rules.
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassVarResolver(
    Tcl_Interp *interp,       /* current interpreter */
    CONST char* name,	      /* name of the variable being accessed */
    Tcl_Namespace *nsPtr,   /* namespace performing the resolution */
    int flags,                /* TCL_LEAVE_ERR_MSG => leave error messages
                               *   in interp if anything goes wrong */
    Tcl_Var *rPtr)            /* returns: resolved variable */
{
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
    ItclObject *contextIoPtr;
    Tcl_HashEntry *hPtr;
    ItclVarLookup *vlookup;

    Tcl_Namespace *upNsPtr;
    upNsPtr = Itcl_GetUplevelNamespace(interp, 1);
    assert(Itcl_IsClassNamespace(nsPtr));

    /*
     *  If this is a global variable, handle it in the usual
     *  Tcl manner.
     */
    if (flags & TCL_GLOBAL_ONLY) {
        return TCL_CONTINUE;
    }

    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
                ITCL_INTERP_DATA, NULL);
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)nsPtr);
    if (hPtr == NULL) {
        return TCL_CONTINUE;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);

    /*
     *  See if this is a formal parameter in the current proc scope.
     *  If so, that variable has precedence.  Look it up and return
     *  it here.  This duplicates some of the functionality of
     *  TclLookupVar, but we return it here (instead of returning
     *  TCL_CONTINUE) to avoid looking it up again later.
     */
    ItclCallContext *callContextPtr;
    callContextPtr = Itcl_PeekStack(&infoPtr->contextStack);
    if ((strstr(name,"::") == NULL) &&
            Itcl_IsCallFrameArgument(interp, name)) {
        return TCL_CONTINUE;
    }

    /*
     *  See if the variable is a known data member and accessible.
     */
    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveVars, name);
    if (hPtr == NULL) {
        return TCL_CONTINUE;
    }

    vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
    if (!vlookup->accessible) {
        return TCL_CONTINUE;
    }

    /*
     * If this is a common data member, then its variable
     * is easy to find.  Return it directly.
     */
    if ((vlookup->ivPtr->flags & ITCL_COMMON) != 0) {
	hPtr = Tcl_FindHashEntry(&vlookup->ivPtr->iclsPtr->classCommons,
	        (char *)vlookup->ivPtr);
	if (hPtr != NULL) {
	    *rPtr = Tcl_GetHashValue(hPtr);
            return TCL_OK;
	}
    }

    /*
     *  If this is an instance variable, then we have to
     *  find the object context,
     */

    if (callContextPtr == NULL) {
        return TCL_CONTINUE;
    }
    if (callContextPtr->ioPtr == NULL) {
        return TCL_CONTINUE;
    }
    contextIoPtr = callContextPtr->ioPtr;

    /*
     *  TRICKY NOTE:  We've resolved the variable in the current
     *    class context, but we must also be careful to get its
     *    index from the most-specific class context.  Variables
     *    are arranged differently depending on which class
     *    constructed the object.
     */
    if (contextIoPtr->iclsPtr != vlookup->ivPtr->iclsPtr) {
	if (strcmp(Tcl_GetString(vlookup->ivPtr->namePtr), "this") == 0) {
            hPtr = Tcl_FindHashEntry(&contextIoPtr->iclsPtr->resolveVars,
                Tcl_GetString(vlookup->ivPtr->namePtr));

            if (hPtr != NULL) {
                vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
            }
        }
    }
    hPtr = Tcl_FindHashEntry(&contextIoPtr->objectVariables,
            (char *)vlookup->ivPtr);
    if (strcmp(name, "this") == 0) {
        Tcl_DString buffer;
	Tcl_DStringInit(&buffer);
	Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
	Tcl_DStringAppend(&buffer, "::", 2);
	Tcl_DStringAppend(&buffer, Tcl_GetString(contextIoPtr->namePtr), -1);
	if (vlookup->ivPtr->iclsPtr->nsPtr == NULL) {
	    /* deletion of class is running */
	    Tcl_DStringAppend(&buffer,
	             Tcl_GetCurrentNamespace(interp)->fullName, -1);
        } else {
	    Tcl_DStringAppend(&buffer,
	             vlookup->ivPtr->iclsPtr->nsPtr->fullName, -1);
	}
	Tcl_DStringAppend(&buffer, "::this", 6);
        Tcl_Var varPtr;
	varPtr = Itcl_FindNamespaceVar(interp, Tcl_DStringValue(&buffer), NULL, 0);
        if (varPtr != NULL) {
            *rPtr = varPtr;
	    return TCL_OK;
        }
    }
    if (strcmp(name, "itcl_options") == 0) {
        Tcl_DString buffer;
	Tcl_DStringInit(&buffer);
	Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
	Tcl_DStringAppend(&buffer, "::", 2);
	Tcl_DStringAppend(&buffer, Tcl_GetString(contextIoPtr->namePtr), -1);
	Tcl_DStringAppend(&buffer, "::itcl_options", -1);
        Tcl_Var varPtr;
	varPtr = Itcl_FindNamespaceVar(interp, Tcl_DStringValue(&buffer), NULL, 0);
        if (varPtr != NULL) {
            *rPtr = varPtr;
	    return TCL_OK;
        }
    }
    if (hPtr != NULL) {
        *rPtr = Tcl_GetHashValue(hPtr);
        return TCL_OK;
    }
    return TCL_CONTINUE;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassCompiledVarResolver()
 *
 *  Used by the class namespaces to handle name resolution for compile
 *  time variable accesses.  This procedure looks for references to
 *  both common variables and instance variables at compile time.  If
 *  the variables are found, they are characterized in a generic way
 *  by their ItclVarLookup record.  At runtime, Tcl constructs the
 *  compiled local variables by calling ItclClassRuntimeVarResolver.
 *
 *  If a variable is found, this procedure returns TCL_OK along with
 *  information about the variable in the rPtr argument.  If a particular
 *  variable is private, this procedure returns TCL_ERROR and access
 *  to the variable is denied.  If a variable is not recognized, this
 *  procedure returns TCL_CONTINUE, and lookup continues via the normal
 *  Tcl name resolution rules.
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassCompiledVarResolver(
    Tcl_Interp *interp,         /* current interpreter */
    CONST char* name,           /* name of the variable being accessed */
    int length,                 /* number of characters in name */
    Tcl_Namespace *nsPtr,       /* namespace performing the resolution */
    Tcl_ResolvedVarInfo **rPtr) /* returns: info that makes it possible to
                                 *   resolve the variable at runtime */
{
    ItclClass *iclsPtr;
    ItclObjectInfo *infoPtr;
    Tcl_HashEntry *hPtr;
    ItclVarLookup *vlookup;
    char *buffer;
    char storage[64];

    assert(Itcl_IsClassNamespace(nsPtr));

    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
                ITCL_INTERP_DATA, NULL);
    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)nsPtr);
    if (hPtr == NULL) {
        return TCL_CONTINUE;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);
    /*
     *  Copy the name to local storage so we can NULL terminate it.
     *  If the name is long, allocate extra space for it.
     */
    if (length < sizeof(storage)) {
        buffer = storage;
    } else {
        buffer = (char*)ckalloc((unsigned)(length+1));
    }
    memcpy((void*)buffer, (void*)name, (size_t)length);
    buffer[length] = '\0';

    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveVars, buffer);

    if (buffer != storage) {
        ckfree(buffer);
    }

    /*
     *  If the name is not found, or if it is inaccessible,
     *  continue on with the normal Tcl name resolution rules.
     */
    if (hPtr == NULL) {
        return TCL_CONTINUE;
    }

    vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
    if (!vlookup->accessible) {
        return TCL_CONTINUE;
    }

    /*
     *  Return the ItclVarLookup record.  At runtime, Tcl will
     *  call ItclClassRuntimeVarResolver with this record, to
     *  plug in the appropriate variable for the current object
     *  context.
     */
    (*rPtr) = (Tcl_ResolvedVarInfo *) ckalloc(sizeof(ItclResolvedVarInfo));
    (*rPtr)->fetchProc = ItclClassRuntimeVarResolver;
    (*rPtr)->deleteProc = NULL;
    ((ItclResolvedVarInfo*)(*rPtr))->vlookup = vlookup;

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  ItclClassRuntimeVarResolver()
 *
 *  Invoked when Tcl sets up the call frame for an [incr Tcl] method/proc
 *  at runtime.  Resolves data members identified earlier by
 *  Itcl_ClassCompiledVarResolver.  Returns the Tcl_Var representation
 *  for the data member.
 * ------------------------------------------------------------------------
 */
static Tcl_Var
ItclClassRuntimeVarResolver(
    Tcl_Interp *interp,               /* current interpreter */
    Tcl_ResolvedVarInfo *resVarInfo)  /* contains ItclVarLookup rep
                                       * for variable */
{
    ItclVarLookup *vlookup = ((ItclResolvedVarInfo*)resVarInfo)->vlookup;

    ItclClass *iclsPtr;
    ItclObject *contextIoPtr;
    Tcl_HashEntry *hPtr;

    /*
     *  If this is a common data member, then the associated
     *  variable is known directly.
     */
    if ((vlookup->ivPtr->flags & ITCL_COMMON) != 0) {
	hPtr = Tcl_FindHashEntry(&vlookup->ivPtr->iclsPtr->classCommons,
	        (char *)vlookup->ivPtr);
	if (hPtr != NULL) {
	    return Tcl_GetHashValue(hPtr);
	}
    }
    iclsPtr = vlookup->ivPtr->iclsPtr;

    /*
     *  Otherwise, get the current object context and find the
     *  variable in its data table.
     *
     *  TRICKY NOTE:  Get the index for this variable using the
     *    virtual table for the MOST-SPECIFIC class.
     */

    ItclCallContext *callContextPtr;
    callContextPtr = Itcl_PeekStack(&iclsPtr->infoPtr->contextStack);
    if (callContextPtr == NULL) {
        return NULL;
    }
    if (callContextPtr->ioPtr == NULL) {
        return NULL;
    }
    contextIoPtr = callContextPtr->ioPtr;
    if (contextIoPtr != NULL) {
        if (contextIoPtr->iclsPtr != vlookup->ivPtr->iclsPtr) {
            hPtr = Tcl_FindHashEntry(&contextIoPtr->iclsPtr->resolveVars,
                Tcl_GetString(vlookup->ivPtr->namePtr));

            if (hPtr != NULL) {
                vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);
            } else {
	    }
        }
        hPtr = Tcl_FindHashEntry(&contextIoPtr->objectVariables,
                (char *)vlookup->ivPtr);
        if (strcmp(Tcl_GetString(vlookup->ivPtr->namePtr), "this") == 0) {
            Tcl_DString buffer;
	    Tcl_DStringInit(&buffer);
	    Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
	    Tcl_DStringAppend(&buffer, "::", 2);
	    Tcl_DStringAppend(&buffer,
	            Tcl_GetString(contextIoPtr->namePtr), -1);
	    if (vlookup->ivPtr->iclsPtr->nsPtr == NULL) {
	        Tcl_DStringAppend(&buffer,
	                Tcl_GetCurrentNamespace(interp)->fullName, -1);
	    } else {
	        Tcl_DStringAppend(&buffer,
	                vlookup->ivPtr->iclsPtr->nsPtr->fullName, -1);
	    }
	    Tcl_DStringAppend(&buffer, "::this", 6);
            Tcl_Var varPtr;
	    varPtr = Itcl_FindNamespaceVar(interp, Tcl_DStringValue(&buffer),
	            NULL, 0);
            if (varPtr != NULL) {
	        return varPtr;
            }
        }
        if (strcmp(Tcl_GetString(vlookup->ivPtr->namePtr),
	        "itcl_options") == 0) {
            Tcl_DString buffer;
	    Tcl_DStringInit(&buffer);
	    Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
	    Tcl_DStringAppend(&buffer, "::", 2);
	    Tcl_DStringAppend(&buffer,
	            Tcl_GetString(contextIoPtr->namePtr), -1);
	    Tcl_DStringAppend(&buffer, "::itcl_options", -1);
            Tcl_Var varPtr;
	    varPtr = Itcl_FindNamespaceVar(interp, Tcl_DStringValue(&buffer),
	            NULL, 0);
            if (varPtr != NULL) {
	        return varPtr;
            }
        }
        if (hPtr != NULL) {
            return (Tcl_Var)Tcl_GetHashValue(hPtr);
        }
    } else {
    }
    return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ParseVarResolver()
 *
 *  Used by the "parser" namespace to resolve variable accesses to
 *  common variables.  The runtime resolver procedure is consulted
 *  whenever a variable is accessed within the namespace.  It can
 *  deny access to certain variables, or perform special lookups itself.
 *
 *  This procedure allows access only to "common" class variables that
 *  have been declared within the class or inherited from another class.
 *  A "set" command can be used to initialized common data members within
 *  the body of the class definition itself:
 *
 *    itcl::class Foo {
 *        common colors
 *        set colors(red)   #ff0000
 *        set colors(green) #00ff00
 *        set colors(blue)  #0000ff
 *        ...
 *    }
 *
 *    itcl::class Bar {
 *        inherit Foo
 *        set colors(gray)  #a0a0a0
 *        set colors(white) #ffffff
 *
 *        common numbers
 *        set numbers(0) zero
 *        set numbers(1) one
 *    }
 *
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
int
Itcl_ParseVarResolver(
    Tcl_Interp *interp,        /* current interpreter */
    const char* name,        /* name of the variable being accessed */
    Tcl_Namespace *contextNs,  /* namespace context */
    int flags,                 /* TCL_GLOBAL_ONLY => global variable
                                * TCL_NAMESPACE_ONLY => namespace variable */
    Tcl_Var* rPtr)             /* returns: Tcl_Var for desired variable */
{
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)contextNs->clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);

    Tcl_HashEntry *hPtr;
    ItclVarLookup *vlookup;

    /*
     *  See if the requested variable is a recognized "common" member.
     *  If it is, make sure that access is allowed.
     */
    hPtr = Tcl_FindHashEntry(&iclsPtr->resolveVars, name);
    if (hPtr) {
        vlookup = (ItclVarLookup*)Tcl_GetHashValue(hPtr);

        if ((vlookup->ivPtr->flags & ITCL_COMMON) != 0) {
            if (!vlookup->accessible) {
                Tcl_AppendResult(interp,
                    "can't access \"", name, "\": ",
                    Itcl_ProtectionStr(vlookup->ivPtr->protection),
                    " variable",
                    (char*)NULL);
                return TCL_ERROR;
            }
	    hPtr = Tcl_FindHashEntry(&vlookup->ivPtr->iclsPtr->classCommons,
	        (char *)vlookup->ivPtr);
	    if (hPtr != NULL) {
                *rPtr = Tcl_GetHashValue(hPtr);
                return TCL_OK;
	    }
        }
    }

    /*
     *  If the variable is not recognized, return TCL_CONTINUE and
     *  let lookup continue via the normal name resolution rules.
     *  This is important for variables like "errorInfo"
     *  that might get set while the parser namespace is active.
     */
    return TCL_CONTINUE;
}



int
ItclSetParserResolver(
    Tcl_Namespace *nsPtr)
{
    Itcl_SetNamespaceResolvers(nsPtr, (Tcl_ResolveCmdProc*)NULL,
            Itcl_ParseVarResolver, (Tcl_ResolveCompiledVarProc*)NULL);
    return TCL_OK;
}
