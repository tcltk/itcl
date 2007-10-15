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
 *  This segment handles "objects" which are instantiated from class
 *  definitions.  Objects contain public/protected/private data members
 *  from all classes in a derivation hierarchy.
 *
 * ========================================================================
 *  AUTHOR:  Michael J. McLennan
 *           Bell Labs Innovations for Lucent Technologies
 *           mmclennan@lucent.com
 *           http://www.tcltk.com/itcl
 *
 *  overhauled version author: Arnulf Wiedemann Copyright (c) 2007
 *
 *     RCS:  $Id: itclObject.c,v 1.1.2.19 2007/10/15 09:22:59 wiede Exp $
 * ========================================================================
 *           Copyright (c) 1993-1998  Lucent Technologies, Inc.
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"

/*
 *  FORWARD DECLARATIONS
 */
static char* ItclTraceThisVar(ClientData cdata, Tcl_Interp *interp,
	CONST char *name1, CONST char *name2, int flags);
static char* ItclTraceOptionVar(ClientData cdata, Tcl_Interp *interp,
	CONST char *name1, CONST char *name2, int flags);

static void ItclDestroyObject(ClientData clientData);
static void ItclFreeObject(char * clientData);

static int ItclDestructBase(Tcl_Interp *interp, ItclObject *contextObj,
        ItclClass *contextClass, int flags);

static int ItclInitObjectVariables(Tcl_Interp *interp, ItclObject *ioPtr,
        ItclClass *iclsPtr, const char *name);
static int ItclInitExtendedClassOptions(Tcl_Interp *interp, ItclObject *ioPtr);


/*
 * ------------------------------------------------------------------------
 *  ItclDeleteObjectMetadata()
 *
 *  Delete the metadata data if any
 *-------------------------------------------------------------------------
 */
void
ItclDeleteObjectMetadata(
    ClientData clientData)
{
    /*
     * nothing to to yet, as there are only ItclClass or ItclObject pointers
     * stored, which are freed elsewhere
     */
}

/*
 * ------------------------------------------------------------------------
 *  ObjectRenamedTrace()
 *
 * ------------------------------------------------------------------------
 */

static void
ObjectRenamedTrace(
    ClientData clientData,      /* The object being deleted. */
    Tcl_Interp *interp,         /* The interpreter containing the object. */
    const char *oldName,        /* What the object was (last) called. */
    const char *newName,        /* Always NULL ??. not for itk!! */
    int flags)                  /* Why was the object deleted? */
{
    ItclObject *ioPtr = clientData;

    if (newName != NULL) {
        return;
    }
    ioPtr->flags |= ITCL_OBJECT_IS_RENAMED;
    if (ioPtr->flags & ITCL_TCLOO_OBJECT_IS_DELETED) {
        ioPtr->oPtr = NULL;
    }
    if (!(ioPtr->flags & (ITCL_OBJECT_IS_DELETED|ITCL_OBJECT_IS_DESTRUCTED))) {
        ItclDestroyObject(ioPtr);
    }
}

/*
 * ------------------------------------------------------------------------
 *  ItclCreateObject()
 *
 *  Creates a new object instance belonging to the given class.
 *  Supports complex object names like "namesp::namesp::name" by
 *  following the namespace path and creating the object in the
 *  desired namespace.
 *
 *  Automatically creates and initializes data members, including the
 *  built-in protected "this" variable containing the object name.
 *  Installs an access command in the current namespace, and invokes
 *  the constructor to initialize the object.
 *
 *  If any errors are encountered, the object is destroyed and this
 *  procedure returns TCL_ERROR (along with an error message in the
 *  interpreter).  Otherwise, it returns TCL_OK
 * ------------------------------------------------------------------------
 */
