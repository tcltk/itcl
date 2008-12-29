#
# Scrolledtext
# ----------------------------------------------------------------------
# Implements a scrolled text widget with additional options to manage
# the vertical scrollbar.  This includes options to control the method
# in which the scrollbar is displayed, i.e. statically or  dynamically.
# Options also exist for adding a label to the scrolled text area and
# controlling its position.  Import/export of methods are provided for 
# file I/O.
#
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Scrolledtext
# written by:
#
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: scrolledtext.tcl,v 1.1.2.1 2008/12/29 12:46:50 wiede Exp $
# ======================================================================

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Scrolledtext class.
# 
proc ::itcl::widgets::scrolledtext {pathName args} {
    uplevel ::itcl::widgets::Scrolledtext $pathName $args
}

# ------------------------------------------------------------------
#                           SCROLLEDTEXT
# ------------------------------------------------------------------
::itcl::extendedclass ::itcl::widgets::Scrolledtext {
    inherit ::itcl::widgets::Scrolledwidget

    component clipper
    component text

    option [list -width width Width] -default 0 -configuremethod configWidth
    option [list -height height Height] -default 0 -configuremethod configHeight
    option [list -visibleitems visibleItems VisibleItems] -default 80x24 -configuremethod configVisibleitems

    delegate option [list -background background Background] to clipper as -highlightbackground
     delegate option [list -textfont textFont Font] to text as -font 
     delegate option [list -textbackground textBackground Background] to text as -background 

    constructor {args} {}
    destructor {}

    protected method configWidth {option value}
    protected method configHeight {option value}
    protected method configVisibleitems {option value}

    public method bbox {index} 
    public method childsite {} 
    public method clear {} 
    public method import {filename {index end}} 
    public method export {filename} 
    public method compare {index1 op index2} 
    public method debug {args} 
    public method delete {first {last {}}} 
    public method dlineinfo {index} 
    public method get {index1 {index2 {}}} 
    public method image {option args}
    public method index {index} 
    public method insert {args} 
    public method mark {option args} 
    public method scan {option args} 
    public method search {args} 
    public method see {index} 
    public method tag {option args} 
    public method window {option args} 
    public method xview {args} 
    public method yview {args} 
}

#
# Use option database to override default resources of base classes.
#
option add *Scrolledtext.labelPos n widgetDefault

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Scrolledtext::constructor {args} {
    #
    # Our -width and -height options are slightly different than
    # those implemented by our base class, so we're going to
    # remove them and redefine our own.
    #
#    itcl_options remove ::itcl::widgets::Scrolledwidget::width
#    itcl_options remove ::itcl::widgets::Scrolledwidget::height

    #
    # Create a clipping frame which will provide the border for
    # relief display.
    #
    setupcomponent clipper using frame $itcl_interior.clipper

    keepcomponentoption clipper -activebackground -activerelief \
        -background -borderwidth -cursor \
	-elementborderwidth -foreground -highlightcolor -highlightthickness \
	-jump -labelfont -selectbackground \
	-selectforeground -textbackground -textfont -troughcolor 

    keepcomponentoption clipper -borderwidth -relief -highlightthickness \
        -highlightcolor
    grid $clipper -row 0 -column 0 -sticky nsew
    grid rowconfigure $_interior 0 -weight 1
    grid columnconfigure $_interior 0 -weight 1

    # 
    # Create the text area.
    #
    setupcomponent text using text $clipper.text \
		-width 1 -height 1 \
	        -xscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.horizsb] \
		-yscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.vertsb] \
	        -borderwidth 0 -highlightthickness 0
    keepcomponentoption text -activebackground -activerelief \
        -background -borderwidth -cursor \
	-elementborderwidth -foreground -highlightcolor \
	-insertbackground -insertborderwidth -insertofftime -insertontime \
	-insertwidth -jump -labelfont -selectbackground -selectborderwidth \
	-selectforeground -textbackground -textfont -troughcolor 

