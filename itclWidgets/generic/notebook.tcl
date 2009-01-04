#
# Notebook Widget
# ----------------------------------------------------------------------
# The Notebook command creates a new window (given by the pathName 
# argument) and makes it into a Notebook widget. Additional options, 
# described above may be specified on the command line or in the 
# option database to configure aspects of the Notebook such as its 
# colors, font, and text. The Notebook command returns its pathName 
# argument. At the time this command is invoked, there must not exist 
# a window named pathName, but path Name's parent must exist.
# 
# A Notebook is a widget that contains a set of pages. It displays one 
# page from the set as the selected page. When a page is selected, the 
# page's contents are displayed in the page area. When first created a 
# Notebook has no pages. Pages may be added or deleted using widget commands 
# described below.
# 
# A special option may be provided to the Notebook. The -auto option 
# specifies whether the Nptebook will automatically handle the unpacking 
# and packing of pages when pages are selected. A value of true signifies 
# that the notebook will automatically manage it. This is the default 
# value. A value of false signifies the notebook will not perform automatic 
# switching of pages.
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
# This code is derived/reimplemented from the ::itcl::widgets package Notebook
# written by:
#    AUTHOR: Bill W. Scott
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: notebook.tcl,v 1.1.2.2 2009/01/04 13:51:10 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Notebook.background          #d9d9d9      widgetDefault
option add *Notebook.auto                true         widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercase access method for the Notebook class
#
proc ::itcl::widgets::notebook {pathName args} {
    uplevel ::itcl::widgets::Notebook $pathName $args
}

# ------------------------------------------------------------------
#                            NOTEBOOK
# ------------------------------------------------------------------
::itcl::extendedclass Notebook {
    component itcl_hull
    component itcl_interior
    component cs
    
    option [list -background background Background] -default #d9d9d9 -configuremethod configBackground
    option [list -auto auto Auto] -default true -configuremethod configAuto
    option [list -scrollcommand scrollCommand ScrollCommand] -default {} -configuremethod configScrollcommand
    
    private variable _currPage -1  ;# numerical index of current page selected
    private variable _pages {}     ;# list of Page components
    private variable _uniqueID 0   ;# one-up number for unique page numbering
    
    constructor {args} {}
    
    private method _childSites { } 
    private method _scrollCommand { } 
    private method _index { pathList index select} 
    private method _createPage { args } 
    private method _deletePages { fromPage toPage } 
    private method _configurePages { args } 
    private method _tabCommand { } 
    
    protected method configBackground {option value}
    protected method configAuto {option value}
    protected method configScrollcommand {option value}

    public method add {args}
    public method childsite {args}
    public method delete {args} 
    public method index {args} 
    public method insert {args} 
    public method prev {} 
    public method next {} 
    public method pageconfigure {args} 
    public method pagecget {index option}
    public method select {index} 
    public method view {args} 
    
}

# ------------------------------------------------------------------
#                      CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Notebook::constructor {args}  {
    set win [createhull frame $this -class [info class] -borderwidth 0]
    set itcl_interior $win
    #
    # Create the outermost frame to maintain geometry.
    #
    setupcomponent cs using frame $itcl_interior.cs 
    keepcomponentoption cs -cursor -background -width -height
    pack $cs -fill both -expand yes
    pack propagate $cs no
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
    # force bg of all pages to reflect Notebook's background.
    _configurePages -background $itcl_options(-background)
}