int
ItclCreateObject(
    Tcl_Interp *interp,      /* interpreter mananging new object */
    CONST char* name,        /* name of new object */
    ItclClass *iclsPtr,        /* class for new object */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int result = TCL_OK;

    Tcl_DString buffer;
    Tcl_CmdInfo cmdInfo;
    Tcl_HashEntry *entry;
    ItclObjectInfo *infoPtr;
    ItclObject *ioPtr;
    Itcl_InterpState istate;
    Tcl_Obj **newObjv;
    int newObjc;
    int newEntry;

    /* just init for the cas of none ItclWidget objects */
    newObjc = objc;
    newObjv = (Tcl_Obj **)objv;
    /*
     *  Create a new object and initialize it.
     */
    ioPtr = (ItclObject*)ckalloc(sizeof(ItclObject));
    memset(ioPtr, 0, sizeof(ItclObject));
    ioPtr->iclsPtr = iclsPtr;
    Itcl_PreserveData((ClientData)iclsPtr);

    ioPtr->constructed = (Tcl_HashTable*)ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitObjHashTable(ioPtr->constructed);

    /*
     *  Add a command to the current namespace with the object name.
     *  This is done before invoking the constructors so that the
     *  command can be used during construction to query info.
     */
    Itcl_PreserveData((ClientData)ioPtr);

    ioPtr->namePtr = Tcl_NewStringObj(name, -1);
    Tcl_IncrRefCount(ioPtr->namePtr);
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
    Tcl_DStringAppend(&buffer, "::", 2);
    Tcl_DStringAppend(&buffer, Tcl_GetString(ioPtr->namePtr), -1);
    ioPtr->varNsNamePtr = Tcl_NewStringObj(Tcl_DStringValue(&buffer), -1);
    Tcl_IncrRefCount(ioPtr->varNsNamePtr);
    Tcl_DStringFree(&buffer);

    Tcl_InitHashTable(&ioPtr->objectVariables, TCL_ONE_WORD_KEYS);
    Tcl_InitObjHashTable(&ioPtr->objectOptions);
    Tcl_InitObjHashTable(&ioPtr->objectDelegatedOptions);
    Tcl_InitObjHashTable(&ioPtr->objectDelegatedFunctions);
    Tcl_InitHashTable(&ioPtr->contextCache, TCL_ONE_WORD_KEYS);

    Itcl_PreserveData((ClientData)ioPtr);  /* while we're using this... */
    Itcl_EventuallyFree((ClientData)ioPtr, ItclFreeObject);

    /*
     *  Install the class namespace and object context so that
     *  the object's data members can be initialized via simple
     *  "set" commands.
     */

    /* first create the object's class variables namespaces
     * and set all the init values for variables
     */

    if (ItclInitObjectVariables(interp, ioPtr, iclsPtr, name) != TCL_OK) {
	Tcl_AppendResult(interp, "error in ItclInitObjectVariables", NULL);
        return TCL_ERROR;
    }
    if (iclsPtr->flags & (ITCL_ECLASS|ITCL_NWIDGET)) {
        ItclInitExtendedClassOptions(interp, ioPtr);
        if (ItclInitObjectOptions(interp, ioPtr, iclsPtr, name) != TCL_OK) {
	    Tcl_AppendResult(interp, "error in ItclInitObjectOptions", NULL);
            return TCL_ERROR;
        }
    }

    infoPtr = iclsPtr->infoPtr;
    infoPtr->currIoPtr = ioPtr;
    if (infoPtr->windgetInfoPtr != NULL) {
        if (iclsPtr->flags & (ITCL_WIDGET|ITCL_WIDGETADAPTOR)) {
            /* 
             * set all the init values for options
             */

	    if (infoPtr->windgetInfoPtr->initObjectOpts != NULL) {
	        if (infoPtr->windgetInfoPtr->initObjectOpts(interp, ioPtr,
		        iclsPtr, name)  != TCL_OK) {
	            Tcl_AppendResult(interp, "error in ItclInitObjectOptions",
		            NULL);
                    return TCL_ERROR;
                }
            }
        }
        if (iclsPtr->flags & ITCL_WIDGET) {
	    if (infoPtr->windgetInfoPtr->hullAndOptsInst != NULL) {
                if (infoPtr->windgetInfoPtr->hullAndOptsInst(interp, ioPtr,
		        iclsPtr, objc, objv, &newObjc, newObjv) != TCL_OK) {
	            return TCL_ERROR;
	        }
	    }
        }
    }
    ioPtr->oPtr = Tcl_NewObjectInstance(interp, iclsPtr->clsPtr, name,
            iclsPtr->nsPtr->fullName, /* objc */-1, NULL, /* skip */0);
    if (ioPtr->oPtr == NULL) {
	// NEED TO FREE STUFF HERE !! 
        return TCL_ERROR;
    }
    Tcl_ObjectSetMapMethodNameProc(ioPtr->oPtr, ItclMapMethodNameProc);

    ioPtr->accessCmd = Tcl_GetObjectCommand(ioPtr->oPtr);
    Tcl_GetCommandInfoFromToken(ioPtr->accessCmd, &cmdInfo);
    cmdInfo.deleteProc = (void *)ItclDestroyObject;
    cmdInfo.deleteData = ioPtr;
    Tcl_SetCommandInfoFromToken(ioPtr->accessCmd, &cmdInfo);
    ioPtr->resolvePtr = (Tcl_Resolve *)ckalloc(sizeof(Tcl_Resolve));
    ioPtr->resolvePtr->cmdProcPtr = Itcl_CmdAliasProc;
    ioPtr->resolvePtr->varProcPtr = Itcl_VarAliasProc;
    ItclResolveInfo *resolveInfoPtr = (ItclResolveInfo *)
    ckalloc(sizeof(ItclResolveInfo));
    memset (resolveInfoPtr, 0, sizeof(ItclResolveInfo));
    resolveInfoPtr->flags = ITCL_RESOLVE_OBJECT;
    resolveInfoPtr->ioPtr = ioPtr;
    ioPtr->resolvePtr->clientData = resolveInfoPtr;
    Tcl_TraceCommand(interp, Tcl_GetString(ioPtr->namePtr),
            TCL_TRACE_RENAME|TCL_TRACE_DELETE, ObjectRenamedTrace, ioPtr);

    Tcl_ObjectSetMetadata(ioPtr->oPtr, iclsPtr->infoPtr->object_meta_type,
            ioPtr);

    /*
     *  Now construct the object.  Look for a constructor in the
     *  most-specific class, and if there is one, invoke it.
     *  This will cause a chain reaction, making sure that all
     *  base classes constructors are invoked as well, in order
     *  from least- to most-specific.  Any constructors that are
     *  not called out explicitly in "initCode" code fragments are
     *  invoked implicitly without arguments.
     */
    result = Itcl_InvokeMethodIfExists(interp, "constructor",
        iclsPtr, ioPtr, newObjc, newObjv);

    /*
     *  If there is no constructor, construct the base classes
     *  in case they have constructors.  This will cause the
     *  same chain reaction.
     */
    Tcl_Obj *objPtr = Tcl_NewStringObj("constructor", -1);
    if (Tcl_FindHashEntry(&iclsPtr->functions, (char *)objPtr) == NULL) {
        result = Itcl_ConstructBase(interp, ioPtr, iclsPtr, newObjc, newObjv);
    }

/* FIX ME this is only for debugging a funny case, where the error message is wrong at the end */
if (result != TCL_OK) {
fprintf(stderr, "DEBUG CONSTRUCTOR error!%s!\n", Tcl_GetStringResult(interp));
}
    /*
     *  If construction failed, then delete the object access
     *  command.  This will destruct the object and delete the
     *  object data.  Be careful to save and restore the interpreter
     *  state, since the destructors may generate errors of their own.
     */
    if (result != TCL_OK) {
        istate = Itcl_SaveInterpState(interp, result);

	/* Bug 227824.
	 * The constructor may destroy the object, possibly indirectly
	 * through the destruction of the main widget in the iTk
	 * megawidget it tried to construct. If this happens we must
	 * not try to destroy the access command a second time.
	 */
	if (ioPtr->accessCmd != (Tcl_Command) NULL) {
	    Tcl_DeleteCommandFromToken(interp, ioPtr->accessCmd);
	    ioPtr->accessCmd = NULL;
	}
        result = Itcl_RestoreInterpState(interp, istate);
    }

    /*
     *  At this point, the object is fully constructed.
     *  Destroy the "constructed" table in the object data, since
     *  it is no longer needed.
     */
    iclsPtr->infoPtr->currIoPtr = NULL;
    Tcl_DeleteHashTable(ioPtr->constructed);
    ckfree((char*)ioPtr->constructed);
    ioPtr->constructed = NULL;

    /*
     *  Add it to the list of all known objects. The only
     *  tricky thing to watch out for is the case where the
     *  object deleted itself inside its own constructor.
     *  In that case, we don't want to add the object to
     *  the list of valid objects. We can determine that
     *  the object deleted itself by checking to see if
     *  its accessCmd member is NULL.
     */
    if (result == TCL_OK && (ioPtr->accessCmd != NULL))  {
        entry = Tcl_CreateHashEntry(&iclsPtr->infoPtr->objects,
            (char*)ioPtr->accessCmd, &newEntry);

        Tcl_SetHashValue(entry, (ClientData)ioPtr);

        if (!(ioPtr->iclsPtr->flags & ITCL_CLASS)) {
	    if (DelegationInstall(interp, ioPtr, iclsPtr) != TCL_OK) {
	        return TCL_ERROR;
	    }
	}
        /* add the objects unknow command to handle all unknown sub commands */
	ClientData pmPtr;
	Tcl_Obj *namePtr;
	Tcl_Obj *argumentPtr;
	Tcl_Obj *bodyPtr;
	namePtr = Tcl_NewStringObj("unknown", -1);
	Tcl_IncrRefCount(namePtr);
	argumentPtr = Tcl_NewStringObj("args", -1);
	Tcl_IncrRefCount(argumentPtr);
	bodyPtr = Tcl_NewStringObj("uplevel 1 ::itcl::builtin::objectunknown ",
	        -1);
	Tcl_AppendToObj(bodyPtr, Tcl_GetString(ioPtr->namePtr), -1);
	Tcl_AppendToObj(bodyPtr, " $args", -1);
	Tcl_IncrRefCount(bodyPtr);
        Itcl_NewProcMethod(interp, ioPtr->oPtr, NULL, NULL, ItclProcErrorProc, 
	    (ItclMemberFunc *)ioPtr, namePtr, argumentPtr, bodyPtr, &pmPtr);
	Tcl_DecrRefCount(namePtr);
	Tcl_DecrRefCount(argumentPtr);
	Tcl_DecrRefCount(bodyPtr);
    }

    /*
     *  Release the object.  If it was destructed above, it will
     *  die at this point.
     */
    Itcl_ReleaseData((ClientData)ioPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclInitObjectVariables()
 *
 *  Init all instance variables and create the necessary variable namespaces
 *  for the given object instance.  This is usually invoked automatically
 *  by Itcl_CreateObject(), when an object is created.
 * ------------------------------------------------------------------------
 */
static int
ItclInitObjectVariables(
   Tcl_Interp *interp,
   ItclObject *ioPtr,
   ItclClass *iclsPtr,
   const char *name)
{
    Tcl_DString buffer;
    Tcl_DString buffer2;
    ItclClass *iclsPtr2;
    ItclHierIter hier;
    ItclVariable *ivPtr;
    Tcl_HashEntry *entry;
    Tcl_HashEntry *hPtr2;;
    Tcl_HashSearch place;
    Tcl_Namespace *varNsPtr;
    Tcl_Namespace *varNsPtr2;
    Tcl_CallFrame frame;
    Tcl_Var varPtr;
    const char *thisName;
    const char *itclOptionsName;
    int itclOptionsIsSet;
    int isNew;

    ivPtr = NULL;
    /*
     * create all the variables for each class in the
     * ::itcl::variables::<object>::<class> namespace as an
     * undefined variable using the Tcl "variable xx" command
     */
    itclOptionsIsSet = 0;
    Itcl_InitHierIter(&hier, iclsPtr);
    iclsPtr2 = Itcl_AdvanceHierIter(&hier);
    while (iclsPtr2 != NULL) {
	Tcl_DStringInit(&buffer);
	Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
	if ((name[0] != ':') && (name[1] != ':')) {
             Tcl_DStringAppend(&buffer, "::", 2);
	}
	Tcl_DStringAppend(&buffer, name, -1);
	Tcl_DStringAppend(&buffer, iclsPtr2->nsPtr->fullName, -1);
	varNsPtr = Tcl_CreateNamespace(interp, Tcl_DStringValue(&buffer),
	        NULL, 0);
	if (varNsPtr == NULL) {
	    varNsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer),
	            NULL, 0);
	}
	/* now initialize the variables which have an init value */
        if (Tcl_PushCallFrame(interp, &frame, varNsPtr,
                /*isProcCallFrame*/0) != TCL_OK) {
            return TCL_ERROR;
        }
        entry = Tcl_FirstHashEntry(&iclsPtr2->variables, &place);
        while (entry) {
            ivPtr = (ItclVariable*)Tcl_GetHashValue(entry);
            if ((ivPtr->flags & ITCL_OPTIONS_VAR) && !itclOptionsIsSet) {
                /* this is the special code for the "itcl_options" variable */
		itclOptionsIsSet = 1;
                itclOptionsName = Tcl_GetString(ivPtr->namePtr);
                Tcl_DStringInit(&buffer2);
	        Tcl_DStringAppend(&buffer2, ITCL_VARIABLES_NAMESPACE, -1);
	        if ((name[0] != ':') && (name[1] != ':')) {
                     Tcl_DStringAppend(&buffer2, "::", 2);
	        }
	        Tcl_DStringAppend(&buffer2, name, -1);
	        varNsPtr2 = Tcl_CreateNamespace(interp,
		        Tcl_DStringValue(&buffer2), NULL, 0);
	        if (varNsPtr2 == NULL) {
	            varNsPtr2 = Tcl_FindNamespace(interp,
		            Tcl_DStringValue(&buffer2), NULL, 0);
	        }
                Tcl_DStringFree(&buffer2);
	        Tcl_PopCallFrame(interp);
	        /* now initialize the variables which have an init value */
                if (Tcl_PushCallFrame(interp, &frame, varNsPtr2,
                        /*isProcCallFrame*/0) != TCL_OK) {
                    return TCL_ERROR;
                }
                if (Tcl_SetVar2(interp, "itcl_options", "",
	                "", TCL_NAMESPACE_ONLY) == NULL) {
	            Tcl_PopCallFrame(interp);
		    return TCL_ERROR;
                }
                Tcl_TraceVar2(interp, "itcl_options",
                        NULL,
                        TCL_TRACE_READS|TCL_TRACE_WRITES,
                        ItclTraceOptionVar, (ClientData)ioPtr);
	        Tcl_PopCallFrame(interp);
                if (Tcl_PushCallFrame(interp, &frame, varNsPtr,
                        /*isProcCallFrame*/0) != TCL_OK) {
                    return TCL_ERROR;
                }
                entry = Tcl_NextHashEntry(&place);
	        continue;
            }
	    if ((ivPtr->flags & ITCL_COMMON) == 0) {
		varPtr = Tcl_NewNamespaceVar(interp, varNsPtr,
		        Tcl_GetString(ivPtr->namePtr));
	        hPtr2 = Tcl_CreateHashEntry(&ioPtr->objectVariables,
		        (char *)ivPtr, &isNew);
	        if (isNew) {
		    Tcl_SetHashValue(hPtr2, varPtr);
		} else {
		}
                IctlVarTraceInfo *traceInfoPtr;
                traceInfoPtr = (IctlVarTraceInfo *)ckalloc(
		        sizeof(IctlVarTraceInfo));
                memset (traceInfoPtr, 0, sizeof(IctlVarTraceInfo));
                traceInfoPtr->flags = ITCL_TRACE_OBJECT;
                traceInfoPtr->ioPtr = ioPtr;
                traceInfoPtr->iclsPtr = iclsPtr2;
                traceInfoPtr->ivPtr = ivPtr;
                Tcl_TraceVar2(interp, Tcl_GetString(ivPtr->namePtr), NULL,
                       TCL_TRACE_UNSETS, ItclTraceUnsetVar,
                       (ClientData)traceInfoPtr);
	        if (ivPtr->flags & ITCL_THIS_VAR) {
                    thisName = Tcl_GetString(ivPtr->namePtr);
		    if (Tcl_SetVar2(interp, thisName, NULL,
		        "", TCL_NAMESPACE_ONLY) == NULL) {
		        return TCL_ERROR;
	            }
	            Tcl_TraceVar2(interp, thisName, NULL,
		        TCL_TRACE_READS|TCL_TRACE_WRITES, ItclTraceThisVar,
		        (ClientData)ioPtr);
	        } else {
	            if (ivPtr->init != NULL) {
		        if (Tcl_ObjSetVar2(interp, ivPtr->namePtr, NULL,
		            ivPtr->init, TCL_NAMESPACE_ONLY) == NULL) {
		            return TCL_ERROR;
	                }
	            }
	        }
	    } else {
	        hPtr2 = Tcl_FindHashEntry(&iclsPtr2->classCommons,
		        (char *)ivPtr);
		if (hPtr2 == NULL) {
	            Tcl_PopCallFrame(interp);
		    return TCL_ERROR;
		}
		varPtr = Tcl_GetHashValue(hPtr2);
	        hPtr2 = Tcl_CreateHashEntry(&ioPtr->objectVariables,
		        (char *)ivPtr, &isNew);
	        if (isNew) {
		    Tcl_SetHashValue(hPtr2, varPtr);
		} else {
		}
	    }
            entry = Tcl_NextHashEntry(&place);
        }
	Tcl_PopCallFrame(interp);
        iclsPtr2 = Itcl_AdvanceHierIter(&hier);
    }
    Tcl_DStringFree(&buffer);
    Itcl_DeleteHierIter(&hier);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclInitObjectOptions()
 *
 *  Collect all instance options for the given object instance to allow
 *  faster runtime access to the options.
 *  It is assumed, that an option can only exist in one class!!
 *  So no duplicates allowed!!
 *  This is usually invoked *  automatically by Itcl_CreateObject(),
 *  when an object is created.
 * ------------------------------------------------------------------------
 */
int
ItclInitObjectOptions(
   Tcl_Interp *interp,
   ItclObject *ioPtr,
   ItclClass *iclsPtr,
   const char *name)
{
    Tcl_DString buffer;
    ItclClass *iclsPtr2;
    ItclHierIter hier;
    ItclOption *ioptPtr;
    ItclDelegatedOption *idoPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashEntry *hPtr2;;
    Tcl_HashSearch place;
    Tcl_CallFrame frame;
    Tcl_Namespace *varNsPtr;
    const char *itclOptionsName;
    int isNew;

    ioptPtr = NULL;
    Itcl_InitHierIter(&hier, iclsPtr);
    iclsPtr2 = Itcl_AdvanceHierIter(&hier);
    while (iclsPtr2 != NULL) {
	/* now initialize the options which have an init value */
        hPtr = Tcl_FirstHashEntry(&iclsPtr2->options, &place);
        while (hPtr) {
            ioptPtr = (ItclOption*)Tcl_GetHashValue(hPtr);
	    hPtr2 = Tcl_CreateHashEntry(&ioPtr->objectOptions,
	            (char *)ioptPtr->namePtr, &isNew);
	    if (isNew) {
		Tcl_SetHashValue(hPtr2, ioptPtr);
                itclOptionsName = Tcl_GetString(ioptPtr->namePtr);
                Tcl_DStringInit(&buffer);
	        Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
	        if ((name[0] != ':') && (name[1] != ':')) {
                     Tcl_DStringAppend(&buffer, "::", 2);
	        }
	        Tcl_DStringAppend(&buffer, name, -1);
	        varNsPtr = Tcl_CreateNamespace(interp,
		        Tcl_DStringValue(&buffer), NULL, 0);
	        if (varNsPtr == NULL) {
	            varNsPtr = Tcl_FindNamespace(interp,
		            Tcl_DStringValue(&buffer), NULL, 0);
	        }
                Tcl_DStringFree(&buffer);
	        /* now initialize the options which have an init value */
                if (Tcl_PushCallFrame(interp, &frame, varNsPtr,
                        /*isProcCallFrame*/0) != TCL_OK) {
                    return TCL_ERROR;
                }
                if (Tcl_SetVar2(interp, "itcl_options", "",
	                "", TCL_NAMESPACE_ONLY) == NULL) {
	            Tcl_PopCallFrame(interp);
		    return TCL_ERROR;
                }
                Tcl_TraceVar2(interp, "itcl_options",
                        NULL,
                        TCL_TRACE_READS|TCL_TRACE_WRITES,
                        ItclTraceOptionVar, (ClientData)ioPtr);
	        Tcl_PopCallFrame(interp);
            }
            hPtr = Tcl_NextHashEntry(&place);
        }
	/* now check for options which are delegated */
        hPtr = Tcl_FirstHashEntry(&iclsPtr2->delegatedOptions, &place);
        while (hPtr) {
            idoPtr = (ItclDelegatedOption*)Tcl_GetHashValue(hPtr);
	    hPtr2 = Tcl_CreateHashEntry(&ioPtr->objectDelegatedOptions,
	            (char *)idoPtr->namePtr, &isNew);
	    if (isNew) {
		Tcl_SetHashValue(hPtr2, idoPtr);
	    }
            hPtr = Tcl_NextHashEntry(&place);
        }
        iclsPtr2 = Itcl_AdvanceHierIter(&hier);
    }
    Tcl_DStringFree(&buffer);
    Itcl_DeleteHierIter(&hier);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_DeleteObject()
 *
 *  Attempts to delete an object by invoking its destructor.
 *
 *  If the destructor is successful, then the object is deleted by
 *  removing its access command, and this procedure returns TCL_OK.
 *  Otherwise, the object will remain alive, and this procedure
 *  returns TCL_ERROR (along with an error message in the interpreter).
 * ------------------------------------------------------------------------
 */
int
Itcl_DeleteObject(
    Tcl_Interp *interp,      /* interpreter mananging object */
    ItclObject *contextIoPtr)  /* object to be deleted */
{
    ItclClass *iclsPtr = (ItclClass*)contextIoPtr->iclsPtr;

    Tcl_HashEntry *entry;
    Tcl_CmdInfo cmdInfo;

    contextIoPtr->flags |= ITCL_OBJECT_IS_DELETED;
    Itcl_PreserveData((ClientData)contextIoPtr);

    /*
     *  Invoke the object's destructors.
     */
    if (Itcl_DestructObject(interp, contextIoPtr, 0) != TCL_OK) {
        Itcl_ReleaseData((ClientData)contextIoPtr);
	contextIoPtr->flags |= ITCL_TCLOO_OBJECT_IS_DELETED;
        return TCL_ERROR;
    }

    /*
     *  Remove the object from the global list.
     */
    entry = Tcl_FindHashEntry(&iclsPtr->infoPtr->objects,
        (char*)contextIoPtr->accessCmd);

    if (entry) {
        Tcl_DeleteHashEntry(entry);
    }

    /*
     *  Change the object's access command so that it can be
     *  safely deleted without attempting to destruct the object
     *  again.  Then delete the access command.  If this is
     *  the last use of the object data, the object will die here.
     */
    if (Tcl_GetCommandInfoFromToken(contextIoPtr->accessCmd, &cmdInfo) == 1) {
        cmdInfo.deleteProc = Itcl_ReleaseData;
	Tcl_SetCommandInfoFromToken(contextIoPtr->accessCmd, &cmdInfo);

        Tcl_DeleteCommandFromToken(interp, contextIoPtr->accessCmd);
    }
    contextIoPtr->oPtr = NULL;
    contextIoPtr->accessCmd = NULL;

    Itcl_ReleaseData((ClientData)contextIoPtr);  /* object should die here */

    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclDeleteObjectVariablesNamespace()
 *
 * ------------------------------------------------------------------------
 */
void
ItclDeleteObjectVariablesNamespace(
    Tcl_Interp *interp,
    ItclObject *ioPtr)
{
    Tcl_DString buffer;
    Tcl_Namespace *varNsPtr;
    const char *name;

    if (!(ioPtr->flags & ITCL_OBJECT_NO_VARNS_DELETE)) {
        /* free the object's variables namespace and variables in it */
        ioPtr->flags &= ~ITCL_OBJECT_SHOULD_VARNS_DELETE;
        Tcl_DStringInit(&buffer);
        name = Tcl_GetCommandName(interp, ioPtr->accessCmd);
	if (strlen(name) == 0) {
	    /* empty command (not found) */
	    return;
	}
        Tcl_DStringInit(&buffer);
        Tcl_DStringAppend(&buffer, ITCL_VARIABLES_NAMESPACE, -1);
        if ((name[0] != ':') && (name[1] != ':')) {
                Tcl_DStringAppend(&buffer, "::", 2);
        }
        Tcl_DStringAppend(&buffer, name, -1);
        varNsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer),
	        NULL, 0);
        if (varNsPtr != NULL) {
            Tcl_DeleteNamespace(varNsPtr);
        }
        Tcl_DStringFree(&buffer);
    } else {
        ioPtr->flags |= ITCL_OBJECT_SHOULD_VARNS_DELETE;
    }
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_DestructObject()
 *
 *  Invokes the destructor for a particular object.  Usually invoked
 *  by Itcl_DeleteObject() or Itcl_DestroyObject() as a part of the
 *  object destruction process.  If the ITCL_IGNORE_ERRS flag is
 *  included, all destructors are invoked even if errors are
 *  encountered, and the result will always be TCL_OK.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */
int
Itcl_DestructObject(
    Tcl_Interp *interp,      /* interpreter mananging new object */
    ItclObject *contextIoPtr,  /* object to be destructed */
    int flags)               /* flags: ITCL_IGNORE_ERRS */
{
    int result;

    if ((contextIoPtr->flags & (ITCL_OBJECT_IS_DESTRUCTED))) {
        if (contextIoPtr->destructed) {
            if ((flags & ITCL_IGNORE_ERRS) == 0) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "can't delete an object while it is being destructed",
                    (char*)NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }
    if (contextIoPtr->accessCmd == NULL) {
        return TCL_OK;
    }
    contextIoPtr->flags |= ITCL_OBJECT_IS_DESTRUCTED;
    /*
     *  If there is a "destructed" table, then this object is already
     *  being destructed.  Flag an error, unless errors are being
     *  ignored.
     */
    if (contextIoPtr->destructed) {
        if ((flags & ITCL_IGNORE_ERRS) == 0) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "can't delete an object while it is being destructed",
                (char*)NULL);
            return TCL_ERROR;
        }
        return TCL_OK;
    }

    result = TCL_OK;
    if (contextIoPtr->oPtr != NULL) {
    /*
     *  Create a "destructed" table to keep track of which destructors
     *  have been invoked.  This is used in ItclDestructBase to make
     *  sure that all base class destructors have been called,
     *  explicitly or implicitly.
     */
    contextIoPtr->destructed = (Tcl_HashTable*)ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitObjHashTable(contextIoPtr->destructed);

    /*
     *  Destruct the object starting from the most-specific class.
     *  If all goes well, return the null string as the result.
     */
    result = ItclDestructBase(interp, contextIoPtr,
            contextIoPtr->iclsPtr, flags);

    if (result == TCL_OK) {
        Tcl_ResetResult(interp);
    }

    Tcl_DeleteHashTable(contextIoPtr->destructed);
    ckfree((char*)contextIoPtr->destructed);
    contextIoPtr->destructed = NULL;
    }
    
    ItclDeleteObjectVariablesNamespace(interp, contextIoPtr);

    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclDestructBase()
 *
 *  Invoked by Itcl_DestructObject() to recursively destruct an object
 *  from the specified class level.  Finds and invokes the destructor
 *  for the specified class, and then recursively destructs all base
 *  classes.  If the ITCL_IGNORE_ERRS flag is included, all destructors
 *  are invoked even if errors are encountered, and the result will
 *  always be TCL_OK.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error message
 *  in interp->result) on error.
 * ------------------------------------------------------------------------
 */
static int
ItclDestructBase(
    Tcl_Interp *interp,         /* interpreter */
    ItclObject *contextIoPtr,   /* object being destructed */
    ItclClass *contextIclsPtr,  /* current class being destructed */
    int flags)                  /* flags: ITCL_IGNORE_ERRS */
{
    int result;
    Itcl_ListElem *elem;
    ItclClass *iclsPtr;

    /*
     *  Look for a destructor in this class, and if found,
     *  invoke it.
     */
    if (Tcl_FindHashEntry(contextIoPtr->destructed,
            (char *)contextIclsPtr->namePtr) == NULL) {
        result = Itcl_InvokeMethodIfExists(interp, "destructor",
            contextIclsPtr, contextIoPtr, 0, (Tcl_Obj* CONST*)NULL);
        if (result != TCL_OK) {
            return TCL_ERROR;
        }
    }

    /*
     *  Scan through the list of base classes recursively and destruct
     *  them.  Traverse the list in normal order, so that we destruct
     *  from most- to least-specific.
     */
    elem = Itcl_FirstListElem(&contextIclsPtr->bases);
    while (elem) {
        iclsPtr = (ItclClass*)Itcl_GetListValue(elem);

        if (ItclDestructBase(interp, contextIoPtr, iclsPtr, flags) != TCL_OK) {
            return TCL_ERROR;
        }
        elem = Itcl_NextListElem(elem);
    }

    /*
     *  Throw away any result from the destructors and return.
     */
    Tcl_ResetResult(interp);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_FindObject()
 *
 *  Searches for an object with the specified name, which have
 *  namespace scope qualifiers like "namesp::namesp::name", or may
 *  be a scoped value such as "namespace inscope ::foo obj".
 *
 *  If an error is encountered, this procedure returns TCL_ERROR
 *  along with an error message in the interpreter.  Otherwise, it
 *  returns TCL_OK.  If an object was found, "roPtr" returns a
 *  pointer to the object data.  Otherwise, it returns NULL.
 * ------------------------------------------------------------------------
 */
int
Itcl_FindObject(
    Tcl_Interp *interp,      /* interpreter containing this object */
    CONST char *name,        /* name of the object */
    ItclObject **roPtr)      /* returns: object data or NULL */
{
    Tcl_Namespace *contextNs = NULL;

    char *cmdName;
    Tcl_Command cmd;
    Tcl_CmdInfo cmdInfo;

    /*
     *  The object name may be a scoped value of the form
     *  "namespace inscope <namesp> <command>".  If it is,
     *  decode it.
     */
    if (Itcl_DecodeScopedCommand(interp, name, &contextNs, &cmdName)
        != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Look for the object's access command, and see if it has
     *  the appropriate command handler.
     */
    cmd = Tcl_FindCommand(interp, cmdName, contextNs, /* flags */ 0);
    if (cmd != NULL && Itcl_IsObject(cmd)) {
        if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) != 1) {
            *roPtr = NULL;
        }
        *roPtr = cmdInfo.deleteData;
    } else {
        *roPtr = NULL;
    }

    ckfree(cmdName);

    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_IsObject()
 *
 *  Checks the given Tcl command to see if it represents an itcl object.
 *  Returns non-zero if the command is associated with an object.
 * ------------------------------------------------------------------------
 */
int
Itcl_IsObject(
    Tcl_Command cmd)         /* command being tested */
{
    Tcl_CmdInfo cmdInfo;

    if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) != 1) {
        return 0;
    }

    if ((void *)cmdInfo.deleteProc == (void *)ItclDestroyObject) {
        return 1;
    }

    /*
     *  This may be an imported command.  Try to get the real
     *  command and see if it represents an object.
     */
    cmd = Tcl_GetOriginalCommand(cmd);
    if (cmd != NULL) {
        if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) != 1) {
            return 0;
        }

        if ((void *)cmdInfo.deleteProc == (void *)ItclDestroyObject) {
            return 1;
        }
    }
    return 0;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ObjectIsa()
 *
 *  Checks to see if an object belongs to the given class.  An object
 *  "is-a" member of the class if the class appears anywhere in its
 *  inheritance hierarchy.  Returns non-zero if the object belongs to
 *  the class, and zero otherwise.
 * ------------------------------------------------------------------------
 */
