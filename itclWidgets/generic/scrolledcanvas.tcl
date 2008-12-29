#
# Scrolledcanvas
# ----------------------------------------------------------------------
# Implements horizontal and vertical scrollbars around a canvas childsite
# Includes options to control display of scrollbars.  The standard
# canvas options and methods are supported.
#
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the ::itcl::widgets package Scrolledcanvas
# written by:
#
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: scrolledcanvas.tcl,v 1.1.2.1 2008/12/29 16:33:41 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Scrolledcanvas.width 200 widgetDefault
option add *Scrolledcanvas.height 230 widgetDefault
option add *Scrolledcanvas.labelPos n widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Scrolledcanvas class.
# 
proc ::::itcl::widgets::scrolledcanvas {pathName args} {
    uplevel ::::itcl::widgets::Scrolledcanvas $pathName $args
}

# ------------------------------------------------------------------
#                            SCROLLEDCANVAS
# ------------------------------------------------------------------
::itcl::extendedclass Scrolledcanvas {
    inherit ::itcl::widgets::Scrolledwidget

    component clipper
    component canvas

    option [list -autoresize autoResize AutoResize] -default 1 -configuremethod configAutoresize
    option [list -automargin autoMargin AutoMargin] -default 0

    delegate option [list -background background Background] to clipper as -highlightbackground 
    delegate option [list -textbackground textBackground Background] to canvas as -background 

    constructor {args} {}
    destructor {}

    protected method configAutoresize {option value}

    public method childsite {} 
    public method justify {direction} 

    public method addtag {args} 
    public method bbox {args} 
    public method bind {args} 
    public method canvasx {args} 
    public method canvasy {args} 
    public method coords {args} 
    public method create {args} 
    public method dchars {args} 
    public method delete {args} 
    public method dtag {args} 
    public method find {args} 
    public method focus {args} 
    public method gettags {args} 
    public method icursor {args} 
    public method index {args} 
    public method insert {args} 
    public method itemconfigure {args} 
    public method itemcget {args} 
    public method lower {args} 
    public method move {args} 
    public method postscript {args} 
    public method raise {args} 
    public method scale {args} 
    public method scan {args} 
    public method select {args} 
    public method type {args} 
    public method xview {args} 
    public method yview {args} 
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::constructor {args} {
    #
    # Create a clipping frame which will provide the border for
    # relief display.
    #
    setupcomponent clipper using frame $itcl_interior.clipper
    keepcomponentoption clipper -activebackground -activerelief -background \
        -borderwidth -cursor \
	-elementborderwidth -foreground -highlightcolor -highlightthickness \
	-insertbackground -insertborderwidth -insertofftime -insertontime \
	-insertwidth -jump -labelfont -selectbackground -selectborderwidth \
	-selectforeground -textbackground -troughcolor

    keepcomponentoption clipper -borderwidth -relief -highlightthickness \
        -highlightcolor
    grid $clipper -row 0 -column 0 -sticky nsew
    grid rowconfigure $_interior 0 -weight 1
    grid columnconfigure $_interior 0 -weight 1

    # 
    # Create a canvas to scroll
    #
    setupcomponent canvas using canvas $clipper.canvas \
		-height 1.0 -width 1.0 \
                -scrollregion "0 0 1 1" \
                -xscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.horizsb] \
		-yscrollcommand \
		[itcl::code $this _scrollWidget $itcl_interior.vertsb]
    keepcomponentoption canvas -activebackground -activerelief -background \
        -borderwidth -cursor \
	-elementborderwidth -foreground -highlightcolor -highlightthickness \
	-insertbackground -insertborderwidth -insertofftime -insertontime \
	-insertwidth -jump -labelfont -selectbackground -selectborderwidth \
	-selectforeground -textbackground -troughcolor

#	ignore -highlightthickness -highlightcolor

    keepcomponentoption canvas -closeenough -confine -scrollregion \
        -xscrollincrement -yscrollincrement

    grid $canvas -row 0 -column 0 -sticky nsew
    grid rowconfigure $clipper 0 -weight 1
    grid columnconfigure $clipper 0 -weight 1
    
    # 
    # Configure the command on the vertical scroll bar in the base class.
    #
    $vertsb configure \
	-command [itcl::code $canvas yview]

    #
    # Configure the command on the horizontal scroll bar in the base class.
    #
    $horizsb configure \
		-command [itcl::code $canvas xview]
    
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
::itcl::body Scrolledcanvas::destructor {} {
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -autoresize
#
# Automatically adjusts the scrolled region to be the bounding 
# box covering all the items in the canvas following the execution 
# of any method which creates or destroys items.  Thus, as new 
# items are added, the scrollbars adjust accordingly.
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::configAutoresize {option value} {
    if {$value} {
	set bbox [$canvas bbox all]

	if {$bbox ne {}} {
	    set marg $value
	    set bbox [lreplace $bbox 0 0 [expr {[lindex $bbox 0] - $marg}]]
	    set bbox [lreplace $bbox 1 1 [expr {[lindex $bbox 1] - $marg}]]
	    set bbox [lreplace $bbox 2 2 [expr {[lindex $bbox 2] + $marg}]]
	    set bbox [lreplace $bbox 3 3 [expr {[lindex $bbox 3] + $marg}]]
	}

	$canvas configure -scrollregion $bbox
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
::itcl::body Scrolledcanvas::childsite {} {
    return $canvas
}

# ------------------------------------------------------------------
# METHOD: justify
#
# Justifies the canvas scrolled region in one of four directions: top,
# bottom, left, or right.
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::justify {direction} {
    if {[winfo ismapped $canvas]} {
	update idletasks
	
	switch $direction {
        left { 
	    $canvas xview moveto 0
          }
        right {
	    $canvas xview moveto 1
          }
        top {
	    $canvas yview moveto 0
          }
        bottom {
	    $canvas yview moveto 1
          }
        default {
	    error "bad justify argument \"$direction\": should be\
		    left, right, top, or bottom"
          }
        }
    }
}

# ------------------------------------------------------------------
# CANVAS METHODS:
#
# The following methods are thin wraps of standard canvas methods.
# Consult the Tk canvas man pages for functionallity and argument
# documentation
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: addtag tag searchSpec ?arg arg ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::addtag {args} {
    return [uplevel 0 $canvas addtag $args]
}

# ------------------------------------------------------------------
# METHOD: bbox tagOrId ?tagOrId tagOrId ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::bbox {args} {
    return [uplevel 0 $canvas bbox $args]
}

# ------------------------------------------------------------------
# METHOD: bind tagOrId ?sequence? ?command?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::bind {args} {
    return [uplevel 0 $canvas bind $args]
}

# ------------------------------------------------------------------
# METHOD: canvasx screenx ?gridspacing?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::canvasx {args} {
    return [uplevel 0 $canvas canvasx $args]
}

# ------------------------------------------------------------------
# METHOD: canvasy screeny ?gridspacing?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::canvasy {args} {
    return [uplevel 0 $canvas canvasy $args]
}

