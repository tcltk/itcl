#
# Scrolledlistbox
# ----------------------------------------------------------------------
# Implements a scrolled listbox with additional options to manage
# horizontal and vertical scrollbars.  This includes options to control
# which scrollbars are displayed and the method, i.e. statically,
# dynamically, or none at all.  
#
# ----------------------------------------------------------------------
#  AUTHOR: Mark L. Ulferts             EMAIL: mulferts@austin.dsccc.com
#
#  @(#) $Id: scrolledlistbox.tcl,v 1.1.2.1 2008/12/28 12:10:38 wiede Exp $
# ----------------------------------------------------------------------
#            Copyright (c) 1995 DSC Technologies Corporation
# ======================================================================
# Permission to use, copy, modify, distribute and license this software 
# and its documentation for any purpose, and without fee or written 
# agreement with DSC, is hereby granted, provided that the above copyright 
# notice appears in all copies and that both the copyright notice and 
# warranty disclaimer below appear in supporting documentation, and that 
# the names of DSC Technologies Corporation or DSC Communications 
# Corporation not be used in advertising or publicity pertaining to the 
# software without specific, written prior permission.
# 
# DSC DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
# ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, AND NON-
# INFRINGEMENT. THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, AND THE
# AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE MAINTENANCE, 
# SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. IN NO EVENT SHALL 
# DSC BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR 
# ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS 
# SOFTWARE.
# ======================================================================

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Scrolledlistbox class.
# 
proc ::itcl::widgets::scrolledlistbox {pathName args} {
    uplevel ::itcl::widgets::Scrolledlistbox $pathName $args
}

#
# Use option database to override default resources of base classes.
#
option add *Scrolledlistbox.labelPos n widgetDefault

