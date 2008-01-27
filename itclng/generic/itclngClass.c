/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclngInt.h"

static Tcl_NamespaceDeleteProc* _TclOONamespaceDeleteProc = NULL;

/*
 *  FORWARD DECLARATIONS
 */
static void ItclngDestroyClass _ANSI_ARGS_((ClientData cdata));
static void ItclngDestroyClassNamesp _ANSI_ARGS_((ClientData cdata));
static void ItclngFreeClass _ANSI_ARGS_((char* cdata));
static void ItclngDeleteComponent(ItclngComponent *icPtr);
static void ItclngDeleteOption(ItclngOption *ioptPtr);
#ifdef NOTDEF
static void ItclngDeleteDelegatedOption(ItclngDelegatedOption *idoPtr);
#endif
static void ItclngDeleteDelegatedFunction(ItclngDelegatedFunction *idmPtr);


/*
 * ------------------------------------------------------------------------
 *  ClassRenamedTrace()
 *
 * ------------------------------------------------------------------------
 */

static void
ClassRenamedTrace(
    ClientData clientData,      /* The class being deleted. */
    Tcl_Interp *interp,         /* The interpreter containing the object. */
    const char *oldName,        /* What the object was (last) called. */
    const char *newName,        /* Always NULL ??. not for itk!! */
    int flags)                  /* Why was the object deleted? */
{
    /* FIX ME !! maybe not needed at all !! */
    if (newName != NULL) {
        return;
    }
}


/*
 * ------------------------------------------------------------------------
 *  ItclngDeleteClassMetadata()
 *
 *  Delete the metadata data if any
 *-------------------------------------------------------------------------
 */
void
ItclngDeleteClassMetadata(
    ClientData clientData)
{
    ItclngClass *iclsPtr;

    iclsPtr = clientData;
    iclsPtr->flags |= ITCLNG_CLASS_DELETE_CALLED;
}

/*
 * ------------------------------------------------------------------------
 *  ClassNamespaceDeleted()
 *
 * ------------------------------------------------------------------------
 */
static char *
ClassNamespaceDeleted(
    ClientData clientData,
    Tcl_Interp *interp,
    CONST84 char *part1,
    CONST84 char *part2,
    int flags)
{
    Tcl_DString buffer;
    Tcl_Namespace *nsPtr;
    ItclngClass *iclsPtr;
    
    iclsPtr = clientData;

    if (iclsPtr->nsPtr == NULL) {
        return NULL;
    }
    if (iclsPtr->flags & ITCLNG_CLASS_DELETED) {
        return NULL;
    }
    iclsPtr->nsPtr = NULL;
    /* delete the namespace for the common variables */
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer,
            Tcl_GetString(iclsPtr->infoPtr->internalVars), -1);
    Tcl_DStringAppend(&buffer, Tcl_GetString(iclsPtr->fullNamePtr), -1);
    nsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer), NULL, 0);
    Tcl_DStringFree(&buffer);
    if (nsPtr != NULL) {
        Tcl_DeleteNamespace(nsPtr);
    }
    ItclngDestroyClassNamesp(iclsPtr);
    return NULL;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateClassCmd()
 *
 *  Creates a namespace and its associated class definition data.
 *  If a namespace already exists with that name, then this routine
 *  returns TCL_ERROR, along with an error message in the interp.
 *  If successful, it returns TCL_OK and the new full class name
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateClassCmd(
    ClientData clientData,	/* info for all known objects */
    Tcl_Interp* interp,		/* interpreter that will contain new class */
    int objc,		        /* number of arguments */
    Tcl_Obj *const *objv)	/* returns: pointer to class definition */
{
    ItclngObjectInfo *infoPtr;
    Tcl_DString buffer;
    Tcl_Command cmd;
    Tcl_CmdInfo cmdInfo;
    Tcl_Namespace *classNs;
    Tcl_Obj *nameObjPtr;
    Tcl_Obj *namePtr;
    Tcl_Object oPtr;
    Tcl_Class clsPtr;
    ItclngClass *iclsPtr;
    ItclngVariable *ivPtr;
    Tcl_HashEntry *hPtr;
    int newEntry;

    ItclngShowArgs(1, "Itclng_CreateClassCmd", objc, objv);
    if (ItclngCheckNumCmdParams(interp, infoPtr, "createClass", objc, 2, 2) !=
            TCL_OK) {
        return TCL_ERROR;
    }
    infoPtr = (ItclngObjectInfo *)clientData;
    oPtr = NULL;
    ivPtr = NULL;
    nameObjPtr = objv[1];
    oPtr = Tcl_GetObjectFromObj(interp, objv[2]);
    if (oPtr == NULL) {
	Tcl_AppendResult(interp,
	        "ITCLNG: cannot get TclOO Object for class \"",
                Tcl_GetString(objv[2]), "\"", NULL);
        return TCL_ERROR;
    }
    if (infoPtr->rootClassObjectPtr == NULL) {
	/* the root class of Itclng muts be the first one to be created !! */
        infoPtr->rootClassObjectPtr = oPtr;
    }
    clsPtr = Tcl_GetObjectAsClass(oPtr);
    /*
     *  Allocate class definition data.
     */
    iclsPtr = (ItclngClass*)ckalloc(sizeof(ItclngClass));
    memset(iclsPtr, 0, sizeof(ItclngClass));
    iclsPtr->interp = interp;
    iclsPtr->infoPtr = infoPtr;
    Tcl_Preserve((ClientData)infoPtr);

    Tcl_InitObjHashTable(&iclsPtr->variables);
    Tcl_InitObjHashTable(&iclsPtr->functions);
    Tcl_InitObjHashTable(&iclsPtr->options);
    Tcl_InitObjHashTable(&iclsPtr->components);
    Tcl_InitObjHashTable(&iclsPtr->delegatedOptions);
    Tcl_InitObjHashTable(&iclsPtr->delegatedFunctions);
    Tcl_InitObjHashTable(&iclsPtr->methodVariables);

    Tcl_InitHashTable(&iclsPtr->classCommons, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&iclsPtr->resolveVars, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&iclsPtr->resolveCmds, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&iclsPtr->contextCache, TCL_ONE_WORD_KEYS);

    Itclng_InitList(&iclsPtr->bases);
    Itclng_InitList(&iclsPtr->derived);

    ItclngResolveInfo *resolveInfoPtr = (ItclngResolveInfo *)
            ckalloc(sizeof(ItclngResolveInfo));
    memset (resolveInfoPtr, 0, sizeof(ItclngResolveInfo));
    resolveInfoPtr->flags = ITCLNG_RESOLVE_CLASS;
    resolveInfoPtr->iclsPtr = iclsPtr;
    iclsPtr->resolvePtr = (Tcl_Resolve *)ckalloc(sizeof(Tcl_Resolve));
    iclsPtr->resolvePtr->cmdProcPtr = Itclng_CmdAliasProc;
    iclsPtr->resolvePtr->varProcPtr = Itclng_VarAliasProc;
    iclsPtr->resolvePtr->clientData = resolveInfoPtr;

    /*
     *  Initialize the heritage info -- each class starts with its
     *  own class definition in the heritage.  Base classes are
     *  added to the heritage from the "inherit" statement.
     */
    Tcl_InitHashTable(&iclsPtr->heritage, TCL_ONE_WORD_KEYS);
    (void) Tcl_CreateHashEntry(&iclsPtr->heritage, (char*)iclsPtr, &newEntry);

    /*
     *  Create a namespace to represent the class.  Add the class
     *  definition info as client data for the namespace.  If the
     *  namespace already exists, then replace any existing client
     *  data with the class data.
     */
    Tcl_Preserve((ClientData)iclsPtr);

    if (strcmp(Tcl_GetString(nameObjPtr),
            Tcl_GetString(infoPtr->rootClassName)) == 0) {
        oPtr = Tcl_GetObjectFromObj(interp, nameObjPtr);
    } else {
        oPtr = Tcl_NewObjectInstance(interp, clsPtr,
                Tcl_GetString(nameObjPtr), Tcl_GetString(nameObjPtr), 0,
	        NULL, 0);
    }
    if (oPtr == NULL) {
        Tcl_AppendResult(interp,
                "ITCL: cannot create Tcl_NewObjectInstance for class \"",
                Tcl_GetString(nameObjPtr), "\"", NULL);
       return TCL_ERROR;
    }
    Tcl_ObjectSetMetadata((Tcl_Object) oPtr, infoPtr->class_meta_type, iclsPtr);
    iclsPtr->clsPtr = Tcl_GetObjectAsClass(oPtr);
    iclsPtr->oPtr = oPtr;
    Tcl_ObjectSetMethodNameMapper(iclsPtr->oPtr, ItclngMapMethodNameProc);
    cmd = Tcl_GetObjectCommand(iclsPtr->oPtr);
    Tcl_GetCommandInfoFromToken(cmd, &cmdInfo);
    cmdInfo.deleteProc = ItclngDestroyClass;
    cmdInfo.deleteData = iclsPtr;
    Tcl_SetCommandInfoFromToken(cmd, &cmdInfo);
    classNs = Tcl_FindNamespace(interp, Tcl_GetString(nameObjPtr),
            (Tcl_Namespace*)NULL, /* flags */ 0);
    if (classNs == NULL) {
	Tcl_AppendResult(interp,
	        "ITCLNG: cannot get class namespace for class \"",
		Tcl_GetString(nameObjPtr), "\"", NULL);
        Tcl_Release((ClientData)iclsPtr);
        return TCL_ERROR;
    }
    if (_TclOONamespaceDeleteProc == NULL) {
        _TclOONamespaceDeleteProc = classNs->deleteProc;
    }
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, classNs->fullName, -1);
    Tcl_DStringAppend(&buffer, "::___DO_NOT_DELETE_THIS_VARIABLE", -1);
    Tcl_SetVar(interp, Tcl_DStringValue(&buffer), "1", 0);
    Tcl_TraceVar(interp, Tcl_DStringValue(&buffer), TCL_TRACE_UNSETS,
            ClassNamespaceDeleted, iclsPtr);

    Tcl_EventuallyFree((ClientData)iclsPtr, ItclngFreeClass);
    Itclng_SetNamespaceResolvers(classNs,
            (Tcl_ResolveCmdProc*)Itclng_ClassCmdResolver,
            (Tcl_ResolveVarProc*)Itclng_ClassVarResolver,
            (Tcl_ResolveCompiledVarProc*)Itclng_ClassCompiledVarResolver);
    iclsPtr->nsPtr = classNs;


    iclsPtr->namePtr = Tcl_NewStringObj(classNs->name, -1);
    Tcl_IncrRefCount(iclsPtr->namePtr);

    iclsPtr->fullNamePtr = Tcl_NewStringObj(classNs->fullName, -1);
    Tcl_IncrRefCount(iclsPtr->fullNamePtr);

    hPtr = Tcl_CreateHashEntry(&infoPtr->classes, (char *)iclsPtr->fullNamePtr,
            &newEntry);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp,
	        "ITCL: cannot create hash entry in infoPtr->classes for class \"",
		Tcl_GetString(iclsPtr->fullNamePtr), "\"", NULL);
	return TCL_ERROR;
    }
    Tcl_SetHashValue(hPtr, (ClientData)iclsPtr);

    hPtr = Tcl_CreateHashEntry(&infoPtr->namespaceClasses, (char *)classNs,
            &newEntry);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp,
	        "ITCLNG: cannot create hash entry in infoPtr->namespaceClasses",
		" for class \"", 
		Tcl_GetString(iclsPtr->fullNamePtr), "\"", NULL);
	return TCL_ERROR;
    }
    Tcl_SetHashValue(hPtr, (ClientData)iclsPtr);

    /*
     * now build the namespace for the common private and protected variables
     * public variables go directly to the class namespace
     */
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer,
            Tcl_GetString(iclsPtr->infoPtr->internalVars), -1);
    Tcl_DStringAppend(&buffer, Tcl_GetString(iclsPtr->fullNamePtr), -1);
    if (Tcl_CreateNamespace(interp, Tcl_DStringValue(&buffer),
            NULL, 0) == NULL) {
	Tcl_AppendResult(interp, "ITCLNG: cannot create variables namespace \"",
	Tcl_DStringValue(&buffer), "\"", NULL);
        return TCL_ERROR;
    }

    Tcl_DStringFree(&buffer);
    /*
     *  Add the built-in "this" variable to the list of data members.
     */
    namePtr = Tcl_NewStringObj("this", -1);
    Tcl_IncrRefCount(namePtr);
    if (ItclngCreateVariable(interp, iclsPtr, namePtr, &ivPtr) != TCL_OK) {
        return TCL_ERROR;
    }

    ivPtr->protection = ITCLNG_PROTECTED;  /* always "protected" */
    ivPtr->flags |= ITCLNG_THIS_VAR;       /* mark as "this" variable */

    hPtr = Tcl_CreateHashEntry(&iclsPtr->variables, (char *)namePtr, &newEntry);
    Tcl_SetHashValue(hPtr, (ClientData)ivPtr);

    /*
     *  Add the built-in "itclng_options" variable to the list of
     *  data members.
     */
    ivPtr = NULL;
    namePtr = Tcl_NewStringObj("itclng_options", -1);
    Tcl_IncrRefCount(namePtr);
    if (ItclngCreateVariable(interp, iclsPtr, namePtr, &ivPtr) != TCL_OK) {
        return TCL_ERROR;
    }
    ivPtr->protection = ITCLNG_PROTECTED;  /* always "protected" */
    ivPtr->flags |= ITCLNG_OPTIONS_VAR;    /* mark as "itclng_options"
	                                      * variable */
    
    hPtr = Tcl_CreateHashEntry(&iclsPtr->variables, (char *)namePtr,
            &newEntry);
    Tcl_SetHashValue(hPtr, (ClientData)ivPtr);
    /*
     *  Create a command in the current namespace to manage the class:
     *    <className>
     *    <className> <objName> ?<constructor-args>?
     */
    Tcl_Preserve((ClientData)iclsPtr);

    iclsPtr->accessCmd = Tcl_GetObjectCommand(oPtr);
    Tcl_TraceCommand(interp, Tcl_GetCommandName(interp, iclsPtr->accessCmd),
                TCL_TRACE_RENAME|TCL_TRACE_DELETE, ClassRenamedTrace, iclsPtr);
    /* FIX ME should set the class objects unknown command to Itclng_HandleClass */
    /*
     *  Push this class onto the class definition stack so that it
     *  becomes the current context for all commands in the parser.
     *  Activate the parser and evaluate the class definition.
     */
    Itclng_PushStack((ClientData)iclsPtr, &infoPtr->clsStack);

    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, Tcl_GetString(iclsPtr->fullNamePtr), NULL);
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  ItclngDeleteClassVariablesNamespace()
 *
 * ------------------------------------------------------------------------
 */
