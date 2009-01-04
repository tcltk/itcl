#
# Tab Class
# ----------------------------------------------------------------------
# A Tab class is an [incr Tcl] class that displays either an image, 
# bitmap, or label in a graphic object on a canvas. This graphic object 
# can have a wide variety of appearances depending on the options set. 
#
# WISH LIST:
#   This section lists possible future enhancements.
#
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the ::itcl::widgets package Tabset
# written by:
#    AUTHOR: Bill W. Scott
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: tab.tcl,v 1.1.2.1 2009/01/04 13:39:38 wiede Exp $
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

#==============================================================
# CLASS: Tab
#==============================================================

::itcl::class Tab {
    private variable   _selected false
    private variable   _padX 0
    private variable   _padY 0
    
    private variable   _canvas
    
    # these are in pixels
    private variable   _left 0
    private variable   _width 0
    private variable   _height 0
    private variable   _oldLeft 0
    private variable   _top 0
    private variable   _oldTop 0
    
    private variable   _right
    private variable   _bottom
    
    private variable   _offset
    private variable   _majorDim
    private variable   _minorDim
    
    private variable   _darkShadow
    private variable   _lightShadow
    # graphic components that make up a tab
    #
    private variable   _gRegion
    private variable   _gLabel
    private variable   _gLightOutline {}
    private variable   _gBlackOutline {}
    private variable   _gTopLine
    private variable   _gTopLineShadow
    private variable   _gLightShadow
    private variable   _gDarkShadow
    private variable   _labelWidth 0
    private variable   _labelHeight 0
    private variable   _labelXOrigin 0
    private variable   _labelYOrigin 0
    private variable   _just left
    private variable   _configTripped true
    
    common _tan
    
    set _tan(0)  0.0
    set _tan(1)  0.0175
    set _tan(2)  0.0349
    set _tan(3)  0.0524
    set _tan(4)  0.0699
    set _tan(5)  0.0875
    set _tan(6)  0.1051
    set _tan(7)  0.1228
    set _tan(8)  0.1405
    set _tan(9)  0.1584
    set _tan(10) 0.1763
    set _tan(11) 0.1944
    set _tan(12) 0.2126
    set _tan(13) 0.2309
    set _tan(14) 0.2493
    set _tan(15) 0.2679
    set _tan(16) 0.2867
    set _tan(17) 0.3057
    set _tan(18) 0.3249
    set _tan(19) 0.3443
    set _tan(20) 0.3640
    set _tan(21) 0.3839
    set _tan(22) 0.4040
    set _tan(23) 0.4245
    set _tan(24) 0.4452
    set _tan(25) 0.4663
    set _tan(26) 0.4877
    set _tan(27) 0.5095
    set _tan(28) 0.5317
    set _tan(29) 0.5543
    set _tan(30) 0.5774
    set _tan(31) 0.6009
    set _tan(32) 0.6294
    set _tan(33) 0.6494
    set _tan(34) 0.6745
    set _tan(35) 0.7002
    set _tan(36) 0.7265
    set _tan(37) 0.7536
    set _tan(38) 0.7813
    set _tan(39) 0.8098
    set _tan(40) 0.8391
    set _tan(41) 0.8693
    set _tan(42) 0.9004
    set _tan(43) 0.9325
    set _tan(44) 0.9657
    set _tan(45) 1.0
    
    # 
    public variable bevelamount 0 {}
    public variable state normal {}
    public variable height 0 {}
    public variable width 0 {}
    public variable anchor c {}
    public variable left 0 {}
    public variable top 0 {}
    public variable image {} {}
    public variable bitmap {} {}
    public variable label {} {}
    public variable padx 4 {}
    public variable pady 4 {}
    public variable selectbackground "gray70" {}
    public variable selectforeground "black" {}
    public variable disabledforeground "gray" {}
    public variable background "white" {}
    public variable foreground "black" {}
    public variable orient vertical {}
    public variable invert false {}
    public variable angle 20 {}
    public variable font \
       "-adobe-helvetica-bold-r-normal--34-240-100-100-p-182-iso8859-1" {}

    public variable tabborders true {}
    
    constructor {args} {}
    destructor {}
    
    private method _makeTab {} 
    private method _createLabel {canvas tagList} 
    private method _makeEastTab {canvas} 
    private method _makeWestTab {canvas} 
    private method _makeNorthTab {canvas}
    private method _makeSouthTab {canvas}
    private method _calcLabelDim {labelItem}
    private method _itk_config  {args} ::itcl::builtin::configure 
    private method _selectNoRaise {}
    private method _deselectNoLower {}
    
    public method configure {args} 
    public method bbox  {} 
    public method deselect {} 
    public method lower {} 
    public method majordim  {} 
    public method minordim  {} 
    public method offset  {} 
    public method raise {} 
    public method select {}
    public method labelheight {}
    public method labelwidth {} 
    
}

