#
# Tabset Widget
# ----------------------------------------------------------------------
# A Tabset is a widget that contains a set of Tab buttons. 
# It displays these tabs in a row or column depending on it tabpos. 
# When a tab is clicked on, it becomes the only tab in the tab set that 
# is selected. All other tabs are deselected. The Tcl command prefix 
# associated with this tab (through the command tab configure option) 
# is invoked with the tab index number appended to its argument list. 
# This allows the Tabset to control another widget such as a Notebook.
#
# WISH LIST:
#   This section lists possible future enhancements.
#
#   1) When too many tabs appear, a small scrollbar should appear to
#      move the tabs over.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Tabset
# written by:
#    AUTHOR: Bill W. Scott
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: tabset.tcl,v 1.1.2.3 2009/01/06 22:03:43 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Tabset.width          0            widgetDefault
option add *Tabset.height         0            widgetDefault
option add *Tabset.equalTabs      true         widgetDefault
option add *Tabset.tabPos         s            widgetDefault
option add *Tabset.raiseSelect    false        widgetDefault
option add *Tabset.start          4            widgetDefault
option add *Tabset.margin         5            widgetDefault
option add *Tabset.tabBorders     true         widgetDefault
option add *Tabset.bevelAmount    0            widgetDefault
option add *Tabset.padX           4            widgetDefault
option add *Tabset.padY           4            widgetDefault
option add *Tabset.gap            overlap      widgetDefault
option add *Tabset.angle          20           widgetDefault
option add *Tabset.font           fixed        widgetDefault
option add *Tabset.state          normal       widgetDefault
option add *Tabset.disabledForeground #a3a3a3  widgetDefault
option add *Tabset.foreground     black        widgetDefault
option add *Tabset.background     #d9d9d9      widgetDefault
option add *Tabset.selectForeground black      widgetDefault
option add *Tabset.selectBackground #ececec    widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercase access method for the Tabset class
#

proc ::itcl::widgets::tabset {pathName args} {
    uplevel ::itcl::widgets::Tabset $pathName $args
}

# ------------------------------------------------------------------
#                              TABSET
# ------------------------------------------------------------------
::itcl::extendedclass Tabset {
    component itcl_hull
    component itcl_interior
    component canvas
    
    option [list -width width Width] -default 0 -configuremethod configWidth
    option [list -equaltabs equalTabs EqualTabs] -default true -configuremethod configEqualtabs
    option [list -height height Height] -default 0 -configuremethod configHeight
    option [list -tabpos tabPos TabPos] -default s -configuremethod configTabpos
    option [list -raiseselect raiseSelect RaiseSelect] -default false -configuremethod configRaiseselect
    option [list -start start Start] -default 4 -configuremethod configStart
    option [list -margin margin Margin] -default 5 -configuremethod configMargin
    option [list -tabborders tabBorders TabBorders] -default true -configuremethod configTabborders
    option [list -bevelamount bevelAmount BevelAmount] -default 0 -configuremethod configBevelamount
    option [list -padx padX PadX] -default 4 -configuremethod configPadx
    option [list -pady padY PadY] -default 4 -configuremethod configPady
    option [list -gap gap Gap] -default overlap -configuremethod configGap
    option [list -angle angle Angle] -default 20 -configuremethod configAngle
    option [list -font font Font] -default fixed -configuremethod configFont
    option [list -state state State] -default normal -configuremethod configState
    option [list \
        -disabledforeground disabledForeground DisabledForeground] -default #a3a3a3 -configuremethod configDisabledforeground
    option [list -foreground foreground Foreground] -default black -configuremethod configForeground
    option [list -background background Background] -default #d9d9d9  -configuremethod configBackground
    option [list -selectforeground selectForeground Background] -default black -configuremethod configSelectforeground
    option [list -backdrop backdrop Backdrop] -default white  -configuremethod configBackdrop
    option [list -selectbackground selectBackground Foreground] -default #ececec -configuremethod configSelectbackground
    option [list -command command Command] -default {}  -configuremethod configCommand
    
    private variable _width 0          ;# Width of the canvas in screen units
    private variable _height 0         ;# Height of the canvas in screen units
    private variable _selectedTop 0    ;# top edge of tab + a margin
    private variable _deselectedTop 0  ;# top edge of tab + a margin&raiseamt
    private variable _selectedLeft 0   ;# left edge of tab + a margin
    private variable _deselectedLeft 0 ;# left edge of tab + a margin&raiseamt
    private variable _tabs {}          ;# our internal list of tabs
    private variable _currTab -1       ;# numerical index # of selected tab
    private variable _uniqueID 0       ;# used to create unique names
    private variable _cmdStr  {}       ;# holds value of itcl_options(-command)
    ;# do not know why I need this!
    private variable _canvasWidth 0    ;# set by canvasReconfigure, is can wid
    private variable _canvasHeight 0   ;# set by canvasReconfigure, is can hgt
    
    private variable _anchorX 0        ;# used by mouse scrolling methods
    private variable _anchorY 0        ;# used by mouse scrolling methods
    
    private variable _margin 0         ;# -margin in screen units
    private variable _start  0         ;# -start in screen units
    private variable _gap overlap      ;# -gap in screen units
    
    private variable _relayout false   ;# flag tripped to tell whether to
                                       ;# relayout tabs after the configure
    private variable _skipRelayout false ;# flag that tells whether to skip
                                       ;# relayouting out the tabs. used by
                                       ;# _endMove.
    constructor {args} {} 
    destructor {}
    
    private method _createTab {args} 
    private method _deleteTabs {fromTab toTab} 
    private method _index {pathList index select} 
    private method _tabConfigure {args} 
    private method _relayoutTabs {} 
    private method _drawBevelBorder {} 
    private method _calcNextTabOffset {tabName} 
    private method _tabBounds {} 
    private method _recalcCanvasGeom {} 
    private method _canvasReconfigure {width height} 
    private method _startMove {x y} 
    private method _moveTabs {x y} 
    private method _endMove {x y} 
    private method _configRelayout {} 

    protected method _selectName {tabName} 
    protected method configWidth {option value}
    protected method configEqualtabs {option value}
    protected method configHeight {option value}
    protected method configTabpos {option value}
    protected method configRaiseselect {option value}
    protected method configStart {option value}
    protected method configMargin {option value}
    protected method configTabborders {option value}
    protected method configBevelamount {option value}
    protected method configPadx {option value}
    protected method configPady {option value}
    protected method configGap {option value}
    protected method configAngle {option value}
    protected method configFont {option value}
    protected method configState {option value}
    protected method configDisabledforeground {option value}
    protected method configForeground {option value}
    protected method configBackground {option value}
    protected method configSelectforeground {option value}
    protected method configBackdrop {option value}
    protected method configSelectbackground {option value}
    protected method configCommand {option value}
    
    public method configure {args} 
    public method add {args} 
    public method delete {args} 
    public method index {index} 
    public method insert {index args} 
    public method prev {} 
    public method next {} 
    public method select {index} 
    public method tabcget {index args} 
    public method tabconfigure {index args} 
    public method bbox {}
}

