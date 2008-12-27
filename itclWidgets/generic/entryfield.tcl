#
# Entryfield
# ----------------------------------------------------------------------
# Implements an enhanced text entry widget using itcl-ng.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version

# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package
# written by:
#    Sue Yockey               E-mail: yockey@acm.org
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: entryfield.tcl,v 1.1.2.2 2008/12/27 19:44:12 wiede Exp $
# ======================================================================

package require itcl

namespace eval ::itcl::widgets {

#
# Provide a lowercase access method for the Entryfield class.
#
proc entryfield {pathName args} {
    uplevel ::itcl::widgets::Entryfield $pathName $args
}



::itcl::extendedclass Entryfield {
    inherit ::itcl::widgets::Labeledwidget

    component entry
    protected component efchildsite

    option -childsitepos childSitePos Position -default e -configuremethod configChildsite
    option -command command Command -default {}
    option -fixed fixed Fixed -default 0
    option -focuscommand focusCommand Command -default {}
    option -invalid invalid Command -default {bell}
    option -pasting pasting Behaviour -default 1 -configuremethod configPasting
    option -validate validate Command -default {}

    delegate method * to entry except [list configure cget childsite]
    delegate option -hullwidth to itcl_hull as -width
    delegate option -textfont to entry as -font
    delegate option -background to entry as -highlightbackground
    delegate option -textbackground to entry as -background
    delegate option * to entry except [list -childsitepos -command -fixed -focuscommand -invalid -pasting -validate -textfont]

    constructor {args} {}

    private method _peek {char}
    private method _checkLength {}

    protected method _focusCommand {}
    protected method _keyPress {char sym state}
    protected method configChildsite {option value}
    protected method configPasting {option value}

    public method childsite {}
    public method clear {}

} ; # end Entryfield

::itcl::body Entryfield::constructor {args} {
    setupcomponent entry using entry $itcl_interior.entry
    setupcomponent efchildsite using frame $itcl_interior.efchildsite
    pack $itcl_interior.entry $itcl_interior.efchildsite -side left
    set itcl_interior $efchildsite
    # Entryfield instance bindings.
    bind $entry <KeyPress> [::itcl::code $this _keyPress %a %K %s]
    bind $entry <FocusIn> [::itcl::code $this _focusCommand]
    if {[llength $args] > 0} {
        uplevel 0 configure $args
    }
}

::itcl::body Entryfield::_checkLength {} {
    if {$itcl_options(-fixed) != 0} {
        if {[catch {::selection get -selection CLIPBOARD} pending]} {
            # Nothing in the clipboard.  Check the primary selection.
            if {[catch {::selection get -selection PRIMARY} pending]} {
                # Nothing here either.  Goodbye.
                return
            }
        }
        set len [expr {[string length $pending] + [string length [get]]}]
        if {$len > $itcl_options(-fixed)} {
            uplevel #0 $itcl_options(-invalid)
            return -code break 0
        }
    }
}

::itcl::body Entryfield::_peek {char} {
    set str [get]
    set insertPos [index insert]
    set firstPart [string range $str 0 [expr {$insertPos - 1}]]
    set lastPart [string range $str $insertPos end]
    regsub -all {\\} "$char" {\\\\} char
    append rtnVal $firstPart $char $lastPart
    return $rtnVal
}

::itcl::body Entryfield::_focusCommand {} {
    uplevel #0 $itcl_options(-focuscommand)
}

::itcl::body Entryfield::_keyPress {char sym state} {
puts stderr "_keyPress!$char!$sym!$state!"
    # a Return key invokes the optionally specified command option.
    if {$sym eq "Return"} {
        if {$itcl_options(-command) eq ""} {
            # Allow <Return> to propagate to parent if the -command is
            # not defined
            return -code continue 1
        }
        uplevel #0 $itcl_options(-command)
        return -code break 1
    }
    #
    # Tabs, BackSpace, and Delete are passed on for other bindings.
    #
    if {($sym eq "Tab") || ($sym eq "BackSpace") || ($sym eq "Delete")} {
	    return -code continue 1
    }

    # 
    # Character is not printable or the state is greater than one which
    # means a modifier was used such as a control, meta key, or control
    # or meta key with numlock down.
    #
    #-----------------------------------------------------------
    # BUG FIX: csmith (Chad Smith: csmith@adc.com), 3/15/99
    #-----------------------------------------------------------
    # The following conditional used to hardcode specific state values, such
    # as "4" and "8".  These values are used to detect <Ctrl>, <Shift>, etc.
    # key combinations.  On the windows platform, the <Alt> key is state
    # 16, and on the unix platform, the <Alt> key is state 8.  All <Ctrl>
    # and <Alt> combinations should be masked out, regardless of the
    # <NumLock> or <CapsLock> status, and regardless of platform.
    #-----------------------------------------------------------
    set CTRL 4
    global tcl_platform
    if {$tcl_platform(platform) == "unix"} {
        set ALT 8
    } elseif {$tcl_platform(platform) == "windows"} {
        set ALT 16
    } else {
        # This is something other than UNIX or WINDOWS.  Default to the
        # old behavior (UNIX).
        set ALT 8
    }
    # Thanks to Rolf Schroedter for the following elegant conditional.  This
    # masks out all <Ctrl> and <Alt> key combinations.
    if {($char == "") || ($state & ($CTRL | $ALT))} {
        return -code continue 1
    }

    #
    # If the fixed length option is not zero, then verify that the
    # current length plus one will not exceed the limit.  If so then
    # invoke the invalid command procedure.
    #
    if {$itcl_options(-fixed) != 0} {
        if {[string length [get]] >= $itcl_options(-fixed)} {
            uplevel #0 $itcl_options(-invalid)
            return -code break 0
        }
    } 
    
    #
    # The validate option may contain a keyword (numeric, alphabetic),
    # the name of a procedure, or nothing.  The numeric and alphabetic
    # keywords engage typical base level checks.  If a command procedure
    # is specified, then invoke it with the object and character passed
    # as arguments.  If the validate procedure returns false, then the 
    # invalid procedure is called.
    #
    if {$itcl_options(-validate) != {}} {
        set cmd $itcl_options(-validate)

        regsub -all "%W" "$cmd" $itcl_hull cmd
        regsub -all "%P" "$cmd" [list [_peek $char]] cmd
        regsub -all "%S" "$cmd" [list [get]] cmd
        regsub -all "%c" "$cmd" [list $char] cmd
        regsub -all {\\} "$cmd" {\\\\} cmd

        set valid [uplevel #0 $cmd]
	
        if {($valid == "") || ([regexp 0|false|off|no $valid])} {
            uplevel #0 $itcl_options(-invalid)
            return -code break 0
        }
    }
    return -code continue 1
}

# ------------------------------------------------------------------
# OPTION: -pasting
#
# Allows the developer to enable and disable pasting into the entry
# component of the entryfield.  This is done to avoid potential stack
# dumps when using the -validate configuration option.  Plus, it's just
# a good idea to have complete control over what you allow the user
# to enter into the entryfield.
# ------------------------------------------------------------------
::itcl::body Entryfield::configPasting {option value} {
    set oldtags [bindtags $entry]
    if {[lindex $oldtags 0] ne "pastetag"} {
        bindtags $entry [linsert $oldtags 0 pastetag] 
    }

    if {($itcl_options(-pasting))} {
        bind pastetag <ButtonRelease-2> [itcl::code $this _checkLength]
        bind pastetag <Control-v> [itcl::code $this _checkLength]
        bind pastetag <Insert> [itcl::code $this _checkLength]
        bind pastetag <KeyPress> {}
    } else {
        bind pastetag <ButtonRelease-2> {break}
        bind pastetag <Control-v> {break}
        bind pastetag <Insert> {break}
        bind pastetag <KeyPress> {
            # Disable function keys > F9.
            if {[regexp {^F[1,2][0-9]+$} "%K"]} {
	            break
            }
        }
    }
}

::itcl::body Entryfield::configChildsite {option value} {
    set parent [winfo parent $entry]
    switch $value {
    n {
        grid $efchildsite -row 0 -column 0 -sticky ew
	grid $entry -row 1 -column 0 -sticky nsew
	grid rowconfigure $parent 0 -weight 0
	grid rowconfigure $parent 1 -weight 1
	grid columnconfigure $parent 0 -weight 1
	grid columnconfigure $parent 1 -weight 0
      }
    e {
        grid $efchildsite -row 0 -column 1 -sticky ns
	grid $entry -row 0 -column 0 -sticky nsew
	grid rowconfigure $parent 0 -weight 1
	grid rowconfigure $parent 1 -weight 0
	grid columnconfigure $parent 0 -weight 1
	grid columnconfigure $parent 1 -weight 0
      }
    s {
        grid $efchildsite -row 1 -column 0 -sticky ew
	grid $entry -row 0 -column 0 -sticky nsew
	grid rowconfigure $parent 0 -weight 1
	grid rowconfigure $parent 1 -weight 0
	grid columnconfigure $parent 0 -weight 1
	grid columnconfigure $parent 1 -weight 0
      }
    w {
        grid $efchildsite -row 0 -column 0 -sticky ns
	grid $entry -row 0 -column 1 -sticky nsew
	grid rowconfigure $parent 0 -weight 1
	grid rowconfigure $parent 1 -weight 0
	grid columnconfigure $parent 0 -weight 0
	grid columnconfigure $parent 1 -weight 1
      }
    default {
        error "bad childsite option \"$itcl_options(-childsitepos)\":
	        should be n, e, s, or w"
      }
    }
}

::itcl::body Entryfield::childsite {} {
    return $efchildsite
}

::itcl::body Entryfield::clear {} {
    delete 0 end
    icursor 0
}

} ; # end ::itcl::widgets

