#
# Spintime
# ----------------------------------------------------------------------
# Implements a Time spinner widget.  A time spinner contains three
# integer spinners:  one for hours, one for minutes and one for
# seconds.  Options exist to manage to behavior, appearance, and
# format of each component spinner.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Spintime
# written by:
#    Sue Yockey               E-mail: yockey@acm.org
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: spintime.tcl,v 1.1.2.1 2009/01/09 17:52:37 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Spintime.hourLabel "Hour" widgetDefault
option add *Spintime.minuteLabel "Minute" widgetDefault
option add *Spintime.secondLabel "Second" widgetDefault
option add *Spintime.hourWidth 3 widgetDefault
option add *Spintime.minuteWidth 3 widgetDefault
option add *Spintime.secondWidth 3 widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Spintime class.
# 
proc ::itcl::widgets::spintime {pathName args} {
    uplevel ::itcl::widgets::Spintime $pathName $args
}

# ------------------------------------------------------------------
#                            SPINTIME
# ------------------------------------------------------------------
::itcl::extendedclass Spintime {
    component itcl_hull
    component itcl_interior
    component hour
    component minute
    component second
    
    option [list -orient orient Orient] -default vertical -configuremethod configOrient
    option [list -labelpos labelPos Position] -default w -configuremethod configLabelpos
    option [list -houron hourOn HourOn] -default true -configuremethod configHouron
    option [list -minuteon minuteOn MinuteOn] -default true -configuremethod configMinuteon
    option [list -secondon secondOn SecondOn] -default true -configuremethod configSecondon
    option [list -timemargin timeMargin Margin] -default 1 -configuremethod configTimemargin
    option [list -militaryon militaryOn MilitaryOn] -default true -configuremethod configMilitaryon
    
    delegate option [list -hourlabel hourLabel Text] to hour as -labeltext
    delegate option [list -hourwidth hourWidth Width] to hour as -width
    delegate option [list -minutelabel minuteLabel Text] to minute as -labeltext
    delegate option [list -minutewidth minuteWidth Width] to minute as -width
    delegate option [list -secondlabel secondLabel Text] to second as -labeltext
    delegate option [list -secondwidth secondWidth Width] to second as -width

    protected variable _repack {}             ;# Reconfiguration flag.
    protected variable _interior

    constructor {args} {}
    destructor {}

    protected method _packTime {{when later}}
    protected method _down60 {comp}
    protected method configOrient {option value}
    protected method configLabelpos {option value}
    protected method configHouron {option value}
    protected method configMinuteon {option value}
    protected method configSecondon {option value}
    protected method configTimemargin {option value}
    protected method configMilitaryon {option value}

    public method get {{format "-string"}} 
    public method show {{date now}}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Spintime::constructor {args} {
    createhull frame $this -class [info class]
    set _interior $itcl_interior
    set clicks [clock seconds]

    #
    # Create Hour Spinner
    #
    setupcomponent hour using ::itcl::widgets::Spinint $itcl_interior.hour -fixed 2 -range {0 23} -justify right
    keepcomponentoption hour -background -cursor -arroworient -foreground \
		-labelfont -labelmargin -relief -textbackground \
		-textfont -repeatdelay -repeatinterval
    #
    # Take off the default bindings for selection and motion.
    #
    bind [$hour component entry] <1> {break}
    bind [$hour component entry] <Button1-Motion> {break}
    #
    # Create Minute Spinner
    #
    setupcomponent minute using ::itcl::widgets::Spinint $itcl_interior.minute \
		-decrement [itcl::code $this _down60 minute] \
		-fixed 2 -range {0 59} -justify right
    keepcomponentoption minute -background -cursor -arroworient -foreground \
		-labelfont -labelmargin -relief -textbackground \
		-textfont -repeatdelay -repeatinterval
    #
    # Take off the default bindings for selection and motion.
    #
    bind [$minute component entry] <1> {break}
    bind [$minute component entry] <Button1-Motion> {break}

    #
    # Create Second Spinner
    #
    setupcomponent second using ::itcl::widgets::Spinint $itcl_interior.second  \
		-decrement [itcl::code $this _down60 second] \
		-fixed 2 -range {0 59} -justify right
    keepcomponentoption second -background -cursor -arroworient -foreground \
		-labelfont -labelmargin -relief -textbackground \
		-textfont -repeatdelay -repeatinterval

    #
    # Take off the default bindings for selction and motion.
    #
    bind [$second component entry] <1> {break}
    bind [$second component entry] <Button1-Motion> {break}

    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args

    # 
    # Show the current time.
    #
    show now
}
	
# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Spintime::destructor {} {
    if {$_repack != ""} {
        after cancel $_repack
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -orient
# 
# Specifies the orientation of the 3 spinners for Hour, Minute 
# and second.
# ------------------------------------------------------------------
::itcl::body Spintime::configOrient {option value} {
    set itcl_options($option) $value
    _packTime
}

# ------------------------------------------------------------------
# OPTION: -labelpos
# 
# Specifies the location of all 3 spinners' labels. 
# Overloaded 
# ------------------------------------------------------------------
::itcl::body Spintime::configLabelpos {option value} {
    switch $value {
    n {
        $hour configure -labelpos n
        $minute configure -labelpos n
        $second configure -labelpos n
        #
        # Un-align labels
        #
        $hour configure -labelmargin 1
        $minute configure -labelmargin 1
        $second configure -labelmargin 1
      }
    s {
        $hour configure -labelpos s
        $minute configure -labelpos s
        $second configure -labelpos s
        #
        # Un-align labels
        #
        $hour configure -labelmargin 1
        $minute configure -labelmargin 1
        $second configure -labelmargin 1
      }
    w {
        $hour configure -labelpos w
        $minute configure -labelpos w
        $second configure -labelpos w
      }
    e {
        $hour configure -labelpos e
        $minute configure -labelpos e
        $second configure -labelpos e
        #
        # Un-align labels
        #
        $hour configure -labelmargin 1
        $minute configure -labelmargin 1
        $second configure -labelmargin 1
      }
    default {
        error "bad labelpos option \"$value\",\
	        should be n, s, w or e"
      }
    }

    set itcl_options($option) $value
    _packTime
}

# ------------------------------------------------------------------
# OPTION: -houron
# 
# Specifies whether or not to display the hour spinner.
# ------------------------------------------------------------------
::itcl::body Spintime::configHouron {option value} {
    set itcl_options($option) $value
    _packTime
}

# ------------------------------------------------------------------
# OPTION: -minuteon
# 
# Specifies whether or not to display the minute spinner.
# ------------------------------------------------------------------
::itcl::body Spintime::configMinuteon {option value} {
    set itcl_options($option) $value
    _packTime
}

# ------------------------------------------------------------------
# OPTION: -secondon
# 
# Specifies whether or not to display the second spinner.
# ------------------------------------------------------------------
::itcl::body Spintime::configSecondon {option value} {
    set itcl_options($option) $value
    _packTime
}


# ------------------------------------------------------------------
# OPTION: -timemargin
# 
# Specifies the margin space between the hour and minute spinners 
# and the minute and second spinners. 
# ------------------------------------------------------------------
::itcl::body Spintime::configTimemargin {option value} {
    set itcl_options($option) $value
    _packTime
}

# ------------------------------------------------------------------
# OPTION: -militaryon
#
# Specifies 24-hour clock or 12-hour.
# ------------------------------------------------------------------
::itcl::body Spintime::configMilitaryon {option value} {
    set clicks [clock seconds]
    if {$value} {
	$hour configure -range {0 23}
	$hour delete 0 end
	$hour insert end [clock format $clicks -format "%H"]
    } else {
	$hour configure -range {1 12}
	$hour delete 0 end
	$hour insert end [clock format $clicks -format "%I"]
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: get ?format?
#
# Get the value of the time spinner in one of two formats string or 
# as an integer clock value using the -string and -clicks options 
# respectively.  The default is by string.  Reference the clock 
# command for more information on obtaining time and its formats.
# ------------------------------------------------------------------
::itcl::body Spintime::get {{format "-string"}} {
    set my_hour [$hour get]
    set my_minute [$minute get]
    set my_second [$second get]
    switch -- $format {
    "-string" {
        return "$my_hour:$my_minute:$my_second"
      }
    "-clicks" {
        return [clock scan "$my_hour:$my_minute:$my_second"]
      }
    default {
        error "bad format option \"$format\":\
                  should be -string or -clicks"
      }
    }
}

# ------------------------------------------------------------------
# PUBLIC METHOD: show time
#
# Changes the currently displayed time to be that of the time
# argument.  The time may be specified either as a string or an
# integer clock value.  Reference the clock command for more 
# information on obtaining time and its format.
# ------------------------------------------------------------------
::itcl::body Spintime::show {{time "now"}} {
    if {$time eq "now"} {
	set seconds [clock seconds]
    } else {
	if {[catch {clock format $time}] == 0} {
	    set seconds $time
	} elseif {[catch {set seconds [clock scan $time]}] != 0} {
	    error "bad time: \"$time\", must be a valid time\
               string, clock clicks value or the keyword now"
	}
    }
    $hour delete 0 end
    if {$itcl_options(-militaryon)} {
	scan [clock format $seconds -format "%H"] "%d" my_hour
    } else {
	scan [clock format $seconds -format "%I"] "%d" my_hour
    }
    $hour insert end $my_hour
    $minute delete 0 end
    scan [clock format $seconds -format "%M"] "%d" my_minute 
    $minute insert end $my_minute
    $second delete 0 end
    scan [clock format $seconds -format "%S"] "%d" my_seconds
    $second insert end $my_seconds
    return
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _packTime ?when?
#
# Pack components of time spinner.  If "when" is "now", the change 
# is applied immediately.  If it is "later" or it is not specified,
# then the change is applied later, when the application is idle.
# ------------------------------------------------------------------
::itcl::body Spintime::_packTime {{when later}} {
    if {$when eq "later"} {
	if {$_repack == ""} {
	    set _repack [after idle [itcl::code $this _packTime now]]
	}
	return
    } elseif {$when != "now"} {
	error "bad option \"$when\": should be now or later"
    }
    for {set i 0} {$i < 5} {incr i} {
	grid rowconfigure $_interior $i -minsize 0
	grid columnconfigure $_interior $i -minsize 0
    }

    if {$itcl_options(-minuteon)} {
	set minuteon 1
    } else {
	set minuteon 0
    }
    if {$itcl_options(-secondon)} {
	set secondon 1
    } else {
	set secondon 0
    }
    set _repack ""
    switch $itcl_options(-orient) {
    vertical {
        set row -1
        if {$itcl_options(-houron)} {
	    grid $hour -row [incr row] -column 0 -sticky nsew 
        } else {
	    grid forget $hour
        }
        if {$itcl_options(-minuteon)} {
	    if {$itcl_options(-houron)} {
	        grid rowconfigure $_interior [incr row] \
		    -minsize $itcl_options(-timemargin)
	    }
	    grid $minute -row [incr row] -column 0 -sticky nsew 
        } else {
	    grid forget $minute
        }
        if {$itcl_options(-secondon)} {
	    if {$minuteon || $secondon} {
	        grid rowconfigure $_interior [incr row] \
		    -minsize $itcl_options(-timemargin)
	    }
	    grid $second -row [incr row] -column 0 -sticky nsew 
        } else {
	    grid forget $second
        }
        if {$itcl_options(-labelpos) == "w"} {
	    ::itcl::widgets::Labeledwidget::alignlabels $hour \
		    $minute $second
        }
      }
    horizontal {
        set column -1
        if {$itcl_options(-houron)} {
	    grid $hour -row 0 -column [incr column] -sticky nsew 
        } else {
	    grid forget $hour
        }
        if {$itcl_options(-minuteon)} {
	    if {$itcl_options(-houron)} {
	        grid columnconfigure $_interior [incr column] \
		    -minsize $itcl_options(-timemargin)
	    }
    
	    grid $minute -row 0 -column [incr column] -sticky nsew 
        } else {
	    grid forget $minute
        }
        if {$itcl_options(-secondon)} {
	    if {$minuteon || $secondon} {
	        grid columnconfigure $_interior [incr column] \
		    -minsize $itcl_options(-timemargin)
	    }
	    grid $second -row 0 -column [incr column] -sticky nsew 
        } else {
	    grid forget $second
        }
        #
        # Un-align labels
        #
        $hour configure -labelmargin 1
        $minute configure -labelmargin 1
        $second configure -labelmargin 1
      }
    default {
        error "bad orient option \"$itcl_options(-orient)\", should\
		    be \"vertical\" or \"horizontal\""
      }
    } 
}

# ------------------------------------------------------------------
# METHOD: down60
#
# Down arrow button press event.  Decrement value in the minute
# or second entry.
# ------------------------------------------------------------------
::itcl::body Spintime::_down60 {comp} {
    set step [[set $comp] cget -step]
    set val [[set $comp] get]
    incr val -$step
    if {$val < 0} {
       set val [expr {60-$step}]
    }
    [set $comp] delete 0 end
    [set $comp] insert 0 $val
}

} ; # end :Itcl::widgets
