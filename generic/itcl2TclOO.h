#define Itcl_NRAddCallback(interp,procPtr,data0,data1,data2,data3) \
  Itcl_NRAddCallback_(interp, #procPtr, procPtr, data0, data1, data2, data3)
EXTERN void Itcl_NRAddCallback_(Tcl_Interp *interp, char *procName,
        void *procPtr, ClientData data0, ClientData data1,
        ClientData data2, ClientData data3);
EXTERN void Itcl_DumpNRCallbacks(Tcl_Interp *interp, char *str);
EXTERN int Itcl_NRCallObjProc(ClientData clientData, Tcl_Interp *interp,
        Tcl_ObjCmdProc *objProc, int objc, Tcl_Obj *const *objv);
EXTERN int Itcl_NRRunCallbacks(Tcl_Interp *interp, void *rootPtr);
EXTERN void * Itcl_GetCurrentCallbackPtr(Tcl_Interp *interp);
EXTERN Tcl_Method Itcl_NewProcClassMethod(Tcl_Interp *interp, Tcl_Class clsPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ClientData *clientData2);
EXTERN Tcl_Method Itcl_NewProcMethod(Tcl_Interp *interp, Tcl_Object oPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ClientData *clientData2);
EXTERN int Itcl_PublicObjectCmd(ClientData clientData, Tcl_Interp *interp,
        Tcl_Class clsPtr, int objc, Tcl_Obj *const *objv);
EXTERN Tcl_Method Itcl_NewForwardClassMethod(Tcl_Interp *interp,
        Tcl_Class clsPtr, int flags, Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
EXTERN Tcl_Method Itcl_NewForwardMethod(Tcl_Interp *interp, Tcl_Object oPtr,
        int flags, Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
EXTERN int Itcl_SelfCmd(ClientData clientData, Tcl_Interp *interp,
        int objc, Tcl_Obj *const *objv);
EXTERN Tcl_Obj * Itcl_TclOOObjectName(Tcl_Interp *interp, Tcl_Object oPtr);

