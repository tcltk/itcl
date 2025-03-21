# -*- tcl -*-

# public API
library itcl
interface itcl
hooks {itclInt}
epoch 0
scspec ITCLAPI

# Declare each of the functions in the public Tcl interface.  Note that
# the an index should never be reused for a different function in order
# to preserve backwards compatibility.

declare 0 {
    int Itcl_RegisterObjC2(Tcl_Interp *interp, const char *name,
	Tcl_ObjCmdProc2 *proc, void *clientData,
	Tcl_CmdDeleteProc *deleteProc)
}
declare 1 {
    int Itcl_FindC2(Tcl_Interp *interp, const char *name,
	Tcl_ObjCmdProc2 **objProcPtr,
	void **cDataPtr)
}
declare 2 {
    int Itcl_RegisterC(Tcl_Interp *interp, const char *name,
	Tcl_CmdProc *proc, void *clientData,
	Tcl_CmdDeleteProc *deleteProc)
}
declare 3 {
    int Itcl_RegisterObjC(Tcl_Interp *interp, const char *name,
	Tcl_ObjCmdProc *proc, void *clientData,
	Tcl_CmdDeleteProc *deleteProc)
}
declare 4 {
    int Itcl_FindC(Tcl_Interp *interp, const char *name,
	Tcl_CmdProc **argProcPtr, Tcl_ObjCmdProc **objProcPtr,
	void **cDataPtr)
}
declare 5 {
    void Itcl_InitStack(Itcl_Stack *stack)
}
declare 6 {
    void Itcl_DeleteStack(Itcl_Stack *stack)
}
declare 7 {
    void Itcl_PushStack(void *cdata, Itcl_Stack *stack)
}
declare 8 {
    void *Itcl_PopStack(Itcl_Stack *stack)
}
declare 9 {
    void *Itcl_PeekStack(Itcl_Stack *stack)
}
declare 10 {
    void *Itcl_GetStackValue(Itcl_Stack *stack, Tcl_Size pos)
}
declare 11 {
    void Itcl_InitList(Itcl_List *listPtr)
}
declare 12 {
    void Itcl_DeleteList(Itcl_List *listPtr)
}
declare 13 {
    Itcl_ListElem *Itcl_CreateListElem(Itcl_List *listPtr)
}
declare 14 {
    Itcl_ListElem *Itcl_DeleteListElem(Itcl_ListElem *elemPtr)
}
declare 15 {
    Itcl_ListElem *Itcl_InsertList(Itcl_List *listPtr, void *val)
}
declare 16 {
    Itcl_ListElem *Itcl_InsertListElem(Itcl_ListElem *pos, void *val)
}
declare 17 {
    Itcl_ListElem *Itcl_AppendList(Itcl_List *listPtr, void *val)
}
declare 18 {
    Itcl_ListElem *Itcl_AppendListElem(Itcl_ListElem *pos, void *val)
}
declare 19 {
    void Itcl_SetListValue(Itcl_ListElem *elemPtr, void *val)
}
declare 20 {
    void Itcl_EventuallyFree(void *cdata, Tcl_FreeProc *fproc)
}
declare 21 {
    void Itcl_PreserveData(void *cdata)
}
declare 22 {
    void Itcl_ReleaseData(void *cdata)
}
declare 23 {
    Itcl_InterpState Itcl_SaveInterpState(Tcl_Interp *interp, int status)
}
declare 24 {
    int Itcl_RestoreInterpState(Tcl_Interp *interp, Itcl_InterpState state)
}
declare 25 {
    void Itcl_DiscardInterpState(Itcl_InterpState state)
}
declare 26 {
    void * Itcl_Alloc(size_t size)
}
declare 27 {
    void Itcl_Free(void *ptr)
}



# private API
interface itclInt
#
# Functions used within the package, but not considered "public"
#

