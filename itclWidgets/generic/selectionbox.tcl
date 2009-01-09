#
# Selectionbox
# ----------------------------------------------------------------------
# Implements a selection box composed of a scrolled list of items and
# a selection entry field.  The user may choose any of the items displayed
# in the scrolled list of alternatives and the selection field will be
# filled with the choice.  The user is also free to enter a new value in
# the selection entry field.  Both the list and entry areas have labels.
# A child site is also provided in which the user may create other widgets
# to be used in conjunction with the selection box.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Selectionbox
# written by:
#  AUTHOR: Mark L. Ulferts              EMAIL: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: selectionbox.tcl,v 1.1.2.1 2009/01/09 20:55:26 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Selectionbox.itemsLabel Items widgetDefault
option add *Selectionbox.selectionLabel Selection widgetDefault
option add *Selectionbox.width 260 widgetDefault
option add *Selectionbox.height 320 widgetDefault

namespace eval ::itcl::widgets {

# Provide a lowercased access method for the Selectionbox class.
# 
proc ::itcl::widgets::selectionbox {pathName args} {
    uplevel ::itcl::widgets::Selectionbox $pathName $args
}

# ------------------------------------------------------------------
#                            SELECTIONBOX
# ------------------------------------------------------------------
::itcl::extendedclass Selectionbox {
    component itcl_hull
    component itcl_interior
    protected component sbchildsite
    component items
    component selection

    option [list -childsitepos childSitePos Position] -default center -configuremethod configChildsitepos
    option [list -margin margin Margin] -default 7 -configuremethod configMargin
    option [list -itemson itemsOn ItemsOn] -default true -configuremethod configItemson
    option [list -selectionon selectionOn SelectionOn] -default true -configuremethod configSelectionon
    option [list -width width Width] -default 260 -configuremethod configWidth
    option [list -height height Height] -default 320 -configuremethod configHeight

    delegate option [list -itemslabel itemsLabel Text] to items as -labeltext
    delegate option [list -itemscommand itemsCommand Command] to items as -selectioncommand
    delegate option [list -selectionlabel selectionLabel Text] to selection as -labeltext
    delegate option [list -selectioncommand selectionCommand Command] to selection as -command

    private variable _repacking {}     ;# non-null => _packComponents pending

    constructor {args} {}
    destructor {}

    private method _packComponents {{when later}}

    protected method configChildsitepos {option value}
    protected method configMargin {option value}
    protected method configItemson {option value}
    protected method configSelectionon {option value}
    protected method configWidth {option value}
    protected method configHeight {option value}

    public method childsite {}
    public method get {}
    public method curselection {}
    public method clear {component}
    public method insert {component index args}
    public method delete {first {last {}}}
    public method size {}
    public method scan {option args}
    public method nearest {y}
    public method index {index}
    public method selection {option args}
    public method selectitem {}

public method component {what args} {
    if {[::info exists $what]} {
        return [uplevel 0 [set $what] $args]
    }
    error "no such component \"$what\""
}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Selectionbox::constructor {args} {
    createhull frame $this -class [info class]
    #
    # Set the borderwidth to zero and add width and height options
    # back to the hull.
    #
# FIXME    $win configure -borderwidth 0
# FIXME   itcl_options add hull.width hull.height
    #
    # Create the child site widget.
    #
    setupcomponent sbchildsite using frame $itcl_interior.sbchildsite
    #
    # Create the items list.
    #
#	 -insertborderwidth -insertofftime -insertontime \
	 -insertwidth \
	 -insertbackground \

    setupcomponent items using ::itcl::widgets::Scrolledlistbox \
                $itcl_interior.items -selectmode single \
		-visibleitems 20x10 -labelpos nw -vscrollmode static \
		-hscrollmode none 
    keepcomponentoption items -activebackground -activerelief -background \
         -borderwidth -cursor \
	 -elementborderwidth -foreground -highlightcolor -highlightthickness \
	 -jump -labelfont -selectbackground -selectborderwidth \
	 -selectforeground -textbackground -textfont -troughcolor
    keepcomponentoption items -dblclickcommand -exportselection 
    configure -itemscommand [itcl::code $this selectitem]
    #
    # Create the selection entry.
    #
    setupcomponent selection using ::itcl::widgets::Entryfield \
        $itcl_interior.selection -labelpos nw
    keepcomponentoption selection -activebackground -activerelief -background \
         -borderwidth -cursor \
	 -elementborderwidth -foreground -highlightcolor -highlightthickness \
	 -insertbackground -insertborderwidth -insertofftime -insertontime \
	 -insertwidth -jump -labelfont -selectbackground -selectborderwidth \
	 -selectforeground -textbackground -textfont -troughcolor
    keepcomponentoption selection -exportselection 
    #
    # Set the interior to the childsite for derived classes.
    #
    set itcl_interior $sbchildsite
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
    # 
    # When idle, pack the components.
    #
    _packComponents
}   

# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Selectionbox::destructor {} {
    if {$_repacking != ""} {
        after cancel $_repacking
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -childsitepos
#
# Specifies the position of the child site in the selection box.
# ------------------------------------------------------------------
::itcl::body Selectionbox::configChildsitepos {option value} {
    set itcl_options($option) $value
    _packComponents 
}

# ------------------------------------------------------------------
# OPTION: -margin
#
# Specifies distance between the items list and selection entry.
# ------------------------------------------------------------------
::itcl::body Selectionbox::configMargin {option value} {
    set itcl_options($option) $value
    _packComponents 
}

# ------------------------------------------------------------------
# OPTION: -itemson
#
# Specifies whether or not to display the items list.
# ------------------------------------------------------------------
::itcl::body Selectionbox::configItemson {option value} {
    set itcl_options($option) $value
    _packComponents 
}

# ------------------------------------------------------------------
# OPTION: -selectionon
#
# Specifies whether or not to display the selection entry widget.
# ------------------------------------------------------------------
::itcl::body Selectionbox::configSelectionon {option value} {
    set itcl_options($option) $value
    _packComponents
}

# ------------------------------------------------------------------
# OPTION: -width
#
# Specifies the width of the hull.  The value may be specified in 
# any of the forms acceptable to Tk_GetPixels.  A value of zero 
# causes the width to be adjusted to the required value based on 
# the size requests of the components.  Otherwise, the width is 
# fixed.
# ------------------------------------------------------------------
::itcl::body Selectionbox::configWidth {option value} {
    #
    # The width option was added to the hull in the constructor.
    # So, any width value given is passed automatically to the
    # hull.  All we have to do is play with the propagation.
    #
    if {$value != 0} {
	set propagate 0
    } else {
	set propagate 1
    }
    #
    # Due to a bug in the tk4.2 grid, we have to check the 
    # propagation before setting it.  Setting it to the same
    # value it already is will cause it to toggle.
    #
    if {[grid propagate $win] != $propagate} {
	grid propagate $win $propagate
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -height
#
# Specifies the height of the hull.  The value may be specified in 
# any of the forms acceptable to Tk_GetPixels.  A value of zero 
# causes the height to be adjusted to the required value based on 
# the size requests of the components. Otherwise, the height is 
# fixed.
# ------------------------------------------------------------------
::itcl::body Selectionbox::configHeight {option value} {
    #
    # The height option was added to the hull in the constructor.
    # So, any height value given is passed automatically to the
    # hull.  All we have to do is play with the propagation.
    #
    if {$value != 0} {
	set propagate 0
    } else {
	set propagate 1
    }
    #
    # Due to a bug in the tk4.2 grid, we have to check the 
    # propagation before setting it.  Setting it to the same
    # value it already is will cause it to toggle.
    #
    if {[grid propagate $win] != $propagate} {
	grid propagate $win $propagate
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Returns the path name of the child site widget.
# ------------------------------------------------------------------
::itcl::body Selectionbox::childsite {} {
    return $sbchildsite
}

# ------------------------------------------------------------------
# METHOD: get 
#
# Returns the current selection.
# ------------------------------------------------------------------
::itcl::body Selectionbox::get {} {
    return [$selection get]
}

# ------------------------------------------------------------------
# METHOD: curselection
#
# Returns the current selection index.
# ------------------------------------------------------------------
::itcl::body Selectionbox::curselection {} {
    return [$items curselection]
}

# ------------------------------------------------------------------
# METHOD: clear component
#
# Delete the contents of either the selection entry widget or items
# list.
# ------------------------------------------------------------------
::itcl::body Selectionbox::clear {comp} {
    switch $comp {
    selection {
        $selection clear
      }
    items {
        delete 0 end
      }
    default {
        error "bad clear argument \"$comp\": should be\
	       selection or items"
      }
    }
}

# ------------------------------------------------------------------
# METHOD: insert component index args
#
# Insert element(s) into either the selection or items list widget.
# ------------------------------------------------------------------
::itcl::body Selectionbox::insert {comp index args} {
    switch $comp {
    selection {
        uplevel 0 $selection insert $index $args
      }
    items {
        uplevel 0 $items insert $index $args
      }
    default {
        error "bad insert argument \"$comp\": should be\
	       selection or items"
      }
    }
}

# ------------------------------------------------------------------
# METHOD: delete first ?last?
#
# Delete one or more elements from the items list box.  The default 
# is to delete by indexed range. If an item is to be removed by name, 
# it must be preceeded by the keyword "item". Only index numbers can 
# be used to delete a range of items. 
# ------------------------------------------------------------------
::itcl::body Selectionbox::delete {first {last {}}} {
    set first [index $first]
    if {$last ne {}} {
	set last [index $last]
    } else {
	set last $first
    }
    if {$first <= $last} {
	uplevel 0 $items delete $first $last
    } else {
	error "first index must not be greater than second"
    }
}

# ------------------------------------------------------------------
# METHOD: size 
#
# Returns a decimal string indicating the total number of elements 
# in the items list.
# ------------------------------------------------------------------
::itcl::body Selectionbox::size {} {
    return [$items size]
}

# ------------------------------------------------------------------
# METHOD: scan option args 
#
# Implements scanning on items list.
# ------------------------------------------------------------------
::itcl::body Selectionbox::scan {option args} {
    uplevel 0 $items scan $option $args
}

# ------------------------------------------------------------------
# METHOD: nearest y
#
# Returns the index to the nearest listbox item given a y coordinate.
# ------------------------------------------------------------------
::itcl::body Selectionbox::nearest {y} {
    return [$items nearest $y]
}

# ------------------------------------------------------------------
# METHOD: index index
#
# Returns the decimal string giving the integer index corresponding 
# to index.
# ------------------------------------------------------------------
::itcl::body Selectionbox::index {index} {
    return [$items index $index]
}

# ------------------------------------------------------------------
# METHOD: selection option args
#
# Adjusts the selection within the items list.
# ------------------------------------------------------------------
::itcl::body Selectionbox::selection {option args} {
    uplevel 0 $items selection $option $args
    selectitem
}

# ------------------------------------------------------------------
# METHOD: selectitem
#
# Replace the selection entry field contents with the currently 
# selected items value.
# ------------------------------------------------------------------
::itcl::body Selectionbox::selectitem {} {
    $selection clear
    set numSelected [$items selecteditemcount]
    if {$numSelected == 1} {
	$selection insert end [$items getcurselection]
    } elseif {$numSelected > 1} {
	$selection insert end [lindex [$items getcurselection] 0]
    }
    $selection icursor end
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _packComponents ?when?
#
# Pack the selection, items, and child site widgets based on options.
# If "when" is "now", the change is applied immediately.  If it is 
# "later" or it is not specified, then the change is applied later, 
# when the application is idle.
# ------------------------------------------------------------------
::itcl::body Selectionbox::_packComponents {{when later}} {
    if {$when eq "later"} {
	if {$_repacking eq ""} {
	    set _repacking [after idle [itcl::code $this _packComponents now]]
	}
	return
    } elseif {$when ne "now"} {
	error "bad option \"$when\": should be now or later"
    }
    set _repacking ""
    set parent [winfo parent $sbchildsite]
    set margin [winfo pixels $win $itcl_options(-margin)]

    switch $itcl_options(-childsitepos) {
    n {
        grid $sbchildsite -row 0 -column 0 -sticky nsew -rowspan 1
        grid $items -row 1 -column 0 -sticky nsew
        grid $selection -row 3 -column 0 -sticky ew
        grid rowconfigure $parent 0 -weight 0 -minsize 0
        grid rowconfigure $parent 1 -weight 1 -minsize 0
        grid rowconfigure $parent 2 -weight 0 -minsize $margin
        grid rowconfigure $parent 3 -weight 0 -minsize 0
        grid columnconfigure $parent 0 -weight 1 -minsize 0
        grid columnconfigure $parent 1 -weight 0 -minsize 0
      }
    w {
        grid $sbchildsite -row 0 -column 0 -sticky nsew -rowspan 3
        grid $items -row 0 -column 1 -sticky nsew
        grid $selection -row 2 -column 1 -sticky ew
        grid rowconfigure $parent 0 -weight 1 -minsize 0
        grid rowconfigure $parent 1 -weight 0 -minsize $margin
        grid rowconfigure $parent 2 -weight 0 -minsize 0
        grid rowconfigure $parent 3 -weight 0 -minsize 0
        grid columnconfigure $parent 0 -weight 0 -minsize 0
        grid columnconfigure $parent 1 -weight 1 -minsize 0
      }
    s {
        grid $items -row 0 -column 0 -sticky nsew
        grid $selection -row 2 -column 0 -sticky ew
        grid $sbchildsite -row 3 -column 0 -sticky nsew -rowspan 1
        grid rowconfigure $parent 0 -weight 1 -minsize 0
        grid rowconfigure $parent 1 -weight 0 -minsize $margin
        grid rowconfigure $parent 2 -weight 0 -minsize 0
        grid rowconfigure $parent 3 -weight 0 -minsize 0
        grid columnconfigure $parent 0 -weight 1 -minsize 0
        grid columnconfigure $parent 1 -weight 0 -minsize 0
      }
    e {
        grid $items -row 0 -column 0 -sticky nsew
        grid $selection -row 2 -column 0 -sticky ew
        grid $sbchildsite -row 0 -column 1 -sticky nsew -rowspan 3
        grid rowconfigure $parent 0 -weight 1 -minsize 0
        grid rowconfigure $parent 1 -weight 0 -minsize $margin
        grid rowconfigure $parent 2 -weight 0 -minsize 0
        grid rowconfigure $parent 3 -weight 0 -minsize 0
        grid columnconfigure $parent 0 -weight 1 -minsize 0
        grid columnconfigure $parent 1 -weight 0 -minsize 0
      }
    center {
        grid $items -row 0 -column 0 -sticky nsew
        grid $sbchildsite -row 1 -column 0 -sticky nsew -rowspan 1
        grid $selection -row 3 -column 0 -sticky ew
        grid rowconfigure $parent 0 -weight 1 -minsize 0
        grid rowconfigure $parent 1 -weight 0 -minsize 0
        grid rowconfigure $parent 2 -weight 0 -minsize $margin
        grid rowconfigure $parent 3 -weight 0 -minsize 0
        grid columnconfigure $parent 0 -weight 1 -minsize 0
        grid columnconfigure $parent 1 -weight 0 -minsize 0
      }
    default {
        error "bad childsitepos option \"$itcl_options(-childsitepos)\":\
	       should be n, e, s, w, or center"
      }
    }
    if {!$itcl_options(-itemson)} {
	grid forget $items
    }
    if {!$itcl_options(-selectionon)} {
	grid forget $selection
    }
    raise $sbchildsite
}

} ; # end ::itcl::widgets
