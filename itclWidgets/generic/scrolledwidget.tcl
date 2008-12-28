#
# Scrolledwidget
# ----------------------------------------------------------------------
# Implements a general purpose base class for scrolled widgets, by
# creating the necessary horizontal and vertical scrollbars and 
# providing protected methods for controlling their display.  The 
# derived class needs to take advantage of the fact that the grid
# is used and the vertical scrollbar is in row 0, column 2 and the
# horizontal scrollbar in row 2, column 0.
#
# ----------------------------------------------------------------------
#  AUTHOR: Mark Ulferts                        mulferts@austin.dsccc.com 
#
#  @(#) $Id: scrolledwidget.tcl,v 1.1.2.1 2008/12/28 12:10:38 wiede Exp $
# ----------------------------------------------------------------------
#            Copyright (c) 1997 DSC Technologies Corporation
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
# Provide a lowercased access method for the Scrolledwidget class.
# 
proc ::itcl::widgets::scrolledwidget {pathName args} {
    uplevel ::itcl::widgets::Scrolledwidget $pathName $args
}

# ------------------------------------------------------------------
#                            SCROLLEDWIDGET
# ------------------------------------------------------------------
::itcl::extendedclass ::itcl::widgets::Scrolledwidget {
    inherit ::itcl::widgets::Labeledwidget

    component horizsb
    component vertsb

    option [list -sbwidth sbWidth Width] -default 15 -configuremethod configSbwidth
    option [list -scrollmargin scrollMargin ScrollMargin] -default 3  -configuremethod configScrollmargin
    option [list -vscrollmode vscrollMode VScrollMode] -default static -configuremethod configVscrollmode
    option [list -hscrollmode hscrollMode HScrollMode] -default static -configuremethod configHscrollmode
    option [list -width width Width] -default 30 -configuremethod configWidth
    option [list -height height Height] -default 30 -configuremethod configHeight

    protected variable _vmode off            ;# Vertical scroll mode
    protected variable _hmode off            ;# Vertical scroll mode
    protected variable _recheckHoriz 1       ;# Flag to check need for 
                                             ;#  horizontal scrollbar
    protected variable _recheckVert 1        ;# Flag to check need for 
                                             ;#  vertical scrollbar
    protected variable _interior {}

    constructor {args} {}
    destructor {}

    protected method _scrollWidget {wid first last} 
    protected method _vertScrollbarDisplay {mode} 
    protected method _horizScrollbarDisplay {mode} 
    protected method _configureEvent {}
    proetcted method configSbwidth {option value}
    proetcted method configScrollmargin {option value}
    proetcted method configHscrollmode {option value}
    proetcted method configVscrollmode {option value}
    proetcted method configHeight {option value}
    proetcted method configWidth {option value}

}