void
ItclngDeleteClassVariablesNamespace(
    Tcl_Interp *interp,
    ItclngClass *iclsPtr)
{
    Tcl_DString buffer;
    Tcl_Namespace *varNsPtr;

    if (iclsPtr->nsPtr == NULL) {
        return;
    }
    if (!(iclsPtr->flags & ITCLNG_CLASS_NO_VARNS_DELETE)) {
        /* free the classes's variables namespace and variables in it */
        Tcl_DStringInit(&buffer);
        Tcl_DStringAppend(&buffer,
	        Tcl_GetString(iclsPtr->infoPtr->internalVars), -1);
        Tcl_DStringAppend(&buffer, iclsPtr->nsPtr->fullName, -1);
        varNsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer),
	        NULL, 0);
        if (varNsPtr != NULL) {
            Tcl_DeleteNamespace(varNsPtr);
        }
        Tcl_DStringFree(&buffer);
        iclsPtr->nsPtr = NULL;
    } else {
        iclsPtr->flags |= ITCLNG_CLASS_SHOULD_VARNS_DELETE;
    }
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_DeleteClass()
 *
 *  Deletes a class by deleting all derived classes and all objects in
 *  that class, and finally, by destroying the class namespace.  This
 *  procedure provides a friendly way of doing this.  If any errors
 *  are detected along the way, the process is aborted.
 *
 *  Returns TCL_OK if successful, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */
int
Itclng_DeleteClass(
    Tcl_Interp *interp,     /* interpreter managing this class */
    ItclngClass *iclsPtr)    /* class */
{
    ItclngClass *iclsPtr2 = NULL;

    Itclng_ListElem *elem;
    ItclngObject *contextIoPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch place;
    Tcl_DString buffer;

    if (iclsPtr->flags & ITCLNG_CLASS_DELETE_CALLED) {
        return TCL_OK;
    }
    /*
     *  Destroy all derived classes, since these lose their meaning
     *  when the base class goes away.  If anything goes wrong,
     *  abort with an error.
     *
     *  TRICKY NOTE:  When a derived class is destroyed, it
     *    automatically deletes itself from the "derived" list.
     */
    elem = Itclng_FirstListElem(&iclsPtr->derived);
    while (elem) {
        iclsPtr2 = (ItclngClass*)Itclng_GetListValue(elem);
        elem = Itclng_NextListElem(elem);  /* advance here--elem will go away */

        if (Itclng_DeleteClass(interp, iclsPtr2) != TCL_OK) {
            goto deleteClassFail;
        }
    }

    /*
     *  Scan through and find all objects that belong to this class.
     *  Note that more specialized objects have already been
     *  destroyed above, when derived classes were destroyed.
     *  Destroy objects and report any errors.
     */
    hPtr = Tcl_FirstHashEntry(&iclsPtr->infoPtr->objects, &place);
    while (hPtr) {
        contextIoPtr = (ItclngObject*)Tcl_GetHashValue(hPtr);

        if (contextIoPtr->iclsPtr == iclsPtr) {
            if (Itclng_DeleteObject(interp, contextIoPtr) != TCL_OK) {
                iclsPtr2 = iclsPtr;
                goto deleteClassFail;
            }

	    /*
	     * Fix 227804: Whenever an object to delete was found we
	     * have to reset the search to the beginning as the
	     * current entry in the search was deleted and accessing it
	     * is therefore not allowed anymore.
	     */

	    hPtr = Tcl_FirstHashEntry(&iclsPtr->infoPtr->objects, &place);
	    continue;
        }

        hPtr = Tcl_NextHashEntry(&place);
    }

    /*
     *  Destroy the namespace associated with this class.
     *
     *  TRICKY NOTE:
     *    The cleanup procedure associated with the namespace is
     *    invoked automatically.  It does all of the same things
     *    above, but it also disconnects this class from its
     *    base-class lists, and removes the class access command.
     */
    if (iclsPtr->nsPtr != NULL) {
        Tcl_DeleteNamespace(iclsPtr->nsPtr);
        ItclngDeleteClassVariablesNamespace(interp, iclsPtr);
    }
    return TCL_OK;

deleteClassFail:
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, "\n    (while deleting class \"", -1);
    Tcl_DStringAppend(&buffer, iclsPtr2->nsPtr->fullName, -1);
    Tcl_DStringAppend(&buffer, "\")", -1);
    Tcl_AddErrorInfo(interp, Tcl_DStringValue(&buffer));
    Tcl_DStringFree(&buffer);
    return TCL_ERROR;
}


