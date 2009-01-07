#
# Optionmenu
# ----------------------------------------------------------------------
# Implements an option menu widget with options to manage it. 
# An option menu displays a frame containing a label and a button.
# A pop-up menu will allow for the value of the button to change. 
#
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Optionmenu
# written by:
#  AUTHOR:  Alfredo Jahn             Phone: (214) 519-3545
#                                    Email: ajahn@spd.dsccc.com
#                                           alfredo@wn.com
#   Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: optionmenu.tcl,v 1.1.2.1 2009/01/07 13:19:44 wiede Exp $
# ======================================================================

#
# Default resources.
#

option add *Optionmenu.highlightThickness	1	widgetDefault
option add *Optionmenu.borderWidth		2	widgetDefault
option add *Optionmenu.labelPos			w	widgetDefault
option add *Optionmenu.labelMargin		2	widgetDefault
option add *Optionmenu.popupCursor		arrow	widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Optionmenu class.
# 
proc ::itcl::widgets::optionmenu {pathName args} {
    uplevel ::itcl::widgets::Optionmenu $pathName $args
}

# ------------------------------------------------------------------
#                            OPTONMENU
# ------------------------------------------------------------------
::itcl::extendedclass Optionmenu {
    inherit ::itcl::widgets::Labeledwidget
    
    component menuBtn
    component popupMenu

    option [list -clicktime clickTime ClickTime] -default 150 -configuremethod configClicktime
    option [list -command command Command] -default {} -configuremethod configCommand
    option [list -cyclicon cyclicOn CyclicOn] -default true -configuremethod configCyclicon
    option [list -width width Width] -default 0 -configuremethod configWidth
    option [list -font font Font] -default -Adobe-Helvetica-Bold-R-Normal--*-120-* -configuremethod configFont
    option [list -borderwidth borderWidth BorderWidth] -default 2 -configuremethod configBorderwidth
    option [list -highlightthickness highlightThickness HighlightThickness] -default 1 -configuremethod configCyclicon
    option [list -state state State] -default normal -configuremethod configState

    delegate option [list -popupcursor popupCursor Cursor] to popupMenu as -cursor
    protected variable _calcSize ""  ;# non-null => _calcSize pending

    private variable _postTime 0
    private variable _items {}       ;# List of popup menu entries
    private variable _numitems 0     ;# List of popup menu entries
    private variable _currentItem "" ;# Active menu selection

    constructor {args} {}
    destructor {}

    private method _buttonRelease {time} 
    private method _getNextItem {index} 
    private method _next {} 
    private method _postMenu {time} 
    private method _previous {} 
    private method _setItem {item} 
    private method _setSize {{when later}} 
    private method _setitems {items} ;# Set the list of menu entries

    protected method configClicktime {option value}
    protected method configCommand {option value}
    protected method configCyclicon {option value}
    protected method configWidth {option value}
    protected method configFont {option value}
    protected method configBorderwidth {option value}
    protected method configHighlightthickness {option value}
    protected method configState {option value}

    public method index {index} 
    public method delete {first {last {}}} 
    public method disable {index} 
    public method enable {args} 
    public method get {{first "current"} {last ""}} 
    public method insert {index string args} 
    public method popupMenu {args} 
    public method select {index} 
    public method sort {{mode "increasing"}} 
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Optionmenu::constructor {args} {
    global tcl_platform
    $win configure -highlightthickness 0
    setupcomponent menuBtn using menubutton $itcl_interior.menuBtn -relief raised -indicatoron on \
            -textvariable [itcl::scope _currentItem] -takefocus 1 \
            -menu $itcl_interior.menuBtn.menu

#    -activeborderwidth \

    keepcomponentoption menuBtn -activebackground \
         -activeforeground -background -borderwidth -cursor \
	 -disabledforeground -font -foreground -highlightcolor \
	 -highlightthickness -labelfont -popupcursor
    keepcomponentoption menuBtn -borderwidth
    pack $itcl_interior.menuBtn -fill x
    pack propagate $itcl_interior no
    setupcomponent popupMenu using menu $itcl_interior.menuBtn.menu -tearoff no
    keepcomponentoption popupMenu -activebackground -activeborderwidth \
         -activeforeground -background -borderwidth -cursor \
	 -disabledforeground -font -foreground -highlightcolor \
	 -highlightthickness -labelfont -popupcursor
# FIXME	ignore -tearoff
    keepcomponentoption popupMenu -activeborderwidth -borderwidth

    #
    # Bind to button release for all components.
    #
    bind $menuBtn <ButtonPress-1> \
	    "[itcl::code $this _postMenu %t]; break"
    bind $menuBtn <KeyPress-space> \
	    "[itcl::code $this _postMenu %t]; break"
    bind $popupMenu <ButtonRelease-1> \
	    [itcl::code $this _buttonRelease %t]

    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
}

# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Optionmenu::destructor {} {
    if {$_calcSize != ""} {
        after cancel $_calcSize
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION -clicktime
#
# Interval time (in msec) used to determine that a single mouse 
# click has occurred. Used to post menu on a quick mouse click.
# **WARNING** changing this value may cause the sigle-click 
# functionality to not work properly!
# ------------------------------------------------------------------
::itcl::body Optionmenu::configClicktime {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION -command
#
# Specifies a command to be evaluated upon change in option menu.
# ------------------------------------------------------------------
::itcl::body Optionmenu::configCommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION -cyclicon
#
# Turns on/off the 3rd mouse button capability. This feature
# allows the right mouse button to cycle through the popup 
# menu list without poping it up. <shift>M3 cycles through
# the menu in reverse order.
# ------------------------------------------------------------------
::itcl::body Optionmenu::configCyclicon {option value} {
    if {$value} {
    	bind $menuBtn <3> [itcl::code $this _next]
    	bind $menuBtn <Shift-3> [itcl::code $this _previous]
        bind $menuBtn <KeyPress-Down> [itcl::code $this _next]
        bind $menuBtn <KeyPress-Up> [itcl::code $this _previous]
    } else {
    	bind $menuBtn <3> break
    	bind $menuBtn <Shift-3> break
        bind $menuBtn <KeyPress-Down> break
        bind $menuBtn <KeyPress-Up> break
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION -width
#
# Allows the menu label width to be set to a fixed size
# ------------------------------------------------------------------
::itcl::body Optionmenu::configWidth {option value} {
    set itcl_options($option) $value
    _setSize
}

# ------------------------------------------------------------------
# OPTION -font
#
# Change all fonts for this widget. Also re-calculate height based
# on font size (used to line up menu items over menu button label).
# ------------------------------------------------------------------
::itcl::body Optionmenu::configFont {option value} {
    set itcl_options($option) $value
    _setSize
}

# ------------------------------------------------------------------
# OPTION -borderwidth
#
# Change borderwidth for this widget. Also re-calculate height based
# on font size (used to line up menu items over menu button label).
# ------------------------------------------------------------------
::itcl::body Optionmenu::configBorderwidth {option value} {
    set itcl_options($option) $value
    _setSize
}

# ------------------------------------------------------------------
# OPTION -highlightthickness
#
# Change highlightthickness for this widget. Also re-calculate
# height based on font size (used to line up menu items over
# menu button label).
# ------------------------------------------------------------------
::itcl::body Optionmenu::configHighlightthickness {option value} {
    set itcl_options($option) $value
    _setSize
}

# ------------------------------------------------------------------
# OPTION -state
#
# Specified one of two states for the Optionmenu: normal, or
# disabled.  If the Optionmenu is disabled, then option menu
# selection is ignored.
# ------------------------------------------------------------------
::itcl::body Optionmenu::configState {option value} {
    switch $value {
    	normal {
            $menuBtn config -state normal
            $label config -fg $itcl_options(-foreground)
    	} 
    	disabled {
            $menuBtn config -state disabled
            $label config -fg $itcl_options(-disabledforeground)
    	}
    	default {
    	    error "bad state option \"$value\":\
		    should be disabled or normal"
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
# Return the numerical index corresponding to index.
# ------------------------------------------------------------------
::itcl::body Optionmenu::index {index} {
    if {[regexp {(^[0-9]+$)} $index]} {
	set idx [$popupMenu index $index]
	if {$idx eq "none"} {
	    return 0
	}
	return [expr {$index > $idx ? $_numitems : $idx}]
	
    } elseif {$index eq "end"} {
	return [expr {$_numitems - 1}]
	
    } elseif {$index eq "select"} {
	return [lsearch $_items $_currentItem]
    }
    set numValue [lsearch -glob $_items $index]
    if {$numValue == -1} {
        error "bad Optionmenu index \"$index\""
    }
    return $numValue
}

# ------------------------------------------------------------------
# METHOD: delete first ?last?
#
# Remove an item (or range of items) from the popup menu. 
# ------------------------------------------------------------------
::itcl::body Optionmenu::delete {first {last {}}} {
    set first [index $first]
    set last [expr {$last != {} ? [index $last] : $first}]    
    set nextAvail $_currentItem
    #
    # If current item is in delete range point to next available.
    #
    if {$_numitems > 1 &&
	([lsearch -exact [lrange $_items $first $last] [get]] != -1)} {
	set nextAvail [_getNextItem $last]
    }
    _setitems [lreplace $_items $first $last]
    
    #
    # Make sure "nextAvail" is still in the list.
    #
    set index [lsearch -exact $_items $nextAvail]
    _setItem [expr {$index != -1 ? $nextAvail : ""}]
}

# ------------------------------------------------------------------
# METHOD: disable index
#
# Disable a menu item in the option menu.  This will prevent the user
# from being able to select this item from the menu.  This only effects
# the state of the item in the menu, in other words, should the item
# be the currently selected item, the user is responsible for 
# determining this condition and taking appropriate action.
# ------------------------------------------------------------------
::itcl::body Optionmenu::disable {index} {
    set index [index $index]
    $popupMenu entryconfigure $index -state disabled
}

# ------------------------------------------------------------------
# METHOD: enable index
#
# Enable a menu item in the option menu.  This will allow the user
# to select this item from the menu.  
# ------------------------------------------------------------------
::itcl::body Optionmenu::enable {index} {
    set index [index $index]
    $popupMenu entryconfigure $index -state normal
}

# ------------------------------------------------------------------
# METHOD: get
#
# Returns the current menu item.
# ------------------------------------------------------------------
::itcl::body Optionmenu::get {{first "current"} {last ""}} {
    if {$first eq "current"} {
        return $_currentItem
    }
    set first [index $first]
    if {$last eq ""} {
        return [$popupMenu entrycget $first -label]
    }
    if {$last eq "end"} {
        set last [$popupMenu index end]
    } else {
        set last [index $last]
    }
    set rval ""
    while {$first <= $last} {
        lappend rval [$popupMenu entrycget $first -label]
        incr first
    }
    return $rval
}

# ------------------------------------------------------------------
# METHOD: insert index string ?string?
#
# Insert an item in the popup menu.
# ------------------------------------------------------------------
::itcl::body Optionmenu::insert {index string args} {
    if {$index eq "end"} {
	set index $_numitems
    } else {
	set index [index $index]
    }
    set args [linsert $args 0 $string]
    _setitems [eval linsert {$_items} $index $args]
    return ""
}

# ------------------------------------------------------------------
# METHOD: select index
#
# Select an item from the popup menu to display on the menu label
# button. 
# ------------------------------------------------------------------
::itcl::body Optionmenu::select {index} {
    set index [index $index]
    if {$index > ($_numitems - 1)} {
      incr index -1 
    }
    _setItem [lindex $_items $index]
}

# ------------------------------------------------------------------
# METHOD: popupMenu
#
# Evaluates the specified args against the popup menu component
# and returns the result.
# ------------------------------------------------------------------
::itcl::body Optionmenu::popupMenu {args} {
    return [uplevel 0 $popupMenu $args]	
}

# ------------------------------------------------------------------
# METHOD: sort mode
#
# Sort the current menu in either "ascending" or "descending" order.
# ------------------------------------------------------------------
::itcl::body Optionmenu::sort {{mode "increasing"}} {
    switch $mode {
    ascending -
    increasing {
        _setitems [lsort -increasing $_items]
      }
    descending -
    decreasing {
        _setitems [lsort -decreasing $_items]
      }
    default {
        error "bad sort argument \"$mode\": should be ascending,\
	        descending, increasing, or decreasing"
      }
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _buttonRelease
#
# Display the popup menu. Menu position is calculated.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_buttonRelease {time} {
    if {(abs([expr $_postTime - $time])) <= $itcl_options(-clicktime)} {
        return -code break
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _getNextItem index
#
# Allows either a string or index number to be passed in, and returns
# the next item in the list in string format. Wrap around is automatic.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_getNextItem {index} {
    if {[incr index] >= $_numitems} {
	set index 0   ;# wrap around
    }
    return [lindex $_items $index]
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _next
#
# Sets the current option label to next item in list if that item is
# not disbaled.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_next {} {
    if {$itcl_options(-state) != "normal"} {
        return
    }
    set i [lsearch -exact $_items $_currentItem]
    for {set cnt 0} {$cnt < $_numitems} {incr cnt} {
        if {[incr i] >= $_numitems} {
            set i 0
        }
        if {[$popupMenu entrycget $i -state] ne "disabled"} {
            _setItem [lindex $_items $i]
            break
        }
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _previous
#
# Sets the current option label to previous item in list if that 
# item is not disbaled.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_previous {} {
    if {$itcl_options(-state) ne "normal"} {
        return
    }
    set i [lsearch -exact $_items $_currentItem]
    for {set cnt 0} {$cnt < $_numitems} {incr cnt} {
	set i [expr {$i - 1}]
    
	if {$i < 0} {
	    set i [expr {$_numitems - 1}]
	}
	if {[$popupMenu entrycget $i -state] ne "disabled"} {
	    _setItem [lindex $_items $i]
	    break
	}
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _postMenu time
#
# Display the popup menu. Menu position is calculated.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_postMenu {time} {
    #
    # Don't bother to post if menu is empty.
    #
    if {[llength $_items] > 0 && ($itcl_options(-state) eq "normal")} {
        set _postTime $time
        set itemIndex [lsearch -exact $_items $_currentItem]
        set margin [expr {$itcl_options(-borderwidth) \
            + $itcl_options(-highlightthickness)}]
        set x [expr {[winfo rootx $menuBtn] + $margin}]
        set y [expr {[winfo rooty $menuBtn] \
            - [$popupMenu yposition $itemIndex] + $margin}]
        tk_popup $popupMenu $x $y
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _setItem
#
# Set the menu button label to item, then dismiss the popup menu.
# Also check if item has been changed. If so, also call user-supplied
# command.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_setItem {item} {
    if {$_currentItem != $item} {
        set _currentItem $item
	if {[winfo ismapped $win]} {
	    uplevel #0 $itcl_options(-command)
	}
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _setitems items
#
# Create a list of items available on the menu. Used to create the
# popup menu.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_setitems {items_} {
    #
    # Delete the old menu entries, and set the new list of
    # menu entries to those specified in "items_".
    #
    $popupMenu delete 0 last
    set _items ""
    set _numitems [llength $items_]
    #
    # Clear the menu button label.
    #
    if {$_numitems == 0} {
	_setItem ""
	return
    }
    set savedCurrentItem $_currentItem
    foreach opt $items_ {
        lappend _items $opt
        $popupMenu add command -label $opt \
            -command [itcl::code $this _setItem $opt]
    }
    set first [lindex $_items 0]
    #
    # Make sure "savedCurrentItem" is still in the list.
    #
    if {$first ne ""} {
        set i [lsearch -exact $_items $savedCurrentItem]
	#-------------------------------------------------------------
	# BEGIN BUG FIX: csmith (Chad Smith: csmith@adc.com), 11/18/99
	#-------------------------------------------------------------
	# The previous code fragment:
	#   <select [expr {$i != -1 ? $savedCurrentItem : $first}]>
	# is faulty because of exponential numbers.  For example,
	# 2e-4 is numerically equal to 2e-04, but the string representation
	# is of course different.  As a result, the select invocation
	# fails, and an error message is printed.
	#-------------------------------------------------------------
	if {$i != -1} {
	  select $savedCurrentItem
	} else {
	  select $first
	}
	#-------------------------------------------------------------
	# END BUG FIX
	#-------------------------------------------------------------
    } else {
	_setItem ""
    }
    _setSize
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _setSize ?when?
#
# Set the size of the option menu.  If "when" is "now", the change 
# is applied immediately.  If it is "later" or it is not specified, 
# then the change is applied later, when the application is idle.
# ------------------------------------------------------------------
::itcl::body Optionmenu::_setSize {{when later}} {
    if {$when eq "later"} {
	if {$_calcSize == ""} {
	    set _calcSize [after idle [itcl::code $this _setSize now]]
	}
	return
    }
    set margin [expr {2*($itcl_options(-borderwidth) \
        + $itcl_options(-highlightthickness))}]
    if {$itcl_options(-width) ne "0"} {
    	set width $itcl_options(-width)
    } else {
	set width [expr {[winfo reqwidth $popupMenu]+$margin+20}]
    }
    set height [winfo reqheight $menuBtn]
    $lwchildsite configure -width $width -height $height
    set _calcSize ""
}

} ; # end ::itcl::widgets
