#
# itclHullCmds.tcl
# ----------------------------------------------------------------------
# Invoked automatically upon startup to customize the interpreter
# for [incr Tcl] when one of setupcomponent or createhull is called.
# ----------------------------------------------------------------------
#   AUTHOR:  Arnulf P. Wiedemann
#
#      RCS:  $Id: itclHullCmds.tcl,v 1.1.2.1 2008/12/26 16:06:07 wiede Exp $
# ----------------------------------------------------------------------
#            Copyright (c) 2008  Arnulf P. Wiedemann
# ======================================================================
# See the file "license.terms" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

package require Tk 8.6

namespace eval ::itcl::builtin {

proc createhull {widget_type path args} {
    variable hullCount
    upvar this this

    set tmp ${this}
    rename $this ${tmp}_
    set options [list]
    foreach {option_name value} $args {
        switch -- $option_name {
        -class {
            lappend options $option_name $value
          }
        default {
          }
        }
    }
    set my_win [string trimleft $path {:}]
    set widget [uplevel 1 $widget_type $my_win {*}$options]
    set opts [uplevel 1 info delegated options]
puts stderr "OPTS!$my_win!$opts!"
    set my_opt [option get $my_win width *]
puts stderr "my_opt!$my_opt!"
    if {$my_opt ne ""} {
        $my_win configure -width $my_opt
    }
    set idx 1
    while {1} {
        set widgetName ::itcl::internal::widgets::hull${idx}$my_win
	if {[string length [::info command $widgetName]] == 0} {
	    break
	}
        incr idx
    }
puts stderr "USING!$widgetName!"
    rename $tmp $widgetName
    uplevel 1 set itcl_hull $widgetName
    rename ${tmp}_ $tmp
    return $my_win
}

proc setupcomponent {args} {
puts stderr "setupcomponent called $args!"
}

}
