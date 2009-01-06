#
# Watch
# ----------------------------------------------------------------------
# Implements a a clock widget in a canvas.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Watch
# written by:
#   AUTHOR:  John A. Tucker          E-mail: jatucker@austin.dsccc.com
#   Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: watch.tcl,v 1.1.2.2 2009/01/06 16:41:22 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Watch.labelFont		\
    -*-Courier-Medium-R-Normal--*-120-*-*-*-*-*-*	widgetDefault

#
# Use option database to override default resources of base classes.
#
option add *Watch.width 155 widgetDefault
option add *Watch.height 175 widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Watch class.
# 
proc ::itcl::widgets::watch {pathName args} {
    uplevel ::itcl::widgets::Watch $pathName $args
}

::itcl::extendedclass ::itcl::widgets::Watch {

    component itcl_hull
    component itcl_interior
    component btframe
    component canvas
    component am
    component pm

    option [list -hourradius hourRadius Radius] -default .50 -configuremethod configHourradius
    option [list -hourcolor hourColor Color] -default red -configuremethod configHourcolor

    option [list -minuteradius minuteRadius Radius] -default .80 -configuremethod configMinuteradius
    option [list -minutecolor minuteColor Color] -default yellow -configuremethod configMinutecolor

    option [list -secondradius secondRadius Radius] -default .90 -configuremethod configSecondradius
    option [list -secondcolor secondColor Color] -default black -configuremethod configSecondcolor

    option [list -pivotradius pivotRadius Radius] -default .10
    option [list -pivotcolor pivotColor Color] -default white -configuremethod configPivotcolor

    option [list -clockcolor clockColor Color] -default white -configuremethod configClockcolor
    option [list -clockstipple clockStipple ClockStipple] -default {} -configuremethod configClockstipple

    option [list -state state State] -default normal -configuremethod configState
    option [list -showampm showAmPm ShowAmPm] -default true -configuremethod configShowampm
    option [list -tickcolor tickColor Color] -default black -configuremethod configTickcolor

    delegate option [list -labelfont labelFont Font] to am as -font
    delegate option [list -labelfont labelFont Font] to pm as -font

    protected variable _interior
    protected variable _radius
    protected variable _theta
    protected variable _extent
    protected variable _reposition ""  ;# non-null => _displayClock pending
    protected variable _timeVar
    protected variable _x0 1
    protected variable _y0 1

    protected common _ampmVar
    protected common PI [expr {2*asin(1.0)}]

    constructor {args} {}
    destructor {}

    protected method _handMotionCB {tag x y}
    protected method _drawHand {tag}
    protected method _handReleaseCB {tag x y}
    protected method _displayClock {{when "later"}}
    protected method configState {option value}
    protected method configShowampm {option value}
    protected method configTickcolor {option value}
    protected method configClockstipple {option value}
    protected method configClockcolor {option value}
    protected method configPivotcolor {option value}
    protected method configHourradius {option value}
    protected method configHourcolor {option value}
    protected method configMinuteradius {option value}
    protected method configMinutecolor {option value}
    protected method configSecondradius {option value}
    protected method configSecondcolor {option value}

    public method get {{format "-string"}}
    public method show {{time "now"}}
    public method watch {args}

}

