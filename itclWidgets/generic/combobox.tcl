#
# Combobox
# ----------------------------------------------------------------------
# Implements a Combobox widget using itcl-ng.
# A Combobox has 2 basic styles: simple and dropdown.
# Dropdowns display an entry field with an arrow button to the 
# right of it. When the arrow button is pressed a selectable list of
# items is popped up. A simple Combobox displays an entry field and a listbox 
# just beneath it which is always displayed. In both types, if the user 
# selects an item in the listbox, the contents of the entry field are 
# replaced with the text from the selected item. If the Combobox is 
# editable, the user can type in the entry field and when <Return> is
# pressed the item will be inserted into the list.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version

# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package
# written by:
# ----------------------------------------------------------------------
# original author: John S. Sigler
# ----------------------------------------------------------------------
# original maintainer Chad Smith EMAIL: csmith@adc.com, itclguy@yahoo.com
#		Copyright (c) 1995	John S. Sigler
#		Copyright (c) 1997	Mitch Gorman
# ----------------------------------------------------------------------
#   @(#) $Id: combobox.tcl,v 1.1.2.2 2008/12/28 12:11:46 wiede Exp $
# ======================================================================

package require itcl

#
# Default resources.
#
option add *Combobox.borderWidth 2 widgetDefault
option add *Combobox.labelPos wn widgetDefault
option add *Combobox.listHeight 150 widgetDefault
option add *Combobox.hscrollMode dynamic widgetDefault
option add *Combobox.vscrollMode dynamic widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercase access method for the Combobox class.
# 
proc combobox {pathName args} {
    uplevel ::itcl::widgets::Combobox $pathName $args
}

::itcl::extendedclass Combobox {
    inherit ::itcl::widgets::Entryfield

    component list
    component arrowBtn
    component popup

    constructor {args} {}
    destructor {}

    option -arrowrelief arrowRelief Relief -default raised
    option -completion completion Completion -default true -configuremethod configCompletion
    option -dropdown dropdown Dropdown -default true -configuremethod configDropdown
    option -editable editable Editable -default true -configuremethod configEditable
    option -grab grab Grab -default local -configuremethod configGrab
    option -listheight listHeight Height -default 150
    option -margin margin Margin -default 1 -configuremethod configMargin
    option -popupcursor popupCursor Cursor -default arrow
    option -selectioncommand selectionCommand SelectionCommand -default {}
    option -state state State -default normal -configuremethod configState
    option -unique unique Unique -default true -configuremethod configUnique

    delegate option -background to arrowBtn
    delegate option -borderwidth to arrowBtn
    delegate option -cursor to arrowBtn
    delegate option -state to arrowBtn
    delegate option -highlightcolor to arrowBtn
    delegate option -highlightthickness to arrowBtn
    delegate option -arrowrelief arrowRelief Relief to arrowBtn as -relief
    delegate option -background background Background to arrowBtn as -highlightbackground

    delegate method justify to list
    delegate method see to list
    delegate method sort to list
    delegate method size to list
    delegate method sort to list
    delegate method xview to list
    delegate method yview to list

    private method _bs {}
    private method _lookup {key}
    private method _slbListbox {}
    private method _stateSelect {}

    protected method _addToList {}
    protected method _createComponents {}
    protected method _deleteList {first {last {}}}
    protected method _deleteText {first {last {}}}
    protected method _doLayout {{when later}}
    protected method _drawArrow {}
    protected method _dropdownBtnRelease {{window {}} {x 1} {y 1}}
    protected method _ignoreNextBtnRelease {ignore}
    protected method _next {}
    protected method _packComponents {{when later}}
    protected method _positionList {}
    protected method _postList {}
    protected method _previous {}
    protected method _resizeArrow {}
    protected method _selectCmd {}
    protected method _toggleList {}
    protected method _unpostList {}
    protected method _commonBindings {}
    protected method _dropdownBindings {}
    protected method _simpleBindings {}
    protected method _listShowing {{val ""}}
    protected method configCompletion {option value}
    protected method configDropdown {option value}
    protected method configEditable {option value}
    protected method configGrab {option value}
    protected method configMargin {option value}
    protected method configState {option value}
    protected method configUnique {option value}

    public method clear {{component all}}
    public method curselection {}
    public method delete {component first {last {}}}
    public method get {{index {}}}
    public method getcurselection {}
    public method insert {component index args}
    public method invoke {}
    public method justify {direction}
    public method see {index}
    public method selection {option first {last {}}}
    public method size {}
    public method sort {{mode ascending}}
    public method xview {args}
    public method yview {args}

    private variable _doit 0;
    private variable _inbs 0;
    private variable _inlookup 0;
    private variable _currItem {};	  ;# current selected item.
    private variable _ignoreRelease false ;# next button release ignored.
    private variable _isPosted false;	  ;# is the dropdown popped up.
    private variable _repacking {}	  ;# non-null => _packComponents pending.
    private variable _grab                ;# used to restore grabs
    private variable _next_prevFLAG 0     ;# Used in _lookup to fix SF Bug 501300
    private common _listShowing
    private common count 0
}	 

