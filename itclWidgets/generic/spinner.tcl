#
# Spinner
# ----------------------------------------------------------------------
# Implements a spinner widget.  The Spinner is comprised of an  
# EntryField plus up and down arrow buttons. 
# Spinner is meant to be used as a base class for creating more
# specific spinners such as SpinInt.itk
# Arrows may be drawn horizontally or vertically.
# User may define arrow behavior or accept the default arrow behavior.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the ::itcl::widgets package Spinner
# written by:
#    Sue Yockey               E-mail: yockey@acm.org
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: spinner.tcl,v 1.1.2.1 2009/01/09 16:58:17 wiede Exp $
# ======================================================================

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Spinner class.
# 
proc ::itcl::widgets::spinner {pathName args} {
    uplevel ::itcl::widgets::Spinner $pathName $args
}

# ------------------------------------------------------------------
#                              SPINNER
# ------------------------------------------------------------------
::itcl::extendedclass Spinner {
    inherit ::itcl::widgets::Entryfield 

    component uparrow
    component downarrow

    option [list -arroworient arrowOrient Orient] -default vertical -configuremethod configArroworient
    option [list -textfont textFont \
	    Font] -default -Adobe-Helvetica-Medium-R-Normal--*-120-*-*-*-*-*-* -configuremethod configTextfont
    option [list -borderwidth borderWidth BorderWidth] -default 2 -configuremethod configBorderwidth
    option [list -highlightthickness highlightThickness \
	    HighlightThickness] -default 2 -configuremethod configHighlightthickness
    option [list -increment increment Command] -default {} -configuremethod configIncrement
    option [list -decrement decrement Command] -default {} -configuremethod configDecrement
    option [list -repeatdelay repeatDelay RepeatDelay] -default 300  -configuremethod configRepeatdelay
    option [list -repeatinterval repeatInterval RepeatInterval] -default 100 -configuremethod configRepeatinterval
    option [list -foreground foreground Foreground] -default black -configuremethod configForeground

#    delegate option -insertborderwidth to entry

    constructor {args} {}
    destructor {}

    public method down {}
    public method up {}

    protected method _pushup {}
    protected method _pushdown {}
    protected method _relup {}
    protected method _reldown {}
    protected method _doup {rate}
    protected method _dodown {rate}
    protected method _up {}
    protected method _down {}
    protected method configArroworient {option value}
    protected method configTextfont {option value}
    protected method configBorderwidth {option value}
    protected method configHighlightthickness {option value}
    protected method configIncrement {option value}
    protected method configDecrement {option value}
    protected method configRepeatdelay {option value}
    protected method configRepeatinterval {option value}
    protected method configForeground {option value}

    protected method _positionArrows {{when later}}

    protected variable _interior {}
    protected variable _reposition ""  ;# non-null => _positionArrows pending
    protected variable _uptimer ""     ;# non-null => _uptimer pending
    protected variable _downtimer ""   ;# non-null => _downtimer pending
}
    
# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Spinner::constructor {args} {
    #
    # Save off the interior for later use.
    #
    set _interior $itcl_interior
    #
    # Create up arrow button.
    # 
    setupcomponent uparrow using canvas $itcl_interior.uparrow -height 10 \
            -width 10 -relief raised -highlightthickness 0
    keepcomponentoption uparrow -background -borderwidth
    
    #
    # Create down arrow button.
    # 
    setupcomponent downarrow using canvas $itcl_interior.downarrow -height 10 \
            -width 10 -relief raised -highlightthickness 0
    keepcomponentoption downarrow -background -borderwidth
    #
    # Add bindings for button press events on the up and down buttons.
    #
    bind $uparrow <ButtonPress-1> [itcl::code $this _pushup]
    bind $uparrow <ButtonRelease-1> [itcl::code $this _relup]
    bind $downarrow  <ButtonPress-1> [itcl::code $this _pushdown]
    bind $downarrow <ButtonRelease-1> [itcl::code $this _reldown]
    uplevel 0 itcl_initoptions $args
    # 
    # When idle, position the arrows.
    #
    _positionArrows
}

