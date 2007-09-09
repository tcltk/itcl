/*
 * itclInt.h --
 *
 * This file contains internal definitions for the C-implemented part of a
 * Itcl
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclInt.h,v 1.17.2.4 2007/09/09 11:04:14 wiede Exp $
 */

#include <string.h>
#include <ctype.h>
#include <tcl.h>
#include <tclOO.h>
#include "itclMigrate2TclCore.h"
#include "itclNeededFromTclOO.h"
#include "itcl.h"

/*
 * Used to tag functions that are only to be visible within the module being
 * built and not outside it (where this is supported by the linker).
 */

#ifndef MODULE_SCOPE
#   ifdef __cplusplus
#       define MODULE_SCOPE extern "C"
#   else
#       define MODULE_SCOPE extern
#   endif
#endif

/*
 * Since the Tcl/Tk distribution doesn't perform any asserts,
 * dynamic loading can fail to find the __assert function.
 * As a workaround, we'll include our own.
 */

#undef  assert
#define DEBUG 1
#ifndef  DEBUG
#define assert(EX) ((void)0)
#else
#define assert(EX) (void)((EX) || (Itcl_Assert(STRINGIFY(EX), __FILE__, __LINE__), 0))
#endif  /* DEBUG */

#define ITCL_INTERP_DATA "itcl_data"

/*
 * Convenience macros for iterating through hash tables. FOREACH_HASH_DECLS
 * sets up the declarations needed for the main macro, FOREACH_HASH, which
 * does the actual iteration. FOREACH_HASH_VALUE is a restricted version that
 * only iterates over values.
 */

#define FOREACH_HASH_DECLS \
    Tcl_HashEntry *hPtr;Tcl_HashSearch search
#define FOREACH_HASH(key,val,tablePtr) \
    for(hPtr=Tcl_FirstHashEntry((tablePtr),&search); hPtr!=NULL ? \
            ((key)=(void *)Tcl_GetHashKey((tablePtr),hPtr),\
            (val)=Tcl_GetHashValue(hPtr),1):0; hPtr=Tcl_NextHashEntry(&search))
#define FOREACH_HASH_VALUE(val,tablePtr) \
    for(hPtr=Tcl_FirstHashEntry((tablePtr),&search); hPtr!=NULL ? \
            ((val)=Tcl_GetHashValue(hPtr),1):0;hPtr=Tcl_NextHashEntry(&search))

/*
 * What sort of size of things we like to allocate.
 */

#define ALLOC_CHUNK 8

#define ITCL_VARIABLES_NAMESPACE "::itcl::variables"
#define ITCL_COMMANDS_NAMESPACE "::itcl::commands"

typedef struct ItclFoundation {
    Itcl_Stack methodCallStack;
    Tcl_Command dispatchCommand;
} ItclFoundation;

typedef struct ItclArgList {
    struct ItclArgList *nextPtr;        /* pointer to next argument */
    Tcl_Obj *namePtr;           /* name of the argument */
    Tcl_Obj *defaultValuePtr;   /* default value or NULL if none */
} ItclArgList;

/*
 *  Common info for managing all known objects.
 *  Each interpreter has one of these data structures stored as
 *  clientData in the "itcl" namespace.  It is also accessible
 *  as associated data via the key ITCL_INTERP_DATA.
 */
struct ItclObject;
struct ItclMemberFunc;
struct EnsembleInfo;

typedef struct ItclObjectInfo {
    Tcl_Interp *interp;             /* interpreter that manages this info */
    Tcl_HashTable objects;          /* list of all known objects */
    Tcl_HashTable classes;          /* list of all known classes */
    Tcl_HashTable namespaceClasses; /* maps from nsPtr to iclsPtr */
    Tcl_HashTable procMethods;      /* maps from procPtr to mFunc */
    int protection;                 /* protection level currently in effect */
    int useOldResolvers;            /* whether to use the "old" style
                                     * resolvers or the CallFrame resolvers */
    Itcl_Stack clsStack;            /* stack of class definitions currently
                                     * being parsed */
    Itcl_Stack contextStack;        /* stack of call contexts */
    Itcl_Stack constructorStack;    /* stack of constructor calls */
    struct ItclObject *currIoPtr;   /* object currently being constructed
                                     * set only during calling of constructors
				     * otherwise NULL */
    Tcl_ObjectMetadataType *class_meta_type;
                                    /* type for getting the Itcl class info
                                     * from a TclOO Tcl_Object */
    Tcl_ObjectMetadataType *object_meta_type;
                                    /* type for getting the Itcl object info
                                     * from a TclOO Tcl_Object */
    struct EnsembleInfo *ensembleInfo;
} ItclObjectInfo;