# -----------------------------------------------------------------------------
#                        CONSTRUCTOR
# -----------------------------------------------------------------------------
::itcl::body Watch::constructor {args} {
    createhull frame $this -class [namespace tail [info class]]
    set itcl_interior $win
    #
    # Add back to the hull width and height options and make the
    # borderwidth zero since we don't need it.
    #
    set _interior $itcl_interior

#    itcl_options add hull.width hull.height
    keepcomponentoption itcl_hull -width -height

    $itcl_hull configure -borderwidth 0
    grid propagate $win no

    set _ampmVar($this) "AM"
    set _radius(outer) 1

    set _radius(hour) 1
    set _radius(minute) 1
    set _radius(second) 1

    set _theta(hour) 30
    set _theta(minute) 6
    set _theta(second) 6

    set _extent(hour) 14
    set _extent(minute) 14
    set _extent(second) 2

    set _timeVar(hour) 12
    set _timeVar(minute) 0
    set _timeVar(second) 0

    #
    # Create the frame in which the "AM" and "PM" radiobuttons will be drawn
    #
    setupcomponent btframe using frame $itcl_interior.frame

    #
    # Create the canvas in which the clock will be drawn
    #
    setupcomponent canvas using canvas $itcl_interior.canvas
    bind $canvas <Map> +[list puts stderr Mapevent]
    bind $canvas <Map> +[itcl::code $this _displayClock]
    bind $canvas <Configure> +[list puts stderr Configurevent]
    bind $canvas <Configure> +[itcl::code $this _displayClock]

    #
    # Create the "AM" and "PM" radiobuttons to be drawn in the canvas
    #
    setupcomponent am using radiobutton $btframe.am \
	    -text "AM" \
	    -value "AM" \
	    -variable [itcl::scope _ampmVar($this)]
    keepcomponentoption am -background -cursor -labelfont -foreground
# FIXME rename -font -labelfont labelFont Font

    setupcomponent pm using radiobutton $btframe.pm \
	    -text "PM" \
	    -value "PM" \
	    -variable [itcl::scope _ampmVar($this)]
    keepcomponentoption am -background -cursor -labelfont -foreground
# FIXME	rename -font -labelfont labelFont Font

    #
    # Create the canvas item for displaying the main oval which encapsulates
    # the entire clock.
    #
    watch create oval 0 0 2 2 -width 5 -tags clock

    #
    # Create the canvas items for displaying the 60 ticks marks around the
    # inner perimeter of the watch.
    #
    set extent 3
    for {set i 0} {$i < 60} {incr i} {
	set start [expr {$i * 6 - 1}]
	set tag [expr {[expr {$i % 5}] == 0 ? "big" : "little"}]
	watch create arc 0 0 0 0 \
	    -style arc \
	    -extent $extent \
	    -start $start \
	    -tags "tick$i tick $tag"
    }

    #
    # Create the canvas items for displaying the hour, minute, and second hands
    # of the watch.  Add bindings to allow the mouse to move and set the
    # clock hands.
    #
    watch create arc 1 1 1 1 -extent 30 -tags minute
    watch create arc 1 1 1 1 -extent 30 -tags hour
    watch create arc 1 1 1 1 -tags second

    #
    # Create the canvas item for displaying the center of the watch in which
    # the hour, minute, and second hands will pivot.
    #
    watch create oval 0 0 1 1 -width 5 -fill black -tags pivot

    #
    # Position the "AM/PM" button frame and watch canvas.
    #
    grid $btframe -row 0 -column 0 -sticky new
    grid $canvas -row 1 -column 0 -sticky nsew

    grid rowconfigure    $itcl_interior 0 -weight 0
    grid rowconfigure    $itcl_interior 1 -weight 1
    grid columnconfigure $itcl_interior 0 -weight 1

    uplevel 0 itcl_initoptions $args
}

# -----------------------------------------------------------------------------
#                           DESTURCTOR
# -----------------------------------------------------------------------------
::itcl::body Watch::destructor {} {
    if {$_reposition != ""} {
	after cancel $_reposition
    }
}

# -----------------------------------------------------------------------------
#                            METHODS
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# METHOD: _handReleaseCB tag x y
#
# -----------------------------------------------------------------------------
::itcl::body Watch::_handReleaseCB {tag x y} {
    set atanab [expr {atan2(double($y-$_y0),double($x-$_x0))*(180/$PI)}]
    set degrees [expr {$atanab > 0 ? [expr {360-$atanab}] : abs($atanab)}]
    set ticks [expr {round($degrees/$_theta($tag))}]
    set _timeVar($tag) [expr {((450-$ticks*$_theta($tag))%360)/$_theta($tag)}]
    if {$tag == "hour" && $_timeVar(hour) == 0} {
	set _timeVar($tag) 12
    }
    _drawHand $tag
}

