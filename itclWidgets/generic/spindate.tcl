#
# Spindate
# ----------------------------------------------------------------------
# Implements a Date spinner widget.  A date spinner contains three
# Spinner widgets:  one Spinner for months, one SpinInt for days,
# and one SpinInt for years.  Months can be specified as abbreviated
# strings, integers or a user-defined list.  Options exist to manage to 
# behavior, appearance, and format of each component spinner.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Spindate
# written by:
#    Sue Yockey               E-mail: yockey@acm.org
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: spindate.tcl,v 1.1.2.1 2009/01/09 17:52:37 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Spindate.monthLabel "Month" widgetDefault
option add *Spindate.dayLabel "Day" widgetDefault
option add *Spindate.yearLabel "Year" widgetDefault
option add *Spindate.monthWidth 4 widgetDefault
option add *Spindate.dayWidth 4 widgetDefault
option add *Spindate.yearWidth 4 widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Spindate class.
# 
proc ::itcl::widgets::spindate {pathName args} {
    uplevel ::itcl::widgets::Spindate $pathName $args
}

# ------------------------------------------------------------------
#                            SPINDATE
# ------------------------------------------------------------------
::itcl::extendedclass Spindate {
    component itcl_hull
    component itcl_interior
    component year
    component month
    component day
    
    option [list -labelpos labelPos Position] -default w -configuremethod configLabelpos
    option [list -orient orient Orient] -default vertical  -configuremethod configOrient
    option [list -monthon monthOn MonthOn] -default true  -configuremethod configMonthon
    option [list -dayon dayOn DayOn] -default true  -configuremethod configDayon
    option [list -yearon yearOn YearOn] -default true  -configuremethod configYearon
    option [list -datemargin dateMargin Margin] -default 1  -configuremethod configDatemargin
    option [list -yeardigits yearDigits YearDigits] -default 4 -configuremethod configYeardigits
    option [list -monthformat monthFormat MonthFormat] -default integer  -configuremethod configMonthformat

    delegate option [list -monthlabel monthLabel Text] to month as -labeltext
    delegate option [list -monthwidth monthWidth Width] to month as -width
    delegate option [list -daylabel dayLabel Text] to day as -labeltext
    delegate option [list -daywidth dayWidth Width] to day as -width
    delegate option [list -yearlabel yearLabel Text] to year as -labeltext
    delegate option [list -yearwidth yearWidth Width] to year as -width

    private variable _monthFormatStr "%m"
    private variable _yearFormatStr "%Y"
    private variable _interior

    protected variable _repack {}             ;# Reconfiguration flag.

    constructor {args} {}
    destructor {}
    
    private method _lastDay {month year}
    private method _spinMonth {direction}
    private method _spinDay {direction}

    protected method _packDate {{when later}}
    protected method configLabelpos {option value}
    protected method configOrient {option value}
    protected method configMonthon {option value}
    protected method configDayon {option value}
    protected method configYearon {option value}
    protected method configDatemargin {option value}
    protected method configYeardigits {option value}
    protected method configMonthformat {option value}

    public method get {{format "-string"}} 
    public method show {{date now}} 
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Spindate::constructor {args} {
    createhull frame $this -class [info class]
    set _interior $itcl_interior
    set clicks [clock seconds]
    #
    # Create Month Spinner
    #
    setupcomponent month using ::itcl::widgets::Spinner $itcl_interior.month \
            -fixed 2 -justify right \
	    -decrement [itcl::code $this _spinMonth -1] \
	    -increment [itcl::code $this _spinMonth 1] 
    keepcomponentoption month -background -cursor -arroworient -foreground \
		-labelfont -labelmargin -relief -textbackground \
		-textfont -repeatdelay -repeatinterval
    #
    # Take off the default bindings for selction and motion.
    #
    bind [$month component entry] <1> {break}
    bind [$month component entry] <Button1-Motion> {break}
    #
    # Create Day Spinner
    #
    setupcomponent day using ::itcl::widgets::Spinint $itcl_interior.day \
            -fixed 2 -justify right \
	    -decrement [itcl::code $this _spinDay -1] \
	    -increment [itcl::code $this _spinDay 1]
    keepcomponentoption day -background -cursor -arroworient -foreground \
		-labelfont -labelmargin -relief -textbackground \
		-textfont -repeatdelay -repeatinterval
    #
    # Take off the default bindings for selction and motion.
    #
    bind [$day component entry] <1> {break}
    bind [$day component entry] <Button1-Motion> {break}
    #
    # Create Year Spinner
    #
    setupcomponent year using ::itcl::widgets::Spinint $itcl_interior.year \
            -fixed 2 -justify right \
	    -range {1900 3000}
    keepcomponentoption year -background -cursor -arroworient -foreground \
		-labelfont -labelmargin -relief -textbackground \
		-textfont -repeatdelay -repeatinterval
    #
    # Take off the default bindings for selction and motion.
    #
    bind [$year component entry] <1> {break}
    bind [$year component entry] <Button1-Motion> {break}
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
    #
    # Show the current date.
    #
    show now
}

# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Spindate::destructor {} {
    if {$_repack ne ""} {
        after cancel $_repack
     }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -labelpos
#
# Specifies the location of all 3 spinners' labels.
# ------------------------------------------------------------------
::itcl::body Spindate::configLabelpos {option value} {
    switch $value {
    n {
        $month configure -labelpos n
        $day configure -labelpos n
        $year configure -labelpos n
        #
        # Un-align labels
        #
        $month configure -labelmargin 1
        $day configure -labelmargin 1
        $year configure -labelmargin 1 
      }
    s {
        $month configure -labelpos s
        $day configure -labelpos s
        $year configure -labelpos s
        
        #
        # Un-align labels
        #
        $month configure -labelmargin 1
        $day configure -labelmargin 1
        $year configure -labelmargin 1 
      }
    w {
        $month configure -labelpos w
        $day configure -labelpos w
        $year configure -labelpos w
      }
    e {
        $month configure -labelpos e
        $day configure -labelpos e
        $year configure -labelpos e
        #
        # Un-align labels
        #
        $month configure -labelmargin 1
        $day configure -labelmargin 1
        $year configure -labelmargin 1 
      }
    default {
        error "bad labelpos option \"$value\",\
	        should be n, s, w or e" 
      }
    }
    set itcl_options($option) $value
    _packDate
}

# ------------------------------------------------------------------
# OPTION: -orient
# 
# Specifies the orientation of the 3 spinners for Month, Day 
# and year.
# ------------------------------------------------------------------
::itcl::body Spindate::configOrient {option value} {
    set itcl_options($option) $value
    _packDate
}

# ------------------------------------------------------------------
# OPTION: -monthon
# 
# Specifies whether or not to display the month spinner.
# ------------------------------------------------------------------
::itcl::body Spindate::configMonthon {option value} {
    set itcl_options($option) $value
    _packDate
}

# ------------------------------------------------------------------
# OPTION: -dayon
# 
# Specifies whether or not to display the day spinner.
# ------------------------------------------------------------------
::itcl::body Spindate::configDayon {option value} {
    set itcl_options($option) $value
    _packDate
}

# ------------------------------------------------------------------
# OPTION: -yearon
# 
# Specifies whether or not to display the year spinner.
# ------------------------------------------------------------------
::itcl::body Spindate::configYearon {option value} {
    set itcl_options($option) $value
    _packDate
}

# ------------------------------------------------------------------
# OPTION: -datemargin
# 
# Specifies the margin space between the month and day spinners 
# and the day and year spinners. 
# ------------------------------------------------------------------
::itcl::body Spindate::configDatemargin {option value} {
    set itcl_options($option) $value
    _packDate
}

# ------------------------------------------------------------------
# OPTION: -yeardigits
#
# Number of digits for year display, 2 or 4 
# ------------------------------------------------------------------
::itcl::body Spindate::configYeardigits {option value} {
    set clicks [clock seconds]
    switch $value {
    "2" {
        $year configure -width 2 -fixed 2
        $year clear
        $year insert 0 [clock format $clicks -format "%y"]
        set _yearFormatStr "%y"
      }
    "4" {
        $year configure -width 4 -fixed 4
        $year clear
        $year insert 0 [clock format $clicks -format "%Y"]
        set _yearFormatStr "%Y"
      }
    default {
        error "bad yeardigits option \"$value\",\
	    should be 2 or 4"
      }
    } 
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -monthformat
#
# Format of month display, integers (1-12) or brief strings (Jan - 
# Dec), or full strings (January - December).
# ------------------------------------------------------------------
::itcl::body Spindate::configMonthformat {option value} {
    set clicks [clock seconds]
    if {$value eq "brief"} {
	$month configure -width 3 -fixed 3
	$month delete 0 end
	$month insert 0 [clock format $clicks -format "%b"]
	set _monthFormatStr "%b"
    } elseif {$value eq "full"} {
	$month configure -width 9 -fixed 9
	$month delete 0 end
	$month insert 0 [clock format $clicks -format "%B"]
	set _monthFormatStr "%B"
    } elseif {$value eq "integer"} {
	$month configure -width 2 -fixed 2 
	$month delete 0 end
	$month insert 0 [clock format $clicks -format "%m"]
	set _monthFormatStr "%m"
    } else {
	error "bad monthformat option\
		\"$value\", should be\
		\"integer\", \"brief\" or \"full\""
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: get ?format?
#
# Return the current contents of the spindate widget in one of 
# two formats string or as an integer clock value using the -string 
# and -clicks options respectively.  The default is by string.  
# Reference the clock command for more information on obtaining dates 
# and their formats.
# ------------------------------------------------------------------
::itcl::body Spindate::get {{format "-string"}} { 
    set month [$month get]
    set day [$day get]
    set year [$year get]
    if {[regexp {[0-9]+} $month]} {
	set datestr "$month/$day/$year"
    } else {
	set datestr "$day $month $year"
    }
    switch -- $format {
    "-string" {
        return $datestr
      }
    "-clicks" {
        return [clock scan $datestr]
      }
    default {
        error "bad format option \"$format\":\
               should be -string or -clicks"
      }
    }
}

# ------------------------------------------------------------------
# PUBLIC METHOD: show date
#
# Changes the currently displayed date to be that of the date 
# argument.  The date may be specified either as a string or an
# integer clock value.  Reference the clock command for more 
# information on obtaining dates and their formats.
# ------------------------------------------------------------------
::itcl::body Spindate::show {{date "now"}} {
    #
    # Convert the date to a clock clicks value.
    #
    if {$date eq "now"} {
	set seconds [clock seconds]
    } else {
	if {[catch {clock format $date}] == 0} {
	    set seconds $date
	} elseif {[catch {set seconds [clock scan $date]}] != 0} {
	    error "bad date: \"$date\", must be a valid date\
               string, clock clicks value or the keyword now"
	}
    }
    #
    # Display the month based on the -monthformat option.
    #
    switch $itcl_options(-monthformat) {
    "brief" {
        $month delete 0 end
        $month insert 0 [clock format $seconds -format "%b"]
      }	
    "full" {
        $month delete 0 end
        $month insert 0 [clock format $seconds -format "%B"]
      }	
    "integer" {
        $month delete 0 end
        $month insert 0 [clock format $seconds -format "%m"]
      }
    }
    #
    # Display the day.
    #
    $day delete 0 end
    $day insert end [clock format $seconds -format "%d"]
    #
    # Display the year based on the -yeardigits option.
    #
    switch $itcl_options(-yeardigits) {
    "2" {
        $year delete 0 end
        $year insert 0 [clock format $seconds -format "%y"]
      }
    "4" {
        $year delete 0 end
        $year insert 0 [clock format $seconds -format "%Y"]
      }
    }
    return
}

# ----------------------------------------------------------------
# PRIVATE METHOD: _spinMonth direction
#
# Increment or decrement month value.  We need to get the values
# for all three fields so we can make sure the day agrees with
# the month.  Should the current day be greater than the day for
# the spun month, then the day is set to the last day for the
# new month.
# ----------------------------------------------------------------
::itcl::body Spindate::_spinMonth {direction} {
    set month [$month get]
    set day [$day get]
    set year [$year get]
    #
    # There appears to be a bug in the Tcl clock command in that it
    # can't scan a date like "12/31/1999 1 month" or any other date with
    # a year above 2000, but it has no problem scanning "07/01/1998 1 month".
    # So, we're going to play a game and increment by days until this
    # is fixed in Tcl.
    #
    if {$direction == 1} {
	set incrdays 32
	set day 01
    } else {
	set incrdays -28
	set day 28
    }
    if {[regexp {[0-9]+} $month]} {
	set clicks [clock scan "$month/$day/$year $incrdays day"]
    } else {
	set clicks [clock scan "$day $month $year $incrdays day"]
    }
    $month clear
    $month insert 0 \
	[clock format $clicks -format $_monthFormatStr]
    set currday [$itk_component(day) get]
    set lastday [_lastDay [$itk_component(month) get] $year]
    if {$currday > $lastday} {
	$day clear
	$day insert end $lastday
    }
}

# ----------------------------------------------------------------
# PRIVATE METHOD: _spinDay direction
#
# Increment or decrement day value.  If the previous day was the
# first, then set the new day to the last day for the current
# month.  If it was the last day of the month, change it to the
# first.  Otherwise, spin it to the next day.
# ----------------------------------------------------------------
::itcl::body Spindate::_spinDay {direction} {
    set month [$month get]
    set day [$day get]
    set year [$year get]
    set lastday [_lastDay $month $year]
    set currclicks [get -clicks]
    $day delete 0 end
    if {(($day == "01") || ($day == "1")) && ($direction == -1)} {
	$day insert 0 $lastday
        return
    }
    if {($day == $lastday) && ($direction == 1)} {
	$day insert 0 "01"
        return
    }
    set clicks [clock scan "$direction day" -base $currclicks]
    $day insert 0 [clock format $clicks -format "%d"]
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _packDate when
#
# Pack the components of the date spinner.  If "when" is "now", the 
# change is applied immediately.  If it is "later" or it is not 
# specified, then the change is applied later, when the application 
# is idle.
# ------------------------------------------------------------------
::itcl::body Spindate::_packDate {{when later}} {
    if {$when eq "later"} {
	if {$_repack == ""} {
	    set _repack [after idle [itcl::code $this _packDate now]]
	}
	return
    } elseif {$when ne "now"} {
	error "bad option \"$when\": should be now or later"
    }
    #
    # Turn off the minsizes for all the rows and columns.  
    #
    for {set i 0} {$i < 5} {incr i} {
	grid rowconfigure $_interior $i -minsize 0
	grid columnconfigure $_interior $i -minsize 0
    }
    set _repack ""
    #
    # Based on the orientation, use the grid to place the components into
    # the proper rows and columns.
    #
    switch $itcl_options(-orient) {
    vertical {
        set row -1
        if {$itcl_options(-monthon)} {
	    grid $month -row [incr row] -column 0 \
	        -sticky nsew 
        } else {
	    grid forget $month
        }
        if {$itcl_options(-dayon)} {
	    if {$itcl_options(-dayon)} {
	        grid rowconfigure $_interior [incr row] \
		    -minsize $itcl_options(-datemargin)
	    }
	    grid $day -row [incr row] -column 0 \
	        -sticky nsew 
        } else {
	    grid forget $day
        }
        if {$itcl_options(-yearon)} {
	    if {$itcl_options(-monthon) || $itcl_options(-dayon)} {
	        grid rowconfigure $_interior [incr row] \
		    -minsize $itcl_options(-datemargin)
	    }
	    grid $year -row [incr row] -column 0 \
	        -sticky nsew 
        } else {
	    grid forget $year
        }
        if {$itcl_options(-labelpos) == "w"} {
	    ::itcl::widgets::Labeledwidget::alignlabels $month \
		    $day $year
        }
      }
    horizontal {
        set column -1
        if {$itcl_options(-monthon)} {
	    grid $month -row 0 -column [incr column] -sticky nsew 
        } else {
	    grid forget $month
        }
        if {$itcl_options(-dayon)} {
            if {$itcl_options(-monthon)} {
	        grid columnconfigure $_interior [incr column] \
		    -minsize $itcl_options(-datemargin)
	    }
	    grid $day -row 0 -column [incr column] -sticky nsew 
        } else {
	    grid forget $itk_component(day)
        }
        if {$itcl_options(-yearon)} {
	    if {$itcl_options(-monthon) || $itcl_options(-dayon)} {
	        grid columnconfigure $_interior [incr column] \
		    -minsize $itcl_options(-datemargin)
	    }
	    grid $year -row 0 -column [incr column] -sticky nsew 
        } else {
	    grid forget $year
        }
        #
        # Un-align labels.
        #
        $month configure -labelmargin 1
        $day configure -labelmargin 1
        $year configure -labelmargin 1
      }
    default {
        error "bad orient option \"$itcl_options(-orient)\", should\
	    be \"vertical\" or \"horizontal\""
      } 
    }
} 

# ------------------------------------------------------------------
# PRIVATE METHOD: _lastDay month year
#
# Internal method which determines the last day of the month for
# the given month and year.  We start at 28 and go forward till
# we fail.  Crude but effective.
# ------------------------------------------------------------------
::itcl::body Spindate::_lastDay {month year} {
    set lastone 28
    for {set lastone 28} {$lastone < 32} {incr lastone} {
	if {[regexp {[0-9]+} $month]} {
	  if {[catch {clock scan "$month/[expr {$lastone + 1}]/$year"}] != 0} {
		return $lastone
	  }
	} else {
	  if {[catch {clock scan "[expr {$lastone + 1}] $month $year"}] != 0} {
		return $lastone
	  }
	}
    }
}

} ; # end ::itcl::widgets