typedef struct EnsembleInfo {
    Tcl_HashTable ensembles;        /* list of all known ensembles */
    Tcl_HashTable subEnsembles;     /* list of all known subensembles */
    int numEnsembles;
    Tcl_Namespace *ensembleNsPtr;
} EnsembleInfo;
/*
 *  Representation for each [incr Tcl] class.
 */
typedef struct ItclClass {
    Tcl_Obj *name;                /* class name */
    Tcl_Obj *fullname;            /* fully qualified class name */
    Tcl_Interp *interp;           /* interpreter that manages this info */
    Tcl_Namespace *namesp;        /* namespace representing class scope */
    Tcl_Command accessCmd;        /* access command for creating instances */

    struct ItclObjectInfo *info;  /* info about all known objects */
    Itcl_List bases;              /* list of base classes */
    Itcl_List derived;            /* list of all derived classes */
    Tcl_HashTable heritage;       /* table of all base classes.  Look up
                                   * by pointer to class definition.  This
                                   * provides fast lookup for inheritance
                                   * tests. */
    Tcl_Obj *initCode;            /* initialization code for new objs */
    Tcl_HashTable variables;      /* definitions for all data members
                                     in this class.  Look up simple string
                                     names and get back ItclVarDefn* ptrs */
    Tcl_HashTable functions;      /* definitions for all member functions
                                     in this class.  Look up simple string
                                     names and get back ItclMemberFunc* ptrs */
    int numInstanceVars;          /* number of instance vars in variables
                                     table */
    Tcl_HashTable classCommons;   /* used for storing variable namespace string for Tcl_Resolve */
    Tcl_HashTable resolveVars;    /* all possible names for variables in
                                   * this class (e.g., x, foo::x, etc.) */
    Tcl_HashTable resolveCmds;    /* all possible names for functions in
                                   * this class (e.g., x, foo::x, etc.) */
    Tcl_HashTable contextCache;   /* cache for function contexts */
    struct ItclMemberFunc *constructor;  /* the class constructor or NULL */
    struct ItclMemberFunc *destructor;   /* the class destructor or NULL */
    struct ItclMemberFunc *constructorInit;  /* the class constructor init code or NULL */
    Tcl_Resolve *resolvePtr;
    Tcl_Object oPtr;		  /* TclOO class object */
    Tcl_Class  classPtr;          /* TclOO class */
    int numCommons;               /* number of commons in this class */
    int numVariables;             /* number of variables in this class */
    int unique;                   /* unique number for #auto generation */
    int flags;                    /* maintains class status */
} ItclClass;

typedef struct ItclHierIter {
    ItclClass *current;           /* current position in hierarchy */
    Itcl_Stack stack;             /* stack used for traversal */
} ItclHierIter;

/*
 *  Representation for each [incr Tcl] object.
 */
typedef struct ItclObject {
    ItclClass *iclsPtr;          /* most-specific class */
    Tcl_Command accessCmd;       /* object access command */

    Tcl_HashTable* constructed;  /* temp storage used during construction */
    Tcl_HashTable* destructed;   /* temp storage used during destruction */
    Tcl_HashTable objectVariables; /* used for storing Tcl_Var entries for variable resolving, key is ivPtr of variable, value is varPtr */
    Tcl_HashTable contextCache;   /* cache for function contexts */
    Tcl_Obj *namePtr;
    Tcl_Obj *varNsNamePtr;
    Tcl_Object oPtr;             /* the TclOO object */
    Tcl_Resolve *resolvePtr;
    int flags;
} ItclObject;

#define ITCL_OBJECT_IS_DELETED           0x01
#define ITCL_OBJECT_IS_DESTRUCTED        0x02
#define ITCL_OBJECT_IS_RENAMED           0x04
#define ITCL_TCLOO_OBJECT_IS_DELETED     0x10
#define ITCL_OBJECT_NO_VARNS_DELETE      0x20
#define ITCL_OBJECT_SHOULD_VARNS_DELETE  0x40
#define ITCL_CLASS_NO_VARNS_DELETE       0x100
#define ITCL_CLASS_SHOULD_VARNS_DELETE   0x200

#define ITCL_IGNORE_ERRS  0x002  /* useful for construction/destruction */

typedef struct ItclResolveInfo {
    int flags;
    ItclClass *iclsPtr;
    ItclObject *ioPtr;
} ItclResolveInfo;

#define ITCL_RESOLVE_CLASS		0x01
#define ITCL_RESOLVE_OBJECT		0x02

/*
 *  Implementation for any code body in an [incr Tcl] class.
 */