# ----------------------------------------------------------------------
#                              CONSTRUCTOR
# ----------------------------------------------------------------------
::itcl::body Tabset::constructor {args} {
    global tcl_platform

    set win [createhull frame $this -class [info class] -borderwidth 0]
    set itcl_interior $win
    #
    # Create the canvas that holds the tabs
    #
    setupcomponent canvas using canvas $itcl_interior.canvas -highlightthickness 0
    keepcomponentoption canvas -cursor -width -height
    pack $canvas -fill both -expand yes -anchor nw
    
    # ... This gives us a chance to redraw our bevel borders, etc when
    # the size of our canvas changes...
    bind $canvas <Configure> [itcl::code $this _canvasReconfigure %w %h]
    bind $canvas <Map> [itcl::code $this _relayoutTabs]
    # ... Allow button 2 scrolling as in label widget.
    if {$tcl_platform(os) ne "HP-UX"} {
        bind $canvas <2> [itcl::code $this _startMove %x %y]
        bind $canvas <B2-Motion> [itcl::code $this _moveTabs %x %y]
        bind $canvas <ButtonRelease-2> [itcl::code $this _endMove %x %y]
    }
    # @@@ 
    # @@@ Is there a better way?
    # @@@
    bind $win <Tab> [itcl::code $this next]
    bind $win <Shift-Tab> [itcl::code $this prev]
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
    _configRelayout 
    _recalcCanvasGeom
}

::itcl::body Tabset::destructor {} {
    foreach tab $_tabs {
        itcl::delete object $tab
    }
}

