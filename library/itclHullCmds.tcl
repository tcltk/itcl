#
# itclHullCmds.tcl
# ----------------------------------------------------------------------
# Invoked automatically upon startup to customize the interpreter
# for [incr Tcl] when one of setupcomponent or createhull is called.
# ----------------------------------------------------------------------
#   AUTHOR:  Arnulf P. Wiedemann
#
# ----------------------------------------------------------------------
#            Copyright (c) 2008  Arnulf P. Wiedemann
# ======================================================================
# See the file "license.terms" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

package require Tk 8.6

namespace eval ::itcl::internal::commands {

# ======================= widgetDeleted ===========================

proc widgetDeleted {oldName newName op} {
    # The widget is beeing deleted, so we have to delete the object
    # which had the widget as itcl_hull too!
    # We have to get the real name from for example
    # ::itcl::internal::widgets::hull1.lw
    # we need only .lw here

#puts stderr "widgetDeleted!$oldName!$newName!$op!"
    set cmdName [namespace tail $oldName]
    set flds [split $cmdName {.}]
    set cmdName .[join [lrange $flds 1 end] {.}]
#puts stderr "DELWIDGET![namespace current]!$cmdName![::info command $cmdName]!"
    rename $cmdName {}
}

}

namespace eval ::itcl::builtin {

# ======================= createhull ===========================
# the hull widget is a tk widget which is the (mega) widget handled behind the itcl
# extendedclass/itcl widget.
# It is created be renaming the itcl class object to a temporary name <itcl objecct name>_
# creating the widget with the
# appropriate options and the installing that as the "hull" widget (the container)
# All the options in args and the options delegated to component itcl_hull are used
# Then a unique name (hull_widget_name) in the itcl namespace is created for widget:
# ::itcl::internal::widgets::hull<unique number><namespace tail path>
# and widget is renamed to that name
# Finally the <itcl objecct name>_ is renamed to the original <itcl object name> again
# Component itcl_hull is created if not existent
# itcl_hull is set to the hull_widget_name and the <itcl object name>
# is returned to the caller
# ==============================================================

proc createhull {widget_type path args} {
    variable hullCount
    upvar this this
    upvar win win


#puts stderr "il-1![::info level -1]!$this!"
#puts stderr "createhull!$widget_type!$path!$args!$this![::info command $this]!"
#puts stderr "ns1![uplevel 1 namespace current]!"
#puts stderr "ns2![uplevel 2 namespace current]!"
#puts stderr "ns3![uplevel 3 namespace current]!"
#puts stderr "level-1![::info level -1]!"
#puts stderr "level-2![::info level -2]!"
#    set my_this [namespace tail $this]
    set my_this $this
    set tmp $my_this
#puts stderr "II![::info command $this]![::info command $tmp]!"
#puts stderr "rename1!rename $my_this ${tmp}_!"
    rename ::$my_this ${tmp}_
    set options [list]
    foreach {option_name value} $args {
        switch -glob -- $option_name {
	-class {
	      lappend options $option_name [namespace tail $value]
	  }
        -* {
            lappend options $option_name $value
          }
        default {
	    return -code error "bad option name\"$option_name\" options must start with a \"-\""
          }
        }
    }
    set my_win [namespace tail $path]
    set cmd [list $widget_type $my_win]
#puts stderr "my_win!$my_win!cmd!$cmd!$path!"
    if {[llength $options] > 0} {
        lappend cmd {*}$options
    }
    set widget [uplevel 1 $cmd]
#puts stderr "widget!$widget!"
    trace add command $widget delete ::itcl::internal::commands::widgetDeleted
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
#puts stderr "widgetName!$widgetName!"
	if {[string length [::info command $widgetName]] == 0} {
	    break
	}
        incr idx
    }
#puts stderr "rename2!rename $widget $widgetName!"
    set dorename 0
    rename $widget $widgetName
#puts stderr "rename3!rename ${tmp}_ $tmp![::info command ${tmp}_]!my_this!$my_this!"
    rename ${tmp}_ ::$tmp
    set exists [uplevel 1 ::info exists itcl_hull]
    if {!$exists} {
	# that does not yet work, beacause of problems with resolving 
        ::itcl::addcomponent $my_this itcl_hull
    }
    upvar itcl_hull itcl_hull
    ::itcl::setcomponent $my_this itcl_hull $widgetName
#puts stderr "IC![::info command $my_win]!"
    set exists [uplevel 1 ::info exists itcl_interior]
    if {!$exists} {
	# that does not yet work, beacause of problems with resolving 
        ::itcl::addcomponent $this itcl_interior
    }
    upvar itcl_interior itcl_interior
    set itcl_interior $my_win
#puts stderr "hull end!win!$win!itcl_hull!$itcl_hull!itcl_interior!$itcl_interior!"
    return $my_win
}