typedef struct ItclMemberCode {
    int flags;                  /* flags describing implementation */
    int argcount;               /* number of args in arglist */
    int maxargcount;            /* max number of args in arglist */
    Tcl_Obj *usagePtr;          /* usage string for error messages */
    Tcl_Obj *argumentPtr;       /* the function arguments */
    Tcl_Obj *bodyPtr;           /* the function body */
    ItclArgList *argListPtr;    /* the parsed arguments */
    union {
        Tcl_CmdProc *argCmd;    /* (argc,argv) C implementation */
        Tcl_ObjCmdProc *objCmd; /* (objc,objv) C implementation */
    } cfunc;
    ClientData clientData;      /* client data for C implementations */
} ItclMemberCode;

/*
 *  Flag bits for ItclMemberCode:
 */
#define ITCL_IMPLEMENT_NONE    0x001  /* no implementation */
#define ITCL_IMPLEMENT_TCL     0x002  /* Tcl implementation */
#define ITCL_IMPLEMENT_ARGCMD  0x004  /* (argc,argv) C implementation */
#define ITCL_IMPLEMENT_OBJCMD  0x008  /* (objc,objv) C implementation */
#define ITCL_IMPLEMENT_C       0x00c  /* either kind of C implementation */

#define Itcl_IsMemberCodeImplemented(mcode) \
    (((mcode)->flags & ITCL_IMPLEMENT_NONE) == 0)

/*
 *  Flag bits for ItclMember:
 */
#define ITCL_CONSTRUCTOR       0x010  /* non-zero => is a constructor */
#define ITCL_DESTRUCTOR        0x020  /* non-zero => is a destructor */
#define ITCL_COMMON            0x040  /* non-zero => is a "proc" */
#define ITCL_ARG_SPEC          0x080  /* non-zero => has an argument spec */
#define ITCL_BODY_SPEC         0x100  /* non-zero => has an body spec */
#define ITCL_THIS_VAR          0x200  /* non-zero => built-in "this" variable */
#define ITCL_CONINIT           0x400  /* non-zero => is a constructor
                                       * init code */
#define ITCL_BUILTIN           0x800  /* non-zero => built-in method */

/*
 *  Representation of member functions in an [incr Tcl] class.
 */
typedef struct ItclMemberFunc {
    Tcl_Obj* namePtr;           /* member name */
    Tcl_Obj* fullNamePtr;       /* member name with "class::" qualifier */
    ItclClass* iclsPtr;         /* class containing this member */
    int protection;             /* protection level */
    int flags;                  /* flags describing member (see above) */
    ItclMemberCode *codePtr;    /* code associated with member */
    Tcl_Command accessCmd;       /* Tcl command installed for this function */
    int argcount;                /* number of args in arglist */
    int maxargcount;             /* max number of args in arglist */
    Tcl_Obj *usagePtr;          /* usage string for error messages */
    Tcl_Obj *argumentPtr;       /* the function arguments */
    Tcl_Obj *origArgsPtr;       /* the argument string of the original definition */
    Tcl_Obj *bodyPtr;           /* the function body */
    ItclArgList *argListPtr;    /* the parsed arguments */
    ItclClass *declaringClassPtr; /* the class which declared the method/proc */
    ClientData tmPtr;           /* TclOO methodPtr */
} ItclMemberFunc;

/*
 *  Instance variables.
 */
typedef struct ItclVariable {
    Tcl_Obj* namePtr;           /* member name */
    Tcl_Obj* fullNamePtr;       /* member name with "class::" qualifier */
    ItclClass* iclsPtr;         /* class containing this member */
    int protection;             /* protection level */
    int flags;                  /* flags describing member (see below) */
    ItclMemberCode *codePtr;    /* code associated with member */
    Tcl_Obj* init;              /* initial value */
} ItclVariable;


typedef struct IctlVarTraceInfo {
    int flags;
    ItclVariable* ivPtr;
    ItclClass *iclsPtr;
    ItclObject *ioPtr;
} IctlVarTraceInfo;

#define ITCL_TRACE_CLASS		0x01
#define ITCL_TRACE_OBJECT		0x02

/*
 *  Instance variable lookup entry.
 */
typedef struct ItclVarLookup {
    ItclVariable* ivPtr;      /* variable definition */
    int usage;                /* number of uses for this record */
    int accessible;           /* non-zero => accessible from class with
                               * this lookup record in its resolveVars */
    char *leastQualName;      /* simplist name for this variable, with
                               * the fewest qualifiers.  This string is
                               * taken from the resolveVars table, so
                               * it shouldn't be freed. */
} ItclVarLookup;

typedef struct ItclCallContext {
    int objectFlags;
    int classFlags;
    Tcl_Namespace *nsPtr;
    ItclObject *ioPtr;
    ItclClass *iclsPtr;
    ItclMemberFunc *imPtr;
    int refCount;
} ItclCallContext;

