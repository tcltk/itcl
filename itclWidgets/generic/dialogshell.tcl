#
# Dialogshell
# ----------------------------------------------------------------------
# This class is implements a dialog shell which is a top level widget
# composed of a button box, separator, and child site area.  The class
# also has methods to control button construction.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Dialogshell
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: dialogshell.tcl,v 1.1.2.1 2009/01/09 21:39:21 wiede Exp $
# ======================================================================

    keep -background -cursor -foreground -modality 

#
# Use option database to override default resources of base classes.
#
option add *Dialogshell.master "." widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Dialogshell class.
# 
proc ::itcl::widgets::dialogshell {pathName args} {
    uplevel ::itcl::widgets::Dialogshell $pathName $args
}

# ------------------------------------------------------------------
#                            DIALOGSHELL
# ------------------------------------------------------------------
::itcl::extendedclass Dialogshell {
    inherit ::itcl::widgets::Shell

    protected component dschildsite
    component separator
    component bbox

    option [list -thickness thickness Thickness] -default 3 -configuremethod configThickness
    option [list -buttonboxpos buttonBoxPos Position] -default s -configuremethod configButtonboxpos
    option [list -separator separator Separator] -default on -configuremethod configSeparator
    option [list -padx padX Pad] -default 10 -configuremethod configPadx
    option [list -pady padY Pad] -default 10 -configuremethod configPady

    delegate option [list -buttonboxpadx buttonBoxPadX Pad] to bbox as -padx
    delegate option [list -buttonboxpady buttonBoxPadY Pad] to bbox as -pady

    constructor {args} {}

    protected method configThickness {option value}
    protected method configButtonboxpos {option value}
    protected method configSeparator {option value}
    protected method configPadx {option value}
    protected method configPady {option value}

    public method childsite {}
    public method index {args}
    public method add {args}
    public method insert {args}
    public method delete {args}
    public method hide {args}
    public method show {args}
    public method default {args}
    public method invoke {args}
    public method buttonconfigure {args}
    public method buttoncget {index option}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Dialogshell::constructor {args} {
# FIXME    itcl_options remove ::itcl::widgets::Shell::padx ::itcl::widgets::Shell::pady
    #
    # Create the user child site, separator, and button box,
    #
    setupcomponent dschildsite using frame $itcl_interior.dschildsite
    setupcomponent separator using frame $itcl_interior.separator -relief sunken
    setupcomponent bbox using ::itcl::widgets::Buttonbox $itcl_interior.bbox
    keepcomponentoption bbox -background -cursor -foreground -modality 
    #
    # Set the itk_interior variable to be the childsite for derived 
    # classes.
    #
    set itk_interior $dschildsite
    #
    # Set up the default button so that if <Return> is pressed in
    # any widget, it will invoke the default button.
    #
    bind $win <Return> [itcl::code $this invoke]
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -thickness
#
# Specifies the thickness of the separator.  It sets the width and
# height of the separator to the thickness value and the borderwidth
# to half the thickness.
# ------------------------------------------------------------------
::itcl::body Dialogshell::configThickness {option value} {
    $separator config -height $value
    $separator config -width $value
    $separator config -borderwidth [expr {$value / 2}]
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -buttonboxpos
#
# Specifies the position of the button box relative to the child site.
# The separator appears between the child site and button box.
# ------------------------------------------------------------------
::itcl::body Dialogshell::configButtonboxpos {option value} {
    set parent [winfo parent $bbox]
    switch $value {
    n {
        $bbox configure -orient horizontal
        grid $bbox -row 0 -column 0 -sticky ew
        grid $separator -row 1 -column 0 -sticky ew
        grid $dschildsite -row 2 -column 0 -sticky nsew
        grid rowconfigure $parent 0 -weight 0
        grid rowconfigure $parent 1 -weight 0
        grid rowconfigure $parent 2 -weight 1
        grid columnconfigure $parent 0 -weight 1
        grid columnconfigure $parent 1 -weight 0
        grid columnconfigure $parent 2 -weight 0
      }
    s {
        $bbox configure -orient horizontal
        grid $dschildsite -row 0 -column 0 -sticky nsew
        grid $separator -row 1 -column 0 -sticky ew
        grid $bbox -row 2 -column 0 -sticky ew
        grid rowconfigure $parent 0 -weight 1
        grid rowconfigure $parent 1 -weight 0
        grid rowconfigure $parent 2 -weight 0
        grid columnconfigure $parent 0 -weight 1
        grid columnconfigure $parent 1 -weight 0
        grid columnconfigure $parent 2 -weight 0
      }
    w {
        $bbox configure -orient vertical
        grid $bbox -row 0 -column 0 -sticky ns
        grid $separator -row 0 -column 1 -sticky ns
        grid $dschildsite -row 0 -column 2 -sticky nsew
        grid rowconfigure $parent 0 -weight 1
        grid rowconfigure $parent 1 -weight 0
        grid rowconfigure $parent 2 -weight 0
        grid columnconfigure $parent 0 -weight 0
        grid columnconfigure $parent 1 -weight 0
        grid columnconfigure $parent 2 -weight 1
      }
    e {
        $bbox configure -orient vertical
        grid $dschildsite -row 0 -column 0 -sticky nsew
        grid $separator -row 0 -column 1 -sticky ns
        grid $bbox -row 0 -column 2 -sticky ns
        grid rowconfigure $parent 0 -weight 1
        grid rowconfigure $parent 1 -weight 0
        grid rowconfigure $parent 2 -weight 0
        grid columnconfigure $parent 0 -weight 1
        grid columnconfigure $parent 1 -weight 0
        grid columnconfigure $parent 2 -weight 0
      }
    default {
        error "bad buttonboxpos option\
	        \"$value\": should be n,\
	        s, e, or w"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -separator 
#
# Boolean option indicating wheather to display the separator.
# ------------------------------------------------------------------
::itcl::body Dialogshell::configSeparator {option value} {
    if {$value} {
	$separator configure -relief sunken
    } else {
	$separator configure -relief flat
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -padx
#
# Specifies a padding distance for the childsite in the X-direction.
# ------------------------------------------------------------------
::itcl::body Dialogshell::configPadx {option value} {
    grid configure $dschildsite -padx $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -pady
#
# Specifies a padding distance for the childsite in the Y-direction.
# ------------------------------------------------------------------
::itcl::body Dialogshell::configPady {option value} {
    grid configure $dschildsite -pady $value
    set itcl_options($option) $value
}
    
# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Return the pathname of the user accessible area.
# ------------------------------------------------------------------
::itcl::body Dialogshell::childsite {} {
    return $dschildsite
}

# ------------------------------------------------------------------
# METHOD: index index
#
# Thin wrapper of Buttonbox's index method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::index {args} {
    uplevel $bbox index $args
}

# ------------------------------------------------------------------
# METHOD: add tag ?option value ...?
#
# Thin wrapper of Buttonbox's add method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::add {args} {
    uplevel $bbox add $args
}

# ------------------------------------------------------------------
# METHOD: insert index tag ?option value ...?
#
# Thin wrapper of Buttonbox's insert method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::insert {args} {
    uplevel $bbox insert $args
}

# ------------------------------------------------------------------
# METHOD: delete tag
#
# Thin wrapper of Buttonbox's delete method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::delete {args} {
    uplevel $bbox delete $args
}

# ------------------------------------------------------------------
# METHOD: hide index
#
# Thin wrapper of Buttonbox's hide method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::hide {args} {
    uplevel $bbox hide $args
}

# ------------------------------------------------------------------
# METHOD: show index
#
# Thin wrapper of Buttonbox's show method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::show {args} {
    uplevel $bbox show $args
}

# ------------------------------------------------------------------
# METHOD: default index
#
# Thin wrapper of Buttonbox's default method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::default {args} {
    uplevel $bbox default $args
}

# ------------------------------------------------------------------
# METHOD: invoke ?index?
#
# Thin wrapper of Buttonbox's invoke method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::invoke {args} {
    uplevel $bbox invoke $args
}

# ------------------------------------------------------------------
# METHOD: buttonconfigure index ?option? ?value option value ...?
#
# Thin wrapper of Buttonbox's buttonconfigure method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::buttonconfigure {args} {
    uplevel $bbox buttonconfigure $args
}

# ------------------------------------------------------------------
# METHOD: buttoncget index option
#
# Thin wrapper of Buttonbox's buttoncget method.
# ------------------------------------------------------------------
::itcl::body Dialogshell::buttoncget {index option} {
  uplevel $ibbox buttoncget [list $index] [list $option]
}

} ; # end ::itcl::widgets
