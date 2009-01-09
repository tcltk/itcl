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
#   @(#) $Id: shell.tcl,v 1.1.2.1 2009/01/09 20:55:26 wiede Exp $
# ======================================================================

    keep -background -cursor -modality 
 
namespace eval ::itcl::widgets {

# ------------------------------------------------------------------
#                            SHELL
# ------------------------------------------------------------------
::itcl::class ::itcl::widgets::Shell {
    inherit itk::Toplevel
 
    constructor {args} {}
 
    itcl_options define -master master Window "" 
    itcl_options define -modality modality Modality none
    itcl_options define -padx padX Pad 0
    itcl_options define -pady padY Pad 0
    itcl_options define -width width Width 0
    itcl_options define -height height Height 0
 
    public method childsite {}
    public method activate {}
    public method deactivate {args}
    public method center {{widget {}}}
 
    protected variable _result {}     ;# Resultant value for modal activation.

    private variable _busied {}     ;# List of busied top level widgets.

    common grabstack {}
    common _wait
}

#
# Provide a lowercased access method for the Shell class.
# 
proc ::::itcl::widgets::shell {pathName args} {
    uplevel ::::itcl::widgets::Shell $pathName $args
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Shell::constructor {args} {
    itcl_options add hull.width hull.height

    #
    # Maintain a withdrawn state until activated.  
    #
    wm withdraw $itk_component(hull)
    
    #
    # Create the user child site
    #
    itk_component add -protected shellchildsite {
        frame $itk_interior.shellchildsite
    } 
    pack $itk_component(shellchildsite) -fill both -expand yes

    #
    # Set the itk_interior variable to be the childsite for derived 
    # classes.
    #
    set itk_interior $itk_component(shellchildsite)

    #
    # Bind the window manager delete protocol to deactivation of the 
    # widget.  This can be overridden by the user via the execution 
    # of a similar command outside the class.
    #
    wm protocol $itk_component(hull) WM_DELETE_WINDOW [itcl::code $this deactivate]
    
    #
    # Initialize the widget based on the command line options.
    #
    eval itk_initialize $args
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
::itcl::configbody ::itcl::widgets::Shell::master {}

# ------------------------------------------------------------------
# OPTION: -modality
#
# Specify the modality of the dialog.
# ------------------------------------------------------------------
::itcl::configbody ::itcl::widgets::Shell::modality {
    switch $itcl_options(-modality) {
        none -
        application -
        global {
        }
        
        default {
            error "bad modality option \"$itcl_options(-modality)\":\
                    should be none, application, or global"
        }
    }
}
 
# ------------------------------------------------------------------
# OPTION: -padx
#
# Specifies a padding distance for the childsite in the X-direction.
# ------------------------------------------------------------------
::itcl::configbody ::itcl::widgets::Shell::padx {
    pack config $itk_component(shellchildsite) -padx $itcl_options(-padx)
}
 
# ------------------------------------------------------------------
# OPTION: -pady
#
# Specifies a padding distance for the childsite in the Y-direction.
# ------------------------------------------------------------------
::itcl::configbody ::itcl::widgets::Shell::pady {
    pack config $itk_component(shellchildsite) -pady $itcl_options(-pady)
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
::itcl::configbody ::itcl::widgets::Shell::width {
    #
    # The width option was added to the hull in the constructor.
    # So, any width value given is passed automatically to the
    # hull.  All we have to do is play with the propagation.
    #
    if {$itcl_options(-width) != 0} {
    pack propagate $itk_component(hull) no
    } else {
    pack propagate $itk_component(hull) yes
    }
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
::itcl::configbody ::itcl::widgets::Shell::height {
    #
    # The height option was added to the hull in the constructor.
    # So, any height value given is passed automatically to the
    # hull.  All we have to do is play with the propagation.
    #
    if {$itcl_options(-height) != 0} {
    pack propagate $itk_component(hull) no
    } else {
    pack propagate $itk_component(hull) yes
    }
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: childsite
#
# Return the pathname of the user accessible area.
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Shell::childsite {} {
    return $itk_component(shellchildsite)
}
 
# ------------------------------------------------------------------
# METHOD: activate
#
# Display the dialog and wait based on the modality.  For application
# and global modal activations, perform a grab operation, and wait
# for the result.  The result may be returned via an argument to the
# "deactivate" method.
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Shell::activate {} {

    if {[winfo ismapped $itk_component(hull)]} {
        raise $itk_component(hull)
    return
    }
    
    if {($itcl_options(-master) != {}) && \
        [winfo exists $itcl_options(-master)]} {
    wm transient $itk_component(hull) $itcl_options(-master)
    } 

    set _wait($this) 0
    raise $itk_component(hull)
    wm deiconify $itk_component(hull)
    tkwait visibility $itk_component(hull)
    # For some mysterious reason, Tk sometimes returns too late from the
    # "tkwait visibility", i.e. after the "deactivate" method was invoked,
    # i.e. after the dialog window already disappeared. This would lead to
    # an infinite vwait on _wait($this) further on. Trap this case.
    # See also 2002-03-15 message to the Tcl/Tk newsgroup.
    # Remark that tests show that if "raise" is given *after* "deiconify" 
    # (see above), "tkwait visibility" always returns duly on time.....
    if {![winfo ismapped $itk_component(hull)]} {
	# means "deactivate" went already through the grab-release stuff.
	return $_result
    }

    # Need to flush the event loop.  This line added as a result of
    # SF ticket #227885.
    update idletasks
    
    if {$itcl_options(-modality) == "application"} {
        if {$grabstack != {}} {
            grab release [lindex $grabstack end]
        }

    set err 1
    while {$err == 1} {
        set err [catch [list grab $itk_component(hull)]]
        if {$err == 1} {
        after 1000
        }
    }

        lappend grabstack [list grab $itk_component(hull)]
        
        tkwait variable [itcl::scope _wait($this)]
        return $_result
        
    } elseif {$itcl_options(-modality) == "global" }  {
        if {$grabstack != {}} {
            grab release [lindex $grabstack end]
        }

    set err 1
    while {$err == 1} {
        set err [catch [list grab -global $itk_component(hull)]]
        if {$err == 1} {
        after 1000
        }
    }

        lappend grabstack [list grab -global $itk_component(hull)]
 
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
::itcl::body ::itcl::widgets::Shell::deactivate {args} {

    if {! [winfo ismapped $itk_component(hull)]} {
        return
    }
    
    if {$itcl_options(-modality) == "none"} {
        wm withdraw $itk_component(hull)
    } elseif {$itcl_options(-modality) == "application"} {
        grab release $itk_component(hull)
        if {$grabstack != {}} {
            if {[set grabstack [lreplace $grabstack end end]] != {}} {
                eval [lindex $grabstack end]
            }
        }
 
        wm withdraw $itk_component(hull)
        
    } elseif {$itcl_options(-modality) == "global"} {
        grab release $itk_component(hull)
        if {$grabstack != {}} {
            if {[set grabstack [lreplace $grabstack end end]] != {}} {
                eval [lindex $grabstack end]
            }
        }
 
        wm withdraw $itk_component(hull)
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
::itcl::body ::itcl::widgets::Shell::center {{widget {}}} {
    update idletasks
 
    set hull $itk_component(hull)
    set w [winfo width $hull]
    set h [winfo height $hull]
    set sh [winfo screenheight $hull]     ;# display screen's height/width
    set sw [winfo screenwidth $hull]
 
    #
    # User can request it centered with respect to root by passing in '{}'
    #
    if { $widget == "" } {
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
        if { ($reqX+$w+$wfudge) > $sw } { set reqX [expr {$sw-$w-$wfudge}] }
        if { $reqX < $wfudge } { set reqX $wfudge }
        if { ($reqY+$h+$hfudge) > $sh } { set reqY [expr {$sh-$h-$hfudge}] }
        if { $reqY < $hfudge } { set reqY $hfudge }
    } 
 
    wm geometry $hull +$reqX+$reqY
}

} ; # end ::itcl::widgets