#
# Use option database to override default resources of base classes.
#
option add *Scrolledwidget.labelPos n widgetDefault

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Scrolledwidget::constructor {args} {

    #
    # Turn off the borderwidth on the hull and save off the 
    # interior for later use.
    #
    $itcl_hull configure -borderwidth 0
    set _interior $itcl_interior

    #
    # Check if the scrollbars need mapping upon a configure event.
    #
    bind $_interior <Configure> [itcl::code $this _configureEvent]

    #
    # Turn off propagation in the containing shell.
    #
    # Due to a bug in the tk4.2 grid, we have to check the 
    # propagation before setting it.  Setting it to the same
    # value it already is will cause it to toggle.
    #
    if {[grid propagate $_interior]} {
	grid propagate $_interior no
    }
	
    # 
    # Create the vertical scroll bar
    #
    setupcomponent vertsb using scrollbar $itk_interior.vertsb -orient vertical 
    keepcomponentoption vertsb -background -borderwidth -cursor \
        -highlightcolor -highlightthickness -activebackground -activerelief \
	-jump -troughcolor -labelfont -foreground
    keepcomponentoption vertsb -borderwidth -elementborderwidth -jump -relief 
#	rename -highlightbackground -background background Background

    #
    # Create the horizontal scrollbar
    #
    setupcomponent horizsb using scrollbar $itk_interior.horizsb -orient horizontal 
    keepcomponentoption horizsb -background -borderwidth -cursor \
        -highlightcolor -highlightthickness -activebackground -activerelief \
	-jump -troughcolor -labelfont -foreground
    keepcomponentoption horizsb -borderwidth -elementborderwidth -jump -relief 
#	rename -highlightbackground -background background Background
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
::itcl::body ::itcl::widgets::Scrolledwidget::destructor {} {
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -sbwidth
#
# Set the width of the scrollbars.
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::configSbwidth {option value} {
    $vertsb configure -width $value
    $horizsb configure -width $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -scrollmargin
#
# Set the distance between the scrollbars and the list box.
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::configScrollmargin {option value} {
    set pixels [winfo pixels $_interior	$value]
    if {$_hmode == "on"} {
	grid rowconfigure $_interior 1 -minsize $pixels
    }
    if {$_vmode == "on"} {
	grid columnconfigure $_interior 1 -minsize $pixels
    }
}

# ------------------------------------------------------------------
# OPTION: -vscrollmode
#
# Enable/disable display and mode of veritcal scrollbars.
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::configVscrollmode {option value} {
    switch $value {
    static {
        _vertScrollbarDisplay on
      }
    dynamic -
    none {
        _vertScrollbarDisplay off
      }
    default {
        error "bad vscrollmode option \"$value\": should be\
            static, dynamic, or none"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -hscrollmode
#
# Enable/disable display and mode of horizontal scrollbars.
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::configHscrollmode {option value} {
    switch $itcl_options(-hscrollmode) {
    static {
        _horizScrollbarDisplay on
    }
    dynamic -
    none {
        _horizScrollbarDisplay off
      }
    default {
        error "bad hscrollmode option \"$value\": should be\
            static, dynamic, or none"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -width
#
# Specifies the width of the scrolled widget.  The value may be 
# specified in any of the forms acceptable to Tk_GetPixels.  
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::configWidth {option value} {
    $_interior configure -width [winfo pixels $_interior $value] 
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -height
#
# Specifies the height of the scrolled widget.  The value may be 
# specified in any of the forms acceptable to Tk_GetPixels.  
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::configHeight {option value} {
    $_interior configure -height [winfo pixels $_interior $value] 
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# PROTECTED METHOD: _vertScrollbarDisplay mode
#
# Displays the vertical scrollbar based on the input mode.
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::_vertScrollbarDisplay {mode} {
    switch $mode  {
    on {
        set _vmode on
        grid columnconfigure $_interior 1 -minsize \
	    [winfo pixels $_interior $itcl_options(-scrollmargin)]
        grid $vertsb -row 0 -column 2 -sticky ns
      }
    off {
	set _vmode off
	grid columnconfigure $_interior 1 -minsize 0
	grid forget $vertsb 
      }
    default {
        error "invalid argument \"$mode\": should be on or off"
      }
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _horizScrollbarDisplay mode
#
# Displays the horizontal scrollbar based on the input mode.
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::_horizScrollbarDisplay {mode} {
    switch $mode  {
    on {
        set _hmode on
        grid rowconfigure $_interior 1 -minsize \
	    [winfo pixels $_interior $itcl_options(-scrollmargin)]
        grid $horizsb -row 2 -column 0 -sticky ew
      }
    off {
        set _hmode off
    
        grid rowconfigure $_interior 1 -minsize 0
        grid forget $horizsb 
      }
    default {
        error "invalid argument \"$mode\": should be on or off"
      }
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _scrollWidget wid first last
#
# Performs scrolling and display of scrollbars based on the total 
# and maximum frame size as well as the current -vscrollmode and 
# -hscrollmode settings.  Parameters are automatic scroll parameters.
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::_scrollWidget {wid first last} {
    $wid set $first $last
    if {$wid == $vertsb} {
	if {$itcl_options(-vscrollmode) == "dynamic"} {
	    if {($_recheckVert != 1) && ($_vmode == "on")} {
		return
	    } else {
		set _recheckVert 0
	    }
	    if {($first == 0) && ($last == 1)} {
		if {$_vmode != "off"} {
		    _vertScrollbarDisplay off
		}
	    } else {
		if {$_vmode != "on"} {
		    _vertScrollbarDisplay on
		}
	    }
	}
	
    } elseif {$wid == $horizsb} {
	if {$itcl_options(-hscrollmode) == "dynamic"} {
	    if {($_recheckHoriz != 1) && ($_hmode == "on")} {
		return
	    } else {
		set _recheckHoriz 0
	    }

	    if {($first == 0) && ($last == 1)} {
		if {$_hmode != "off"} {
		    _horizScrollbarDisplay off
		}
		
	    } else {
		if {$_hmode != "on"} {
		    _horizScrollbarDisplay on
		}
	    }
	}
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _configureEvent
#
# Resets the recheck flags which determine if we'll try and map
# the scrollbars in dynamic mode.  
# ------------------------------------------------------------------
::itcl::body Scrolledwidget::_configureEvent {} {
    update idletasks
    set _recheckVert 1
    set _recheckHoriz 1
}

} ; # end ::itcl::widgets
