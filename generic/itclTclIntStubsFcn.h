/* these functions are Tcl internal stubs so make an Itcl_* wrapper */
EXTERN int Itcl_PushCallFrame (Tcl_Interp * interp,
                Tcl_CallFrame * framePtr,
	        Tcl_Namespace * nsPtr, int isProcCallFrame);
EXTERN void Itcl_PopCallFrame (Tcl_Interp * interp);
EXTERN void Itcl_GetVariableFullName (Tcl_Interp * interp,
                Tcl_Var variable, Tcl_Obj * objPtr);
EXTERN Tcl_Var Itcl_FindNamespaceVar (Tcl_Interp * interp,
                CONST char * name, Tcl_Namespace * contextNsPtr, int flags);
EXTERN void Itcl_SetNamespaceResolvers (Tcl_Namespace * namespacePtr,
        Tcl_ResolveCmdProc * cmdProc, Tcl_ResolveVarProc * varProc,
        Tcl_ResolveCompiledVarProc * compiledVarProc);

#ifndef _TCL_PROC_DEFINED
typedef struct Tcl_Proc_ *Tcl_Proc;
#define _TCL_PROC_DEFINED 1
#endif
#ifndef _TCL_RESOLVE_DEFINED
struct Tcl_Resolve;
#endif

#define Tcl_GetOriginalCommand _Tcl_GetOriginalCommand
#define Tcl_CreateProc _Tcl_CreateProc
#define Tcl_ProcDeleteProc _Tcl_ProcDeleteProc
#define Tcl_GetObjInterpProc _Tcl_GetObjInterpProc
#define Tcl_SetNamespaceResolver _Tcl_SetNamespaceResolver

EXTERN Tcl_Command _Tcl_GetOriginalCommand(Tcl_Command command);
EXTERN int _Tcl_CreateProc(Tcl_Interp *interp, Tcl_Namespace *nsPtr,
	CONST char *procName, Tcl_Obj *argsPtr, Tcl_Obj *bodyPtr,
        Tcl_Proc *procPtrPtr);
EXTERN void _Tcl_ProcDeleteProc(ClientData clientData);
EXTERN void *_Tcl_GetObjInterpProc(void);
EXTERN int _Tcl_SetNamespaceResolver(Tcl_Namespace *nsPtr,
        struct Tcl_Resolve *resolvePtr);
EXTERN int Tcl_RenameCommand(Tcl_Interp *interp, const char *oldName,
	const char *newName);


