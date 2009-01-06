#
# Toolbar
# ----------------------------------------------------------------------
# The Toolbar command creates a new window (given by the pathName 
# argument) and makes it into a Tool Bar widget. Additional options, 
# described above may be specified on the command line or in the 
# option database to configure aspects of the Toolbar such as its 
# colors, font, and orientation. The Toolbar command returns its 
# pathName argument. At the time this command is invoked, there 
# must not exist a window named pathName, but pathName's parent 
# must exist.
# 
# A Toolbar is a widget that displays a collection of widgets arranged 
# either in a row or a column (depending on the value of the -orient 
# option). This collection of widgets is usually for user convenience 
# to give access to a set of commands or settings. Any widget may be 
# placed on a Toolbar. However, command or value-oriented widgets (such 
# as button, radiobutton, etc.) are usually the most useful kind of 
# widgets to appear on a Toolbar.
#
# WISH LIST: 
#   This section lists possible future enhancements.  
#
#	Toggle between text and image/bitmap so that the toolbar could
#     display either all text or all image/bitmaps.
#   Implementation of the -toolbarfile option that allows toolbar
#     add commands to be read in from a file.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Toolbar
# written by:
#  AUTHOR: Bill W. Scott                 EMAIL: bscott@spd.dsccc.com
#   Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: toolbar.tcl,v 1.1.2.1 2009/01/06 21:50:16 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Toolbar*padX 5 widgetDefault
option add *Toolbar*padY 5 widgetDefault
option add *Toolbar*orient horizontal widgetDefault
option add *Toolbar*highlightThickness 0 widgetDefault
option add *Toolbar*indicatorOn false widgetDefault
option add *Toolbar*selectColor [. cget -bg] widgetDefault

    keep -activebackground -activeforeground -background -balloonbackground \
	 -balloondelay1 -balloondelay2 -balloonfont -balloonforeground \
	 -borderwidth -cursor -disabledforeground -font -foreground \
	 -highlightbackground -highlightcolor -highlightthickness \
	 -insertbackground -insertforeground -selectbackground \
	 -selectborderwidth -selectcolor -selectforeground -troughcolor