# -----------------------------------------------------------------------------
# PROTECTED METHOD: _handMotionCB tag x y
#
# -----------------------------------------------------------------------------
::itcl::body Watch::_handMotionCB {tag x y} {
    if {$x == $_x0 || $y == $_y0} {
	return
    }
    set a [expr {$y-$_y0}]
    set b [expr {$x-$_x0}]
    set c [expr {hypot($a,$b)}]
    set atanab [expr {atan2(double($a),double($b))*(180/$PI)}]
    set degrees [expr {$atanab > 0 ? [expr 360-$atanab] : abs($atanab)}]
    set x2 [expr {$_x0+$_radius($tag)*($b/double($c))}]
    set y2 [expr {$_y0+$_radius($tag)*($a/double($c))}]
    watch coords $tag \
	[expr {$x2-$_radius($tag)}] \
	[expr {$y2-$_radius($tag)}] \
	[expr {$x2+$_radius($tag)}] \
	[expr {$y2+$_radius($tag)}]
    set start [expr {$degrees-180-($_extent($tag)/2)}]
    watch itemconfigure $tag -start $start -extent $_extent($tag)
}

# -----------------------------------------------------------------------------
# PROTECTED METHOD: get ?format?
#
# -----------------------------------------------------------------------------
::itcl::body Watch::get {{format "-string"}} {
    set timestr [format "%02d:%02d:%02d %s" \
		     $_timeVar(hour) $_timeVar(minute) \
		     $_timeVar(second) $_ampmVar($this)]
    switch -- $format {
    "-string" {
        return $timestr
      }
    "-clicks" {
        return [clock scan $timestr]
      }
    default {
        error "bad format option \"$format\":\
                  should be -string or -clicks"
      }
    }
}

# -----------------------------------------------------------------------------
# METHOD: watch ?args?
#
# Evaluates the specified args against the canvas component.
# -----------------------------------------------------------------------------
::itcl::body Watch::watch {args} {
    return [uplevel 0 $canvas $args]
}

# -----------------------------------------------------------------------------
# METHOD: _drawHand tag
#
# -----------------------------------------------------------------------------
::itcl::body Watch::_drawHand {tag} {
    set degrees [expr {abs(450-($_timeVar($tag)*$_theta($tag)))%360}]
    set radians [expr {$degrees*($PI/180)}]
    set x [expr {$_x0+$_radius($tag)*cos($radians)}]
    set y [expr {$_y0+$_radius($tag)*sin($radians)*(-1)}]
    watch coords $tag \
	[expr {$x-$_radius($tag)}] \
	[expr {$y-$_radius($tag)}] \
	[expr {$x+$_radius($tag)}] \
	[expr {$y+$_radius($tag)}]
    set start [expr {$degrees-180-($_extent($tag)/2)}]
    watch itemconfigure $tag -start $start
}

# ------------------------------------------------------------------
# PUBLIC METHOD: show time
#
# Changes the currently displayed time to be that of the time
# argument.  The time may be specified either as a string or an
# integer clock value.  Reference the clock command for more 
# information on obtaining times and their formats.
# ------------------------------------------------------------------
::itcl::body Watch::show {{time "now"}} {
    if {$time == "now"} {
	set seconds [clock seconds]
    } elseif {![catch {clock format $time}]} {
	set seconds $time
    } elseif {[catch {set seconds [clock scan $time]}]} {
	error "bad time: \"$time\", must be a valid time\
               string, clock clicks value or the keyword now"
    }
    set timestring [clock format $seconds -format "%I %M %S %p"]
    set _timeVar(hour)   [expr int(1[lindex $timestring 0] - 100)]
    set _timeVar(minute) [expr int(1[lindex $timestring 1] - 100)]
    set _timeVar(second) [expr int(1[lindex $timestring 2] - 100)]
    set _ampmVar($this) [lindex $timestring 3]
    _drawHand hour
    _drawHand minute
    _drawHand second
}

