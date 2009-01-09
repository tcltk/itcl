#
# Spinint
# ----------------------------------------------------------------------
# Implements an integer spinner widget.  It inherits basic spinner
# functionality from Spinner and adds specific features to create 
# an integer-only spinner. 
# Arrows may be placed horizontally or vertically.
# User may specify an integer range and step value.
# Spinner may be configured to wrap when min or max value is reached.
#
# NOTE:
# Spinint integer values should not exceed the size of a long integer.
# For a 32 bit long the integer range is -2147483648 to 2147483647.
#
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Spinint
# written by:
#    Sue Yockey               E-mail: yockey@acm.org
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: spinint.tcl,v 1.1.2.1 2009/01/09 16:58:17 wiede Exp $
# ======================================================================

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Spinint class.
# 
proc ::itcl::widgets::spinint {pathName args} {
    uplevel ::itcl::widgets::Spinint $pathName $args
}

# ------------------------------------------------------------------
#                            SPININT
# ------------------------------------------------------------------
::itcl::extendedclass Spinint {
    inherit ::itcl::widgets::Spinner 

    option [list -range range Range] -default "" -configuremethod configRange
    option [list -step step Step] -default 1 -configuremethod configStep
    option [list -wrap wrap Wrap] -default true -configuremethod configWrap

    constructor {args} {Spinner::constructor -validate numeric} {}

    protected method configRange {option value}
    protected method configStep {option value}
    protected method configWrap {option value}

    public method up {}
    public method down {}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Spinint::constructor {args} {
    uplevel 0 itcl_initoptions $args
    $entry delete 0 end
    if {[lindex $itcl_options(-range) 0] == ""} {
	$entry insert 0 "0"
    } else { 
	$entry insert 0 [lindex $itcl_options(-range) 0] 
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -range
#
# Set min and max values for spinner.
# ------------------------------------------------------------------
::itcl::body Spinint::configRange {option value} {
    if {$value ne ""} {
	if {[llength $value] != 2} {
	    error "wrong # args: should be\
		    \"$win configure -range {begin end}\""
    	}

    	set min [lindex $value 0]
    	set max [lindex $value 1]

    	if {![regexp {^-?[0-9]+$} $min]} {
    	    error "bad range option \"$min\": begin value must be\
		    an integer"
    	}
    	if {![regexp {^-?[0-9]+$} $max]} {
    	    error "bad range option \"$max\": end value must be\
		    an integer"
    	}
    	if {$min > $max} {
    	    error "bad option starting range \"$min\": must be less\
		    than ending: \"$max\""
    	}
    } 
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -step
#
# Increment spinner by step value.
# ------------------------------------------------------------------
::itcl::body Spinint::configStep {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -wrap
#
# Specify whether spinner should wrap value if at min or max.
# ------------------------------------------------------------------
::itcl::body Spinint::configWrap {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: up
#
# Up arrow button press event.  Increment value in entry.
# ------------------------------------------------------------------
::itcl::body Spinint::up {} {
    set min_range [lindex $itcl_options(-range) 0]
    set max_range [lindex $itcl_options(-range) 1]
    set val [$entry get]
    if {[lindex $itcl_options(-range) 0] != ""} {
	#
	# Check boundaries.
	#
	if {$val >= $min_range && $val < $max_range} {
	    incr val $itcl_options(-step)
	    $entry delete 0 end
	    $entry insert 0 $val
	} else {
	    if {$itcl_options(-wrap)} {
		if {$val >= $max_range} {
		    $entry delete 0 end
		    $entry insert 0 $min_range 
		} elseif {$val < $min_range} {
		    $entry delete 0 end
		    $entry insert 0 $min_range 
		} else {
		    uplevel #0 $itcl_options(-invalid)
		}
	    } else {
		uplevel #0 $itcl_options(-invalid)
	    }
	}
    } else {
	#
	# No boundaries.
	#
	incr val $itcl_options(-step)
	$entry delete 0 end
	$entry insert 0 $val
    }
}

# ------------------------------------------------------------------
# METHOD: down 
#
# Down arrow button press event.  Decrement value in entry.
# ------------------------------------------------------------------
::itcl::body Spinint::down {} {
    set min_range [lindex $itcl_options(-range) 0]
    set max_range [lindex $itcl_options(-range) 1]
    set val [$entry get]
    if {[lindex $itcl_options(-range) 0] != ""} {
	#
	# Check boundaries.
	#
	if {$val > $min_range && $val <= $max_range} {
	    incr val -$itcl_options(-step)
	    $entry delete 0 end
	    $entry insert 0 $val
	} else {
	    if {$itcl_options(-wrap)} {
		if {$val <= $min_range} {
		    $entry delete 0 end
		    $entry insert 0 $max_range
		} elseif {$val > $max_range} {
		    $entry delete 0 end
		    $entry insert 0 $max_range
		} else {
		    uplevel #0 $itcl_options(-invalid)
		}
	    } else {
		uplevel #0 $itcl_options(-invalid)
	    }
	}
    } else {
	#
	# No boundaries.
	#
	incr val -$itcl_options(-step)
	$entry delete 0 end
	$entry insert 0 $val
    }
}

} ; # end ::itcl::widgets
