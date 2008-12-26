#
# itclHullCmds.tcl
# ----------------------------------------------------------------------
# Invoked automatically upon startup to customize the interpreter
# for [incr Tcl] when one of setupcomponent or createhull is called.
# ----------------------------------------------------------------------
#   AUTHOR:  Arnulf P. Wiedemann
#
#      RCS:  $Id: itclHullCmds.tcl,v 1.1.2.2 2008/12/26 18:10:31 wiede Exp $
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
        switch -glob -- $option_name {
        -* {
            lappend options $option_name $value
          }
        default {
	    return -code error "bad option name\"$option_name\" options must start with a \"-\""
          }
        }
    }
    set my_win [string trimleft $path {:}]
    set cmd [list $widget_type $my_win]
    if {[llength $options] > 0} {
        lappend cmd {*}$options
    }
    set widget [uplevel 1 $cmd]
    set opts [uplevel 1 info delegated options]
    foreach entry $opts {
        foreach {optName compName} $entry break
	if {$compName eq "itcl_hull"} {
	    set optInfos [uplevel 1 info delegated option $optName]
	    set realOptName [lindex $optInfos 4]
	    # strip off the "-" at the beginning
	    set myOptName [string range $realOptName 1 end]
            set my_opt_val [option get $my_win $myOptName *]
            if {$my_opt_val ne ""} {
                $my_win configure -$myOptName $my_opt_val
            }
	}
    }
    set idx 1
    while {1} {
        set widgetName ::itcl::internal::widgets::hull${idx}$my_win
	if {[string length [::info command $widgetName]] == 0} {
	    break
	}
        incr idx
    }
    rename $tmp $widgetName
    uplevel 1 set itcl_hull $widgetName
    rename ${tmp}_ $tmp
    return $my_win
}

proc setupcomponent {comp using widget_type path args} {
    set options [list]
    foreach {option_name value} $args {
        switch -glob -- $option_name {
        -* {
            lappend options $option_name $value
          }
        default {
	    return -code error "bad option name\"$option_name\" options must start with a \"-\""
          }
        }
    }
    set cmd [list $widget_type $path]
    if {[llength $options] > 0} {
        lappend cmd {*}$options
    }
    set my_comp [uplevel 1 $cmd]
    uplevel 1 set $comp $my_comp
    set opts [uplevel 1 info delegated options]
    foreach entry $opts {
        foreach {optName compName} $entry break
	if {$compName eq $my_comp} {
	    set optInfos [uplevel 1 info delegated option $optName]
	    set realOptName [lindex $optInfos 4]
	    # strip off the "-" at the beginning
	    set myOptName [string range $realOptName 1 end]
            set my_opt_val [option get $my_win $myOptName *]
            if {$my_opt_val ne ""} {
                $my_comp configure -$myOptName $my_opt_val
            }
	}
    }
}

}