declare 0 {
    int Itcl_IsClassNamespace(Tcl_Namespace *namesp)
}
declare 1 {
    int Itcl_IsClass(Tcl_Command cmd)
}
declare 2 {
    ItclClass *Itcl_FindClass(Tcl_Interp *interp, const char *path, int autoload)
}
declare 3 {
    int Itcl_FindObject(Tcl_Interp *interp, const char *name, ItclObject **roPtr)
}
declare 4 {
    int Itcl_IsObject(Tcl_Command cmd)
}
declare 5 {
    int Itcl_ObjectIsa(ItclObject *contextObj, ItclClass *cdefn)
}
declare 6 {
    int Itcl_Protection(Tcl_Interp *interp, int newLevel)
}
declare 7 {
    const char *Itcl_ProtectionStr(int pLevel)
}
declare 8 {
    int Itcl_CanAccess(ItclMemberFunc *memberPtr, Tcl_Namespace *fromNsPtr)
}
declare 9 {
    int Itcl_CanAccessFunc(ItclMemberFunc *mfunc, Tcl_Namespace *fromNsPtr)
}
declare 11 {
    void Itcl_ParseNamespPath(const char *name, Tcl_DString *buffer,
	const char **head, const char **tail)
}
declare 12 {
    int Itcl_DecodeScopedCommand(Tcl_Interp *interp, const char *name,
	Tcl_Namespace **rNsPtr, char **rCmdPtr)
}
declare 13 {
    int Itcl_EvalArgs(Tcl_Interp *interp, Tcl_Size objc, Tcl_Obj *const objv[])
}
declare 14 {
    Tcl_Obj *Itcl_CreateArgs(Tcl_Interp *interp, const char *string,
	Tcl_Size objc, Tcl_Obj *const objv[])
}
declare 17 {
    int Itcl_GetContext(Tcl_Interp *interp, ItclClass **iclsPtrPtr,
	ItclObject **ioPtrPtr)
}
declare 18 {
    void Itcl_InitHierIter(ItclHierIter *iter, ItclClass *iclsPtr)
}
declare 19 {
    void Itcl_DeleteHierIter(ItclHierIter *iter)
}
declare 20 {
    ItclClass *Itcl_AdvanceHierIter(ItclHierIter *iter)
}
declare 21 {
    int Itcl_FindClassesCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 22 {
    int Itcl_FindObjectsCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 24 {
    int Itcl_DelClassCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 25 {
    int Itcl_DelObjectCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 26 {
    int Itcl_ScopeCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 27 {
    int Itcl_CodeCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 28 {
    int Itcl_StubCreateCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 29 {
    int Itcl_StubExistsCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 30 {
    int Itcl_IsStub(Tcl_Command cmd)
}


#
#  Functions for manipulating classes
#

declare 31 {
    int Itcl_CreateClass(Tcl_Interp *interp, const char *path,
	ItclObjectInfo *info, ItclClass **rPtr)
}
declare 32 {
    int Itcl_DeleteClass(Tcl_Interp *interp, ItclClass *iclsPtr)
}
declare 33 {
    Tcl_Namespace *Itcl_FindClassNamespace(Tcl_Interp *interp, const char *path)
}
declare 34 {
    int Itcl_HandleClass(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 38 {
    void Itcl_BuildVirtualTables(ItclClass *iclsPtr)
}
declare 39 {
    int Itcl_CreateVariable(Tcl_Interp *interp, ItclClass *iclsPtr,
	Tcl_Obj *name, char *init, char *config, ItclVariable **ivPtr)
}
declare 40 {
    void Itcl_DeleteVariable(char *cdata)
}
declare 41 {
    const char *Itcl_GetCommonVar(Tcl_Interp *interp, const char *name,
	ItclClass *contextClass)
}


#
#  Functions for manipulating objects
#

declare 44 {
    int Itcl_CreateObject(Tcl_Interp *interp, const char* name, ItclClass *iclsPtr,
	Tcl_Size objc, Tcl_Obj *const objv[], ItclObject **rioPtr)
}
declare 45 {
    int Itcl_DeleteObject(Tcl_Interp *interp, ItclObject *contextObj)
}
declare 46 {
    int Itcl_DestructObject(Tcl_Interp *interp, ItclObject *contextObj,
	int flags)
}
declare 48 {
    const char *Itcl_GetInstanceVar(Tcl_Interp *interp, const char *name,
	ItclObject *contextIoPtr, ItclClass *contextIclsPtr)
}

#
#  Functions for manipulating methods and procs
#

declare 50 {
    int Itcl_BodyCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 51 {
    int Itcl_ConfigBodyCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 52 {
    int Itcl_CreateMethod(Tcl_Interp *interp, ItclClass *iclsPtr,
	Tcl_Obj *namePtr, const char *arglist, const char *body)
}
declare 53 {
    int Itcl_CreateProc(Tcl_Interp *interp, ItclClass *iclsPtr,
	Tcl_Obj *namePtr, const char *arglist, const char *body)
}
declare 54 {
    int Itcl_CreateMemberFunc(Tcl_Interp *interp, ItclClass *iclsPtr,
	Tcl_Obj *name, const char *arglist, const char *body,
	ItclMemberFunc **mfuncPtr)
}
declare 55 {
    int Itcl_ChangeMemberFunc(Tcl_Interp *interp, ItclMemberFunc *mfunc,
	const char *arglist, const char *body)
}
declare 56 {
    void Itcl_DeleteMemberFunc(void *cdata)
}
declare 57 {
    int Itcl_CreateMemberCode(Tcl_Interp *interp, ItclClass *iclsPtr, \
	const char *arglist, const char *body, ItclMemberCode **mcodePtr)
}
declare 58 {
    void Itcl_DeleteMemberCode(void *cdata)
}
declare 59 {
    int Itcl_GetMemberCode(Tcl_Interp *interp, ItclMemberFunc *mfunc)
}
declare 61 {
    int Itcl_EvalMemberCode(Tcl_Interp *interp, ItclMemberFunc *mfunc,
	ItclObject *contextObj, Tcl_Size objc, Tcl_Obj *const objv[])
}
declare 62 {
    int Itcl_ExecMethod2(void *clientData, Tcl_Interp *interp, Tcl_Size objc,
	Tcl_Obj *const objv[])
}
declare 63 {
    int Itcl_ExecProc2(void *clientData, Tcl_Interp *interp,
	Tcl_Size objc, Tcl_Obj *const objv[])
}
declare 67 {
    void Itcl_GetMemberFuncUsage(ItclMemberFunc *mfunc,
	ItclObject *contextObj, Tcl_Obj *objPtr)
}
declare 68 {
    int Itcl_ExecMethod(void *clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 69 {
    int Itcl_ExecProc(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 71 {
    int Itcl_ConstructBase(Tcl_Interp *interp, ItclObject *contextObj,
	ItclClass *contextClass)
}
declare 72 {
    int Itcl_InvokeMethodIfExists(Tcl_Interp *interp, const char *name,
	ItclClass *contextClass, ItclObject *contextObj, Tcl_Size objc,
	Tcl_Obj *const objv[])
}
declare 74 {
    int Itcl_ReportFuncErrors(Tcl_Interp *interp, ItclMemberFunc *mfunc,
	ItclObject *contextObj, int result)
}


#
#  Commands for parsing class definitions
#

declare 75 {
    int Itcl_ParseInit(Tcl_Interp *interp, ItclObjectInfo *info)
}
declare 76 {
    int Itcl_ClassCmd(void *clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 77 {
    int Itcl_ClassInheritCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 78 {
    int Itcl_ClassProtectionCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 79 {
    int Itcl_ClassConstructorCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 80 {
    int Itcl_ClassDestructorCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 81 {
    int Itcl_ClassMethodCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 82 {
    int Itcl_ClassProcCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 83 {
    int Itcl_ClassVariableCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 84 {
    int Itcl_ClassCommonCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 85 {
    int Itcl_ParseVarResolver(Tcl_Interp *interp, const char *name,
	Tcl_Namespace *contextNs, int flags, Tcl_Var *rPtr)
}

#
#  Commands in the "builtin" namespace
#

declare 86 {
    int Itcl_BiInit(Tcl_Interp *interp, ItclObjectInfo *infoPtr)
}
declare 87 {
    int Itcl_InstallBiMethods(Tcl_Interp *interp, ItclClass *cdefn)
}
declare 88 {
    int Itcl_BiIsaCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 89 {
    int Itcl_BiConfigureCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 90 {
    int Itcl_BiCgetCmd(void *clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 91 {
    int Itcl_BiChainCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 92 {
    int Itcl_BiInfoClassCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 93 {
    int Itcl_BiInfoInheritCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 94 {
    int Itcl_BiInfoHeritageCmd(void *dummy, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 95 {
    int Itcl_BiInfoFunctionCmd(void *dummy, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 96 {
    int Itcl_BiInfoVariableCmd(void *dummy, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 97 {
    int Itcl_BiInfoBodyCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 98 {
    int Itcl_BiInfoArgsCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
#declare 99 {
#    int Itcl_DefaultInfoCmd(void *dummy, Tcl_Interp *interp, int objc,
#        Tcl_Obj *const objv[])
#}


#
#  Ensembles
#

declare 100 {
    int Itcl_EnsembleInit(Tcl_Interp *interp)
}
declare 101 {
    int Itcl_CreateEnsemble(Tcl_Interp *interp, const char *ensName)
}
declare 102 {
    int Itcl_AddEnsemblePart(Tcl_Interp *interp, const char *ensName,
	const char *partName, const char *usageInfo, Tcl_ObjCmdProc *objProc,
	void *clientData, Tcl_CmdDeleteProc *deleteProc)
}
declare 103 {
    int Itcl_GetEnsemblePart(Tcl_Interp *interp, const char *ensName,
	const char *partName, Tcl_CmdInfo *infoPtr)
}
declare 104 {
    int Itcl_IsEnsemble(Tcl_CmdInfo *infoPtr)
}
declare 105 {
    int Itcl_GetEnsembleUsage(Tcl_Interp *interp, const char *ensName,
	Tcl_Obj *objPtr)
}
declare 106 {
    int Itcl_GetEnsembleUsageForObj(Tcl_Interp *interp, Tcl_Obj *ensObjPtr,
	Tcl_Obj *objPtr)
}
declare 107 {
    int Itcl_EnsembleCmd(void *clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 108 {
    int Itcl_EnsPartCmd(void *clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 109 {
    int Itcl_EnsembleErrorCmd(void *clientData, Tcl_Interp *interp,
	Tcl_Size objc, Tcl_Obj *const objv[])
}
declare 115 {
    void Itcl_Assert(const char *testExpr, const char *fileName, int lineNum)
}
declare 116 {
    int Itcl_IsObjectCmd(void *clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *const objv[])
}
declare 117 {
    int Itcl_IsClassCmd(void *clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *const objv[])
}
declare 118 {
    int Itcl_AddEnsemblePart2(Tcl_Interp *interp, const char *ensName,
	const char *partName, const char *usageInfo, Tcl_ObjCmdProc2 *objProc,
	void *clientData, Tcl_CmdDeleteProc *deleteProc)
}

#
#  new commands to use TclOO functionality
#

declare 140 {
    int Itcl_FilterAddCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 141 {
    int Itcl_FilterDeleteCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 142 {
    int Itcl_ForwardAddCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 143 {
    int Itcl_ForwardDeleteCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 144 {
    int Itcl_MixinAddCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 145 {
    int Itcl_MixinDeleteCmd(void *clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}

#
#  Helper commands
#

#declare 150 {
#    int Itcl_BiInfoCmd(void *clientData, Tcl_Interp *interp, int objc,
#        Tcl_Obj *const objv[])
#}
declare 151 {
    int Itcl_BiInfoUnknownCmd(void *dummy, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 152 {
    int Itcl_BiInfoVarsCmd(void *dummy, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 153 {
    int Itcl_CanAccess2(ItclClass *iclsPtr, int protection,
	Tcl_Namespace *fromNsPtr)
}
declare 160 {
    int Itcl_SetCallFrameResolver(Tcl_Interp *interp,
		    Tcl_Resolve *resolvePtr)
}
declare 161 {
    int ItclEnsembleSubCmd(void *clientData, Tcl_Interp *interp,
	    const char *ensembleName, int objc, Tcl_Obj *const *objv,
	    const char *functionName)
}
declare 162 {
    Tcl_Namespace *Itcl_GetUplevelNamespace(Tcl_Interp *interp, int level)
}
declare 163 {
    void *Itcl_GetCallFrameClientData(Tcl_Interp *interp)
}
declare 165 {
    int Itcl_SetCallFrameNamespace(Tcl_Interp *interp, Tcl_Namespace *nsPtr)
}
declare 166 {
    Tcl_Size Itcl_GetCallFrameObjc(Tcl_Interp *interp)
}
declare 167 {
    Tcl_Obj *const *Itcl_GetCallFrameObjv(Tcl_Interp *interp)
}
declare 168 {
    int Itcl_NWidgetCmd(void *infoPtr, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 169 {
    int Itcl_AddOptionCmd(void *infoPtr, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 170 {
    int Itcl_AddComponentCmd(void *infoPtr, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 171 {
    int Itcl_BiInfoOptionCmd(void *dummy, Tcl_Interp *interp, int objc,
	Tcl_Obj *const objv[])
}
declare 172 {
    int Itcl_BiInfoComponentCmd(void *dummy, Tcl_Interp *interp,
	int objc, Tcl_Obj *const objv[])
}
declare 173 {
    int Itcl_RenameCommand(Tcl_Interp *interp, const char *oldName,
	const char *newName)
}
declare 174 {
    int Itcl_PushCallFrame(Tcl_Interp *interp, Tcl_CallFrame *framePtr,
	Tcl_Namespace *nsPtr, int isProcCallFrame)
}
declare 175 {
    void Itcl_PopCallFrame(Tcl_Interp *interp)
}
declare 176 {
    Tcl_CallFrame *Itcl_GetUplevelCallFrame(Tcl_Interp *interp,
				    int level)
}
declare 177 {
    Tcl_CallFrame *Itcl_ActivateCallFrame(Tcl_Interp *interp,
	    Tcl_CallFrame *framePtr)
}
declare 178 {
    const char* ItclSetInstanceVar(Tcl_Interp *interp,
	    const char *name, const char *name2, const char *value,
	    ItclObject *contextIoPtr, ItclClass *contextIclsPtr)
}
declare 179 {
    Tcl_Obj * ItclCapitalize(const char *str)
}
declare 180 {
    int ItclClassBaseCmd(void *clientData, Tcl_Interp *interp,
	    int flags, int objc, Tcl_Obj *const objv[], ItclClass **iclsPtrPtr)
}
declare 181 {
    int ItclCreateComponent(Tcl_Interp *interp, ItclClass *iclsPtr,
	    Tcl_Obj *componentPtr, int type, ItclComponent **icPtrPtr)
}
declare 182 {
    void Itcl_SetContext(Tcl_Interp *interp, ItclObject *ioPtr)
}
declare 183 {
    void Itcl_UnsetContext(Tcl_Interp *interp)
}
declare 184 {
    const char * ItclGetInstanceVar(Tcl_Interp *interp, const char *name,
	    const char *name2, ItclObject *ioPtr, ItclClass *iclsPtr)
}