int
Itcl_ObjectIsa(
    ItclObject *contextIoPtr, /* object being tested */
    ItclClass *iclsPtr)       /* class to test for "is-a" relationship */
{
    Tcl_HashEntry *entry;
    entry = Tcl_FindHashEntry(&contextIoPtr->iclsPtr->heritage, (char*)iclsPtr);
    return (entry != NULL);
}

/*
 * ------------------------------------------------------------------------
 *  ItclGetInstanceVar()
 *
 *  Returns the current value for an object data member.  The member
 *  name is interpreted with respect to the given class scope, which
 *  is usually the most-specific class for the object.
 *
 *  If successful, this procedure returns a pointer to a string value
 *  which remains alive until the variable changes it value.  If
 *  anything goes wrong, this returns NULL.
 * ------------------------------------------------------------------------
 */
CONST char*
ItclGetInstanceVar(
    Tcl_Interp *interp,        /* current interpreter */
    const char *name1,         /* name of desired instance variable */
    const char *name2,         /* array element or NULL */
    ItclObject *contextIoPtr,  /* current object */
    ItclClass *contextIclsPtr) /* name is interpreted in this scope */
{
    Tcl_CallFrame frame;
    Tcl_CallFrame *framePtr;
    Tcl_Namespace *nsPtr;
    Tcl_DString buffer;
    CONST char *val;
    int doAppend;

    /*
     *  Make sure that the current namespace context includes an
     *  object that is being manipulated.
     */
    if (contextIoPtr == NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "cannot access object-specific info without an object context",
            (char*)NULL);
        return NULL;
    }

    /*
     *  Install the object context and access the data member
     *  like any other variable.
     */
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, Tcl_GetString(contextIoPtr->varNsNamePtr), -1);
    doAppend = 1;
    if (contextIclsPtr->flags & ITCL_ECLASS) {
        if (strcmp(name1, "itcl_options") == 0) {
	    doAppend = 0;
        }
    }
    if (doAppend) {
        Tcl_DStringAppend(&buffer,
                Tcl_GetString(contextIclsPtr->fullNamePtr), -1);
    }
    nsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer), NULL, 0);
    Tcl_DStringFree(&buffer);
    val = NULL;
    if (nsPtr != NULL) {
	framePtr = &frame;
	Tcl_PushCallFrame(interp, framePtr, nsPtr, /*isProcCallFrame*/0);
        val = Tcl_GetVar2(interp, (CONST84 char *)name1, (char*)name2,
	        TCL_LEAVE_ERR_MSG);
        Tcl_PopCallFrame(interp);
    }

    return val;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_GetInstanceVar()
 *
 *  Returns the current value for an object data member.  The member
 *  name is interpreted with respect to the given class scope, which
 *  is usually the most-specific class for the object.
 *
 *  If successful, this procedure returns a pointer to a string value
 *  which remains alive until the variable changes it value.  If
 *  anything goes wrong, this returns NULL.
 * ------------------------------------------------------------------------
 */