# ------------------------------------------------------------------
#                          SCROLLEDLISTBOX
# ------------------------------------------------------------------
::itcl::class ::itcl::widgets::Scrolledlistbox {
    inherit ::itcl::widgets::Scrolledwidget

    #
    # List the event sequences that invoke single and double selection.
    # Should these change in the underlying Tk listbox, then they must
    # change here too.
    #
    common doubleSelectSeq [list \
        <Double-1> 
    ]

    common singleSelectSeq [list \
        <Control-Key-backslash> \
        <Control-Key-slash> \
        <Key-Escape> \
        <Shift-Key-Select> \
        <Control-Shift-Key-space> \
        <Key-Select> \
        <Key-space> \
        <Control-Shift-Key-End> \
        <Control-Key-End> \
        <Control-Shift-Key-Home> \
        <Control-Key-Home> \
        <Key-Down> \
        <Key-Up> \
        <Shift-Key-Down> \
        <Shift-Key-Up> \
        <Control-Button-1> \
        <Shift-Button-1> \
        <ButtonRelease-1> \
    ]

    component listbox
    component vertsb
    component horizsb

    option [list -dblclickcommand dblClickCommand Command] -default {}
    option [list -selectioncommand selectionCommand Command] -default {}
    option [list -width width Width] -default 0 -configurecommand configWidth
    option [list -height height Height] -default 0 -configurecommand configHeight
    option [list -visibleitems visibleItems VisibleItems] -default 20x10 -configurecommand configVisibleitems
    option [list -state state State] -default normal -configurecommand configState

    constructor {args} {}
    destructor {}

    protected method _makeSelection {} 
    protected method _dblclick {} 
    protected method _fixIndex {index}
    protected method configState {option value}

    public method curselection {} 
    public method activate {index} 
    public method bbox {index} 
    public method clear {} 
    public method see {index} 
    public method index {index} 
    public method delete {first {last {}}} 
    public method get {first {last {}}} 
    public method getcurselection {} 
    public method insert {index args} 
    public method nearest {y} 
    public method scan {option args} 
    public method selection {option first {last {}}} 
    public method size {} 
    public method selecteditemcount {} 
    public method justify {direction} 
    public method sort {{mode ascending}} 
    public method xview {args} 
    public method yview {args} 
    public method itemconfigure {args}

}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::constructor {args} {
    #
    # Our -width and -height options are slightly different than
    # those implemented by our base class, so we're going to
    # remove them and redefine our own.
    #
#    itcl_options remove ::itcl::widgets::Scrolledwidget::width
#    itcl_options remove ::itcl::widgets::Scrolledwidget::height

    # 
    # Create the listbox.
    #
    setupcomponent listbox using listbox $itk_interior.listbox \
        -width 1 -height 1 \
        -xscrollcommand \
        [itcl::code $this _scrollWidget $itk_interior.horizsb] \
        -yscrollcommand \
        [itcl::code $this _scrollWidget $itk_interior.vertsb]
    keepcomponentoption listbox -activebackground -activerelief -background \
        -borderwidth -cursor -elementborderwidth -foreground -highlightcolor \
	-highlightthickness -jump -labelfont -selectbackground \
	-selectborderwidth -selectforeground -textbackground -textfont \
	-troughcolor 

    keepcomponentoption listbox -borderwidth -exportselection -relief \
        -selectmode -listvariable
    
#    rename -font -textfont textFont Font
#    rename -background -textbackground textBackground Background
#    rename -highlightbackground -background background Background

    grid $listbox -row 0 -column 0 -sticky nsew
    grid rowconfigure $_interior 0 -weight 1
    grid columnconfigure $_interior 0 -weight 1
    
    # 
    # Configure the command on the vertical scroll bar in the base class.
    #
    $vertsb configure -command [itcl::code $listbox yview]

    #
    # Configure the command on the horizontal scroll bar in the base class.
    #
    $horizsb configure -command [itcl::code $listbox xview]
    
    # 
    # Create a set of bindings for monitoring the selection and install
    # them on the listbox component.
    #
    foreach seq $singleSelectSeq {
        bind SLBSelect$this $seq [itcl::code $this _makeSelection]
    }

    foreach seq $doubleSelectSeq {
        bind SLBSelect$this $seq [itcl::code $this _dblclick]
    }

    bindtags $listbox [linsert [bindtags $listbox] end SLBSelect$this]

    #
    # Also create a set of bindings for disabling the scrolledlistbox.
    # Since the command for it is "break", we can drop the $this since
    # they don't need to be unique to the object level.
    #
    if {[bind SLBDisabled] == {}} {
        foreach seq $singleSelectSeq {
            bind SLBDisabled $seq break
        }
        bind SLBDisabled <Button-1> break
        foreach seq $doubleSelectSeq {
            bind SLBDisabled $seq break
        }
    }

    #
    # Initialize the widget based on the command line options.
    #
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

# ------------------------------------------------------------------
#                           DESTURCTOR
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::destructor {} {
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -width
#
# Specifies the width of the scrolled list box as an entire unit.
# The value may be specified in any of the forms acceptable to 
# Tk_GetPixels.  Any additional space needed to display the other
# components such as margins and scrollbars force the listbox
# to be compressed.  A value of zero along with the same value for 
# the height causes the value given for the visibleitems option 
# to be applied which administers geometry constraints in a different
# manner.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::configWidth {option value} {
    if {$value != 0} {
        set shell [lindex [grid info $listbox] 1]
        #
        # Due to a bug in the tk4.2 grid, we have to check the 
        # propagation before setting it.  Setting it to the same
        # value it already is will cause it to toggle.
        #
        if {[grid propagate $shell]} {
            grid propagate $shell no
        }
        $listbox configure -width 1
        $shell configure -width [winfo pixels $shell $value] 
    } else {
        configure -visibleitems $itcl_options(-visibleitems)
    }
}

# ------------------------------------------------------------------
# OPTION: -height
#
# Specifies the height of the scrolled list box as an entire unit.
# The value may be specified in any of the forms acceptable to 
# Tk_GetPixels.  Any additional space needed to display the other
# components such as margins and scrollbars force the listbox
# to be compressed.  A value of zero along with the same value for 
# the width causes the value given for the visibleitems option 
# to be applied which administers geometry constraints in a different
# manner.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::configHeight {option value} {
    if {$value != 0} {
        set shell [lindex [grid info $listbox] 1]

        #
        # Due to a bug in the tk4.2 grid, we have to check the 
        # propagation before setting it.  Setting it to the same
        # value it already is will cause it to toggle.
        #
        if {[grid propagate $shell]} {
            grid propagate $shell no
        }
    
        $listbox configure -height 1
        $shell configure -height [winfo pixels $shell $value] 
    } else {
        configure -visibleitems $itcl_options(-visibleitems)
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -visibleitems
#
# Specified the widthxheight in characters and lines for the listbox.
# This option is only administered if the width and height options
# are both set to zero, otherwise they take precedence.  With the
# visibleitems option engaged, geometry constraints are maintained
# only on the listbox.  The size of the other components such as 
# labels, margins, and scrollbars, are additive and independent, 
# effecting the overall size of the scrolled list box.  In contrast,
# should the width and height options have non zero values, they
# are applied to the scrolled list box as a whole.  The listbox 
# is compressed or expanded to maintain the geometry constraints.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::configVisibleitems {option value} {
    if {[regexp {^[0-9]+x[0-9]+$} $value]} {
        if {($itcl_options(-width) == 0) && ($itcl_options(-height) == 0)} {
            set chars [lindex [split $value x] 0]
            set lines [lindex [split $value x] 1]
            set shell [lindex [grid info $listbox] 1]

            #
            # Due to a bug in the tk4.2 grid, we have to check the 
            # propagation before setting it.  Setting it to the same
            # value it already is will cause it to toggle.
            #
            if {! [grid propagate $shell]} {
                grid propagate $shell yes
            }
        
            $listbox configure -width $chars -height $lines
        }
    
    } else {
        error "bad visibleitems option \"$value\": should be widthxheight"
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -state
#
# Specifies the state of the scrolledlistbox which may be either
# disabled or normal.  In a disabled state, the scrolledlistbox 
# does not accept user selection.  The default is normal.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::configState {option value} {
    set tags [bindtags $listbox]
    #
    # If the state is normal, then we need to remove the disabled 
    # bindings if they exist.  If the state is disabled, then we need
    # to install the disabled bindings if they haven't been already.
    #
    switch -- $value {
    normal {
        $listbox configure -foreground $itcl_options(-foreground)
        $listbox configure -selectforeground $itcl_options(-selectforeground)
        if {[set index [lsearch $tags SLBDisabled]] != -1} {
            bindtags $listbox [lreplace $tags $index $index]
        }
      }
    disabled {
        $listbox configure -foreground $itcl_options(-disabledforeground)
        $listbox configure -selectforeground $itcl_options(-disabledforeground)
        if {[set index [lsearch $tags SLBDisabled]] == -1} {
            bindtags $listbox [linsert $tags 1 SLBDisabled]
        }
      }
    default {
        error "bad state value \"$value\": must be normal or disabled"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: curselection 
#
# Returns a list containing the indices of all the elements in the 
# listbox that are currently selected.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::curselection {} {
    return [$listbox curselection]
}

# ------------------------------------------------------------------
# METHOD: activate index
#
# Sets the active element to the one indicated by index.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::activate {index} {
    return [$listbox activate [_fixIndex $index]]
}

# ------------------------------------------------------------------
# METHOD: bbox index
#
# Returns four element list describing the bounding box for the list
# item at index
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::bbox {index} {
    return [$listbox bbox [_fixIndex $index]]
}

# ------------------------------------------------------------------
# METHOD clear 
#
# Clear the listbox area of all items.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::clear {} {
    delete 0 end
}

# ------------------------------------------------------------------
# METHOD: see index
#
# Adjusts the view such that the element given by index is visible.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::see {index} {
    $listbox see [_fixIndex $index]
}

# ------------------------------------------------------------------
# METHOD: index index
#
# Returns the decimal string giving the integer index corresponding 
# to index.  The index value may be a integer number, active,
# anchor, end, @x,y, or a pattern.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::index {index} {
    if {[regexp {(^[0-9]+$)|(^active$)|(^anchor$)|(^end$)|(^@-?[0-9]+,-?[0-9]+$)} $index]} {
        return [$listbox index $index]
    } else {
        set indexValue [lsearch -glob [get 0 end] $index]
        if {$indexValue == -1} {
            error "bad Scrolledlistbox index \"$index\": must be active,\
                    anchor, end, @x,y, number, or a pattern"
        }
        return $indexValue
    }
}