# -----------------------------------------------------------------------------
# PROTECTED METHOD: _displayClock ?when?
#
# Places the hour, minute, and second dials in the canvas.  If "when" is "now",
# the change is applied immediately.  If it is "later" or it is not specified,
# then the change is applied later, when the application is idle.
# -----------------------------------------------------------------------------
::itcl::body Watch::_displayClock {{when "later"}} {
puts stderr "_displayClock!$when!"
    if {$when == "later"} {
	if {$_reposition == ""} {
	    set _reposition [after idle [itcl::code $this _displayClock now]]
	}
	return
    }
    #
    # Compute the center coordinates for the clock based on the
    # with and height of the canvas.
    #
puts stderr "_displayClock2![winfo width $canvas]![winfo height $canvas]!"
    set width [winfo width $canvas]
    set height [winfo height $canvas]
    set _x0 [expr {$width/2}]
    set _y0 [expr {$height/2}]
    #
    # Set the radius of the watch, pivot, hour, minute and second items.
    #
    set _radius(outer)  [expr {$_x0 < $_y0 ? $_x0 : $_y0}]
    set _radius(pivot)  [expr {$itcl_options(-pivotradius)*$_radius(outer)}]
    set _radius(hour)   [expr {$itcl_options(-hourradius)*$_radius(outer)}]
    set _radius(minute) [expr {$itcl_options(-minuteradius)*$_radius(outer)}]
    set _radius(second) [expr {$itcl_options(-secondradius)*$_radius(outer)}]
    set outerWidth [watch itemcget clock -width]
puts stderr "outerWidth!$outerWidth!"
    #
    # Set the coordinates of the clock item
    #
    set x1Outer $outerWidth
    set y1Outer $outerWidth
    set x2Outer [expr {$width-$outerWidth}]
    set y2Outer [expr {$height-$outerWidth}]
puts stderr "watch coords clock $x1Outer $y1Outer $x2Outer $y2Outer"
    watch coords clock $x1Outer $y1Outer $x2Outer $y2Outer
    #
    # Set the coordinates of the tick items
    #
    set offset [expr {$outerWidth*2}]
    set x1Tick [expr {$x1Outer+$offset}]
    set y1Tick [expr {$y1Outer+$offset}]
    set x2Tick [expr {$x2Outer-$offset}]
    set y2Tick [expr {$y2Outer-$offset}]
    for {set i 0} {$i < 60} {incr i} {
	watch coords tick$i $x1Tick $y1Tick $x2Tick $y2Tick
    }
    set maxTickWidth [expr {$_radius(outer)-$_radius(second)+1}]
    set minTickWidth [expr {round($maxTickWidth/2)}]
    watch itemconfigure big -width $maxTickWidth
    watch itemconfigure little -width [expr {round($maxTickWidth/2)}]
    #
    # Set the coordinates of the pivot item
    #
    set x1Center [expr {$_x0-$_radius(pivot)}]
    set y1Center [expr {$_y0-$_radius(pivot)}]
    set x2Center [expr {$_x0+$_radius(pivot)}]
    set y2Center [expr {$_y0+$_radius(pivot)}]
puts stderr "watch coords pivot $x1Center $y1Center $x2Center $y2Center"
    watch coords pivot $x1Center $y1Center $x2Center $y2Center        
    #
    # Set the coordinates of the hour, minute, and second dial items
    #
    watch itemconfigure hour -extent $_extent(hour)
    _drawHand hour
puts stderr "after _drawHand hour"
    watch itemconfigure minute -extent $_extent(minute)
    _drawHand minute
    watch itemconfigure second -extent $_extent(second)
    _drawHand second
    set _reposition ""
puts stderr "_displayClock end"
}

