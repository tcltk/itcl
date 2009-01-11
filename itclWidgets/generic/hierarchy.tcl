#
# Hierarchy
# ----------------------------------------------------------------------
# Hierarchical data viewer.  Manages a list of nodes that can be
# expanded or collapsed.  Individual nodes can be highlighted.
# Clicking with the right mouse button on any item brings up a
# special item menu.  Clicking on the background area brings up
# a different popup menu.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Hierarchy
# written by:
#   AUTHOR:  Michael J. McLennan
#            Bell Labs Innovations for Lucent Technologies
#            mmclennan@lucent.com
#    Copyright (c) 1995 DSC Technologies Corporation
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1996  Lucent Technologies
# ----------------------------------------------------------------------
#
#   @(#) $Id: hierarchy.tcl,v 1.1.2.3 2009/01/11 11:40:11 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Hierarchy.menuCursor arrow widgetDefault
option add *Hierarchy.labelPos n widgetDefault
option add *Hierarchy.tabs 30 widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Hierarchy class.
# 
proc ::itcl::widgets::hierarchy {pathName args} {
    uplevel ::itcl::widgets::Hierarchy $pathName $args
}

# ------------------------------------------------------------------
#                            HIERARCHY
# ------------------------------------------------------------------
::itcl::extendedclass Hierarchy {
    inherit ::itcl::widgets::Scrolledwidget

    component clipper
    component list
    component itemMenu
    component bgMenu

    option [list -alwaysquery alwaysQuery AlwaysQuery] -default 0 -configuremethod configAlwaysquery
    option [list -closedicon closedIcon Icon] -default {} -configuremethod configClosedicon
    option [list -dblclickcommand dblClickCommand Command] -default {} -configuremethod configDblclickcommand
    option [list -expanded expanded Expanded] -default 0  -configuremethod configExpanded
    option [list -filter filter Filter] -default 0  -configuremethod configFilter
    option [list -font font Font] -default \
	-*-Courier-Medium-R-Normal--*-120-*-*-*-*-*-* -configuremethod configFont
    option [list -height height Height] -default 0 -configuremethod configHeight
    option [list -iconcommand iconCommand Command] -default {} -configuremethod configIconcommand

    option [list -icondblcommand iconDblCommand Command] -default {} -configuremethod configIcondblcommand

    option [list -imagecommand imageCommand Command] -default {} -configuremethod configImagecommand

    option [list -imagedblcommand imageDblCommand Command] -default {} -configuremethod configImagedblcommand

    option [list -imagemenuloadcommand imageMenuLoadCommand Command] -default {} -configuremethod configImageloadcommand

    option [list -markbackground markBackground Foreground] -default #a0a0a0  -configuremethod configMarkbackground

    option [list -markforeground markForeground Background] -default Black  -configuremethod configMarkforeground

    option [list -nodeicon nodeIcon Icon] -default {} -configuremethod configNodeicon

    option [list -openicon openIcon Icon] -default {} -configuremethod configOpenicon

    option [list -querycommand queryCommand Command] -default {} -configuremethod configQuerycommand

    option [list -selectcommand selectCommand Command] -default {} -configuremethod configSelectcommand

    option [list -selectbackground selectBackground Foreground] -default #c3c3c3 -configuremethod configSelectbackground

    option [list -selectforeground selectForeground Background] -default Black  -configuremethod configSelectforeground

    option [list -textmenuloadcommand textMenuLoadCommand Command] -default {} -configuremethod configTextmenuloadcommand

    option [list -visibleitems visibleItems VisibleItems] -default 80x24 -configuremethod configVisibleitems

    option [list -width width Width] -default 0 -configuremethod configWidth


    delegate option [list -background background Background] to clipper as -highlightbackground
    delegate option [list -textfont textFont Font] to list as -font
    delegate option [list -textbackground textBackground Background] to list as -background
    delegate option [list -menucursor menuCursor Cursor] to itemMenu as -cursor
    delegate option [list -menucursor menuCursor Cursor] to bgMenu as -cursor

    private variable _filterCode ""  ;# Compact view flag.
    private variable _hcounter 0     ;# Counter for hierarchy icons
    private variable _icons          ;# Array of user icons by uid
    private variable _images         ;# Array of our icons by uid
    private variable _indents        ;# Array of indentation by uid
    private variable _marked         ;# Array of marked nodes by uid
    private variable _markers ""     ;# List of markers for level being drawn
    private variable _nodes          ;# Array of subnodes by uid
    private variable _pending ""     ;# Pending draw flag
    private variable _posted ""      ;# List of tags at posted menu position
    private variable _selected       ;# Array of selected nodes by uid
    private variable _tags           ;# Array of user tags by uid
    private variable _text           ;# Array of displayed text by uid
    private variable _states         ;# Array of selection state by uid
    private variable _ucounter 0     ;# Counter for user icons

    constructor {args} {}
    destructor {}

    private method _configureTags {}

    protected method _contents {uid}
    protected method _post {x y}
    protected method _drawLevel {node indent}
    protected method _select {x y}
    protected method _deselectSubNodes {uid}
    protected method _deleteNodeInfo {uid}
    protected method _getParent {uid}
    protected method _getHeritage {uid}
    protected method _isInternalTag {tag}
    protected method _iconSelect {node icon}
    protected method _iconDblSelect {node icon}
    protected method _imageSelect {node}
    protected method _imageDblClick {node}
    protected method _imagePost {node image type x y}
    protected method _double {x y}
    protected method configAlwaysquery {option value}
    protected method configClosedicon {option value}
    protected method configDblclickcommand {option value}
    protected method configExpanded {option value}
    protected method configFilter {option value}
    protected method configFont {option value}
    protected method configHeight {option value}
    protected method configIconcommand {option value}
    protected method configIcondblcommand {option value}
    protected method configImagecommand {option value}
    protected method configImagedblcommand {option value}
    protected method configImagemenuloadcommand {option value}
    protected method configMarkbackground {option value}
    protected method configMarkforeground {option value}
    protected method configNodeicon {option value}
    protected method configOpenicon {option value}
    protected method configQuerycommand {option value}
    protected method configSelectcommand {option value}
    protected method configSelectbackground {option value}
    protected method configSelectforeground {option value}
    protected method configTextmenuloadcommand {option value}
    protected method configVisibleitems {option value}
    protected method configWidth {option value}
    
    public method clear {}
    public method collapse {node}
    public method current {}
    public method draw {{when -now}}
    public method expand {node}
    public method expanded {node}
    public method expState { }
    public method mark {op args}
    public method prune {node}
    public method refresh {node}
    public method selection {op args}
    public method toggle {node}
	
    public method bbox {index} 
    public method compare {index1 op index2} 
    public method debug {args} {eval $args}
    public method delete {first {last {}}} 
    public method dlineinfo {index} 
    public method dump {args}
    public method get {index1 {index2 {}}} 
    public method index {index} 
    public method insert {args} 
    public method scan {option args} 
    public method search {args} 
    public method see {index} 
    public method tag {op args} 
    public method window {option args} 
    public method xview {args}
    public method yview {args}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Hierarchy::constructor {args} {
# FIXME    itcl_options remove ::itcl::widgets::Labeledwidget::state
    #
    # Our -width and -height options are slightly different than
    # those implemented by our base class, so we're going to
    # remove them and redefine our own.
    #
# FIXME    itcl_options remove ::itcl::widgets::Scrolledwidget::width
# FIXME    itcl_options remove ::itcl::widgets::Scrolledwidget::height
    #
    # Create a clipping frame which will provide the border for
    # relief display.
    #
    setupcomponent clipper using frame $itcl_interior.clipper
    keepcomponentoption clipper -cursor -textfont  \
        -background -foreground -textbackground
    keepcomponentoption clipper -borderwidth -relief -highlightthickness -highlightcolor
    grid $clipper -row 0 -column 0 -sticky nsew
    grid rowconfigure $_interior 0 -weight 1
    grid columnconfigure $_interior 0 -weight 1
    #
    # Create a text widget for displaying our hierarchy.
    #
    setupcomponent list using text $clipper.list -wrap none -cursor center_ptr \
                -state disabled -width 1 -height 1 \
	        -xscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.horizsb] \
		-yscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.vertsb] \
	        -borderwidth 0 -highlightthickness 0
    keepcomponentoption list -cursor -textfont -font \
        -background -foreground -textbackground  \
        -selectbackground -selectforeground -activebackground
    keepcomponentoption list -spacing1 -spacing2 -spacing3 -tabs
