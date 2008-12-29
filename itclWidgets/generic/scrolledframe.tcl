#
# Scrolledframe
# ----------------------------------------------------------------------
# Implements horizontal and vertical scrollbars around a childsite
# frame.  Includes options to control display of scrollbars.
#
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Scrolledframe
# written by:
#
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: scrolledframe.tcl,v 1.1.2.1 2008/12/29 13:07:54 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Scrolledframe.width 100 widgetDefault
option add *Scrolledframe.height 100 widgetDefault
option add *Scrolledframe.labelPos n widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Scrolledframe class.
# 
proc ::itcl::widgets::scrolledframe {pathName args} {
    uplevel ::itcl::widgets::Scrolledframe $pathName $args
}

# ------------------------------------------------------------------
#                            SCROLLEDFRAME
# ------------------------------------------------------------------
itcl::extendedclass Scrolledframe {
    inherit ::itcl::widgets::Scrolledwidget

    component clipper
    component canvas
    protected component sfchildsite

    constructor {args} {}
    destructor {}

    protected method _configureCanvas {} 
    protected method _configureFrame {} 

    public method childsite {} 
    public method justify {direction} 
    public method xview {args} 
    public method yview {args} 
}


# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
itcl::body Scrolledframe::constructor {args} {
#    itcl_options remove ::itcl::widgets::Labeledwidget::state

    #
    # Create a clipping frame which will provide the border for
    # relief display.
    #
    setupcomponent clipper using frame $itcl_interior.clipper 
    keepcomponentoption clipper -activebackground -activerelief -background \
        -borderwidth -cursor \
	-elementborderwidth -foreground -highlightcolor -highlightthickness \
	-jump -labelfont -troughcolor

    keepcomponentoption clipper -borderwidth -relief 

    grid $clipper -row 0 -column 0 -sticky nsew
    grid rowconfigure $_interior 0 -weight 1
    grid columnconfigure $_interior 0 -weight 1

    # 
    # Create a canvas to scroll
    #
    setupcomponent canvas using canvas $clipper.canvas \
		-height 1.0 -width 1.0 \
                -scrollregion "0 0 1 1" \
                -xscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.horizsb] \
		-yscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.vertsb] \
	        -highlightthickness 0 -takefocus 0
#	ignore -highlightcolor -highlightthickness
    keepcomponentoption canvas -background -cursor

    grid $canvas -row 0 -column 0 -sticky nsew
    grid rowconfigure $clipper 0 -weight 1
    grid columnconfigure $clipper 0 -weight 1
    
    # 
    # Configure the command on the vertical scroll bar in the base class.
    #
    $vertsb configure \
	-command [itcl::code $canvas yview]

    #
    # Configure the command on the horizontal scroll bar in the base class.
    #
    $horizsb configure \
		-command [itcl::code $canvas xview]
    
    #
    # Handle configure events on the canvas to adjust the frame size
    # according to the scrollregion.
    #
    bind $canvas <Configure> [itcl::code $this _configureCanvas]
    
    #
    # Create a Frame inside canvas to hold widgets to be scrolled 
    #
    setupcomponent sfchildsite using frame $canvas.sfchildsite 
    keepcomponentoption sfchildsite -activebackground -activerelief \
        -background \
        -borderwidth -cursor \
	-elementborderwidth -foreground -highlightcolor -highlightthickness \
	-jump -labelfont -troughcolor
    keepcomponentoption sfchildsite -background -cursor
    pack $sfchildsite -fill both -expand yes
    $canvas create window 0 0 -tags frameTag \
            -window $sfchildsite -anchor nw
    set itcl_interior $sfchildsite
    bind $sfchildsite <Configure> [itcl::code $this _configureFrame]
    
    #
    # Initialize the widget based on the command line options.
    #
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

# ------------------------------------------------------------------
#                           DESTURCTOR
# ------------------------------------------------------------------
itcl::body Scrolledframe::destructor {} {
}


# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Returns the path name of the child site widget.
# ------------------------------------------------------------------
itcl::body Scrolledframe::childsite {} {
    return $sfchildsite
}

# ------------------------------------------------------------------
# METHOD: justify
#
# Justifies the scrolled region in one of four directions: top,
# bottom, left, or right.
# ------------------------------------------------------------------
itcl::body Scrolledframe::justify {direction} {
    if {[winfo ismapped $canvas]} {
	update idletasks
	
	switch $direction {
	left {
	    $canvas xview moveto 0
	  }
	right {
	    $canvas xview moveto 1
	  }
	top {
	    $canvas yview moveto 0
	  }
	bottom {
	    $canvas yview moveto 1
	  }
	default {
	    error "bad justify argument \"$direction\": should be\
			left, right, top, or bottom"
	  }
	}
    }
}

# ------------------------------------------------------------------
# METHOD: xview index
#
# Adjust the view in the frame so that character position index
# is displayed at the left edge of the widget.
# ------------------------------------------------------------------
itcl::body Scrolledframe::xview {args} {
    return [uplevel 0 $canvas xview $args]
}

# ------------------------------------------------------------------
# METHOD: yview index
#
# Adjust the view in the frame so that character position index
# is displayed at the top edge of the widget.
# ------------------------------------------------------------------
itcl::body Scrolledframe::yview {args} {
    return [uplevel 0 $canvas yview $args]
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _configureCanvas 
#
# Responds to configure events on the canvas widget.  When canvas 
# changes size, adjust frame size.
# ------------------------------------------------------------------
itcl::body Scrolledframe::_configureCanvas {} {
    set sr [$canvas cget -scrollregion]
    set srw [lindex $sr 2]
    set srh [lindex $sr 3]
    
    $sfchildsite configure -height $srh -width $srw
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _configureFrame 
#
# Responds to configure events on the frame widget.  When the frame 
# changes size, adjust scrolling region size.
# ------------------------------------------------------------------
itcl::body Scrolledframe::_configureFrame {} {
    $canvas configure \
	    -scrollregion [$canvas bbox frameTag] 
}

} ; # end ::itcl::widgets
