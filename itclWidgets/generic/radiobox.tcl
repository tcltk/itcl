#
# Radiobox
# ----------------------------------------------------------------------
# Implements a radiobuttonbox.  Supports adding, inserting, deleting,
# selecting, and deselecting of radiobuttons by tag and index.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the ::itcl::widgets package Radiobox
# written by:
#  AUTHOR: Michael J. McLennan          EMAIL: mmclennan@lucent.com
#          Mark L. Ulferts              EMAIL: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: radiobox.tcl,v 1.1.2.1 2009/01/10 17:05:53 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Radiobox.labelMargin	10	widgetDefault
option add *Radiobox.labelFont     \
      "-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-*"  widgetDefault
option add *Radiobox.labelPos		nw	widgetDefault
option add *Radiobox.borderWidth	2	widgetDefault
option add *Radiobox.relief		groove	widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Radiobox class.
#
proc ::itcl::widgets::radiobox {pathName args} {
    uplevel ::itcl::widgets::Radiobox $pathName $args
}

# ------------------------------------------------------------------
#                            RADIOBOX
# ------------------------------------------------------------------
::itcl::extendedclass Radiobox {
    inherit ::itcl::widgets::Labeledframe

    option [list -disabledforeground \
	disabledForeground DisabledForeground] -default {}
    option [list -selectcolor selectColor Background] -default {}
    option [list -command command Command] -default {} -configuremethod configCommand
    option [list -orient orient Orient] -default vertical -configuremethod configOrint

    private variable _buttons {}       ;# List of radiobutton tags.
    private variable _unique 0         ;# Unique id for choice creation.

    private common _modes              ;# Current selection.

    constructor {args} {}
    destructor  {}

    private method gettag {index}      ;# Get the tag of the checkbutton
                                       ;# associated with a numeric index
    private method _rearrange {}       ;# List of radiobutton tags.

    protected method _command { name1 name2 opt }
    protected method configCommand {option value}
    protected method configOrient {option value}

    public method add {tag args}
    public method buttonconfigure {index args}
    public method component {{name ""} args}
    public method delete {index}
    public method deselect {index}
    public method flash {index}
    public method get {}
    public method index {index}
    public method insert {index tag args}
    public method select {index}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Radiobox::constructor {args} {
    #
    # Initialize the _modes array element prior to setting the trace. This
    # prevents the -command command (if defined) from being triggered when
    # the first radiobutton is added via the add method.
    #
    set _modes($this) {}
    trace variable [itcl::scope _modes($this)] w [itcl::code $this _command]
    grid columnconfigure $childsite 0 -weight 1
    uplevel 0 itcl_initoptions $args
}

# ------------------------------------------------------------------
#                        DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Radiobox::destructor { } {
    trace vdelete [itcl::scope _modes($this)] w [itcl::code $this _command]
    catch {unset _modes($this)}
}