# ------------------------------------------------------------------
#						CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Combobox::constructor {args} {
    set _listShowing($this) 0
    set _grab(window) ""
    set _grab(status) ""

    # combobox is different as all components are created 
    # after determining what the dropdown style is...

    # configure args
    if {[llength $args]  > 0} {
        uplevel 0 configure $args
    }
    
    # create components that are dependent on options 
    # (Scrolledlistbox, arrow button) and pack them.
    if {$count == 0} {
	image create bitmap downarrow -data {
	    #define down_width 16
	    #define down_height 16
	    static unsigned char down_bits[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0xf8, 0x3f, 
		0xf0, 0x1f, 0xe0, 0x0f, 0xc0, 0x07, 0x80, 0x03, 
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	    };
	}
	image create bitmap uparrow -data {
	    #define up_width 16
	    #define up_height 16
	    static unsigned char up_bits[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 
		0xc0, 0x01, 0xe0, 0x03, 0xf0, 0x07, 0xf8, 0x0f, 
		0xfc, 0x1f, 0xfe, 0x3f, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	    };
	}
    }
    incr count
    _doLayout
}

# ------------------------------------------------------------------
#						   DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Combobox::destructor {} {
    # catch any repacking that may be waiting for idle time
    if {$_repacking != ""} {
	after cancel $_repacking
    }
    incr count -1
    if {$count == 0} {
	image delete uparrow
	image delete downarrow
    }
}

# --------------------------------------------------------------------
# OPTION:  -completion
# Relief style used on the arrow button.
# --------------------------------------------------------------------
::itcl::body Combobox::configCompletion {option value} {
    switch -- $value {
    0 -
    no -
    false -
    off { 
      }
    1 -
    yes -
    true -
    on {
      }
    default {
        error "bad completion option \"$value\": should be boolean"
      }
    }
}

# --------------------------------------------------------------------
# OPTION:  -dropdown  
#
# Boolean which determines the Combobox style: dropdown or simple.
# Because the two style's lists reside in different toplevel widgets
# this is more complicated than it should be.
# --------------------------------------------------------------------
::itcl::body Combobox::configDropdown {option value} {
    switch -- $value {
    1 -
    yes -
    true -
    on {
        if {[winfo exists $itcl_interior.list]} {
	    set vals [$list get 0 end]
	    destroy $list
	    _doLayout
	    if [llength $vals] {
	        uplevel 0 insert list end $vals
	    }
        }
      }
    0 -
    no -
    false -
    off {
        if {[winfo exists $itcl_interior.popup.list]} {
	    set vals [$list get 0 end]
	    catch {destroy $arrowBtn}
	    destroy $popup  ;# this deletes the list too
	    _doLayout
	    if [llength $vals] {
	        eval insert list end $vals
	    }
        }
    }
    default {
        error "bad dropdown option \"$value\": should be boolean"
      }
    }
}

# --------------------------------------------------------------------
# OPTION: -editable	 
#
# Boolean which allows/disallows user input to the entry field area.
# --------------------------------------------------------------------
::itcl::body Combobox::configEditable {option value} {
    switch -- $value {
    1 -
    true -
    yes -
    on {
        switch -- $itcl_options(-state) {
	normal {
	    $entry configure -state normal
	  }
        }
      }
    0 -
    false -
    no -
    off {
        $entry configure -state readonly
      }
    default {
        error "bad editable option \"$value\": should be boolean"
      }
    }
}

# --------------------------------------------------------------------
# OPTION:  -grab
#
# grab-state of megawidget
# --------------------------------------------------------------------
::itcl::body Combobox::configGrab {option value} {
    switch -- $itk_option(-grab) {
    local {
      }
    global {
      }
    default {
        error "bad grab value \"$value\": must be global or local"
      }
   }
}

# --------------------------------------------------------------------
# OPTION:  -margin
#
# Spacer between the entry field and arrow button of dropdown style
# Comboboxes.
# --------------------------------------------------------------------
::itcl::body Combobox::configMargin {option value} {
    grid columnconfigure $itcl_interior 0 -minsize $value
}


# --------------------------------------------------------------------
# OPTION:  -state
#
# overall state of megawidget
# --------------------------------------------------------------------
::itcl::body Combobox::configState {option value} {
    switch -- $value {
    disabled {
        $entry configure -state disabled
      }
    normal {
        switch -- $itcl_options(-editable) {
	1 -
	true -
	yes -
	on {
	    $entry configure -state normal
	  }
	0 -
	false -
	no -
	off {
	    $entry configure -state readonly
	  }
        }
      }
    readonly {
        $entry configure -state readonly
      }
    default {
        error "bad state value \"$value\": must be normal  or disabled"
      }
    }
    if {[info exists arrowBtn]} {
	$arrowBtn configure -state $value
    }
}

