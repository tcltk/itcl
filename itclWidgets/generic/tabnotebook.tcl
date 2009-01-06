#
# Tabnotebook Widget
# ----------------------------------------------------------------------
# The Tabnotebook command creates a new window (given by the pathName 
# argument) and makes it into a Tabnotebook widget. Additional options, 
# described above may be specified on the command line or in the option 
# database to configure aspects of the Tabnotebook such as its colors, 
# font, and text. The Tabnotebook command returns its pathName argument. 
# At the time this command is invoked, there must not exist a window 
# named pathName, but pathName's parent must exist.
# 
# A Tabnotebook is a widget that contains a set of tabbed pages. It 
# displays one page from the set as the selected page. A Tab displays 
# the label for the page to which it is attached and serves as a page 
# selector.   When a page's tab is selected, the page's contents are 
# displayed in the page area. The selected tab has a three-dimensional 
# effect to make it appear to float above the other tabs. The tabs are 
# displayed as a group along either the left, top, right, or bottom 
# edge. When first created a Tabnotebook has no pages. Pages may be 
# added or deleted using widget commands described below.
# 
# A special option may be provided to the Tabnotebook. The -auto 
# option specifies whether the Tabnotebook will automatically handle 
# the unpacking and packing of pages when pages are selected. A value 
# of true sig nifies that the notebook will automatically manage it. This 
# is the default value. A value of false signifies the notebook will not 
# perform automatic switching of pages.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Tabnotebook
# written by:
#    AUTHOR: Bill W. Scott
#    CURRENT MAINTAINER: Chad Smith --> csmith@adc.com or itclguy@yahoo.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: tabnotebook.tcl,v 1.1.2.3 2009/01/06 22:03:43 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Tabnotebook.borderWidth 2 widgetDefault
option add *Tabnotebook.state normal widgetDefault
option add *Tabnotebook.disabledForeground #a3a3a3 widgetDefault
option add *Tabnotebook.scrollCommand {} widgetDefault
option add *Tabnotebook.equalTabs true widgetDefault
option add *Tabnotebook.font \
	-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-* widgetDefault
option add *Tabnotebook.width 300 widgetDefault
option add *Tabnotebook.height 150 widgetDefault
option add *Tabnotebook.foreground Black widgetDefault
option add *Tabnotebook.background #d9d9d9 widgetDefault
option add *Tabnotebook.tabForeground Black widgetDefault
option add *Tabnotebook.tabBackground #d9d9d9 widgetDefault
option add *Tabnotebook.backdrop #d9d9d9 widgetDefault
option add *Tabnotebook.margin 4 widgetDefault
option add *Tabnotebook.tabBorders true widgetDefault
option add *Tabnotebook.bevelAmount 0 widgetDefault
option add *Tabnotebook.raiseSelect false widgetDefault
option add *Tabnotebook.auto true widgetDefault
option add *Tabnotebook.start 4 widgetDefault
option add *Tabnotebook.padX 4 widgetDefault
option add *Tabnotebook.padY 4 widgetDefault
option add *Tabnotebook.gap overlap widgetDefault
option add *Tabnotebook.angle 15 widgetDefault
option add *Tabnotebook.tabPos s widgetDefault

