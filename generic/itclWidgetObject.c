/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 * Implementation of objects for package ItclWidget
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetObject.c,v 1.1.2.8 2007/09/22 13:39:23 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007 Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclInt.h"


/*
 * ------------------------------------------------------------------------
 *  ItclInitObjectOptions()
 *
 *  Init all instance options 
 *  This is usually invoked automatically
 *  by Itcl_CreateObject(), when an object is created.
 * ------------------------------------------------------------------------
 */
int
ItclInitObjectOptions(
   Tcl_Interp *interp,
   ItclObject *ioPtr,
   ItclClass *iclsPtr,
   const char *name)
{
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  HullAndOptionsInstall()
 * ------------------------------------------------------------------------
 */

int
HullAndOptionsInstall(
    Tcl_Interp *interp,
    ItclObject *ioPtr,
    ItclClass *iclsPtr,
    int objc,
    Tcl_Obj * const objv[],
    int *newObjc,
    Tcl_Obj **newObjv)
{
    FOREACH_HASH_DECLS;
    Tcl_Obj *widgetClassPtr;
    Tcl_Obj **hullObjv;
    ItclOption *ioptPtr;
    char *token;
    const char *hullType;
    int hullObjc;
    int foundWclass;
    int result;
    int i;

    ItclShowArgs(1, "HullAndOptionsInstall", objc, objv);
    FOREACH_HASH_VALUE(ioptPtr, &iclsPtr->options) {
	if (ioptPtr->init != NULL) {
	    ItclSetInstanceVar(interp, "options",
	            Tcl_GetString(ioptPtr->namePtr),
		    Tcl_GetString(ioptPtr->init), ioPtr, iclsPtr);
	}
        
    }
    widgetClassPtr = iclsPtr->widgetClassPtr;
    foundWclass = 0;
    iclsPtr->infoPtr->buildingWidget = 1;
    iclsPtr->infoPtr->currIoPtr = ioPtr;
    for (i = 0; i < objc; i++) {
        token = Tcl_GetString(objv[i]);
	if ((*token == '-') && (strcmp(token, "-class") == 0)) {
	    /* check if it is in the -option position */
	    if (((i % 2) == 0) && (i + 1 <= objc)) {
		widgetClassPtr = objv[i+1];
		foundWclass = 1;
	        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc-1));
		*newObjc = objc - 2;
		memcpy(newObjv, objv, i * sizeof(Tcl_Obj *));
		if (objc-i-2 > 0) {
		    memcpy(newObjv+i, objv+i+2, (objc-i-2)*sizeof(Tcl_Obj *));
		}
	    }
	}
    }
    if (widgetClassPtr == NULL) {
	char buf[2];
	char *cp;
	cp = Tcl_GetString(iclsPtr->namePtr);
        widgetClassPtr = Tcl_NewStringObj("", -1);
	buf[0] = toupper(*cp);
	buf[1] = '\0';
	Tcl_AppendToObj(widgetClassPtr, buf, -1);
	Tcl_AppendToObj(widgetClassPtr, cp+1, -1);
	Tcl_IncrRefCount(widgetClassPtr);
    }
    hullObjc = 5;
    hullObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*hullObjc);
    hullObjv[0] = Tcl_NewStringObj("hullinstall", -1);
    Tcl_IncrRefCount(hullObjv[0]);
    hullObjv[1] = Tcl_NewStringObj("using", -1);
    Tcl_IncrRefCount(hullObjv[1]);
    if (iclsPtr->flags & ITCL_WIDGET_IS_FRAME) {
        hullType = "frame";
    }
    if (iclsPtr->flags & ITCL_WIDGET_IS_LABEL_FRAME) {
        hullType = "labelframe";
    }
    if (iclsPtr->flags & ITCL_WIDGET_IS_TOPLEVEL) {
        hullType = "toplevel";
    }
    if (iclsPtr->flags & ITCL_WIDGET_IS_TTK_FRAME) {
        hullType = "ttk::frame";
    }
    if (iclsPtr->flags & ITCL_WIDGET_IS_TTK_LABEL_FRAME) {
        hullType = "ttk::labelframe";
    }
    if (iclsPtr->flags & ITCL_WIDGET_IS_TTK_TOPLEVEL) {
        hullType = "ttk::toplevel";
    }
    hullObjv[2] = Tcl_NewStringObj(hullType, -1);
    Tcl_IncrRefCount(hullObjv[2]);
    hullObjv[3] = Tcl_NewStringObj("-class", -1);
    Tcl_IncrRefCount(hullObjv[3]);
    hullObjv[4] = Tcl_NewStringObj(Tcl_GetString(widgetClassPtr), -1);
    Tcl_IncrRefCount(hullObjv[4]);

    result = Itcl_BiHullInstallCmd(iclsPtr, interp, hullObjc, hullObjv);
    Tcl_DecrRefCount(hullObjv[0]);
    Tcl_DecrRefCount(hullObjv[1]);
    Tcl_DecrRefCount(hullObjv[2]);
    Tcl_DecrRefCount(hullObjv[3]);
    Tcl_DecrRefCount(hullObjv[4]);
    iclsPtr->infoPtr->buildingWidget = 0;
    return result;
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
    Tcl_Obj *listPtr;;
    Tcl_Obj *componentNamePtr;
    ItclMemberFunc *imPtr;
    ItclDelegatedFunction *idmPtr;
    FOREACH_HASH_DECLS;
    char *methodName;
    const char *val;
    const char **argv;
    int argc;
    int noDelegate;
    int delegateAll;
    int j;

    delegateAll = 0;
    noDelegate = ITCL_CONSTRUCTOR|ITCL_DESTRUCTOR|ITCL_COMPONENT;
    componentNamePtr = NULL;
    FOREACH_HASH_VALUE(idmPtr, &iclsPtr->delegatedFunctions) {
	/* save to allow nested FOREACH */
	if (*Tcl_GetString(idmPtr->namePtr) == '*') {
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
        search2 = search;
        FOREACH_HASH_VALUE(imPtr, &iclsPtr->functions) {
	    methodName = Tcl_GetString(imPtr->namePtr);
            if (delegateAll) {
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
	    } else {
	        if (strcmp(methodName, Tcl_GetString(idmPtr->namePtr)) != 0) {
		    continue;
		}
	    }
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
	            Tcl_ListObjAppendElement(interp, listPtr, imPtr->namePtr);
	        }
	    }
	    Tcl_IncrRefCount(imPtr->namePtr);
	    /* and now for the argument */
	    Tcl_IncrRefCount(imPtr->namePtr);
            Tcl_Method mPtr;
	    mPtr = Itcl_NewForwardClassMethod(interp, iclsPtr->clsPtr, 1,
	            imPtr->namePtr, listPtr);
        }
        search = search2;
        Tcl_DecrRefCount(componentNamePtr);
    }
    return TCL_OK;
}
