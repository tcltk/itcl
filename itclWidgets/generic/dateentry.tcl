#
# Dateentry
# ----------------------------------------------------------------------
# Implements a quicken style date entry field with a popup calendar
# by combining the datefield and calendar widgets together.  This
# allows a user to enter the date via the keyboard or by using the
# mouse by selecting the calendar icon which brings up a popup calendar.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Dateentry
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
# Modified 2001-10-23 by Mark Alston to pass options to the datefield 
# constructor.  Needed to allow use of new option -int which lets the 
# user use dates in YYYY-MM-DD format as well as MM/DD/YYYY format.
#
# option -int yes sets dates to YYYY-MM-DD format
#        -int no sets dates to MM/DD/YYYY format.
#
# ----------------------------------------------------------------------
#   @(#) $Id: dateentry.tcl,v 1.1.2.1 2009/01/10 18:31:32 wiede Exp $
# ======================================================================

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the dateentry class.
# 
proc ::itcl::widgets::dateentry {pathName args} {
    uplevel ::itcl::widgets::Dateentry $pathName $args
}

# ------------------------------------------------------------------
#                              DATEENTRY
# ------------------------------------------------------------------
::itcl::extendedclass Dateentry {
    inherit ::itcl::widgets::Datefield

    component iconbutton
    private component popup
    component calendar

    option [list -grab grab Grab] -default "global" -configuremethod configGrab
    option [list -icon icon Icon] -default {} -configuremethod configIcon
    #
    # The calendar widget isn't created until needed, yet we need
    # its options to be available upon creation of a dateentry widget.
    # So, we'll define them in this class now so they can just be
    # propagated onto the calendar later.
    #
    option [list -days days Days] -default {Su Mo Tu We Th Fr Sa}
    option [list -forwardimage forwardImage Image] -default {}
    option [list -backwardimage backwardImage Image] -default {}
    option [list -weekdaybackground weekdayBackground Background] -default \#d9d9d9
    option [list -weekendbackground weekendBackground Background] -default \#d9d9d9
    option [list -outline outline Outline] -default \#d9d9d9
    option [list -buttonforeground buttonForeground Foreground] -default blue
    option [list -foreground foreground Foreground] -default black
    option [list -selectcolor selectColor Foreground] -default red
    option [list -selectthickness selectThickness SelectThickness] -default 3
    option [list -titlefont titleFont Font] -default \
	-*-helvetica-bold-r-normal--*-140-*
    option [list -dayfont dayFont Font] -default \
	-*-helvetica-medium-r-normal--*-120-*
    option [list -datefont dateFont Font] -default \
	-*-helvetica-medium-r-normal--*-120-*
    option [list -currentdatefont currentDateFont Font] -default \
	-*-helvetica-bold-r-normal--*-120-*
    option [list -startday startDay Day] -default sunday
    option [list -height height Height] -default 165
    option [list -width width Width] -default 200
    option [list -state state State] -default normal

    protected common _defaultIcon ""

    constructor {args} {uplevel 0 Datefield::constructor $args} {}

    protected method _getPopupDate {date}
    protected method _releaseGrab {}
    protected method _releaseGrabCheck {rootx rooty}
    protected method _popup {}
    protected method _getDefaultIcon {}
    protected method configGrab {option value}
    protected method configIcon {option value}
    protected method configState {option value}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Dateentry::constructor {args} {
    #
    # Create an icon label to act as a button to bring up the 
    # calendar popup.
    #
    setupcomponent iconbutton using label $itcl_interior.iconbutton \
            -relief raised
    keepcomponentoption iconbutton -borderwidth -cursor -foreground 
    grid $iconbutton -row 0 -column 0 -sticky ns
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -icon
#
# Specifies the calendar icon image to be used in the date.
# Should one not be provided, then a default pixmap will be used
# if possible, bitmap otherwise.
# ------------------------------------------------------------------
::itcl::body Dateentry::configIcon {option value} {
    if {$value eq {}} {
	$iconbutton configure -image [_getDefaultIcon]
    } else {
	if {[lsearch [image names] $ivalue] == -1} {
	    error "bad icon option \"$itcl_options(-icon)\":\
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
# bringing up the popup calendar.  The default is global.
# ------------------------------------------------------------------
::itcl::body Dateentry::configGrab {option value} {
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
# normal.  A disabled state prevents selection of the date field
# or date icon button.
# ------------------------------------------------------------------
::itcl::body Dateentry::configState {option value} {
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
::itcl::body Dateentry::_getDefaultIcon {} {
    if {[lsearch [image types] pixmap] != -1} {
      set _defaultIcon [image create pixmap -data {
	  /* XPM */
	  static char *calendar[] = {
	  /* width height num_colors chars_per_pixel */
	  "    25    20        6            1",
	  /* colors */
	  ". c #808080",
	  "# c #040404",
	  "a c #848484",
	  "b c #fc0404",
	  "c c #fcfcfc",
	  "d c #c0c0c0",
	  /* pixels */
	  "d##########d###########dd",
	  "d#ccccccccc##ccccccccca#d",
	  "##ccccccccc.#ccccccccc..#",
	  "##cccbbcccca#cccbbbccca.#",
	  "##cccbbcccc.#ccbbbbbcc..#",
	  "##cccbbccc####ccccbbcc..#",
	  "##cccbbcccca#ccccbbbcca.#",
	  "##cccbbcccc.#cccbbbccc..#",
	  "##cccbbcccca#ccbbbcccca.#",
	  "##cccbbbccc.#ccbbbbbcc..#",
	  "##ccccccccc.#ccccccccc..#",
	  "##ccccccccca#ccccccccca.#",
	  "##cc#####c#cd#c#####cc..#",
	  "##cccccccc####cccccccca.#",
	  "##cc#####cc.#cc#####cc..#",
	  "##ccccccccc.#ccccccccc..#",
	  "##ccccccccc.#ccccccccc..#",
	  "##..........#...........#",
	  "###..........#..........#",
	  "#########################"
	 };
	}]
    } else {
	set _defaultIcon [image create bitmap -data {
	    #define calendr2_width 25
	    #define calendr2_height 20
	    static char calendr2_bits[] = {
		0xfe,0xf7,0x7f,0xfe,0x02,0x18,0xc0,0xfe,0x03,
		0x18,0x80,0xff,0x63,0x10,0x47,0xff,0x43,0x98,
		0x8a,0xff,0x63,0x3c,0x4c,0xff,0x43,0x10,0x8a,
		0xff,0x63,0x18,0x47,0xff,0x23,0x90,0x81,0xff,
		0xe3,0x98,0x4e,0xff,0x03,0x10,0x80,0xff,0x03,
		0x10,0x40,0xff,0xf3,0xa5,0x8f,0xff,0x03,0x3c,
		0x40,0xff,0xf3,0x99,0x8f,0xff,0x03,0x10,0x40,
		0xff,0x03,0x18,0x80,0xff,0x57,0x55,0x55,0xff,
		0x57,0xb5,0xaa,0xff,0xff,0xff,0xff,0xff};
        }]
    }

    #
    # Since this image will only need to be created once, we redefine
    # this method to just return the image name for subsequent calls.
    #
    ::itcl::body ::itcl::widgets::Dateentry::_getDefaultIcon {} {
	return $_defaultIcon
    }
    return $_defaultIcon
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _popup
#
# This method is invoked upon selection of the icon button.  It 
# creates a calendar widget within a toplevel popup, calculates 
# the position at which to display the calendar, performs a grab
# and displays the calendar.
# ------------------------------------------------------------------
::itcl::body Dateentry::_popup {} {
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
    setupcomponent popup using toplevel $itcl_interior.popup 
    $popup configure -borderwidth 2 -background black
    wm withdraw $popup
    wm overrideredirect $popup 1
    #
    # Add a binding to button 1 events in order to detect mouse
    # clicks off the calendar in which case we'll release the grab.
    # Also add a binding for Escape to always release.
    #
    bind $popup <1> [itcl::code $this _releaseGrabCheck %X %Y]
    bind $popup <KeyPress-Escape> [itcl::code $this _releaseGrab]
    #
    # Create the calendar widget and set its cursor properly.
    #
    setupcomponent calendar using ::itcl::widgets::Calendar $popup.calendar \
	    -command [itcl::code $this _getPopupDate %d] \
	    -int $itcl_options(-int)
    keepcomponentoption calendar -background -borderwidth -currentdatefont \
        -cursor \
	-datefont -dayfont -foreground -highlightcolor \
	-highlightthickness -labelfont -textbackground -textfont \
	-titlefont -int
	usual
    keepcomponentoption calendar -days -forwardimage -backwardimage \
            -weekdaybackground  -weekendbackground -outline \
	    -buttonforeground -selectcolor \
	    -selectthickness -titlefont -dayfont -datefont \
	    -currentdatefont -startday -width -height
    grid $calendar -row 0 -column 0
    $calendar configure -cursor top_left_arrow
    #
    # The icon button will be used as the basis for the position of the
    # popup on the screen.  We'll always attempt to locate the popup
    # off the lower right corner of the button.  If that would put
    # the popup off the screen, then we'll put above the upper left.
    #
    set rootx [winfo rootx $iconbutton]
    set rooty [winfo rooty $iconbutton]
    set popupwidth [winfo reqwidth $popup]
    set popupheight [winfo reqheight $popup]
    set popupx [expr {$rootx + 3 + [winfo width $iconbutton]}]
    set popupy [expr {$rooty + 3 + [winfo height $iconbutton]}]
    if {(($popupx + $popupwidth) > [winfo screenwidth .]) || \
	    (($popupy + $popupheight) > [winfo screenheight .])} {
	set popupx [expr {$rootx - 3 - $popupwidth}]
	set popupy [expr {$rooty - 3 - $popupheight}]
    }
    #
    # Get the current date from the datefield widget and both
    # show and select it on the calendar.
    #
    # Added catch for bad dates. Calendar then shows current date.
    if [catch "$calendar show [get]" err] {
	$calendar show now
	$calendar select now
    } else {
	$calendar select [get]
    }
    #
    # Display the popup at the calculated position.
    #
    wm geometry $popup +$popupx+$popupy
    wm deiconify $popup
    tkwait visibility $popup
    #
    # Perform either a local or global grab based on the -grab option.
    #
    if {$itcl_options(-grab) eq "local"} {
	::grab $popup
    } else {
	::grab -global $popup
    }
    #
    # Make sure the widget is above all others and give it focus.
    #
    raise $popup
    focus $calendar
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _popupGetDate
#
# This method is the callback for selection of a date on the 
# calendar.  It releases the grab and sets the date in the
# datefield widget.
# ------------------------------------------------------------------
::itcl::body Dateentry::_getPopupDate {date} {
    _releaseGrab 
    show $date
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _releaseGrabCheck rootx rooty
#
# This method handles mouse button 1 events.  If the selection
# occured within the bounds of the calendar, then return normally
# and let the calendar handle the event.  Otherwise, we'll drop
# the calendar and release the grab.
# ------------------------------------------------------------------
::itcl::body Dateentry::_releaseGrabCheck {rootx rooty} {
    set calx [winfo rootx $calendar]
    set caly [winfo rooty $calendar]
    set calwidth [winfo reqwidth $calendar]
    set calheight [winfo reqheight $calendar]
    if {($rootx < $calx) || ($rootx > ($calx + $calwidth)) || \
	    ($rooty < $caly) || ($rooty > ($caly + $calheight))} {
	_releaseGrab
	return -code break
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _releaseGrab
#
# This method releases the grab, destroys the popup, changes the 
# relief of the button back to raised and reapplies the binding
# to the icon button that engages the popup action.
# ------------------------------------------------------------------
::itcl::body Dateentry::_releaseGrab {} {
    ::grab release $popup
    $iconbutton configure -relief raised
    destroy $popup 
    bind $iconbutton <Button-1> [itcl::code $this _popup]
}

} ; # end ::itcl::widgets