/*
 * ------------------------------------------------------------------------
 *  ItclngDestroyClass()
 *
 *  Invoked whenever the access command for a class is destroyed.
 *  Destroys the namespace associated with the class, which also
 *  destroys all objects in the class and all derived classes.
 *  Disconnects this class from the "derived" class lists of its
 *  base classes, and releases any claim to the class definition
 *  data.  If this is the last use of that data, the class will
 *  completely vanish at this point.
 * ------------------------------------------------------------------------
 */
static void
ItclngDestroyClass(
    ClientData cdata)  /* class definition to be destroyed */
{
    ItclngClass *iclsPtr = (ItclngClass*)cdata;

    if (iclsPtr->accessCmd == NULL) {
        return;
    }
    iclsPtr->accessCmd = NULL;
    if (iclsPtr->nsPtr != NULL) {
        if (iclsPtr->flags & ITCLNG_CLASS_DELETED) {
            Tcl_DeleteNamespace(iclsPtr->nsPtr);
            iclsPtr->nsPtr = NULL;
        }
    }
    Tcl_Release((ClientData)iclsPtr);
}


/*
 * ------------------------------------------------------------------------
 *  ItclngDestroyClassNamesp()
 *
 *  Invoked whenever the namespace associated with a class is destroyed.
 *  Destroys all objects associated with this class and all derived
 *  classes.  Disconnects this class from the "derived" class lists
 *  of its base classes, and removes the class access command.  Releases
 *  any claim to the class definition data.  If this is the last use
 *  of that data, the class will completely vanish at this point.
 * ------------------------------------------------------------------------
 */
static void
ItclngDestroyClassNamesp(
    ClientData cdata)  /* class definition to be destroyed */
{
    ItclngClass *iclsPtr = (ItclngClass*)cdata;
    ItclngObject *contextObj;
    Itclng_ListElem *elem;
    Itclng_ListElem *belem;
    ItclngClass *iclsPtr2;
    ItclngClass *basePtr;
    ItclngClass *derivedPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch place;

    /*
     *  Destroy all derived classes, since these lose their meaning
     *  when the base class goes away.
     *
     *  TRICKY NOTE:  When a derived class is destroyed, it
     *    automatically deletes itself from the "derived" list.
     */
    elem = Itclng_FirstListElem(&iclsPtr->derived);
    while (elem) {
        iclsPtr2 = (ItclngClass*)Itclng_GetListValue(elem);
	if (iclsPtr2->nsPtr != NULL) {
            Tcl_DeleteNamespace(iclsPtr2->nsPtr);
            iclsPtr2->nsPtr = NULL;
        }

	/* As the first namespace is now destroyed we have to get the
         * new first element of the hash table. We cannot go to the
         * next element from the current one, because the current one
         * is deleted. itcl Patch #593112, for Bug #577719.
	 */

        elem = Itclng_FirstListElem(&iclsPtr->derived);
    }

    /*
     *  Scan through and find all objects that belong to this class.
     *  Destroy them quietly by deleting their access command.
     */
    hPtr = Tcl_FirstHashEntry(&iclsPtr->infoPtr->objects, &place);
    while (hPtr) {
        contextObj = (ItclngObject*)Tcl_GetHashValue(hPtr);
        if (contextObj->iclsPtr == iclsPtr) {
            Tcl_DeleteCommandFromToken(iclsPtr->interp, contextObj->accessCmd);
	    contextObj->accessCmd = NULL;
	    /*
	     * Fix 227804: Whenever an object to delete was found we
	     * have to reset the search to the beginning as the
	     * current entry in the search was deleted and accessing it
	     * is therefore not allowed anymore.
	     */

	    hPtr = Tcl_FirstHashEntry(&iclsPtr->infoPtr->objects, &place);
	    continue;
        }
        hPtr = Tcl_NextHashEntry(&place);
    }

    /*
     *  Next, remove this class from the "derived" list in
     *  all base classes.
     */
    belem = Itclng_FirstListElem(&iclsPtr->bases);
    while (belem) {
        basePtr = (ItclngClass*)Itclng_GetListValue(belem);

        elem = Itclng_FirstListElem(&basePtr->derived);
        while (elem) {
            derivedPtr = (ItclngClass*)Itclng_GetListValue(elem);
            if (derivedPtr == iclsPtr) {
                Tcl_Release(Itclng_GetListValue(elem));
                elem = Itclng_DeleteListElem(elem);
            } else {
                elem = Itclng_NextListElem(elem);
            }
        }
        belem = Itclng_NextListElem(belem);
    }

    /*
     *  Next, destroy the access command associated with the class.
     */
    iclsPtr->flags |= ITCLNG_CLASS_NS_TEARDOWN;
    if (iclsPtr->accessCmd) {
        Tcl_CmdInfo cmdInfo;
	if (Tcl_GetCommandInfoFromToken(iclsPtr->accessCmd, &cmdInfo) == 1) {
	    if (cmdInfo.deleteProc != NULL) {
                Tcl_DeleteCommandFromToken(iclsPtr->interp, iclsPtr->accessCmd);
	    }
	    iclsPtr->accessCmd = NULL;
        }
        if (ItclngDeleteClassDictInfo(iclsPtr) != TCL_OK) {
fprintf(stderr, "INTERNAL ERROR!ItclngDeleteClassDictInfo!%s!\n", Tcl_GetStringResult(iclsPtr->interp));
	}
    }

    /*
     *  Release the namespace's claim on the class definition.
     */
    Tcl_Release((ClientData)iclsPtr);
}


/*
 * ------------------------------------------------------------------------
 *  ItclngFreeClass()
 *
 *  Frees all memory associated with a class definition.  This is
 *  usually invoked automatically by Tcl_Release(), when class
 *  data is no longer being used.
 * ------------------------------------------------------------------------
 */