# -----------------------------------------------------------------------------
#                             OPTIONS
# -----------------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: state
#
# Configure the editable state of the widget.  Valid values are
# normal and disabled.  In a disabled state, the hands of the 
# watch are not selectabled.
# ------------------------------------------------------------------
::itcl::body Watch::configState {option value} {
    if {$value eq "normal"} {
	watch bind minute <B1-Motion> \
	    [itcl::code $this _handMotionCB minute %x %y]
	watch bind minute <ButtonRelease-1> \
	    [itcl::code $this _handReleaseCB minute %x %y]
	watch bind hour <B1-Motion> \
	    [itcl::code $this _handMotionCB hour %x %y]
	watch bind hour <ButtonRelease-1> \
	    [itcl::code $this _handReleaseCB hour %x %y]
	watch bind second <B1-Motion> \
	    [itcl::code $this _handMotionCB second %x %y]
	watch bind second <ButtonRelease-1> \
	    [itcl::code $this _handReleaseCB second %x %y]
	$am configure -state normal
	$pm configure -state normal
    } elseif {$value eq "disabled"} {
	watch bind minute <B1-Motion> {}
	watch bind minute <ButtonRelease-1> {}
	watch bind hour <B1-Motion> {}
	watch bind hour <ButtonRelease-1> {}
	watch bind second <B1-Motion> {}
	watch bind second <ButtonRelease-1> {}
	$am configure -state disabled \
	    -disabledforeground [$am cget -background]
	$pm configure -state normal \
	    -disabledforeground [$am cget -background]
    } else {
	error "bad state option \"$value\":\
                   should be normal or disabled"
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: showampm
#
# Configure the display of the AM/PM radio buttons.
# ------------------------------------------------------------------
::itcl::body Watch::configShowampm {option value} {
    switch -- $value {
    0 -
    no -
    false -
    off { 
        pack forget $am
        pack forget $pm
      }
    1 -
    yes -
    true -
    on { 
        pack $am -side left -fill both -expand 1
        pack $pm -side right -fill both -expand 1 
      }

    default {
           error "bad showampm option \"$value\":\
                   should be boolean"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: pivotcolor
#
# Configure the color of the clock pivot.
#
::itcl::body Watch::configPivotcolor {option value} {
    watch itemconfigure pivot -fill $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: clockstipple
#
# Configure the stipple pattern for the clock fill color.
#
::itcl::body Watch::configClockstipple {option value} {
    watch itemconfigure clock -stipple $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: clockcolor
#
# Configure the color of the clock.
#
::itcl::body Watch::configClockcolor {option value} {
    watch itemconfigure clock -fill $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: hourcolor
#
# Configure the color of the hour hand.
#
::itcl::body Watch::configHourcolor {option value} {
puts stderr "Watch::configHourcolor!$value!"
    watch itemconfigure hour -fill $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: minutecolor
#
# Configure the color of the minute hand.
#
::itcl::body Watch::configMinutecolor {option value} {
    watch itemconfigure minute -fill $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: secondcolor
#
# Configure the color of the second hand.
#
::itcl::body Watch::configSecondcolor {option value} {
    watch itemconfigure second -fill $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: tickcolor
#
# Configure the color of the ticks.
#
::itcl::body Watch::configTickcolor {option value} {
    watch itemconfigure tick -outline $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: hourradius
#
# Configure the radius of the hour hand.
#
::itcl::body Watch::configHourradius {option value} {
puts stderr "configHourradius"
    set itcl_options($option) $value
    _displayClock
}

# ------------------------------------------------------------------
# OPTION: minuteradius
#
# Configure the radius of the minute hand.
#
::itcl::body Watch::configMinuteradius {option value} {
puts stderr "configMinuteradius"
    set itcl_options($option) $value
    _displayClock
}

# ------------------------------------------------------------------
# OPTION: secondradius
#
# Configure the radius of the second hand.
#
::itcl::body Watch::configSecondradius {option value} {
puts stderr "configSecondradius"
    set itcl_options($option) $value
    _displayClock
}

} ; # end ::itcl::widgets