namespace eval ::itcl::widgets {

proc ::itcl::widgets::tabnotebook {pathName args} {
    uplevel ::itcl::widgets::Tabnotebook $pathName $args
}

# ------------------------------------------------------------------
#                            TABNOTEBOOK
# ------------------------------------------------------------------
::itcl::extendedclass Tabnotebook {
    component itcl_hull
    component itcl_interior
    component canvas
    component notebook
    component tabset
    
    option [list -borderwidth borderWidth BorderWidth] -default 2 -configuremethod configBorderwidth
    option [list -state state State] -default normal -configuremethod configState
    option [list \
	    -disabledforeground disabledForeground DisabledForeground] -default #a3a3a3 -configuremethod configDisabledforeground
    option [list -scrollcommand scrollCommand ScrollCommand] -default {} -configuremethod configScrollcommand
    option [list -equaltabs equalTabs EqualTabs] -default true -configuremethod configEqualtabs
    option [list -font font Font ] -default \
	    -Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-* -configuremethod configFont
    option [list -width width Width] -default 300 -configuremethod configWidth
    option [list -height height Height] -default 150 -configuremethod configHeight
    option [list -foreground foreground Foreground] -default Black -configuremethod configForeground
    option [list -background background Background] -default #d9d9d9 -configuremethod configBackground
    option [list -tabforeground tabForeground TabForeground] -default Black -configuremethod configTabforeground
    option [list -tabbackground tabBackground TabBackground] -default #d9d9d9 -configuremethod configTabbackground
    option [list -backdrop backdrop Backdrop] -default #d9d9d9 -configuremethod configBackdrop
    option [list -margin margin Margin] -default 4  -configuremethod configMargin
    option [list -tabborders tabBorders TabBorders] -default true -configuremethod configTabborders
    option [list -bevelamount bevelAmount BevelAmount] -default 0 -configuremethod configBevelamount
    option [list -raiseselect raiseSelect RaiseSelect] -default false -configuremethod configRaiseselect
    option [list -auto auto Auto] -default true -configuremethod configAuto
    option [list -start start Start] -default 4 -configuremethod configStart
    option [list -padx padX PadX] -default 4 -configuremethod configPadx
    option [list -pady padY PadY] -default 4 -configuremethod configPady
    option [list -gap gap Gap] -default overlap -configuremethod configGap
    option [list -angle angle Angle] -default 15 -configuremethod configAngle
    option [list -tabpos tabPos TabPos] -default s -configuremethod configTabpos
    
    private variable _canvasWidth 0       ;# currently tabnote canvas width
    private variable _canvasHeight 0      ;# currently tabnote canvas height
    private variable _nbOptList {}        ;# list of notebook options available
    private variable _tsOptList {}        ;# list of tabset options available
    private variable _tabPos s            ;# holds -tabPos, because of ordering
    private variable _borderRecompute false   ;# did we dirty border after cfg?
    private variable _tabsetReconfigure false ;# did we dirty tabsets after cfg?
    
    constructor {args} {}
    destructor {}
    
    private method _getArgs {optList args}
    private method _redrawBorder {wid hgt} 
    private method _recomputeBorder {}
    private method _pack {tabPos} 
    private method _resize {newWidth_ newHeight_}
    
    protected method _reconfigureTabset {} 
    protected method _canvasReconfigure {wid hgt}
    protected method _pageReconfigure {pageName page wid hgt}
    protected method configBorderwidth {option value}
    protected method configState {option value}
    protected method configDisabledforeground {option value}
    protected method configScrollcommand {option value}
    protected method configEqualtabs {option value}
    protected method configFont {option value}
    protected method configWidth {option value}
    protected method configHeight {option value}
    protected method configForeground {option value}
    protected method configBackground {option value}
    protected method configTabforeground {option value}
    protected method configTabbackground {option value}
    protected method configBackdrop {option value}
    protected method configMargin {option value}
    protected method configTabborders {option value}
    protected method configBevelamount {option value}
    protected method configRaiseselect {option value}
    protected method configAuto {option value}
    protected method configStart {option value}
    protected method configPadx {option value}
    protected method configPady {option value}
    protected method configGap {option value}
    protected method configAngle {option value}
    protected method configTabpos {option value}
    
    public method add {args}
    public method configure {args} 
    public method childsite {args}
    public method delete {args} 
    public method index {args} 
    public method insert {index args}
    public method prev {} 
    public method next {}
    public method pageconfigure {index args} 
    public method select {index} 
    public method view {args} 

    public method NBSelect {index}
}


# ----------------------------------------------------------------------
#                              CONSTRUCTOR
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::constructor {args} {
    createhull frame $this -class [info class] -borderwidth 0 -padx 0 -pady 0
    #
    # Create the outermost canvas to maintain geometry.
    #
    setupcomponent canvas using canvas $itcl_interior.canvas -highlightthickness 0
    keepcomponentoption canvas -cursor -background -width -height
    bind $canvas <Configure> [itcl::code $this _resize %w %h]

    # .......................
    # Create the NOTEBOOK
    #
    setupcomponent notebook using ::itcl::widgets::Notebook $itcl_interior.canvas.notebook 
    keepcomponentoption notebook -cursor -background 
    
    #
    # Ouch, create a dummy page, go pageconfigure to get its options
    # and munge them into a list for later doling by pageconfigure
    #
    $notebook add
    set nbConfigList [$notebook pageconfigure 0]
    foreach config $nbConfigList {
	lappend _nbOptList [lindex $config 0]
    }
    $notebook delete 0
    
    # 
    # Create the tabset.
    #
    setupcomponent tabset using ::itcl::widgets::Tabset \
            $itcl_interior.canvas.tabset -command [itcl::code $this NBSelect]
    keepcomponentoption tabset -cursor 
    
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
    
    #
    # Ouch, create a dummy tab, go tabconfigure to get its options
    # and munge them into a list for later doling by pageconfigure
    #
    $tabset add
    set tsConfigList [$tabset tabconfigure 0]
    foreach config $tsConfigList {
	lappend _tsOptList [lindex $config 0]
    }
    $tabset delete 0
    
    bind $tabset <Configure> \
	    [itcl::code $this _reconfigureTabset]
    
    _pack $_tabPos
    $win configure -width [cget -width] -height [cget -height]
}


# -------------------------------------------------------------
# DESTRUCTOR: destroy the Tabnotebook
# -------------------------------------------------------------
::itcl::body Tabnotebook::destructor {} {
}

# ----------------------------------------------------------------------
# OPTION -borderwidth
#
# Thickness of Notebook Border
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configBorderwidth {option value} {
    if {$value ne {}} {
	#_recomputeBorder
	set _borderRecompute true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -state
#
# State of the tabs in the tab notebook: normal or disabled
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configState {option value} {
    if {$value ne {}} {
	$tabset configure -state $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -disabledforeground
#
# Specifies a foreground color to use for displaying a 
# tab's label when its state is disabled.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configDisabledforeground {option value} {
    if {$value ne {}} {
	$tabset configure \
		-disabledforeground $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -scrollcommand
#
# Standard option. See options man pages.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configScrollcommand {option value} {
    if {$value ne {}} {
	$notebook configure -scrollcommand $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -equaltabs
#
# Specifies whether to force tabs to be equal sized or not. 
# A value of true means constrain tabs to be equal sized. 
# A value of false allows each tab to size based on the text 
# label size. The value may have any of the forms accepted by 
# the Tcl_GetBoolean, such as true, false, 0, 1, yes, or no.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configEqualtabs {option value} {
    if {$value ne {}} {
	$tabset configure -equaltabs $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -font
#
# Font for tab labels when they are set to text (-label set)
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configFont {option value} {
    if {$value ne {}} {
	$tabset configure -font $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -width
#
# Width of the Tabnotebook
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configWidth {option value} {
    if {$value ne {}} {
	$canvas configure -width $value
	#_recomputeBorder
	set _borderRecompute true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -height
#
# Height of the Tabnotebook
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configHeight {option value} {
    if {$value ne {}} {
	$canvas configure -height $value
	#_recomputeBorder
	set _borderRecompute true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -foreground
#
# Specifies a foreground color to use for displaying a page 
# and its associated tab label (this is the selected state).
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configForeground {option value} {
    if {$value ne {}} {
	$tabset configure -selectforeground $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -background
#
# Specifies a background color to use for displaying a page 
# and its associated tab bg (this is the selected state).
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configBackground {option value} {
    if {$value ne {}} {
	$tabset configure -selectbackground $value
	#_recomputeBorder
	set _borderRecompute true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -tabforeground
#
# Specifies a foreground color to use for displaying tab labels 
# when they are in their unselected state.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configTabforeground {option value} {
    if {$value ne {}} {
	$tabset configure -foreground $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -tabbackground
#
# Specifies a background color to use for displaying tab backgrounds 
# when they are in their unselected state.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configTabbackground {option value} {
    if {$value ne {}} {
	$tabset configure -background $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -backdrop
#
# Specifies a background color to use when filling in the 
# area behind the tabs.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configBackdrop {option value} {
    if {$value ne {}} {
	$tabset configure -backdrop $value
    }
}

# ----------------------------------------------------------------------
# OPTION -margin
#
# Sets the backdrop margin between tab edge and backdrop edge
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configMargin {option value} {
    if {$value ne {}} {
	$tabset configure -margin $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -tabborders
#
# Boolean that specifies whether to draw the borders of
# the unselected tabs (tabs in background)
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configTabborders {option value} {
    if {$value ne {}} {
	$tabset configure -tabborders $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -bevelamount
#
# Specifies pixel size of tab corners. 0 means no corners.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configBevelamount {option value} {
    if {$value ne {}} {
	$tabset configure -bevelamount $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -raiseselect
#
# Sets whether to raise selected tabs
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configRaiseselect {option value} {
    if {$value ne {}} {
	$tabset configure -raiseselect $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -auto
#
# Determines whether pages are automatically unpacked and
# packed when pages get selected.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configAuto {option value} {
    if {$value ne {}} {
	$notebook configure -auto $value
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -start
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configStart {option value} {
    if {$value ne {}} {
	$tabset configure -start $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -padx
#
# Specifies a non-negative value indicating how much extra space 
# to request for a tab around its label in the X-direction. 
# When computing how large a window it needs, the tab will add 
# this amount to the width it would normally need The tab will 
# end up with extra internal space to the left and right of its 
# text label. This value may have any of the forms acceptable 
# to Tk_GetPixels.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configPadx {option value} {
    if {$value ne {}} {
	$tabset configure -padx $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -pady
#
# Specifies a non-negative value indicating how much extra space to 
# request for a tab around its label in the Y-direction. When computing 
# how large a window it needs, the tab will add this amount to the 
# height it would normally need The tab will end up with extra internal 
# space to the top and bot tom of its text label. This value may have 
# any of the forms acceptable to Tk_GetPixels.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configPady {option value} {
    if {$value ne {}} {
	$tabset configure -pady $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -gap
#
# Specifies the amount of pixel space to place between each tab. 
# Value may be any pixel offset value. In addition, a special keyword 
# 'overlap' can be used as the value to achieve a standard overlap of 
# tabs. This value may have any of the forms acceptable to Tk_GetPixels.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configGap {option value} {
    if {$value ne {}} {
	$tabset configure -gap $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -angle
#
# Specifes the angle of slope from the inner edge to the outer edge 
# of the tab. An angle of 0 specifies square tabs. Valid ranges are 
# 0 to 45 degrees inclusive. Default is 15 degrees. If tabPos is 
# e or w, this option is ignored.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configAngle {option value} {
    if {$value ne {}} {
	$tabset configure -angle $value
	#_reconfigureTabset
	set _tabsetReconfigure true
    }
    set itcl_options($option) $value
}

# ----------------------------------------------------------------------
# OPTION -tabpos
#
# Specifies the location of the set of tabs in relation to the 
# Notebook area. Must be n, s, e, or w.   Defaults to s.
# ----------------------------------------------------------------------
::itcl::body Tabnotebook::configTabpos {option value} {
    if {$value ne {}} {
	set _tabPos $value
	$tabset configure -tabpos $value
	pack forget $canvas
	pack forget $tabset
	pack forget $notebook
	_pack $_tabPos
    }
    set itcl_options($option) $value
}

# -------------------------------------------------------------
# METHOD: configure ?<option>? ?<value> <option> <value>...?
#
# Acts as an addendum to the itk::Widget::configure method.
#
# Checks the _recomputeBorder flag and the _tabsetReconfigure to
# determine what work has been batched to after the configure
# -------------------------------------------------------------
::itcl::body Tabnotebook::configure {args} {
    set result [eval itcl::builtin::configure $args]
    # check for flags then do update...
    if {$_borderRecompute eq "true"} { 
	_recomputeBorder
	set _borderRecompute false
    }
    if {$_tabsetReconfigure eq "true"} { 
	_reconfigureTabset
	set _tabsetReconfigure false
    }
    return $result
}

# -------------------------------------------------------------
# METHOD: NBSelect <index>
# Callback for selection of a notebook
#
# -------------------------------------------------------------
::itcl::body Tabnotebook::NBSelect {index} {
        $notebook select $index
}
# -------------------------------------------------------------
# METHOD: add ?<option> <value>...?
#
# Creates a page and appends it to the list of pages.
# processes pageconfigure for the page added.
#
# Returns the page's childsite frame
# -------------------------------------------------------------
::itcl::body Tabnotebook::add {args} {
    # The args list should be an even # of params, if not then
    # prob missing value for last item in args list. Signal error.
    set len [llength $args]
    if {[expr {$len % 2}]} {
	error "value for \"[lindex $args [expr {$len - 1}]]\" missing"
    }
    # pick out the notebook args
    set nbArgs [uplevel 0 _getArgs [list $_nbOptList] $args]
    set pageName [uplevel 0 $notebook add $nbArgs]
    
    # pick out the tabset args
    set tsArgs [eval _getArgs [list $_tsOptList] $args]
    uplevel 0 $tabset add $tsArgs
    set page [index end]
    bind $pageName <Configure> \
	    [itcl::code $this _pageReconfigure $pageName $page %w %h]
    return $pageName
}

# -------------------------------------------------------------
# METHOD: childsite ?<index>?
#
# If index is supplied, returns the child site widget 
# corresponding to the page index.  If called with no arguments,
# returns a list of all child sites
# -------------------------------------------------------------
::itcl::body Tabnotebook::childsite {args} {
    return [uplevel 0 $notebook childsite $args]
}

# -------------------------------------------------------------
# METHOD: delete <index1> ?<index2>?
#
# Deletes a page or range of pages from the notebook
# -------------------------------------------------------------
::itcl::body Tabnotebook::delete {args} {
    uplevel 0 $notebook delete $args
    uplevel 0 $tabset delete $args
}

# -------------------------------------------------------------
# METHOD: index <index>
#
# Given an index identifier returns the numeric index of the page
# -------------------------------------------------------------
::itcl::body Tabnotebook::index {args} {
    return [uplevel 0 $notebook index $args]
}

# -------------------------------------------------------------
# METHOD: insert <index> ?<option> <value>...?
#
# Inserts a page before a index. The before page may
# be specified as a label or a page position.
#
# Note that since we use eval to preserve the $args list,
# we must use list around $index to keep it together as a unit
#
# Returns the name of the page's child site
# -------------------------------------------------------------
::itcl::body Tabnotebook::insert {index args} {
    # pick out the notebook args
    set nbArgs [uplevel 0 _getArgs [list $_nbOptList] $args]
    set pageName [uplevel 0 $notebook insert [list $index] $nbArgs]
    # pick out the tabset args
    set tsArgs [uplevel 0 _getArgs [list $_tsOptList] $args]
    uplevel 0 $tabset insert [list $index] $tsArgs
    return $pageName
    
}

# -------------------------------------------------------------
# METHOD: prev
#
# Selects the previous page. Wraps at first back to last page.
# -------------------------------------------------------------
::itcl::body Tabnotebook::prev {} {
    uplevel 0 $notebook prev
    uplevel 0 $tabset prev
}

# -------------------------------------------------------------
# METHOD: next
#
# Selects the next page. Wraps at last back to first page.
# -------------------------------------------------------------
::itcl::body Tabnotebook::next {} {
    uplevel 0 $notebook next
    uplevel 0 $tabset next
}

# -------------------------------------------------------------
# METHOD: pageconfigure <index> ?<option> <value>...?
#
# Performs configure on a given page denoted by index.
# Index may be a page number or a pattern matching the label
# associated with a page.
# -------------------------------------------------------------
::itcl::body Tabnotebook::pageconfigure {index args} {
    set nbArgs [uplevel 0 _getArgs [list $_nbOptList] $args]
    set tsArgs [uplevel 0 _getArgs [list $_tsOptList] $args]
    set len [llength $args]
    switch $len {
    0 {
        # Here is the case where they just want to query options
        set nbConfig \
	    [eval $notebook pageconfigure $index $nbArgs]
        set tsConfig \
	    [eval $tabset tabconfigure $index $tsArgs]
        #
        # BUG: this currently just concatenates a page and a tab's
        # config lists together... We should bias to the Page
        # since this is what we are using as primary when both??
        #
        # a pageconfigure index -background will return something like:
        # -background background Background #9D008FF583C1 gray70 \
        # -background background background white gray 70
        #
        return [concat $nbConfig $tsConfig]
      }
    1 {
        # Here is the case where they are asking for only one
        # one options value... need to figure out which one
        # (page or tab) can service this. Then only return
        # that one's result.
    
        if { [llength $nbArgs] != 0 } {
	    return [uplevel 0 $notebook pageconfigure $index $nbArgs]
        } elseif { [llength $tsArgs] != 0 } {
	    return [uplevel 0 $tabset tabconfigure $index $tsArgs]
        } else {
	    error "unknown option \"$args\""
        }
      }
    default {
        # pick out the notebook args
        set nbConfig \
	    [uplevel 0 $notebook pageconfigure [list $index] $nbArgs]
        # pick out the tabset args
        set tsConfig \
	    [uplevel 0 $tabset tabconfigure [list $index] $tsArgs]
        return ""
        #return [concat $nbConfig $tsConfig]
      }
    }
}

# -------------------------------------------------------------
# METHOD: select index
#
# Select a page by index
# -------------------------------------------------------------
::itcl::body Tabnotebook::select {index} {
    $notebook select $index
    $tabset select $index
}

# -------------------------------------------------------------
# METHOD: view
#
# Return the current page
#
#         view index
#
# Selects the page denoted by index to be current page
#
#	  view 'moveto' fraction
#
# Selects the page by using fraction amount
#
#	  view 'scroll' num what
#
# Selects the page by using num as indicator of next or
# previous
#
# -------------------------------------------------------------
::itcl::body Tabnotebook::view {args} {
    uplevel 0 $notebook view $args
    $tabset select [index select]
}

# -------------------------------------------------------------
# PRIVATE METHOD: _getArgs
#
# Given an optList returned from a configure on an object and
# given a candidate argument list, peruse throught the optList
# and build a new argument list with only those options found
# in optList.
#
# This is used by the add, insert, and pageconfigure methods.
# It is useful for a container kind of class like Tabnotebook
# to be smart about args it gets for its concept of a "page"
# which is actually a Notebook Page and a Tabset Tab.
#
# -------------------------------------------------------------
::itcl::body Tabnotebook::_getArgs {optList args} {
    set len [llength $args]
    set retArgs {}
    for {set i 0} {$i < $len} {incr i} {
	# get the option for this pair
	set opt [lindex $args $i]
	# move ahead to the value
	incr i
	# option exists!
	if { [lsearch -exact $optList $opt] != -1} {
	    lappend retArgs $opt
	    if {$i < [llength $args]} {
		lappend retArgs [lindex $args $i]
	    }
	    # option does not exist
	}
    }
    return $retArgs
}

# -------------------------------------------------------------
# PROTECTED METHOD: _reconfigureTabset
#
# bound to the tabset reconfigure... We call our canvas 
# reconfigure as if the canvas resized, it then configures
# the tabset correctly.
# -------------------------------------------------------------
::itcl::body Tabnotebook::_reconfigureTabset {} {
    _canvasReconfigure $_canvasWidth $_canvasHeight
}

# -------------------------------------------------------------
# PROTECTED METHOD: _canvasReconfigure
#
# bound to window Reconfigure event of the canvas
# keeps the tabset area stretched in its major dimension.
# -------------------------------------------------------------
::itcl::body Tabnotebook::_canvasReconfigure {wid hgt} {
    if { $_tabPos == "n" || $_tabPos == "s" } {
	$tabset configure -width $wid
    } else {
	$tabset configure -height $hgt
    }
    set _canvasWidth $wid
    set _canvasHeight $hgt
    _redrawBorder $wid $hgt 
}

# -------------------------------------------------------------
# PRIVATE METHOD: _redrawBorder
#
# called by methods when the packing changes, borderwidths, etc.
# and height
# -------------------------------------------------------------
::itcl::body Tabnotebook::_redrawBorder {wid hgt} {
    # Get the top of the Notebook area...
    set nbTop [winfo y $notebook]
    set canTop [expr {$nbTop - $itcl_options(-borderwidth)}]
    $canvas delete BORDER
    if {$itcl_options(-borderwidth) > 0} {
	# For south, east, and west -- draw the top/north edge
	if {$_tabPos ne "n"} {
	    $canvas create line \
		    [expr {floor(0 + ($itcl_options(-borderwidth)/2.0))}] \
		    [expr {floor(0 + ($itcl_options(-borderwidth)/2.0))}] \
		    $wid \
		    [expr {floor(0 + ($itcl_options(-borderwidth)/2.0))}] \
		    -width $itcl_options(-borderwidth) \
		    -fill [::itcl::widgets::colors::topShadow $itcl_options(-background)] \
		    -tags BORDER
	}
	# For north, east, and west -- draw the bottom/south edge
	if {$_tabPos ne "s"} {
	    $canvas create line \
		    [expr {floor(0 + ($itcl_options(-borderwidth)/2.0))}] \
		    [expr {floor($hgt - ($itcl_options(-borderwidth)/2.0))}] \
		    [expr {floor($wid - ($itcl_options(-borderwidth)/2.0))}] \
		    [expr {floor($hgt - ($itcl_options(-borderwidth)/2.0))}] \
		    -width $itcl_options(-borderwidth) \
		    -fill [::itcl::widgets::colors::bottomShadow $itcl_options(-background)] \
		    -tags BORDER
	} 
	# For north, south, and east -- draw the left/west edge
	if {$_tabPos ne "w"} {
	    $canvas create line \
		    [expr {floor(0 + ($itcl_options(-borderwidth)/2.0))}] \
		    0 \
		    [expr {floor(0 + ($itcl_options(-borderwidth)/2.0))}] \
		    $hgt \
		    -width $itcl_options(-borderwidth) \
		    -fill [::itcl::widgets::colors::topShadow $itcl_options(-background)] \
		    -tags BORDER
	}
	# For north, south, and west -- draw the right/east edge
	if {$_tabPos ne "e"} {
	    $canvas create line \
		    [expr {floor($wid - ($itcl_options(-borderwidth)/2.0))}] \
		    [expr {floor(0 + ($itcl_options(-borderwidth)/2.0))}] \
		    [expr {floor($wid - ($itcl_options(-borderwidth)/2.0))}] \
		    $hgt \
		    -width $itcl_options(-borderwidth) \
		    -fill [::itcl::widgets::colors::bottomShadow $itcl_options(-background)] \
		    -tags BORDER
	}
    }
    
}

# -------------------------------------------------------------
# PRIVATE METHOD: _recomputeBorder
#
# Based on current width and height of our canvas, repacks
# the notebook with padding for borderwidth, and calls
# redraw border method 
# -------------------------------------------------------------
::itcl::body Tabnotebook::_recomputeBorder {} {
    set wid [winfo width $canvas]
    set hgt [winfo height $canvas]
    _pack $_tabPos
    _redrawBorder $wid $hgt
}

# -------------------------------------------------------------
# PROTECTED METHOD: _pageReconfigure
#
# This method will eventually reconfigure the tab notebook's 
# notebook area to contain the resized child site
# -------------------------------------------------------------
::itcl::body Tabnotebook::_pageReconfigure {pageName page wid hgt} {
}

# -------------------------------------------------------------
# PRIVATE METHOD: _pack
#
# This method packs the notebook and tabset correctly according
# to the current $tabPos
# -------------------------------------------------------------
::itcl::body Tabnotebook::_pack {tabPos} {
    pack $canvas -fill both -expand yes 
    pack propagate $canvas no
    switch $tabPos {
    n {
        # north
        pack $tabset \
	    -anchor nw \
	    -fill x \
	    -expand no
        pack $notebook \
	    -fill both \
	    -expand yes \
	    -padx $itcl_options(-borderwidth) \
	    -pady $itcl_options(-borderwidth) \
	    -side bottom 
      }
    s {
        # south
        pack $notebook \
	    -anchor nw \
	    -fill both \
	    -expand yes \
	    -padx $itcl_options(-borderwidth) \
	    -pady $itcl_options(-borderwidth)
    
        pack $tabset \
	    -side left \
	    -fill x \
	    -expand yes
      }
    w {
        # west
        pack $tabset \
	    -anchor nw \
	    -side left \
	    -fill y \
	    -expand no
        pack $notebook \
	    -anchor nw \
	    -side left \
	    -fill both \
	    -expand yes \
	    -padx $itcl_options(-borderwidth) \
	    -pady $itcl_options(-borderwidth)
    
      }
    e {
        # east
        pack $notebook \
	    -side left \
	    -anchor nw \
	    -fill both \
	    -expand yes \
	    -padx $itcl_options(-borderwidth) \
	    -pady $itcl_options(-borderwidth)
    
        pack $tabset \
	    -fill y \
	    -expand yes
      }
    }
    
    set wid [winfo width $canvas]
    set hgt [winfo height $canvas]
    _redrawBorder $wid $hgt
}

# -------------------------------------------------------------
# PRIVATE METHOD: _resize
#
# This method added by csmith, 5/1/01, to fix a bug with the
# geometry of the tabnotebook.  The hull component's geometry
# was not being updated properly on <Configure> events.
# -------------------------------------------------------------
::itcl::body Tabnotebook::_resize {newWidth_ newHeight_} {
  _canvasReconfigure $newWidth_ $newHeight_
  # csmith: 9/14/01 - Commenting out the following code due to
  # SF ticket 461471, which is a dup of the original 452803. Since I
  # can't remember the exact problem surrounding the need to add
  # the _resize method, I'm going to do an undo here, leaving the
  # code for future reference if needed.  Should the original problem
  # arise again I will reinvestigate the need for _resize.
  #
  #  after idle \
  #    "$win configure -width $newWidth_ -height $newHeight_"
}

} ; # end ::itcl::widgets
