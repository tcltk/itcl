#
# Labeledwidget
# ----------------------------------------------------------------------
# Implements a labeled widget which contains a label and child site.
# The child site is a frame which can filled with any widget via a 
# derived class or though the use of the childsite method.  This class
# was designed to be a general purpose base class for supporting the 
# combination of label widget and a childsite, where a label may be 
# text, bitmap or image.  The options include the ability to position 
# the label around the childsite widget, modify the font and margin, 
# and control the display of the label.  
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version

# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: labeledwidget.tcl,v 1.1.2.2 2009/01/05 19:30:47 wiede Exp $
# ======================================================================

package require itcl

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Labeledwidget class.
# 
proc labeledwidget {pathName args} {
    uplevel ::itcl::widgets::Labeledwidget $pathName $args
}

# ------------------------------------------------------------------
#                            LABELEDWIDGET
# ------------------------------------------------------------------
::itcl::extendedclass Labeledwidget {

    component itcl_hull
    component itcl_interior
    protected component lwchildsite
    component label

    constructor {args} {}
    destructor {}

    option [list -disabledforeground disabledForeground \
	DisabledForeground] -default \#a3a3a3
    option [list -labelpos labelPos Position] -default w -configuremethod configLabelpos
    option [list -labelmargin labelMargin Margin] -default 2 -configuremethod configLabelmargin
    option [list -labeltext labelText Text] -default {} -configuremethod configLabeltext
    option [list -labelvariable labelVariable Variable] -default {} -configuremethod configLabelvariable
    option [list -labelbitmap labelBitmap Bitmap] -default {} -configuremethod configLabelbitmap
    option [list -labelimage labelImage Image] -default {} -configuremethod configLabelimage
    option [list -state state State] -default normal -configuremethod configState
    option [list -sticky sticky Sticky] -default nsew -configuremethod configSticky

    delegate option [list -labelfont labelFont Font] to label as -font

    protected variable _reposition ""  ;# non-null => _positionLabel pending

    private method _positionLabel {{when later}}

    protected method configLabelbitmap {option value}
    protected method configLabelimage {option value}
    protected method configLabelpos {option value}
    protected method configLabelmargin {option value}
    protected method configLabeltext {option value}
    protected method configLabelvariable {option value}
    protected method configState {option value}
    protected method configSticky {option value}

    public method childsite
    
    proc alignlabels {args} {}
}
    
# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Labeledwidget::constructor {args} {
    set win [createhull frame $this -class [info class]]
    #
    # Create a frame for the childsite widget.
    #
    setupcomponent lwchildsite using frame $win.lwchildsite
    
    #
    # Create label.
    #
    setupcomponent label using label $win.label
    keepcomponentoption label -background -cursor -foreground -labelfont
    
    #
    # Set the interior to be the childsite for derived classes.
    #
    set itcl_interior $lwchildsite

    #
    # Initialize the widget based on the command line options.
    #
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }

    # 
    # When idle, position the label.
    #
    _positionLabel
}

