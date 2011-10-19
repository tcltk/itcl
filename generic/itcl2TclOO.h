
#ifndef TCL_OO_INTERNAL_H
typedef int (TclOO_PreCallProc)(ClientData clientData, Tcl_Interp *interp,
	Tcl_ObjectContext context, Tcl_CallFrame *framePtr, int *isFinished);
typedef int (TclOO_PostCallProc)(ClientData clientData, Tcl_Interp *interp,
	Tcl_ObjectContext context, Tcl_Namespace *namespacePtr, int result);
#endif

extern int Itcl_NRRunCallbacks(Tcl_Interp *interp, void *rootPtr);
extern void * Itcl_GetCurrentCallbackPtr(Tcl_Interp *interp);
extern Tcl_Method Itcl_NewProcClassMethod(Tcl_Interp *interp, Tcl_Class clsPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ClientData *clientData2);
extern Tcl_Method Itcl_NewProcMethod(Tcl_Interp *interp, Tcl_Object oPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ClientData *clientData2);
extern int Itcl_PublicObjectCmd(ClientData clientData, Tcl_Interp *interp,
        Tcl_Class clsPtr, int objc, Tcl_Obj *const *objv);
extern Tcl_Method Itcl_NewForwardClassMethod(Tcl_Interp *interp,
        Tcl_Class clsPtr, int flags, Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
extern int Itcl_SelfCmd(ClientData clientData, Tcl_Interp *interp,
        int objc, Tcl_Obj *const *objv);
extern int Itcl_InvokeEnsembleMethod(Tcl_Interp *interp, Tcl_Namespace *nsPtr,
    Tcl_Obj *namePtr, Tcl_Proc procPtr, int objc, Tcl_Obj *const *objv);
extern int Itcl_InvokeProcedureMethod(ClientData clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *const *objv);