# ----------------------------------------------------------------------
#                              CONSTRUCTOR
# ----------------------------------------------------------------------
::itcl::body Tab::constructor {args} {
    set _canvas [lindex $args 0]
    set args [lrange $args 1 end]
    set _darkShadow  [::itcl::widgets::colors::bottomShadow $selectbackground]
    set _lightShadow [::itcl::widgets::colors::topShadow $selectbackground]
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

# ----------------------------------------------------------------------
#                              DESTRUCTOR
# ----------------------------------------------------------------------
::itcl::body Tab::destructor {} {
    if {[winfo exists $_canvas]} {
        $_canvas delete $this
    }
}

# ----------------------------------------------------------------------
#                              OPTIONS
# ----------------------------------------------------------------------
#
# Note, we trip _configTripped for every option that requires the tab
# to be remade.
#
# ----------------------------------------------------------------------
# OPTION -bevelamount
#
# Specifies the size of tab corners. A value of 0 with angle set 
# to 0 results in square tabs. A bevelAmount of 4, means that the 
# tab will be drawn with angled corners that cut in 4 pixels from 
# the edge of the tab. The default is 0.
# ----------------------------------------------------------------------
::itcl::configbody Tab::bevelamount { 
}

# ----------------------------------------------------------------------
# OPTION -state
#
# sets the active state of the tab. specifying normal allows 
# the tab to be selectable. Specifying disabled disables the tab, 
# causing its image, bitmap, or label to be drawn with the 
# disabledForeground color.
# ----------------------------------------------------------------------
::itcl::configbody Tab::state { 
}

# ----------------------------------------------------------------------
# OPTION -height
#
# the height of the tab. if 0, uses the font label height.
# ----------------------------------------------------------------------
::itcl::configbody Tab::height {
    set _height [winfo pixels $_canvas $height]
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -width
#
# The width of the tab. If 0, uses the font label width.
# ----------------------------------------------------------------------
::itcl::configbody Tab::width {
    set _width [winfo pixels $_canvas $width]
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -anchor
#
# Where the text in the tab will be anchored: n,nw,ne,s,sw,se,e,w,center
# ----------------------------------------------------------------------
::itcl::configbody Tab::anchor { 
}

# ----------------------------------------------------------------------
# OPTION -left
#
# Specifies the left edge of the tab's bounding box. This value 
# may have any of the forms acceptable to Tk_GetPixels.
# ----------------------------------------------------------------------
::itcl::configbody Tab::left {
    # get into pixels
    set _left [winfo pixels $_canvas $left]
    # move by offset from last setting
    $_canvas move $this [expr {$_left - $_oldLeft}] 0
    # update old for next time
    set _oldLeft $_left
}

# ----------------------------------------------------------------------
# OPTION -top
#
# Specifies the topedge of the tab's bounding box. This value may 
# have any of the forms acceptable to Tk_GetPixels.
# ----------------------------------------------------------------------
::itcl::configbody Tab::top {
    # get into pixels
    set _top [winfo pixels $_canvas $top]
    # move by offset from last setting
    $_canvas move $this 0 [expr {$_top - $_oldTop}]
    # update old for next time
    set _oldTop $_top
}

# ----------------------------------------------------------------------
# OPTION -image
#
# Specifies the imageto display in the tab. 
# Images are created with the image create command. 
# ----------------------------------------------------------------------
::itcl::configbody Tab::image { 
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -bitmap
#
# If bitmap is an empty string, specifies the bitmap to display in 
# the tab. Bitmap may be of any of the forms accepted by Tk_GetBitmap. 
# ----------------------------------------------------------------------
::itcl::configbody Tab::bitmap { 
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -label
#
# If image is an empty string and bitmap is an empty string, 
# it specifies a text string to be placed in the tab's label. 
# This label serves as an additional identifier used to reference 
# the tab. Label may be used for the index value in widget commands.
# ----------------------------------------------------------------------
::itcl::configbody Tab::label { 
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -padx
#
# Horizontal padding around the label (text, image, or bitmap).
# ----------------------------------------------------------------------
::itcl::configbody Tab::padx {
    set _configTripped true
    set _padX [winfo pixels $_canvas $padx]
}

# ----------------------------------------------------------------------
# OPTION -pady
#
# Vertical padding around the label (text, image, or bitmap).
# ----------------------------------------------------------------------
::itcl::configbody Tab::pady {
    set _configTripped true
    set _padY [winfo pixels $_canvas $pady]
}

# ----------------------------------------------------------------------
# OPTION -selectbackground
# ----------------------------------------------------------------------
::itcl::configbody Tab::selectbackground {
    set _darkShadow  [::itcl::widgets::colors::bottomShadow $selectbackground]
    set _lightShadow [::itcl::widgets::colors::topShadow $selectbackground]
    if {$_selected} {
        _selectNoRaise
    } else {
        _deselectNoLower
    }
}

# ----------------------------------------------------------------------
# OPTION -selectforeground
#
# Foreground of tab when selected
# ----------------------------------------------------------------------
::itcl::configbody Tab::selectforeground { 
    if {$_selected} {
        _selectNoRaise
    } else {
        _deselectNoLower
    }
}

# ----------------------------------------------------------------------
# OPTION -disabledforeground
#
# Background of tab when -state is disabled
# ----------------------------------------------------------------------
::itcl::configbody Tab::disabledforeground { 
    if {$_selected} {
        _selectNoRaise
    } else {
        _deselectNoLower
    }
}

# ----------------------------------------------------------------------
# OPTION -background
#
# Normal background of tab.
# ----------------------------------------------------------------------
::itcl::configbody Tab::background { 
    if {$_selected} {
        _selectNoRaise
    } else {
        _deselectNoLower
    }
}

# ----------------------------------------------------------------------
# OPTION -foreground
#
# Foreground of tabs when in normal unselected state
# ----------------------------------------------------------------------
::itcl::configbody Tab::foreground { 
    if {$_selected} {
        _selectNoRaise
    } else {
        _deselectNoLower
    }
}

# ----------------------------------------------------------------------
# OPTION -orient
#
# Specifies the orientation of the tab. Orient can be either 
# horizontal or vertical. 
# ----------------------------------------------------------------------
::itcl::configbody Tab::orient { 
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -invert
#
# Specifies the direction to draw the tab. If invert is true, 
# it draws horizontal tabs upside down and vertical tabs opening 
# to the left (pointing right). The value may have any of the 
# forms accepted by the Tcl_GetBoolean, such as true, 
# false, 0, 1, yes, or no.
# ----------------------------------------------------------------------
::itcl::configbody Tab::invert { 
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -angle
#
# Specifes the angle of slope from the inner edge to the outer edge 
# of the tab. An angle of 0 specifies square tabs. Valid ranges are 
# 0 to 45 degrees inclusive. Default is 15 degrees. If this option 
# is specified as an empty string (the default), then the angle 
# option for the overall Tabset is used.
# ----------------------------------------------------------------------
::itcl::configbody Tab::angle {
    if {($angle < 0) || ($angle > 45)} {
        error "bad angle: must be between 0 and 45"
    }
    set _configTripped true
}

# ----------------------------------------------------------------------
# OPTION -font
#
# Font for tab text.
# ----------------------------------------------------------------------
::itcl::configbody Tab::font { 
}


# ----------------------------------------------------------------------
# OPTION -tabborders
#
# Specifies whether to draw the borders of a deselected tab. 
# Specifying true (the default) draws these borders, 
# specifying false disables this drawing. If the tab is in 
# its selected state this option has no effect. 
# The value may have any of the forms accepted by the 
# Tcl_GetBoolean, such as true, false, 0, 1, yes, or no.
# ----------------------------------------------------------------------
::itcl::configbody Tab::tabborders { 
    set _configTripped true
}

# ----------------------------------------------------------------------
# METHOD: configure ?option value?
#
# Configures the Tab, checks a configTripped flag to see if the tab
# needs to be remade. We take the easy way since it is so inexpensive
# to delete canvas items and remake them.
# ----------------------------------------------------------------------
::itcl::body Tab::configure {args} {
    set len [llength $args]
    switch $len {
    0 {
        set result [::itcl::builtin::configure]
        return $result
      }
    1 {
        set result [uplevel 0 ::itcl::builtin::configure $args]
        return $result
      }
    default {
        uplevel 0 ::itcl::builtin::configure $args
        if {$_configTripped} {
            _makeTab 
            set _configTripped false
        }
        return ""
      }
    }
}

# ----------------------------------------------------------------------
# METHOD: bbox
#
# Returns the bounding box of the tab
# ----------------------------------------------------------------------
::itcl::body Tab::bbox {} {
    return [lappend bbox $_left $_top $_right $_bottom]
}
# ----------------------------------------------------------------------
# METHOD: deselect
#
# Causes the given tab to be drawn as deselected and lowered
# ----------------------------------------------------------------------
::itcl::body Tab::deselect {} {
    global tcl_platform
    $_canvas lower $this
    if {$tcl_platform(os) eq "HP-UX"} {
        update idletasks
    }
    _deselectNoLower
}

# ----------------------------------------------------------------------
# METHOD: lower
#
# Lowers the tab below all others in the canvas.
#
# This is used as our tag name on the canvas.
# ----------------------------------------------------------------------
::itcl::body Tab::lower {} {
    $_canvas lower $this
}

# ----------------------------------------------------------------------
# METHOD: majordim
#
# Returns the width for horizontal tabs and the height for
# vertical tabs.
# ----------------------------------------------------------------------
::itcl::body Tab::majordim {} {
    return $_majorDim  
}

# ----------------------------------------------------------------------
# METHOD: minordim
#
# Returns the height for horizontal tabs and the width for
# vertical tabs.
# ----------------------------------------------------------------------
::itcl::body Tab::minordim {} {
    return $_minorDim  
}

# ----------------------------------------------------------------------
# METHOD: offset
#
# Returns the width less the angle offset. This allows a 
# geometry manager to ask where to place a sibling tab.
# ----------------------------------------------------------------------
::itcl::body Tab::offset {} {
    return $_offset  
}

# ----------------------------------------------------------------------
# METHOD: raise
#
# Raises the tab above all others in the canvas.
#
# This is used as our tag name on the canvas.
# ----------------------------------------------------------------------
::itcl::body Tab::raise {} {
    $_canvas raise $this
}

# ----------------------------------------------------------------------
# METHOD: select
#
# Causes the given tab to be drawn as selected. 3d shadows are
# turned on and top line and top line shadow are drawn in sel
# bg color to hide them.
# ----------------------------------------------------------------------
::itcl::body Tab::select {} {
    global tcl_platform
    $_canvas raise $this
    if {$tcl_platform(os) eq "HP-UX"} {
        update idletasks
    }
    _selectNoRaise
}

# ----------------------------------------------------------------------
# METHOD: labelheight
#
# Returns the height of the tab's label in its current font.
# ----------------------------------------------------------------------
::itcl::body Tab::labelheight {} {
    if {$_gLabel != 0} {
        set labelBBox [$_canvas bbox $_gLabel]
        set labelHeight [expr {[lindex $labelBBox 3] - [lindex $labelBBox 1]}]
    } else {
        set labelHeight 0
    }
    return $labelHeight
}

# ----------------------------------------------------------------------
# METHOD: labelwidth
#
# Returns the width of the tab's label in its current font.
# ----------------------------------------------------------------------
::itcl::body Tab::labelwidth {} {
    if {$_gLabel != 0} {
        set labelBBox [$_canvas bbox $_gLabel]
        set labelWidth [expr {[lindex $labelBBox 2] - [lindex $labelBBox 0]}]
    } else {
        set labelWidth 0
    }
    return $labelWidth
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _selectNoRaise
#
# Draws tab as selected without raising it.
# ----------------------------------------------------------------------
::itcl::body Tab::_selectNoRaise {} {
    if {![info exists _gRegion]} {
        return
    }
    $_canvas itemconfigure $_gRegion -fill $selectbackground
    $_canvas itemconfigure $_gTopLine -fill $selectbackground
    $_canvas itemconfigure $_gTopLineShadow -fill $selectbackground
    $_canvas itemconfigure $_gLightShadow -fill $_lightShadow
    $_canvas itemconfigure $_gDarkShadow -fill $_darkShadow
    if { $_gLightOutline != {} } {
        $_canvas itemconfigure $_gLightOutline -fill $_lightShadow
    }
    if { $_gBlackOutline != {} } {
        $_canvas itemconfigure $_gBlackOutline -fill black
    }
    if {$state eq "normal"} {
        if { $image != {}} {
            # do nothing for now
        } elseif { $bitmap != {}} {
            $_canvas itemconfigure $_gLabel \
                -foreground $selectforeground \
                -background $selectbackground 
        } else {
            $_canvas itemconfigure $_gLabel -fill $selectforeground
        }
    } else {
        if {$image ne {}} {
            # do nothing for now
        } elseif {$bitmap ne {}} {
            $_canvas itemconfigure $_gLabel \
                -foreground $disabledforeground \
                -background $selectbackground 
        } else {
            $_canvas itemconfigure $_gLabel -fill $disabledforeground
        }
    }
    set _selected true
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _deselectNoLower
#
# Causes the given tab to be drawn as deselected. 3d shadows are
# removed and top line and top line shadow are drawn in visible
# colors to reveal them.
# ----------------------------------------------------------------------
::itcl::body Tab::_deselectNoLower {} {
    if {![info exists _gRegion]} {
        return
    }
    $_canvas itemconfigure $_gRegion -fill $background
    $_canvas itemconfigure $_gTopLine -fill black
    $_canvas itemconfigure $_gTopLineShadow -fill $_darkShadow
    $_canvas itemconfigure $_gLightShadow -fill $background
    $_canvas itemconfigure $_gDarkShadow -fill $background
    if {$tabborders} {
        if {$_gLightOutline ne {}} {
            $_canvas itemconfigure $_gLightOutline -fill $_lightShadow
        }
        if {$_gBlackOutline ne {}} {
            $_canvas itemconfigure $_gBlackOutline -fill black
        }
    } else {
        if {$_gLightOutline ne {}} {
            $_canvas itemconfigure $_gLightOutline -fill $background
        }
        if {$_gBlackOutline ne {}} {
            $_canvas itemconfigure $_gBlackOutline -fill $background
        }
    }
    if {$state eq "normal"} {
        if {$image ne {}} {
            # do nothing for now
        } elseif {$bitmap ne {}} {
            $_canvas itemconfigure $_gLabel \
                -foreground $foreground \
                -background $background 
        } else {
            $_canvas itemconfigure $_gLabel -fill $foreground
        }
    } else {
        if {$image ne {}} {
            # do nothing for now
        } elseif {$bitmap ne {}} {
            $_canvas itemconfigure $_gLabel \
                -foreground $disabledforeground \
                -background $background 
        } else {
            $_canvas itemconfigure $_gLabel -fill $disabledforeground
        }
    }
    set _selected false
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _makeTab
# ----------------------------------------------------------------------
::itcl::body Tab::_makeTab {} {
    if {$orient eq "horizontal"} {
        if {$invert} {
            _makeNorthTab $_canvas
        } else {
            _makeSouthTab $_canvas
        }
    } elseif {$orient eq "vertical"} {
        if {$invert} {
            _makeEastTab $_canvas
        } else {
            _makeWestTab $_canvas
        }
    } else {
        error "bad value for option -orient"
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _createLabel
#
# Creates the label for the tab. Can be either a text label
# or a bitmap label.
# ----------------------------------------------------------------------
::itcl::body Tab::_createLabel {canvas tagList} {
    if {$image ne {}} {
        set _gLabel [$canvas create image \
            0 0 \
            -image $image \
            -anchor nw \
            -tags $tagList \
            ]
    } elseif { $bitmap != {}} {
        set _gLabel [$canvas create bitmap \
            0 0 \
            -bitmap $bitmap \
            -anchor nw \
            -tags $tagList \
            ]
    } else {
        set _gLabel [$canvas create text \
            0 0 \
            -text $label \
            -font $font \
            -anchor nw \
            -tags $tagList \
            ]
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _makeEastTab
#
# Makes a tab that hangs to the east and opens to the west.
# ----------------------------------------------------------------------
::itcl::body Tab::_makeEastTab {canvas} {
    $canvas delete $this
    set _gLightOutline {}
    set _gBlackOutline {}
    lappend tagList $this TAB
    _createLabel $canvas $tagList
    _calcLabelDim $_gLabel
    set right  [expr {$_left + $_labelWidth}]
    # now have _left, _top, right...
    # Turn off calculating angle tabs on Vertical orientations
    set angleOffset 0
    set outerTop $_top
    set outerBottom \
        [expr {$outerTop + $angleOffset + $_labelHeight + $angleOffset}]
    set innerTop [expr {$outerTop + $angleOffset}]
    set innerBottom [expr {$outerTop + $angleOffset + $_labelHeight}]
    
    # now have _left, _top, right, outerTop, innerTop,
    # innerBottom, outerBottom, width, height
    set bottom $innerBottom
    # tab area... gets filled either white or selected
    # done
    set _gRegion [$canvas create polygon \
        $_left $outerTop \
        [expr {$right - $bevelamount}] $innerTop \
        $right [expr {$innerTop + $bevelamount}] \
        $right [expr {$innerBottom - $bevelamount}] \
        [expr {$right - $bevelamount}] $innerBottom \
        $_left $outerBottom \
        $_left $outerTop \
        -tags $tagList  \
        ]
    
    # lighter shadow (left edge)
    set _gLightShadow [$canvas create line \
        [expr {$_left - 3}] [expr {$outerTop + 1}] \
        [expr {$right - $bevelamount}] [expr {$innerTop + 1}] \
        -tags $tagList \
        ]
    
    # darker shadow (bottom and right edges)
    set _gDarkShadow [$canvas create line \
        [expr {$right - $bevelamount}] [expr {$innerTop + 1}] \
        [expr {$right - 1}] [expr {$innerTop + $bevelamount}] \
        [expr {$right - 1}] [expr {$innerBottom - $bevelamount}] \
        [expr {$right - $bevelamount}] [expr {$innerBottom - 1}] \
        [expr {$_left - 3}] [expr {$outerBottom - 1}] \
        -tags $tagList \
        ]
    
    # outline of tab
    set _gLightOutline [$canvas create line \
        $_left $outerTop \
        [expr {$right - $bevelamount}] $innerTop \
        -tags $tagList \
        ]
    # outline of tab
    set _gBlackOutline [$canvas create line \
        [expr {$right - $bevelamount}] $innerTop \
        $right [expr {$innerTop + $bevelamount}] \
        $right [expr {$innerBottom - $bevelamount}] \
        [expr {$right - $bevelamount}] $innerBottom \
        $_left $outerBottom \
        $_left $outerTop \
        -tags $tagList \
        ]
    
    # line closest to the edge
    set _gTopLineShadow [$canvas create line \
        $_left $outerTop \
        $_left $outerBottom \
        -tags $tagList \
        ]
    
    # next line down
    set _gTopLine [$canvas create line \
        [expr {$_left + 1}] [expr {$outerTop + 2}] \
        [expr {$_left + 1}] [expr {$outerBottom - 1}] \
        -tags $tagList  \
        ]
    $canvas coords $_gLabel [expr {$_left + $_labelXOrigin}] \
        [expr {$innerTop + $_labelYOrigin}]
    if {($image ne {}) || ($bitmap ne {})} {
        $canvas itemconfigure $_gLabel -anchor $anchor
    } else {
        $canvas itemconfigure $_gLabel -anchor $anchor -justify $_just
    }
    $canvas raise $_gLabel $_gRegion
    set _offset    [expr {$innerBottom - $outerTop}]
    # height
    set _majorDim  [expr {$outerBottom - $outerTop}]
    # width
    set _minorDim [expr {$right - $_left}]
    set _right   $right 
    set _bottom  $outerBottom
    
    # draw in correct state...
    if { $_selected } {
        select
    } else {
        deselect
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _makeWestTab
#
# Makes a tab that hangs to the west and opens to the east.
# ----------------------------------------------------------------------
::itcl::body Tab::_makeWestTab {canvas} {
    $canvas delete $this
    set _gLightOutline {}
    set _gBlackOutline {}
    lappend tagList $this TAB
    _createLabel $canvas $tagList
    _calcLabelDim $_gLabel
    set right  [expr {$_left + $_labelWidth}]
    # now have _left, _top, right...
    # Turn off calculating angle tabs on Vertical orientations
    set angleOffset 0
    set outerTop $_top
    set outerBottom \
        [expr {$outerTop + $angleOffset + $_labelHeight + $angleOffset}]
    set innerTop [expr {$outerTop + $angleOffset}]
    set innerBottom [expr {$outerTop + $angleOffset + $_labelHeight}]
    
    # now have _left, _top, right, outerTop, innerTop,
    # innerBottom, outerBottom, width, height
    # tab area... gets filled either white or selected
    # done
    set _gRegion [$canvas create polygon \
        $right $outerTop \
        [expr {$_left + $bevelamount}] $innerTop \
        $_left [expr {$innerTop + $bevelamount}] \
        $_left [expr {$innerBottom - $bevelamount}]\
        [expr {$_left + $bevelamount}] $innerBottom \
        $right $outerBottom \
        $right $outerTop \
        -tags $tagList  \
        ]
    # lighter shadow (left edge)
    set _gLightShadow [$canvas create line \
        $right [expr {$outerTop+1}] \
        [expr {$_left + $bevelamount}] [expr {$innerTop + 1}] \
        [expr {$_left + 1}] [expr {$innerTop + $bevelamount}] \
        [expr {$_left + 1}] [expr {$innerBottom - $bevelamount}] \
        -tags $tagList \
        ]
    
    # darker shadow (bottom and right edges)
    set _gDarkShadow [$canvas create line \
        [expr {$_left + 1}] [expr {$innerBottom - $bevelamount}] \
        [expr {$_left + $bevelamount}] [expr {$innerBottom - 1}] \
        $right [expr {$outerBottom - 1}] \
        -tags $tagList \
        ]
    
    # outline of tab -- lighter top left sides
    set _gLightOutline [$canvas create line \
        $right $outerTop \
        [expr {$_left + $bevelamount}] $innerTop \
        $_left [expr {$innerTop + $bevelamount}] \
        $_left [expr {$innerBottom - $bevelamount}]\
        -tags $tagList \
        ]
    # outline of tab -- darker bottom side
    set _gBlackOutline [$canvas create line \
        $_left [expr {$innerBottom - $bevelamount}]\
        [expr {$_left + $bevelamount}] $innerBottom \
        $right $outerBottom \
        $right $outerTop \
        -tags $tagList \
        ]
    
    # top of tab
    set _gTopLine [$canvas create line \
        [expr {$right + 1}] $outerTop \
        [expr {$right + 1}] $outerBottom \
        -tags $tagList  \
        ]
    
    # line below top of tab
    set _gTopLineShadow [$canvas create line \
        $right $outerTop \
        $right $outerBottom \
        -tags $tagList \
        ]
    
    $canvas coords $_gLabel [expr {$_left + $_labelXOrigin}] \
        [expr {$innerTop + $_labelYOrigin}]
    if {($image ne {}) || ($bitmap ne {})} {
        $canvas itemconfigure $_gLabel -anchor $anchor 
    } else {
        $canvas itemconfigure $_gLabel -anchor $anchor -justify $_just
    }
    $canvas raise $_gLabel $_gRegion
    
    set _offset    [expr {$innerBottom - $outerTop}]
    # height
    set _majorDim  [expr {$outerBottom - $outerTop}]
    # width
    set _minorDim [expr {$right - $_left}]
    
    set _right   $right 
    set _bottom  $outerBottom
    
    # draw in correct state...
    if {$_selected} {
        select
    } else {
        deselect
    }
    
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _makeNorthTab
#
# Makes a tab that hangs to the north and opens to the south.
# ----------------------------------------------------------------------
::itcl::body Tab::_makeNorthTab {canvas} {
    $canvas delete $this
    set _gLightOutline {}
    set _gBlackOutline {}
    
    lappend tagList $this TAB
    
    _createLabel $canvas $tagList
    
    # first get the label width and height
    _calcLabelDim $_gLabel
    
    set bottom [expr {$_top + $_labelHeight}]
    
    set angleOffset [expr {$_labelHeight * $_tan($angle)}]
    
    set outerLeft $_left
    set outerRight \
        [expr {$outerLeft + $angleOffset + $_labelWidth + $angleOffset}]
    set innerLeft [expr {$outerLeft + $angleOffset}]
    set innerRight [expr {$outerLeft + $angleOffset + $_labelWidth}]
    
    # tab area... gets filled either white or selected
    set _gRegion [$canvas create polygon \
            $outerLeft [expr {$bottom + 3}]  \
            $innerLeft [expr {$_top + $bevelamount}] \
            [expr {$innerLeft +  $bevelamount}] $_top \
            [expr {$innerRight - $bevelamount}] $_top \
            $innerRight [expr {$_top + $bevelamount}]\
            $outerRight [expr {$bottom + 3}] \
            $outerLeft [expr {$bottom + 3}] \
        -tags $tagList  \
        ]
    
    # lighter shadow (left edge)
    set _gLightShadow [$canvas create line \
            [expr {$outerLeft + 1}] [expr {$bottom + 3}]  \
            [expr {$innerLeft + 1}] [expr {$_top + $bevelamount}] \
            [expr {$innerLeft + $bevelamount}] [expr {$_top + 1}]\
            [expr {$innerRight - $bevelamount}] [expr {$_top + 1}]\
        -tags $tagList \
        ]
    
    # darker shadow (bottom and right edges)
    set _gDarkShadow [$canvas create line \
            [expr {$innerRight - $bevelamount}] [expr {$_top + 1}]\
            [expr {$innerRight - 1}] [expr {$_top + $bevelamount}]\
            [expr {$outerRight - 1}] [expr {$bottom + 3}]\
        -tags $tagList \
        ]
    
    set _gLightOutline [$canvas create line \
            $outerLeft [expr {$bottom + 3}]  \
            $innerLeft [expr {$_top + $bevelamount}] \
            [expr {$innerLeft +  $bevelamount}] $_top \
            [expr {$innerRight - $bevelamount}] $_top \
        -tags $tagList \
        ]
    
    set _gBlackOutline [$canvas create line \
            [expr {$innerRight - $bevelamount}] $_top \
            $innerRight [expr {$_top + $bevelamount}]\
            $outerRight [expr {$bottom + 3}] \
            $outerLeft [expr {$bottom + 3}] \
        -tags $tagList \
        ]
    
    # top of tab... to make it closed off
    set _gTopLine [$canvas create line \
        0 0 0 0\
        -tags $tagList  \
        ]
    
    # top of tab... to make it closed off
    set _gTopLineShadow [$canvas create line \
        0 0 0 0 \
        -tags $tagList \
        ]
    
    $canvas coords $_gLabel [expr {$innerLeft + $_labelXOrigin}] \
        [expr {$_top + $_labelYOrigin}]
    
    if { $image != {} || $bitmap != {} } {
        $canvas itemconfigure $_gLabel -anchor $anchor 
    } else {
        $canvas itemconfigure $_gLabel -anchor $anchor -justify $_just
    }
    $canvas raise $_gLabel $_gRegion
    
    set _offset    [expr {$innerRight - $outerLeft}]
    # width
    set _majorDim  [expr {$outerRight - $outerLeft}]
    # height
    set _minorDim [expr {$bottom - $_top}]
    
    set _right     $outerRight
    set _bottom    $bottom
    
    # draw in correct state...
    if {$_selected} {
        select
    } else {
        deselect
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _makeSouthTab
#
# Makes a tab that hangs to the south and opens to the north.
# ----------------------------------------------------------------------
::itcl::body Tab::_makeSouthTab {canvas} {
    $canvas delete $this
    set _gLightOutline {}
    set _gBlackOutline {}
    lappend tagList $this TAB
    _createLabel $canvas $tagList
    # first get the label width and height
    _calcLabelDim $_gLabel
    set bottom [expr {$_top + $_labelHeight}]
    set angleOffset [expr {$_labelHeight * $_tan($angle)}]
    set outerLeft $_left
    set outerRight \
            [expr {$outerLeft + $angleOffset + $_labelWidth + $angleOffset}]
     set innerLeft [expr {$outerLeft + $angleOffset}]
     set innerRight [expr {$outerLeft + $angleOffset + $_labelWidth}]
    
    # tab area... gets filled either white or selected
    set _gRegion [$canvas create polygon \
            $outerLeft [expr {$_top + 1}] \
            $innerLeft [expr {$bottom  - $bevelamount}]\
            [expr {$innerLeft + $bevelamount}] $bottom \
            [expr {$innerRight - $bevelamount}] $bottom \
            $innerRight [expr {$bottom - $bevelamount}]\
            $outerRight [expr {$_top + 1}] \
            $outerLeft [expr {$_top + 1}] \
        -tags $tagList  \
        ]
    
    
    # lighter shadow (left edge)
    set _gLightShadow [$canvas create line \
            [expr {$outerLeft+1}] $_top \
            [expr {$innerLeft+1}] [expr {$bottom-$bevelamount}] \
        -tags $tagList \
        ]
    
    # darker shadow (bottom and right edges)
    set _gDarkShadow [$canvas create line \
            [expr {$innerLeft+1}] [expr {$bottom-$bevelamount}] \
            [expr {$innerLeft+$bevelamount}] [expr {$bottom-1}] \
            [expr {$innerRight-$bevelamount}] [expr {$bottom-1}] \
            [expr {$innerRight-1}] [expr {$bottom-$bevelamount}] \
            [expr {$outerRight-1}] [expr {$_top + 1}] \
        -tags $tagList \
        ]
    # outline of tab
    set _gBlackOutline [$canvas create line \
            $outerLeft [expr {$_top + 1}] \
            $innerLeft [expr {$bottom  -$bevelamount}]\
            [expr {$innerLeft + $bevelamount}] $bottom \
            [expr {$innerRight - $bevelamount}] $bottom \
            $innerRight [expr {$bottom - $bevelamount}]\
            $outerRight [expr {$_top + 1}] \
        -tags $tagList \
        ]
    
    # top of tab... to make it closed off
    set _gTopLine [$canvas create line \
            $outerLeft [expr {$_top + 1}] \
            $outerRight [expr {$_top + 1}] \
        -tags $tagList  \
        ]
    
    # top of tab... to make it closed off
    set _gTopLineShadow [$canvas create line \
        $outerLeft $_top \
        $outerRight $_top \
        -tags $tagList \
        ]
    
     $canvas coords $_gLabel [expr {$innerLeft + $_labelXOrigin}] \
            [expr {$_top + $_labelYOrigin}]
    
    if {($image ne {}) || ($bitmap ne {})} {
        $canvas itemconfigure $_gLabel -anchor $anchor 
    } else {
        $canvas itemconfigure $_gLabel -anchor $anchor -justify $_just
    }
    $canvas raise $_gLabel $_gRegion
    
    set _offset    [expr {$innerRight - $outerLeft}]
    # width
    set _majorDim  [expr {$outerRight - $outerLeft}]
    # height
    set _minorDim [expr {$bottom - $_top}]
    set _right     $outerRight
    set _bottom    $bottom
    # draw in correct state...
    if {$_selected} {
        select
    } else {
        deselect
    }
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _calcLabelDim
#
# Calculate the width and height of the label bbox of labelItem
# can be either text or bitmap (in future also an image)
#
# There are two ways to calculate the label bbox.
#
# First, if the $_width and/or $_height is specified, we will use
# it to determine that dimension(s) width and/or height. For 
# a width/height of 0 we use the labels bbox to
# give us a base width/height.
# Then we add in the padx/pady to determine final bounds.
#
# Uses the following option or option derived variables:
#   -padx     ($_padX - converted to pixels)
#   -pady     ($_padY - converted to pixels)
#   -anchor   ($anchor)
#   -width    ($_width) This is the width for inside tab (label area)
#   -height   ($_height) This is the width for inside tab (label area)
#
# Side Effects:
#   _labelWidth will be set
#   _labelHeight will be set
#   _labelXOrigin will be set
#   _labelYOrigin will be set
# ----------------------------------------------------------------------
::itcl::body Tab::_calcLabelDim {labelItem} {
    # ... calculate the label width and height
    set labelBBox [$_canvas bbox $labelItem]
    if {$_width > 0} {
        set _labelWidth [expr {$_width + ($_padX * 2)}]
    } else {
        set _labelWidth [expr {
              ([lindex $labelBBox 2] - [lindex $labelBBox 0]) + ($_padX * 2)}]
    }
    
    if {$_height > 0} {
        set _labelHeight [expr {$_height + ($_padY * 2)}]
    } else {
        set _labelHeight [expr {
          ([lindex $labelBBox 3] - [lindex $labelBBox 1]) + ($_padY * 2)}]
    }
    
    # ... calculate the label anchor point
    set centerX [expr {$_labelWidth/2.0}]
    set centerY [expr {$_labelHeight/2.0 - 1}]
    switch $anchor {
    n {
        set _labelXOrigin $centerX
        set _labelYOrigin $_padY
        set _just center
      }
    s {
        set _labelXOrigin $centerX
        set _labelYOrigin [expr {$_labelHeight - $_padY}]
        set _just center
      }
    e {
        set _labelXOrigin [expr {$_labelWidth - $_padX - 1}]
        set _labelYOrigin $centerY
        set _just right
      }
    w {
        set _labelXOrigin [expr {$_padX + 2}]
        set _labelYOrigin $centerY
        set _just left
      }
    c {
        set _labelXOrigin $centerX
        set _labelYOrigin $centerY
        set _just center
      }
    ne {
        set _labelXOrigin [expr {$_labelWidth - $_padX - 1}]
        set _labelYOrigin $_padY
        set _just right
      }
    nw {
        set _labelXOrigin [expr {$_padX + 2}]
        set _labelYOrigin $_padY
        set _just left
      }
    se {
        set _labelXOrigin [expr {$_labelWidth - $_padX - 1}]
        set _labelYOrigin [expr {$_labelHeight - $_padY}]
        set _just right
      }
    sw {
        set _labelXOrigin [expr {$_padX + 2}]
        set _labelYOrigin [expr {$_labelHeight - $_padY}]
        set _just left
      }
    default {
        error "bad anchor position: \
            \"$tabpos\" must be n, ne, nw, s, sw, se, e, w, or center"
      }
    }
}

} ; # end ::itcl::widgets
