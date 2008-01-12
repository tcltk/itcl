/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  This file contains procedures that use the internal Tcl core stubs
 *  entries.
 *
 * ========================================================================
 *  AUTHOR:  Arnulf Wiedemann
 *
 *     RCS:  $Id: itclngTclIntStubsFcn.c,v 1.1.2.1 2008/01/12 23:39:48 wiede Exp $
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include <tcl.h>
#include <tclInt.h>
#include "itclngTclIntStubsFcn.h"

Tcl_Command
_Tcl_GetOriginalCommand(
    Tcl_Command command) 
{
    return TclGetOriginalCommand(command);
}

int
_Tcl_CreateProc(
    Tcl_Interp *interp,         /* Interpreter containing proc. */
    Tcl_Namespace *nsPtr,       /* Namespace containing this proc. */
    CONST char *procName,       /* Unqualified name of this proc. */
    Tcl_Obj *argsPtr,           /* Description of arguments. */
    Tcl_Obj *bodyPtr,           /* Command body. */
    Tcl_Proc *procPtrPtr)       /* Returns: pointer to proc data. */
{
    return TclCreateProc(interp, (Namespace *)nsPtr, procName, argsPtr,
            bodyPtr, (Proc **)procPtrPtr);
}

void *
_Tcl_GetObjInterpProc(
    void)
{
    return (void *)TclGetObjInterpProc();
}

void
_Tcl_ProcDeleteProc(
    ClientData clientData)
{
    TclProcDeleteProc(clientData);
}

int
Itclng_RenameCommand(
    Tcl_Interp *interp,
    const char *oldName,
    const char *newName)
{
    return TclRenameCommand(interp, oldName, newName);
}

int
Itclng_PushCallFrame(
    Tcl_Interp * interp,
    Tcl_CallFrame * framePtr,
    Tcl_Namespace * nsPtr,
    int isProcCallFrame)
{
    return Tcl_PushCallFrame(interp, framePtr, nsPtr, isProcCallFrame);
}

void
Itclng_PopCallFrame(
    Tcl_Interp * interp)
{
    Tcl_PopCallFrame(interp);
}

void
Itclng_GetVariableFullName(
    Tcl_Interp * interp,
    Tcl_Var variable,
    Tcl_Obj * objPtr)
{
    Tcl_GetVariableFullName(interp, variable, objPtr);
}

Tcl_Var
Itclng_FindNamespaceVar(
    Tcl_Interp * interp,
    CONST char * name,
    Tcl_Namespace * contextNsPtr,
    int flags)
{
    return Tcl_FindNamespaceVar(interp, name, contextNsPtr, flags);
}

void
Itclng_SetNamespaceResolvers (
    Tcl_Namespace * namespacePtr,
    Tcl_ResolveCmdProc * cmdProc,
    Tcl_ResolveVarProc * varProc,
    Tcl_ResolveCompiledVarProc * compiledVarProc)
{
    Tcl_SetNamespaceResolvers(namespacePtr, cmdProc, varProc, compiledVarProc);
}