# FIXME	ignore -highlightthickness -highlightcolor
# FIXME	ignore -insertbackground -insertborderwidth
# FIXME	ignore -insertontime -insertofftime -insertwidth
# FIXME	ignore -selectborderwidth
# FIXME	ignore -borderwidth
    grid $list -row 0 -column 0 -sticky nsew
    grid rowconfigure $clipper 0 -weight 1
    grid columnconfigure $clipper 0 -weight 1
    # 
    # Configure the command on the vertical scroll bar in the base class.
    #
    $vertsb configure -command [itcl::code $list yview]
    #
    # Configure the command on the horizontal scroll bar in the base class.
    #
    $horizsb configure -command [itcl::code $list xview]
    #
    # Configure our text component's tab settings for twenty levels.
    #
    set tabs ""
    for {set i 1} {$i < 20} {incr i} {
	lappend tabs [expr {$i*12+4}]
    }
    $list configure -tabs $tabs
    #
    # Add popup menus that can be configured by the user to add
    # new functionality.
    #
    setupcomponent itemMenu using menu $list.itemmenu -tearoff 0
    keepcomponentoption itemMenu -cursor -textfont -font \
        -background -foreground -textbackground  -activeforeground \
        -selectbackground -selectforeground -selectcolor -menucursor
# FIXME ignore -tearoff
    setupcomponent bgMenu using menu $list.bgmenu -tearoff 0
# FIXME	ignore -tearoff
    keepcomponentoption bgMenu -cursor -textfont -font \
        -background -foreground -textbackground  \
        -selectbackground -selectforeground -menucursor
    #
    # Adjust the bind tags to remove the class bindings.  Also, add
    # bindings for mouse button 1 to do selection and button 3 to 
    # display a popup.
    #
    bindtags $list [list $list . all]
    bind $list <ButtonPress-1> [itcl::code $this _select %x %y]
    bind $list <Double-1> \ [itcl::code $this _double %x %y]
    bind $list <ButtonPress-3> \ [itcl::code $this _post %x %y]
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
}

# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Hierarchy::destructor {} {
    if {$_pending ne ""} {
	after cancel $_pending
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -font
#
# Font used for text in the list.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configFont {option value} {
    $list tag configure info -font $value -spacing1 6
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -selectbackground
#
# Background color scheme for selected nodes.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configSelectbackground {option value} {
    $list tag configure hilite -background $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -selectforeground
#
# Foreground color scheme for selected nodes.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configSelectforeground {option value} {
    $list tag configure hilite -foreground $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -markbackground
#
# Background color scheme for marked nodes.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configMarkbackground {option value} {
    $list tag configure lowlite -background $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -markforeground
#
# Foreground color scheme for marked nodes.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configMarkforeground {option value} {
    $list tag configure lowlite -foreground $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -querycommand
#
# Command executed to query the contents of each node.  If this 
# command contains "%n", it is replaced with the name of the desired 
# node.  In its simpilest form it should return the children of the 
# given node as a list which will be depicted in the display.
#
# Since the names of the children are used as tags in the underlying 
# text widget, each child must be unique in the hierarchy.  Due to
# the unique requirement, the nodes shall be reffered to as uids 
# or uid in the singular sense.
# 
#   {uid [uid ...]}
#
#   where uid is a unique id and primary key for the hierarchy entry
#
# Should the unique requirement pose a problem, the list returned
# can take on another more extended form which enables the 
# association of text to be displayed with the uids.  The uid must
# still be unique, but the text does not have to obey the unique
# rule.  In addition, the format also allows the specification of
# additional tags to be used on the same entry in the hierarchy
# as the uid and additional icons to be displayed just before
# the node.  The tags and icons are considered to be the property of
# the user in that the hierarchy widget will not depend on any of 
# their values.
#
#   {{uid [text [tags [icons]]]} {uid [text [tags [icons]]]} ...}
#
#   where uid is a unique id and primary key for the hierarchy entry
#         text is the text to be displayed for this uid
#         tags is a list of user tags to be applied to the entry
#         icons is a list of icons to be displayed in front of the text
#
# The hierarchy widget does a look ahead from each node to determine
# if the node has a children.  This can be cost some performace with
# large hierarchies.  User's can avoid this by providing a hint in
# the user tags.  A tag of "leaf" or "branch" tells the hierarchy
# widget the information it needs to know thereby avoiding the look
# ahead operation.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configQuerycommand {option value} {
    set itcl_options($option) $value
    clear
    draw -eventually
    # Added for SF ticket #596111
    _configureTags
}

