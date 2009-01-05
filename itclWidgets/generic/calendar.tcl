#
# Calendar
# ----------------------------------------------------------------------
# Implements a calendar widget for the selection of a date.  It displays
# a single month at a time.  Buttons exist on the top to change the 
# month in effect turning th pages of a calendar.  As a page is turned, 
# the dates for the month are modified.  Selection of a date visually 
# marks that date.  The selected value can be monitored via the 
# -command option or just retrieved using the get method.  Methods also
# exist to select a date and show a particular month.  The option set
# allows the calendars appearance to take on many forms.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Calendar
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    ACKNOWLEDGEMENTS: Michael McLennan   E-mail: mmclennan@lucent.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: calendar.tcl,v 1.1.2.2 2009/01/05 21:11:13 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Calendar.width 200 widgetDefault
option add *Calendar.height 165 widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Calendar class.
# 
proc ::itcl::widgets::calendar {pathName args} {
    uplevel ::itcl::widgets::Calendar $pathName $args
}

# ------------------------------------------------------------------
#                            CALENDAR
# ------------------------------------------------------------------
::itcl::extendedclass Calendar {
    component itcl_hull
    component itcl_interior
    component page
    component backward
    component forward
    
    option [list -days days Days] -default {Su Mo Tu We Th Fr Sa} -configuremethod configDays
    option [list -command command Command] -default {}
    option [list -forwardimage forwardImage Image] -default {} -configuremethod configForwardimage
    option [list -backwardimage backwardImage Image] -default {} -configuremethod configBackwardimage
    option [list -weekdaybackground weekdayBackground Background] -default \#d9d9d9 -configuremethod configWeekdaybackground
    option [list -weekendbackground weekendBackground Background] -default \#d9d9d9 -configuremethod configWeekendbackground
    option [list -outline outline Outline] -default \#d9d9d9 -configuremethod configOutline
    option [list -buttonforeground buttonForeground Foreground] -default blue -configuremethod configButtonforeground
    option [list -foreground foreground Foreground] -default black
    option [list -selectcolor selectColor Foreground] -default red -configuremethod configSelectcolor
    option [list -selectthickness selectThickness SelectThickness] -default 3 -configuremethod configSelectthickness
    option [list -titlefont titleFont Font] -default \
	-*-helvetica-bold-r-normal--*-140-* -configuremethod configTitlefont
    option [list -dayfont dayFont Font] -default \
	-*-helvetica-medium-r-normal--*-120-* -configuremethod configDayfont
    option [list -datefont dateFont Font] -default \
	-*-helvetica-medium-r-normal--*-120-* -configuremethod configDatefont
    option [list -currentdatefont currentDateFont Font] -default \
	-*-helvetica-bold-r-normal--*-120-* -configuremethod configCurrentdatefont
    option [list -startday startDay Day] -default sunday -configuremethod configStartday
    option [list -int int DateFormat] -default no -configuremethod configInt

    private variable _time {}
    private variable _selected {}
    private variable _initialized 0
    private variable _offset 0
    private variable _format {}

    constructor {args} {}

    private method _change {delta_}
    private method _configureHandler {}
    private method _redraw {}
    private method _days {{wmax {}}}
    private method _layout {time_}
    private method _select {date_}
    private method _selectEvent {date_}
    private method _adjustday {day_}
    private method _percentSubst {pattern_ string_ subst_}

    protected method _drawtext {canvas_ day_ date_ now_ x0_ y0_ x1_ y1_} 
    protected method configInt {option value}
    protected method configDays {option value}
    protected method configForwardimage {option value}
    protected method configBackwardimage {option value}
    protected method configWeekdaybackground {option value}
    protected method configWeekendbackground {option value}
    protected method configForeground {option value}
    protected method configOutline {option value}
    protected method configButtonforeground {option value}
    protected method configSelectcolor {option value}
    protected method configSelectthickness {option value}
    protected method configTitlefont {option value}
    protected method configDatefont {option value}
    protected method configCurrentdatefont {option value}
    protected method configDayfont {option value}
    protected method configStartday {option value}

    public method get {{format "-string"}} ;# Returns the selected date
    public method select {{date_ "now"}}   ;# Selects date, moving select ring
    public method show {{date_ "now"}}     ;# Displays a specific date
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Calendar::constructor {args} {
    set win [createhull frame $this -class [info class] -borderwidth 0]
    set itcl_interior $win
    #
    # Create the canvas which displays each page of the calendar.
    #
    setupcomponent page using canvas $itcl_interior.page
    keepcomponentoption page -background -cursor -width -height
    pack $page -expand yes -fill both
    
    #
    # Create the forward and backward buttons.  Rather than pack
    # them directly in the hull, we'll waittill later and make
    # them canvas window items.
    #
    setupcomponent backward using button $page.backward \
		-command [itcl::code $this _change -1]
    keepcomponentoption backward -background -cursor 

    setupcomponent forward using button $page.forward \
		-command [itcl::code $this _change +1]
    keepcomponentoption forward -background -cursor 

    #
    # Set the initial time to now.
    #
    set _time [clock seconds]

    # 
    # Bind to the configure event which will be used to redraw
    # the calendar and display the month.
    #
    bind $page <Configure> [itcl::code $this _configureHandler]
    
    #
    # Evaluate the option arguments.
    #
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------
# ------------------------------------------------------------------
# OPTION: -int
#
# Added by Mark Alston 2001/10/21
#
# Allows for the use of dates in "international" format: YYYY-MM-DD.
# It must be a boolean value.
# ------------------------------------------------------------------
::itcl::body Calendar::configInt {option value} { 
    switch $value {
    1 - yes - true - on {
         set itcl_options($option) yes
      }
    0 - no - false - off {
        set itcl_options($option) no
      }
    default {
        error "bad int option \"$value\": should be boolean"
     }
   }
}

# ------------------------------------------------------------------
# OPTION: -days
#
# The days option takes a list of values to set the text used to display the 
# days of the week header above the dates.  The default value is 
# {Su Mo Tu We Th Fr Sa}.
# ------------------------------------------------------------------
::itcl::body Calendar::configDays {option value} {
    set itcl_options($option) $value
    if {$_initialized} {
	if {[$page find withtag days] ne {}} {
	    $page delete days
	    _days
	}
    }
}

# ------------------------------------------------------------------
# OPTION: -backwardimage
#
# Specifies a image to be displayed on the backwards calendar 
# button.  If none is specified, a default is provided.
# ------------------------------------------------------------------
::itcl::body Calendar::configBackwardimage {option value} {

    #
    # If no image is given, then we'll use the default image.
    #
    if {$value eq {}} {
	#
	# If the default image hasn't yet been created, then we
	# need to create it.
	#
	if {[lsearch [image names] $this-backward] == -1} {
	    image create bitmap $this-backward \
		    -foreground $itcl_options(-buttonforeground) -data {
		#define back_width 16
		#define back_height 16
		static unsigned char back_bits[] = {
		    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x30, 
		    0xe0, 0x38, 0xf0, 0x3c, 0xf8, 0x3e, 0xfc, 0x3f, 
		    0xfc, 0x3f, 0xf8, 0x3e, 0xf0, 0x3c, 0xe0, 0x38,
		    0xc0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		}
	}

	#
	# Configure the button to use the default image.
	#
	$backward configure -image $this-backward
	
    #
    # Else, an image has been specified.  First, we'll need to make sure
    # the image really exists before configuring the button to use it.  
    # If it doesn't generate an error.
    #
    } else {
	if {[lsearch [image names] $value] != -1} {
	    $backward configure \
		    -image $value
	} else {
	    error "bad image name \"$value\":\
		    image does not exist"
	}

	#
	# If we previously created a default image, we'll just remove it.
	#
	if {[lsearch [image names] $this-backward] != -1} {
	    image delete $this-backward
	}
    }
    set itcl_options($option) $value
}


# ------------------------------------------------------------------
# OPTION: -forwardimage
#
# Specifies a image to be displayed on the forwards calendar 
# button.  If none is specified, a default is provided.
# ------------------------------------------------------------------
::itcl::body Calendar::configForwardimage {option value} {

    #
    # If no image is given, then we'll use the default image.
    #
    if {$value eq {}} {

	#
	# If the default image hasn't yet been created, then we
	# need to create it.
	#
	if {[lsearch [image names] $this-forward] == -1} {
	    image create bitmap $this-forward \
		    -foreground $itcl_options(-buttonforeground) -data {
		#define fwd_width 16
		#define fwd_height 16
		static unsigned char fwd_bits[] = {
		    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x03, 
		    0x1c, 0x07, 0x3c, 0x0f, 0x7c, 0x1f, 0xfc, 0x3f, 
		    0xfc, 0x3f, 0x7c, 0x1f, 0x3c, 0x0f, 0x1c, 0x07,
		    0x0c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		}
	}

	#
	# Configure the button to use the default image.
	#
	$forward configure -image $this-forward
	
    #
    # Else, an image has been specified.  First, we'll need to make sure
    # the image really exists before configuring the button to use it.  
    # If it doesn't generate an error.
    #
    } else {
	if {[lsearch [image names] $value] != -1} {
	    $forward configure \
		    -image $value
	} else {
	    error "bad image name \"$value\":\
		    image does not exist"
	}

	#
	# If we previously created a default image, we'll just remove it.
	#
	if {[lsearch [image names] $this-forward] != -1} {
	    image delete $this-forward
	}
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -weekdaybackground
#
# Specifies the background for the weekdays which allows it to
# be visually distinguished from the weekend.
# ------------------------------------------------------------------
::itcl::body Calendar::configWeekdaybackground {option value} {
    if {$_initialized} {
	$page itemconfigure weekday \
		-fill $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -weekendbackground
#
# Specifies the background for the weekdays which allows it to
# be visually distinguished from the weekdays.
# ------------------------------------------------------------------
::itcl::body Calendar::configWeekendbackground {option value} {
    if {$_initialized} {
	$page itemconfigure weekend \
		-fill $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -foreground
#
# Specifies the foreground color for the textual items, buttons,
# and divider on the calendar.
# ------------------------------------------------------------------
::itcl::body Calendar::configForeground {option value} {
    if {$_initialized} {
	$page itemconfigure text \
		-fill $value
	$page itemconfigure line \
		-fill $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -outline
#
# Specifies the outline color used to surround the date text.
# ------------------------------------------------------------------
::itcl::body Calendar::configOutline {option value} {
    if {$_initialized} {
	$page itemconfigure square \
		-outline $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -buttonforeground
#
# Specifies the foreground color of the forward and backward buttons.
# ------------------------------------------------------------------
::itcl::body Calendar::configButtonforeground {option value} {
    if {$_initialized} {
	if {$itcl_options(-forwardimage) == {}} {
	    if {[lsearch [image names] $this-forward] != -1} {
		$this-forward configure \
		    -foreground $value
	    }
	} else {
	    $forward configure \
		    -foreground $value
	}
	
	if {$itcl_options(-backwardimage) == {}} {
	    if {[lsearch [image names] $this-backward] != -1} {
		$this-backward configure \
		    -foreground $value
	    }
	} else {
	    $backward configure \
		    -foreground $value
	}
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -selectcolor
#
# Specifies the color of the ring displayed that distinguishes the 
# currently selected date.  
# ------------------------------------------------------------------
::itcl::body Calendar::configSelectcolor {option value} {
    if {$_initialized} {
	$page itemconfigure $_selected-sensor \
		-outline $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -selectthickness
#
# Specifies the thickness of the ring displayed that distinguishes 
# the currently selected date.  
# ------------------------------------------------------------------
::itcl::body Calendar::configSelectthickness {option value} {
    if {$_initialized} {
	$page itemconfigure $_selected-sensor \
		-width $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -titlefont
#
# Specifies the font used for the title text that consists of the 
# month and year.
# ------------------------------------------------------------------
::itcl::body Calendar::configTitlefont {option value} {
    if {$_initialized} {
	$page itemconfigure title \
		-font $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -datefont
#
# Specifies the font used for the date text that consists of the 
# day of the month.
# ------------------------------------------------------------------
::itcl::body Calendar::configDatefont {option value} {
    if {$_initialized} {
	$page itemconfigure date \
		-font $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -currentdatefont
#
# Specifies the font used for the current date text.
# ------------------------------------------------------------------
::itcl::body Calendar::configCurrentdatefont {option value} {
    if {$_initialized} {
	$page itemconfigure now \
		-font $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -dayfont
#
# Specifies the font used for the day of the week text.
# ------------------------------------------------------------------
::itcl::body Calendar::configDayfont {option value} {
    if {$_initialized} {
	$page itemconfigure days \
		-font $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -startday
#
# Specifies the starting day for the week.  The value must be a day of the
# week: sunday, monday, tuesday, wednesday, thursday, friday, or
# saturday.  The default is sunday.
# ------------------------------------------------------------------
::itcl::body Calendar::configStartday {option value} {
    set day [string tolower $value]
    switch $day {
    sunday {
        set _offset 0
      }
    monday {
        set _offset 1
      }
    tuesday {
        set _offset 2
      }
    wednesday {
        set _offset 3
      }
    thursday {
        set _offset 4
      }
    friday {
        set _offset 5
      }
    saturday {
        set _offset 6
      }
    default {
        error "bad startday option \"$value\":\
                   should be sunday, monday, tuesday, wednesday,\
                   thursday, friday, or saturday"
      }
    }
    set itcl_options($option) $value

    if {$_initialized} {
	$page delete all-page
	_redraw
    }
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# PUBLIC METHOD: get ?format?
#
# Returns the currently selected date in one of two formats, string 
# or as an integer clock value using the -string and -clicks
# options respectively.  The default is by string.  Reference the 
# clock command for more information on obtaining dates and their 
# formats.
# ------------------------------------------------------------------
::itcl::body Calendar::get {{format "-string"}} {
    switch -- $format {
    "-string" {
        return $_selected
      }
    "-clicks" {
        return [clock scan $_selected]
      }
    default {
        error "bad format option \"$format\":\
                  should be -string or -clicks"
      }
    }
}

# ------------------------------------------------------------------
# PUBLIC METHOD: select date_
#
# Changes the currently selected date to the value specified.
# ------------------------------------------------------------------
::itcl::body Calendar::select {{date_ "now"}} {
    if {$date_ eq "now"} {
	set time [clock seconds]
    } else {
	if {[catch {clock format $date_}] == 0} {
	    set time $date_
	} elseif {[catch {set time [clock scan $date_]}] != 0} {
	    error "bad date: \"$date_\", must be a valid date string, clock clicks value or the keyword now"
	}
    }
    switch $itcl_options(-int) {
    yes {
        set _format "%Y-%m-%d"
      }
    no {
        set _format "%m/%d/%Y"
       }
    }
    _select [clock format $time -format "$_format"]
}

# ------------------------------------------------------------------
# PUBLIC METHOD: show date_
#
# Changes the currently display month to be that of the specified 
# date.
# ------------------------------------------------------------------
::itcl::body Calendar::show {{date_ "now"}} {
    if {$date_ == "now"} {
	set _time [clock seconds]
    } else {
	if {[catch {clock format $date_}] == 0} {
	    set _time $date_
	} elseif {[catch {set _time [clock scan $date_]}] != 0} {
	    error "bad date: \"$date_\", must be a valid date string, clock clicks value or the keyword now"
	}
    }
    $page delete all-page
    _redraw
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _drawtext canvas_ day_ date_ now_
#                             x0_ y0_ x1_ y1_
#
# Draws the text in the date square.  The method is protected such that
# it can be overridden in derived classes that may wish to add their
# own unique text.  The method receives the day to draw along with
# the coordinates of the square.
# ------------------------------------------------------------------
::itcl::body Calendar::_drawtext {canvas_ day_ date_ now_ x0_ y0_ x1_ y1_} {
    set item [$canvas_ create text \
		  [expr {(($x1_ - $x0_) / 2) + $x0_}] \
		  [expr {(($y1_ -$y0_) / 2) + $y0_ + 1}] \
		  -anchor center -text "$day_" \
		  -fill $itcl_options(-foreground)]

    if {$date_ == $now_} {
	$canvas_ itemconfigure $item \
	    -font $itcl_options(-currentdatefont) \
	    -tags [list all-page date $date_-date text now]
    } else {
	$canvas_ itemconfigure $item \
	    -font $itcl_options(-datefont) \
	    -tags [list all-page date $date_-date text]
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _configureHandler
#
# Processes a configure event received on the canvas.  The method
# deletes all the current canvas items and forces a redraw.
# ------------------------------------------------------------------
::itcl::body Calendar::_configureHandler {} {
    set _initialized 1
    $page delete all
    _redraw
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _change delta_
#
# Changes the current month displayed in the calendar, moving
# forward or backward by <delta_> months where <delta_> is +/-
# some number.
# ------------------------------------------------------------------
::itcl::body Calendar::_change {delta_} {
    set dir [expr {($delta_ > 0) ? 1 : -1}]
    set month [clock format $_time -format "%m"]
    set month [string trimleft $month 0]
    set year [clock format $_time -format "%Y"]

    for {set i 0} {$i < abs($delta_)} {incr i} {
        incr month $dir
        if {$month < 1} {
            set month 12
            incr year -1
        } elseif {$month > 12} {
            set month 1
            incr year 1
        }
    }
    if {[catch {set _time [clock scan "$month/1/$year"]}]} {
	bell
    } else {
	_redraw 
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _redraw
#
# Redraws the calendar.  This method is invoked whenever the 
# calendar changes size or we need to effect a change such as draw
# it with a new month.
# ------------------------------------------------------------------
::itcl::body Calendar::_redraw {} {
    #
    # Set the format based on the option -int
    #
    switch $itcl_options(-int) {
    yes {
        set _format "%Y-%m-%d"
      }
    no {
        set _format "%m/%d/%Y"
       }
    }
    #
    # Remove all the items that typically change per redraw request
    # such as the title and dates.  Also, get the maximum width and
    # height of the page.
    #
    $page delete all-page

    set wmax [winfo width $page]
    set hmax [winfo height $page]

    #
    # If we haven't yet created the forward and backwards buttons,
    # then dot it; otherwise, skip it.
    #
    if {[$page find withtag button] == {}} {
	$page create window 3 3 -anchor nw \
		-window $backward -tags button
	$page create window [expr {$wmax-3}] 3 -anchor ne \
		-window $forward -tags button
    }

    #
    # Create the title centered between the buttons.
    #
    foreach {x0 y0 x1 y1} [$page bbox button] {
	set x [expr {(($x1 - $x0) / 2) + $x0}]
	set y [expr {(($y1 - $y0) / 2) + $y0}]
    }

    set title [clock format $_time -format "%B %Y"]
    $page create text $x $y -anchor center \
        -text $title -font $itcl_options(-titlefont) \
	-fill $itcl_options(-foreground) \
	-tags [list title text all-page]

    #
    # Add the days of the week labels if they haven't yet been created.
    #
    if {[$page find withtag days] == {}} {
	_days $wmax
    }

    #
    # Add a line between the calendar header and the dates if needed.
    #
    set bottom [expr {[lindex [$page bbox all] 3] + 3}]

    if {[$page find withtag line] == {}} {
	$page create line 0 $bottom $wmax $bottom \
		-width 2 -tags line
    }
    incr bottom 3
    #
    # Get the layout for the time value and create the date squares.
    # This includes the surrounding date rectangle, the date text,
    # and the sensor.  Bind selection to the sensor.
    #
    set current ""
    set now [clock format [clock seconds] -format "$_format"]

    set layout [_layout $_time]
    set weeks [expr {[lindex $layout end] + 1}]

    foreach {day date kind dcol wrow} $layout {
        set x0 [expr {$dcol * ($wmax - 7) / 7 + 3}]
        set y0 [expr {$wrow * ($hmax - $bottom - 4) / $weeks + $bottom}]
        set x1 [expr {($dcol + 1) * ($wmax - 7) / 7 + 3}]
        set y1 [expr {($wrow + 1) * ($hmax - $bottom - 4) / $weeks + $bottom}]

        if {$date == $_selected} {
            set current $date
        }

	#
	# Create the rectangle that surrounds the date and configure
	# its background based on the wheather it is a weekday or
	# a weekend.
	#
	set item [$page create rectangle $x0 $y0 $x1 $y1 \
		-outline $itcl_options(-outline)]

	if {$kind eq "weekend"} {
	    $page itemconfigure $item \
		    -fill $itcl_options(-weekendbackground) \
		    -tags [list all-page square weekend]
	} else {
	    $page itemconfigure $item \
		    -fill $itcl_options(-weekdaybackground) \
		    -tags [list all-page square weekday]
	}

	#
	# Create the date text and configure its font based on the 
	# wheather or not it is the current date.
	#
	_drawtext $page $day $date $now $x0 $y0 $x1 $y1

	#
	# Create a sensor area to detect selections.  Bind the 
	# sensor and pass the date to the bind script.
	#
        $page create rectangle $x0 $y0 $x1 $y1 \
            -outline "" -fill "" \
            -tags [list $date-sensor all-sensor all-page]

        $page bind $date-sensor <ButtonPress-1> \
            [itcl::code $this _selectEvent $date]

        $page bind $date-date <ButtonPress-1> \
            [itcl::code $this _selectEvent $date]
    }

    #
    # Highlight the selected date if it is on this page.
    #
    if {$current ne ""} {
        $page itemconfigure $current-sensor \
            -outline $itcl_options(-selectcolor) \
	    -width $itcl_options(-selectthickness)

        $page raise $current-sensor

    } elseif {$_selected == ""} {
        set date [clock format $_time -format "$_format"]
        _select $date
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _days
#
# Used to rewite the days of the week label just below the month 
# title string.  The days are given in the -days option.
# ------------------------------------------------------------------
::itcl::body Calendar::_days {{wmax {}}} {
    if {$wmax == {}} {
	set wmax [winfo width $page]
    }

    set col 0
    set bottom [expr {[lindex [$page bbox title buttons] 3] + 7}]

    foreach dayoweek $itcl_options(-days) {
	set x0 [expr {$col * ($wmax / 7)}]
	set x1 [expr {($col + 1) * ($wmax / 7)}]

	$page create text \
	    [expr {(($x1 - $x0) / 2) + $x0}] $bottom \
	    -anchor n -text "$dayoweek" \
	    -fill $itcl_options(-foreground) \
	    -font $itcl_options(-dayfont) \
	    -tags [list days text]

	incr col
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _layout time_
#
# Used whenever the calendar is redrawn.  Finds the month containing
# a <time_> in seconds, and returns a list for all of the days in 
# that month.  The list looks like this:
#
#    {day1 date1 kind1 c1 r1  day2 date2 kind2 c2 r2  ...}
#
# where dayN is a day number like 1,2,3,..., dateN is the date for
# dayN, kindN is the day type of weekday or weekend, and cN,rN 
# are the column/row indices for the square containing that date.
# ------------------------------------------------------------------
::itcl::body Calendar::_layout {time_} {

    switch $itcl_options(-int) {
    yes {
        set _format "%Y-%m-%d"
      }
    no {
        set _format "%m/%d/%Y"
      }
    }

    set month [clock format $time_ -format "%m"]
    set year  [clock format $time_ -format "%Y"]

    if {[info tclversion] >= 8.5} {
	set startOfMonth [clock scan "$year-$month-01" -format %Y-%m-%d]
	set lastday [clock format [clock add $startOfMonth 1 month -1 day] -format %d]
    } else {
	foreach lastday {31 30 29 28} {
	    if {[catch {clock scan "$month/$lastday/$year"}] == 0} {
		break
	    }
	}
    }
    set seconds [clock scan "$month/1/$year"]
    set firstday [_adjustday [clock format $seconds -format %w]]

    set weeks [expr {ceil(double($lastday+$firstday)/7)}]

    set rlist ""
    for {set day 1} {$day <= $lastday} {incr day} {
        set seconds [clock scan "$month/$day/$year"]
        set date [clock format $seconds -format "$_format"]
	set dayoweek [clock format $seconds -format %w]

	if {$dayoweek == 0 || $dayoweek == 6} {
	    set kind "weekend"
	} else {
	    set kind "weekday"
	}

        set daycol [_adjustday $dayoweek]

        set weekrow [expr {($firstday+$day-1)/7}]
        lappend rlist $day $date $kind $daycol $weekrow 
    }
    return $rlist
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _adjustday day_
#
# Modifies the day to be in accordance with the startday option.
# ------------------------------------------------------------------
::itcl::body Calendar::_adjustday {day_} {
    set retday [expr {$day_ - $_offset}]

    if {$retday < 0} {
	set retday [expr {$retday + 7}]
    }

    return $retday
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _select date_
#
# Selects the current <date_> on the calendar.  Highlights the date 
# on the calendar, and executes the command associated with the 
# calendar, with the selected date substituted in place of "%d".
# ------------------------------------------------------------------
::itcl::body Calendar::_select {date_} {

    switch $itcl_options(-int) {
	yes { set _format "%Y-%m-%d" }
	no { set _format "%m/%d/%Y" }
    }


    set time [clock scan $date_]
    set date [clock format $time -format "$_format"]

    set _selected $date
    set current [clock format $_time -format "%m %Y"]
    set selected [clock format $time -format "%m %Y"]

    if {$current == $selected} {
        $page itemconfigure all-sensor \
            -outline "" -width 1

        $page itemconfigure $date-sensor \
            -outline $itcl_options(-selectcolor) \
	    -width $itcl_options(-selectthickness)
        $page raise $date-sensor
    } else {
        set _time $time
        _redraw 
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _selectEvent date_
#
# Selects the current <date_> on the calendar.  Highlights the date 
# on the calendar, and executes the command associated with the 
# calendar, with the selected date substituted in place of "%d".
# ------------------------------------------------------------------
::itcl::body Calendar::_selectEvent {date_} {
    _select $date_

    if {[string trim $itcl_options(-command)] != ""} {
        set cmd $itcl_options(-command)
        set cmd [_percentSubst %d $cmd [get]]
        uplevel #0 $cmd
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _percentSubst pattern_ string_ subst_
#
# This command is a "safe" version of regsub, for substituting
# each occurance of <%pattern_> in <string_> with <subst_>.  The
# usual Tcl "regsub" command does the same thing, but also
# converts characters like "&" and "\0", "\1", etc. that may
# be present in the <subst_> string.
#
# Returns <string_> with <subst_> substituted in place of each
# <%pattern_>.
# ------------------------------------------------------------------
::itcl::body Calendar::_percentSubst {pattern_ string_ subst_} {
    if {![string match %* $pattern_]} {
        error "bad pattern \"$pattern_\": should be %something"
    }

    set rval ""
    while {[regexp "(.*)${pattern_}(.*)" $string_ all head tail]} {
        set rval "$subst_$tail$rval"
        set string_ $head
    }
    set rval "$string_$rval"
}


} ; # end ::itcl::widgets