# ------------------------------------------------------------------
#                          DESTRUCTOR
# ------------------------------------------------------------------

::itcl::body Spinner::destructor {} {
    if {$_reposition ne ""} {
        after cancel $_reposition
    }
    if {$_uptimer ne ""} {
        after cancel $_uptimer
    }
    if {$_downtimer ne ""} {
        after cancel $_downtimer
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -arroworient
#
# Place arrows vertically or horizontally .
# ------------------------------------------------------------------
::itcl::body Spinner::configArroworient {option value} {
    set itcl_options($option) $value
    _positionArrows
}

# ------------------------------------------------------------------
# OPTION: -textfont
#
# Change font, resize arrow buttons.
# ------------------------------------------------------------------
::itcl::body Spinner::configTextfont {option value} {
    set itcl_options($option) $value
    _positionArrows
}

# ------------------------------------------------------------------
# OPTION: -highlightthickness
#
# Change highlightthickness, resize arrow buttons.
# ------------------------------------------------------------------
::itcl::body Spinner::configHighlightthickness {option value} {
    set itcl_options($option) $value
    _positionArrows
}

# ------------------------------------------------------------------
# OPTION: -borderwidth
#
# Change borderwidth, resize arrow buttons.
# ------------------------------------------------------------------
::itcl::body Spinner::configBorderwidth {option value} {
    set itcl_options($option) $value
    _positionArrows
}

# ------------------------------------------------------------------
# OPTION: -increment
#
# Up arrow callback. 
# ------------------------------------------------------------------
::itcl::body Spinner::configIncrement {option value} {
    if {$value eq {}} {
	set value [itcl::code $this up]
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -decrement
#
# Down arrow callback. 
# ------------------------------------------------------------------
::itcl::body Spinner::configDecrement {option value} {
    if {$value eq {}} {
	set value [itcl::code $this down]
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -repeatinterval
#
# Arrow repeat rate in milliseconds. A repeatinterval of 0 disables 
# button repeat.
# ------------------------------------------------------------------
::itcl::body Spinner::configRepeatinterval {option value} {
    if {$value < 0} {
       set value 0
    } 
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -repeatdelay
#
# Arrow repeat delay in milliseconds. 
# ------------------------------------------------------------------
::itcl::body Spinner::configRepeatdelay {option value} {
    if {$value < 0} {
       set value 0
    } 
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -foreground
#
# Set the foreground color of the up and down arrows. Remember
# to make sure the "tag" exists before setting them...
# ------------------------------------------------------------------
::itcl::body Spinner::configForeground {option value} {
    if {[$uparrow gettags up] ne "" } {
	$uparrow itemconfigure up -fill $value
    }
    if { [$downarrow gettags down] ne "" } {
	$downarrow itemconfigure down -fill $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: up
#
# Up arrow command.  Meant to be overloaded by derived class. 
# ------------------------------------------------------------------
::itcl::body Spinner::up {} {
}

# ------------------------------------------------------------------
# METHOD: down 
#
# Down arrow command.  Meant to be overloaded by derived class.
# ------------------------------------------------------------------
::itcl::body Spinner::down {} {
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _positionArrows ?when?
#
# Draw Arrows for spinner. If "when" is "now", the change is applied
# immediately.  If it is "later" or it is not specified, then the 
# change is applied later, when the application is idle.
# ------------------------------------------------------------------
::itcl::body Spinner::_positionArrows {{when later}} {
    if {$when eq "later"} {
	if {$_reposition == ""} {
	    set _reposition [after idle [itcl::code $this _positionArrows now]]
	}
	return
    } elseif {$when != "now"} {
	error "bad option \"$when\": should be now or later"
    }
    set _reposition ""
    set bdw [cget -borderwidth]
    #
    # Based on the orientation of the arrows, pack them accordingly and
    # determine the width and height of the spinners.  For vertical 
    # orientation, it is really tight in the y direction, so we'll take 
    # advantage of the highlightthickness.  Horizontal alignment has 
    # plenty of space vertically, thus we'll ignore the thickness.
    # 
    switch $itcl_options(-arroworient) {
    vertical {
        grid $uparrow -row 0 -column 0
        grid $downarrow -row 1 -column 0
        set totalHgt [winfo reqheight $entry] 
        set spinHgt [expr {$totalHgt / 2}]
        set spinWid [expr {round ($spinHgt * 1.6)}]
      }
    horizontal {
        grid $uparrow -row 0 -column 0
        grid $downarrow -row 0 -column 1
        set spinHgt [expr {[winfo reqheight $entry] - \
	    (2 * [$entry cget -highlightthickness])}]
        set spinWid $spinHgt
      }
    default {
        error "bad orientation option \"$itcl_options(-arroworient)\",\
	       should be horizontal or vertical"
      }
    }
    #
    # Configure the width and height of the spinners minus the borderwidth.
    # Next delete the previous spinner polygons and create new ones.
    #
    $uparrow config \
	    -height [expr {$spinHgt - (2 * $bdw)}] \
	    -width [expr {$spinWid - (2 * $bdw)}]
    $uparrow delete up
    $uparrow create polygon \
	    [expr {$spinWid / 2}] $bdw \
	    [expr {$spinWid - $bdw - 1}] [expr {$spinHgt - $bdw -1}] \
	    [expr {$bdw + 1}] [expr {$spinHgt - $bdw - 1}] \
	    -fill $itcl_options(-foreground) -tags up
    $downarrow config \
	    -height [expr {$spinHgt - (2 * $bdw)}] \
	    -width [expr {$spinWid - (2 * $bdw)}]
    $downarrow delete down
    $downarrow create polygon \
	    [expr {$spinWid / 2}] [expr {($spinHgt - $bdw) - 1}] \
	    [expr {$bdw + 2}] [expr {$bdw + 1}] \
	    [expr {$spinWid - $bdw - 2}] [expr {$bdw + 1}] \
	    -fill $itcl_options(-foreground) -tags down
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _pushup
#
# Up arrow button press event.  Call _doup with repeatdelay. 
# ------------------------------------------------------------------
::itcl::body Spinner::_pushup {} {
    $uparrow config -relief sunken
    _doup $itcl_options(-repeatdelay)
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _pushdown
#
# Down arrow button press event.  Call _dodown with repeatdelay. 
# ------------------------------------------------------------------
::itcl::body Spinner::_pushdown {} {
    $downarrow config -relief sunken
    _dodown $itcl_options(-repeatdelay)
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _doup
#
# Call _up and post to do another one after "rate" milliseconds if
# repeatinterval > 0.
# ------------------------------------------------------------------
::itcl::body Spinner::_doup {rate} {
    _up 
    if {$itcl_options(-repeatinterval) > 0} {
	set _uptimer [after $rate [itcl::code $this _doup $itcl_options(-repeatinterval)]]
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _dodown
#
# Call _down and post to do another one after "rate" milliseconds if 
# repeatinterval > 0.
# ------------------------------------------------------------------
::itcl::body Spinner::_dodown {rate} {
    _down 
    if {$itcl_options(-repeatinterval) > 0} {
	set _downtimer \
		[after $rate [itcl::code $this _dodown $itcl_options(-repeatinterval)]]
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _relup
#
# Up arrow button release event.  Cancel pending up timer.
# ------------------------------------------------------------------
::itcl::body Spinner::_relup {} {
    $uparrow config -relief raised
    if {$_uptimer != ""} {
	after cancel $_uptimer 
	set _uptimer ""
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _reldown
#
# Up arrow button release event.  Cancel pending down timer.
# ------------------------------------------------------------------
::itcl::body Spinner::_reldown {} {
    $downarrow config -relief raised
    if {$_downtimer ne ""} { 
	after cancel $_downtimer
	set _downtimer ""
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _up
#
# Up arrow button press event.  Call defined increment command. 
# ------------------------------------------------------------------
::itcl::body Spinner::_up {} {
    uplevel #0 $itcl_options(-increment)
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _down 
#
# Down arrow button press event.  Call defined decrement command. 
# ------------------------------------------------------------------
::itcl::body Spinner::_down {} {
    uplevel #0 $itcl_options(-decrement)
}

} ; # end ::itcl::widgets
