#
# Pushbutton
# ----------------------------------------------------------------------
# Implements a Motif-like Pushbutton with an optional default ring.
#
# WISH LIST:
#    1)  Allow bitmaps and text on the same button face (Tk limitation).
#    2)  provide arm and disarm bitmaps.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Pushbutton
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Bret A. Schuhmacher      EMAIL: bas@wn.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: pushbutton.tcl,v 1.1.2.3 2009/01/05 21:11:13 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Pushbutton.borderWidth 2 widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Pushbutton class.
# 
proc ::itcl::widgets::pushbutton {pathName args} {
    uplevel ::itcl::widgets::Pushbutton $pathName $args
}

# ------------------------------------------------------------------
#                            PUSHBUTTON
# ------------------------------------------------------------------
::itcl::extendedclass Pushbutton {
    component itcl_hull
    component itcl_interior
    component pushbutton

    option [list -padx padX Pad] -default 11 -configuremethod configPadx
    option [list -pady padY Pad] -default 4 -configuremethod configPady
    option [list -font font Font] -default \
	    -Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-* -configuremethod configFont
    option [list -text text Text] -default {} -configuremethod configText
    option [list -bitmap bitmap Bitmap] -default {} -configuremethod configBitmap
    option [list -image image Image] -default {} -configuremethod configImage
    option [list -highlightthickness highlightThickness \
	    HighlightThickness] -default 2 -configuremethod configHighlightthickness
    option [list -borderwidth borderWidth BorderWidth] -default 2 -configuremethod configBorderwidth
    option [list -defaultring defaultRing DefaultRing] -default 0 -configuremethod configDefaultring
    option [list -defaultringpad defaultRingPad Pad] -default 4 -configuremethod configDefaultringpad
    option [list -height height Height] -default 0 -configuremethod configHeight
    option [list -width width Width] -default 0 -configuremethod configWidth
    option [list -takefocus takeFocus TakeFocus] -default 0

    private variable _origWin

    constructor {args} {}
    destructor {}

    protected method _relayout {{when later}} 
    protected variable _reposition ""  ;# non-null => _relayout pending
    protected method configPadx {option value}
    protected method configPady {option value}
    protected method configFont {option value}
    protected method configText {option value}
    protected method configBitmap {option value}
    protected method configImage {option value}
    protected method configHighlightthickness {option value}
    protected method configBorderwidth {option value}
    protected method configDefaultring {option value}
    protected method configDefaultringpad {option value}
    protected method configHeight {option value}
    protected method configWidth {option value}

    public method flash {} 
    public method invoke {} 
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Pushbutton::constructor {args} {
    set win [createhull frame $this -class [info class] -borderwidth 0]
    set _origWin $win
    set itcl_interior $win
    #
    # Reconfigure the hull to act as the outer sunken ring of
    # the pushbutton, complete with focus ring.
    #

    $itcl_hull configure -padx 0 -pady 0 -borderwidth $itcl_options(-borderwidth)
    pack propagate $win no
    setupcomponent pushbutton using button $win.pushbutton 
    keepcomponentoption pushbutton -activebackground -activeforeground \
         -background -borderwidth -cursor -disabledforeground -font \
	 -foreground -highlightbackground -highlightcolor \
	 -highlightthickness 
    keepcomponentoption pushbutton -underline -wraplength -state -command
    pack $pushbutton -expand 1 -fill both
    #
    # Initialize the widget based on the command line options.
    #
    if {[llength $args] > 0} {
        uplevel 0  configure $args
    }

    #
    # Layout the pushbutton.
    #
    _relayout
}

# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Pushbutton::destructor {} {
    if {$_reposition != ""} {
        after cancel $_reposition
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -padx
#
# Specifies the extra space surrounding the label in the x direction.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configPadx {option value} {
    $pushbutton configure -padx $$value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -pady
#
# Specifies the extra space surrounding the label in the y direction.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configPady {option value} {
    $pushbutton configure -pady $value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -font
#
# Specifies the label font.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configFont {option value} {
    $pushbutton configure -font $value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -text
#
# Specifies the label text.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configText {option value} {
    $pushbutton configure -text $value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -bitmap
#
# Specifies the label bitmap.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configBitmap {option value} {
    $pushbutton configure -bitmap $value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -image
#
# Specifies the label image.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configImage {option value} {
    $pushbutton configure -image $value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -highlightthickness
#
# Specifies the thickness of the highlight ring.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configHighlightthickness {option value} {
    $pushbutton configure -highlightthickness $value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -borderwidth
#
# Specifies the width of the relief border.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configBorderwidth {option value} {
    $pushbutton configure -borderwidth $value
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -defaultring
#
# Boolean describing whether the button displays its default ring.  
# ------------------------------------------------------------------
::itcl::body Pushbutton::configDefaultring {option value} {
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -defaultringpad
#
# The size of the padded default ring around the button.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configDefaultringpad {option value} {
    pack $pushbutton -padx $value -pady $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -height
#
# Specifies the height of the button inclusive of any default ring.
# A value of zero lets the push button determine the height based
# on the requested height plus highlightring and defaultringpad.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configHeight {option value} {
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -width
#
# Specifies the width of the button inclusive of any default ring.
# A value of zero lets the push button determine the width based
# on the requested width plus highlightring and defaultringpad.
# ------------------------------------------------------------------
::itcl::body Pushbutton::configWidth {option value} {
    _relayout
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: flash
#
# Thin wrap of standard button widget flash method.
# ------------------------------------------------------------------
::itcl::body Pushbutton::flash {} {
    $pushbutton flash
}

# ------------------------------------------------------------------
# METHOD: invoke
#
# Thin wrap of standard button widget invoke method.
# ------------------------------------------------------------------
::itcl::body Pushbutton::invoke {} {
    $pushbutton invoke
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _relayout ?when?
#
# Adjust the width and height of the Pushbutton to accomadate all the
# current options settings.  Add back in the highlightthickness to 
# the button such that the correct reqwidth and reqheight are computed.  
# Set the width and height based on the reqwidth/reqheight, 
# highlightthickness, and ringpad.  Finally, configure the defaultring
# properly. If "when" is "now", the change is applied immediately.  If 
# it is "later" or it is not specified, then the change is applied later,
# when the application is idle.
# ------------------------------------------------------------------
::itcl::body Pushbutton::_relayout {{when later}} {
    if {$when eq "later"} {
	if {$_reposition eq ""} {
	    set _reposition [after idle [itcl::code $this _relayout now]]
	}
	return
    } elseif {$when ne "now"} {
	error "bad option \"$when\": should be now or later"
    }
    set _reposition ""
    if {$itcl_options(-width) == 0} {
	set w [expr {[winfo reqwidth $pushbutton] \
		+ 2 * $itcl_options(-highlightthickness) \
		+ 2 * $itcl_options(-borderwidth) \
		+ 2 * $itcl_options(-defaultringpad)}]
    } else {
	set w $itcl_options(-width)
    }
    if {$itcl_options(-height) == 0} {
	set h [expr {[winfo reqheight $pushbutton] \
		+ 2 * $itcl_options(-highlightthickness) \
		+ 2 * $itcl_options(-borderwidth) \
		+ 2 * $itcl_options(-defaultringpad)}]
    } else {
	set h $itcl_options(-height)
    }
    $itcl_hull configure -width $w -height $h
    if {$itcl_options(-defaultring)} {
	$itcl_hull configure -relief sunken \
		-highlightthickness [$this cget -highlightthickness] \
		-takefocus 1
	configure -takefocus 1
	$pushbutton configure \
		-highlightthickness 0 -takefocus 0
	
    } else {
	$itcl_hull configure -relief flat \
		-highlightthickness 0 -takefocus 0
	$pushbutton configure \
		-highlightthickness [$this cget -highlightthickness] \
		-takefocus 1
	configure -takefocus 0
    }
}

} ; # end ::itcl::widgets