static void
ItclngFreeClass(
    char *cdata)  /* class definition to be destroyed */
{
    ItclngClass *iclsPtr = (ItclngClass*)cdata;
    ItclngClass *iclsPtr2;
    ItclngMemberFunc *imPtr;
    ItclngVariable *ivPtr;
    ItclngOption *ioptPtr;
    ItclngComponent *icPtr;
    ItclngDelegatedOption *idoPtr;
    ItclngDelegatedFunction *idmPtr;
    Itclng_ListElem *elem;
    ItclngVarLookup *vlookup;
    FOREACH_HASH_DECLS;
    int found;
    found = 0;

    if (!(iclsPtr->flags & ITCLNG_CLASS_DELETE_CALLED)) {
	Tcl_Preserve(iclsPtr);
        return;
    }
    if (iclsPtr->flags & ITCLNG_CLASS_DELETED) {
        return;
    }
    iclsPtr->flags |= ITCLNG_CLASS_DELETED;

    /*
     *  Tear down the list of derived classes.  This list should
     *  really be empty if everything is working properly, but
     *  release it here just in case.
     */
    elem = Itclng_FirstListElem(&iclsPtr->derived);
    while (elem) {
        Tcl_Release( Itclng_GetListValue(elem) );
        elem = Itclng_NextListElem(elem);
    }
    Itclng_DeleteList(&iclsPtr->derived);

    /*
     *  Tear down the variable resolution table.  Some records
     *  appear multiple times in the table (for x, foo::x, etc.)
     *  so each one has a reference count.
     */
//    Tcl_InitHashTable(&varTable, TCL_STRING_KEYS);

    FOREACH_HASH_VALUE(vlookup, &iclsPtr->resolveVars) {
        if (--vlookup->usage == 0) {
            /*
             *  If this is a common variable owned by this class,
             *  then release the class's hold on it. FIX ME !!!
             */
            ckfree((char*)vlookup);
        }
    }

//    TclDeleteVars((Interp*)iclsPtr->interp, &varTable);
    Tcl_DeleteHashTable(&iclsPtr->resolveVars);

    /*
     *  Tear down the virtual method table...
     */
    Tcl_DeleteHashTable(&iclsPtr->resolveCmds);

    /*
     *  Delete all variable definitions.
     */
    FOREACH_HASH_VALUE(ivPtr, &iclsPtr->variables) {
        Itclng_DeleteVariable(ivPtr);
    }
    Tcl_DeleteHashTable(&iclsPtr->variables);

    /*
     *  Delete all option definitions.
     */
    FOREACH_HASH_VALUE(ioptPtr, &iclsPtr->options) {
        ItclngDeleteOption(ioptPtr);
    }
    Tcl_DeleteHashTable(&iclsPtr->options);

    /*
     *  Delete all components
     */
    FOREACH_HASH_VALUE(icPtr, &iclsPtr->components) {
        ItclngDeleteComponent(icPtr);
    }
    Tcl_DeleteHashTable(&iclsPtr->components);

    /*
     *  Delete all function definitions.
     */
    FOREACH_HASH_VALUE(imPtr, &iclsPtr->functions) {
        Tcl_Release(imPtr);
    }
    Tcl_DeleteHashTable(&iclsPtr->functions);

    /*
     *  Delete all delegated options.
     */
    FOREACH_HASH_VALUE(idoPtr, &iclsPtr->delegatedOptions) {
        Tcl_Release(idoPtr);
    }
    Tcl_DeleteHashTable(&iclsPtr->delegatedOptions);

    /*
     *  Delete all delegated functions.
     */
    FOREACH_HASH_VALUE(idoPtr, &iclsPtr->delegatedFunctions) {
        ItclngDeleteDelegatedFunction(idmPtr);
    }
    Tcl_DeleteHashTable(&iclsPtr->delegatedFunctions);

    /*
     *  Release the claim on all base classes.
     */
    elem = Itclng_FirstListElem(&iclsPtr->bases);
    while (elem) {
        Tcl_Release( Itclng_GetListValue(elem) );
        elem = Itclng_NextListElem(elem);
    }
    Itclng_DeleteList(&iclsPtr->bases);
    Tcl_DeleteHashTable(&iclsPtr->heritage);

    /* FIX ME !!!
      free classCommons
      free contextCache
      free resolvePtr
     */

    /*
     *  Free up the object initialization code.
     */
    if (iclsPtr->initCode) {
        Tcl_DecrRefCount(iclsPtr->initCode);
    }

    Tcl_Release((ClientData)iclsPtr->infoPtr);

    Tcl_DecrRefCount(iclsPtr->namePtr);
    Tcl_DecrRefCount(iclsPtr->fullNamePtr);

    FOREACH_HASH_VALUE(iclsPtr2, &iclsPtr->infoPtr->namespaceClasses) {
        if (iclsPtr2 == iclsPtr) {
	    Tcl_DeleteHashEntry(hPtr);
	}
    }
    ckfree((char*)iclsPtr);
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_IsClassNamespace()
 *
 *  Checks to see whether or not the given namespace represents an
 *  [incr Tcl] class.  Returns non-zero if so, and zero otherwise.
 * ------------------------------------------------------------------------
 */
int
Itclng_IsClassNamespace(
    Tcl_Namespace *nsPtr)  /* namespace being tested */
{
    if (nsPtr != NULL) {
        if (nsPtr->deleteProc == NULL) {
	   return 0;
	}
        return (nsPtr->deleteProc == _TclOONamespaceDeleteProc);
    }
    return 0;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_IsClass()
 *
 *  Checks the given Tcl command to see if it represents an itcl class.
 *  Returns non-zero if the command is associated with a class.
 * ------------------------------------------------------------------------
 */
int
Itclng_IsClass(
    Tcl_Command cmd)         /* command being tested */
{
    Tcl_CmdInfo cmdInfo;

    if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) == 0) {
        return 0;
    }
    if (cmdInfo.deleteProc == ItclngDestroyClass) {
        return 1;
    }

    /*
     *  This may be an imported command.  Try to get the real
     *  command and see if it represents a class.
     */
    cmd = Tcl_GetOriginalCommand(cmd);
    if (cmd != NULL) {
	if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) == 0) {
            return 0;
        }
        if (cmdInfo.deleteProc == ItclngDestroyClass) {
            return 1;
        }
    }
    return 0;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_FindClass()
 *
 *  Searches for the specified class in the active namespace.  If the
 *  class is found, this procedure returns a pointer to the class
 *  definition.  Otherwise, if the autoload flag is non-zero, an
 *  attempt will be made to autoload the class definition.  If it
 *  still can't be found, this procedure returns NULL, along with an
 *  error message in the interpreter.
 * ------------------------------------------------------------------------
 */