#ifdef ITCL_DEBUG
MODULE_SCOPE int _itcl_debug_level;
MODULE_SCOPE void ItclShowArgs(int level, const char *str, int objc,
	Tcl_Obj * const* objv);
#else
#define ItclShowArgs(a,b,c,d)
#endif

MODULE_SCOPE Tcl_ObjCmdProc ItclCallCCommand;
MODULE_SCOPE Tcl_ObjCmdProc ItclObjectUnknownCommand;
MODULE_SCOPE int ItclCheckCallProc(ClientData clientData, Tcl_Interp *interp,
        Tcl_ObjectContext contextPtr, Tcl_CallFrame *framePtr, int *isFinished);

MODULE_SCOPE ItclFoundation *ItclGetFoundation(Tcl_Interp *interp);
MODULE_SCOPE Tcl_ObjCmdProc ItclClassCommandDispatcher;
MODULE_SCOPE Tcl_Command Itcl_CmdAliasProc(Tcl_Interp *interp,
        Tcl_Namespace *nsPtr, CONST char *cmdName, ClientData clientData);
MODULE_SCOPE Tcl_Var Itcl_VarAliasProc(Tcl_Interp *interp,
        Tcl_Namespace *nsPtr, CONST char *VarName, ClientData clientData);
MODULE_SCOPE int ItclIsClass(Tcl_Interp *interp, Tcl_Command cmd);
MODULE_SCOPE int ItclCheckCallMethod(ClientData clientData, Tcl_Interp *interp,
        Tcl_ObjectContext contextPtr, Tcl_CallFrame *framePtr, int *isFinished);
MODULE_SCOPE int ItclAfterCallMethod(ClientData clientData, Tcl_Interp *interp,
        Tcl_ObjectContext contextPtr, Tcl_Namespace *nsPtr, int result);
MODULE_SCOPE void ItclReportObjectUsage(Tcl_Interp *interp,
        ItclObject *contextIoPtr, Tcl_Namespace *callerNsPtr,
	Tcl_Namespace *contextNsPtr);
MODULE_SCOPE void ItclGetInfoUsage(Tcl_Interp *interp, Tcl_Obj *objPtr);
MODULE_SCOPE int ItclMapCmdNameProc(ClientData clientData, Tcl_Interp *interp,
        Tcl_Obj *mappedCmd, Tcl_Class *clsPtr);
MODULE_SCOPE int ItclCreateArgList(Tcl_Interp *interp, const char *str,
        int *argcPtr, int *maxArgcPtr, Tcl_Obj **usagePtr,
	ItclArgList **arglistPtrPtr, ItclMemberFunc *imPtr,
	const char *commandName);
MODULE_SCOPE int ItclObjectCmd(ClientData clientData, Tcl_Interp *interp,
        Tcl_Object oPtr, Tcl_Class clsPtr, int objc, Tcl_Obj *const *objv);
MODULE_SCOPE int ItclCreateObject (Tcl_Interp *interp, CONST char* name,
        ItclClass *iclsPtr, int objc, Tcl_Obj *CONST objv[]);
MODULE_SCOPE void ItclDeleteObjectVariablesNamespace(Tcl_Interp *interp,
        ItclObject *ioPtr);
MODULE_SCOPE void ItclDeleteClassVariablesNamespace(Tcl_Interp *interp,
        ItclClass *iclsPtr);
MODULE_SCOPE int ItclInfoInit(Tcl_Interp *interp);
MODULE_SCOPE char * ItclTraceUnsetVar(ClientData clientData, Tcl_Interp *interp,
	const char *name1, const char *name2, int flags);

struct Tcl_ResolvedVarInfo;
MODULE_SCOPE int Itcl_ClassCmdResolver(Tcl_Interp *interp, CONST char* name,
	Tcl_Namespace *nsPtr, int flags, Tcl_Command *rPtr);
MODULE_SCOPE int Itcl_ClassVarResolver(Tcl_Interp *interp, CONST char* name,
        Tcl_Namespace *nsPtr, int flags, Tcl_Var *rPtr);
MODULE_SCOPE int Itcl_ClassCompiledVarResolver(Tcl_Interp *interp,
        CONST char* name, int length, Tcl_Namespace *nsPtr,
        struct Tcl_ResolvedVarInfo **rPtr);
MODULE_SCOPE int ItclSetParserResolver(Tcl_Namespace *nsPtr);
MODULE_SCOPE void ItclProcErrorProc(Tcl_Interp *interp, Tcl_Obj *procNameObj);

#include "itcl2TclOO.h"

/*
 * Include all the private API, generated from tclOO.decls.
 */

#include "itclIntDecls.h"