# ======================= setupcomponent ===========================

proc setupcomponent {comp using widget_type path args} {
    upvar this this

#puts stderr "setupcomponent!$comp!$widget_type!$path!$args!$this!"
#puts stderr "ns1![uplevel 1 namespace current]!"
#puts stderr "ns2![uplevel 2 namespace current]!"
#puts stderr "ns3![uplevel 3 namespace current]!"
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
#puts stderr "cmd0![::info command $widget_type]!$path![::info command $path]!"
#puts stderr "cmd1!$cmd!"
#    set my_comp [uplevel 3 $cmd]
    set my_comp [uplevel #0 $cmd]
#puts stderr 111![::info command $path]!
    ::itcl::setcomponent $this $comp $my_comp
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
#puts stderr END!$path![::info command $path]!
}

proc itcl_initoptions {args} {
puts stderr "ITCL_INITOPT!$args!"
}

# ======================= initoptions ===========================

proc initoptions {args} {
    upvar win win
    upvar itcl_hull itcl_hull
    upvar itcl_option_components itcl_option_components

    if {[llength $args]} {
        set argsDict [dict create {*}$args]
    } else {
        set argsDict [dict create]
    }
    set my_class [uplevel 1 namespace current]
#    set myOptions [namespace eval $my_class {info classoptions}]
    set myOptions [lsort -unique [namespace eval $my_class {info options}]]
#puts stderr "OPTS!$win!$my_class![join [lsort $myOptions]] \n]!"
    set myDelegatedOptions [uplevel 1 info delegated options]
    set opt_lst [list configure]
    set my_win $win
    set class_info_dict [dict get $::itcl::internal::dicts::classComponents $my_class]
    foreach opt [lsort $myOptions] {
	set found 0
        if {[catch {
            set resource [uplevel 1 info option $opt -resource]
            set class [uplevel 1 info option $opt -class]
            set default_val [uplevel 1 info option $opt -default]
	    set found 1
        } msg]} {
#            puts stderr "MSG!$opt!$msg!"
        }
	if {$found} {
           set val [uplevel #0 ::option get $my_win $resource $class]
           if {[::dict exists $argsDict $opt]} {
               # we have an explicitly set option
               set val [::dict get $argsDict $opt]
           } else {
	       if {[string length $val] == 0} {
                   set val $default_val
	       }
           }
           set ::itcl::internal::variables::${win}::itcl_options($opt) $val
#	   uplevel 1 [list set itcl_options($opt) [list $val]]
           if {[catch {uplevel 1 $my_win configure $opt [list $val]} msg]} {
puts stderr "initoptions ERR!$msg!$my_class!$my_win!configure!$opt!$val!"
	   }
        }
        foreach comp [dict keys $class_info_dict] {
#puts stderr "OPT1!$opt!$comp![dict get $class_info_dict $comp]!"
            if {[dict exists $class_info_dict $comp -keptoptions]} {
                if {[lsearch [dict get $class_info_dict $comp -keptoptions] $opt] >= 0} {
                    if {$found == 0} {
                        # we use the option value of the first component for setting
                        # the option, as the components are traversed in the dict
                        # depending on the ordering of the component creation!!
                        set my_info [uplevel 1 \[set $comp\] configure $opt]
                        set resource [lindex $my_info 1]
                        set class [lindex $my_info 2]
                        set default_val [lindex $my_info 3]
                        set found 2
                        set val [uplevel #0 ::option get $my_win $resource $class]
                        if {[::dict exists $argsDict $opt]} {
                            # we have an explicitly set option
                            set val [::dict get $argsDict $opt]
                        } else {
	                    if {[string length $val] == 0} {
                                set val $default_val
	                    }
                        }
		        set ::itcl::internal::variables::${win}::itcl_options($opt) $val
#	                uplevel 1 [list set itcl_options($opt) [list $val]]
                    }
                    if {[catch {uplevel 1 \[set $comp\] configure $opt [list $val]} msg]} {
puts stderr "initoptions ERR2!$msg!$my_class!$comp!configure!$opt!$val!"
	            }
		    if {![uplevel 1 info exists itcl_option_components($opt)]} {
                        set itcl_option_components($opt) [list]
		    }
		    if {[lsearch [set itcl_option_components($opt)] $comp] < 0} {
		        if {![catch {
		            set optval [uplevel 1 [list set itcl_options($opt)]]
                        } msg3]} {
                                uplevel 1 \[set $comp\] configure $opt $optval
                        }
                        lappend itcl_option_components($opt) $comp
		    }
                }
            }
        }
    }
#    uplevel 1 $opt_lst
}

# ========================= keepcomponentoption ======================
#  Invoked by Tcl during evaluating constructor whenever
#  the "keepcomponentoption" command is invoked to list the options
#  to be kept when an ::itcl::extendedclass component has been setup
#  for an object.
#
#  It checks, for all arguments, if the opt is an option of that class
#  and of that component. If that is the case it adds the component name
#  to the list of components for that option.
#  The variable is the object variable: itcl_option_components($opt)
#
#  Handles the following syntax:
#
#    keepcomponentoption <componentName> <optionName> ?<optionName> ...?
#
# ======================================================================


proc keepcomponentoption {args} {
    set usage "wrong # args, should be: keepcomponentoption componentName optionName ?optionName ...?"

    if {[llength $args] < 2} {
        puts stderr $usage
	return -code error
    }
    set my_hull [uplevel 1 set itcl_hull]
    set my_class [uplevel 1 namespace current]
    set comp [lindex $args 0]
    set args [lrange $args 1 end]
    set class_info_dict [dict get $::itcl::internal::dicts::classComponents $my_class]
    if {![dict exists $class_info_dict $comp]} {
        puts stderr "keepcomponentoption cannot find component \"$comp\""
	return -code error
    }
    set class_comp_dict [dict get $class_info_dict $comp]
    if {![dict exists $class_comp_dict -keptoptions]} {
        dict set class_comp_dict -keptoptions [list]
    }
    foreach opt $args {
	if {[string range $opt 0 0] ne "-"} {
            puts stderr "keepcomponentoption: option must begin with a \"-\"!"
	    return -code error
	}
        if {[lsearch [dict get $class_comp_dict -keptoptions] $opt] < 0} {
            dict lappend class_comp_dict -keptoptions $opt
	}
    }
    dict set class_info_dict $comp $class_comp_dict
    dict set ::itcl::internal::dicts::classComponents $my_class $class_info_dict
}

proc ignorecomponentoption {args} {
puts stderr "IGNORE_COMPONENT_OPTION!$args!"
}

proc renamecomponentoption {args} {
puts stderr "rename_COMPONENT_OPTION!$args!"
}

proc addoptioncomponent {args} {
puts stderr "ADD_OPTION_COMPONENT!$args!"
}

proc ignoreoptioncomponent {args} {
puts stderr "IGNORE_OPTION_COMPONENT!$args!"
}

proc renameoptioncomponent {args} {
puts stderr "RENAME_OPTION_COMPONENT!$args!"
}

}