ItclngClass*
Itclng_FindClass(
    Tcl_Interp* interp,      /* interpreter containing class */
    CONST char* path,         /* path name for class */
    int autoload)
{
    Tcl_Namespace* classNs;

    /*
     *  Search for a namespace with the specified name, and if
     *  one is found, see if it is a class namespace.
     */
    classNs = Itclng_FindClassNamespace(interp, path);

    if (classNs && Itclng_IsClassNamespace(classNs)) {
	ItclngObjectInfo *infoPtr;
	infoPtr = Tcl_GetAssocData(interp, ITCLNG_INTERP_DATA, NULL);
        return (ItclngClass*)Tcl_ObjectGetMetadata(classNs->clientData,
	        infoPtr->class_meta_type);
    }

    /*
     *  If the autoload flag is set, try to autoload the class
     *  definition.
     */
    if (autoload) {
        if (Tcl_VarEval(interp, "::auto_load ", path, (char*)NULL) != TCL_OK) {
            char msg[256];
            sprintf(msg, "\n    (while attempting to autoload class \"%.200s\")", path);
            Tcl_AddErrorInfo(interp, msg);
            return NULL;
        }
        Tcl_ResetResult(interp);

        classNs = Itclng_FindClassNamespace(interp, path);
        if (classNs && Itclng_IsClassNamespace(classNs)) {
	    ItclngObjectInfo *infoPtr;
	    Tcl_HashEntry *hPtr;

	    infoPtr = Tcl_GetAssocData(interp, ITCLNG_INTERP_DATA, NULL);
	    hPtr = Tcl_FindHashEntry(&infoPtr->namespaceClasses, (char *)
	            classNs);
	    if (hPtr == NULL) {
	        char msg[256];
		sprintf(msg,
		        "\n    (while attempting to autoload class \"%.200s\")"
		        , path);
                Tcl_AddErrorInfo(interp, msg);
                return NULL;
	    }
	    return (ItclngClass *)Tcl_GetHashValue(hPtr);
        }
    }

    Tcl_AppendResult(interp, "class \"", path, "\" not found in context \"",
        Tcl_GetCurrentNamespace(interp)->fullName, "\"",
        (char*)NULL);

    return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_FindClassNamespace()
 *
 *  Searches for the specified class namespace.  The normal Tcl procedure
 *  Tcl_FindNamespace also searches for namespaces, but only in the
 *  current namespace context.  This makes it hard to find one class
 *  from within another.  For example, suppose. you have two namespaces
 *  Foo and Bar.  If you're in the context of Foo and you look for
 *  Bar, you won't find it with Tcl_FindNamespace.  This behavior is
 *  okay for namespaces, but wrong for classes.
 *
 *  This procedure search for a class namespace.  If the name is
 *  absolute (i.e., starts with "::"), then that one name is checked,
 *  and the class is either found or not.  But if the name is relative,
 *  it is sought in the current namespace context and in the global
 *  context, just like the normal command lookup.
 *
 *  This procedure returns a pointer to the desired namespace, or
 *  NULL if the namespace was not found.
 * ------------------------------------------------------------------------
 */
Tcl_Namespace*
Itclng_FindClassNamespace(
    Tcl_Interp* interp,        /* interpreter containing class */
    CONST char* path)                /* path name for class */
{
    Tcl_Namespace* contextNs = Tcl_GetCurrentNamespace(interp);
    Tcl_Namespace* classNs;
    Tcl_DString buffer;

    /*
     *  Look up the namespace.  If the name is not absolute, then
     *  see if it's the current namespace, and try the global
     *  namespace as well.
     */
    classNs = Tcl_FindNamespace(interp, (CONST84 char *)path,
	    (Tcl_Namespace*)NULL, /* flags */ 0);

    if ( !classNs && contextNs->parentPtr != NULL &&
         (*path != ':' || *(path+1) != ':') ) {

        if (strcmp(contextNs->name, path) == 0) {
            classNs = contextNs;
        }
        else {
            Tcl_DStringInit(&buffer);
            Tcl_DStringAppend(&buffer, "::", -1);
            Tcl_DStringAppend(&buffer, path, -1);

            classNs = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer),
                (Tcl_Namespace*)NULL, /* flags */ 0);

            Tcl_DStringFree(&buffer);
        }
    }
    return classNs;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateObjectCmd()
 *
 *  first argument is createObject 
 *  Invoked by Tcl whenever the user issues the command associated with
 *  a class name.  Handles the following syntax:
 *
 *    <className> <objName> ?<args>...?
 *
 *  If arguments are specified, then this procedure creates a new
 *  object named <objName> in the appropriate class.  Note that if
 *  <objName> contains "#auto", that part is automatically replaced
 *  by a unique string built from the class name.
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateObjectCmd(
    ClientData clientData,   /* class definition */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclngClass *iclsPtr;
    ItclngObjectInfo *infoPtr = (ItclngObjectInfo *)clientData;
    Tcl_HashEntry *hPtr;
    int result = TCL_OK;

    char unique[256];    /* buffer used for unique part of object names */
    Tcl_DString buffer;  /* buffer used to build object names */
    char *token;
    char *objName;
    char tmp;
    char *start;
    char *pos;
    char *match;

    ItclngShowArgs(1, "Itclng_CreateObjectCmd", objc, objv);
    /*
     *  If the command is invoked without an object name, then do nothing.
     *  This used to support autoloading--that the class name could be
     *  invoked as a command by itself, prompting the autoloader to
     *  load the class definition.  We retain the behavior here for
     *  backward-compatibility with earlier releases.
     */
    if (objc <= 2) {
        return TCL_OK;
    }

    hPtr = Tcl_FindHashEntry(&infoPtr->classes, (char *)objv[1]);
    if (hPtr == NULL) {
        Tcl_AppendResult(interp, "no such class: \"",
	        Tcl_GetString(objv[1]), "\"", NULL);
	return TCL_ERROR;
    }
    iclsPtr = Tcl_GetHashValue(hPtr);

    token = Tcl_GetString(objv[2]);
    /*
     *  we have a proper object name.  Create a new instance
     *  with that name.  If the name contains "#auto", replace this with
     *  a uniquely generated string based on the class name.
     */
    Tcl_DStringInit(&buffer);
    objName = NULL;

    match = "#auto";
    start = token;
    for (pos=start; *pos != '\0'; pos++) {
        if (*pos == *match) {
            if (*(++match) == '\0') {
                tmp = *start;
                *start = '\0';  /* null-terminate first part */

                /*
                 *  Substitute a unique part in for "#auto", and keep
                 *  incrementing a counter until a valid name is found.
                 */
                do {
		    Tcl_CmdInfo dummy;

                    sprintf(unique,"%.200s%d", Tcl_GetString(iclsPtr->namePtr),
                        iclsPtr->unique++);
                    unique[0] = tolower(unique[0]);

                    Tcl_DStringTrunc(&buffer, 0);
                    Tcl_DStringAppend(&buffer, token, -1);
                    Tcl_DStringAppend(&buffer, unique, -1);
                    Tcl_DStringAppend(&buffer, start+5, -1);

                    objName = Tcl_DStringValue(&buffer);

		    /*
		     * [Fix 227811] Check for any command with the
		     * given name, not only objects.
		     */

                    if (Tcl_GetCommandInfo (interp, objName, &dummy) == 0) {
                        break;  /* if an error is found, bail out! */
                    }
                } while (1);

                *start = tmp;       /* undo null-termination */
                objName = Tcl_DStringValue(&buffer);
                break;              /* object name is ready to go! */
            }
        }
        else {
            match = "#auto";
            pos = start++;
        }
    }

    /*
     *  If "#auto" was not found, then just use object name as-is.
     */
    if (objName == NULL) {
        objName = token;
    }

    /*
     *  Try to create a new object.  If successful, return the
     *  object name as the result of this command.
     */
    result = ItclngCreateObject(interp, objName, iclsPtr, objc-3, objv+3);

    if (result == TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(objName, -1));
    }

    Tcl_DStringFree(&buffer);
    if (result == TCL_ERROR) {
	Tcl_Obj *objPtr;
	
	(void)Tcl_GetReturnOptions(interp, result);
	objPtr = Tcl_NewStringObj("-level 2", -1);
	result = Tcl_SetReturnOptions(interp, objPtr);
    }
    return result;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_BuildVirtualTables()
 *
 *  Invoked whenever the class heritage changes or members are added or
 *  removed from a class definition to rebuild the member lookup
 *  tables.  There are two tables:
 *
 *  METHODS:  resolveCmds
 *    Used primarily in Itclng_ClassCmdResolver() to resolve all
 *    command references in a namespace.
 *
 *  DATA MEMBERS:  resolveVars
 *    Used primarily in Itclng_ClassVarResolver() to quickly resolve
 *    variable references in each class scope.
 *
 *  These tables store every possible name for each command/variable
 *  (member, class::member, namesp::class::member, etc.).  Members
 *  in a derived class may shadow members with the same name in a
 *  base class.  In that case, the simple name in the resolution
 *  table will point to the most-specific member.
 * ------------------------------------------------------------------------
 */
void
Itclng_BuildVirtualTables(
    ItclngClass* iclsPtr)       /* class definition being updated */
{
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch place;
    ItclngVarLookup *vlookup;
    ItclngVariable *ivPtr;
    ItclngMemberFunc *imPtr;
    ItclngHierIter hier;
    ItclngClass *iclsPtr2;
    Tcl_Namespace* nsPtr;
    Tcl_DString buffer, buffer2;
    int newEntry;

    Tcl_DStringInit(&buffer);
    Tcl_DStringInit(&buffer2);

    /*
     *  Clear the variable resolution table.
     */
    hPtr = Tcl_FirstHashEntry(&iclsPtr->resolveVars, &place);
    while (hPtr) {
        vlookup = (ItclngVarLookup*)Tcl_GetHashValue(hPtr);
        if (--vlookup->usage == 0) {
            ckfree((char*)vlookup);
        }
        hPtr = Tcl_NextHashEntry(&place);
    }
    Tcl_DeleteHashTable(&iclsPtr->resolveVars);
    Tcl_InitHashTable(&iclsPtr->resolveVars, TCL_STRING_KEYS);
    iclsPtr->numInstanceVars = 0;

    /*
     *  Set aside the first object-specific slot for the built-in
     *  "this" variable.  Only allocate one of these, even though
     *  there is a definition for "this" in each class scope.
     */
    iclsPtr->numInstanceVars++;

    /*
     *  Scan through all classes in the hierarchy, from most to
     *  least specific.  Add a lookup entry for each variable
     *  into the table.
     */
    Itclng_InitHierIter(&hier, iclsPtr);
    iclsPtr2 = Itclng_AdvanceHierIter(&hier);
    while (iclsPtr2 != NULL) {
        hPtr = Tcl_FirstHashEntry(&iclsPtr2->variables, &place);
        while (hPtr) {
            ivPtr = (ItclngVariable*)Tcl_GetHashValue(hPtr);

            vlookup = (ItclngVarLookup*)ckalloc(sizeof(ItclngVarLookup));
            vlookup->ivPtr = ivPtr;
            vlookup->usage = 0;
            vlookup->leastQualName = NULL;

            /*
             *  If this variable is PRIVATE to another class scope,
             *  then mark it as "inaccessible".
             */
            vlookup->accessible = (ivPtr->protection != ITCLNG_PRIVATE ||
	            ivPtr->iclsPtr == iclsPtr);

/* FIX ME !!! sould use for var lookup !! */
#ifdef NOTDEF
            /*
             *  If this is a common variable, then keep a reference to
             *  the variable directly.  Otherwise, keep an index into
             *  the object's variable table.
             */
            if ((ivPtr->flags & ITCLNG_COMMON) != 0) {
                nsPtr = (Namespace*)iclsPtr2->nsPtr;
                hPtr = Tcl_FindHashEntry(&nsPtr->varTable, ivPtr->name);
                assert(hPtr != NULL);

                vlookup->var.common = (Tcl_Var)Tcl_GetHashValue(hPtr);
            } else {
                /*
                 *  If this is a reference to the built-in "this"
                 *  variable, then its index is "0".  Otherwise,
                 *  add another slot to the end of the table.
                 */
                if ((ivPtr->flags & ITCLNG_THIS_VAR) != 0) {
                    vlookup->var.index = 0;
                }
                else {
                    vlookup->var.index = iclsPtr->numInstanceVars++;
                }
            }
#endif

            /*
             *  Create all possible names for this variable and enter
             *  them into the variable resolution table:
             *     var
             *     class::var
             *     namesp1::class::var
             *     namesp2::namesp1::class::var
             *     ...
             */
            Tcl_DStringSetLength(&buffer, 0);
            Tcl_DStringAppend(&buffer, Tcl_GetString(ivPtr->namePtr), -1);
            nsPtr = iclsPtr2->nsPtr;

            while (1) {
                hPtr = Tcl_CreateHashEntry(&iclsPtr->resolveVars,
                    Tcl_DStringValue(&buffer), &newEntry);

                if (newEntry) {
                    Tcl_SetHashValue(hPtr, (ClientData)vlookup);
                    vlookup->usage++;

                    if (!vlookup->leastQualName) {
                        vlookup->leastQualName =
                            Tcl_GetHashKey(&iclsPtr->resolveVars, hPtr);
                    }
                }

                if (nsPtr == NULL) {
                    break;
                }
                Tcl_DStringSetLength(&buffer2, 0);
                Tcl_DStringAppend(&buffer2, Tcl_DStringValue(&buffer), -1);
                Tcl_DStringSetLength(&buffer, 0);
                Tcl_DStringAppend(&buffer, nsPtr->name, -1);
                Tcl_DStringAppend(&buffer, "::", -1);
                Tcl_DStringAppend(&buffer, Tcl_DStringValue(&buffer2), -1);

                nsPtr = nsPtr->parentPtr;
            }

            /*
             *  If this record is not needed, free it now.
             */
            if (vlookup->usage == 0) {
                ckfree((char*)vlookup);
            }
            hPtr = Tcl_NextHashEntry(&place);
        }
        iclsPtr2 = Itclng_AdvanceHierIter(&hier);
    }
    Itclng_DeleteHierIter(&hier);

    /*
     *  Clear the command resolution table.
     */
    Tcl_DeleteHashTable(&iclsPtr->resolveCmds);
    Tcl_InitHashTable(&iclsPtr->resolveCmds, TCL_STRING_KEYS);

    /*
     *  Scan through all classes in the hierarchy, from most to
     *  least specific.  Look for the first (most-specific) definition
     *  of each member function, and enter it into the table.
     */
    Itclng_InitHierIter(&hier, iclsPtr);
    iclsPtr2 = Itclng_AdvanceHierIter(&hier);
    while (iclsPtr2 != NULL) {
        hPtr = Tcl_FirstHashEntry(&iclsPtr2->functions, &place);
        while (hPtr) {
            imPtr = (ItclngMemberFunc*)Tcl_GetHashValue(hPtr);
            /*
             *  Create all possible names for this function and enter
             *  them into the command resolution table:
             *     func
             *     class::func
             *     namesp1::class::func
             *     namesp2::namesp1::class::func
             *     ...
             */
            Tcl_DStringSetLength(&buffer, 0);
            Tcl_DStringAppend(&buffer, Tcl_GetString(imPtr->namePtr), -1);
            nsPtr = iclsPtr2->nsPtr;

            while (1) {
                hPtr = Tcl_CreateHashEntry(&iclsPtr->resolveCmds,
                    Tcl_DStringValue(&buffer), &newEntry);

                if (newEntry) {
                    Tcl_SetHashValue(hPtr, (ClientData)imPtr);
                }

                if (nsPtr == NULL) {
                    break;
                }
                Tcl_DStringSetLength(&buffer2, 0);
                Tcl_DStringAppend(&buffer2, Tcl_DStringValue(&buffer), -1);
                Tcl_DStringSetLength(&buffer, 0);
                Tcl_DStringAppend(&buffer, nsPtr->name, -1);
                Tcl_DStringAppend(&buffer, "::", -1);
                Tcl_DStringAppend(&buffer, Tcl_DStringValue(&buffer2), -1);

                nsPtr = nsPtr->parentPtr;
            }
            hPtr = Tcl_NextHashEntry(&place);
        }
        iclsPtr2 = Itclng_AdvanceHierIter(&hier);
    }
    Itclng_DeleteHierIter(&hier);

    Tcl_DStringFree(&buffer);
    Tcl_DStringFree(&buffer2);
}