CONST char*
Itcl_GetInstanceVar(
    Tcl_Interp *interp,        /* current interpreter */
    const char *name,          /* name of desired instance variable */
    ItclObject *contextIoPtr,  /* current object */
    ItclClass *contextIclsPtr) /* name is interpreted in this scope */
{
    return ItclGetInstanceVar(interp, name, NULL, contextIoPtr,
            contextIclsPtr);
}

/*
 * ------------------------------------------------------------------------
 *  ItclSetInstanceVar()
 *
 *  Sets the current value for an object data member.  The member
 *  name is interpreted with respect to the given class scope, which
 *  is usually the most-specific class for the object.
 *
 *  If successful, this procedure returns a pointer to a string value
 *  which remains alive until the variable changes it value.  If
 *  anything goes wrong, this returns NULL.
 * ------------------------------------------------------------------------
 */
CONST char*
ItclSetInstanceVar(
    Tcl_Interp *interp,        /* current interpreter */
    const char *name1,         /* name of desired instance variable */
    const char *name2,         /* array member or NULL */
    const char *value,         /* the value to set */
    ItclObject *contextIoPtr,  /* current object */
    ItclClass *contextIclsPtr) /* name is interpreted in this scope */
{
    Tcl_CallFrame frame;
    Tcl_CallFrame *framePtr;
    Tcl_Namespace *nsPtr;
    Tcl_DString buffer;
    CONST char *val;
    int doAppend;

    /*
     *  Make sure that the current namespace context includes an
     *  object that is being manipulated.
     */
    if (contextIoPtr == NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "cannot access object-specific info without an object context",
            (char*)NULL);
        return NULL;
    }

    /*
     *  Install the object context and access the data member
     *  like any other variable.
     */
    Tcl_DStringInit(&buffer);
    Tcl_DStringAppend(&buffer, Tcl_GetString(contextIoPtr->varNsNamePtr), -1);
    doAppend = 1;
    if (contextIclsPtr->flags & ITCL_ECLASS) {
        if (strcmp(name1, "itcl_options") == 0) {
	    doAppend = 0;
        }
    }
    if (doAppend) {
        Tcl_DStringAppend(&buffer,
                Tcl_GetString(contextIclsPtr->fullNamePtr), -1);
    }
    nsPtr = Tcl_FindNamespace(interp, Tcl_DStringValue(&buffer), NULL, 0);
    Tcl_DStringFree(&buffer);
    val = NULL;
    if (nsPtr != NULL) {
	framePtr = &frame;
	Tcl_PushCallFrame(interp, framePtr, nsPtr, /*isProcCallFrame*/0);
        val = Tcl_SetVar2(interp, (CONST84 char *)name1, (char*)name2,
	        value, TCL_LEAVE_ERR_MSG);
        Tcl_PopCallFrame(interp);
    }

    return val;
}

