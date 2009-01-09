#
# Shell
# ----------------------------------------------------------------------
# This class is implements a shell which is a top level widget
# giving a childsite and providing activate, deactivate, and center 
# methods.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Shell
# written by:
#  AUTHOR: Mark L. Ulferts              EMAIL: mulferts@austin.dsccc.com
#          Kris Raney                   EMAIL: kraney@spd.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: shell.tcl,v 1.1.2.2 2009/01/09 21:40:52 wiede Exp $
# ======================================================================

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Shell class.
# 
proc ::itcl::widgets::shell {pathName args} {
    uplevel ::itcl::widgets::Shell $pathName $args
}

# ------------------------------------------------------------------
#                            SHELL
# ------------------------------------------------------------------
::itcl::extendedclass Shell {
    component itcl_hull
    component itcl_interior
    protected component shellchildsite
 
    option [list -master master Window] -default "" -configuremethod configMaster
    option [list -modality modality Modality] -default none -configuremethod configModality
    option [list -padx padX Pad] -default 0-configuremethod configPadx
    option [list -pady padY Pad] -default 0-configuremethod configPady
    option [list -width width Width] -default 0-configuremethod configWidth
    option [list -height height Height] -default 0-configuremethod configHeight
 
    private variable _busied {}     ;# List of busied top level widgets.
 
    protected variable _result {}     ;# Resultant value for modal activation.

    common grabstack {}
    common _wait

    constructor {args} {}
 
    protected method configMaster {option value}
    protected method configModality {option value}
    protected method configPadx {option value}
    protected method configPady {option value}
    protected method configWidth {option value}
    protected method configHeight {option value}

    public method childsite {}
    public method activate {}
    public method deactivate {args}
    public method center {{widget {}}}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Shell::constructor {args} {
    createhull toplevel $this -class [info class]
# FIXME    itcl_options add hull.width hull.height
    #
    # Maintain a withdrawn state until activated.  
    #
    wm withdraw $win
    #
    # Create the user child site
    #
    setupcomponent shellchildsite using frame $itcl_interior.shellchildsite
    pack $shellchildsite -fill both -expand yes
    #
    # Set the itk_interior variable to be the childsite for derived 
    # classes.
    #
    set itcl_interior $shellchildsite
    #
    # Bind the window manager delete protocol to deactivation of the 
    # widget.  This can be overridden by the user via the execution 
    # of a similar command outside the class.
    #
    wm protocol $win WM_DELETE_WINDOW [itcl::code $this deactivate]
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------
 
# ------------------------------------------------------------------
# OPTION: -master
#
# Specifies the master window for the shell.  The window manager is
# informed that the shell is a transient window whose master is
# -masterwindow.
# ------------------------------------------------------------------
::itcl::body Shell::configMaster {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -modality
#
# Specify the modality of the dialog.
# ------------------------------------------------------------------
::itcl::body Shell::configModality {option value} {
    switch $value {
    none -
    application -
    global {
      }
    default {
        error "bad modality option \"$value\":\
                should be none, application, or global"
      }
    }
    set itcl_options($option) $value
}
 
# ------------------------------------------------------------------
# OPTION: -padx
#
# Specifies a padding distance for the childsite in the X-direction.
# ------------------------------------------------------------------
::itcl::body Shell::configPadx {option value} {
    pack config $shellchildsite -padx $value
    set itcl_options($option) $value
}
 
# ------------------------------------------------------------------
# OPTION: -pady
#
# Specifies a padding distance for the childsite in the Y-direction.
# ------------------------------------------------------------------
::itcl::body Shell::configPady {option value} {
    pack config $shellchildsite -pady $value
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -width
#
# Specifies the width of the shell.  The value may be specified in 
# any of the forms acceptable to Tk_GetPixels.  A value of zero 
# causes the width to be adjusted to the required value based on 
# the size requests of the components placed in the childsite.  
# Otherwise, the width is fixed.
# ------------------------------------------------------------------
::itcl::body Shell::configWidth {option value} {
    #
    # The width option was added to the hull in the constructor.
    # So, any width value given is passed automatically to the
    # hull.  All we have to do is play with the propagation.
    #
    if {$value != 0} {
        pack propagate $win no
    } else {
        pack propagate $win yes
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -height
#
# Specifies the height of the shell.  The value may be specified in 
# any of the forms acceptable to Tk_GetPixels.  A value of zero 
# causes the height to be adjusted to the required value based on 
# the size requests of the components placed in the childsite.
# Otherwise, the height is fixed.
# ------------------------------------------------------------------
::itcl::body Shell::configHeight {option value} {
    #
    # The height option was added to the hull in the constructor.
    # So, any height value given is passed automatically to the
    # hull.  All we have to do is play with the propagation.
    #
    if {$value != 0} {
        pack propagate $win no
    } else {
        pack propagate $win yes
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Return the pathname of the user accessible area.
# ------------------------------------------------------------------
::itcl::body Shell::childsite {} {
    return $shellchildsite
}
 
# ------------------------------------------------------------------
# METHOD: activate
#
# Display the dialog and wait based on the modality.  For application
# and global modal activations, perform a grab operation, and wait
# for the result.  The result may be returned via an argument to the
# "deactivate" method.
# ------------------------------------------------------------------
::itcl::body Shell::activate {} {

    if {[winfo ismapped $win]} {
        raise $win
    return
    }
    
    if {($itcl_options(-master) != {}) && \
            [winfo exists $itcl_options(-master)]} {
        wm transient $win $itcl_options(-master)
    } 
    set _wait($this) 0
    raise $win
    wm deiconify $win
    tkwait visibility $win
    # For some mysterious reason, Tk sometimes returns too late from the
    # "tkwait visibility", i.e. after the "deactivate" method was invoked,
    # i.e. after the dialog window already disappeared. This would lead to
    # an infinite vwait on _wait($this) further on. Trap this case.
    # See also 2002-03-15 message to the Tcl/Tk newsgroup.
    # Remark that tests show that if "raise" is given *after* "deiconify" 
    # (see above), "tkwait visibility" always returns duly on time.....
    if {![winfo ismapped $win]} {
	# means "deactivate" went already through the grab-release stuff.
	return $_result
    }
    # Need to flush the event loop.  This line added as a result of
    # SF ticket #227885.
    update idletasks
    if {$itcl_options(-modality) eq "application"} {
        if {$grabstack != {}} {
            grab release [lindex $grabstack end]
        }

        set err 1
        while {$err == 1} {
            set err [catch [list grab $win]]
            if {$err == 1} {
                after 1000
            }
        }
        lappend grabstack [list grab $win]
        tkwait variable [itcl::scope _wait($this)]
        return $_result
        
    } elseif {$itcl_options(-modality) eq "global" }  {
        if {$grabstack ne {}} {
            grab release [lindex $grabstack end]
        }
        set err 1
        while {$err == 1} {
            set err [catch [list grab -global $win]]
            if {$err == 1} {
                after 1000
            }
        }
        lappend grabstack [list grab -global $win]
        tkwait variable [itcl::scope _wait($this)]
        return $_result
    }
}
 
# ------------------------------------------------------------------
# METHOD: deactivate
#
# Deactivate the display of the dialog.  The method takes an optional
# argument to passed to the "activate" method which returns the value.
# This is only effective for application and global modal dialogs.
# ------------------------------------------------------------------
::itcl::body Shell::deactivate {args} {
    if {! [winfo ismapped $win]} {
        return
    }
    if {$itcl_options(-modality) eq "none"} {
        wm withdraw $win
    } elseif {$itcl_options(-modality) eq "application"} {
        grab release $win
        if {$grabstack != {}} {
            if {[set grabstack [lreplace $grabstack end end]] != {}} {
                eval [lindex $grabstack end]
            }
        }
        wm withdraw $win
    } elseif {$itcl_options(-modality) == "global"} {
        grab release $win
        if {$grabstack != {}} {
            if {[set grabstack [lreplace $grabstack end end]] != {}} {
                uplevel 0 [lindex $grabstack end]
            }
        }
        wm withdraw $win
    }
    if {[llength $args]} {
        set _result $args
    } else {
        set _result {}
    }
    set _wait($this) 1
    return
}
 
# ------------------------------------------------------------------
# METHOD: center
#
# Centers the dialog with respect to another widget or the screen
# as a whole.
# ------------------------------------------------------------------
::itcl::body Shell::center {{widget {}}} {
    update idletasks
    set w [winfo width $win]
    set h [winfo height $win]
    set sh [winfo screenheight $win]     ;# display screen's height/width
    set sw [winfo screenwidth $win]
    #
    # User can request it centered with respect to root by passing in '{}'
    #
    if {$widget eq ""} {
        set reqX [expr {($sw-$w)/2}]
        set reqY [expr {($sh-$h)/2}]
    } else {
        set wfudge 5      ;# wm width fudge factor
        set hfudge 20     ;# wm height fudge factor
        set widgetW [winfo width $widget]
        set widgetH [winfo height $widget]
        set reqX [expr {[winfo rootx $widget]+($widgetW-($widgetW/2))-($w/2)}]
        set reqY [expr {[winfo rooty $widget]+($widgetH-($widgetH/2))-($h/2)}]
        #
        # Adjust for errors - if too long or too tall
        #
        if {($reqX + $w + $wfudge) > $sw} {
	    set reqX [expr {$sw - $w - $wfudge}]
	}
        if {$reqX < $wfudge} {
	    set reqX $wfudge
	}
        if {($reqY + $h + $hfudge) > $sh} {
	    set reqY [expr {$sh - $h - $hfudge}]
	}
        if {$reqY < $hfudge} {
	    set reqY $hfudge
	}
    } 
    wm geometry $win +$reqX+$reqY
}

} ; # end ::itcl::widgets