# ------------------------------------------------------------------
# OPTION: -selectcommand
#
# Command executed to select an item in the list.  If this command
# contains "%n", it is replaced with the name of the selected node.  
# If it contains a "%s", it is replaced with a boolean indicator of 
# the node's current selection status, where a value of 1 denotes
# that the node is currently selected and 0 that it is not.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configSelectcommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -dblclickcommand
#
# Command executed to double click an item in the list.  If this command
# contains "%n", it is replaced with the name of the selected node.  
# If it contains a "%s", it is replaced with a boolean indicator of 
# the node's current selection status, where a value of 1 denotes
# that the node is currently selected and 0 that it is not.
#
# Douglas R. Howard, Jr.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configDblclickcommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -iconcommand
#
# Command executed upon selection of user icons.  If this command 
# contains "%n", it is replaced with the name of the node the icon
# belongs to.  Should it contain "%i" then the icon name is 
# substituted.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configIconcommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -icondblcommand
#
# Command executed upon double selection of user icons.  If this command 
# contains "%n", it is replaced with the name of the node the icon
# belongs to.  Should it contain "%i" then the icon name is 
# substituted.
#
# Douglas R. Howard, Jr.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configIcondblcommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -imagecommand
#
# Command executed upon selection of image icons.  If this command 
# contains "%n", it is replaced with the name of the node the icon
# belongs to.  Should it contain "%i" then the icon name is 
# substituted.
#
# Douglas R. Howard, Jr.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configImagecommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -imagedblcommand
#
# Command executed upon double selection of user icons.  If this command 
# contains "%n", it is replaced with the name of the node the icon
# belongs to.
#
# Douglas R. Howard, Jr.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configImagedblcommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -alwaysquery
#
# Boolean flag which tells the hierarchy widget weather or not
# each refresh of the display should be via a new query using
# the -querycommand option or use the values previous found the
# last time the query was made.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configAlwaysquery {option value} {
    switch -- $value {
    1 -
    true -
    yes -
    on {
        ;# okay
      }
    0 -
    false -
    no -
    off {
        ;# okay
      }
    default {
        error "bad alwaysquery option \"$value\":\
               should be boolean"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -filter
#
# When true only the branch nodes and selected items are displayed.
# This gives a compact view of important items.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configFilter {option value} {
    switch -- $value {
    1 -
    true -
    yes -
    on {
        set newCode {set display [info exists _selected($child)]}
      }
    0 -
    false -
    no -
    off {
        set newCode {set display 1}
      }
    default {
        error "bad filter option \"$value\":\
                  should be boolean"
    }
    }
    set itcl_options($option) $value
    if {$newCode != $_filterCode} {
        set _filterCode $newCode
        draw -eventually
    }
}

# ------------------------------------------------------------------
# OPTION: -expanded
#
# When true, the hierarchy will be completely expanded when it
# is first displayed.  A fresh display can be triggered by
# resetting the -querycommand option.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configExpanded {option value} {
    switch -- $value {
    1 -
    true -
    yes -
    on {
        ;# okay
      }
    0 -
    false -
    no -
    off {
        ;# okay
      }
    default {
        error "bad expanded option \"$value\":\
                  should be boolean"
      }
    }
    set itcl_options($option) $value
}
    
# ------------------------------------------------------------------
# OPTION: -openicon
#
# Specifies the open icon image to be used in the hierarchy.  Should
# one not be provided, then one will be generated, pixmap if 
# possible, bitmap otherwise.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configOpenicon {option value} {
    if {$itcl_options($option) eq {}} {
	if {[lsearch [image names] openFolder] == -1} {
	    if {[lsearch [image types] pixmap] != -1} {
		image create pixmap openFolder -data {
		    /* XPM */
		    static char * dir_opened [] = {
			"16 16 4 1",
			/* colors */
			". c grey85 m white g4 grey90",
			"b c black  m black g4 black",
			"y c yellow m white g4 grey80",
			"g c grey70 m white g4 grey70",
			/* pixels */
			"................",
			"................",
			"................",
			"..bbbb..........",
			".bggggb.........",
			"bggggggbbbbbbb..",
			"bggggggggggggb..",
			"bgbbbbbbbbbbbbbb",
			"bgbyyyyyyyyyyybb",
			"bbyyyyyyyyyyyyb.",
			"bbyyyyyyyyyyybb.",
			"byyyyyyyyyyyyb..",
			"bbbbbbbbbbbbbb..",
			"................",
			"................",
			"................"};
		}
	    } else {
		image create bitmap openFolder -data {
		    #define open_width 16
		    #define open_height 16
		    static char open_bits[] = {
			0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x42, 0x00, 
			0x81, 0x3f, 0x01, 0x20, 0xf9, 0xff, 0x0d, 0xc0, 
			0x07, 0x40, 0x03, 0x60, 0x01, 0x20, 0x01, 0x30,
			0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		}
	    }
	}
	set itcl_options(-openicon) openFolder
        set itcl_options($option) $value
    } else {
	if {[lsearch [image names] $value] == -1} {
	    error "bad openicon option \"$value\":\
                   should be an existing image"
	}
        set itcl_options($option) $value
    }
}

# ------------------------------------------------------------------
# OPTION: -closedicon
#
# Specifies the closed icon image to be used in the hierarchy.  
# Should one not be provided, then one will be generated, pixmap if 
# possible, bitmap otherwise.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configClosedicon {option value} {
    if {$itcl_options($option) eq {}} {
	if {[lsearch [image names] closedFolder] == -1} {
	    if {[lsearch [image types] pixmap] != -1} {
		image create pixmap closedFolder -data {
		    /* XPM */
		    static char *dir_closed[] = {
			"16 16 3 1",
			". c grey85 m white g4 grey90",
			"b c black  m black g4 black",
			"y c yellow m white g4 grey80",
			"................",
			"................",
			"................",
			"..bbbb..........",
			".byyyyb.........",
			"bbbbbbbbbbbbbb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"bbbbbbbbbbbbbb..",
			"................",
			"................",
			"................"};	
		}
	    } else {
		image create bitmap closedFolder -data {
		    #define closed_width 16
		    #define closed_height 16
		    static char closed_bits[] = {
			0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x84, 0x00, 
			0xfe, 0x7f, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 
			0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40,
			0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		}
	    }
	}
	set itcl_options(-closedicon) closedFolder
        set itcl_options($option) $value
    } else {
	if {[lsearch [image names] $value] == -1} {
	    error "bad closedicon option \"$value\":\
                   should be an existing image"
	}
        set itcl_options($option) $value
    }
}

