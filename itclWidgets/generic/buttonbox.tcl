#
# Buttonbox
# ----------------------------------------------------------------------
# Manages a framed area with Motif style buttons.  The button box can 
# be configured either horizontally or vertically.  
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Buttonbox
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Bret A. Schuhmacher      EMAIL: bas@wn.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: buttonbox.tcl,v 1.1.2.1 2009/01/05 21:08:42 wiede Exp $
# ======================================================================

namespace eval ::itcl::widgets {

if {0} {
namespace eval Buttonbox {
    #
    # Set up some class level bindings for map and configure events.
    #
    bind bbox-map <Map> [itcl::code %W _setBoxSize]
    bind bbox-config <Configure> [itcl::code %W _positionButtons]
}
}

#
# Provide a lowercased access method for the Buttonbox class.
# 
proc ::itcl::widgets::buttonbox {pathName args} {
    uplevel ::itcl::widgets::Buttonbox $pathName $args
}
    
# ------------------------------------------------------------------
#                            BUTTONBOX
# ------------------------------------------------------------------
::itcl::extendedclass Buttonbox {
    component itcl_hull
    component itcl_interior

    option [list -activebackground activeBackground Foreground] -default #ececec
    option [list -activeforeground activeForeground Background] -default #000000
    option [list -disabledforeground disabledForeground DisabledForeground] -default #a3a3a3
    option [list -font font Font] -default TkDefaultFont
    option [list -pady padY Pad] -default 5 -configuremethod configPady
    option [list -padx padX Pad] -default 5 -configuremethod configPadx
    option [list -orient orient Orient] -default "horizontal" -configuremethod configOrient
    option [list -foreground foreground Foreground] -default black
    
    delegate option -borderwidth to itcl_hull
    delegate option -highlightcolor to itcl_hull
    delegate option -highlightthickness to itcl_hull

    constructor {args} {}
    destructor {}

    private variable _resizeFlag {}         ;# Flag for resize needed.
    private variable _buttonList {}         ;# List of all buttons in box.
    private variable _displayList {}        ;# List of displayed buttons.
    private variable _unique 0              ;# Counter for button widget ids.

    private method _positionButtons {}
    private method _setBoxSize {{when later}}
    private method _getMaxWidth {}
    private method _getMaxHeight {}

    protected method configPadx {option value}
    protected method configPady {option value}
    protected method configOrient {option value}

    public method index {args}
    public method add {args}
    public method insert {args}
    public method delete {args}
    public method default {args}
    public method hide {args}
    public method show {args}
    public method invoke {args}
    public method buttonconfigure {args}
    public method buttoncget {index option}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Buttonbox::constructor {args} {
    set win [createhull frame $this -class [info class] -borderwidth 2 -highlightthickness 1]
    set itcl_interior $win
    # 
    # Add Configure bindings for geometry management.  
    #
    bindtags $win [linsert [bindtags $win] 0 bbox-map]
    bindtags $win [linsert [bindtags $win] 1 bbox-config]
    pack propagate $win no
    
    #
    # Initialize the widget based on the command line options.
    #
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Buttonbox::destructor {} {
    if {$_resizeFlag ne ""} {
        after cancel $_resizeFlag
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -pady
#
# Pad the y space between the button box frame and the hull.
# ------------------------------------------------------------------
::itcl::body Buttonbox::configPady {option value} {
    _setBoxSize
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -padx
#
# Pad the x space between the button box frame and the hull.
# ------------------------------------------------------------------
::itcl::body Buttonbox::configPadx {option value} {
    _setBoxSize
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -orient
#
# Position buttons either horizontally or vertically.
# ------------------------------------------------------------------
::itcl::body Buttonbox::configOrient {option value} {
    switch $value {
    "horizontal" -
    "vertical" {
        _setBoxSize
      }
    default {
        error "bad orientation option \"$value\",\
	    should be either horizontal or vertical"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: index index
#
# Searches the buttons in the box for the one with the requested tag,
# numerical index, keyword "end" or "default".  Returns the button's 
# tag if found, otherwise error.
# ------------------------------------------------------------------    
::itcl::body Buttonbox::index {index} {
    if {[llength $_buttonList] > 0} {
	if {[regexp {(^[0-9]+$)} $index]} {
	    if {$index < [llength $_buttonList]} {
		return $index
	    } else {
		error "Buttonbox index \"$index\" is out of range"
	    }
	    
	} elseif {$index == "end"} {
	    return [expr {[llength $_buttonList] - 1}]
	    
	} elseif {$index == "default"} {
	    set idx 0
	    foreach knownButton $_buttonList {
		set knownButtonPath [lindex $_displayList $idx]
		if {[$knownButtonPath cget -defaultring]} {
		    return [lsearch -exact $_buttonList $knownButton]
		}
	        incr idx
	    }
	    error "Buttonbox \"$win\" has no default"
	} else {
	    if {[set idx [lsearch $_buttonList $index]] != -1} {
		return $idx
	    }
	    error "bad Buttonbox index \"$index\": must be number, end,\
		    default, or pattern"
	}
    } else {
	error "Buttonbox \"$win\" has no buttons"
    }
}

# ------------------------------------------------------------------
# METHOD: add tag ?option value option value ...?
#
# Add the specified button to the button box.  All PushButton options
# are allowed.  New buttons are added to the list of buttons and the 
# list of displayed buttons.  The PushButton path name is returned.
# ------------------------------------------------------------------
::itcl::body Buttonbox::add {tag args} {
    if {![::info exists $tag]} {
        ::itcl::addcomponent $this $tag
    }
    setupcomponent $tag using ::itcl::widgets::Pushbutton $win.[incr _unique]
    keepcomponentoption $tag -background -cursor -foreground
#rename -highlightbackground -background background Background
    if {$args ne ""} {
	uplevel [set $tag] configure $args
    }
    lappend _buttonList $tag
    lappend _displayList [set $tag]
    _setBoxSize
}

# ------------------------------------------------------------------
# METHOD: insert index tag ?option value option value ...?
#
# Insert the specified button in the button box just before the one 
# given by index.  All PushButton options are allowed.  New buttons 
# are added to the list of buttons and the list of displayed buttons.
# The PushButton path name is returned.
# ------------------------------------------------------------------
::itcl::body Buttonbox::insert {index tag args} {
    if {![::info exists $tag]} {
        ::itcl::addcomponent $this $tag
    }
    setupcomponent $tag using ::itcl::widgets::Pushbutton $win.[incr _unique]
    keepcomponentoption $tag -background -cursor -foreground
#rename -highlightbackground -background background Background
    if {$args ne ""} {
	uplevel [set $tag] configure $args
    }
    set index [index $index]
    set _buttonList [linsert $_buttonList $index $tag]
    set _displayList [linsert $_displayList $index [set $tag]]
    _setBoxSize
}

# ------------------------------------------------------------------
# METHOD: delete index
#
# Delete the specified button from the button box.
# ------------------------------------------------------------------
::itcl::body Buttonbox::delete {index} {
    set index [index $index]
    set tag [lindex $_buttonList $index]
    destroy $tag
    set _buttonList [lreplace $_buttonList $index $index]
    if {[set dind [lsearch $_displayList $tag]] != -1} {
	set _displayList [lreplace $_displayList $dind $dind]
    }
    _setBoxSize
    update idletasks
}

# ------------------------------------------------------------------
# METHOD: default index
#
# Sets the default to the push button given by index.
# ------------------------------------------------------------------
::itcl::body Buttonbox::default {index} {
    set index [index $index]
    set defbtn [lindex $_buttonList $index]
    foreach knownButton $_displayList {
	if {$knownButton eq [set $defbtn]} {
	    $knownButton configure -defaultring yes
	} else {
	    $knownButton configure -defaultring no
	}
    }
}

# ------------------------------------------------------------------
# METHOD: hide index
#
# Hide the push button given by index.  This doesn't remove the button 
# permanently from the display list, just inhibits its display.
# ------------------------------------------------------------------
::itcl::body Buttonbox::hide {index} {
    set index [index $index]
    set tag [lindex $_buttonList $index]
    if {[set dind [lsearch $_displayList $tag]] != -1} {
	place forget [set $tag]
	set _displayList [lreplace $_displayList $dind $dind] 
	_setBoxSize
    }
}

# ------------------------------------------------------------------
# METHOD: show index
#
# Displays a previously hidden push button given by index.  Check if 
# the button is already in the display list.  If not then add it back 
# at it's original location and redisplay.
# ------------------------------------------------------------------
::itcl::body Buttonbox::show {index} {
    set index [index $index]
    set tag [lindex $_buttonList $index]
    if {[lsearch $_displayList $tag] == -1} {
	set _displayList [linsert $_displayList $index [set $tag]]
	_setBoxSize
    }
}

# ------------------------------------------------------------------
# METHOD: invoke ?index?
#
# Invoke the command associated with a push button.  If no arguments
# are given then the default button is invoked, otherwise the argument
# is expected to be a button index.
# ------------------------------------------------------------------
::itcl::body Buttonbox::invoke {args} {
    if {[llength $args] == 0} {
	set name [lindex $_buttonList [index default]]
	[set $name] invoke
    } else {
	set name [lindex $_buttonList [index [lindex $args 0]]]
	[set $name] invoke
    }
}

# ------------------------------------------------------------------
# METHOD: buttonconfigure index ?option? ?value option value ...?
#
# Configure a push button given by index.  This method allows 
# configuration of pushbuttons from the Buttonbox level.  The options
# may have any of the values accepted by the add method.
# ------------------------------------------------------------------
::itcl::body Buttonbox::buttonconfigure {index args} {
    set tag [lindex $_buttonList [index $index]]
    set retstr [uplevel [set $tag] configure $args]
    _setBoxSize
    return $retstr
}

# ------------------------------------------------------------------
# METHOD: buttonccget index option
#
# Return value of option for push button given by index.  Option may
# have any of the values accepted by the add method.
# ------------------------------------------------------------------
::itcl::body Buttonbox::buttoncget {index option} {
    set tag [lindex $_buttonList [index $index]]
    set retstr [uplevel [set $tag] cget [list $option]]
    return $retstr
}

# -----------------------------------------------------------------
# PRIVATE METHOD: _getMaxWidth
#
# Returns the required width of the largest button.
# -----------------------------------------------------------------
::itcl::body Buttonbox::_getMaxWidth {} {
    set max 0
    foreach tag $_displayList {
	set w [winfo reqwidth $tag]
	if {$w > $max} {
	    set max $w
	}
    }
    
    return $max
}

# -----------------------------------------------------------------
# PRIVATE METHOD: _getMaxHeight
#
# Returns the required height of the largest button.
# -----------------------------------------------------------------
::itcl::body Buttonbox::_getMaxHeight {} {
    set max 0
    foreach tag $_displayList {
	set h [winfo reqheight $tag]
	if {$h > $max} {
	    set max $h
	}
    }
    return $max
}

# ------------------------------------------------------------------
# METHOD: _setBoxSize ?when?
#
# Sets the proper size of the frame surrounding all the buttons.
# If "when" is "now", the change is applied immediately.  If it is 
# "later" or it is not specified, then the change is applied later, 
# when the application is idle.
# ------------------------------------------------------------------
::itcl::body Buttonbox::_setBoxSize {{when later}} {
    if {[winfo ismapped $win]} {
	if {$when == "later"} {
	    if {$_resizeFlag == ""} {
		set _resizeFlag [after idle [itcl::code $this _setBoxSize now]]
	    }
	    return
	} elseif {$when != "now"} {
	    error "bad option \"$when\": should be now or later"
	}
	set _resizeFlag ""
	set numBtns [llength $_displayList]
	if {$itcl_options(-orient) == "horizontal"} {
	    set minw [expr {$numBtns * [_getMaxWidth] \
		    + ($numBtns+1) * $itcl_options(-padx)}]
	    set minh [expr {[_getMaxHeight] + 2 * $itcl_options(-pady)}]
	} else {
	    set minw [expr {[_getMaxWidth] + 2 * $itcl_options(-padx)}]
	    set minh [expr {$numBtns * [_getMaxHeight] \
		    + ($numBtns+1) * $itcl_options(-pady)}]
	}
	#
	# Remove the configure event bindings on the hull while we adjust the
	# width/height and re-position the buttons.  Once we're through, we'll
	# update and reinstall them.  This prevents double calls to position
	# the buttons.
	#
	set tags [bindtags $win]
	if {[set i [lsearch $tags bbox-config]] != -1} {
	    set tags [lreplace $tags $i $i]
	    bindtags $win $tags
	}
	$itcl_hull configure -width $minw -height $minh
	update idletasks
	_positionButtons
	bindtags $win [linsert $tags 0 bbox-config]
    }
}
    
# ------------------------------------------------------------------
# METHOD: _positionButtons
# 
# This method is responsible setting the width/height of all the 
# displayed buttons to the same value and for placing all the buttons
# in equidistant locations.
# ------------------------------------------------------------------
::itcl::body Buttonbox::_positionButtons {} {
    set bf $win
    set numBtns [llength $_displayList]
    # 
    # First, determine the common width and height for all the 
    # displayed buttons.
    #
    if {$numBtns > 0} {
	set bfWidth [winfo width $win]
	set bfHeight [winfo height $win]
	
	if {$bfWidth >= [winfo reqwidth $win]} {
	    set _btnWidth [_getMaxWidth] 
	    
	} else {
	    if {$itcl_options(-orient) == "horizontal"} {
		set _btnWidth [expr {$bfWidth / $numBtns}]
	    } else {
		set _btnWidth $bfWidth
	    }
	}	    
	if {$bfHeight >= [winfo reqheight $win]} {
	    set _btnHeight [_getMaxHeight]
	} else {
	    if {$itcl_options(-orient) == "vertical"} {
		set _btnHeight [expr {$bfHeight / $numBtns}]
	    } else {
		set _btnHeight $bfHeight
	    }
	}	    
    }
    #
    # Place the buttons at the proper locations.
    #
    if {$numBtns > 0} {
	if {$itcl_options(-orient) == "horizontal"} {
	    set leftover [expr {[winfo width $bf] \
		    - 2 * $itcl_options(-padx) - $_btnWidth * $numBtns}]
	    
	    if {$numBtns > 0} {
		set offset [expr {$leftover / ($numBtns + 1)}]
	    } else {
		set offset 0
	    }
	    if {$offset < 0} {set offset 0}
	    
	    set xDist [expr {$itcl_options(-padx) + $offset}]
	    set incrAmount [expr {$_btnWidth + $offset}]
	    foreach button $_displayList {
		place $button -anchor w \
			-x $xDist -rely .5 -y 0 -relx 0 \
			-width $_btnWidth -height $_btnHeight
		set xDist [expr {$xDist + $incrAmount}]
	    }
	    
	} else {
	    set leftover [expr {[winfo height $bf] \
		    - 2 * $itcl_options(-pady) - $_btnHeight * $numBtns}]
	    if {$numBtns > 0} {
		set offset [expr {$leftover / ($numBtns + 1)}]
	    } else {
		set offset 0
	    }
	    if {$offset < 0} {
	        set offset 0
	    }
	    set yDist [expr {$itcl_options(-pady) + $offset}]
	    set incrAmount [expr {$_btnHeight + $offset}]
	    foreach button $_displayList {
		place $button -anchor n \
			-y $yDist -relx .5 -x 0 -rely 0 \
			-width $_btnWidth -height $_btnHeight
		set yDist [expr {$yDist + $incrAmount}]
	    }
	}
    }
}

} ; # end ::itcl::widgets
