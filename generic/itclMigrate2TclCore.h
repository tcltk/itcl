#ifndef ITCL_USE_MODIFIED_TCL_H
/* this is just to provide the definition. This struct is only used if
 * infoPtr->useOldResolvers == 0 which is not the default
 */
#define FRAME_HAS_RESOLVER 0x100
struct Tcl_Resolve;
struct Tcl_Var;
typedef Tcl_Command (Tcl_CmdAliasProc)(Tcl_Interp *interp,
        Tcl_Namespace *nsPtr, CONST char *cmdName,
        ClientData clientData);
typedef Tcl_Var (Tcl_VarAliasProc)(Tcl_Interp *interp,
        Tcl_Namespace *nsPtr, CONST char *varName,
        ClientData clientData);

typedef struct Tcl_Resolve {
    Tcl_VarAliasProc *varProcPtr;
    Tcl_CmdAliasProc *cmdProcPtr;
    ClientData clientData;
} Tcl_Resolve;
#endif

#ifndef _TCLINT
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
	CONST84 char *name, int length, Tcl_Namespace *context,
	Tcl_ResolvedVarInfo **rPtr);

typedef int (Tcl_ResolveVarProc) (Tcl_Interp *interp, CONST84 char *name,
	Tcl_Namespace *context, int flags, Tcl_Var *rPtr);

typedef int (Tcl_ResolveCmdProc) (Tcl_Interp *interp, CONST84 char *name,
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

/* these functions exist as Tcl_* but are NOT PUBLISHED so make an Itcl_* wrapper */
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
/* end of functions that exist but are NOT PUBLISHED */


/* here come the definitions for code which should be migrated to Tcl core */
/* these functions DO NOT exist and are not published */
typedef struct Tcl_Proc_ *Tcl_Proc;

typedef void (*Tcl_ProcErrorProc)(Tcl_Interp *interp, Tcl_Obj *procNameObj);

#define Tcl_GetOriginalCommand _Tcl_GetOriginalCommand
#define Tcl_GetNamespaceCommandTable _Tcl_GetNamespaceCommandTable
#define Tcl_GetNamespaceChildTable _Tcl_GetNamespaceChildTable
#define Tcl_InitRewriteEnsemble _Tcl_InitRewriteEnsemble
#define Tcl_ResetRewriteEnsemble _Tcl_ResetRewriteEnsemble
#define Tcl_CreateProc _Tcl_CreateProc
#define Tcl_ProcDeleteProc _Tcl_ProcDeleteProc
#define Tcl_GetObjInterpProc _Tcl_GetObjInterpProc
#define Tcl_SetProcCmd _Tcl_SetProcCmd
#define Tcl_SetNamespaceResolver _Tcl_SetNamespaceResolver
#define Tcl_InvokeNamespaceProc _Tcl_InvokeNamespaceProc
#define Tcl_RenameCommand Tcl_RenameCommand


EXTERN Tcl_Command _Tcl_GetOriginalCommand(Tcl_Command command);
EXTERN Tcl_HashTable *_Tcl_GetNamespaceChildTable(Tcl_Namespace *nsPtr);

EXTERN Tcl_HashTable *_Tcl_GetNamespaceCommandTable(Tcl_Namespace *nsPtr);
EXTERN int _Tcl_InitRewriteEnsemble(Tcl_Interp *interp, int numRemoved,
	int numInserted, int objc, Tcl_Obj *const *objv);
EXTERN void _Tcl_ResetRewriteEnsemble(Tcl_Interp *interp,
        int isRootEnsemble);
EXTERN int _Tcl_CreateProc(Tcl_Interp *interp, Tcl_Namespace *nsPtr,
	CONST char *procName, Tcl_Obj *argsPtr, Tcl_Obj *bodyPtr,
        Tcl_Proc *procPtrPtr);
EXTERN void _Tcl_ProcDeleteProc(ClientData clientData);
EXTERN void *_Tcl_GetObjInterpProc(void);
EXTERN int _Tcl_SetNamespaceResolver(Tcl_Namespace *nsPtr,
        Tcl_Resolve *resolvePtr);
EXTERN int _Tcl_InvokeNamespaceProc(Tcl_Interp *interp, Tcl_Proc proc,
        Tcl_Namespace *nsPtr, Tcl_Obj *namePtr, int objc, Tcl_Obj *const *objv);
EXTERN Tcl_Var Tcl_NewNamespaceVar(Tcl_Interp *interp, Tcl_Namespace *nsPtr,
	const char *varName);
EXTERN int Itcl_IsCallFrameArgument(Tcl_Interp *interp, const char *name);
EXTERN int Itcl_ProcessReturn(Tcl_Interp *interp, int code, int level,
        Tcl_Obj *returnOpts);
EXTERN int ItclGetInterpErrorLine(Tcl_Interp *interp);
EXTERN int Tcl_RenameCommand(Tcl_Interp *interp, const char *oldName,
	const char *newName);


/* end migration code */