# ------------------------------------------------------------------
# OPTION: -nodeicon
#
# Specifies the node icon image to be used in the hierarchy.  Should 
# one not be provided, then one will be generated, pixmap if 
# possible, bitmap otherwise.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configNodeicon {option value} {
    if {$itcl_options($option) eq {}} {
	if {[lsearch [image names] nodeFolder] == -1} {
	    if {[lsearch [image types] pixmap] != -1} {
		image create pixmap nodeFolder -data {
		    /* XPM */
		    static char *dir_node[] = {
			"16 16 3 1",
			". c grey85 m white g4 grey90",
			"b c black  m black g4 black",
			"y c yellow m white g4 grey80",
			"................",
			"................",
			"................",
			"...bbbbbbbbbbb..",
			"..bybyyyyyyyyb..",
			".byybyyyyyyyyb..",
			"byyybyyyyyyyyb..",
			"bbbbbyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"byyyyyyyyyyyyb..",
			"bbbbbbbbbbbbbb..",
			"................",
			"................",
			"................"};	
		}
	    } else {
		image create bitmap nodeFolder -data {
		    #define node_width 16
		    #define node_height 16
		    static char node_bits[] = {
			0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x50, 0x40, 
			0x48, 0x40, 0x44, 0x40, 0x42, 0x40, 0x7e, 0x40, 
			0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40,
			0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		}
	    }
	}
	set itcl_options(-nodeicon) nodeFolder
        set itcl_options($option) $value
    } else {
	if {[lsearch [image names] $value] == -1} {
	    error "bad nodeicon option \"$value\":\
                   should be an existing image"
	}
        set itcl_options($option) $value
    }
}