/*
 * ------------------------------------------------------------------------
 *  ItclReportObjectUsage()
 *
 *  Appends information to the given interp summarizing the usage
 *  for all of the methods available for this object.  Useful when
 *  reporting errors in Itcl_HandleInstance().
 * ------------------------------------------------------------------------
 */
void
ItclReportObjectUsage(
    Tcl_Interp *interp,           /* current interpreter */
    ItclObject *contextIoPtr,     /* current object */
    Tcl_Namespace *callerNsPtr,
    Tcl_Namespace *contextNsPtr)  /* the context namespace */
{
    ItclClass *iclsPtr = (ItclClass*)contextIoPtr->iclsPtr;
    int ignore = ITCL_CONSTRUCTOR | ITCL_DESTRUCTOR | ITCL_COMMON;

    int cmp;
    char *name;
    Itcl_List cmdList;
    Itcl_ListElem *elem;
    Tcl_HashEntry *entry;
    Tcl_HashSearch place;
    ItclMemberFunc *imPtr;
    ItclMemberFunc *cmpFunc;
    Tcl_Obj *resultPtr;

    /*
     *  Scan through all methods in the virtual table and sort
     *  them in alphabetical order.  Report only the methods
     *  that have simple names (no ::'s) and are accessible.
     */
    Itcl_InitList(&cmdList);
    entry = Tcl_FirstHashEntry(&iclsPtr->resolveCmds, &place);
    while (entry) {
        name  = Tcl_GetHashKey(&iclsPtr->resolveCmds, entry);
        imPtr = (ItclMemberFunc*)Tcl_GetHashValue(entry);

        if (strstr(name,"::") || (imPtr->flags & ignore) != 0) {
            imPtr = NULL;
        } else {
	    if (imPtr->protection != ITCL_PUBLIC) {
		if (contextNsPtr != NULL) {
                    if (!Itcl_CanAccessFunc(imPtr, contextNsPtr)) {
                        imPtr = NULL;
                    }
                }
	    }
        }
        if ((imPtr != NULL) && (imPtr->codePtr != NULL)) {
	    if (imPtr->codePtr->flags & ITCL_BUILTIN) {
	        char *body;
	        if (imPtr->codePtr != NULL) {
	            body = Tcl_GetString(imPtr->codePtr->bodyPtr);
	            if ((*body == '@') &&
		            (strcmp(body, "@itcl-builtin-info") == 0)) {
	                imPtr = NULL;
	            }
	        }
	    }
	}

        if (imPtr) {
            elem = Itcl_FirstListElem(&cmdList);
            while (elem) {
                cmpFunc = (ItclMemberFunc*)Itcl_GetListValue(elem);
                cmp = strcmp(Tcl_GetString(imPtr->namePtr),
		        Tcl_GetString(cmpFunc->namePtr));
                if (cmp < 0) {
                    Itcl_InsertListElem(elem, (ClientData)imPtr);
                    imPtr = NULL;
                    break;
                } else {
		    if (cmp == 0) {
                        imPtr = NULL;
                        break;
		    }
                }
                elem = Itcl_NextListElem(elem);
            }
            if (imPtr) {
                Itcl_AppendList(&cmdList, (ClientData)imPtr);
            }
        }
        entry = Tcl_NextHashEntry(&place);
    }

    /*
     *  Add a series of statements showing usage info.
     */
    resultPtr = Tcl_GetObjResult(interp);
    elem = Itcl_FirstListElem(&cmdList);
    while (elem) {
        imPtr = (ItclMemberFunc*)Itcl_GetListValue(elem);
        Tcl_AppendToObj(resultPtr, "\n  ", -1);
        Itcl_GetMemberFuncUsage(imPtr, contextIoPtr, resultPtr);

        elem = Itcl_NextListElem(elem);
    }
    Itcl_DeleteList(&cmdList);
}

