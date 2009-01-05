#
# Timeentry
# ----------------------------------------------------------------------
# Implements a quicken style time entry field with a popup clock
# by combining the timefield and watch widgets together.  This
# allows a user to enter the time via the keyboard or by using the
# mouse by selecting the watch icon which brings up a popup clock.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Timeentry
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: timeentry.tcl,v 1.1.2.1 2009/01/05 21:32:23 wiede Exp $
# ======================================================================

    keep -background -borderwidth -cursor -foreground -highlightcolor \
	-highlightthickness -labelfont -textbackground -textfont

#
# Use option database to override default resources of base classes.
#
option add *Timeentry.watchWidth 155 widgetDefault
option add *Timeentry.watchHeight 175 widgetDefault
 
namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the timeentry class.
# 
proc ::itcl::widgets::timeentry {pathName args} {
    uplevel ::itcl::widgets::Timeentry $pathName $args
}

# ------------------------------------------------------------------
#                              TIMEENTRY
# ------------------------------------------------------------------
::itcl::extendedclass Timeentry {
    inherit ::itcl::widgets::Timefield
    
    component iconbutton

    option [list -grab grab Grab] -default "global" -configuremethod configGrab
    option [list -icon icon Icon] -default {} -configuremethod configIcon
    option [list -state state State] -default normal -configuremethod configState
    option [list -closetext closeText Text] -default Close

    #
    # The watch widget isn't created until needed, yet we need
    # its options to be available upon creation of a timeentry widget.
    # So, we'll define them in these class now so they can just be
    # propagated onto the watch later.
    #
    option [list -hourradius hourRadius Radius] -default .50
    option [list -hourcolor hourColor Color] -default red
    
    option [list -minuteradius minuteRadius Radius] -default .80
    option [list -minutecolor minuteColor Color] -default yellow
    
    option [list -pivotradius pivotRadius Radius] -default .10
    option [list -pivotcolor pivotColor Color] -default white
    
    option [list -secondradius secondRadius Radius] -default .90
    option [list -secondcolor secondColor Color] -default black
    
    option [list -clockcolor clockColor Color] -default white
    option [list -clockstipple clockStipple ClockStipple] -default {}
    
    option [list -tickcolor tickColor Color] -default black

    option [list -watchheight watchHeight Height] -default 175
    option [list -watchwidth watchWidth Width] -default 155

    constructor {args} {}

    protected method _getPopupTime {}
    protected method _releaseGrab {}
    protected method _popup {}
    protected method _getDefaultIcon {}
    protected method configGrab {option value}
    protected method configIcon {option value}
    protected method configState {option value}

    protected common _defaultIcon ""
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Timeentry::constructor {args} {
    #
    # Create an icon label to act as a button to bring up the 
    # watch popup.
    #
    setupcomponent iconbutton using label $itcl_interior.iconbutton -relief raised
    keepcomponentoption iconbutton -borderwidth -cursor -foreground 
    }
    grid $iconbutton -row 0 -column 0 -sticky ns

    #
    # Initialize the widget based on the command line options.
    #
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -icon
#
# Specifies the clock icon image to be used in the time entry.  
# Should one not be provided, then a default pixmap will be used
# if possible, bitmap otherwise.
# ------------------------------------------------------------------
::itcl::body Timeentry::configIcon {option value} {
    if {$value eq {}} {
	$iconbutton configure -image [_getDefaultIcon]
    } else {
	if {[lsearch [image names] $value] == -1} {
	    error "bad icon option \"$value\":\
                   should be an existing image"
	} else {
	    $iconbutton configure -image $value
	}
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -grab
#
# Specifies the grab level, local or global, to be obtained when 
# bringing up the popup watch.  The default is global.
# ------------------------------------------------------------------
::itcl::body Timeentry::configGrab {option value} {
    switch -- $value {
    "local" -
    "global" {
      }
    default {
        error "bad grab option \"$value\":\
                  should be local or global"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -state
#
# Specifies the state of the widget which may be disabled or
# normal.  A disabled state prevents selection of the time field
# or time icon button.
# ------------------------------------------------------------------
::itcl::body Timeentry::state {option value} {
    switch -- $value {
    normal {
        bind $iconbutton <Button-1> [itcl::code $this _popup]
      }
    disabled {
        bind $iconbutton <Button-1> {}
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------
# ------------------------------------------------------------------
# PROTECTED METHOD: _getDefaultIcon
#
# This method is invoked uto retrieve the name of the default icon
# image displayed in the icon button.
# ------------------------------------------------------------------
::itcl::body Timeentry::_getDefaultIcon {} {
  if {[lsearch [image types] pixmap] != -1} {
	set _defaultIcon [image create pixmap -data {
	    /* XPM */
	    static char *watch1a[] = {
		/* width height num_colors chars_per_pixel */
		"    20    20        8            1",
		/* colors */
		". c #000000",
		"# c #000099",
		"a c #009999",
		"b c #999999",
		"c c #cccccc",
		"d c #ffff00",
		"e c #d9d9d9",
		"f c #ffffff",
		/* pixels */
		"eeeeebbbcccccbbbeeee",
		"eeeee...#####..beeee",
		"eeeee#aacccccaabeeee",
		"eeee#accccccccc##eee",
		"eee#ccc#cc#ccdcff#ee",
		"ee#accccccccccfcca#e",
		"eeaccccccc.cccfcccae",
		"eeac#cccfc.cccc##cae",
		"e#cccccffc.cccccccc#",
		"e#ccccfffc.cccccccc#",
		"e#cc#ffcc......c#cc#",
		"e#ccfffccc.cccccccc#",
		"e#cffccfcc.cccccccc#",
		"eeafdccfcccccccd#cae",
		"eeafcffcccccccccccae",
		"eee#fcc#cccccdccc#ee",
		"eee#fcc#cc#cc#ccc#ee",
		"eeee#accccccccc##eee",
		"eeeee#aacccccaabeeee",
		"eeeee...#####..beeee"
	    };
	}]
    } else {
	set _defaultIcon [image create bitmap -data {
	    #define watch1a_width 20
	    #define watch1a_height 20
	    static char watch1a_bits[] = {
		0x40,0x40,0xf0,0xe0,0x7f,0xf0,0xe0,0xe0,0xf0,0x30,
		0x80,0xf1,0x88,0x04,0xf2,0x0c,0x00,0xf6,0x04,0x04,
		0xf4,0x94,0x84,0xf5,0x02,0x06,0xf8,0x02,0x0c,0xf8,
		0x12,0x7e,0xf9,0x02,0x04,0xf8,0x02,0x24,0xf8,0x04,
		0x00,0xf5,0x04,0x00,0xf4,0x88,0x02,0xf2,0x88,0x64,
		0xf2,0x30,0x80,0xf1,0xe0,0x60,0xf0,0xe0,0xff,0xf0};
	}]
    }

    #
    # Since this image will only need to be created once, we redefine
    # this method to just return the image name for subsequent calls.
    #
    itcl::body ::itcl::widgets::Timeentry::_getDefaultIcon {} {
	return $_defaultIcon
    }

    return $_defaultIcon
}


# ------------------------------------------------------------------
# PROTECTED METHOD: _popup
#
# This method is invoked upon selection of the icon button.  It 
# creates a watch widget within a toplevel popup, calculates 
# the position at which to display the watch, performs a grab
# and displays the watch.
# ------------------------------------------------------------------
::itcl::body Timeentry::_popup {} {
    #
    # First, let's nullify the icon binding so that any another 
    # selections are ignored until were done with this one.  Next,
    # change the relief of the icon.
    #
    bind $iconbutton <Button-1> {}
    $iconbutton configure -relief sunken

    #
    # Create a withdrawn toplevel widget and remove the window 
    # decoration via override redirect.
    #
    if {![::info exists popup]} {
        ::itcl::addcomponent $this popup -private
    }
    setupcomponent -private popup using toplevel $itcl_interior.popup 
    $popup configure -borderwidth 2 -background black
    wm withdraw $popup
    wm overrideredirect $popup 1

    #
    # Add a binding to for Escape to always release the grab.
    #
    bind $popup <KeyPress-Escape> [itcl::code $this _releaseGrab]

    #
    # Create the watch widget.
    #
    if {![::info exists popup]} {
        ::itcl::addcomponent $this watch
    }
    setupcomponent watch using ::itcl::widgets::Watch $popup.watch
    keepcomponentoption watch -background -borderwidth -cursor -foreground \
        -highlightcolor -highlightthickness -labelfont -textbackground \
	-textfont

# FIXME	rename -width -watchwidth watchWidth Width 
# FIXME	rename -height -watchheight watchHeight Height 

    keepcomponentoption watch -hourradius -minuteradius -minutecolor \
            -pivotradius -pivotcolor -secondradius -secondcolor -clockcolor \
	    -clockstipple -tickcolor 
    
    grid $watch -row 0 -column 0
    $watch configure -cursor top_left_arrow

    #
    # Create a button widget so the user can say they are done.
    #
    setupcomponent close using button $popup.close -command [itcl::code $this _getPopupTime]
    keepcomponentoption watch -hourradius -minuteradius -minutecolor \
            -pivotradius -pivotcolor -secondradius -secondcolor -clockcolor \
	    -clockstipple -tickcolor 
# FIXME	rename -text -closetext closeText Text

    grid $close -row 1 -column 0 -sticky ew
    $close configure -cursor top_left_arrow

    #
    # The icon button will be used as the basis for the position of the
    # popup on the screen.  We'll always attempt to locate the popup
    # off the lower right corner of the button.  If that would put
    # the popup off the screen, then we'll put above the upper left.
    #
    set rootx [winfo rootx $iconbutton]
    set rooty [winfo rooty $iconbutton]
    set popupwidth [cget -watchwidth]
    set popupheight [expr {[cget -watchheight] + \
			 [winfo reqheight $close]}]

     set popupx [expr {$rootx + 3 + \
                    [winfo width $iconbutton]}]
     set popupy [expr {$rooty + 3 + \
                    [winfo height $iconbutton]}]

     if {(($popupx + $popupwidth) > [winfo screenwidth .]) || \
            (($popupy + $popupheight) > [winfo screenheight .])} {
        set popupx [expr {$rootx - 3 - $popupwidth}]
        set popupy [expr {$rooty - 3 - $popupheight}]
    }
    
    #
    # Get the current time from the timefield widget and both
    # show and select it on the watch.
    #
    $itk_component(watch) show [get]

    #
    # Display the popup at the calculated position.
    #
    wm geometry $popup +$popupx+$popupy
    wm deiconify $popup
    tkwait visibility $popup

    #
    # Perform either a local or global grab based on the -grab option.
    #
    if {$itcl_options(-grab) == "local"} {
	grab $popup
    } else {
	grab -global $popup
    }

    #
    # Make sure the widget is above all others and give it focus.
    #
    raise $popup
    focus $watch
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _popupGetTime
#
# This method is the callback for selection of a time on the 
# watch.  It releases the grab and sets the time in the
# timefield widget.
# ------------------------------------------------------------------
::itcl::body Timeentry::_getPopupTime {} {
    show [$watch get -clicks]
    _releaseGrab 
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _releaseGrab
#
# This method releases the grab, destroys the popup, changes the 
# relief of the button back to raised and reapplies the binding
# to the icon button that engages the popup action.
# ------------------------------------------------------------------
::itcl::body Timeentry::_releaseGrab {} {
    grab release $popup
    $iconbutton configure -relief raised
    destroy $popup 
    bind $iconbutton <Button-1> [itcl::code $this _popup]
}

} ; # end ::itcl::widgets
