/*
 * C-implemented methods have the following extra information. It is a
 * single-field structure because this allows for future expansion without
 * changing vast amounts of code.
 */

typedef struct CMethod {
    int version;		/* Version of this structure. Currently must
				 * be 0. */
    Tcl_ObjCmdProc *cMethodPtr;	/* Core of the implementation of the method */
    int flags;			/* Flags to control features. */
    ClientData clientData;
    TclOO_PmCDDeleteProc deleteClientdataProc;
    TclOO_PmCDCloneProc cloneClientdataProc;
    ProcErrorProc errProc;	/* Replacement error handler. */
    TclOO_PreCallProc preCallProc;
				/* Callback to allow for additional setup
				 * before the method executes. */
    TclOO_PostCallProc postCallProc;
				/* Callback to allow for additional cleanup
				 * after the method executes. */
    GetFrameInfoValueProc gfivProc;
				/* Callback to allow for fine tuning of how
				 * the method reports itself. */
} CMethod;

#define TCLOO_C_METHOD_VERSION 0

MODULE_SCOPE Tcl_Method TclOONewCMethod(Tcl_Interp *interp, Tcl_Object oPtr,
        int flags, Tcl_Obj *nameObj, Tcl_Obj *argsObj, Tcl_ObjCmdProc *cMethod,
        struct CMethod **cmPtrPtr);
MODULE_SCOPE Tcl_Method TclOONewCClassMethod(Tcl_Interp *interp,
        Tcl_Class clsPtr, int flags, Tcl_Obj *nameObj, Tcl_Obj *argsObj,
	Tcl_ObjCmdProc *cMethod, struct CMethod **cmPtrPtr);

