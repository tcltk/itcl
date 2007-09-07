#ifndef _TCLOOINT_H
typedef int (*TclOO_PreCallProc)(ClientData clientData, Tcl_Interp *interp,
        Tcl_ObjectContext contextPtr, Tcl_CallFrame *framePtr, int *isFinished);
typedef int (*TclOO_PostCallProc)(ClientData clientData, Tcl_Interp *interp,
        Tcl_ObjectContext contextPtr, Tcl_Namespace *nsPtr, int result);
typedef int (*TclOO_MapCmdName)(ClientData clientData, Tcl_Interp *interp,
        Tcl_Obj *mappedCmd, Tcl_Class *clsPtr);
#endif

EXTERN void Tcl_ObjectSetMapCmdNameProc(Tcl_Object oPtr,
        TclOO_MapCmdName mapCmdNameProc);


#define Tcl_ProcPtrFromPM _Tcl_ProcPtrFromPM
#define Tcl_NewProcMethod _Tcl_NewProcMethod
#define Tcl_NewProcClassMethod _Tcl_NewProcClassMethod

EXTERN Tcl_Method _Tcl_NewProcMethod(Tcl_Interp *interp,
        Tcl_Object oPtr, Tcl_Obj *nameObj, Tcl_Obj *argsObj, Tcl_Obj *bodyObj,
        int flags, ClientData *clientData);
EXTERN Tcl_Method _Tcl_NewProcClassMethod(Tcl_Interp *interp, Tcl_Class clsPtr,
        TclOO_PreCallProc preCallPtr, TclOO_PostCallProc postCallPtr,
        ClientData clientData, Tcl_Obj *nameObj, Tcl_Obj *argsObj,
        Tcl_Obj *bodyObj,  int flags, ClientData *clientData2);

EXTERN ClientData _Tcl_ProcPtrFromPM(ClientData clientData);