/*
 * ------------------------------------------------------------------------
 *  ItclTraceThisVar()
 *
 *  Invoked to handle read/write traces on the "this" variable built
 *  into each object.
 *
 *  On read, this procedure updates the "this" variable to contain the
 *  current object name.  This is done dynamically, since an object's
 *  identity can change if its access command is renamed.
 *
 *  On write, this procedure returns an error string, warning that
 *  the "this" variable cannot be set.
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
static char*
ItclTraceThisVar(
    ClientData cdata,	    /* object instance data */
    Tcl_Interp *interp,	    /* interpreter managing this variable */
    CONST char *name1,	    /* variable name */
    CONST char *name2,	    /* unused */
    int flags)		    /* flags indicating read/write */
{
    ItclObject *contextIoPtr = (ItclObject*)cdata;
    const char *objName;
    Tcl_Obj *objPtr;

    /*
     *  Handle read traces on "this"
     */
    if ((flags & TCL_TRACE_READS) != 0) {
        objPtr = Tcl_NewStringObj("", -1);
        Tcl_IncrRefCount(objPtr);

	if (strcmp(name1, "this") == 0) {
            if (contextIoPtr->accessCmd) {
                Tcl_GetCommandFullName(contextIoPtr->iclsPtr->interp,
                    contextIoPtr->accessCmd, objPtr);
            }

            objName = Tcl_GetString(objPtr);
	} else {
	    /* thiswidget variable */
	    objName = Tcl_GetCommandName(contextIoPtr->iclsPtr->interp,
	            contextIoPtr->accessCmd);
	}
        Tcl_SetVar(interp, (CONST84 char *)name1, objName, 0);

        Tcl_DecrRefCount(objPtr);
        return NULL;
    }

    /*
     *  Handle write traces on "this"
     */
    if ((flags & TCL_TRACE_WRITES) != 0) {
        return "variable \"this\" cannot be modified";
    }
    return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  ItclTraceOptionVar()
 *
 *  Invoked to handle read/write traces on "option" variables
 *
 *  On read, this procedure checks if there is a cgetMethodPtr and calls it
 *  On write, this procedure checks if there is a configureMethodPtr
 *  or validateMethodPtr and calls it
 * ------------------------------------------------------------------------
 */
/* ARGSUSED */
static char*
ItclTraceOptionVar(
    ClientData cdata,	    /* object instance data */
    Tcl_Interp *interp,	    /* interpreter managing this variable */
    CONST char *name1,	    /* variable name */
    CONST char *name2,	    /* unused */
    int flags)		    /* flags indicating read/write */
{
    ItclObject *ioPtr;
    ItclOption *ioptPtr;

/* FIX ME !!! */
/* don't know yet if ItclTraceOptionVar is really needed !! */
    if (cdata != NULL) {
        ioPtr = (ItclObject*)cdata;
    } else {
        ioptPtr = (ItclOption*)cdata;
        /*
         *  Handle read traces "itcl_options"
         */
        if ((flags & TCL_TRACE_READS) != 0) {
            return NULL;
        }
    
        /*
         *  Handle write traces "itcl_options"
         */
        if ((flags & TCL_TRACE_WRITES) != 0) {
            return NULL;
        }
    }
    return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  ItclDestroyObject()
 *
 *  Invoked when the object access command is deleted to implicitly
 *  destroy the object.  Invokes the object's destructors, ignoring
 *  any errors encountered along the way.  Removes the object from
 *  the list of all known objects and releases the access command's
 *  claim to the object data.
 *
 *  Note that the usual way to delete an object is via Itcl_DeleteObject().
 *  This procedure is provided as a back-up, to handle the case when
 *  an object is deleted by removing its access command.
 * ------------------------------------------------------------------------
 */
static void
ItclDestroyObject(
    ClientData cdata)  /* object instance data */
{
    ItclObject *contextIoPtr = (ItclObject*)cdata;
    ItclClass *iclsPtr = (ItclClass*)contextIoPtr->iclsPtr;
    Tcl_HashEntry *entry;
    Itcl_InterpState istate;

    if (contextIoPtr->accessCmd == NULL) {
	/* object has already been destroyed !! */
        return;
    }
    /*
     *  Attempt to destruct the object, but ignore any errors.
     */
    istate = Itcl_SaveInterpState(iclsPtr->interp, 0);
    Itcl_DestructObject(iclsPtr->interp, contextIoPtr, ITCL_IGNORE_ERRS);
    Itcl_RestoreInterpState(iclsPtr->interp, istate);

    /*
     *  Now, remove the object from the global object list.
     *  We're careful to do this here, after calling the destructors.
     *  Once the access command is nulled out, the "this" variable
     *  won't work properly.
     */
    if (contextIoPtr->accessCmd != NULL) {
        entry = Tcl_FindHashEntry(&iclsPtr->infoPtr->objects,
            (char*)contextIoPtr->accessCmd);

        if (entry) {
            Tcl_DeleteHashEntry(entry);
        }
        contextIoPtr->accessCmd = NULL;
    }

    Itcl_ReleaseData((ClientData)contextIoPtr);
}

/*
 * ------------------------------------------------------------------------
 *  ItclFreeObject()
 *
 *  Deletes all instance variables and frees all memory associated with
 *  the given object instance.  This is usually invoked automatically
 *  by Itcl_ReleaseData(), when an object's data is no longer being used.
 * ------------------------------------------------------------------------
 */
static void
ItclFreeObject(
    char * cdata)  /* object instance data */
{
    ItclObject *contextObj = (ItclObject*)cdata;

    if (contextObj->accessCmd == NULL) {
	/* object has already been freed */
        return;
    }
    /*
     *  Install the class namespace and object context so that
     *  the object's data members can be destroyed via simple
     *  "unset" commands.  This makes sure that traces work properly
     *  and all memory gets cleaned up.
     *
     *  NOTE:  Be careful to save and restore the interpreter state.
     *    Data can get freed in the middle of any operation, and
     *    we can't affort to clobber the interpreter with any errors
     *    from below.
     */

    if (contextObj->constructed) {
        Tcl_DeleteHashTable(contextObj->constructed);
        ckfree((char*)contextObj->constructed);
    }
    if (contextObj->destructed) {
        Tcl_DeleteHashTable(contextObj->destructed);
        ckfree((char*)contextObj->destructed);
    }
    Itcl_ReleaseData((ClientData)contextObj->iclsPtr);

    ckfree((char*)contextObj);
}

/*
 * ------------------------------------------------------------------------
 *  ItclObjectCmd()
 *
 * ------------------------------------------------------------------------
 */

int Itcl_InvokeProcedureMethod(ClientData clientData, Tcl_Interp *interp,
        void * preCallProc, int objc, Tcl_Obj *const *objv);

int
ItclObjectCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    Tcl_Object oPtr,
    Tcl_Class clsPtr,
    int objc,
    Tcl_Obj *const *objv)
{
    Tcl_Obj *methodNamePtr;
    Tcl_Obj **newObjv;
    Tcl_DString buffer;
    ItclMemberFunc *imPtr;
    ItclClass *iclsPtr;
    Itcl_ListElem *elem;
    ItclClass *basePtr;
    char *className;
    char *tail;
    char *cp;
    int isDirectCall;
    int incr;
    int result;
    int found;

    ItclShowArgs(1, "ItclObjectCmd", objc, objv);

    incr = 0;
    found = 0;
    isDirectCall = 0;
    imPtr = (ItclMemberFunc *)clientData;
    iclsPtr = imPtr->iclsPtr;
    if ((oPtr == NULL) && (clsPtr == NULL)) {
         isDirectCall = 1;
    }
    if (oPtr == NULL) {
	ClientData clientData;
	clientData = Itcl_GetCallFrameClientData(interp);
	if (clientData == NULL) {
	    if ((imPtr->flags & ITCL_COMMON)
	            || ((imPtr->codePtr != NULL)
		        && (imPtr->codePtr->flags &ITCL_BUILTIN))) {
	        result = Itcl_InvokeProcedureMethod(imPtr->tmPtr, interp,
		        ItclCheckCallProc, objc, objv);
                return result;
	    }
	    Tcl_AppendResult(interp,
	            "ItclObjectCmd cannot get context object (NULL)", NULL);
	    return TCL_ERROR;
	}
        oPtr = Tcl_ObjectContextObject((Tcl_ObjectContext)clientData);
    }
    methodNamePtr = NULL;
    if (objv[0] != NULL) {
        Itcl_ParseNamespPath(Tcl_GetString(objv[0]), &buffer,
	        &className, &tail);
        if (className != NULL) {
            methodNamePtr = Tcl_NewStringObj(tail, -1);
	    Tcl_IncrRefCount(methodNamePtr);
	    /* look for the class in the hierarchy */
	    cp = className;
	    if ((*cp == ':') && (*(cp+1) == ':')) {
	        cp += 2;
	    }
            elem = Itcl_FirstListElem(&iclsPtr->bases);
	    if (elem == NULL) {
	        /* check the class itself */
		if (strcmp((const char *)cp,
		        (const char *)Tcl_GetString(iclsPtr->namePtr)) == 0) {
		    found = 1;
		    clsPtr = iclsPtr->clsPtr;
		}
	    }
            while (elem != NULL) {
                basePtr = (ItclClass*)Itcl_GetListValue(elem);
		if (strcmp((const char *)cp,
		        (const char *)Tcl_GetString(basePtr->namePtr)) == 0) {
		    clsPtr = basePtr->clsPtr;
		    found = 1;
		    break;
		}
                elem = Itcl_NextListElem(elem);
	    }
        }
        Tcl_DStringFree(&buffer);
    }
    if (isDirectCall) {
	if (!found) {
            methodNamePtr = objv[0];
            Tcl_IncrRefCount(methodNamePtr);
        }
    }
    if (methodNamePtr != NULL) {
        incr = 1;
    }
    newObjv = NULL;
    if (methodNamePtr != NULL) {
        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc+incr));
        newObjv[0] = Tcl_NewStringObj("my", 2);
        newObjv[1] = methodNamePtr;
        Tcl_IncrRefCount(newObjv[0]);
        Tcl_IncrRefCount(newObjv[1]);
        memcpy(newObjv+incr+1, objv+1, (sizeof(Tcl_Obj*)*(objc-1)));
    }
    if (methodNamePtr != NULL) {
        result = Itcl_PublicObjectCmd(oPtr, interp, clsPtr, objc+incr, newObjv);
    } else {
        result = Itcl_PublicObjectCmd(oPtr, interp, clsPtr, objc, objv);
    }

    if (methodNamePtr != NULL) {
        Tcl_DecrRefCount(newObjv[0]);
        Tcl_DecrRefCount(newObjv[1]);
        Tcl_DecrRefCount(methodNamePtr);
        ckfree((char *)newObjv);
    }
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclObjectUnknownCommand()
 *  syntax: is
 *  objv[0]    command name of myself (::itcl::methodset::objectUnknownCommand)
 *  objv[1]    object name for [self]
 *  objv[2]    object name as found on the stack
 *  objv[3]    method name
 * ------------------------------------------------------------------------
 */