#	ignore -highlightthickness -highlightcolor -borderwidth

    keepcomponentoption text -exportselection -padx -pady -setgrid \
	     -spacing1 -spacing2 -spacing3 -state -tabs -wrap

    grid $text -row 0 -column 0 -sticky nsew
    grid rowconfigure $clipper 0 -weight 1
    grid columnconfigure $clipper 0 -weight 1
    
    # 
    # Configure the command on the vertical scroll bar in the base class.
    #
    $vertsb configure \
	-command [itcl::code $text yview]

    #
    # Configure the command on the horizontal scroll bar in the base class.
    #
    $horizsb configure \
		-command [itcl::code $text xview]
    
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
::itcl::body Scrolledtext::destructor {} {
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -width
#
# Specifies the width of the scrolled text as an entire unit.
# The value may be specified in any of the forms acceptable to 
# Tk_GetPixels.  Any additional space needed to display the other
# components such as labels, margins, and scrollbars force the text
# to be compressed.  A value of zero along with the same value for 
# the height causes the value given for the visibleitems option 
# to be applied which administers geometry constraints in a different
# manner.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::configWidth {option value} {
    if {$value != 0} {
	set shell [lindex [grid info $clipper] 1]

	#
	# Due to a bug in the tk4.2 grid, we have to check the 
	# propagation before setting it.  Setting it to the same
	# value it already is will cause it to toggle.
	#
	if {[grid propagate $shell]} {
	    grid propagate $shell no
	}
	
	$text configure -width 1
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
# Specifies the height of the scrolled text as an entire unit.
# The value may be specified in any of the forms acceptable to 
# Tk_GetPixels.  Any additional space needed to display the other
# components such as labels, margins, and scrollbars force the text
# to be compressed.  A value of zero along with the same value for 
# the width causes the value given for the visibleitems option 
# to be applied which administers geometry constraints in a different
# manner.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::configHeight {option value} {
    if {$value != 0} {
	set shell [lindex [grid info $clipper] 1]

	#
	# Due to a bug in the tk4.2 grid, we have to check the 
	# propagation before setting it.  Setting it to the same
	# value it already is will cause it to toggle.
	#
	if {[grid propagate $shell]} {
	    grid propagate $shell no
	}
	
	$text configure -height 1
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
::itcl::body Scrolledtext::configVisibleitems {option value} {
    if {[regexp {^[0-9]+x[0-9]+$} $value]} {
	if {($itcl_options(-width) == 0) && \
		($itcl_options(-height) == 0)} {
	    set chars [lindex [split $value x] 0]
	    set lines [lindex [split $value x] 1]
	    
	    set shell [lindex [grid info $clipper] 1]

	    #
	    # Due to a bug in the tk4.2 grid, we have to check the 
	    # propagation before setting it.  Setting it to the same
	    # value it already is will cause it to toggle.
	    #
	    if {! [grid propagate $shell]} {
		grid propagate $shell yes
	    }
	    
	    $text configure -width $chars -height $lines
	}
	
    } else {
	error "bad visibleitems option \"$value\": should be widthxheight"
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
::itcl::body Scrolledtext::childsite {} {
    return $text
}

# ------------------------------------------------------------------
# METHOD: bbox index
#
# Returns four element list describing the bounding box for the list
# item at index
# ------------------------------------------------------------------
::itcl::body Scrolledtext::bbox {index} {
    return [$text bbox $index]
}

# ------------------------------------------------------------------
# METHOD clear 
#
# Clear the text area.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::clear {} {
    $text delete 1.0 end
}

# ------------------------------------------------------------------
# METHOD import filename
#
# Load text from an existing file (import filename)
# ------------------------------------------------------------------
::itcl::body Scrolledtext::import {filename {index end}} {
    set f [open $filename r]
    insert $index [read $f]
    close $f
}

# ------------------------------------------------------------------
# METHOD export filename
#
# write text to a file (export filename)
# ------------------------------------------------------------------
::itcl::body Scrolledtext::export {filename} {
    set f [open $filename w]
    
    set txt [$text get 1.0 end]
    puts $f $txt
    
    flush $f
    close $f
}

# ------------------------------------------------------------------
# METHOD compare index1 op index2
#
# Compare indices according to relational operator.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::compare {index1 op index2} {
    return [$text compare $index1 $op $index2]
}

# ------------------------------------------------------------------
# METHOD debug ?boolean?
#
# Activates consistency checks in B-tree code associated with text
# widgets.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::debug {args} {
    eval $text debug $args
}

# ------------------------------------------------------------------
# METHOD delete first ?last?
#
# Delete a range of characters from the text.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::delete {first {last {}}} {
    $text delete $first $last
}

# ------------------------------------------------------------------
# METHOD dlineinfo index
#
# Returns a five element list describing the area occupied by the
# display line containing index.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::dlineinfo {index} {
    return [$text dlineinfo $index]
}

# ------------------------------------------------------------------
# METHOD get index1 ?index2?
#
# Return text from start index to end index.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::get {index1 {index2 {}}} {
    return [$text get $index1 $index2]
}

# ------------------------------------------------------------------
# METHOD image option ?arg arg ...?
#
# Manipulate images dependent on options.
#
# ------------------------------------------------------------------
::itcl::body Scrolledtext::image {option args} {
  return [uplevel 0 $text image $option $args]
}


# ------------------------------------------------------------------
# METHOD index index
#
# Return position corresponding to index.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::index {index} {
    return [$text index $index]
}

# ------------------------------------------------------------------
# METHOD insert index chars ?tagList?
#
# Insert text at index.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::insert {args} {
    uplevel 0 $text insert $args
}

# ------------------------------------------------------------------
# METHOD mark option ?arg arg ...?
#
# Manipulate marks dependent on options.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::mark {option args} {
    return [uplevle 0 $text mark $option $args]
}

# ------------------------------------------------------------------
# METHOD scan option args
#
# Implements scanning on texts.
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Scrolledtext::scan {option args} {
    uplevel 0 $text scan $option $args
}

# ------------------------------------------------------------------
# METHOD search ?switches? pattern index ?varName?
#
# Searches the text for characters matching a pattern.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::search {args} {
    #-----------------------------------------------------------
    # BUG FIX: csmith (Chad Smith: csmith@adc.com), 11/18/99
    #-----------------------------------------------------------
    # Need to run this command up one level on the stack since
    # the text widget may modify one of the arguments, which is
    # the case when -count is specified.
    #-----------------------------------------------------------
    return [uplevel 1 $text search $args]
}

# ------------------------------------------------------------------
# METHOD see index
#
# Adjusts the view in the window so the character at index is 
# visible.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::see {index} {
    $text see $index
}

# ------------------------------------------------------------------
# METHOD tag option ?arg arg ...?
#
# Manipulate tags dependent on options.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::tag {option args} {
    return [uplevel 0 $text tag $option $args]
}

# ------------------------------------------------------------------
# METHOD window option ?arg arg ...?
#
# Manipulate embedded windows.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::window {option args} {
    return [uplevle 0 $text window $option $args]
}

# ------------------------------------------------------------------
# METHOD xview
#
# Changes x view in widget's window.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::xview {args} {
    return [uplevel 0 $text xview $args]
}

# ------------------------------------------------------------------
# METHOD yview
#
# Changes y view in widget's window.
# ------------------------------------------------------------------
::itcl::body Scrolledtext::yview {args} {
    return [uplevle 0 $text yview $args]
}

} ; # end ::itcl::widgets