# --------------------------------------------------------------------
# OPTION: -unique  
#
# Boolean which disallows/allows adding duplicate items to the listbox.
# --------------------------------------------------------------------
::itcl::body Combobox::configUnique {option value} {
    # boolean error check
    switch -- $value {
    1 -
    true -
    yes -
    on {
      }
    0 -
    false -
    no -
    off {
      }
    default {
        error "bad unique value \"$value\": should be boolean"
      }
    }
}

# =================================================================
#							 METHODS
# =================================================================

# ------------------------------------------------------
#  PUBLIC METHOD: clear ?component?
#
#  Remove all elements from the listbox, all contents
#  from the entry component, or both (if all).
#
# ------------------------------------------------------
::itcl::body Combobox::clear {{component all}} {
    switch -- $component {
    entry {
	    itcl::widgets::Entryfield::clear
      }
    list {
        delete list 0 end
      }
    all {
        delete list 0 end
        itcl::widgets::Entryfield::clear
      }
    default {
        error "bad Combobox component \"$component\":\
			   must be entry, list, or all."
      }
    }
    return
}

# ------------------------------------------------------
# PUBLIC METHOD: curselection
#
# Return the current selection index.
#
# ------------------------------------------------------
::itcl::body Combobox::curselection {} {
    return [$list curselection]
}

# ------------------------------------------------------
# PUBLIC METHOD: delete component first ?last?
#
# Delete an item or items from the listbox OR delete
# text from the entry field. First argument determines
# which component deletion occurs in - valid values are
# entry or list.
#
# ------------------------------------------------------
::itcl::body Combobox::delete {component first {last {}}} {
    switch -- $component {
    entry {
        if {$last == {}} {
            set last [expr {$first + 1}]
        }
        itcl::widgets::Entryfield::delete $first $last
      }
    list {
        _deleteList $first $last
      }
    default {
        error "bad Combobox component \"$component\":\
			   must be entry or list."
      }
    }
}

# ------------------------------------------------------
# PUBLIC METHOD: get ?index?
#
#
# Retrieve entry contents if no args OR use args as list 
# index and retrieve list item at index .
#
# ------------------------------------------------------
::itcl::body Combobox::get {{index {}}} {
    # no args means to get the current text in the entry field area
    if {$index == {}} {
	itcl::widgets::Entryfield::get
    } else {
	uplevel 0 $list get $index
    }
}

# ------------------------------------------------------
# PUBLIC METHOD: getcurselection
#
# Return currently selected item in the listbox. Shortcut
# version of get curselection command combination.
#
# ------------------------------------------------------
::itcl::body Combobox::getcurselection {} {
    return [$list getcurselection]
}

# ------------------------------------------------------------------
# PUBLIC METHOD: invoke
#
# Pops up or down a dropdown combobox.
# 
# ------------------------------------------------------------------
::itcl::body Combobox::invoke {} {
    if {$itcl_options(-dropdown)} {
	return [_toggleList]
    }
    return 
}

# ------------------------------------------------------------
# PUBLIC METHOD: insert comonent index string ?string ...?
#
# Insert an item into the listbox OR text into the entry area.
# Valid component names are entry or list.
#
# ------------------------------------------------------------
::itcl::body Combobox::insert {component index args} {
    set nargs [llength $args]

    if {$nargs == 0} {
	error "no value given for parameter \"string\" in function\
			   \"Combobox::insert\""
    } 

    switch -- $component {
    entry {
        if { $nargs > 1} {
	    error "called function \"Combobox::insert entry\"\
				   with too many arguments"
        } else {
	    if {$itcl_options(-state) == "normal"} {
	        uplevel 0 itcl::widgets::Entryfield::insert $index $args
	        [itcl::code $this _lookup ""]
	    }
        }
      }
    list {
        if {$itcl_options(-state) == "normal"} {
	    eval $list insert $index $args
        }
      }
    default {
        error "bad Combobox component \"$component\": must\
			   be entry or list."
      }
    }
}

# ------------------------------------------------------------------
# PUBLIC METHOD: selection option first ?last?
#
# Adjusts the selection within the listbox and changes the contents
# of the entry component to be the value of the selected list item.
# ------------------------------------------------------------------
::itcl::body Combobox::selection {option first {last {}}} {
    # thin wrap
    if {$option == "set"} {
	$list selection clear 0 end
	$list selection set $first
	set rtn ""
    } else {
	set rtn [uplevel 0 $ist selection $option $first $last]
    }
    set _currItem $first

    # combobox additions
    set theText [getcurselection]
    if {$theText != [$entry get]} {
	clear entry
	if {$theText != ""} {
	    insert entry 0 $theText
	}
    }
    return $rtn
}