/*
 * ------------------------------------------------------------------------
 *  ItclngCreateVariableMemberCode()
 *
 *  Creates the data record representing the implementation behind a
 *  class variable config code.
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
ItclngCreateVariableMemberCode(
    Tcl_Interp* interp,            /* interpreter managing this action */
    ItclngClass *iclsPtr,          /* class containing this member */
    const char *name,              /* name of variable */
    const char *config,            /* the config code */
    ItclngMemberCode** mcodePtr)   /* returns: pointer to new implementation */
{
    ItclngMemberCode *mcode;

    /*
     *  Allocate some space to hold the implementation.
     */
    mcode = (ItclngMemberCode*)ckalloc(sizeof(ItclngMemberCode));
    memset(mcode, 0, sizeof(ItclngMemberCode));
    mcode->argcount = 0;
    mcode->maxargcount = 0;
    if (config == NULL) {
        mcode->flags |= ITCLNG_IMPLEMENT_NONE;
    } else {
        mcode->flags |= ITCLNG_IMPLEMENT_TCL;
    }
    *mcodePtr = mcode;
    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  ItclngCreateVariable()
 *
 *  Creates a new class variable definition.  If this is a public
 *  variable, it may have a bit of "config" code that is used to
 *  update the object whenever the variable is modified via the
 *  built-in "configure" method.
 *
 *  Returns TCL_ERROR along with an error message in the specified
 *  interpreter if anything goes wrong.  Otherwise, this returns
 *  TCL_OK and a pointer to the new variable definition in "ivPtr".
 * ------------------------------------------------------------------------
 */
int
ItclngCreateVariable(
    Tcl_Interp *interp,       /* interpreter managing this transaction */
    ItclngClass* iclsPtr,       /* class containing this variable */
    Tcl_Obj* namePtr,         /* variable name */
    ItclngVariable** ivPtrPtr)  /* returns: new variable definition */
{
    Tcl_Obj *dictPtr;
    Tcl_Obj *valuePtr;
    Tcl_Obj *statePtr;
    Tcl_HashEntry *hPtr;
    ItclngVariable *ivPtr;
    ItclngMemberCode *mCodePtr;
    const char *stateStr;
    const char* init;       /* initial value */
    const char *name;
    int isSpecialVar;
    int newEntry;

    if (*ivPtrPtr != NULL) {
        ivPtrPtr = NULL;
    }
    init = NULL;
    mCodePtr = NULL;
    name = Tcl_GetString(namePtr);
    /*
     *  Add this variable to the variable table for the class.
     *  Make sure that the variable name does not already exist.
     */
    hPtr = Tcl_CreateHashEntry(&iclsPtr->variables, (char *)namePtr, &newEntry);
    if (!newEntry) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "variable name \"", Tcl_GetString(namePtr),
	    "\" already defined in class \"",
            Tcl_GetString(iclsPtr->fullNamePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }
    Tcl_IncrRefCount(namePtr);

    isSpecialVar = 0;
    if (strcmp (name, "this") == 0) {
        isSpecialVar = 1;
    }
    if (strcmp (name, "itclng_options") == 0) {
        isSpecialVar = 1;
    }
    if (!isSpecialVar) {
        statePtr = ItclngGetVariableStateString(iclsPtr, name);
        if (statePtr == NULL) {
            Tcl_AppendResult(interp, "cannot get state string", NULL);
            return TCL_ERROR;
        }
        stateStr = Tcl_GetString(statePtr);
        dictPtr = ItclngGetClassDictInfo(iclsPtr, "variables", name);
        if (dictPtr == NULL) {
            Tcl_AppendResult(interp, "cannot get variables info", NULL);
            return TCL_ERROR;
        }
    
        /*
         *  If this variable has some "config" code, try to capture
         *  its implementation.
         */
        if (strcmp(stateStr, "COMPLETE") == 0) {
            valuePtr = ItclngGetDictValueInfo(interp, dictPtr, "config");
            if (valuePtr == NULL) {
                Tcl_AppendResult(interp, "cannot get variable config", NULL);
                return TCL_ERROR;
            }
            if (ItclngCreateVariableMemberCode(interp, iclsPtr, (char*)NULL,
	            Tcl_GetString(valuePtr), &mCodePtr) != TCL_OK) {
                Tcl_DeleteHashEntry(hPtr);
	        Tcl_DecrRefCount(valuePtr);
                return TCL_ERROR;
            }
            Tcl_DecrRefCount(valuePtr);
            Tcl_Preserve((ClientData)mCodePtr);
            Tcl_EventuallyFree((ClientData)mCodePtr, Itclng_DeleteMemberCode);
        } else {
            if (strcmp(stateStr, "NO_CONFIG") == 0) {
                valuePtr = ItclngGetDictValueInfo(interp, dictPtr, "init");
                if (valuePtr == NULL) {
                    Tcl_AppendResult(interp, "cannot get variable config", NULL);
                    return TCL_ERROR;
                }
	        init = Tcl_GetString(valuePtr);
	    }
            mCodePtr = NULL;
        }
        Tcl_DecrRefCount(dictPtr);
    }
        

    /*
     *  If everything looks good, create the variable definition.
     */
    ivPtr = (ItclngVariable*)ckalloc(sizeof(ItclngVariable));
    memset(ivPtr, 0, sizeof(ItclngVariable));
    ivPtr->iclsPtr      = iclsPtr;
    ivPtr->protection   = Itclng_Protection(interp, 0);
    ivPtr->codePtr      = mCodePtr;
    ivPtr->namePtr      = namePtr;
    Tcl_IncrRefCount(ivPtr->namePtr);
    ivPtr->fullNamePtr = Tcl_NewStringObj(
            Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_AppendToObj(ivPtr->fullNamePtr, "::", 2);
    Tcl_AppendToObj(ivPtr->fullNamePtr, Tcl_GetString(namePtr), -1);
    Tcl_IncrRefCount(ivPtr->fullNamePtr);

    if (init) {
        ivPtr->init = Tcl_NewStringObj(init, -1);
        Tcl_IncrRefCount(ivPtr->init);
        Tcl_DecrRefCount(valuePtr);
    } else {
        ivPtr->init = NULL;
    }

    Tcl_SetHashValue(hPtr, (ClientData)ivPtr);

    *ivPtrPtr = ivPtr;
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclngCreateCommonOrVariable()
 *
 *  Installs a common/variable into the namespace associated with a class.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error message
 *  in the specified interp) if anything goes wrong.
 * ------------------------------------------------------------------------
 */
int
ItclngCreateCommonOrVariable(
    Tcl_Interp* interp,    /* interpreter managing this action */
    ItclngClass *iclsPtr,  /* class definition */
    Tcl_Obj *namePtr,      /* name of new common/variable */
    int flags)             /* whether this is a common or variable */
{
    ItclngVariable *ivPtr;

    ivPtr = NULL;
    /*
     *  Create the common/variable definition.
     */
    if (ItclngCreateVariable(interp, iclsPtr, namePtr, &ivPtr)
        != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Mark commons as "common".  This distinguishes them from variables.
     */
    ivPtr->flags |= flags;
    if (flags & ITCLNG_COMMON) {
	Tcl_HashEntry *hPtr;
	Tcl_Var varPtr;
	Tcl_Namespace *commonNsPtr;
	Tcl_DString buffer;
	Tcl_CallFrame frame;
	int result;
	int isNew;

	iclsPtr->numCommons++;
        /*
         *  Create the variable in the namespace associated with the
         *  class.  Do this the hard way, to avoid the variable resolver
         *  procedures.  These procedures won't work until we rebuild
         *  the virtual tables below.
         */
        Tcl_DStringInit(&buffer);
        if (ivPtr->protection != ITCLNG_PUBLIC) {
            /* public commons go to the class namespace directly the others
	     * go to the variables namespace of the class */
            Tcl_DStringAppend(&buffer,
	            Tcl_GetString(ivPtr->iclsPtr->infoPtr->internalVars), -1);
        }
        Tcl_DStringAppend(&buffer,
	        Tcl_GetString(ivPtr->iclsPtr->fullNamePtr), -1);
        commonNsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer),
	        NULL, 0);
        if (commonNsPtr == NULL) {
            Tcl_AppendResult(interp, "ITCLNG: cannot find common variables",
	            "namespace for class \"",
		    Tcl_GetString(ivPtr->iclsPtr->fullNamePtr),
		    "\"", NULL);
	    return TCL_ERROR;
        }
        varPtr = Tcl_NewNamespaceVar(interp, commonNsPtr,
                Tcl_GetString(ivPtr->namePtr));
        hPtr = Tcl_CreateHashEntry(&iclsPtr->classCommons, (char *)ivPtr,
                &isNew);
        if (isNew) {
            Tcl_SetHashValue(hPtr, varPtr);
        }
        result = Itclng_PushCallFrame(interp, &frame, commonNsPtr,
            /* isProcCallFrame */ 0);
        ItclngVarTraceInfo *traceInfoPtr;
        traceInfoPtr = (ItclngVarTraceInfo *)ckalloc(sizeof(ItclngVarTraceInfo));
        memset (traceInfoPtr, 0, sizeof(ItclngVarTraceInfo));
        traceInfoPtr->flags = ITCLNG_TRACE_CLASS;
        traceInfoPtr->ioPtr = NULL;
        traceInfoPtr->iclsPtr = ivPtr->iclsPtr;
        traceInfoPtr->ivPtr = ivPtr;
        Tcl_TraceVar2(interp, Tcl_GetString(ivPtr->namePtr), NULL,
               TCL_TRACE_UNSETS, ItclngTraceUnsetVar,
               (ClientData)traceInfoPtr);
        Itclng_PopCallFrame(interp);

        /*
         *  TRICKY NOTE:  Make sure to rebuild the virtual tables for this
         *    class so that this variable is ready to access.  The variable
         *    resolver for the parser namespace needs this info to find the
         *    variable if the developer tries to set it within the class
         *    definition.
         *
         *  If an initialization value was specified, then initialize
         *  the variable now.
         */
        Itclng_BuildVirtualTables(iclsPtr);
    
        if (ivPtr->init) {
            Tcl_DStringAppend(&buffer, "::", -1);
            Tcl_DStringAppend(&buffer, Tcl_GetString(ivPtr->namePtr), -1);
            CONST char *val = Tcl_SetVar(interp,
	            Tcl_DStringValue(&buffer), Tcl_GetString(ivPtr->init),
                    TCL_NAMESPACE_ONLY);
    
            if (!val) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "cannot initialize common variable \"",
                    Tcl_GetString(ivPtr->namePtr), "\"",
                    (char*)NULL);
                return TCL_ERROR;
            }
        }
        Tcl_DStringFree(&buffer);
    } else {
	iclsPtr->numVariables++;
    }

    Tcl_Preserve((ClientData)ivPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateOption()
 *
 *  Creates a new class option definition.  If this is a public
 *  option, it may have a bit of "config" code that is used to
 *  update the object whenever the option is modified via the
 *  built-in "configure" method.
 *
 *  Returns TCL_ERROR along with an error message in the specified
 *  interpreter if anything goes wrong.  Otherwise, this returns
 *  TCL_OK and a pointer to the new option definition in "ioptPtr".
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateOption(
    Tcl_Interp *interp,       /* interpreter managing this transaction */
    ItclngClass* iclsPtr,       /* class containing this variable */
    ItclngOption* ioptPtr)      /* new option definition */
{
    int newEntry;
    ItclngMemberCode *mCodePtr;
    Tcl_HashEntry *hPtr;

    mCodePtr = NULL;
    /*
     *  Add this option to the options table for the class.
     *  Make sure that the option name does not already exist.
     */
    hPtr = Tcl_CreateHashEntry(&iclsPtr->options,
            (char *)ioptPtr->namePtr, &newEntry);
    if (!newEntry) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "option name \"", Tcl_GetString(ioptPtr->namePtr),
	    "\" already defined in class \"",
            Tcl_GetString(iclsPtr->fullNamePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }

    ioptPtr->iclsPtr = iclsPtr;
    ioptPtr->codePtr = mCodePtr;
    ioptPtr->fullNamePtr = Tcl_NewStringObj(
            Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_AppendToObj(ioptPtr->fullNamePtr, "::", 2);
    Tcl_AppendToObj(ioptPtr->fullNamePtr, Tcl_GetString(ioptPtr->namePtr), -1);
    Tcl_IncrRefCount(ioptPtr->fullNamePtr);
    Tcl_SetHashValue(hPtr, (ClientData)ioptPtr);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_CreateMethodVariable()
 *
 *  Creates a new class methdovariable definition.  If this is a public
 *  methodvariable, 
 *
 *  Returns TCL_ERROR along with an error message in the specified
 *  interpreter if anything goes wrong.  Otherwise, this returns
 *  TCL_OK and a pointer to the new option definition in "imvPtr".
 * ------------------------------------------------------------------------
 */
int
Itclng_CreateMethodVariable(
    Tcl_Interp *interp,       /* interpreter managing this transaction */
    ItclngClass* iclsPtr,       /* class containing this variable */
    Tcl_Obj* namePtr,         /* variable name */
    Tcl_Obj* defaultPtr,      /* initial value */
    Tcl_Obj* callbackPtr,     /* code invoked when variable is set */
    ItclngMethodVariable** imvPtrPtr)
                              /* returns: new methdovariable definition */
{
    int isNew;
    ItclngMethodVariable *imvPtr;
    Tcl_HashEntry *hPtr;

    /*
     *  Add this methodvariable to the options table for the class.
     *  Make sure that the methodvariable name does not already exist.
     */
    hPtr = Tcl_CreateHashEntry(&iclsPtr->methodVariables,
            (char *)namePtr, &isNew);
    if (!isNew) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "methdovariable name \"", Tcl_GetString(namePtr),
	    "\" already defined in class \"",
            Tcl_GetString (iclsPtr->fullNamePtr), "\"",
            (char*)NULL);
        return TCL_ERROR;
    }
    Tcl_IncrRefCount(namePtr);

    /*
     *  If everything looks good, create the option definition.
     */
    imvPtr = (ItclngMethodVariable*)ckalloc(sizeof(ItclngMethodVariable));
    memset(imvPtr, 0, sizeof(ItclngMethodVariable));
    imvPtr->iclsPtr      = iclsPtr;
    imvPtr->protection   = Itclng_Protection(interp, 0);
    imvPtr->namePtr      = namePtr;
    Tcl_IncrRefCount(imvPtr->namePtr);
    imvPtr->fullNamePtr = Tcl_NewStringObj(
            Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_AppendToObj(imvPtr->fullNamePtr, "::", 2);
    Tcl_AppendToObj(imvPtr->fullNamePtr, Tcl_GetString(namePtr), -1);
    Tcl_IncrRefCount(imvPtr->fullNamePtr);
    imvPtr->defaultValuePtr    = defaultPtr;
    if (defaultPtr != NULL) {
        Tcl_IncrRefCount(imvPtr->defaultValuePtr);
    }
    imvPtr->callbackPtr    = callbackPtr;
    if (callbackPtr != NULL) {
        Tcl_IncrRefCount(imvPtr->callbackPtr);
    }

    Tcl_SetHashValue(hPtr, (ClientData)imvPtr);

    *imvPtrPtr = imvPtr;
    return TCL_OK;
}



/*
 * ------------------------------------------------------------------------
 *  Itclng_GetCommonVar()
 *
 *  Returns the current value for a common class variable.  The member
 *  name is interpreted with respect to the given class scope.  That
 *  scope is installed as the current context before querying the
 *  variable.  This by-passes the protection level in case the variable
 *  is "private".
 *
 *  If successful, this procedure returns a pointer to a string value
 *  which remains alive until the variable changes it value.  If
 *  anything goes wrong, this returns NULL.
 * ------------------------------------------------------------------------
 */
CONST char*
Itclng_GetCommonVar(
    Tcl_Interp *interp,        /* current interpreter */
    CONST char *name,          /* name of desired instance variable */
    ItclngClass *contextIclsPtr) /* name is interpreted in this scope */
{
    CONST char *val = NULL;
    Tcl_HashEntry *hPtr;
    Tcl_DString buffer;
    Tcl_Obj *namePtr;
    ItclngVariable *ivPtr;
    const char *cp;
    const char *lastCp;

    lastCp = name;
    cp = name;
    while (cp != NULL) {
        cp = strstr(lastCp, "::");
        if (cp != NULL) {
	    lastCp = cp + 2;
	}
    }
    namePtr = Tcl_NewStringObj(lastCp, -1);
    Tcl_IncrRefCount(namePtr);
    hPtr = Tcl_FindHashEntry(&contextIclsPtr->variables, (char *)namePtr);
    Tcl_DecrRefCount(namePtr);
    if (hPtr == NULL) {
        return NULL;
    }
    ivPtr = Tcl_GetHashValue(hPtr);
    /*
     *  Activate the namespace for the given class.  That installs
     *  the appropriate name resolution rules and by-passes any
     *  security restrictions.
     */
    Tcl_DStringInit(&buffer);
    if (ivPtr->protection != ITCLNG_PUBLIC) {
        Tcl_DStringAppend(&buffer,
	        Tcl_GetString(ivPtr->iclsPtr->infoPtr->internalVars), -1);
    }
    Tcl_DStringAppend(&buffer, name, -1);

    val = Tcl_GetVar2(interp, (CONST84 char *)Tcl_DStringValue(&buffer),
            (char*)NULL, 0);
    Tcl_DStringFree(&buffer);
    return val;
}


/*
 * ------------------------------------------------------------------------
 *  Itclng_InitHierIter()
 *
 *  Initializes an iterator for traversing the hierarchy of the given
 *  class.  Subsequent calls to Itclng_AdvanceHierIter() will return
 *  the base classes in order from most-to-least specific.
 * ------------------------------------------------------------------------
 */
void
Itclng_InitHierIter(
    ItclngHierIter *iter,   /* iterator used for traversal */
    ItclngClass *iclsPtr)     /* class definition for start of traversal */
{
    Itclng_InitStack(&iter->stack);
    Itclng_PushStack((ClientData)iclsPtr, &iter->stack);
    iter->current = iclsPtr;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_DeleteHierIter()
 *
 *  Destroys an iterator for traversing class hierarchies, freeing
 *  all memory associated with it.
 * ------------------------------------------------------------------------
 */
void
Itclng_DeleteHierIter(
    ItclngHierIter *iter)  /* iterator used for traversal */
{
    Itclng_DeleteStack(&iter->stack);
    iter->current = NULL;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_AdvanceHierIter()
 *
 *  Moves a class hierarchy iterator forward to the next base class.
 *  Returns a pointer to the current class definition, or NULL when
 *  the end of the hierarchy has been reached.
 * ------------------------------------------------------------------------
 */
ItclngClass*
Itclng_AdvanceHierIter(
    ItclngHierIter *iter)  /* iterator used for traversal */
{
    register Itclng_ListElem *elem;
    ItclngClass *iclsPtr;

    iter->current = (ItclngClass*)Itclng_PopStack(&iter->stack);

    /*
     *  Push classes onto the stack in reverse order, so that
     *  they will be popped off in the proper order.
     */
    if (iter->current) {
        iclsPtr = (ItclngClass*)iter->current;
        elem = Itclng_LastListElem(&iclsPtr->bases);
        while (elem) {
            Itclng_PushStack(Itclng_GetListValue(elem), &iter->stack);
            elem = Itclng_PrevListElem(elem);
        }
    }
    return iter->current;
}

/*
 * ------------------------------------------------------------------------
 *  Itclng_DeleteVariable()
 *
 *  Destroys a variable definition created by ItclngCreateVariable(),
 *  freeing all resources associated with it.
 * ------------------------------------------------------------------------
 */
void
Itclng_DeleteVariable(
    ItclngVariable *ivPtr)   /* variable definition to be destroyed */
{
    Tcl_DecrRefCount(ivPtr->namePtr);
    Tcl_DecrRefCount(ivPtr->fullNamePtr);

    if (ivPtr->codePtr != NULL) {
        Tcl_Release((ClientData)ivPtr->codePtr);
    }
    if (ivPtr->init) {
        Tcl_DecrRefCount(ivPtr->init);
    }
    ckfree((char*)ivPtr);
}

/*
 * ------------------------------------------------------------------------
 *  ItclngDeleteComponent()
 *
 *  free data associated with a component
 * ------------------------------------------------------------------------
 */
static void
ItclngDeleteComponent(
    ItclngComponent *icPtr)
{
    Tcl_DecrRefCount(icPtr->namePtr);
    /* the variable and the command are freed when freeing variables,
     * functions */
    ckfree((char*)icPtr);
}

/*
 * ------------------------------------------------------------------------
 *  ItclngDeleteOption()
 *
 *  free data associated with an option
 * ------------------------------------------------------------------------
 */
static void
ItclngDeleteOption(
    ItclngOption *ioptPtr)
{
    Tcl_DecrRefCount(ioptPtr->namePtr);
    Tcl_DecrRefCount(ioptPtr->fullNamePtr);
    Tcl_DecrRefCount(ioptPtr->resourceNamePtr);
    Tcl_DecrRefCount(ioptPtr->classNamePtr);
    if (ioptPtr->codePtr != NULL) {
        Tcl_Release((ClientData)ioptPtr->codePtr);
    }
    if (ioptPtr->defaultValuePtr != NULL) {
        Tcl_DecrRefCount(ioptPtr->defaultValuePtr);
    }
    if (ioptPtr->defaultValuePtr != NULL) {
        Tcl_DecrRefCount(ioptPtr->cgetMethodPtr);
    }
    if (ioptPtr->configureMethodPtr != NULL) {
        Tcl_DecrRefCount(ioptPtr->configureMethodPtr);
    }
    if (ioptPtr->validateMethodPtr != NULL) {
        Tcl_DecrRefCount(ioptPtr->validateMethodPtr);
    }
    ckfree((char*)ioptPtr);
}
#ifdef NOTDEF

/*
 * ------------------------------------------------------------------------
 *  ItclngDeleteDelegatedOption()
 *
 *  free data associated with a delegated option
 * ------------------------------------------------------------------------
 */
static void
ItclngDeleteDelegatedOption(
    ItclngDelegatedOption *idoPtr)
{
    Tcl_Obj *objPtr;
    FOREACH_HASH_DECLS;

    Tcl_DecrRefCount(idoPtr->namePtr);
    Tcl_DecrRefCount(idoPtr->resourceNamePtr);
    Tcl_DecrRefCount(idoPtr->classNamePtr);
    if (idoPtr->asPtr != NULL) {
        Tcl_DecrRefCount(idoPtr->asPtr);
    }
    FOREACH_HASH_VALUE(objPtr, &idoPtr->exceptions) {
        Tcl_DecrRefCount(objPtr);
    }
    Tcl_DeleteHashTable(&idoPtr->exceptions);
    ckfree((char *)idoPtr);
}
#endif

/*
 * ------------------------------------------------------------------------
 *  ItclngDeleteDelegatedFunction()
 *
 *  free data associated with a delegated function
 * ------------------------------------------------------------------------
 */
static void ItclngDeleteDelegatedFunction(
    ItclngDelegatedFunction *idmPtr)
{
    Tcl_Obj *objPtr;
    FOREACH_HASH_DECLS;

    Tcl_DecrRefCount(idmPtr->namePtr);
    if (idmPtr->asPtr != NULL) {
        Tcl_DecrRefCount(idmPtr->asPtr);
    }
    if (idmPtr->usingPtr != NULL) {
        Tcl_DecrRefCount(idmPtr->usingPtr);
    }
    FOREACH_HASH_VALUE(objPtr, &idmPtr->exceptions) {
        Tcl_DecrRefCount(objPtr);
    }
    Tcl_DeleteHashTable(&idmPtr->exceptions);
    ckfree((char *)idmPtr);
}