int
ItclObjectUnknownCommand(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Tcl_Object oPtr;
    Tcl_Command cmd;
    Tcl_CmdInfo cmdInfo;
    ItclObject *ioPtr;
    ItclObjectInfo *infoPtr;

    ItclShowArgs(0, "ItclObjectUnknownCommand", objc, objv);
    cmd = Tcl_GetCommandFromObj(interp, objv[1]);
    if (Tcl_GetCommandInfoFromToken(cmd, &cmdInfo) != 1) {
    }
    oPtr = cmdInfo.objClientData;
    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
            ITCL_INTERP_DATA, NULL);
    ioPtr = (ItclObject *)Tcl_ObjectGetMetadata(oPtr,
            infoPtr->object_meta_type);
    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
            "bad option \"", Tcl_GetString(objv[3]), "\": should be one of...",
	    (char*)NULL);
    ItclReportObjectUsage(interp, ioPtr, NULL, NULL);
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  GetClassFromClassName()
 * ------------------------------------------------------------------------
 */

static ItclClass *
GetClassFromClassName(
    const char *className,
    ItclClass *iclsPtr)
{
    ItclClass *basePtr;
    Itcl_ListElem *elem;

    /* look for the class in the hierarchy */
    /* first check the class itself */
    if (strcmp(className,
            (const char *)Tcl_GetString(iclsPtr->namePtr)) == 0) {
	return iclsPtr;
    }
    elem = Itcl_FirstListElem(&iclsPtr->bases);
    while (elem != NULL) {
        basePtr = (ItclClass*)Itcl_GetListValue(elem);
	basePtr = GetClassFromClassName(className, basePtr);
	if (basePtr != NULL) {
	    return basePtr;
	}
        elem = Itcl_NextListElem(elem);
    }
    return NULL;
}

/*
 * ------------------------------------------------------------------------
 *  ItclMapProcCmd()
 * ------------------------------------------------------------------------
 */

int
ItclMapMethodNameProc(
    Tcl_Interp *interp,
    Tcl_Object oPtr,
    Tcl_Class *startClsPtr,
    Tcl_Obj *methodObj)
{
    Tcl_Obj *methodName;
    Tcl_Obj *className;
    Tcl_DString buffer;
    ItclObject *ioPtr;
    ItclClass *iclsPtr;
    ItclClass *iclsPtr2;
    ItclObjectInfo *infoPtr;
    char *head;
    char *tail;
    char *sp;

    sp = Tcl_GetString(methodObj);
    Itcl_ParseNamespPath(sp, &buffer, &head, &tail);
    if (head != NULL) {
        infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
                ITCL_INTERP_DATA, NULL);
        ioPtr = (ItclObject *)Tcl_ObjectGetMetadata(oPtr,
                infoPtr->object_meta_type);
	if (ioPtr == NULL) {
	    /* try to get the class (if a class is creating an object) */
            iclsPtr = (ItclClass *)Tcl_ObjectGetMetadata(oPtr,
                    infoPtr->class_meta_type);
	} else {
            iclsPtr = ioPtr->iclsPtr;
	}
        className = NULL;
        methodName = Tcl_NewStringObj(tail, -1);
        Tcl_IncrRefCount(methodName);
        className = Tcl_NewStringObj(head, -1);
        Tcl_IncrRefCount(className);
        sp = Tcl_GetString(className);
	iclsPtr2 = GetClassFromClassName(sp, iclsPtr);
	if (iclsPtr2 != NULL) {
	    *startClsPtr = iclsPtr2->clsPtr;
	    Tcl_SetStringObj(methodObj, Tcl_GetString(methodName), -1);
	}
        Tcl_DecrRefCount(className);
        Tcl_DecrRefCount(methodName);
    }
    Tcl_DStringFree(&buffer);
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  DelegationFunction()
 * ------------------------------------------------------------------------
 */