# ------------------------------------------------------
# PROTECTED METHOD: _addToList
#
# Add the current item in the entry to the listbox.
#
# ------------------------------------------------------
::itcl::body Combobox::_addToList {} {
    set input [get]
    if {$input != ""} {
	if {$itcl_options(-unique)} {
	    # if item is already in list, select it and exit
	    set item [lsearch -exact [$list get 0 end] $input]
	    if {$item != -1} {
		selection clear 0 end
		if {$item != {}} {
		    selection set $item $item
		    set _currItem $item
		}
		return
	    }
	}
	# add the item to end of list
	selection clear 0 end
	insert list end $input
	selection set end end
    }
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _createComponents
#
# Create deferred combobox components and add bindings.
#
# ------------------------------------------------------
::itcl::body Combobox::_createComponents {} {
    if {$itcl_options(-dropdown)} {
	# --- build a dropdown combobox ---

	# make the arrow childsite be on the right hand side

  	#-------------------------------------------------------------
	# BUG FIX: csmith (Chad Smith: csmith@adc.com), 3/4/99
  	#-------------------------------------------------------------
	# The following commented line of code overwrites the -command
	# option when passed into the constructor.  The order of calls
	# in the constructor is:
	# 	1) uplevel 0 configure $args (initializes -command)
	#       2) _doLayout
	#       3) _createComponents (overwrites -command)
	# The solution is to only set the -command option if it hasn't
	# already been set.  The following 4 lines of code do this.
  	#-------------------------------------------------------------
	# ** configure -childsitepos e -command [code $this _addToList]
  	#-------------------------------------------------------------
	configure -childsitepos e
	if ![llength [cget -command]] {
	    configure -command [itcl::code $this _addToList]
	}
	
	# arrow button to popup the list
	setupcomponent arrowBtn using \
	    button $itcl_interior.arrowBtn -borderwidth 2 \
		-width 15 -height 15 -image downarrow \
		-command [itcl::code $this _toggleList] \
		-state $itcl_options(-state)
	
	# popup list container
	setupcomponent popup using toplevel $itk_interior.popup
	keepcomponentoption popup -background -cursor
	wm withdraw $popup
	
	# the listbox
	setupcomponent list using
	    ::itcl::widgets::Scrolledlistbox $itcl_interior.popup.list \
	        -exportselection no \
		-vscrollmode dynamic -hscrollmode dynamic -selectmode browse
	keepcomponentoption -background -borderwidth -cursor -foreground \
		-highlightcolor -highlightthickness \
		-hscrollmode -selectbackground \
		-selectborderwidth -selectforeground -textbackground \
		-textfont -vscrollmode
#	    rename -height -listheight listHeight Height
#	    rename -cursor -popupcursor popupCursor Cursor

	# mode specific bindings
	_dropdownBindings

	# Ugly hack to avoid tk buglet revealed in _dropdownBtnRelease where 
	# relief is used but not set in scrollbar.tcl. 
	global tkPriv
	set tkPriv(relief) raise

    } else {
	# --- build a simple combobox ---
	configure -childsitepos s
	setcomponent list using
	    ::itcl::widgets::Scrolledlistbox $itcl_interior.list \
	        -exportselection no \
		-vscrollmode dynamic -hscrollmode dynamic 
	keepcomponentoption -background -borderwidth -cursor -foreground \
		-highlightcolor -highlightthickness \
		-hscrollmode -selectbackground \
		-selectborderwidth -selectforeground -textbackground \
		-textfont -visibleitems -vscrollmode 
#	    rename -height -listheight listHeight Height
	# add mode specific bindings
	_simpleBindings
    }

    # popup cursor applies only to the list within the combobox
    configure -popupcursor $itcl_options(-popupcursor)

    # add mode independent bindings
    _commonBindings
}

# ------------------------------------------------------
# PROTECTED METHOD: _deleteList first ?last?
#
# Delete an item or items from the listbox. Called via 
# "delete list args".
#
# ------------------------------------------------------
::itcl::body Combobox::_deleteList {first {last {}}} {
    if {$last == {}} {
	set last $first
    }
    $list delete $first $last

    # remove the item if it is no longer in the list
    set text [$this get]
    if {$text != ""} {
	set index [lsearch -exact [$list get 0 end] $text ]
	if {$index == -1} {
	    clear entry
	}
    }
    return
}

# ------------------------------------------------------
# PROTECTED METHOD: _deleteText first ?last?
#
# Renamed Entryfield delete method. Called via "delete entry args".
#
# ------------------------------------------------------
::itcl::body Combobox::_deleteText {first {last {}}} {
    $entry configure -state normal 
    set rtrn [delete $first $last]
    switch -- $itcl_options(-editable) {
    0 -
    false -
    no -
    off {
	$entry configure -state readonly
      }
    }
    return $rtrn
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _doLayout ?when?
#
# Call methods to create and pack the Combobox components.
#
# ------------------------------------------------------
::itcl::body Combobox::_doLayout {{when later}} {
    _createComponents
    _packComponents $when
}


# ------------------------------------------------------
# PROTECTED METHOD:	  _drawArrow 
#
# Draw the arrow button. Determines packing according to
# -labelpos.
#
# ------------------------------------------------------
::itcl::body Combobox::_drawArrow {} {
    set flip false
    set relief ""
    set fg [cget -foreground]
    if {$_isPosted} {
	set flip true
	set relief "-relief sunken"
    } else {
	set relief "-relief $itk_option(-arrowrelief)"
    }

    if {$flip} {
	#	 
	#	draw up arrow
	#
	uplevel 0 $arrowBtn configure -image uparrow $relief
    } else {
	#	 
	#	draw down arrow
	#
	uplevel 0 $arrowBtn configure -image downarrow $relief
    }
}

# ------------------------------------------------------
# PROTECTED METHOD: _dropdownBtnRelease window x y
#
# Event handler for button releases while a dropdown list
# is posted.
#
# ------------------------------------------------------
::itcl::body Combobox::_dropdownBtnRelease {{window {}} {x 1} {y 1}} {

    # if it's a scrollbar then ignore the release
    if {($window == [$list component vertsb]) ||
	($window == [$list component horizsb])} {
	return
    }

    # 1st release allows list to stay up unless we are in listbox
    if {$_ignoreRelease} {
	_ignoreNextBtnRelease false
	return
    }
    
    # should I use just the listbox or also include the scrollbars
    if { ($x >= 0) && ($x < [winfo width [_slbListbox]])
	 && ($y >= 0) && ($y < [winfo height [_slbListbox]])} {
	_stateSelect
    }
    
    _unpostList

    # execute user command
    if {$itcl_options(-selectioncommand) != ""} {
	uplevel #0 $itcl_options(-selectioncommand)
    }
}

# ------------------------------------------------------
# PROTECTED METHOD: _ignoreNextBtnRelease ignore
#
# Set private variable _ignoreRelease. If this variable
# is true then the next button release will not remove
# a dropdown list.
#
# ------------------------------------------------------
::itcl::body Combobox::_ignoreNextBtnRelease {ignore} {
    set _ignoreRelease $ignore
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _next
#
# Select the next item in the list.
#
# ------------------------------------------------------
::itcl::body Combobox::_next {} {

    set _next_prevFLAG 1

    if {[size] <= 1} {
	return
    }
    set i [curselection]
    if {($i == {}) || ($i == ([size]-1)) } {
	set i 0
    } else {
	incr i
    }
    selection clear 0 end
    selection set $i $i
    see $i
    set _currItem $i
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _packComponents ?when?
#
# Pack the components of the combobox and add bindings.
#
# ------------------------------------------------------
::itcl::body Combobox::_packComponents {{when later}} {
    if {$when == "later"} {
	if {$_repacking == ""} {
	    set _repacking [after idle [itcl::code $this _packComponents now]]
	    return
	}
    } elseif {$when != "now"} {
	error "bad option \"$when\": should be now or later"
    }

    if {$-dropdown} {
	grid configure $list -row 1 -column 0 -sticky news
	_resizeArrow
        grid config $arrowBtn -row 0 -column 1 -sticky nsew
    } else {
	# size and pack list hack
	grid configure $entry -row 0 -column 0 -sticky ew
	grid configure $efchildsite -row 1 -column 0 -sticky nsew
	grid configure $list -row 0 -column 0 -sticky nsew

	grid rowconfigure $efchildsite 1 -weight 1
	grid columnconfigure $efchildsite 0 -weight 1
    }
    set _repacking ""
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _positionList
#
# Determine the position (geometry) for the popped up list
# and map it to the screen.
#
# ------------------------------------------------------
::itcl::body Combobox::_positionList {} {

    set x [winfo rootx $entry ]
    set y [expr {[winfo rooty $entry ] + \
	       [winfo height $entry ]}]
    set w [winfo width $entry ]
    set h [winfo height [_slbListbox] ]
    set sh [winfo screenheight .]

    if {(($y+$h) > $sh) && ($y > ($sh/2))} {
	set y [expr {[winfo rooty $entry ] - $h}]
    }
    
    $list configure -width $w
    wm overrideredirect $popup 0
    wm geometry $popup +$x+$y
    wm overrideredirect $popup 1
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _postList
#
# Pop up the list in a dropdown style Combobox.
#
# ------------------------------------------------------
::itcl::body Combobox::_postList {} {
    if {[$list size] == ""} {
	return
    }

    set _isPosted true
    _positionList

    # map window and do a grab
    wm deiconify $popup
    _listShowing -wait

    # Added by csmith, 12/19/00.  Thanks to Erik Leunissen for
    # finding this problem.  We need to restore any previous
    # grabs after the dropdown listbox is withdrawn.  To do this,
    # save the currently grabbed window.  It is then restored in
    # the _unpostList method.
    set _grab(window) [::grab current]
    if {$_grab(window) != ""} {
      set _grab(status) [::grab status $_grab(window)]
    }

    # Now grab the dropdown listbox.
    if {$itcl_options(-grab) == "global"} {
	::grab -global $popup 
    } else {
	::grab $popup 
    }
    raise $popup
    focus $popup
    _drawArrow

    # Added by csmith, 10/26/00.  This binding keeps the listbox
    # from staying mapped if the window in which the combobox
    # is packed is iconified.
    bind $entry <Unmap> [itcl::code $this _unpostList]
}

# ------------------------------------------------------
# PROTECTED METHOD:	   _previous
#
# Select the previous item in the list. Wraps at front
# and end of list. 
#
# ------------------------------------------------------
::itcl::body Combobox::_previous {} {

    set _next_prevFLAG 1

    if {[size] <= 1} {
	return
    }
    set i [curselection]
    if {$i == "" || $i == 0} {
	set i [expr {[size] - 1}]
    } else {
	incr i -1
    }
    selection clear 0 end
    selection set $i $i
    see $i
    set _currItem $i
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _resizeArrow
#
# Recalculate the arrow button size and then redraw it.
#
# ------------------------------------------------------
::itcl::body Combobox::_resizeArrow {} {
    set bw [expr {[$arrowBtn cget -borderwidth]+ \
		[$arrowBtn cget -highlightthickness]}]
    set newHeight [expr {[winfo reqheight $entry]-(2*$bw) - 2}]
    $arrowBtn configure -width $newHeight -height $newHeight
    _drawArrow
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _selectCmd
#
# Called when list item is selected to insert new text 
# in entry, and call user -command callback if defined.
#
# ------------------------------------------------------
::itcl::body Combobox::_selectCmd {} {
    $entry configure -state normal
    
    set _currItem [$list curselection]
    set item [$list getcurselection]
    clear entry
    $entry insert 0 $item
    switch -- $itcl_options(-editable) {
    0 -
    false -
    no -
    off {
	    $entry configure -state readonly
      }
    }
}

# ------------------------------------------------------
# PROTECTED METHOD:	 _toggleList
#
# Post or unpost the dropdown listbox (toggle).
#
# ------------------------------------------------------
::itcl::body Combobox::_toggleList {} {
    if {[winfo ismapped $popup] } {
	_unpostList
    } else {
	_postList
    }
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _unpostList
#
# Unmap the listbox (pop it down).
#
# ------------------------------------------------------
::itcl::body Combobox::_unpostList {} {
    # Determine if event occured in the scrolledlistbox and, if it did, 
    # don't unpost it. (A selection in the list unposts it correctly and 
    # in the scrollbar we don't want to unpost it.)
    set x [winfo x $list]
    set y [winfo y $list]
    set w [winfo width $list]
    set h [winfo height $list]

    wm withdraw $popup
    ::grab release $popup	

    # Added by csmith, 12/19/00.  Thanks to Erik Leunissen for finding
    # this problem.  We need to restore any previous grabs when the
    # dropdown listbox is unmapped.
    if {$_grab(window) != ""} {
      if {$_grab(status) == "global"} {
        ::grab -global $_grab(window)
      } else {
	::grab $_grab(window)
      }
      set _grab(window) ""
      set _grab(status) ""
    }

    # Added by csmith, 10/26/00.  This binding resets the binding
    # created in _postList - see that method for further details.
    bind $entry <Unmap> {}
    
    set _isPosted false
    
    $list selection clear 0 end
    if {$_currItem != {}} {
	$list selection set $_currItem $_currItem
	$lis) activate $_currItem
    }

    switch -- $itcl_options(-editable) {
    1 -
    true -
    yes -
    on {
        $entry configure -state normal
      }
    0 -
    false -
    no -
    off {
        $entry configure -state readonly
      }
    }

    _drawArrow
    update
}

# ------------------------------------------------------
# PROTECTED METHOD:	  _commonBindings
#
# Bindings that are used by both simple and dropdown
# style Comboboxes.
#
# ------------------------------------------------------
::itcl::body Combobox::_commonBindings {} {
    bind $entry <KeyPress-BackSpace> [itcl::code $this _bs]
    bind $entry <KeyRelease> [itcl::code $this _lookup %K]
    bind $entry <Down>       [itcl::code $this _next]
    bind $entry <Up>         [itcl::code $this _previous]
    bind $entry <Control-n>  [itcl::code $this _next]
    bind $entry <Control-p>  [itcl::code $this _previous]
    bind [_slbListbox]         <Control-n>  [itcl::code $this _next]
    bind [_slbListbox]         <Control-p>  [itcl::code $this _previous]
}


# ------------------------------------------------------
# PROTECTED METHOD: _dropdownBindings
#
# Bindings used only by the dropdown type Combobox.
#
# ------------------------------------------------------
::itcl::body Combobox::_dropdownBindings {} {
    bind $popup  <Escape> [itcl::code $this _unpostList]
    bind $popup  <space>  \
	"[itcl::code $this _stateSelect]; [itcl::code $this _unpostList]"
    bind $popup  <Return> \
	"[itcl::code $this _stateSelect]; [itcl::code $this _unpostList]"
    bind $popup  <ButtonRelease-1> \
        [itcl::code $this _dropdownBtnRelease %W %x %y]

    bind $list  <Map> \
	[itcl::code $this _listShowing 1]
    bind $list  <Unmap> \
        [itcl::code $this _listShowing 0]

    # once in the listbox, we drop on the next release (unless in scrollbar)
    bind [_slbListbox]   <Enter>   \
	[itcl::code $this _ignoreNextBtnRelease false]

    bind $arrowBtn <3>          [itcl::code $this _next]
    bind $arrowBtn <Shift-3>    [itcl::code $this _previous]
    bind $arrowBtn <Down>       [itcl::code $this _next]
    bind $arrowBtn <Up>         [itcl::code $this _previous]
    bind $arrowBtn <Control-n>  [itcl::code $this _next]
    bind $arrowBtn <Control-p>  [itcl::code $this _previous]
    bind $arrowBtn <Shift-Down> [itcl::code $this _toggleList]
    bind $arrowBtn <Shift-Up>   [itcl::code $this _toggleList]
    bind $arrowBtn <Return>     [itcl::code $this _toggleList]
    bind $arrowBtn <space>      [itcl::code $this _toggleList]

    bind $entry    <Configure>  [itcl::code $this _resizeArrow]
    bind $entry    <Shift-Down> [itcl::code $this _toggleList]
    bind $entry    <Shift-Up>   [itcl::code $this _toggleList]
}

# ------------------------------------------------------
# PROTECTED METHOD: _simpleBindings
#
# Bindings used only by the simple type Comboboxes.
#
# ------------------------------------------------------
::itcl::body Combobox::_simpleBindings {} {
    bind [_slbListbox]         <ButtonRelease-1> [itcl::code $this _stateSelect]
    bind [_slbListbox]         <space>     [itcl::code $this _stateSelect]
    bind [_slbListbox]         <Return>    [itcl::code $this _stateSelect]
    bind $entry <Escape>     ""
    bind $entry <Shift-Down> ""
    bind $entry <Shift-Up>   ""
    bind $entry <Configure>  ""
}

# ------------------------------------------------------
# PROTECTED METHOD: _listShowing ?val?
#
# Used instead of "tkwait visibility" to make sure that
# the dropdown list is visible.	 Whenever the list gets
# mapped or unmapped, this method is called to keep
# track of it.	When it is called with the value "-wait",
# it waits for the list to be mapped.
# ------------------------------------------------------
::itcl::body Combobox::_listShowing {{val ""}} {
    if {$val == ""} {
	return $_listShowing($this)
    } elseif {$val == "-wait"} {
	while {!$_listShowing($this)} {
	    tkwait variable [itcl::scope _listShowing($this)]
	}
	return
    }
    set _listShowing($this) $val
}

# ------------------------------------------------------
# PRIVATE METHOD:	 _slbListbox
#
# Access the tk listbox window out of the scrolledlistbox.
#
# ------------------------------------------------------
::itcl::body Combobox::_slbListbox {} {
    return [$list component listbox]
}

# ------------------------------------------------------
# PRIVATE METHOD:	 _stateSelect
#
# only allows a B1 release in the listbox to have an effect if -state is
#	normal.
#
# ------------------------------------------------------
::itcl::body Combobox::_stateSelect {} {
    switch --  $itcl_options(-state) {
	normal {
	    [itcl::code $this _selectCmd]
	}
    }
}

# ------------------------------------------------------
# PRIVATE METHOD:	 _bs
#
# A part of the auto-completion code, this function sets a flag when the
#	Backspace key is hit and there is a selection in the entry field.
# Note that it's probably buggy to assume that a selection being present
#	means that that selection came from auto-completion.
#
# ------------------------------------------------------
::itcl::body Combobox::_bs {} {
    #
    #		exit if completion is turned off
    #
    switch -- $itcl_options(-completion) {
    0 -
    no -
    false -
    off {
        return
      }
    }
    #
    #	critical section flag.  it ain't perfect, but for most usage it'll
    #	keep us from being in this code "twice" at the same time
    #	(auto-repeated keystrokes are a pain!)
    #
    if {$_inbs} {
	return
    } else {
	set _inbs 1
    }

    #
    #	set the _doit flag if there is a selection set in the entry field
    #
    set _doit 0
    if [$entry selection present] {
	set _doit 1
    }

    #
    #	clear the semaphore and return
    #
    set _inbs 0
}

# ------------------------------------------------------
# PRIVATE METHOD:	 _lookup
#
# handles auto-completion of text typed (or insert'd) into the entry field.
#
# ------------------------------------------------------
::itcl::body Combobox::_lookup {key} {

    #
    # Don't process auto-completion stuff if navigation key was released
    # Fixes SF bug 501300
    #
    if {$_next_prevFLAG} {
        set _next_prevFLAG 0
        return
    }

    #
    #	exit if completion is turned off
    #
    switch -- $itcl_options(-completion) {
    0 -
    no -
    false -
    off {
        return
      }
    }

    #
    #	critical section flag.  it ain't perfect, but for most usage it'll
    #	keep us from being in this code "twice" at the same time
    #	(auto-repeated keystrokes are a pain!)
    #
    if {$_inlookup} {
	return
    } else {
	set _inlookup 1
    }

    #
    #	if state of megawidget is disabled, or the entry is not editable,
    #	clear the semaphore and exit
    #
    if {$itcl_options(-state) == "disabled" \
	    || [lsearch {on 1 true yes} $itcl_options(-editable)] == -1} {
	set _inlookup 0
	return
    }

    #
    #	okay, *now* we can get to work
    #	the _bs function is called on keyPRESS of BackSpace, and will set
    #	the _doit flag if there's a selection set in the entryfield.  If
    #	there is, we're assuming that it's generated by completion itself
    #	(this is probably a Bad Assumption), so we'll want to whack the
    #	selected text, as well as the character immediately preceding the
    #	insertion cursor.
    #
    if {$key == "BackSpace"} {
	if {$_doit} {
	    set first [expr {[$entry index insert] -1}]
	    $entry delete $first end
	    $entry icursor $first
	}
    }

    #
    #	get the text left in the entry field, and its length.  if
    #	zero-length, clear the selection in the listbox, clear the
    #	semaphore, and boogie.
    #
    set text [get]
    set len [string length $text]
    if {$len == 0} {
	$list selection clear 0 end
	set _inlookup 0
	return
    }

    # No need to do lookups for Shift keys or Arrows.  The up/down
    # arrow keys should walk up/down the listbox entries.
    switch $key {
    Shift_L -
    Shift_R -
    Up -
    Down -
    Left -
    Right {
        set _inlookup 0
        return
      }
    default {
      }
    }

    # Added by csmith 12/11/01 to resolve SF ticket #474817.  It's an unusual
    # circumstance, but we need to make sure the character passed into this
    # method matches the last character in the entry's text string.  It's
    # possible to type fast enough that the _lookup method gets invoked
    # *after* multiple characters have been typed and *before* the first
    # character has been processed.  For example, you can type "bl" very
    # quickly, and by the time the interpreter processes "b", the "l" has
    # already been placed in the entry field.  This causes problems as noted
    # in the SF ticket.
    #
    # Thus, if the character currently being processed does not match the
    # last character in the entry field, reset the _inlookup flag and return.
    # Also, note that we're only concerned with single characters here, not
    # keys such as backspace, delete, etc.
    if {$key != [string range $text end end] && [string match ? $key]} {
      set _inlookup 0
      return
    }

    #
    #	okay, so we have to do a lookup.  find the first match in the
    #	listbox to the text we've got in the entry field (glob).
    #	if one exists, clear the current listbox selection, and set it to
    #	the one we just found, making that one visible in the listbox.
    #	then, pick off the text from the listbox entry that hadn't yet been
    #	entered into the entry field.  we need to tack that text onto the
    #	end of the entry field, select it, and then set the insertion cursor
    #	back to just before the point where we just added that text.
    #	if one didn't exist, then just clear the listbox selection
    #
    set item [lsearch [$list get 0 end] "$text*" ]
    if {$item != -1} {
	$list selection clear 0 end
	$list selection set $item $item
	see $item
	set remainder [string range [$list get $item] $len end]
	$entry insert end $remainder
	$entry selection range $len end
	$entry icursor $len
    } else {
	$list selection clear 0 end
    }
    #
    #	clear the semaphore and return
    #
    set _inlookup 0
    return
}

} ; # end ::itcl::widgets

 	  	 
