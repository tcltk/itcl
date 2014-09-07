

#define ITCL_DEBUG_C_INTERFACE 1
#ifdef ITCL_DEBUG_C_INTERFACE

#include <stdio.h>
#include "itclInt.h"

Tcl_CmdProc cArgFunc;
Tcl_ObjCmdProc cObjFunc;

int
cArgFunc(
    ClientData clientData,
    Tcl_Interp *interp,
    int argc,
    const char **argv)
{
    int i;

fprintf(stderr, "cArgFunc called:\n");
for(i = 0; i<argc;i++) {
    fprintf(stderr, "arg:%d:%s:\n", i, argv[i]);
}
    return TCL_OK;
}

int
cObjFunc(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Tcl_Namespace *nsPtr;
    int i;

    ItclShowArgs(0, "cObjFunc called", objc, objv);
fprintf(stderr, "XX:%d %p\n", objc, objv);
for(i = 0; i<objc;i++) {
    fprintf(stderr, "arg:%d:%s:\n", i, Tcl_GetString(objv[i]));
}
    nsPtr = Tcl_GetCurrentNamespace(interp);
fprintf(stderr, "IP:%p:\n",interp);
    return TCL_OK;
}


void
RegisterDebugCFunctions(Tcl_Interp *interp)
{
    int result;

    result = Itcl_RegisterC(interp, "cArgFunc", cArgFunc, NULL, NULL);
    result = Itcl_RegisterObjC(interp, "cObjFunc", cObjFunc, NULL, NULL);
}
#endif
