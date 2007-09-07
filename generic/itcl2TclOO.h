EXTERN Tcl_Method Itcl_NewProcClassMethod(Tcl_Interp *interp, Tcl_Class clsPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        ClientData clientData, Tcl_Obj *nameObj, Tcl_Obj *argsObj,
        Tcl_Obj *bodyObj, ClientData *clientData2);
EXTERN int Itcl_PublicObjectCmd(ClientData clientData, Tcl_Interp *interp,
        Tcl_Class clsPtr, int objc, Tcl_Obj *const *objv);
EXTERN void ItclSetErrProc(ClientData *clientDataPtr, ItclMemberFunc *mPtr);