# ------------------------------------------------------------------
# METHOD: _fixIndex index
#
# Similar to the regular "index" method, but it only converts
# the index to a numerical value if it is a string pattern.  If
# the index is in the proper form to be used with the listbox,
# it is left alone.  This fixes problems associated with converting
# an index such as "end" to a numerical value.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::_fixIndex {index} {
    if {[regexp {(^[0-9]+$)|(^active$)|(^anchor$)|(^end$)|(^@[0-9]+,[0-9]+$)} \
            $index]} {
        return $index
    } else {
        set indexValue [lsearch -glob [get 0 end] $index]
        if {$indexValue == -1} {
            error "bad Scrolledlistbox index \"$index\": must be active,\
                    anchor, end, @x,y, number, or a pattern"
        }
        return $indexValue
    }
}

# ------------------------------------------------------------------
# METHOD: delete first ?last?
#
# Delete one or more elements from list box based on the first and 
# last index values.  Indexes may be a number, active, anchor, end,
# @x,y, or a pattern.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::delete {first {last {}}} {
    set first [_fixIndex $first]
    if {$last ne ""} {
        set last [_fixIndex $last]
    } else {
        set last $first
    }
    uplevel 0 $listbox delete $first $last
}

# ------------------------------------------------------------------
# METHOD: get first ?last?
#
# Returns the elements of the listbox indicated by the indexes. 
# Indexes may be a number, active, anchor, end, @x,y, ora pattern.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::get {first {last {}}} {
    set first [_fixIndex $first]
    if {$last ne ""} {
        set last [_fixIndex $last]
    }
    if {$last eq ""} {
        return [$listbox get $first]
    } else {
        return [$listbox get $first $last]
    }
}