# ------------------------------------------------------------------
#                            OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -command
#
# Specifies a command to be evaluated upon change in the radiobox
# ------------------------------------------------------------------
::itcl::body Radiobox::configCommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -orient
#
# Allows the user to orient the radiobuttons either horizontally
# or vertically.
# ------------------------------------------------------------------
::itcl::body Radiobox::configOrient {option value} {
    if {($value eq "horizontal") || ($value eq "vertical")} {
        _rearrange
    } else {
        error "Bad orientation: $value.  Should be\
            \"horizontal\" or \"vertical\"."
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: index index
#
# Searches the radiobutton tags in the radiobox for the one with the
# requested tag, numerical index, or keyword "end".  Returns the 
# choices's numerical index if found, otherwise error.
# ------------------------------------------------------------------
::itcl::body Radiobox::index {index} {
    if {[llength $_buttons] > 0} {
        if {[regexp {(^[0-9]+$)} $index]} {
            if {$index < [llength $_buttons]} {
                return $index
            } else {
                error "Radiobox index \"$index\" is out of range"
            }

        } elseif {$index == "end"} {
            return [expr {[llength $_buttons] - 1}]
        } else {
            if {[set idx [lsearch $_buttons $index]] != -1} {
                return $idx
            }
            error "bad Radiobox index \"$index\": must be number, end,\
                    or pattern"
        }
    } else {
        error "Radiobox \"$win\" has no radiobuttons"
    }
}

# ------------------------------------------------------------------
# METHOD: add tag ?option value option value ...?
#
# Add a new tagged radiobutton to the radiobox at the end.  The method 
# takes additional options which are passed on to the radiobutton
# constructor.  These include most of the typical radiobutton 
# options.  The tag is returned.
# ------------------------------------------------------------------
::itcl::body Radiobox::add {tag args} {
    set options {-value -variable}
    foreach option $options {
      if {[lsearch $args $option] != -1} {
	error "Error: specifying values for radiobutton component options\
	  \"-value\" and\n  \"-variable\" is disallowed.  The Radiobox must\
	  use these options when\n  adding radiobuttons."
      }
    }
    if {![::info exists $tag]} {
        ::itcl::addcomponent $this $tag
    }
    setupcomponent $tag using radiobutton $childsite.rb[incr _unique] \
            -variable [list [itcl::scope _modes($this)]] \
            -anchor w \
            -justify left \
            -highlightthickness 0 \
            -value $tag {*}$args
    keepcomponentoption $tag -background -borderwidth -cursor \
        -disabledforeground -foreground -labelfont -selectcolor
    keepcomponentoption $tag -state
#FIXME      ignore -highlightthickness -highlightcolor
# FIXME delegate option [list -labelfont labelFont Font] to $tag as -font
    lappend _buttons $tag
    grid [set $tag]
    after idle [itcl::code $this _rearrange]
    return $tag
}

# ------------------------------------------------------------------
# METHOD: insert index tag ?option value option value ...?
#
# Insert the tagged radiobutton in the radiobox just before the 
# one given by index.  Any additional options are passed on to the
# radiobutton constructor.  These include the typical radiobutton
# options.  The tag is returned.
# ------------------------------------------------------------------
::itcl::body Radiobox::insert {index tag args} {
    set options {-value -variable}
    foreach option $options {
      if {[lsearch $args $option] != -1} {
	error "Error: specifying values for radiobutton component options\
	  \"-value\" and\n  \"-variable\" is disallowed.  The Radiobox must\
	  use these options when\n  adding radiobuttons."
      }
    }
    if {![::info exists $tag]} {
        ::itcl::addcomponent $this $tag
    }
    setupcomponent $tag using radiobutton $childsite.rb[incr _unique] \
            -variable [list [itcl::scope _modes($this)]] \
            -highlightthickness 0 \
            -anchor w \
            -justify left \
            -value $tag {*}$args
    keepcomponentoption $tag -background -borderwidth -cursor \
        -disabledforeground -foreground -labelfont -selectcolor
# FIXME      ignore -highlightthickness -highlightcolor
# FIXME      rename -font -labelfont labelFont Font
    set index [index $index]
    set before [lindex $_buttons $index]
    set _buttons [linsert $_buttons $index $tag]
    grid [set $tag]
    after idle [itcl::code $this _rearrange]
    return $tag
}

# ------------------------------------------------------------------
# METHOD: _rearrange
#
# Rearrange the buttons in the childsite frame using the grid
# geometry manager.  This method was modified by Chad Smith on 3/9/00
# to take into consideration the newly added -orient config option.
# ------------------------------------------------------------------
::itcl::body Radiobox::_rearrange {} {
    if {[set count [llength $_buttons]] > 0} {
	if {$itcl_options(-orient) == "vertical"} {
            set row 0
	    foreach tag $_buttons {
	        grid configure [set $tag] -column 0 -row $row -sticky nw
	        grid rowconfigure $childsite $row -weight 0
	        incr row
	    }
	    grid rowconfigure $childsite [expr {$count-1}] \
	      -weight 1
	} else {
            set col 0
	    foreach tag $_buttons {
		grid configure [set $tag] -column $col -row 0 -sticky nw
	        grid columnconfigure $childsite $col -weight 1
		incr col
	    }
	}
    }
}

# ------------------------------------------------------------------
# METHOD: component ?name? ?arg arg arg...?
#
# This method overrides the base class definition to provide some
# error checking. The user is disallowed from modifying the values
# of the -value and -variable options for individual radiobuttons.
# Addition of this method prompted by SF ticket 227923.
# ------------------------------------------------------------------
::itcl::body Radiobox::component {{name ""} args} {
  if {[lsearch $_buttons $name] != -1} {
    # See if the user's trying to use the configure method. Note that
    # because of globbing, as few characters as "co" are expanded to
    # "config".  Similarly, "configu" will expand to "configure".
    if [regexp {^co+} [lindex $args 0]] {
      # The user's trying to modify a radiobutton.  This is all fine and
      # dandy unless -value or -variable is being modified.
      set options {-value -variable}
      foreach option $options {
	set index [lsearch $args $option]
        if {$index != -1} {
          # If a value is actually specified, throw an error.
          if {[lindex $args [expr {$index + 1}]] != ""} {
            error "Error: specifying values for radiobutton component options\
              \"-value\" and\n  \"-variable\" is disallowed.  The Radiobox\
              uses these options internally."
          }
        }
      }
    }
  }
  uplevel 0 chain $name $args
}

# ------------------------------------------------------------------
# METHOD: delete index
#
# Delete the specified radiobutton.
# ------------------------------------------------------------------
::itcl::body Radiobox::delete {index} {
    set tag [gettag $index]
    set index [index $index]
    destroy [set $tag]
    set _buttons [lreplace $_buttons $index $index]
    if {$_modes($this) == $tag} {
        set _modes($this) {}
    }
    after idle [itcl::code $this _rearrange]
    return
}

# ------------------------------------------------------------------
# METHOD: select index
#
# Select the specified radiobutton.
# ------------------------------------------------------------------
::itcl::body Radiobox::select {index} {
    set tag [gettag $index]
    [set $tag] invoke
}

# ------------------------------------------------------------------
# METHOD: get
#
# Return the tag of the currently selected radiobutton.
# ------------------------------------------------------------------
::itcl::body Radiobox::get {} {
    return $_modes($this)
}

# ------------------------------------------------------------------
# METHOD: deselect index
#
# Deselect the specified radiobutton.
# ------------------------------------------------------------------
::itcl::body Radiobox::deselect {index} {
    set tag [gettag $index]
    [set $tag] deselect
}

# ------------------------------------------------------------------
# METHOD: flash index
#
# Flash the specified radiobutton.
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Radiobox::flash {index} {
    set tag [gettag $index]
    [set $tag] flash  
}

# ------------------------------------------------------------------
# METHOD: buttonconfigure index ?option? ?value option value ...?
#
# Configure a specified radiobutton.  This method allows configuration 
# of radiobuttons from the Radiobox level.  The options may have any 
# of the values accepted by the add method.
# ------------------------------------------------------------------
::itcl::body Radiobox::buttonconfigure {index args} { 
    set tag [gettag $index]
    uplevel 0 [set $tag] configure $args
}

# ------------------------------------------------------------------
# CALLBACK METHOD: _command name1 name2 opt 
#
# Tied to the trace on _modes($this). Whenever our -variable for our
# radiobuttons change, this method is invoked. It in turn calls
# the user specified tcl script given by -command.
# ------------------------------------------------------------------
::itcl::body Radiobox::_command { name1 name2 opt } {
    uplevel #0 $itcl_options(-command)
}

# ------------------------------------------------------------------
# METHOD: gettag index
#
# Return the tag of the checkbutton associated with a specified
# numeric index
# ------------------------------------------------------------------
::itcl::body Radiobox::gettag {index} {
    return [lindex $_buttons [index $index]]
}

} ; # end ::itcl::widgets
