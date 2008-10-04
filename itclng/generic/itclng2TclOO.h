#define Itclng_NRAddCallback(interp,procPtr,data0,data1,data2,data3) \
  Itclng_NRAddCallback_(interp, #procPtr, procPtr, data0, data1, data2, data3)
EXTERN void Itclng_NRAddCallback_(Tcl_Interp *interp, char *procName,
        void *procPtr, ClientData data0, ClientData data1,
        ClientData data2, ClientData data3);
EXTERN void Itclng_DumpNRCallbacks(Tcl_Interp *interp, char *str);
EXTERN int Itclng_NRCallObjProc(ClientData clientData, Tcl_Interp *interp,
        Tcl_ObjCmdProc *objProc, int objc, Tcl_Obj *const *objv);
EXTERN int Itclng_NRRunCallbacks(Tcl_Interp *interp, void *rootPtr);
EXTERN void * Itclng_GetCurrentCallbackPtr(Tcl_Interp *interp);
EXTERN Tcl_Method Itclng_NewProcClassMethod(Tcl_Interp *interp,
        Tcl_Class clsPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ClientData *clientData2);
EXTERN Tcl_Method Itclng_NewProcMethod(Tcl_Interp *interp, Tcl_Object oPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ClientData *clientData2);
EXTERN Tcl_Method Itclng_NewCClassMethod(Tcl_Interp *interp,
        Tcl_Class clsPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_ObjCmdProc *cMethod, ClientData *clientData2);
EXTERN Tcl_Method Itclng_NewCMethod(Tcl_Interp *interp, Tcl_Object oPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        Tcl_ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj,
	Tcl_Obj *argsObj, Tcl_ObjCmdProc *cMethod, ClientData *clientData2);
EXTERN int Itclng_PublicObjectCmd(ClientData clientData, Tcl_Interp *interp,
        Tcl_Class clsPtr, int objc, Tcl_Obj *const *objv);
EXTERN Tcl_Method Itclng_NewForwardClassMethod(Tcl_Interp *interp,
        Tcl_Class clsPtr, int flags, Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
EXTERN Tcl_Method Itclng_NewForwardMethod(Tcl_Interp *interp, Tcl_Object oPtr,
        int flags, Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
