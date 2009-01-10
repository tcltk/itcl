#
# Selectiondialog
# ----------------------------------------------------------------------
# Implements a selection box similar to the OSF/Motif standard selection
# dialog composite widget.  The Selectiondialog is derived from the 
# Dialog class and is composed of a SelectionBox with attributes to
# manipulate the dialog buttons.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Selectiondialog
# written by:
#  AUTHOR: Mark L. Ulferts              EMAIL: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: selectiondialog.tcl,v 1.1.2.2 2009/01/10 17:11:54 wiede Exp $
# ======================================================================


#
# Use option database to override default resources of base classes.
#
option add *Selectiondialog.title "Selection Dialog" widgetDefault
option add *Selectiondialog.master "." widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Selectiondialog class.
# 
proc ::itcl::widgets::selectiondialog {pathName args} {
    uplevel ::itcl::widgets::Selectiondialog $pathName $args
}

# ------------------------------------------------------------------
#                           SELECTIONDIALOG
# ------------------------------------------------------------------
::itcl::extendedclass Selectiondialog {
    inherit ::itcl::widgets::Dialog

    component selectionbox

    constructor {args} {}

    public method childsite {}
    public method get {}
    public method curselection {}
    public method clear {component}
    public method insert {component index args}
    public method delete {first {last {}}}
    public method size {}
    public method scan {option args}
    public method nearest {y}
    public method index {index}
    public method selection {option args}
    public method selectitem {}
}
    
# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Selectiondialog::constructor {args} {
puts stderr Selectiondialog::constructor
    #
    # Set the borderwidth to zero.
    #
# FIXME    $win configure -borderwidth 0
    # 
    # Instantiate a selection box widget.
    #
    setupcomponent selectionbox using ::itcl::widgets::Selectionbox \
            $itcl_interior.selectionbox \
	    -dblclickcommand [itcl::code $this invoke]
    keepcomponentoption selectionbox -activebackground -activerelief \
         -background -borderwidth -cursor \
	 -elementborderwidth -foreground -highlightcolor -highlightthickness \
	 -insertbackground -insertborderwidth -insertofftime -insertontime \
	 -insertwidth -jump -labelfont -modality -selectbackground \
	 -selectborderwidth -selectforeground -textbackground -textfont \
	 -troughcolor
    keepcomponentoption selectionbox -childsitepos -exportselection \
            -itemscommand -itemslabel \
	    -itemson -selectionlabel -selectionon -selectioncommand
    configure -itemscommand [itcl::code $this selectitem]
    pack $selectionbox -fill both -expand yes
    set itk_interior [$selectionbox childsite]
    hide Help
    uplevel 0 itcl_initoptions $args
puts stderr Selectiondialog::constructor!END
}   

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::childsite {} {
    return [$selectionbox childsite]
}

# ------------------------------------------------------------------
# METHOD: get
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::get {} {
    return [$selectionbox get]
}

# ------------------------------------------------------------------
# METHOD: curselection
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::curselection {} {
    return [$selectionbox curselection]
}

# ------------------------------------------------------------------
# METHOD: clear component
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::clear {comp} {
    $selectionbox clear $comp
    return
}

# ------------------------------------------------------------------
# METHOD: insert component index args
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::insert {comp index args} {
    uplevel 0 $selectionbox insert $comp $index $args
    return
}

# ------------------------------------------------------------------
# METHOD: delete first ?last?
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::delete {first {last {}}} {
    $selectionbox delete $first $last
    return
}

# ------------------------------------------------------------------
# METHOD: size
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::size {} {
    return [$selectionbox size]
}

# ------------------------------------------------------------------
# METHOD: scan option args
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::scan {option args} {
    return [uplevel 0 $selectionbox scan $option $args]
}

# ------------------------------------------------------------------
# METHOD: nearest y
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::nearest {y} {
    return [$selectionbox nearest $y]
}

# ------------------------------------------------------------------
# METHOD: index index
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::index {index} {
    return [$selectionbox index $index]
}

# ------------------------------------------------------------------
# METHOD: selection option args
#
# Thinwrapped method of selection box class.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::selection {option args} {
    uplevel 0 $selectionbox selection $option $args
}

# ------------------------------------------------------------------
# METHOD: selectitem
#
# Set the default button to ok and select the item.
# ------------------------------------------------------------------
::itcl::body Selectiondialog::selectitem {} {
    default OK
    $selectionbox selectitem
}

} ; # end ::itcl::widgets