# ------------------------------------------------------------------
# METHOD: coords tagOrId ?x0 y0 ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::coords {args} {
    return [uplevel 0 $canvas coords $args]
}

# ------------------------------------------------------------------
# METHOD: create type x y ?x y ...? ?option value ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::create {args} {
    set retval [uplevel 0 $canvas create $args]
    configure -autoresize $itcl_options(-autoresize)
    return $retval
}

# ------------------------------------------------------------------
# METHOD: dchars  tagOrId first ?last?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::dchars {args} {
    return [uplevel 0 $canvas dchars $args]
}

# ------------------------------------------------------------------
# METHOD: delete tagOrId ?tagOrId tagOrId ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::delete {args} {
    set retval [uplevel 0 $canvas delete $args]
    configure -autoresize $itcl_options(-autoresize)
    return $retval
}

# ------------------------------------------------------------------
# METHOD: dtag tagOrId ?tagToDelete?
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Scrolledcanvas::dtag {args} {
    uplevel 0 $canvas dtag $args
    configure -autoresize $itcl_options(-autoresize)
}

# ------------------------------------------------------------------
# METHOD: find searchCommand ?arg arg ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::find {args} {
    return [uplevel 0 $canvas find $args]
}

# ------------------------------------------------------------------
# METHOD: focus ?tagOrId?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::focus {args} {
    return [uplevel 0 $canvas focus $args]
}

# ------------------------------------------------------------------
# METHOD: gettags tagOrId
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::gettags {args} {
    return [uplevel 0 $canvas gettags $args]
}

# ------------------------------------------------------------------
# METHOD: icursor tagOrId index
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::icursor {args} {
    uplevel 0 $canvas icursor $args
}

# ------------------------------------------------------------------
# METHOD: index tagOrId index
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::index {args} {
    return [uplevel 0 $canvas index $args]
}

# ------------------------------------------------------------------
# METHOD: insert tagOrId beforeThis string
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::insert {args} {
    uplevel 0 $canvas insert $args
}

# ------------------------------------------------------------------
# METHOD: itemconfigure tagOrId ?option? ?value? ?option value ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::itemconfigure {args} {
    set retval [uplevel 0 $canvas itemconfigure $args]
    configure -autoresize $itcl_options(-autoresize)
    return $retval
}

# ------------------------------------------------------------------
# METHOD: itemcget tagOrId ?option? 
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::itemcget {args} {
    set retval [uplevel 0 $canvas itemcget $args]
    return $retval
}

# ------------------------------------------------------------------
# METHOD: lower tagOrId ?belowThis?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::lower {args} {
    uplevel 0 $canvas lower $args
}

# ------------------------------------------------------------------
# METHOD: move tagOrId xAmount yAmount
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::move {args} {
    uplevel 0 $canvas move $args
    return [configure -autoresize $itcl_options(-autoresize)]
}

# ------------------------------------------------------------------
# METHOD: postscript ?option value ...?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::postscript {args} {
    #
    # Make sure the fontmap is in scope.
    #
    set fontmap ""
    regexp -- {-fontmap +([^ ]+)} $args all fontmap
    if {$fontmap != ""} {
	global $fontmap
    }
    return [uplevel 0 $canvas postscript $args]
}

# ------------------------------------------------------------------
# METHOD: raise tagOrId ?aboveThis?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::raise {args} {
    uplevel 0 $canvas raise $args
}

# ------------------------------------------------------------------
# METHOD: scale tagOrId xOrigin yOrigin xScale yScale
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::scale {args} {
    uplevel 0 $canvas scale $args
}

# ------------------------------------------------------------------
# METHOD: scan option args
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::scan {args} {
    uplevel 0 $canvas scan $args
}

# ------------------------------------------------------------------
# METHOD: select option ?tagOrId arg?
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::select {args} {
    uplevel 0 $canvas select $args
}

# ------------------------------------------------------------------
# METHOD: type tagOrId
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::type {args} {
    return [uplevel 0 $canvas type $args]
}

# ------------------------------------------------------------------
# METHOD: xview index
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::xview {args} {
    uplevel 0 $canvas xview $args
}

# ------------------------------------------------------------------
# METHOD: yview index 
# ------------------------------------------------------------------
::itcl::body Scrolledcanvas::yview {args} {
    uplevel 0 $canvas yview $args
}

} ; # end ::itcl::widgets
