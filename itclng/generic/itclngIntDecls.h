/*
 * $Id: itclngIntDecls.h,v 1.1.2.1 2008/01/12 23:42:29 wiede Exp $
 *
 * This file is (mostly) automatically generated from itcl.decls.
 */


#ifdef NOTDEF
#if defined(USE_ITCLNG_STUBS)

extern const char *Itclng_InitStubs(
	Tcl_Interp *, const char *version, int exact);
#define Itclng_InitStubs(interp,version,exact) Itclng_InitStubs( \
	interp, ITCLNG_VERSION, 1)
#else

#define Itclng_InitStubs(interp,version,exact) Tcl_PkgRequire(interp,"Itclng",version, exact)

#endif
#endif


/* !BEGIN!: Do not edit below this line. */

#define ITCLNGINT_STUBS_EPOCH 0
#define ITCLNGINT_STUBS_REVISION 18

#if !defined(USE_ITCLNG_STUBS)

/*
 * Exported function declarations:
 */

/* 0 */
ITCLNGAPI char*		Itclng_ProtectionStr (int pLevel);

#endif /* !defined(USE_ITCLNG_STUBS) */

typedef struct ItclngIntStubs {
    int magic;
    int epoch;
    int revision;
    struct ItclngIntStubHooks *hooks;

    char* (*itclng_ProtectionStr) (int pLevel); /* 0 */
} ItclngIntStubs;

#ifdef __cplusplus
extern "C" {
#endif
extern const ItclngIntStubs *itclngIntStubsPtr;
#ifdef __cplusplus
}
#endif

#if defined(USE_ITCLNG_STUBS)

/*
 * Inline function declarations:
 */

#ifndef Itclng_ProtectionStr
#define Itclng_ProtectionStr \
	(itclngIntStubsPtr->itclng_ProtectionStr) /* 0 */
#endif

#endif /* defined(USE_ITCLNG_STUBS) */

/* !END!: Do not edit above this line. */

struct ItclngStubAPI {
    ItclngStubs *stubsPtr;
    ItclngIntStubs *intStubsPtr;
};
