/*
 * itclng.h --
 *
 * This file contains definitions for the C-implemeted part of a Itcl
 * this version of [incr Tcl] (Itcl) is a completely new implementation
 * based on TclOO extension of Tcl 8.5
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclng.h,v 1.1.2.2 2008/01/12 23:43:46 wiede Exp $
 */

/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl next generation]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *---------------------------------------------------------------------
 */

#ifndef ITCLNG_H_INCLUDED
#define ITCLNG_H_INCLUDED

#include <string.h>
#include <ctype.h>
#include "tcl.h"

#if defined(BUILD_itclng)
#       define ITCLNGAPI DLLEXPORT
#       undef USE_ITCLNG_STUBS
#else
#       define ITCLNGAPI DLLIMPORT
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

#define ITCLNG_MAJOR_VERSION	4
#define ITCLNG_MINOR_VERSION	0
#define ITCLNG_RELEASE_LEVEL    TCL_ALPHA_RELEASE
#define ITCLNG_RELEASE_SERIAL   0

#define ITCLNG_VERSION            "0.1"
#define ITCLNG_PATCH_LEVEL        "0.1.0.0"
#define ITCLNG_NAMESPACE          "::itclng"

#undef TCL_STORAGE_CLASS
#ifdef BUILD_itclng
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_ITCLNG_STUBS
#       define TCL_STORAGE_CLASS
#   else
#       define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

/*
 * Protection levels:
 *
 * ITCLNG_PUBLIC    - accessible from any namespace
 * ITCLNG_PROTECTED - accessible from namespace that imports in "protected" mode
 * ITCLNG_PRIVATE   - accessible only within the namespace that contains it
 */
#define ITCLNG_PUBLIC           1
#define ITCLNG_PROTECTED        2
#define ITCLNG_PRIVATE          3

/*
 *  Generic stack.
 */
typedef struct Itclng_Stack {
    ClientData *values;          /* values on stack */
    int len;                     /* number of values on stack */
    int max;                     /* maximum size of stack */
    ClientData space[5];         /* initial space for stack data */
} Itclng_Stack;

#define Itclng_GetStackSize(stackPtr) ((stackPtr)->len)

/*
 *  Generic linked list.
 */
struct Itclng_List;
typedef struct Itclng_ListElem {
    struct Itclng_List* owner;     /* list containing this element */
    ClientData value;            /* value associated with this element */
    struct Itclng_ListElem *prev;  /* previous element in linked list */
    struct Itclng_ListElem *next;  /* next element in linked list */
} Itclng_ListElem;

typedef struct Itclng_List {
    int validate;                /* validation stamp */
    int num;                     /* number of elements */
    struct Itclng_ListElem *head;  /* previous element in linked list */
    struct Itclng_ListElem *tail;  /* next element in linked list */
} Itclng_List;

#define Itclng_FirstListElem(listPtr) ((listPtr)->head)
#define Itclng_LastListElem(listPtr)  ((listPtr)->tail)
#define Itclng_NextListElem(elemPtr)  ((elemPtr)->next)
#define Itclng_PrevListElem(elemPtr)  ((elemPtr)->prev)
#define Itclng_GetListLength(listPtr) ((listPtr)->num)
#define Itclng_GetListValue(elemPtr)  ((elemPtr)->value)

/*
 * Include all the public API, generated from itcl.decls.
 */

#include "itclngDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* ITCLNG_H_INCLUDED */