# ----------------------------------------------------------------------
#                              OPTIONS
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# OPTION -width
#
# Sets the width explicitly for the canvas of the tabset
# ----------------------------------------------------------------------
::itcl::body Tabset::configWidth {option value} {
    if {$value ne {}} {
    }
    set _width [winfo pixels $itcl_interior $value]
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -equaltabs
#
# If set to true, causes horizontal tabs to be equal in
# in width and vertical tabs to equal in height.
# ----------------------------------------------------------------------
::itcl::body Tabset::configEqualtabs {option value} {
    if {$value ne {}} {
        set _relayout true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -height
#
# Sets the height explicitly for the canvas of the tabset
# ----------------------------------------------------------------------
::itcl::body Tabset::configHeight {option value} {
    set _height [winfo pixels $itcl_interior $value]
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -tabpos
#
# Sets the tab position of tabs, n, s, e, w
# ----------------------------------------------------------------------
::itcl::body Tabset::configTabpos {option value} {
    if {$value ne {}} {
        switch $value {
        n {
            _tabConfigure -invert true -orient horizontal
          }
        s {
            _tabConfigure -invert false -orient horizontal
          }
        w {
            _tabConfigure -invert false -orient vertical
          }
        e {
            _tabConfigure -invert true -orient vertical
          }
        default {
            error "bad anchor position\
                \"$value\" must be n, s, e, or w"
          }
        }
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -raiseselect
#
# Sets whether to raise selected tabs slightly
# ----------------------------------------------------------------------
::itcl::body Tabset::configRaiseselect {option value} {
    if {$value ne {}} {
        set _relayout true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -start
#
# Sets the offset to start of tab set
# ----------------------------------------------------------------------
::itcl::body Tabset::configStart {option value} {
    if {$value ne {}} {
        set _start [winfo pixels $itcl_interior $value]
        set _relayout true
    } else {
        set _start 4
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -margin
#
# Sets the margin used above n tabs, below s tabs, left of e
# tabs, right of w tabs
# ----------------------------------------------------------------------
::itcl::body Tabset::configMargin {option value} {
    if {$value ne {}} {
    set _margin [winfo pixels $itcl_interior $value]
        set _relayout true
    } else {
        set _margin 5
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -tabborders
#
# Boolean that specifies whether to draw the borders of
# the unselected tabs (tabs in background)
# ----------------------------------------------------------------------
::itcl::body Tabset::configTabborders {option value} {
    if {$value ne {}} {
        _tabConfigure -tabborders $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -bevelamount
#
# Specifies pixel size of tab corners. 0 means no corners.
# ----------------------------------------------------------------------
::itcl::body Tabset::configBevelamount {option value} {
    if {$value ne {}} {
        _tabConfigure -bevelamount $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -padx
#
# Sets the padding in each tab to the left and right of label
# I don't convert for fpixels, since Tab does it for me.
# ----------------------------------------------------------------------
::itcl::body Tabset::configPadx {option value} {
    if {$value ne {}} {
        _tabConfigure -padx $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -pady
#
# Sets the padding in each tab to the left and right of label
# I don't convert for fpixels, since Tab does it for me.
# ----------------------------------------------------------------------
::itcl::body Tabset::configPady {option value} {
    if {$value ne {}} {
        _tabConfigure -pady $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -gap
#
# Sets the amount of spacing between tabs in pixels
# ----------------------------------------------------------------------
::itcl::body Tabset::configGap {option value} {
    if {$value ne {}} {
        if {$value ne "overlap"} {
            set _gap [winfo pixels $itcl_interior $value]
        } else {
            set _gap overlap
        }
        set _relayout true 
    } else {
        set _gap overlap
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -angle
#
# Sets the angle of the tab's sides
# ----------------------------------------------------------------------
::itcl::body Tabset::configAngle {option value} {
    if {$value ne {}} {
        _tabConfigure -angle $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -font
#
# Sets the font of the tab (SELECTED and UNSELECTED)
# ----------------------------------------------------------------------
::itcl::body Tabset::configFont {option value} {
    if {$value ne {}} {
        _tabConfigure -font $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -state
# ----------------------------------------------------------------------
::itcl::body Tabset::configState {option value} {
    if {$value ne {}} {
        _tabConfigure -state $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -disabledforeground
# ----------------------------------------------------------------------
::itcl::body Tabset::configDisabledforeground {option value} {
    if {$value ne {}} {
        _tabConfigure -disabledforeground $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -foreground
#
# Sets the foreground label color of UNSELECTED tabs
# ----------------------------------------------------------------------
::itcl::body Tabset::configForeground {option value} {
    _tabConfigure -foreground $value
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -background
#
# Sets the background color of UNSELECTED tabs
# ----------------------------------------------------------------------
::itcl::body Tabset::configBackground {option value} {
    if {$value ne {}} {
         _tabConfigure -background $value
    } else {
         _tabConfigure -background [$canvas cget -background]
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -selectforeground
#
# Sets the foreground label color of SELECTED tabs
# ----------------------------------------------------------------------
::itcl::body Tabset::configSelectforeground {option value} {
    _tabConfigure -selectforeground $value
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -backdrop
#
# Sets the background color of the Tabset backdrop (behind the tabs)
# ----------------------------------------------------------------------
::itcl::body Tabset::configBackdrop {option value} {
    if {$value ne {}} {
        $canvas configure -background $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -selectbackground
#
# Sets the background color of SELECTED tabs
# ----------------------------------------------------------------------
::itcl::body Tabset::configSelectbackground {option value} {
    if {$value ne {}} {
    } else {
        #set _selectBackground [$canvas cget -background]
    }
    _tabConfigure -selectbackground $value
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -command
#
# The command to invoke when a tab is hit.
# ----------------------------------------------------------------------
::itcl::body Tabset::configCommand {option value} {
    if {$value ne {}} {
        set _cmdStr $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# METHOD: add ?option value...?
#
# Creates a tab and appends it to the list of tabs.
# processes tabconfigure for the tab added.
# ----------------------------------------------------------------------
::itcl::body Tabset::add {args} {
    set tabName [uplevel 0 _createTab $args]
    lappend _tabs $tabName
    _relayoutTabs
    return $tabName
}

# ----------------------------------------------------------------------
# METHOD: configure ?option? ?value option value...?
#
# Acts as an addendum to the itk::Widget::configure method.
#
# Checks the _relayout flag to see if after configures are done
# we need to relayout the tabs.
#
# _skipRelayout is set in the MB2 scroll methods, to avoid constant
# relayout of tabs while dragging the mouse.
# ----------------------------------------------------------------------
::itcl::body Tabset::configure {args} {
    set result [eval itcl::builtin::configure $args]
    _configRelayout
    return $result
}

::itcl::body Tabset::_configRelayout {} {
    # then relayout tabs if necessary
    if {$_relayout} {
        if {!$_skipRelayout} {
            _relayoutTabs
        }
        set _relayout false
    }
}

# ----------------------------------------------------------------------
# METHOD: delete index1 ?index2?
#
# Deletes a tab or range of tabs from the tabset
# ----------------------------------------------------------------------
::itcl::body Tabset::delete {args} {
    if {$_tabs eq {}} {
        error "can't delete tabs,\
            no tabs in the tabset named $win"
    }
    
    set len [llength $args]
    switch $len {
    0 {
        error "wrong # args: should be\
            \"$win delete index1 ?index2?\""
    }
    
    1 {
        set fromTab [index [lindex $args 0]]
        if {$fromTab == -1} {
            error "bad value for index1:\
                [lindex $args 0] in call to delete"
        }
        set toTab $fromTab
        _deleteTabs $fromTab $toTab
      }
    2 {
        set fromTab [index [lindex $args 0]]
        if {$fromTab == -1} {
            error "bad value for index1:\
                [lindex $args 0] in call to delete"
        }
        set toTab [index [lindex $args 1]]
        if {$toTab == -1} {
            error "bad value for index2:\
                [lindex $args 1] in call to delete"
        }
        _deleteTabs $fromTab $toTab
      }
    default {
        error "wrong # args: should be\
            \"$win delete index1 ?index2?\""
      }
    }
}

# ----------------------------------------------------------------------
# METHOD: index index
#
# Given an index identifier returns the numeric index of the tab
# ----------------------------------------------------------------------
::itcl::body Tabset::index {index} {
    return [_index $_tabs $index $_currTab]
}

# ----------------------------------------------------------------------
# METHOD: insert index ?option value...?
#
# Inserts a tab before a index. The before tab may
# be specified as a label or a tab position.
# ----------------------------------------------------------------------
::itcl::body Tabset::insert {index args} {
    if {$_tabs eq {}} {
        error "no tab to insert before,\
            tabset '$win' is empty"
    }
    # get the tab
    set tab [index $index]
    # catch bad value for before tab.
    if {($tab < 0) || ($tab >= [llength $_tabs])} {
        error "bad value $tab for index:\
            should be between 0 and [expr {[llength $_tabs] - 1}]"
    }
    # create the new tab and get its name...
    set tabName [eval _createTab $args]
    # grab the name of the tab currently selected. (to keep in sync)
    set currTabName [lindex $_tabs $_currTab]
    # insert tabName before $tab
    set _tabs [linsert $_tabs $tab $tabName]
    # keep the _currTab in sync with the insert.
    set _currTab [lsearch -exact $_tabs $currTabName]
    _relayoutTabs
    return $tabName
}

# ----------------------------------------------------------------------
# METHOD: prev
#
# Selects the prev tab. Wraps at first back to last tab.
# ----------------------------------------------------------------------
::itcl::body Tabset::prev {} {
    if {$_tabs eq {}} {
        error "can't goto previous tab,\
            no tabs in the tabset: $win"
    }
    
    # bump to the previous tab and wrap if necessary
    set prev [expr {$_currTab - 1}]
    if {$prev < 0} {
        set prev [expr {[llength $_tabs] - 1}]
    }
    select $prev
}

# ----------------------------------------------------------------------
# METHOD: next
#
# Selects the next tab. Wraps at last back to first tab.
# ----------------------------------------------------------------------
::itcl::body ::itcl::widgets::Tabset::next {} {
    if {$_tabs eq {}} {
        error "can't goto next tab,\
            no tabs in the tabset: $win"
    }
    # bump to the next tab and wrap if necessary
    set next [expr {$_currTab + 1}]
    if {$next >= [llength $_tabs]} {
        set next 0
    }
    select $next
}

# ----------------------------------------------------------------------
# METHOD: select index
#
# Select a tab by index
#
# Lowers the last _currTab if it existed.
# Then raises the new one if it exists.
#
# Returns numeric index of selection, -1 if failed.
# -------------------------------------------------------------
::itcl::body Tabset::select {index} {
    if {$_tabs eq {}} {
        error "can't activate a tab,\
            no tabs in the tabset: $win"
    }
    # if there is not current selection just ignore trying this selection
    if {($index eq "select") && ($_currTab == -1)} {
        return -1
    }
    # is selection request in range ? 
    set reqTab [index $index]
    if {$reqTab == -1} {
        error "bad value $index for index:\
            should be from 0 to [expr {[llength $_tabs] - 1}]"
    }
    # If already selected then ignore and return...
    if {$reqTab == $_currTab} {
        return $reqTab
    }
    # ---- Deselect
    if {$_currTab != -1} {
        set currTabName [lindex $_tabs $_currTab]
        $currTabName deselect
        # handle different orientations...
        if {($itcl_options(-tabpos) eq "n") || ($itcl_options(-tabpos) eq "s")} {
            $currTabName configure -top $_deselectedTop
        } else {
            $currTabName configure -left $_deselectedLeft
        }
    }
    # get the stacking order correct...
    foreach tab $_tabs {
        $tab lower
    }
    # set this now so that the -command cmd can do an 'index select'
    # to operate on this tab.
    set _currTab $reqTab
    # ---- Select
    set reqTabName [lindex $_tabs $reqTab]
    $reqTabName select
    if {($itcl_options(-tabpos) eq "n") || ($itcl_options(-tabpos) eq "s")} {
        $reqTabName configure -top $_selectedTop
    } else {
        $reqTabName configure -left $_selectedLeft
    }
    set _currTab $reqTab
    # invoke any user command string, appended with tab index number
    if {$_cmdStr ne {}} {
        set newCmd $_cmdStr
        uplevel 0 [lappend newCmd $reqTab]
    }
    return $reqTab
}

# ----------------------------------------------------------------------
# METHOD: tabcget index ?option? 
#
# Returns the value for the option setting of the tab at index $index.
# ----------------------------------------------------------------------
::itcl::body Tabset::tabcget {index args} {
    return [lindex [uplevel 0 tabconfigure $index $args] 2]
}

# ----------------------------------------------------------------------
# METHOD: tabconfigure index ?option? ?value option value?
#
# tabconfigure index : returns configuration list
# tabconfigure index -option : returns option values
# tabconfigure index ?option value option value ...? sets options
#   and returns empty string.
#
# Performs configure on a given tab denoted by index.
#
# Index may be a tab number or a pattern matching the label
# associated with a tab.
# ----------------------------------------------------------------------
::itcl::body Tabset::tabconfigure {index args} {
    # convert index to numeric
    set tab [index $index]
    if {$tab == -1} {
        error "bad index value:\
            $index for $win tabconfigure"
    }
    set tabName [lindex $_tabs $tab]
    set len [llength $args]
    switch $len {
    0 {
        return [uplevel 0 $tabName configure]
      }
    1 {
        return [uplevel 0 $tabName configure $args]
      }
    default {
        uplevel 0 $tabName configure $args
        _relayoutTabs
        select select
      }
    }
    return ""
}

# ----------------------------------------------------------------------
# METHOD: bbox
# 
# calculates the bounding box that will completely enclose 
# all the tabs.
# ----------------------------------------------------------------------
::itcl::body Tabset::bbox {} {
    return [_tabBounds]
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _selectName
#
# internal method to allow selection by internal tab name 
# rather than index. This is used by the bind methods
# ----------------------------------------------------------------------
::itcl::body Tabset::_selectName {tabName} {
    # if the tab is disabled, then ignore this selection...
    if {[$tabName cget -state] eq "disabled"} {
        return
    }
    set tab [lsearch -exact $_tabs $tabName]
    select $tab
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _createTab
#
# Creates a tab, using unique tab naming, propagates background
# and keeps unique id up to date.
# ----------------------------------------------------------------------
::itcl::body Tabset::_createTab {args} {
    #
    # create an internal name for the tab: tab0, tab1, etc.
    # these are one-up numbers they do not 
    # correspond to the position the tab is located in.
    #
    set tabName $this-tab$_uniqueID
    switch $itcl_options(-tabpos) {
    n {
        set invert true
        set orient horizontal
        set x 0
        set y [expr {$_margin + 1}]
      }
    s {
        set invert false
        set orient horizontal
        set x 0
        set y 0
      }
    w {
        set invert false
        set orient vertical
        set x 0 
        set y 0
      }
    e {
        set invert true
        set orient vertical
        set x [expr {$_margin + 1}]
        set y 0
      }
    default {
        error "bad anchor position\
            \"$itcl_options(-tabpos)\" must be n, s, e, or w"
      }
    }
    set tabOptions [list \
        -left             $x \
        -top              $y \
        -font             [list $itcl_options(-font)] \
        -background       $itcl_options(-background) \
        -foreground       $itcl_options(-foreground) \
        -selectforeground $itcl_options(-selectforeground) \
        -disabledforeground $itcl_options(-disabledforeground) \
        -selectbackground $itcl_options(-selectbackground) \
        -angle            $itcl_options(-angle) \
        -padx             $itcl_options(-padx) \
        -pady             $itcl_options(-pady) \
        -bevelamount      $itcl_options(-bevelamount) \
        -state            $itcl_options(-state) \
        -tabborders       $itcl_options(-tabborders) \
        -invert           $invert \
        -orient           $orient \
    ]
    uplevel 0 ::itcl::widgets::Tab $tabName $canvas $tabOptions $args
    $tabName lower
    $canvas bind $tabName <Button-1> [itcl::code $this _selectName $tabName]
    incr _uniqueID
    return $tabName
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _deleteTabs
#
# Deletes tabs from $fromTab to $toTab.
#
# Operates in two passes, destroys all the widgets
# Then removes the pathName from the tab list
#
# Also keeps the current selection in bounds.
# ----------------------------------------------------------------------
::itcl::body Tabset::_deleteTabs {fromTab toTab} {
    for {set tab $fromTab} {$tab <= $toTab} {incr tab} {
        set tabName [lindex $_tabs $tab]
        # unbind Button-1 from this window name
        $canvas bind $tabName <Button-1> {}
        # Destroy the Tab class...
        itcl::delete object $tabName 
    }
    # physically remove the tab
    set _tabs [lreplace $_tabs $fromTab $toTab]
    # If we deleted a selected tab set our selection to none
    if {($_currTab >= $fromTab) && ($_currTab <= $toTab)} {
        set _currTab -1
        _drawBevelBorder
    }
    # make sure _currTab stays in sync with new numbering...
    if {$_tabs eq {}} {
        # if deleted only remaining tab,
        # reset current tab to undefined
        set _currTab -1
        # or if the current tab was the last tab, it needs come back
    } elseif {$_currTab >= [llength $_tabs]} {
        incr _currTab -1
        if {$_currTab < 0} {
            # but only to zero
            set _currTab 0
        }
    }
    _relayoutTabs
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _index
#
# pathList : list of path names to search thru if index is a label
# index    : either number, 'select', 'end', or pattern
# select   : current selection
#
# _index takes takes the value $index converts it to
# a numeric identifier. If the value is not already
# an integer it looks it up in the $pathList array.
# If it fails it returns -1
# ----------------------------------------------------------------------
::itcl::body Tabset::_index {pathList index select} {
    switch $index {
    select {
        set number $select
      }
    end {
        set number [expr {[llength $pathList] -1}]
      }
    default {
        # is it an number already?
        if {[regexp {^[0-9]+$} $index]} {
            set number $index
            if {($number < 0) || ($number >= [llength $pathList])} {
                set number -1
            }
            # otherwise it is a label
        } else {
            # look thru the pathList of pathNames and
            # get each label and compare with index.
            # if we get a match then set number to postion in $pathList
            # and break out.
            # otherwise number is still -1
            set i 0
            set number -1
            foreach pathName $pathList {
                set label [$pathName cget -label]
                if {$label eq $index} {
                    set number $i
                    break
                }
                incr i
            }
        }
      }
    }
    return $number
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _tabConfigure
# ----------------------------------------------------------------------
::itcl::body Tabset::_tabConfigure {args} {
    foreach tab $_tabs {
        uplevel 0 $tab configure $args
    }
    set _relayout true
    if {$_tabs ne {}} {
        select select
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _relayoutTabs
# 
# relays out the tabs with correct spacing...
# ----------------------------------------------------------------------
::itcl::body Tabset::_relayoutTabs {} {
    if {([llength $_tabs] == 0) || ![winfo viewable $win]} {
        return
    }
    # get the max width for fixed width tabs...
    set maxWidth 0
    foreach tab $_tabs {
        set width [$tab labelwidth]
        if {$width > $maxWidth} {
            set maxWidth $width
        }
    }
    # get the max height for fixed height tabs...
    set maxHeight 0
    foreach tab $_tabs {
        set height [$tab labelheight]
        if {$height > $maxHeight} {
            set maxHeight $height
        }
    }
    # get curr tab's name
    set currTabName [lindex $_tabs $_currTab]
    # Start with our margin offset in pixels...
    set tabStart $_start
    if { $itcl_options(-raiseselect) } {
        set raiseAmt 2
    } else {
        set raiseAmt 0
    }
    #
    # Depending on the tab layout: n, s, e, or w place the tabs
    # according to orientation, raise, margins, etc.
    #
    switch $itcl_options(-tabpos) {
    n {
        set _selectedTop [expr {$_margin + 1}]
        set _deselectedTop [expr {$_selectedTop + $raiseAmt}]
        if { $itcl_options(-equaltabs) } {
            set tabWidth $maxWidth
        } else {
            set tabWidth 0
        }
        
        foreach tab $_tabs {
            if {$tab eq $currTabName} {
                $tab configure -left $tabStart -top $_selectedTop \
                    -height $maxHeight -width $tabWidth -anchor c
            } else {
                $tab configure -left $tabStart -top $_deselectedTop \
                    -height $maxHeight -width $tabWidth -anchor c
            }
            set tabStart [expr {$tabStart + [_calcNextTabOffset $tab]}]
        }
      }
    s {
        set _selectedTop 0
        set _deselectedTop [expr {$_selectedTop - $raiseAmt}]
        if {$itcl_options(-equaltabs)} {
            set tabWidth $maxWidth
        } else {
            set tabWidth 0
        }
        foreach tab $_tabs {
            if {$tab eq $currTabName} {
                $tab configure -left $tabStart -top $_selectedTop \
                    -height $maxHeight -width $tabWidth -anchor c
            } else {
                $tab configure -left $tabStart -top $_deselectedTop \
                    -height $maxHeight -width $tabWidth -anchor c
            }
            set tabStart [expr {$tabStart + [_calcNextTabOffset $tab]}]
        }
        
      }
    w {
        set _selectedLeft [expr {$_margin + 1}]
        set _deselectedLeft [expr {$_selectedLeft + $raiseAmt}]
        if {$itcl_options(-equaltabs)} {
            set tabHeight $maxHeight
        } else {
            set tabHeight 0
        }
        foreach tab $_tabs {
            # selected
            if {$tab eq $currTabName} {
                $tab configure -top $tabStart -left $_selectedLeft \
                    -height $tabHeight -width $maxWidth -anchor e
                # deselected
            } else {
                $tab configure -top $tabStart -left $_deselectedLeft \
                    -height $tabHeight -width $maxWidth -anchor e
            }
            set tabStart [expr {$tabStart + [_calcNextTabOffset $tab]}]
        }
        
      }
    e {
        set _selectedLeft 0
        set _deselectedLeft [expr {$_selectedLeft - $raiseAmt}]
        if {$itcl_options(-equaltabs)} {
            set tabHeight $maxHeight
        } else {
            set tabHeight 0
        }
        foreach tab $_tabs {
            # selected
            if {$tab eq $currTabName} {
                $tab configure -top $tabStart -left $_selectedLeft \
                    -height $tabHeight -width $maxWidth -anchor w
                # deselected
            } else {
                $tab configure -top $tabStart -left $_deselectedLeft \
                    -height $tabHeight -width $maxWidth -anchor w
            }
            set tabStart [expr {$tabStart + [_calcNextTabOffset $tab]}]
        }
      }
    default {
        error "bad anchor position\
            \"$itcl_options(-tabpos)\" must be n, s, e, or w"
      }
    }
    # put border on & calc our new canvas size...
    _drawBevelBorder
    _recalcCanvasGeom
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _drawBevelBorder
# 
# draws the bevel border along tab edge (below selected tab)
# ----------------------------------------------------------------------
::itcl::body Tabset::_drawBevelBorder {} {
    $canvas delete bevelBorder
    switch $itcl_options(-tabpos) {
    n {
        $canvas create line \
            0 [expr {$_canvasHeight - 1}] \
            $_canvasWidth [expr {$_canvasHeight - 1}] \
            -fill [::itcl::widgets::colors::topShadow $itcl_options(-selectbackground)] \
            -tags bevelBorder
        $canvas create line \
            0 $_canvasHeight \
            $_canvasWidth $_canvasHeight \
            -fill [::itcl::widgets::colors::topShadow $itcl_options(-selectbackground)] \
            -tags bevelBorder
      }
    s {
        $canvas create line \
            0 0 \
            $_canvasWidth 0 \
            -fill [::itcl::widgets::colors::bottomShadow $itcl_options(-selectbackground)] \
            -tags bevelBorder
        $canvas create line \
            0 1 \
            $_canvasWidth 1 \
            -fill black \
            -tags bevelBorder
      }
    w {
        $canvas create line \
            $_canvasWidth 0 \
            $_canvasWidth [expr {$_canvasHeight - 1}] \
            -fill [::itcl::widgets::colors::topShadow $itcl_options(-selectbackground)] \
            -tags bevelBorder
        $canvas create line \
            [expr {$_canvasWidth - 1}] 0 \
            [expr {$_canvasWidth - 1}] [expr {$_canvasHeight - 1}] \
            -fill [::itcl::widgets::colors::topShadow $itcl_options(-selectbackground)] \
            -tags bevelBorder
      }
    e {
        $canvas create line \
            0 0 \
            0 [expr {$_canvasHeight - 1}] \
            -fill black \
            -tags bevelBorder
        $canvas create line \
            1 0 \
            1 [expr {$_canvasHeight - 1}] \
            -fill [::itcl::widgets::colors::bottomShadow $itcl_options(-selectbackground)] \
            -tags bevelBorder
        
      }
    }
    $canvas raise bevelBorder
    if {$_currTab != -1} {
        set currTabName [lindex $_tabs $_currTab]
        $currTabName raise
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _calcNextTabOffset
# 
# given $tabName, determines the offset in pixels to place
# the next tab's start edge at.
# ----------------------------------------------------------------------
::itcl::body Tabset::_calcNextTabOffset {tabName} {
    if {$_gap eq "overlap"} {
        return [$tabName offset]
    } else {
        return [expr {[$tabName majordim] + $_gap}]
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _tabBounds
# 
# calculates the bounding box that will completely enclose 
# all the tabs.
# ----------------------------------------------------------------------
::itcl::body Tabset::_tabBounds {} {
    set bbox {100000 100000 -10000 -10000}
    foreach tab $_tabs {
        set tabBBox [$tab bbox]
        # if this left is less use it
        if {[lindex $tabBBox 0] < [lindex $bbox 0]} {
            set bbox [lreplace $bbox 0 0 [lindex $tabBBox 0]]
        }
        # if this top is greater use it
        if {[lindex $tabBBox 1] < [lindex $bbox 1]} {
            set bbox [lreplace $bbox 1 1 [lindex $tabBBox 1]]
        }
        # if this right is less use it
        if {[lindex $tabBBox 2] > [lindex $bbox 2]} {
            set bbox [lreplace $bbox 2 2 [lindex $tabBBox 2]]
        }
        # if this bottom is greater use it
        if {[lindex $tabBBox 3] > [lindex $bbox 3]} {
            set bbox [lreplace $bbox 3 3 [lindex $tabBBox 3]]
        }
    }
    return $bbox
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _recalcCanvasGeom
# 
# Based on size of tabs, recalculates the canvas geometry that
# will hold the tabs.
# ----------------------------------------------------------------------
::itcl::body Tabset::_recalcCanvasGeom {} {
    if {[llength $_tabs] == 0} {
        return
    }
    set bbox [_tabBounds]
    set width [lindex [_tabBounds] 2]
    set height [lindex [_tabBounds] 3]
    # now we have the dimensions of all the tabs in the canvas.
    switch $itcl_options(-tabpos) {
    n {
        # height already includes margin
        $canvas configure \
            -width $width \
            -height $height
      }
    s {
        $canvas configure \
            -width $width \
            -height [expr {$height + $_margin}]
      }
    w {
        # width already includes margin
        $canvas configure \
            -width $width \
            -height [expr {$height + 1}]
      }
    e {
        $canvas configure \
            -width [expr {$width + $_margin}] \
            -height [expr {$height + 1}]
      }
    default {
      }
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _canvasReconfigure
# 
# Bound to the reconfigure notify event of a canvas, this 
# method resets canvas's correct width (since we are fill x)
# and redraws the beveled edge border.
# will hold the tabs.
# ----------------------------------------------------------------------
::itcl::body Tabset::_canvasReconfigure {width height} {
    set _canvasWidth $width
    set _canvasHeight $height
    
    if {[llength $_tabs] > 0} {
        _drawBevelBorder
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _startMove
# 
# This method is bound to the MB2 down in the canvas area of the
# tab set. This starts animated scrolling of the tabs along their
# major axis.
# ----------------------------------------------------------------------
::itcl::body Tabset::_startMove {x y} {
    if {($itcl_options(-tabpos) eq "n") || ($itcl_options(-tabpos) eq "s")} {
        set _anchorX $x
    } else {
        set _anchorY $y
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _moveTabs
# 
# This method is bound to the MB2 motion in the canvas area of the
# tab set. This causes the tabset to move with the mouse.
# ----------------------------------------------------------------------
::itcl::body Tabset::_moveTabs {x y} {
    if {($itcl_options(-tabpos) eq "n") || ($itcl_options(-tabpos) eq "s")} {
        set startX [expr {$_start + $x - $_anchorX}]
        foreach tab $_tabs {
            $tab configure -left $startX 
            set startX [expr {$startX + [_calcNextTabOffset $tab]}]
        }
    } else {
        set startY [expr {$_start + $y - $_anchorY}]
        foreach tab $_tabs {
            $tab configure -top $startY 
            set startY [expr {$startY + [_calcNextTabOffset $tab]}]
        }
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _endMove
# 
# This method is bound to the MB2 release in the canvas area of the
# tab set. This causes the tabset to end moving tabs.
# ----------------------------------------------------------------------
::itcl::body Tabset::_endMove {x y} {
    if {($itcl_options(-tabpos) eq "n") || ($itcl_options(-tabpos) eq "s")} {
        set startX [expr {$_start + $x - $_anchorX}]
        set _skipRelayout true
        configure -start $startX
        set _skipRelayout false
    } else {
        set startY [expr {$_start + $y - $_anchorY}]
        set _skipRelayout true
        configure -start $startY
        set _skipRelayout false
    }
}

} ; # end ::itcl::widgets

