/*
 * $Id: itclStubLib.c,v 1.9.2.3 2007/09/09 11:04:20 wiede Exp $
 * SOURCE: tk/generic/tkStubLib.c, version 1.9 2004/03/17
 */

#include "tcl.h"

#define USE_ITCL_STUBS 1
#include "itcl.h"
#include "itclInt.h"

#ifdef Itcl_InitStubs
#undef Itcl_InitStubs
#endif

const ItclStubs *itclStubsPtr;
const ItclIntStubs *itclIntStubsPtr;

/*
 *----------------------------------------------------------------------
 *
 * Itcl_InitStubs --
 *	Load the tclOO package, initialize stub table pointer. Do not call
 *	this function directly, use Itcl_InitStubs() macro instead.
 *
 * Results:
 *	The actual version of the package that satisfies the request, or
 *	NULL to indicate that an error occurred.
 *
 * Side effects:
 *	Sets the stub table pointer.
 *
 */

const char *
Itcl_InitStubs(
    Tcl_Interp *interp, const char *version, int exact)
{
    const char *packageName = "Itcl";
    const char *errMsg = NULL;
    ClientData clientData = NULL;
    const char *actualVersion =
	    Tcl_PkgRequireEx(interp, packageName, version, exact, &clientData);
    struct ItclStubAPI *stubsAPIPtr = clientData;
    ItclStubs *stubsPtr = stubsAPIPtr->stubsPtr;
    ItclIntStubs *intStubsPtr = stubsAPIPtr->intStubsPtr;

    if (!actualVersion) {
	return NULL;
    }

    if (!stubsPtr || !intStubsPtr) {
//    if (!stubsPtr) {
	errMsg = "missing stub table pointer";
	goto error;
    }
#ifdef NOTDEF
    if (stubsPtr->epoch != epoch || intStubsPtr->epoch != epoch) {
    if (stubsPtr->epoch != epoch) {
	errMsg = "epoch number mismatch";
	goto error;
    }
    if (stubsPtr->revision < revision || intStubsPtr->revision < revision) {
    if (stubsPtr->revision < revision) {
	errMsg = "require later revision";
	goto error;
    }
#endif

    itclStubsPtr = stubsPtr;
    itclIntStubsPtr = intStubsPtr;
    return actualVersion;

  error:
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, "Error loading ", packageName, " package",
	    " (requested version '", version, "', loaded version '",
	    actualVersion, "'): ", errMsg, NULL);
    return NULL;
}
