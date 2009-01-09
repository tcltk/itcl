#
# Dialog
# ----------------------------------------------------------------------
# Implements a standard dialog box providing standard buttons and a 
# child site for use in derived classes.  The buttons include ok, apply,
# cancel, and help.  Options exist to configure the buttons.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Dialog
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: dialog.tcl,v 1.1.2.1 2009/01/09 21:39:21 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Dialog.master "." widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Dialog class.
# 
proc ::::itcl::widgets::dialog {pathName args} {
    uplevel ::::itcl::widgets::Dialog $pathName $args
}

# ------------------------------------------------------------------
#                            DIALOG
# ------------------------------------------------------------------
::itcl::extendedclass Dialog {
    inherit ::itcl::widgets::Dialogshell

    constructor {args} {}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Dialog::constructor {args} {
    #
    # Add the standard buttons: OK, Apply, Cancel, and Help, making
    # OK be the default button.
    #
    add OK -text OK -command [itcl::code $this deactivate 1]
    add Apply -text Apply
    add Cancel -text Cancel -command [itcl::code $this deactivate 0]
    add Help -text Help
    default OK
    #
    # Bind the window manager delete protocol to invocation of the
    # cancel button.  This can be overridden by the user via the
    # execution of a similar command outside the class.
    #
    wm protocol $itk_component(hull) WM_DELETE_WINDOW \
	[itcl::code $this invoke Cancel]
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
}

} ; # end ::itcl::widgets