# ------------------------------------------------------------------
# METHOD: getcurselection 
#
# Returns the contents of the listbox element indicated by the current 
# selection indexes.  Short cut version of get and curselection 
# command combination.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::getcurselection {} {
    set rlist {}
    if {[selecteditemcount] > 0} {
        set cursels [$listbox curselection]
        switch $itcl_options(-selectmode) {
        single -
        browse {
            set rlist [$listbox get $cursels]
          }
        multiple -
        extended {
            foreach sel $cursels {
                lappend rlist [$listbox get $sel]
            }
          }
        }
    }
    return $rlist
}

# ------------------------------------------------------------------
# METHOD: insert index string ?string ...?
#
# Insert zero or more elements in the list just before the element 
# given by index.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::insert {index args} {
    set index [_fixIndex $index]
    uplevel 0 $listbox insert $index $args
}

# ------------------------------------------------------------------
# METHOD: nearest y
#
# Given a y-coordinate within the listbox, this command returns the 
# index of the visible listbox element nearest to that y-coordinate.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::nearest {y} {
    $listbox nearest $y
}

# ------------------------------------------------------------------
# METHOD: scan option args 
#
# Implements scanning on listboxes.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::scan {option args} {
    uplevel 0 $listbox scan $option $args
}

# ------------------------------------------------------------------
# METHOD: selection option first ?last?
#
# Adjusts the selection within the listbox.  The index value may be 
# a integer number, active, anchor, end, @x,y, or a pattern.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::selection {option first {last {}}} {
    set first [_fixIndex $first]
    if {$last != {}} {
        set last [_fixIndex $last]
        $listbox selection $option $first $last
    } else {
        $listbox selection $option $first 
    }
}

# ------------------------------------------------------------------
# METHOD: size 
#
# Returns a decimal string indicating the total number of elements 
# in the listbox.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::size {} {
    return [$listbox size]
}

# ------------------------------------------------------------------
# METHOD: selecteditemcount 
#
# Returns a decimal string indicating the total number of selected 
# elements in the listbox.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::selecteditemcount {} {
    return [llength [$listbox curselection]]
}

# ------------------------------------------------------------------
# METHOD: justify direction
#
# Justifies the list scrolled region in one of four directions: top,
# bottom, left, or right.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::justify {direction} {
    switch $direction {
    left { 
        $listbox xview moveto 0
      }
    right {
        $listbox xview moveto 1
      }
    top {
        $listbox yview moveto 0
      }
    bottom {
        $listbox yview moveto 1
      }
    default {
        error "bad justify argument \"$direction\": should\
            be left, right, top, or bottom"
      }
    }
}

# ------------------------------------------------------------------
# METHOD: sort mode
#
# Sort the current list. This can take any sort switch from
# the lsort command: ascii, integer, real, command, 
# increasing/ascending, decreasing/descending, etc.
#     
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::sort {{mode ascending}} {

    set vals [$listbox get 0 end]
    if {[llength $vals] == 0} {
        return
    }

    switch $mode {
    ascending   {
        set mode increasing
      }
    descending  {
        set mode decreasing
      }
    }

    $listbox delete 0 end
    if {[catch {uplevel 0 $listbox insert end \
            [lsort -${mode} $vals]} errorstring]} {
        error "bad sort argument \"$mode\": must be a valid argument to the\
                Tcl lsort command"
    }

    return
}

# ------------------------------------------------------------------
# METHOD: xview args
#
# Change or query the vertical position of the text in the list box.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::xview {args} {
    return [uplevel 0 $listbox xview $args]
}

# ------------------------------------------------------------------
# METHOD: yview args
#
# Change or query the horizontal position of the text in the list box.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::yview {args} {
    return [uplevel 0 $listbox yview $args]
}

# ------------------------------------------------------------------
# METHOD: itemconfigure args
#
# This is a wrapper method around the new tk8.3 itemconfigure command
# for the listbox.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::itemconfigure {args} {
    return [uplevel 0 $listbox itemconfigure $args]
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _makeSelection 
#
# Evaluate the selection command.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::_makeSelection {} {
    uplevel #0 $itcl_options(-selectioncommand)
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _dblclick 
#
# Evaluate the double click command option if not empty.
# ------------------------------------------------------------------
::itcl::body Scrolledlistbox::_dblclick {} {
    uplevel #0 $itcl_options(-dblclickcommand)
}   

} ; # end ::itcl::widgets