# ------------------------------------------------------------------
#                      OPTIONS
# ------------------------------------------------------------------
# ------------------------------------------------------------------
# OPTION -background
#
# Sets the bg color of all the pages in the Notebook.
# ------------------------------------------------------------------
::itcl::body Notebook::configBackground {option value} {
    if {$value ne {}} {
	_configurePages -background $value
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION -auto
#
# Determines whether pages are automatically unpacked and
# packed when pages get selected.
# ------------------------------------------------------------------
::itcl::body Notebook::configAuto {option value} {
    if {$value ne {}} {
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION -scrollcommand
#
# Command string to be invoked when the notebook 
# has any changes to its current page, or number of pages.
# 
# typically for scrollbars.
# ------------------------------------------------------------------
::itcl::body Notebook::configScrollcommand {option value} {
    if {$value ne {}} {
	_scrollCommand
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# METHOD: add add ?<option> <value>...?
# 
# Creates a page and appends it to the list of pages.
# processes pageconfigure for the page added.
# ------------------------------------------------------------------
::itcl::body Notebook::add {args} {
    # The args list should be an even # of params, if not then
    # prob missing value for last item in args list. Signal error.
    set len [llength $args]
    if {$len % 2} {
	error "value for \"[lindex $args [expr {$len - 1}]]\" missing"
    }
    # add a Page component
    set pathName [uplevel 0 _createPage $args]
    lappend _pages $pathName
    # update scroller
    _scrollCommand 
    # return childsite for the Page component
    return [uplevel 0 $pathName childsite]
}

# ------------------------------------------------------------------
# METHOD: childsite ?<index>?
#
# If index is supplied, returns the child site widget corresponding 
# to the page index.  If called with no arguments, returns a list 
# of all child sites
# ------------------------------------------------------------------
::itcl::body Notebook::childsite {args} {
    set len [llength $args]
    switch $len {
    0 {
        # ... called with no arguments, return a list
        if {[llength $args] == 0} {
	    return [_childSites]
        }
      }
    1 {
        set index [lindex $args 0]
        # ... otherwise, return child site for the index given
        # empty notebook
        if {$_pages == {}} {
	    error "can't get childsite,\
		    no pages in the notebook \"$itcl_hull\""
        }
        set index [_index $_pages $index $_currPage]
        # index out of range
        if {($index < 0) || ($index >= [llength $_pages])} {
	    error "bad Notebook page index in childsite method:\
		    should be between 0 and [expr {[llength $_pages] - 1}]"
        }
        set pathName [lindex $_pages $index]
        set my_cs [uplevel 0 $pathName childsite]
        return $my_cs
      }
    default {
        # ... too many parameters passed
        error "wrong # args: should be\
	    \"$itcl_hull childsite ?index?\""
      }
    }
}

# ------------------------------------------------------------------
# METHOD: delete <index1> ?<index2>?
# 
# Deletes a page or range of pages from the notebook
# ------------------------------------------------------------------
::itcl::body Notebook::delete {args} {
    # empty notebook
    if {$_pages == {}} {
	error "can't delete page, no pages in the notebook\
		\"$itcl_hull\""
    }
    set len [llength $args]
    switch -- $len {
    1 {
        set fromPage [_index $_pages [lindex $args 0] $_currPage]
        if {($fromPage < 0) || ($fromPage >= [llength $_pages])} {
	    error "bad Notebook page index in delete method:\
		should be between 0 and [expr {[llength $_pages] - 1}]"
        }
        set toPage $fromPage
        _deletePages $fromPage $toPage
      }
    2 {
        set fromPage [_index $_pages [lindex $args 0] $_currPage]
        if {($fromPage < 0) || ($fromPage >= [llength $_pages])} {
	    error "bad Notebook page index1 in delete method:\
		should be between 0 and [expr {[llength $_pages] - 1}]"
        }
        set toPage [_index $_pages [lindex $args 1] $_currPage]
        if {($toPage < 0) || ($toPage >= [llength $_pages])} {
	    error "bad Notebook page index2 in delete method:\
		should be between 0 and [expr {[llength $_pages] - 1}]"
	    error "bad Notebook page index2"
        }
        if { $fromPage > $toPage } {
	    error "bad Notebook page index1 in delete method:\
		index1 is greater than index2"
        }
        _deletePages $fromPage $toPage
      }
    default {
        # ... too few/many parameters passed
        error "wrong # args: should be\
	    \"$itcl_hull delete index1 ?index2?\""
      }
    }
}

# ------------------------------------------------------------------
# METHOD: index <index>
#
# Given an index identifier returns the numeric index of the page
# ------------------------------------------------------------------
::itcl::body Notebook::index { args } {
    if {[llength $args] != 1} {
	error "wrong # args: should be\
		\"$itcl_hull index index\""
    }
    set index $args
    set number [_index $_pages $index $_currPage]
    return $number
}

# ------------------------------------------------------------------
# METHOD: insert <index> ?<option> <value>...?
#
# Inserts a page before a index. The before page may
# be specified as a label or a page position. 
# ------------------------------------------------------------------
::itcl::body Notebook::insert {args} {
    # ... Error: no args passed
    set len [llength $args]
    if {$len == 0} {
	error "wrong # args: should be\
		\"$itcl_hull insert index ?option value?\""
    }
    # ... set up index and args 
    set index [lindex $args 0]
    set args [lrange $args 1 $len]
    # ... Error: unmatched option value pair (len is odd)
    # The args list should be an even # of params, if not then
    # prob missing value for last item in args list. Signal error.
    set len [llength $args]
    if {$len % 2} {
	error "value for \"[lindex $args [expr {$len - 1}]]\" missing"
    }
    # ... Error: catch notebook empty
    if {$_pages == {}} {
	error "can't insert page, no pages in the notebook\
		\"$itcl_hull\""
    }
    # ok, get the page
    set page [_index $_pages $index $_currPage]
    # ... Error: catch bad value for before page.
    if {($page < 0) || ($page >= [llength $_pages])} {
	error "bad Notebook page index in insert method:\
		should be between 0 and [expr {[llength $_pages] - 1}]"
    }
    # ... Start the business of inserting
    # create the new page and get its path name...
    set pathName [eval _createPage $args]
    # grab the name of the page currently selected. (to keep in sync)
    set currPathName [lindex $_pages $_currPage]
    # insert pathName before $page
    set _pages [linsert $_pages $page $pathName]
    # keep the _currPage in sync with the insert.
    set _currPage [lsearch -exact $_pages $currPathName]
    # give scrollcommand chance to update
    _scrollCommand 
    # give them child site back...
    return [uplevel 0 $pathName childsite]
}

# ------------------------------------------------------------------
# METHOD: prev
#
# Selects the previous page. Wraps at first back to last page.
# ------------------------------------------------------------------
::itcl::body Notebook::prev { } {
    # catch empty notebook
    if {$_pages == {}} {
	error "can't move to previous page,\
		no pages in the notebook \"$itcl_hull\""
    }
    # bump to the previous page and wrap if necessary
    set prev [expr {$_currPage - 1}]
    if {$prev < 0} {
	set prev [expr {[llength $_pages] - 1}]
    }
    select $prev
    return $prev
}

# ------------------------------------------------------------------
# METHOD: next
#
# Selects the next page. Wraps at last back to first page.
# ------------------------------------------------------------------
::itcl::body Notebook::next {} {
    # catch empty notebook
    if {$_pages == {}} {
	error "can't move to next page,\
		no pages in the notebook \"$itcl_hull\""
    }
    
    # bump to the next page and wrap if necessary
    set next [expr {$_currPage + 1}]
    if {$next >= [llength $_pages]} {
	set next 0
    }
    select $next
    return $next
}

# ------------------------------------------------------------------
# METHOD: pageconfigure <index> ?<option> <value>...?
#
# Performs configure on a given page denoted by index.  Index may 
# be a page number or a pattern matching the label associated with 
# a page.
# ------------------------------------------------------------------
::itcl::body Notebook::pageconfigure {args} {
    # ... Error: no args passed
    set len [llength $args]
    if {$len == 0} {
	error "wrong # args: should be\
		\"$itcl_hull pageconfigure index ?option value?\""
    }
    # ... set up index and args 
    set index [lindex $args 0]
    set args [lrange $args 1 $len]
    set page [_index $_pages $index $_currPage]
    # ... Error: page out of range
    if {($page < 0) || ($page >= [llength $_pages])} {
	error "bad Notebook page index in pageconfigure method:\
		should be between 0 and [expr {[llength $_pages] - 1}]"
    }
    # Configure the page component
    set pathName [lindex $_pages $page]
    return [uplevel 0 $pathName configure $args]
}

# ------------------------------------------------------------------
# METHOD: pagecget <index> <option>
#
# Performs cget on a given page denoted by index.  Index may 
# be a page number or a pattern matching the label associated with 
# a page.
# ------------------------------------------------------------------
::itcl::body Notebook::pagecget {index option} {
    set page [_index $_pages $index $_currPage]
    # ... Error: page out of range
    if {($page < 0) || ($page >= [llength $_pages])} {
	error "bad Notebook page index in pagecget method:\
		should be between 0 and [expr {[llength $_pages] - 1}]"
    }
    # Get the page info.
    set pathName [lindex $_pages $page]
    return [$pathName cget $option]
}
 
# ------------------------------------------------------------------
# METHOD: select <index>
#
# Select a page by index.  Hide the last _currPage if it existed.
# Then show the new one if it exists.  Returns the currently 
# selected page or -1 if tried to do a select select when there is 
# no selection.
# ------------------------------------------------------------------
::itcl::body Notebook::select {index} {
    global page$win
    # ... Error: empty notebook
    if {$_pages == {}} {
	error "can't select page $index,\
		no pages in the notebook \"$itcl_hull\""
    }
    # if there is not current selection just ignore trying this selection
    if {($index eq "select") && ($_currPage == -1)} {
	return -1
    }
    set reqPage [_index $_pages $index $_currPage]
    if {($reqPage < 0) || ($reqPage >= [llength $_pages])} {
	error "bad Notebook page index in select method:\
		should be between 0 and [expr {[llength $_pages] - 1}]"
    }
    # if we already have this page selected, then ignore selection.
    if {$reqPage == $_currPage} {
	return $_currPage
    }
    # if we are handling packing and unpacking the unpack if we can
    if {$itcl_options(-auto)} {
	# if there is a current page packed, then unpack it
	if {$_currPage != -1} {
	    set currPathName [lindex $_pages $_currPage]
	    pack forget $currPathName
	}
    }
    # set this now so that the -command cmd can do an 'index select'
    # to operate on this page.
    set _currPage $reqPage
    # invoke the command for this page
    set cmd [lindex [pageconfigure $index -command] 4]
    uplevel 0 $cmd
    # give scrollcommand chance to update
    _scrollCommand 
    # if we are handling packing and unpacking the pack if we can
    if {$itcl_options(-auto)} {
	set reqPathName [lindex $_pages $reqPage]
	pack $reqPathName -anchor nw -fill both -expand yes
    }
    return $_currPage
}


# ------------------------------------------------------------------
# METHOD: view
#
# Return the current page
#
#	  view <index>
#
# Selects the page denoted by index to be current page
#
#         view 'moveto' <fraction>
#
# Selects the page by using fraction amount
#
#	  view 'scroll' <num> <what>
#
# Selects the page by using num as indicator of next or	previous
# ------------------------------------------------------------------
::itcl::body Notebook::view {args} {
    set len [llength $args]
    switch -- $len {
    0 {
        # Return current page
        return $_currPage
      }
    1 {
        # Select by index
        select [lindex $args 0]
      }
    2 {
        # Select using moveto
        set arg [lindex $args 0]
        if {$arg eq "moveto"} {
	    set fraction [lindex $args 1]
	    if {[catch {set page \
		    [expr {round($fraction/(1.0/[llength $_pages]))}]}]} {
	        error "expected floating-point number \
		        but got \"$fraction\""
	    }
	    if {$page == [llength $_pages]} {
	        incr page -1
	    }
	    if {($page >= 0) && ($page < [llength $_pages])} {
	        select $page
	    }
        } else {
	    error "expected \"moveto\" but got $arg"
        }
      }
    3 {
        # Select using scroll keyword
        set arg [lindex $args 0]
        if {$arg eq "scroll"} {
	    set amount [lindex $args 1]
	    # check for integer value
	    if {![regexp {^[-]*[0-9]*$} $amount]} {
	        error "expected integer but got \"$amount\""
	    }
	    set page [expr {$_currPage + $amount}]
	    if {($page >= 0) && ($page < [llength $_pages])} {
	        select $page
	    }
        } else {
	    error "expected \"scroll\" but got $arg"
        }
      }
    default {
        set arg [lindex $args 0]
        if {$arg eq "moveto"} {
	    error "wrong # args: should be\
		\"$itcl_hull view moveto fraction\""
        } elseif {$arg eq "scroll"} {
	    error "wrong # args: should be\
		\"$itcl_hull view scroll units|pages\""
        } else {
	    error "wrong # args: should be\
		\"$itcl_hull view index\""
        }
      }
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _childSites
#
# Returns a list of child sites for all pages in the notebook.
# ------------------------------------------------------------------
::itcl::body Notebook::_childSites {} {
    # empty notebook
    if {$_pages eq {}} {
	error "can't get childsite list,\
		no pages in the notebook \"$itcl_hull\""
    }
    set csList {}
    foreach pathName $_pages { 
	lappend csList [eval $pathName childsite]
    }
    return $csList
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _scrollCommand
#
# If there is a -scrollcommand set up, then call the tcl command
# and suffix onto it the standard 4 numbers scrollbars get.
#
# Invoke the scrollcommand, this is like the y/xscrollcommand
# it is designed to talk to scrollbars and the the
# tabset also knows how to obey scrollbar protocol.
# ------------------------------------------------------------------
::itcl::body Notebook::_scrollCommand {} {
    if {$itcl_options(-scrollcommand) ne {}} {
        if {$_currPage != -1}  {
	    set relTop [expr {($_currPage*1.0) / [llength $_pages]}]
	    set relBottom [expr {(($_currPage+1)*1.0) / [llength $_pages]}]
	    set scrollCommand "$itcl_options(-scrollcommand) $relTop $relBottom"
	} else {
	    set scrollCommand "$itcl_options(-scrollcommand) 0 1"
	}
	uplevel #0 $scrollCommand
    }
}

# ------------------------------------------------------------------
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
# ------------------------------------------------------------------
::itcl::body Notebook::_index { pathList index select} {
    switch -- $index {
    select {
        set number $select
      }
    end {
        set number [expr {[llength $pathList] -1}]
      }
    default {
        # is it a number already?
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
	        set label [lindex [$pathName configure -label] 4]
	        if {[string match $label $index]} {
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

# ------------------------------------------------------------------
# PRIVATE METHOD: _createPage
#
# Creates a page, using unique page naming, propagates background
# and keeps unique id up to date.
# ------------------------------------------------------------------
::itcl::body Notebook::_createPage {args} {
    #
    # create an internal name for the page: .n.cs.page0, .n.cs.page1, etc.
    #
    set pathName $cs.page$_uniqueID
    
    uplevel #0 ::itcl::widgets::Page $pathName -background $itcl_options(-background) $args
    incr _uniqueID
    return $pathName
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _deletePages
#
# Deletes pages from $fromPage to $toPage.
#
# Operates in two passes, destroys all the widgets
# Then removes the pathName from the page list
#
# Also keeps the current selection in bounds.
# ------------------------------------------------------------------
::itcl::body Notebook::_deletePages {fromPage toPage} {
    for {set page $fromPage} {$page <= $toPage} {incr page} {
	# kill the widget
	set pathName [lindex $_pages $page]
	destroy $pathName
    }
    # physically remove the page
    set _pages [lreplace $_pages $fromPage $toPage]
    # If we deleted a selected page set our selection to none
    if {($_currPage >= $fromPage) && ($_currPage <= $toPage)} {
	set _currPage -1
    }
    # make sure _currPage stays in sync with new numbering...
    if {$_pages == {}} {
	# if deleted only remaining page,
	# reset current page to undefined
	set _currPage -1
	# or if the current page was the last page, it needs come back
    } elseif {$_currPage >= [llength $_pages]} {
	incr _currPage -1
	if { $_currPage < 0 } {
	    # but only to zero
	    set _currPage 0
	}
    }
    # give scrollcommand chance to update
    _scrollCommand 
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _configurePages
#
# Does the pageconfigure method on each page in the notebook
# ------------------------------------------------------------------
::itcl::body Notebook::_configurePages {args} {
    # make sure we have pages
    if {[catch {set _pages}]} {
	return
    }
    # go thru all pages and pageconfigure them.
    foreach pathName $_pages {
	uplevel 0 $pathName configure $args
    }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _tabCommand
#
# Calls the command that was passed in through the 
# $itcl_options(-tabcommand) argument.
#
# This method is up for debate... do we need the -tabcommand option?
# ------------------------------------------------------------------
::itcl::body Notebook::_tabCommand { } {
    global page$win
    
    if {$itcl_options(-tabcommand) ne {}} {
	set newTabCmdStr $itcl_options(-tabcommand)
	lappend newTabCmdStr [set page$itk_component(hull)]
	#eval $newTabCmdStr
	uplevel #0 $newTabCmdStr
    }
}
    
#
# Page widget
# ------------------------------------------------------------------
#
# The Page command creates a new window (given by the pathName argument) 
# and makes it into a Page widget. Additional options, described above 
# may be specified on the com mand line or in the option database to 
# configure aspects of the Page such as its back ground, cursor, and 
# geometry. The Page command returns its pathName argument. At the time 
# this command is invoked, there must not exist a window named pathName, 
# but path Name's parent must exist.
# 
# A Page is a frame that holds a child site. It is nothing more than a 
# frame widget with some intelligence built in. Its primary purpose is 
# to support the Notebook's concept of a page. It allows another widget 
# like the Notebook to treat a page as a single object. The Page has an 
# associated label and knows how to return its child site.
#
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Tabnotebook
# written by:
#  AUTHOR: Bill W. Scott                 EMAIL: bscott@spd.dsccc.com
#
# ------------------------------------------------------------------
#               Copyright (c) 1995  DSC Communications Corp.
# ======================================================================

# Option database default resources:
#
option add *Page.disabledForeground #a3a3a3     widgetDefault
option add *Page.label              {}       widgetDefault
option add *Page.command            {}       widgetDefault

::itcl::extendedclass Page {
    component itcl_hull
    component itcl_interior
    component cs
    
    option [list \
	    -disabledforeground disabledForeground DisabledForeground] -default #a3a3a3 
    option [list -label label Label] -default {} 
    option [list -command command Command] -default {}
    
    constructor {args} {}
    
    protected method configDisabledforeground {option value}
    protected method configLabel {option value}
    protected method configCommand {option value}

    public method childsite { } 
}

# ------------------------------------------------------------------
#                          CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Page::constructor {args} {
    set win [createhull frame $this -class [info class] -borderwidth 0]
    set itcl_interior $win
    #
    # Create the outermost frame to maintain geometry.
    #
    setupcomponent cs using frame $itcl_interior.cs 
    keepcomponentoption cs -cursor -background -width -height
    pack $cs -fill both -expand yes 
    pack propagate $cs no
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

# ------------------------------------------------------------------
#                            OPTIONS
# ------------------------------------------------------------------
# ------------------------------------------------------------------
# OPTION -disabledforeground
#
# Sets the disabledForeground color of this page
# ------------------------------------------------------------------
::itcl::body Page::configDisabledforeground {option value} {
}

# ------------------------------------------------------------------
# OPTION -label
#
# Sets the label of this page.  The label is a string identifier 
# for this page.
# ------------------------------------------------------------------
::itcl::body Page::configLabel {option value} {
}

# ------------------------------------------------------------------
# OPTION -command
#
# The Tcl Command to associate with this page.
# ------------------------------------------------------------------
::itcl::body Page::configCommand {option value} {
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Returns the child site widget of this page
# ------------------------------------------------------------------
::itcl::body Page::childsite { } {
    return $cs
}

} ; # end ::itcl::widgets
