/*
 * itcl.h --
 *
 * This file contains definitions for the C-implemeted part of a Itcl
 * this version of [incr Tcl] (Itcl) is a completely new implementation
 * based on TclOO extension of Tcl 8.5
 * It tries to provide the same interfaces as the original implementation
 * of Michael J. McLennan
 * Some small pieces of code are taken from that implementation
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itcl.h,v 1.31.2.2 2007/09/07 21:19:41 wiede Exp $
 */

/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  [incr Tcl] provides object-oriented extensions to Tcl, much as
 *  C++ provides object-oriented extensions to C.  It provides a means
 *  of encapsulating related procedures together with their shared data
 *  in a local namespace that is hidden from the outside world.  It
 *  promotes code re-use through inheritance.  More than anything else,
 *  it encourages better organization of Tcl applications through the
 *  object-oriented paradigm, leading to code that is easier to
 *  understand and maintain.
 *
 *  ADDING [incr Tcl] TO A Tcl-BASED APPLICATION:
 *
 *    To add [incr Tcl] facilities to a Tcl application, modify the
 *    Tcl_AppInit() routine as follows:
 *
 *    1) Include this header file near the top of the file containing
 *       Tcl_AppInit():
 *
 *         #include "itcl.h"
*
 *    2) Within the body of Tcl_AppInit(), add the following lines:
 *
 *         if (Itcl_Init(interp) == TCL_ERROR) {
 *             return TCL_ERROR;
 *         }
 *
 *    3) Link your application with libitcl.a
 *
 *    NOTE:  An example file "tclAppInit.c" containing the changes shown
 *           above is included in this distribution.
 *
 *---------------------------------------------------------------------
 */

#ifndef ITCL_H_INCLUDED
#define ITCL_H_INCLUDED

#include <string.h>
#include <ctype.h>
#include "tcl.h"

#if defined(BUILD_itcl)
#       define ITCLAPI DLLEXPORT
#       undef USE_ITCL_STUBS
#else
#       define ITCLAPI DLLIMPORT
#endif

#ifndef TCL_ALPHA_RELEASE
#   define TCL_ALPHA_RELEASE    0
#endif
#ifndef TCL_BETA_RELEASE
#   define TCL_BETA_RELEASE     1
#endif
#ifndef TCL_FINAL_RELEASE
#   define TCL_FINAL_RELEASE    2
#endif

#define ITCL_MAJOR_VERSION	4
#define ITCL_MINOR_VERSION	0
#define ITCL_RELEASE_LEVEL      TCL_ALPHA_RELEASE
#define ITCL_RELEASE_SERIAL     0

#define ITCL_VERSION            "4.0"
#define ITCL_PATCH_LEVEL        "4.0.0.0"
#define ITCL_NAMESPACE          "::itcl"

#undef TCL_STORAGE_CLASS
#ifdef BUILD_itcl
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_ITCL_STUBS
#       define TCL_STORAGE_CLASS
#   else
#       define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

/*
 * Protection levels:
 *
 * ITCL_PUBLIC    - accessible from any namespace
 * ITCL_PROTECTED - accessible from namespace that imports in "protected" mode
 * ITCL_PRIVATE   - accessible only within the namespace that contains it
 */
#define ITCL_PUBLIC           1
#define ITCL_PROTECTED        2
#define ITCL_PRIVATE          3
#define ITCL_DEFAULT_PROTECT  4

/*
 *  Generic stack.
 */
typedef struct Itcl_Stack {
    ClientData *values;          /* values on stack */
    int len;                     /* number of values on stack */
    int max;                     /* maximum size of stack */
    ClientData space[5];         /* initial space for stack data */
} Itcl_Stack;

#define Itcl_GetStackSize(stackPtr) ((stackPtr)->len)

/*
 *  Generic linked list.
 */
struct Itcl_List;
typedef struct Itcl_ListElem {
    struct Itcl_List* owner;     /* list containing this element */
    ClientData value;            /* value associated with this element */
    struct Itcl_ListElem *prev;  /* previous element in linked list */
    struct Itcl_ListElem *next;  /* next element in linked list */
} Itcl_ListElem;

typedef struct Itcl_List {
    int validate;                /* validation stamp */
    int num;                     /* number of elements */
    struct Itcl_ListElem *head;  /* previous element in linked list */
    struct Itcl_ListElem *tail;  /* next element in linked list */
} Itcl_List;

#define Itcl_FirstListElem(listPtr) ((listPtr)->head)
#define Itcl_LastListElem(listPtr)  ((listPtr)->tail)
#define Itcl_NextListElem(elemPtr)  ((elemPtr)->next)
#define Itcl_PrevListElem(elemPtr)  ((elemPtr)->prev)
#define Itcl_GetListLength(listPtr) ((listPtr)->num)
#define Itcl_GetListValue(elemPtr)  ((elemPtr)->value)

/*
 *  Token representing the state of an interpreter.
 */
typedef struct Itcl_InterpState_ *Itcl_InterpState;


/*
 * Include all the public API, generated from itcl.decls.
 */

#include "itclDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* ITCL_H_INCLUDED */
