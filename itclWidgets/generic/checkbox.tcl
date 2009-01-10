#
# Checkbox
# ----------------------------------------------------------------------
# Implements a checkbuttonbox.  Supports adding, inserting, deleting,
# selecting, and deselecting of checkbuttons by tag and index.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Checkbox
# written by:
#  AUTHOR: John A. Tucker                EMAIL: jatucker@spd.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: checkbox.tcl,v 1.1.2.1 2009/01/10 19:06:14 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Checkbox.labelMargin	10	widgetDefault
option add *Checkbox.labelFont     \
      "-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-*"  widgetDefault
option add *Checkbox.labelPos		nw	widgetDefault
option add *Checkbox.borderWidth	2	widgetDefault
option add *Checkbox.relief		groove	widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Checkbox class.
#
proc ::itcl::widgets::checkbox {pathName args} {
    uplevel ::itcl::widgets::Checkbox $pathName $args
}

# ------------------------------------------------------------------
#                            CHECKBOX
# ------------------------------------------------------------------
::itcl::extendedclass Checkbox {
    inherit ::itcl::widgets::Labeledframe

    option [list -orient orient Orient] -default vertical -configuremethod configOrient

    private variable _unique 0         ;# Unique id for choice creation.
    private variable _buttons {}       ;# List of checkbutton tags.

    private common buttonVar           ;# Array of checkbutton "-variables"

    constructor {args} {}

    private method gettag {index}      ;# Get the tag of the checkbutton
                                       ; # associated with a numeric index

    protected method configOrient {option value}

    public method add {tag args}
    public method insert {index tag args}
    public method delete {index}
    public method get {{index ""}}
    public method index {index}
    public method select {index}
    public method deselect {index}
    public method flash {index}
    public method toggle {index}
    public method buttonconfigure {index args}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Checkbox::constructor {args} {
    uplevel 0 itcl_initoptions $args
}

# ------------------------------------------------------------------
#                            OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -orient
#
# Allows the user to orient the checkbuttons either horizontally
# or vertically.  Added by Chad Smith (csmith@adc.com) 3/10/00.
# ------------------------------------------------------------------
::itcl::body Checkbox::configOrient {option value} {
    if {$value eq "horizontal"} {
        foreach tag $_buttons {
            pack [set $tag] -side left -anchor nw -padx 4 -expand 1
        }
    } elseif {$value eq "vertical"} {
        foreach tag $_buttons {
            pack [set $tag] -side top -anchor w -padx 4 -expand 0
        }
    } else {
        error "Bad orientation: $value.  Should be\
            \"horizontal\" or \"vertical\"."
    }
}


# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: index index
#
# Searches the checkbutton tags in the checkbox for the one with the
# requested tag, numerical index, or keyword "end".  Returns the 
# choices's numerical index if found, otherwise error.
# ------------------------------------------------------------------
::itcl::body Checkbox::index {index} {
    if {[llength $_buttons] > 0} {
        if {[regexp {(^[0-9]+$)} $index]} {
            if {$index < [llength $_buttons]} {
                return $index
            } else {
                error "Checkbox index \"$index\" is out of range"
            }
        } elseif {$index eq "end"} {
            return [expr {[llength $_buttons] - 1}]
        } else {
            if {[set idx [lsearch $_buttons $index]] != -1} {
                return $idx
            }
            error "bad Checkbox index \"$index\": must be number, end,\
                    or pattern"
        }

    } else {
        error "Checkbox \"$win\" has no checkbuttons"
    }
}

# ------------------------------------------------------------------
# METHOD: add tag ?option value option value ...?
#
# Add a new tagged checkbutton to the checkbox at the end.  The method 
# takes additional options which are passed on to the checkbutton
# constructor.  These include most of the typical checkbutton 
# options.  The tag is returned.
# ------------------------------------------------------------------
::itcl::body Checkbox::add {tag args} {
    if {![::info exists $tag]} {
        ::itcl::addcomponent $this $tag
    }
    setupcomponent $tag using checkbutton $childsite.cb[incr _unique] \
            -variable [list [itcl::scope buttonVar($this,$tag)]] \
            -anchor w \
            -justify left \
            -highlightthickness 0 \
            {*}$args
    keepcomponentoption $tag -background -borderwidth -cursor -foreground -labelfont
    keepcomponentoption $tag -command -disabledforeground -selectcolor -state
# FIXME     ignore -highlightthickness -highlightcolor
# FIXME      rename -font -labelfont labelFont Font
    # Redraw the buttons with the proper orientation.
    if {$itcl_options(-orient) == "vertical"} {
        pack [set $tag] -side top -anchor w -padx 4 -expand 0
    } else {
        pack [set $tag] -side left -anchor nw -expand 1
    }
    lappend _buttons $tag
    return $tag
}

# ------------------------------------------------------------------
# METHOD: insert index tag ?option value option value ...?
#
# Insert the tagged checkbutton in the checkbox just before the 
# one given by index.  Any additional options are passed on to the
# checkbutton constructor.  These include the typical checkbutton
# options.  The tag is returned.
# ------------------------------------------------------------------
::itcl::body Checkbox::insert {index tag args} {
    if {![::info exists $tag]} {
        ::itcl::addcomponent $this $tag
    }
    setupcomponent $tag using checkbutton $childsite.cb[incr _unique] \
            -variable [list [itcl::scope buttonVar($this,$tag)]] \
            -anchor w \
            -justify left \
            -highlightthickness 0 \
            {*}$args
    keepcomponentoption $tag -background -borderwidth -cursor -foreground -labelfont
# FIXME     ignore -highlightthickness -highlightcolor
# FIXME      rename -font -labelfont labelFont Font
    set index [index $index]
    set before [lindex $_buttons $index]
    set _buttons [linsert $_buttons $index $tag]
    pack [set $tag] -anchor w -padx 4 -before [set $before]
    return $tag
}

# ------------------------------------------------------------------
# METHOD: delete index
#
# Delete the specified checkbutton.
# ------------------------------------------------------------------
::itcl::body Checkbox::delete {index} {
    set tag [gettag $index]
    set index [index $index]
    destroy [set $tag]
    set _buttons [lreplace $_buttons $index $index]
    if { [info exists buttonVar($this,$tag)] == 1 } {
	unset buttonVar($this,$tag)
    }
}

# ------------------------------------------------------------------
# METHOD: select index
#
# Select the specified checkbutton.
# ------------------------------------------------------------------
::itcl::body Checkbox::select {index} {
    set tag [gettag $index]
    #-----------------------------------------------------------
    # BUG FIX: csmith (Chad Smith: csmith@adc.com), 3/30/99
    #-----------------------------------------------------------
    # This method should only invoke the checkbutton if it's not
    # already selected.  Check its associated variable, and if
    # it's set, then just ignore and return.
    #-----------------------------------------------------------
    if {[set [itcl::scope buttonVar($this,$tag)]] == 
	[[set $tag] cget -onvalue]} {
      return
    }
    [set $tag] invoke
}

# ------------------------------------------------------------------
# METHOD: toggle index
#
# Toggle a specified checkbutton between selected and unselected
# ------------------------------------------------------------------
::itcl::body Checkbox::toggle {index} {
    set tag [gettag $index]
    [set $tag] toggle
}

# ------------------------------------------------------------------
# METHOD: get
#
# Return the value of the checkbutton with the given index, or a
# list of all checkbutton values in increasing order by index.
# ------------------------------------------------------------------
::itcl::body Checkbox::get {{index ""}} {
    set result {}
    if {$index eq ""} {
	foreach tag $_buttons {
	    if {$buttonVar($this,$tag)} {
		lappend result $tag
	    }
	}
    } else {
        set tag [gettag $index]
	set result $buttonVar($this,$tag)
    }
    return $result
}

# ------------------------------------------------------------------
# METHOD: deselect index
#
# Deselect the specified checkbutton.
# ------------------------------------------------------------------
::itcl::body Checkbox::deselect {index} {
    set tag [gettag $index]
    [set $tag] deselect
}

# ------------------------------------------------------------------
# METHOD: flash index
#
# Flash the specified checkbutton.
# ------------------------------------------------------------------
::itcl::body Checkbox::flash {index} {
    set tag [gettag $index]
    [set $tag] flash  
}

# ------------------------------------------------------------------
# METHOD: buttonconfigure index ?option? ?value option value ...?
#
# Configure a specified checkbutton.  This method allows configuration 
# of checkbuttons from the Checkbox level.  The options may have any 
# of the values accepted by the add method.
# ------------------------------------------------------------------
::itcl::body Checkbox::buttonconfigure {index args} { 
    set tag [gettag $index]
    [set $tag] configure {*}$args
}

# ------------------------------------------------------------------
# METHOD: gettag index
#
# Return the tag of the checkbutton associated with a specified
# numeric index
# ------------------------------------------------------------------
::itcl::body Checkbox::gettag {index} {
    return [lindex $_buttons [index $index]]
}

} ; # end ::itcl::widgets