namespace eval ::itcl::widgets {

#
# Provide a lowercase access method for the Toolbar class
#
proc ::itcl::widgets::toolbar {pathName args} {
    uplevel ::itcl::widgets::Toolbar $pathName $args
}

# ------------------------------------------------------------------
#                            TOOLBAR
# ------------------------------------------------------------------
::itcl::extendedclass ::itcl::widgets::Toolbar {
    component itcl_hull
    component itcl_interior
    
    option [list -balloonbackground \
	    balloonBackground BalloonBackground] -default yellow -configuremethod configBalloonbackground
    option [list -balloonforeground \
	    balloonForeground BalloonForeground] -default black -configuremethod configBalloonforeground
    option [list -balloonfont balloonFont BalloonFont] -default 6x10 -configuremethod configBalloonfont
    option [list -balloondelay1 \
	    balloonDelay1 BalloonDelay1] -default 1000
    option [list -balloondelay2 \
	    balloonDelay2 BalloonDelay2] -default 200
    option [list -helpvariable helpVariable HelpVariable] -default {} 
    option [list -orient orient Orient] -default "horizontal" -configuremethod configOrient
    
    #
    # The following options implement propogated configurations to
    # any widget that might be added to us. The problem is this is
    # not deterministic as someone might add a new kind of widget with
    # and option like -armbackground, so we would not be aware of
    # this kind of option. Anyway we support as many of the obvious
    # ones that we can. They can always configure them with itemconfigures.
    #
    option [list -activebackground activeBackground Foreground] -default #c3c3c3
    option [list -activeforeground activeForeground Background] -default Black 
    option [list -background background Background] -default #d9d9d9 
    option [list -borderwidth borderWidth BorderWidth] -default 2 
    option [list -cursor cursor Cursor] -default {}
    option [list -disabledforeground \
	    disabledForeground DisabledForeground] -default #a3a3a3 
    option [list -font \
	    font Font] -default "-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-*" 
    option [list -foreground foreground Foreground] -default #000000000000 
    option [list -highlightbackground \
	    highlightBackground HighlightBackground] -default #d9d9d9 
    option [list -highlightcolor highlightColor HighlightColor] -default Black 
    option [list -highlightthickness \
	    highlightThickness HighlightThickness] -default 0 
    option [list -insertforeground insertForeground Background] -default #c3c3c3 
    option [list -insertbackground insertBackground Foreground] -default Black 
    option [list -selectbackground selectBackground Foreground] -default #c3c3c3 
    option [list -selectborderwidth selectBorderWidth BorderWidth] -default {} 
    option [list -selectcolor selectColor Background] -default #b03060 
    option [list -selectforeground selectForeground Background] -default Black 
    option [list -state state State] -default normal 
    option [list -troughcolor troughColor Background] -default #c3c3c3 
    
    private variable _balloonTimer 0
    private variable _balloonAfterID 0
    private variable _balloonClick false
    
    private variable _interior {}
    private variable _initialMapping 1   ;# Is this the first mapping?
    private variable _toolList {}        ;# List of all widgets on toolbar
    private variable _opts               ;# New options for child widgets
    private variable _currHelpWidget {}  ;# Widget currently displaying help for
    private variable _hintWindow {}      ;# Balloon help bubble.
    
    # list of options we want to propogate to widgets added to toolbar.
    private common _optionList {
	-activebackground \
	-activeforeground \
	-background \
	-borderwidth \
	-cursor \
	-disabledforeground \
	-font \
	-foreground \
	-highlightbackground \
	-highlightcolor \
	-highlightthickness \
	-insertbackground \
	-insertforeground \
	-selectbackground \
	-selectborderwidth \
	-selectcolor \
	-selectforeground \
	-state \
	-troughcolor \
    }

    constructor {args} {}
    destructor {}
    
    private method _deleteWidgets {index1 index2} 
    private method _addWidget {widgetCommand name args}
    private method _index {toolList index} 
    private method _getAttachedOption {optionListName widget args retValue} 
    private method _setAttachedOption {optionListName widget option args} 
    private method _packToolbar {} 
    
    protected method configBalloonbackground {option value}
    protected method configBalloonforeground {option value}
    protected method configBalloonfont {option value}
    protected method configOrient {option value}

    public method add {widgetCommand name args} 
    public method delete {args} 
    public method index {index} 
    public method insert {beforeIndex widgetCommand name args} 
    public method itemcget {index args} 
    public method itemconfigure {index args} 
    public method _resetBalloonTimer {}
    public method _startBalloonDelay {window}
    public method _stopBalloonDelay {window balloonClick}
    public method hideHelp {} 
    public method showHelp {window} 
    public method showBalloon {window} 
    public method hideBalloon {} 
    
}

# ------------------------------------------------------------------
#                            CONSTRUCTOR 
# ------------------------------------------------------------------
::itcl::body Toolbar::constructor {args} {
    createhull frame $this -class [info class]
    $itcl_hull configure -borderwidth 0
    set _interior $itcl_interior

    #
    # Handle configs
    #
    uplevel 0 itcl_initoptions $args

    # build balloon help window
    set _hintWindow [toplevel $itcl_hull.balloonHintWindow]
    wm withdraw $_hintWindow
    label $_hintWindow.label \
	-foreground $itcl_options(-balloonforeground) \
	-background $itcl_options(-balloonbackground) \
	-font $itcl_options(-balloonfont) \
	-relief raised \
	-borderwidth 1
    pack $_hintWindow.label
    
    # ... Attach help handler to this widget
    bind toolbar-help-$itcl_hull \
	    <Enter> "+[itcl::code $this showHelp %W]"
    bind toolbar-help-$itcl_hull \
	    <Leave> "+[itcl::code $this hideHelp]"
    
    # ... Set up Microsoft style balloon help display.
    set _balloonTimer $itcl_options(-balloondelay1)
    bind $_interior \
	    <Leave> "+[itcl::code $this _resetBalloonTimer]"
    bind toolbar-balloon-$itcl_hull \
	    <Enter> "+[itcl::code $this _startBalloonDelay %W]"
    bind toolbar-balloon-$itcl_hull \
	    <Leave> "+[itcl::code $this _stopBalloonDelay %W false]"
    bind toolbar-balloon-$itcl_hull \
	    <Button-1> "+[itcl::code $this _stopBalloonDelay %W true]"
}

# ------------------------------------------------------------------
#                           DESTURCTOR
# ------------------------------------------------------------------
::itcl::body Toolbar::destructor {} {
    if {$_balloonAfterID != 0} {after cancel $_balloonAfterID}
}

# ------------------------------------------------------------------
#                            OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION -balloonbackground
# ------------------------------------------------------------------
::itcl::body Toolbar::configBalloonbackground {option value} {
    if { $_hintWindow ne {}} {
	if {$value ne {}} {
	    $_hintWindow.label configure -background $value
	}
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION -balloonforeground
# ------------------------------------------------------------------
::itcl::body Toolbar::configBalloonforeground {option value} {
    if {$_hintWindow ne {}} {
	if {$value ne {}} {
	    $_hintWindow.label configure -foreground $value
	}
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION -balloonfont
# ------------------------------------------------------------------
::itcl::body Toolbar::configBalloonfont {option value} {
    if {$_hintWindow ne {}} {
	if {$value ne {}} {
	    $_hintWindow.label configure -font $value
	}
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -orient
#
# Position buttons either horizontally or vertically.
# ------------------------------------------------------------------
::itcl::body Toolbar::configOrient {option value} {
    switch $ivalue {
    "horizontal" -
    "vertical" {
        _packToolbar
      }
    default {
        error "Invalid orientation. Must be either \
		horizontal or vertical"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------
    
# -------------------------------------------------------------
# METHOD: add widgetCommand name ?option value?
#
# Adds a widget with the command widgetCommand whose name is 
# name to the Toolbar.   If widgetCommand is radiobutton 
# or checkbutton, its packing is slightly padded to match the 
# geometry of button widgets.
# -------------------------------------------------------------
::itcl::body Toolbar::add {widgetCommand name args} {
    uplevel 0 "_addWidget $widgetCommand $name $args"
    lappend _toolList [set $name]
    if {($widgetCommand eq "radiobutton") || \
	    ($widgetCommand eq "checkbutton") } {
	set iPad 1
    } else {
	set iPad 0
    }
    # repack the tool bar
    _packToolbar
    return [set $name]
}

# -------------------------------------------------------------
#
# METHOD: delete index ?index2?
#
# This command deletes all components between index and 
# index2 inclusive. If index2 is omitted then it defaults 
# to index. Returns an empty string
#
# -------------------------------------------------------------
::itcl::body Toolbar::delete {args} {
    # empty toolbar
    if {$_toolList eq {}} {
	error "can't delete widget, no widgets in the Toolbar \
		\"$win\""
    }
    set len [llength $args]
    switch -- $len {
    1 {
        set fromWidget [_index $_toolList [lindex $args 0]]
        if {($fromWidget < 0) || ($fromWidget >= [llength $_toolList])} {
	    error "bad Toolbar widget index in delete method: \
	      should be between 0 and [expr {[llength $_toolList] - 1} ]"
        }
        set toWidget $fromWidget
        _deleteWidgets $fromWidget $toWidget
      }
    2 {
        set fromWidget [_index $_toolList [lindex $args 0]]
        if {($fromWidget < 0) || ($fromWidget >= [llength $_toolList])} {
	    error "bad Toolbar widget index1 in delete method: \
	      should be between 0 and [expr {[llength $_toolList] - 1} ]"
        }
        set toWidget [_index $_toolList [lindex $args 1]]
        if {($toWidget < 0) || ($toWidget >= [llength $_toolList])} {
	    error "bad Toolbar widget index2 in delete method: \
	      should be between 0 and [expr {[llength $_toolList] - 1} ]"
        }
        if { $fromWidget > $toWidget } {
	    error "bad Toolbar widget index1 in delete method: \
		    index1 is greater than index2"
        }
        _deleteWidgets $fromWidget $toWidget
      }
    default {
        # ... too few/many parameters passed
        error "wrong # args: should be \
	    \"$win delete index1 ?index2?\""
      }
    }
    return {}
}


# -------------------------------------------------------------
#
# METHOD: index index 
#
# Returns the widget's numerical index for the entry corresponding 
# to index. If index is not found, -1 is returned
#
# -------------------------------------------------------------
::itcl::body Toolbar::index {index} {
    return [_index $_toolList $index]
}

# -------------------------------------------------------------
#
# METHOD: insert beforeIndex widgetCommand name ?option value?
#
# Insert a new component named name with the command 
# widgetCommand before the com ponent specified by beforeIndex. 
# If widgetCommand is radiobutton or checkbutton, its packing 
# is slightly padded to match the geometry of button widgets.
#
# -------------------------------------------------------------
::itcl::body Toolbar::insert {beforeIndex widgetCommand name args} {
    set beforeIndex [_index $_toolList $beforeIndex]
    if {($beforeIndex < 0) || ($beforeIndex > [llength $_toolList])} {
	error "bad toolbar entry index $beforeIndex"
    }
    eval "_addWidget $widgetCommand $name $args"
    # linsert into list
    set _toolList [linsert $_toolList $beforeIndex [set $name]]
    # repack the tool bar
    _packToolbar
    return [set $name]
}

# ----------------------------------------------------------------------
# METHOD: itemcget index ?option? 
#
# Returns the value for the option setting of the widget at index $index.
# index can be numeric or widget name
#
# ----------------------------------------------------------------------
::itcl::body Toolbar::itemcget { index args} {
    return [lindex [eval itemconfigure $index $args] 4]

# -------------------------------------------------------------
#
# METHOD: itemconfigure index ?option? ?value? ?option value...?
#
# Query or modify the configuration options of the widget of 
# the Toolbar specified by index. If no option is specified, 
# returns a list describing all of the available options for 
# index (see Tk_ConfigureInfo for information on the format 
# of this list). If option is specified with no value, then 
# the command returns a list describing the one named option 
# (this list will be identical to the corresponding sublist 
# of the value returned if no option is specified). If one 
# or more option-value pairs are specified, then the command 
# modifies the given widget option(s) to have the given 
# value(s); in this case the command returns an empty string. 
# The component type of index determines the valid available options.
#
# -------------------------------------------------------------
::itcl::body Toolbar::itemconfigure { index args } {
    # Get a numeric index.
    set index [_index $_toolList $index]
    # Get the tool path
    set toolPath [lindex $_toolList $index]
    set len [llength $args]
    switch $len {
    0 {
        # show all options
        # ''''''''''''''''
        # support display of -helpstr and -balloonstr configs
        set optList [$toolPath configure]
        ## @@@ might want to use _getAttachedOption instead...
        if {[info exists _opts($toolPath,-helpstr)]} {
	    set value $_opts($toolPath,-helpstr)
        } else {
	    set value {}
        }
        lappend optList [list -helpstr helpStr HelpStr {} $value]
        if {[info exists _opts($toolPath,-balloonstr)]} {
	    set value $_opts($toolPath,-balloonstr)
        } else {
	    set value {}
        }
        lappend optList [list -balloonstr balloonStr BalloonStr {} $value]
        return $optList
      }
    1 {
        # show only option specified
        # ''''''''''''''''''''''''''
        # did we satisfy the option get request?
        if {[regexp -- {-helpstr} $args]} {
	    if {[info exists _opts($toolPath,-helpstr)]} {
	        set value $_opts($toolPath,-helpstr)
	    } else {
	        set value {}
	    }
	    return [list -helpstr helpStr HelpStr {} $value]
        } elseif {[regexp -- {-balloonstr} $args]} {
	    if {[info exists _opts($toolPath,-balloonstr)]} {
	        set value $_opts($toolPath,-balloonstr)
	    } else {
	        set value {}
	    }
	    return [list -balloonstr balloonStr BalloonStr {} $value]
        } else {
	    return [eval $toolPath configure $args]
        }
      }
    default {
        # ... do a normal configure
        # first screen for all our child options we are adding
        _setAttachedOption \
	    _opts \
	    $toolPath \
	    "-helpstr" \
	    $args
        _setAttachedOption \
	    _opts \
	    $toolPath \
	    "-balloonstr" \
	    $args 
        # with a clean args list do a configure
        # if the stripping process brought us down to no options
        # to set, then forget the configure of widget.
        if {[llength $args] != 0} {
	    return [eval $toolPath configure $args]
        } else {
	    return ""
        }
      }
    }
}

# -------------------------------------------------------------
#
# METHOD: _resetBalloonDelay1 
#
# Sets the delay that will occur before a balloon could be popped
# up to balloonDelay1
#
# -------------------------------------------------------------
::itcl::body Toolbar::_resetBalloonTimer {} {
    set _balloonTimer $itcl_options(-balloondelay1)
    # reset the <1> longer delay
    set _balloonClick false
}

# -------------------------------------------------------------
#
# METHOD: _startBalloonDelay 
#
# Starts waiting to pop up a balloon id
#
# -------------------------------------------------------------
::itcl::body Toolbar::_startBalloonDelay {window} {
    if {$_balloonAfterID != 0} {
	after cancel $_balloonAfterID
    }
    set _balloonAfterID [after $_balloonTimer [itcl::code $this showBalloon $window]]
}

# -------------------------------------------------------------
#
# METHOD: _stopBalloonDelay  
#
# This method will stop the timer for a balloon popup if one is
# in progress. If however there is already a balloon window up
# it will hide the balloon window and set timing to delay 2 stage.
#
# -------------------------------------------------------------
::itcl::body Toolbar::_stopBalloonDelay {window balloonClick} {
    # If <1> then got a click cancel
    if {$balloonClick} {
	set _balloonClick true
    }
    if {$_balloonAfterID != 0} {
	after cancel $_balloonAfterID
	set _balloonAfterID 0
    } else {
	hideBalloon
	# If this was cancelled with a <1> use longer delay.
	if {$_balloonClick} {
	    set _balloonTimer $itcl_options(-balloondelay1)
	} else {
	    set _balloonTimer $itcl_options(-balloondelay2)
	}
    }
}

# -------------------------------------------------------------
# PRIVATE METHOD: _addWidget
#
# widgetCommand : command to invoke to create the added widget
# name          : name of the new widget to add
# args          : options for the widget create command
#
# Looks for -helpstr, -balloonstr and grabs them, strips from
# args list. Then tries to add a component and keeps based
# on known type. If it fails, it tries to clean up. Then it
# binds handlers for helpstatus and balloon help.
#
# Returns the path of the widget added.
#
# -------------------------------------------------------------
::itcl::body Toolbar::_addWidget {widgetCommand name args} {
    # ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
    # Add the widget to the tool bar
    # '''''''''''''''''''''''''''''''''''''''''''''''''''''
    # ... Strip out and save the -helpstr, -balloonstr options from args
    #     and save it in _opts
    _setAttachedOption \
        _opts \
        $_interior.$name \
        -helpstr \
        $args 
    _setAttachedOption \
        _opts \
        $_interior.$name \
        -balloonstr \
        $args
    # ... Add the new widget as a component (catch an error if occurs)
    set createFailed [catch { 
	::itcl::addcomponent $this $name
	setupcomponent $name uisng $widgetCommand $_interior.$name {*}$args
    } errMsg]
    # ... Clean up if the create failed, and exit.
    #     The _opts list if it has -helpstr, -balloonstr just entered for
    #     this, it must be cleaned up.
    if {$createFailed} {
	# clean up
	if {![catch {set _opts($_interior.$name,-helpstr)}]} {
	    set lastIndex [\
		    expr {[llength \
		    $_opts($_interior.$name,-helpstr) ]-1}]
	    lreplace $_opts($_interior.$name,-helpstr) \
		    $lastIndex $lastIndex ""
	}
	if {![catch {set _opts($_interior.$name,-balloonstr)}]} {
	    set lastIndex [\
		    expr {[llength \
		    $_opts($_interior.$name,-balloonstr) ]-1}]
	    lreplace $_opts($_interior.$name,-balloonstr) \
		    $lastIndex $lastIndex ""
	}
	error $errMsg
    }
    # ... Add in dynamic options that apply from the _optionList
    foreach optionSet [[set $name] configure] {
	set option [lindex $optionSet 0]
	if { [lsearch $_optionList $option] != -1 } {
	    itcl_options add $name.$option
	}
    }
    set my_comp [set $name]
    bindtags $my_comp \
	    [linsert [bindtags $my_comp] end \
	    toolbar-help-$itcl_hull]
    bindtags $my_comp \
	    [linsert [bindtags $my_comp] end \
	    toolbar-balloon-$itcl_hull]
    
    return $my_comp
}

# -------------------------------------------------------------
#
# PRIVATE METHOD: _deleteWidgets
#
# deletes widget range by numerical index numbers.
#
# -------------------------------------------------------------
::itcl::body Toolbar::_deleteWidgets {index1 index2} {
    for {set index $index1} {$index <= $index2} {incr index} {
	# kill the widget
	set my_comp [lindex $_toolList $index]
	destroy $my_comp
    }
    # physically remove the page
    set _toolList [lreplace $_toolList $index1 $index2]
    
}

# -------------------------------------------------------------
# PRIVATE METHOD: _index
#
# toolList : list of widget names to search thru if index 
#            is non-numeric
# index    : either number, 'end', 'last', or pattern
#
# _index takes takes the value $index converts it to
# a numeric identifier. If the value is not already
# an integer it looks it up in the $toolList array.
# If it fails it returns -1
#
# -------------------------------------------------------------
::itcl::body Toolbar::_index {toolList index} {
    switch -- $index {
    end -
    last {
        set number [expr {[llength $toolList] -1}]
      }
    default {
        # is it a number already? Then just use the number
        if {[regexp {^[0-9]+$} $index]} {
	    set number $index
	    # check bounds
	    if {($number < 0) || ($number >= [llength $toolList])} {
	        set number -1
	    }
	    # otherwise it is a widget name
        } else {
	    if {[catch {set $index}]} {
	        set number -1
	    } else {
	        set number [lsearch -exact $toolList \
		        [set $index]]
	    }
        }
      }
    }
    return $number
}
    
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# STATUS HELP for linking to helpVariable
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# -------------------------------------------------------------
# 
# PUBLIC METHOD: hideHelp
#
# Bound to the <Leave> event on a toolbar widget. This clears the
# status widget help area and resets the help entry.
#
# -------------------------------------------------------------
::itcl::body Toolbar::hideHelp {} {
    if {$itcl_options(-helpvariable) ne {}} {
        upvar #0 $itcl_options(-helpvariable) helpvar
	set helpvar {}
    }
    set _currHelpWidget {}
}

# -------------------------------------------------------------
# 
# PUBLIC METHOD: showHelp
#
# Bound to the <Motion> event on a tool bar widget. This puts the
# help string associated with the tool bar widget into the 
# status widget help area. If no help exists for the current
# entry, the status widget is cleared.
#
# -------------------------------------------------------------
::itcl::body Toolbar::showHelp {window} {
    set widgetPath $window
    # already on this item?
    if {$window == $_currHelpWidget} {
	return
    }
    set _currHelpWidget $window
    # Do we have a helpvariable set on the toolbar?
    if {$itcl_options(-helpvariable) ne {}} {
        upvar #0 $itcl_options(-helpvariable) helpvar
	# is the -helpstr set for this widget?
	set args "-helpstr"
	if {[_getAttachedOption _opts \
		$window args value]} {
	    set helpvar $value.
	} else {
	    set helpvar {}
	}
    }
}

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# BALLOON HELP for show/hide of hint window
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# -------------------------------------------------------------
# 
# PUBLIC METHOD: showBalloon
#
# -------------------------------------------------------------
::itcl::body Toolbar::showBalloon {window} {
    set _balloonClick false
    set _balloonAfterID 0
    # Are we still inside the window?
    set mouseWindow \
	    [winfo containing [winfo pointerx .] [winfo pointery .]]
    if {[string match $window* $mouseWindow]} {
	# set up the balloonString
	set args "-balloonstr"
	if {[_getAttachedOption _opts \
		$window args hintStr]} {
	    # configure the balloon help
	    $_hintWindow.label configure -text $hintStr		
	    # Coordinates of the balloon
	    set balloonLeft \
		    [expr {[winfo rootx $window] + round(([winfo width $window]/2.0))}]
	    set balloonTop \
		    [expr {[winfo rooty $window] + [winfo height $window]}]
	    # put up balloon window
	    wm overrideredirect $_hintWindow 0
	    wm geometry $_hintWindow "+$balloonLeft+$balloonTop"
	    wm overrideredirect $_hintWindow 1
	    wm deiconify $_hintWindow
	    raise $_hintWindow
	} else {
	    #NO BALLOON HELP AVAILABLE
	}
    } else {
	#NOT IN BUTTON
    }
}

# -------------------------------------------------------------
# 
# PUBLIC METHOD: hideBalloon
#
# -------------------------------------------------------------
::itcl::body Toolbar::hideBalloon {} {
    wm withdraw $_hintWindow
}

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# OPTION MANAGEMENT for -helpstr, -balloonstr
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# -------------------------------------------------------------
# PRIVATE METHOD: _getAttachedOption
#
# optionListName : the name of the array that holds all attached
#              options. It is indexed via widget,option to get
#              the value.
# widget     : the widget that the option is associated with
# option     : the option whose value we are looking for on 
#              this widget.
#
# expects to be called only if the $option is length 1
# -------------------------------------------------------------
::itcl::body Toolbar::_getAttachedOption {optionListName widget args retValue} {
    # get a reference to the option, so we can change it.
    upvar $args argsRef
    upvar $retValue retValueRef
    
    set success false
    if {![catch { set retValueRef \
	    [eval set [subst [set optionListName]]($widget,$argsRef)]}]} {
	
	# remove the option argument 
	set success true
	set argsRef ""
    }
    return $success
}

# -------------------------------------------------------------
# PRIVATE METHOD: _setAttachedOption
#
# This method allows us to attach new options to a widget. It
# catches the 'option' to be attached, strips it out of 'args'
# attaches it to the 'widget' by stuffing the value into
# 'optionList(widget,option)'
#
# optionListName:  where to store the option and widget association
# widget: is the widget we want to associate the attached option
# option: is the attached option (unknown to this widget)
# args:   the arg list to search and remove the option from (if found)
#
# Modifies the args parameter.
# Returns boolean indicating the success of the method
#
# -------------------------------------------------------------
::itcl::body Toolbar::_setAttachedOption {optionListName widget option args} {
    upvar args argsRef
    
    set success false
    # check for 'option' in the 'args' list for the 'widget'
    set optPos [eval lsearch $args $option]
    # ... found it
    if {$optPos != -1} {
	# grab a copy of the option from arg list
	set [subst [set optionListName]]($widget,$option) \
		[eval lindex $args [expr {$optPos + 1}]]
	
	# remove the option argument and value from the arg list
	set argsRef [eval lreplace $args $optPos [expr {$optPos + 1}]]
	set success true
    }
    # ... if not found, will leave args alone
    return $success
}

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# GEOMETRY MANAGEMENT for tool widgets
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# -------------------------------------------------------------
# 
# PRIVATE METHOD: _packToolbar
#
#
#
# -------------------------------------------------------------
::itcl::body Toolbar::_packToolbar {} {
    # forget the previous locations
    foreach tool $_toolList {
	pack forget $tool
    }
    # pack in order of _toolList.
    foreach tool $_toolList {
	# adjust for radios and checks to match buttons
	if { [winfo class $tool] == "Radiobutton" || 
	[winfo class $tool] == "Checkbutton" } {
	    set iPad 1
	} else {
	    set iPad 0
	}
	# pack by horizontal or vertical orientation
	if {$itcl_options(-orient) == "horizontal" } {
	    pack $tool -side left -fill y \
		    -ipadx $iPad -ipady $iPad
	} else {
	    pack $tool -side top -fill x \
		    -ipadx $iPad -ipady $iPad
	}
    }
}

} ; # end ::itcl::widgets