int
DelegateFunction(
    Tcl_Interp *interp,
    ItclObject *ioPtr,
    ItclClass *iclsPtr,
    Tcl_Obj *componentNamePtr,
    ItclDelegatedFunction *idmPtr)
{
    Tcl_Obj *listPtr;;
    const char **argv;
    int argc;
    int j;

    listPtr = Tcl_NewListObj(0, NULL);
    if (componentNamePtr != NULL) {
        Tcl_ListObjAppendElement(interp, listPtr, componentNamePtr);
        Tcl_IncrRefCount(componentNamePtr);
    }
    if (idmPtr->asPtr != NULL) {
        Tcl_SplitList(interp, Tcl_GetString(idmPtr->asPtr),
	        &argc, &argv);
        for(j=0;j<argc;j++) {
            Tcl_ListObjAppendElement(interp, listPtr,
                    Tcl_NewStringObj(argv[j], -1));
        }
    } else {
	if (idmPtr->usingPtr != NULL) {
	    char *cp;
	    char *ep;
	    cp = Tcl_GetString(idmPtr->usingPtr);
	    ep = cp;
	    while (*ep != '\0') {
	        if (*ep == '%') {
		    if (*(ep+1) == '%') {
		        ep++;
		        continue;
		    }
		    switch (*(ep+1)) {
		    case 'c':
			if (ep-cp-1 > 0) {
		            Tcl_ListObjAppendElement(interp, listPtr,
			            Tcl_NewStringObj(cp, ep-cp-1));
			}
			if (idmPtr->icPtr == NULL) {
			    Tcl_AppendResult(interp,
			            "no component for %c", NULL);
			    return TCL_ERROR;
			}
		        Tcl_ListObjAppendElement(interp, listPtr,
			        Tcl_NewStringObj(Tcl_GetString(
				        componentNamePtr), -1));
		        break;
		    case 'm':
			if (ep-cp-1 > 0) {
		            Tcl_ListObjAppendElement(interp, listPtr,
			            Tcl_NewStringObj(cp, ep-cp-1));
			}
		        Tcl_ListObjAppendElement(interp, listPtr,
			        Tcl_NewStringObj(Tcl_GetString(
				        idmPtr->namePtr), -1));
		        break;
		    case 'n':
			if (ep-cp-1 > 0) {
		            Tcl_ListObjAppendElement(interp, listPtr,
			            Tcl_NewStringObj(cp, ep-cp-1));
			}
		        Tcl_ListObjAppendElement(interp, listPtr,
			        Tcl_NewStringObj(iclsPtr->nsPtr->name,
				        -1));
		        break;
		    case 's':
			if (ep-cp-1 > 0) {
		            Tcl_ListObjAppendElement(interp, listPtr,
			            Tcl_NewStringObj(cp, ep-cp-1));
			}
		        Tcl_ListObjAppendElement(interp, listPtr,
			        Tcl_NewStringObj(Tcl_GetString(
				        ioPtr->namePtr), -1));
		        break;
		    case 't':
			if (ep-cp-1 > 0) {
		            Tcl_ListObjAppendElement(interp, listPtr,
			            Tcl_NewStringObj(cp, ep-cp-1));
			}
		        Tcl_ListObjAppendElement(interp, listPtr,
			        Tcl_NewStringObj(
				        iclsPtr->nsPtr->fullName,
					-1));
		        break;
		    default:
		      {
			char buf[2];
			buf[1] = '\0';
			sprintf(buf, "%c", *(ep+1));
			Tcl_AppendResult(interp,
			        "there is no %%", buf, " substitution",
				NULL);
			return TCL_ERROR;
		        break;
		      }
		    }
		    ep +=2;
		    cp = ep;
		} else {
		    if (*ep == ' ') {
			if (ep-cp > 0) {
		            Tcl_ListObjAppendElement(interp, listPtr,
			            Tcl_NewStringObj(cp, ep-cp));
			}
		        while((*ep != '\0') && (*ep == ' ')) {
			    ep++;
			}
			cp = ep;
		    } else {
		        ep++;
		    }
		}
	    }
	    if (cp != ep) {
                Tcl_ListObjAppendElement(interp, listPtr,
	                Tcl_NewStringObj(cp, ep-cp-1));
	    }
	} else {
            Tcl_ListObjAppendElement(interp, listPtr, idmPtr->namePtr);
        }
    }
    Tcl_IncrRefCount(idmPtr->namePtr);
    /* and now for the argument */
    Tcl_IncrRefCount(idmPtr->namePtr);
    Tcl_Method mPtr;
    mPtr = Itcl_NewForwardClassMethod(interp, iclsPtr->clsPtr, 1,
            idmPtr->namePtr, listPtr);
    if (mPtr != NULL) {
        return TCL_OK;
    }
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  DelegatOptionsInstall()
 * ------------------------------------------------------------------------
 */

int
DelegatedOptionsInstall(
    Tcl_Interp *interp,
    ItclClass *iclsPtr)
{
    Tcl_HashEntry *hPtr2;
    Tcl_HashSearch search2;
    ItclDelegatedOption *idoPtr;
    ItclOption *ioptPtr;
    FOREACH_HASH_DECLS;
    char *optionName;

    FOREACH_HASH_VALUE(idoPtr, &iclsPtr->delegatedOptions) {
	optionName = Tcl_GetString(idoPtr->namePtr);
	if (*optionName == '*') {
	    /* allow nested FOREACH */
	    search2 = search;
	    FOREACH_HASH_VALUE(ioptPtr, &iclsPtr->options) {
	        if (Tcl_FindHashEntry(&idoPtr->exceptions,
		        (char *)idoPtr->namePtr) == NULL) {
		    ioptPtr->idoPtr = idoPtr;
		}
	    }
	    search = search2;
	} else {
            hPtr2 = Tcl_FindHashEntry(&iclsPtr->options,
	            (char *)idoPtr->namePtr);
	    if (hPtr2 == NULL) {
                Tcl_AppendResult(interp, "missing option \"", optionName,
		        "\" in options for delegate option", NULL);
		return TCL_ERROR;
	    }
	    ioptPtr = Tcl_GetHashValue(hPtr2);
	    idoPtr->ioptPtr = ioptPtr;
	    ioptPtr->idoPtr = idoPtr;
        }
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  DelegationInstall()
 * ------------------------------------------------------------------------
 */

int
DelegationInstall(
    Tcl_Interp *interp,
    ItclObject *ioPtr,
    ItclClass *iclsPtr)
{
    Tcl_HashEntry *hPtr2;
    Tcl_HashSearch search2;
    Tcl_Obj *componentNamePtr;
    ItclDelegatedFunction *idmPtr;
    ItclMemberFunc *imPtr;
    FOREACH_HASH_DECLS;
    char *methodName;
    const char *val;
    int result;
    int noDelegate;
    int delegateAll;

    result = TCL_OK;
    delegateAll = 0;
    noDelegate = ITCL_CONSTRUCTOR|ITCL_DESTRUCTOR|ITCL_COMPONENT;
    componentNamePtr = NULL;
    FOREACH_HASH_VALUE(idmPtr, &iclsPtr->delegatedFunctions) {
	methodName = Tcl_GetString(idmPtr->namePtr);
	if (*methodName == '*') {
	    delegateAll = 1;
	}
	if (idmPtr->icPtr != NULL) {
            val = Itcl_GetInstanceVar(interp,
	            Tcl_GetString(idmPtr->icPtr->namePtr), ioPtr, iclsPtr);
	    componentNamePtr = Tcl_NewStringObj(val, -1);
            Tcl_IncrRefCount(componentNamePtr);
	} else {
	    componentNamePtr = NULL;
	}
	if (!delegateAll) {
	    result = DelegateFunction(interp, ioPtr, iclsPtr,
	            componentNamePtr, idmPtr);
	    if (result != TCL_OK) {
	        return result;
	    }
	} else {
	    /* save to allow nested FOREACH */
            search2 = search;
            FOREACH_HASH_VALUE(imPtr, &iclsPtr->functions) {
	        methodName = Tcl_GetString(imPtr->namePtr);
                if (imPtr->flags & noDelegate) {
		    continue;
		}
                if (strcmp(methodName, "info") == 0) {
	            continue;
	        }
                if (strcmp(methodName, "isa") == 0) {
	            continue;
	        }
                hPtr2 = Tcl_FindHashEntry(&idmPtr->exceptions,
		        (char *)imPtr->namePtr);
                if (hPtr2 != NULL) {
		    continue;
		}
	        result = DelegateFunction(interp, ioPtr, iclsPtr,
	                componentNamePtr, idmPtr);
	        if (result != TCL_OK) {
	            break;
	        }
            }
            search = search2;
        }
	if (componentNamePtr != NULL) {
            Tcl_DecrRefCount(componentNamePtr);
        }
    }
    result = DelegatedOptionsInstall(interp, iclsPtr);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  ItclInitExtendedClassOptions()
 * ------------------------------------------------------------------------
 */

static int
ItclInitExtendedClassOptions(
    Tcl_Interp *interp,
    ItclObject *ioPtr)
{
    ItclClass *iclsPtr;
    ItclOption *ioptPtr;
    ItclHierIter hier;
    FOREACH_HASH_DECLS;

    iclsPtr = ioPtr->iclsPtr;
    Itcl_InitHierIter(&hier, iclsPtr);
    while ((iclsPtr = Itcl_AdvanceHierIter(&hier)) != NULL) {
        FOREACH_HASH_VALUE(ioptPtr, &iclsPtr->options) {
            if (ioptPtr->init != NULL) {
                ItclSetInstanceVar(interp, "itcl_options",
                        Tcl_GetString(ioptPtr->namePtr),
                        Tcl_GetString(ioptPtr->init), ioPtr, iclsPtr);
            }
	}
    }
    Itcl_DeleteHierIter(&hier);
    return TCL_OK;
}