# ------------------------------------------------------------------
# OPTION: -width
#
# Specifies the width of the hierarchy widget as an entire unit.
# The value may be specified in any of the forms acceptable to 
# Tk_GetPixels.  Any additional space needed to display the other
# components such as labels, margins, and scrollbars force the text
# to be compressed.  A value of zero along with the same value for 
# the height causes the value given for the visibleitems option 
# to be applied which administers geometry constraints in a different
# manner.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configWidth {option value} {
    if {$value ne 0} {
	set shell [lindex [grid info $clipper] 1]
	#
	# Due to a bug in the tk4.2 grid, we have to check the 
	# propagation before setting it.  Setting it to the same
	# value it already is will cause it to toggle.
	#
	if {[grid propagate $shell]} {
	    grid propagate $shell no
	}
	$list configure -width 1
	$shell configure \
		-width [winfo pixels $shell $value] 
    } else {
	configure -visibleitems $itcl_options(-visibleitems)
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -height
#
# Specifies the height of the hierarchy widget as an entire unit.
# The value may be specified in any of the forms acceptable to 
# Tk_GetPixels.  Any additional space needed to display the other
# components such as labels, margins, and scrollbars force the text
# to be compressed.  A value of zero along with the same value for 
# the width causes the value given for the visibleitems option 
# to be applied which administers geometry constraints in a different
# manner.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configHeight {option value} {
    if {$value ne 0} {
	set shell [lindex [grid info $clipper] 1]
	#
	# Due to a bug in the tk4.2 grid, we have to check the 
	# propagation before setting it.  Setting it to the same
	# value it already is will cause it to toggle.
	#
	if {[grid propagate $shell]} {
	    grid propagate $shell no
	}
	$list configure -height 1
	$shell configure \
		-height [winfo pixels $shell $value] 
    } else {
	configure -visibleitems $itcl_options(-visibleitems)
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -visibleitems
#
# Specified the widthxheight in characters and lines for the text.
# This option is only administered if the width and height options
# are both set to zero, otherwise they take precedence.  With the
# visibleitems option engaged, geometry constraints are maintained
# only on the text.  The size of the other components such as 
# labels, margins, and scroll bars, are additive and independent, 
# effecting the overall size of the scrolled text.  In contrast,
# should the width and height options have non zero values, they
# are applied to the scrolled text as a whole.  The text is 
# compressed or expanded to maintain the geometry constraints.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configVisibleitems {option value} {
    if {[regexp {^[0-9]+x[0-9]+$} $value]} {
	if {($itcl_options(-width) == 0) && \
		($itcl_options(-height) == 0)} {
	    set chars [lindex [split $value x] 0]
	    set lines [lindex [split $value) x] 1]
	    set shell [lindex [grid info $clipper] 1]
	    #
	    # Due to a bug in the tk4.2 grid, we have to check the 
	    # propagation before setting it.  Setting it to the same
	    # value it already is will cause it to toggle.
	    #
	    if {! [grid propagate $shell]} {
		grid propagate $shell yes
	    }
	    $list configure -width $chars -height $lines
	}
    } else {
	error "bad visibleitems option\
		\"$value\": should be\
		widthxheight"
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -textmenuloadcommand
#
# Dynamically loads the popup menu based on what was selected.
#
# Douglas R. Howard, Jr.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configTextmenuloadcommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -imagemenuloadcommand
#
# Dynamically loads the popup menu based on what was selected.
#
# Douglas R. Howard, Jr.
# ------------------------------------------------------------------
::itcl::body Hierarchy::configImagemenuloadcommand {option value} {
    set itcl_options($option) $value
}


# ------------------------------------------------------------------
#                         PUBLIC METHODS
# ------------------------------------------------------------------

# ----------------------------------------------------------------------
# PUBLIC METHOD: clear
#
# Removes all items from the display including all tags and icons.  
# The display will remain empty until the -filter or -querycommand 
# options are set.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::clear {} {
    $list configure -state normal -cursor watch
    $list delete 1.0 end
    $list configure -state disabled -cursor $itcl_options(-cursor)
    # Clear the tags
    uplevel 0 $list tag delete [$list tag names]
    catch {unset _nodes}
    catch {unset _text}
    catch {unset _tags}
    catch {unset _icons}
    catch {unset _states}
    catch {unset _images}
    catch {unset _indents}
    catch {unset _marked}
    catch {unset _selected}
    set _markers  ""
    set _posted   ""
    set _ucounter 0
    set _hcounter 0 
    foreach mark [$list mark names] {
        $list mark unset $mark
    }
    return
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: selection option ?uid uid...?
#
# Handles all operations controlling selections in the hierarchy.
# Selections may be cleared, added, removed, or queried.  The add and
# remove options accept a series of unique ids.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::selection {op args} {
    switch -- $op {
    clear {
        $list tag remove hilite 1.0 end
        catch {unset _selected}
        return
      }
    add {
        foreach node $args {
            set _selected($node) 1
            catch {
                $list tag add hilite "$node.first" "$node.last"
	    }
        }
      }
    remove {
        foreach node $args {
            catch {
                unset _selected($node)
                $list tag remove hilite "$node.first" "$node.last"
            }
        }
      }
    get {
        return [array names _selected]
      }
    default {
        error "bad selection operation \"$op\":\
               should be add, remove, clear or get"
      }
    }
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: mark option ?arg arg...?
#
# Handles all operations controlling marks in the hierarchy.  Marks may 
# be cleared, added, removed, or queried.  The add and remove options 
# accept a series of unique ids.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::mark {op args} {
    switch -- $op {
    clear {
        $list tag remove lowlite 1.0 end
        catch {unset _marked}
        return
      }
    add {
        foreach node $args {
            set _marked($node) 1
            catch {
                $list tag add lowlite "$node.first" "$node.last"
            }
        }
      }
    remove {
        foreach node $args {
            catch {
                unset _marked($node)
                $list tag remove lowlite "$node.first" "$node.last"
            }
        }
      }
    get {
        return [array names _marked]
      }
    default {
        error "bad mark operation \"$op\":\
               should be add, remove, clear or get"
      }
    }
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: current
#
# Returns the node that was most recently selected by the right mouse
# button when the item menu was posted.  Usually used by the code
# in the item menu to figure out what item is being manipulated.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::current {} {
    return $_posted
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: expand node
#
# Expands the hierarchy beneath the specified node.  Since this can take
# a moment for large hierarchies, the cursor will be changed to a watch
# during the expansion.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::expand {node} {
    if {! [info exists _states($node)]} {
	error "bad expand node argument: \"$node\", the node doesn't exist"
    }
    if {!$_states($node) && \
	    (([lsearch $_tags($node) branch] != -1) || \
	     ([llength [_contents $node]] > 0))} {
        $list configure -state normal -cursor watch
        update
	#
	# Get the indentation level for the node.
	#
        set indent $_indents($node)
        set _markers ""
        $list mark set insert "$node:start"
        _drawLevel $node $indent

	#
	# Following the draw, all our markers need adjusting.
	#
        foreach {name index} $_markers {
            $list mark set $name $index
        }
	#
	# Set the image to be the open icon, denote the new state,
	# and set the cursor back to normal along with the state.
	#
	$_images($node) configure -image $itcl_options(-openicon)
        set _states($node) 1
        $list configure -state disabled -cursor $itcl_options(-cursor)
    }
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: collapse node
#
# Collapses the hierarchy beneath the specified node.  Since this can 
# take a moment for large hierarchies, the cursor will be changed to a 
# watch during the expansion.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::collapse {node} {
    if {! [info exists _states($node)]} {
	error "bad collapse node argument: \"$node\", the node doesn't exist"
    }
    if {[info exists _states($node)] && $_states($node) && \
	    (([lsearch $_tags($node) branch] != -1) || \
	     ([llength [_contents $node]] > 0))} {
        $list configure -state normal -cursor watch
	update
	_deselectSubNodes $node
        $list delete "$node:start" "$node:end"
	catch {$_images($node) configure -image $itcl_options(-closedicon)}
        set _states($node) 0
        $list configure -state disabled -cursor $itcl_options(-cursor)
    }
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: toggle node
#
# Toggles the hierarchy beneath the specified node.  If the hierarchy
# is currently expanded, then it is collapsed, and vice-versa.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::toggle {node} {
    if {! [info exists _states($node)]} {
	error "bad toggle node argument: \"$node\", the node doesn't exist"
    }
    if {$_states($node)} {
        collapse $node
    } else {
        expand $node
    }
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: prune node
#
# Removes a particular node from the hierarchy.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::prune {node} {
    #
    # While we're working, change the state and cursor so we can
    # edit the text and give a busy visual clue.
    #
    $list configure -state normal -cursor watch
    #
    # Recursively delete all the subnode information from our internal
    # arrays and remove all the tags.  
    #
    _deleteNodeInfo $node
    #
    # If the mark $node:end exists then the node has decendents so
    # so we'll remove from the mark $node:start to $node:end in order 
    # to delete all the subnodes below it in the text.  
    # 
    if {[lsearch [$list mark names] $node:end] != -1} {
	$list delete $node:start $node:end
	$list mark unset $node:end
    } 
    #
    # Next we need to remove the node itself.  Using the ranges for
    # its tag we'll remove it from line start to the end plus one
    # character which takes us to the start of the next node.
    #
    foreach {start end} [$list tag ranges $node] {
	$list delete "$start linestart" "$end + 1 char"
    }
    #
    # Delete the tag for this node.
    #
    $list tag delete $node
    #
    # The node must be removed from the list of subnodes for its parent.
    # We don't really have a clean way to do upwards referencing, so
    # the dirty way will have to do.  We'll cycle through each node
    # and if this node is in its list of subnodes, we'll remove it.
    #
    foreach uid [array names _nodes] {
	if {[set index [lsearch $_nodes($uid) $node]] != -1} {
	    set _nodes($uid) [lreplace $_nodes($uid) $index $index]
	}
    }
    #
    # We're done, so change the state and cursor back to their 
    # original values.
    #
    $list configure -state disabled -cursor $itcl_options(-cursor)
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: draw ?when?
#
# Performs a complete draw of the entire hierarchy.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::draw {{when -now}} {
    if {$when eq "-eventually"} {
        if {$_pending eq ""} {
            set _pending [after idle [itcl::code $this draw -now]]
        }
        return
    } elseif {$when != "-now"} {
        error "bad when option \"$when\": should be -eventually or -now"
    }
    $list configure -state normal -cursor watch
    update
    $list delete 1.0 end
    catch {unset _images}
    set _markers ""
    _drawLevel "" ""
    foreach {name index} $_markers {
        $list mark set $name $index
    }
    $list configure -state disabled -cursor $itcl_options(-cursor)
    set _pending ""
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: refresh node
#
# Performs a redraw of a specific node.  If that node is currently 
# not visible, then no action is taken.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::refresh {node} {
    if {![info exists _nodes($node)]} {
	error "bad refresh node argument: \"$node\", the node doesn't exist"
    }
    if {!$_states($node)} {
        return
    }
    foreach parent [_getHeritage $node] {
	if {!$_states($parent)} {
	    return
	}
    }
    $list configure -state normal -cursor watch
    $list delete $node:start $node:end
    set _markers ""
    $list mark set insert "$node:start"
    set indent $_indents($node)
    _drawLevel $node $indent
    foreach {name index} $_markers {
        $list mark set $name $index
    }
    $list configure -state disabled -cursor $itcl_options(-cursor)
}

# ------------------------------------------------------------------
# THIN WRAPPED TEXT METHODS:
#
# The following methods are thin wraps of standard text methods.
# Consult the Tk text man pages for functionallity and argument
# documentation.
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# PUBLIC METHOD: bbox index
#
# Returns four element list describing the bounding box for the list
# item at index
# ------------------------------------------------------------------
::itcl::body Hierarchy::bbox {index} {
    return [$list bbox $index]
}

# ------------------------------------------------------------------
# PUBLIC METHOD compare index1 op index2
#
# Compare indices according to relational operator.
# ------------------------------------------------------------------
::itcl::body Hierarchy::compare {index1 op index2} {
    return [$list compare $index1 $op $index2]
}

# ------------------------------------------------------------------
# PUBLIC METHOD delete first ?last?
#
# Delete a range of characters from the text.
# ------------------------------------------------------------------
::itcl::body Hierarchy::delete {first {last {}}} {
    $list configure -state normal -cursor watch
    $list delete $first $last
    $list configure -state disabled -cursor $itcl_options(-cursor)
}

# ------------------------------------------------------------------
# PUBLIC METHOD dump ?switches? index1 ?index2?
#
# Returns information about the contents of the text widget from 
# index1 to index2.
# ------------------------------------------------------------------
::itcl::body Hierarchy::dump {args} {
    return [uplevel 0 $list dump $args]
}

# ------------------------------------------------------------------
# PUBLIC METHOD dlineinfo index
#
# Returns a five element list describing the area occupied by the
# display line containing index.
# ------------------------------------------------------------------
::itcl::body Hierarchy::dlineinfo {index} {
    return [$list dlineinfo $index]
}

# ------------------------------------------------------------------
# PUBLIC METHOD get index1 ?index2?
#
# Return text from start index to end index.
# ------------------------------------------------------------------
::itcl::body Hierarchy::get {index1 {index2 {}}} {
    return [$list get $index1 $index2]
}

# ------------------------------------------------------------------
# PUBLIC METHOD index index
#
# Return position corresponding to index.
# ------------------------------------------------------------------
::itcl::body Hierarchy::index {index} {
    return [$list index $index]
}

# ------------------------------------------------------------------
# PUBLIC METHOD insert index chars ?tagList?
#
# Insert text at index.
# ------------------------------------------------------------------
::itcl::body Hierarchy::insert {args} {
    $list configure -state normal -cursor watch
    uplevel 0 $list insert $args
    $list configure -state disabled -cursor $itcl_options(-cursor)
}

# ------------------------------------------------------------------
# PUBLIC METHOD scan option args
#
# Implements scanning on texts.
# ------------------------------------------------------------------
::itcl::body Hierarchy::scan {option args} {
    uplevel 0 $list scan $option $args
}

# ------------------------------------------------------------------
# PUBLIC METHOD search ?switches? pattern index ?varName?
#
# Searches the text for characters matching a pattern.
# ------------------------------------------------------------------
::itcl::body Hierarchy::search {args} {
    return [uplevel 0 $list search $args]
}

# ------------------------------------------------------------------
# PUBLIC METHOD see index
#
# Adjusts the view in the window so the character at index is 
# visible.
# ------------------------------------------------------------------
::itcl::body Hierarchy::see {index} {
    $list see $index
}

# ------------------------------------------------------------------
# PUBLIC METHOD tag option ?arg arg ...?
#
# Manipulate tags dependent on options.
# ------------------------------------------------------------------
::itcl::body Hierarchy::tag {op args} {
    return [uplevel 0 $list tag $op $args]
}

# ------------------------------------------------------------------
# PUBLIC METHOD window option ?arg arg ...?
#
# Manipulate embedded windows.
# ------------------------------------------------------------------
::itcl::body Hierarchy::window {option args} {
    return [uplevel 0 $list window $option $args]
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: xview args
#
# Thin wrap of the text widget's xview command.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::xview {args} {
    return [uplevel 0 $list xview $args]
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: yview args
#
# Thin wrap of the text widget's yview command.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::yview {args} {
    return [uplevel 0 $list yview $args]
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: expanded node
#
# Tells if a node is expanded or collapsed
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::expanded {node} {
    if {![info exists _states($node)]} {
	error "bad collapse node argument: \"$node\", the node doesn't exist"
    }
    return $_states($node)
}

# ----------------------------------------------------------------------
# PUBLIC METHOD: expState
#
# Returns a list of all expanded nodes
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::expState {} {
    set nodes [_contents ""]
    set open ""
    set i 0
    while {1} {
	if {[info exists _states([lindex $nodes $i])] &&
	$_states([lindex $nodes $i])} {
	    lappend open [lindex $nodes $i]
	    foreach child [_contents [lindex $nodes $i]] {
		lappend nodes $child
	    }
	}
	incr i
	if {$i >= [llength $nodes]} {break}
    }
    return $open
}

# ------------------------------------------------------------------
#                       PROTECTED METHODS
# ------------------------------------------------------------------

# ----------------------------------------------------------------------
# PROTECTED METHOD: _drawLevel node indent
#
# Used internally by draw to draw one level of the hierarchy.
# Draws all of the nodes under node, using the indent string to
# indent nodes.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_drawLevel {node indent} {
    lappend _markers "$node:start" [$list index insert]
    set bg [$list cget -background]
    # Obtain the list of subnodes for this node and cycle through
    # each one displaying it in the hierarchy.
    #
    foreach child [_contents $node] {
	set _images($child) "$list.hicon[incr _hcounter]"
        if {![info exists _states($child)]} {
            set _states($child) $itcl_options(-expanded)
        }
	#
	# Check the user tags to see if they have been kind enough
	# to tell us ahead of time what type of node we are dealing
	# with branch or leaf.  If they neglected to do so, then
	# get the contents of the child node to see if it has children
	# itself.
	#
	set display 0
	if {[lsearch $_tags($child) leaf] != -1} {
	    set type leaf
	} elseif {[lsearch $_tags($child) branch] != -1} {
	    set type branch
	} else {
	    if {[llength [_contents $child]] == 0} {
		set type leaf
	    } else {
		set type branch
	    }
	}
	#
	# Now that we know the type of node, branch or leaf, we know
	# the type of icon to use.
	#
	if {$type eq "leaf"} {
            set icon $itcl_options(-nodeicon)
            eval $_filterCode
	} else {
            if {$_states($child)} {
                set icon $itcl_options(-openicon)
            } else {
                set icon $itcl_options(-closedicon)
            }
            set display 1
	}
	#
	# If display is set then we're going to be drawing this node.
	# Save off the indentation level for this node and do the indent.
	#
	if {$display} {
	    set _indents($child) "$indent\t"
	    $list insert insert $indent
	    #
	    # Add the branch or leaf icon and setup a binding to toggle
	    # its expanded/collapsed state.
	    #
	    label $_images($child) -image $icon -background $bg 
	    # DRH - enhanced and added features that handle image clicking,
	    # double clicking, and right clicking behavior
	    bind $_images($child) <ButtonPress-1> \
	      "[itcl::code $this toggle $child]; [itcl::code $this _imageSelect $child]"
	    bind $_images($child) <Double-1> [itcl::code $this _imageDblClick $child]
	    bind $_images($child) <ButtonPress-3> \
	      [itcl::code $this _imagePost $child $_images($child) $type %x %y]
	    $list window create insert -window $_images($child)
	    #
	    # If any user icons exist then draw them as well.  The little
	    # regexp is just to check and see if they've passed in a
	    # command which needs to be evaluated as opposed to just
	    # a variable.  Also, attach a binding to call them if their
	    # icon is selected.
	    #
	    if {[info exists _icons($child)]} {
		foreach image $_icons($child) {
		    set wid "$list.uicon[incr _ucounter]"
		    if {[regexp {\[.*\]} $image]} {
			uplevel 0 label $wid -image $image -background $bg 
		    } else {
			label $wid -image $image -background $bg 
		    }
		    # DRH - this will bind events to the icons to allow
		    # clicking, double clicking, and right clicking actions.
		    bind $wid <ButtonPress-1> \
			    [itcl::code $this _iconSelect $child $image]
		    bind $wid <Double-1> \
			    [itcl::code $this _iconDblSelect $child $image]
		    bind $wid <ButtonPress-3> \
			    [itcl::code $this _imagePost $child $wid $type %x %y]
		    $list window create insert -window $wid
		}
	    }
	    #
	    # Create the list of tags to be applied to the text.  Start
	    # out with a tag of "info" and append "hilite" if the node
	    # is currently selected, finally add the tags given by the
	    # user.
	    #
	    set texttags [list "info" $child]

	    if {[info exists _selected($child)]} {
		lappend texttags hilite
	    } 
            # The following conditional added for SF ticket #600941.
            if {[info exists _marked($child)]} { 
                lappend texttags lowlite 
            } 
	    foreach tag $_tags($child) {
		lappend texttags $tag
	    }
	    #
	    # Insert the text for the node along with the tags and 
	    # append to the markers the start of this node.  The text
	    # has been broken at newlines into a list.  We'll make sure
	    # that each line is at the same indentation position.
	    #
	    set firstline 1
	    foreach line $_text($child) {
		if {$firstline} {
		    $list insert insert " "
		} else {
		    $list insert insert "$indent\t"
		}

		$list insert insert $line $texttags "\n"
		set firstline 0
	    }
	    $list tag raise $child
	    lappend _markers "$child:start" [$list index insert]
	    #
	    # If the state of the node is open, proceed to draw the next 
	    # node below it in the hierarchy.
	    #
	    if {$_states($child)} {
		_drawLevel $child "$indent\t"
	    }
	}
    }
    lappend _markers "$node:end" [$list index insert]
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _contents uid
#
# Used internally to get the contents of a particular node.  If this
# is the first time the node has been seen or the -alwaysquery
# option is set, the -querycommand code is executed to query the node 
# list, and the list is stored until the next time it is needed.
#
# The querycommand may return not only the list of subnodes for the 
# node but additional information on the tags and icons to be used.  
# The return value must be parsed based on the number of elements in 
# the list where the format is a list of lists:
#
# {{uid [text [tags [icons]]]} {uid [text [tags [icons]]]} ...}
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_contents {uid} {
    if {$itcl_options(-alwaysquery)} {
    } else {
      if {[info exists _nodes($uid)]} {
          return $_nodes($uid)
      }
    }
    # 
    # Substitute any %n's for the node name whose children we're
    # interested in obtaining.
    #
    set cmd $itcl_options(-querycommand)
    regsub -all {%n} $cmd [list $uid] cmd
    set nodeinfolist [uplevel \#0 $cmd]
    #
    # Cycle through the node information returned by the query
    # command determining if additional information such as text,
    # user tags, or user icons have been provided.  For text,
    # break it into a list at any newline characters.
    #
    set _nodes($uid) {}
    foreach nodeinfo $nodeinfolist {
	set subnodeuid [lindex $nodeinfo 0]
	lappend _nodes($uid) $subnodeuid
	set llen [llength $nodeinfo] 
	if {$llen == 0 || $llen > 4} {
	    error "invalid number of elements returned by query\
                       command for node: \"$uid\",\
                       should be uid \[text \[tags \[icons\]\]\]"
	}
	if {$llen == 1} {
	    set _text($subnodeuid) [split $subnodeuid \n]
	} 
	if {$llen > 1} {
	    set _text($subnodeuid) [split [lindex $nodeinfo 1] \n]
	}
	if {$llen > 2} {
	    set _tags($subnodeuid) [lindex $nodeinfo 2]
	} else {
	    set _tags($subnodeuid) unknown
	}
	if {$llen > 3} {
	    set _icons($subnodeuid) [lindex $nodeinfo 3]
	}
    }
    #
    # Return the list of nodes.
    #
    return $_nodes($uid)
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _post x y
#
# Used internally to post the popup menu at the coordinate (x,y)
# relative to the widget.  If (x,y) is on an item, then the itemMenu
# component is posted.  Otherwise, the bgMenu is posted.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_post {x y} {
    set rx [expr {[winfo rootx $list]+$x}]
    set ry [expr {[winfo rooty $list]+$y}]
    set index [$list index @$x,$y]
    #
    # The posted variable will hold the list of tags which exist at
    # this x,y position that will be passed back to the user.  They
    # don't need to know about our internal tags, info, hilite, and
    # lowlite, so remove them from the list.
    # 
    set _posted {}
    foreach tag [$list tag names $index] {
        if {![_isInternalTag $tag]} {
            lappend _posted $tag
        }
    }
    #
    # If we have tags then do the popup at this position.
    #
    if {$_posted != {}} {
	# DRH - here is where the user's function for dynamic popup
	# menu loading is done, if the user has specified to do so with the
	# "-textmenuloadcommand"
	if {$itcl_options(-textmenuloadcommand) != {}} {
	    eval $itcl_options(-textmenuloadcommand)
	}
	tk_popup $itemMenu $rx $ry
    } else {
	tk_popup $bgMenu $rx $ry
    }
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _imagePost node image type x y
#
# Used internally to post the popup menu at the coordinate (x,y)
# relative to the widget.  If (x,y) is on an image, then the itemMenu
# component is posted.
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_imagePost {node image type x y} {
    set rx [expr {[winfo rootx $image]+$x}]
    set ry [expr {[winfo rooty $image]+$y}]
    #
    # The posted variable will hold the list of tags which exist at
    # this x,y position that will be passed back to the user.  They
    # don't need to know about our internal tags, info, hilite, and
    # lowlite, so remove them from the list.
    # 
    set _posted {}
    lappend _posted $node $type
    #
    # If we have tags then do the popup at this position.
    #
    if {$itcl_options(-imagemenuloadcommand) != {}} {
	eval $itcl_options(-imagemenuloadcommand)
    }
    tk_popup $itemMenu $rx $ry
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _select x y
#
# Used internally to select an item at the coordinate (x,y) relative 
# to the widget.  The command associated with the -selectcommand
# option is execute following % character substitutions.  If %n
# appears in the command, the selected node is substituted.  If %s
# appears, a boolean value representing the current selection state
# will be substituted.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_select {x y} {
    if {$itcl_options(-selectcommand) != {}} {
	if {[set seltags [$list tag names @$x,$y]] != {}} {
	    foreach tag $seltags {
		if {![_isInternalTag $tag]} {
		    lappend node $tag
		}
	    }

	    if {[lsearch $seltags "hilite"] == -1} {
		set selectstatus 0
	    } else {
		set selectstatus 1
	    }
	    set cmd $itcl_options(-selectcommand)
	    regsub -all {%n} $cmd [lindex $node end] cmd
	    regsub -all {%s} $cmd [list $selectstatus] cmd
	    uplevel #0 $cmd
	}
    }
    return
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _double x y
#
# Used internally to double click an item at the coordinate (x,y) relative 
# to the widget.  The command associated with the -dblclickcommand
# option is execute following % character substitutions.  If %n
# appears in the command, the selected node is substituted.  If %s
# appears, a boolean value representing the current selection state
# will be substituted.
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_double {x y} {
    if {$itcl_options(-dblclickcommand) ne {}} {
	if {[set seltags [$list tag names @$x,$y]] != {}} {
	    foreach tag $seltags {
		if {![_isInternalTag $tag]} {
		    lappend node $tag
		}
	    }
	    if {[lsearch $seltags "hilite"] == -1} {
		set selectstatus 0
	    } else {
		set selectstatus 1
	    }
	    set cmd $itcl_options(-dblclickcommand)
	    regsub -all {%n} $cmd [list $node] cmd
	    regsub -all {%s} $cmd [list $selectstatus] cmd
	    uplevel #0 $cmd
	}
    }
    return
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _iconSelect node icon
#
# Used internally to upon selection of user icons.  The -iconcommand
# is executed after substitution of the node for %n and icon for %i.
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_iconSelect {node icon} {
    set cmd $itcl_options(-iconcommand)
    regsub -all {%n} $cmd [list $node] cmd
    regsub -all {%i} $cmd [list $icon] cmd
    uplevel \#0 $cmd
    return {}
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _iconDblSelect node icon
#
# Used internally to upon double selection of user icons.  The 
# -icondblcommand is executed after substitution of the node for %n and 
# icon for %i.
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_iconDblSelect {node icon} {
    if {$itcl_options(-icondblcommand) ne {}} {
	set cmd $itcl_options(-icondblcommand)
	regsub -all {%n} $cmd [list $node] cmd
	regsub -all {%i} $cmd [list $icon] cmd
	uplevel \#0 $cmd
    }
    return {}
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _imageSelect node icon
#
# Used internally to upon selection of user icons.  The -imagecommand
# is executed after substitution of the node for %n.
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_imageSelect {node} {
    if {$itcl_options(-imagecommand) ne {}} {
	set cmd $itcl_options(-imagecommand)
	regsub -all {%n} $cmd [list $node] cmd
	uplevel \#0 $cmd
    }
    return {}
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _imageDblClick node
#
# Used internally to upon double selection of images.  The 
# -imagedblcommand is executed.
#
# Douglas R. Howard, Jr.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_imageDblClick {node} {
    if {$itcl_options(-imagedblcommand) != {}} {
	set cmd $itcl_options(-imagedblcommand)
	regsub -all {%n} $cmd [list $node] cmd
	uplevel \#0 $cmd
    }
    return {}
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _deselectSubNodes uid
#
# Used internally to recursively deselect all the nodes beneath a 
# particular node.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_deselectSubNodes {uid} {
    foreach node $_nodes($uid) {
	if {[array names _selected $node] != {}} {
	    unset _selected($node)
	}
	if {[array names _nodes $node] != {}} {
	    _deselectSubNodes $node
	}
    }
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _deleteNodeInfo uid
#
# Used internally to recursively delete all the information about a
# node and its decendents.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_deleteNodeInfo {uid} {
    #
    # Recursively call ourseleves as we go down the hierarchy beneath
    # this node.
    #
    if {[info exists _nodes($uid)]} {
	foreach node $_nodes($uid) {
	    if {[array names _nodes $node] != {}} {
		_deleteNodeInfo $node
	    }
	}
    }
    #
    # Unset any entries in our arrays for the node.
    #
    catch {unset _nodes($uid)}
    catch {unset _text($uid)}
    catch {unset _tags($uid)}
    catch {unset _icons($uid)}
    catch {unset _states($uid)}
    catch {unset _images($uid)}
    catch {unset _indents($uid)}
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _getParent uid
#
# Used internally to determine the parent for a node.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_getParent {uid} {
    foreach node [array names _nodes] {
	if {[set index [lsearch $_nodes($node) $uid]] != -1} {
	    return $node
	}
    }
}

# ----------------------------------------------------------------------
# PROTECTED METHOD: _getHeritage uid
#
# Used internally to determine the list of parents for a node.
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_getHeritage {uid} {
    set parents {}
    if {[set parent [_getParent $uid]] != {}} {
	lappend parents $parent
    }
    return $parents
}

# ----------------------------------------------------------------------
# PROTECTED METHOD (could be proc?): _isInternalTag tag
#
# Used internally to tags not to used for user callback commands
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_isInternalTag {tag} {
   set ii [expr {[lsearch -exact {info hilite lowlite unknown} $tag] != -1}]
   return $ii
}

# ----------------------------------------------------------------------
# PRIVATE METHOD: _configureTags
#
# This method added to fix SF ticket #596111.  When the -querycommand
# is reset after initial construction, the text component loses its
# tag configuration.  This method resets the hilite, lowlite, and info
# tags.  csmith: 9/5/02
# ----------------------------------------------------------------------
::itcl::body Hierarchy::_configureTags {} {
  tag configure hilite -background $itcl_options(-selectbackground) \
    -foreground $itcl_options(-selectforeground)
  tag configure lowlite -background $itcl_options(-markbackground) \
    -foreground $itcl_options(-markforeground)
  tag configure info -font $itcl_options(-font) -spacing1 6
}

} ; # end ::itcl::widgets