# ------------------------------------------------------------------
#                           DESTRUCTOR
# ------------------------------------------------------------------
::itcl::body Labeledwidget::destructor {} {
    if {$_reposition != ""} {
        after cancel $_reposition
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -labelpos
#
# Set the position of the label on the labeled widget.  The margin
# between the label and childsite comes along for the ride.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configLabelpos {option value} {
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
# OPTION: -labelmargin
#
# Specifies the distance between the widget and label.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configLabelmargin {option value} {
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
# OPTION: -labeltext
#
# Specifies the label text.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configLabeltext {option value} {
    if {[string length $label] > 0} {
        $label configure -text $value
    }
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
# OPTION: -labelvariable
#
# Specifies the label text variable.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configLabelvariable {option value} {
    if {[string length $label] > 0} {
        $label configure -textvariable $value
    }
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
# OPTION: -labelbitmap
#
# Specifies the label bitmap.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configLabelbitmap {option value} {
    if {[string length $label] > 0} {
        $label configure -bitmap $$value
    }
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
# OPTION: -labelimage
#
# Specifies the label image.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configLabelimage {option value} {
    if {[string length $label] > 0} {
        $label configure -image $value
    }
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
# OPTION: -sticky
#
# Specifies the stickyness of the child site. This option was added
# by James Bonfield (committed by Chad Smith 8/20/01).
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configSticky {option value} {
    set itcl_options($option) $value
    grid $lwchildsite -sticky $value
}

# ------------------------------------------------------------------
# OPTION: -state
#
# Specifies the state of the label.  
# ------------------------------------------------------------------
::itcl::body Labeledwidget::configState {option value} {
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Returns the path name of the child site widget.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::childsite {} {
    return $lwchildsite
}

# ------------------------------------------------------------------
# PROCEDURE: alignlabels widget ?widget ...?
#
# The alignlabels procedure takes a list of widgets derived from
# the Labeledwidget class and adjusts the label margin to align 
# the labels.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::alignlabels {args} {
    update
    set maxLabelWidth 0
    
    #
    # Verify that all the widgets are of type Labeledwidget and 
    # determine the size of the maximum length label string.
    #
    foreach iwid $args {
	set objcmd [itcl::find objects -isa Labeledwidget *::$iwid]

	if {$objcmd == ""} {
	    error "$iwid is not a \"Labeledwidget\""
	}
	
	set csWidth [winfo reqwidth $iwid.lwchildsite]
	set shellWidth [winfo reqwidth $iwid]
	    
	if {($shellWidth - $csWidth) > $maxLabelWidth} {
	    set maxLabelWidth [expr {$shellWidth - $csWidth}]
	}
    }
    
    #
    # Adjust the margins for the labels such that the child sites and
    # labels line up.
    #
    foreach iwid $args {
	set csWidth [winfo reqwidth $iwid.lwchildsite]
	set shellWidth [winfo reqwidth $iwid]
	
	set labelSize [expr {$shellWidth - $csWidth}]
	
	if {$maxLabelWidth > $labelSize} {
	    set objcmd [itcl::find objects -isa Labeledwidget *::$iwid]
	    set dist [expr {$maxLabelWidth - \
		    ($labelSize - [$objcmd cget -labelmargin])}]
	    
	    $objcmd configure -labelmargin $dist 
	}
    }	
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _positionLabel ?when?
#
# Packs the label and label margin.  If "when" is "now", the
# change is applied immediately.  If it is "later" or it is not
# specified, then the change is applied later, when the application
# is idle.
# ------------------------------------------------------------------
::itcl::body Labeledwidget::_positionLabel {{when later}} {
    if {$when == "later"} {
	if {$_reposition == ""} {
	    set _reposition [after idle [itcl::code $this _positionLabel now]]
	}
	return

    } elseif {$when != "now"} {
	error "bad option \"$when\": should be now or later"
    }

    #
    # If we have a label, be it text, bitmap, or image continue.
    #
    if {($itcl_options(-labeltext) != {}) || \
	($itcl_options(-labelbitmap) != {}) || \
	($itcl_options(-labelimage) != {}) || \
	($itcl_options(-labelvariable) != {})} {

	#
	# Set the foreground color based on the state.
	#
	if {[string length $label] > 0} {
	if {[info exists itcl_options(-state)]} {
	    switch -- $itcl_options(-state) {
	    disabled {
		if {[string length $label] > 0} {
	            $label configure \
		        -foreground $itcl_options(-disabledforeground)
	        }
	      }
	    normal {
		if {[string length $label] > 0} {
	            $label configure -foreground $itcl_options(-foreground)
		}
	      }
	    }
	}
        }
	set parent [winfo parent $lwchildsite]

	#
	# Switch on the label position option.  Using the grid,
	# adjust the row/column setting of the label, margin, and
	# and childsite.  The margin height/width is adjust based
        # on the orientation as well.  Finally, set the weights such
        # that the childsite takes the heat on expansion and shrinkage.
	#
	switch $itcl_options(-labelpos) {
	    nw -
	    n -
	    ne {
		if {[string length $label] > 0} {
		    grid $label -row 0 -column 0 \
			-sticky $itcl_options(-labelpos)
		grid $lwchildsite -row 2 -column 0 \
			-sticky $itcl_options(-sticky)
		
		grid rowconfigure $parent 0 -weight 0 -minsize 0
		grid rowconfigure $parent 1 -weight 0 -minsize \
			[winfo pixels $label \
			 $itcl_options(-labelmargin)]
		grid rowconfigure $parent 2 -weight 1 -minsize 0

		grid columnconfigure $parent 0 -weight 1 -minsize 0
		grid columnconfigure $parent 1 -weight 0 -minsize 0
		grid columnconfigure $parent 2 -weight 0 -minsize 0
		}
	    }

	    en -
	    e -
	    es {
		grid $lwchildsite -row 0 -column 0 \
			-sticky $itcl_options(-sticky)
		if {[string length $label] > 0} {
		    grid $label -row 0 -column 2 \
			-sticky $itcl_options(-labelpos)
	
		
		grid rowconfigure $parent 0 -weight 1 -minsize 0
		grid rowconfigure $parent 1 -weight 0 -minsize 0
		grid rowconfigure $parent 2 -weight 0 -minsize 0

		grid columnconfigure $parent 0 -weight 1 -minsize 0
		grid columnconfigure $parent 1 -weight 0 -minsize \
			[winfo pixels $label \
			$itcl_options(-labelmargin)]
		grid columnconfigure $parent 2 -weight 0 -minsize 0
		}
	    }
	    
	    se -
	    s -
	    sw {
		grid $lwchildsite -row 0 -column 0 \
			-sticky $itcl_options(-sticky)
		if {[string length $label] > 0} {
		    grid $label -row 2 -column 0 \
			-sticky $itcl_options(-labelpos)
		
		grid rowconfigure $parent 0 -weight 1 -minsize 0
		grid rowconfigure $parent 1 -weight 0 -minsize \
			[winfo pixels $label \
			$itcl_options(-labelmargin)]
		grid rowconfigure $parent 2 -weight 0 -minsize 0

		grid columnconfigure $parent 0 -weight 1 -minsize 0
		grid columnconfigure $parent 1 -weight 0 -minsize 0
		grid columnconfigure $parent 2 -weight 0 -minsize 0
		}
	    }
	    
	    wn -
	    w -
	    ws {
		grid $lwchildsite -row 0 -column 2 \
			-sticky $itcl_options(-sticky)
		if {[string length $label] > 0} {
		    grid $label -row 0 -column 0 \
			-sticky $itcl_options(-labelpos)
		
		grid rowconfigure $parent 0 -weight 1 -minsize 0
		grid rowconfigure $parent 1 -weight 0 -minsize 0
		grid rowconfigure $parent 2 -weight 0 -minsize 0

		grid columnconfigure $parent 0 -weight 0 -minsize 0
		grid columnconfigure $parent 1 -weight 0 -minsize \
			[winfo pixels $label \
			$itcl_options(-labelmargin)]
		grid columnconfigure $parent 2 -weight 1 -minsize 0
		}
	    }

	    default {
		error "bad labelpos option\
			\"$itcl_options(-labelpos)\": should be\
			nw, n, ne, sw, s, se, en, e, es, wn, w, or ws"
	    }
	}

    #
    # Else, neither the  label text, bitmap, or image have a value, so
    # forget them so they don't appear and manage only the childsite.
    #
    } else {
	if {[string length $label] > 0} {
	    grid forget $label
        }

	grid $lwchildsite -row 0 -column 0 -sticky $itcl_options(-sticky)

	set parent [winfo parent $lwchildsite]

	grid rowconfigure $parent 0 -weight 1 -minsize 0
	grid rowconfigure $parent 1 -weight 0 -minsize 0
	grid rowconfigure $parent 2 -weight 0 -minsize 0
	grid columnconfigure $parent 0 -weight 1 -minsize 0
	grid columnconfigure $parent 1 -weight 0 -minsize 0
	grid columnconfigure $parent 2 -weight 0 -minsize 0
    }

    #
    # Reset the resposition flag.
    #
    set _reposition ""
}

} ; # end ::itcl::widgets
