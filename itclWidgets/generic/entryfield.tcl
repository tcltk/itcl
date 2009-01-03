#
# Entryfield
# ----------------------------------------------------------------------
# Implements an enhanced text entry widget using itcl-ng.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Entryfield
# written by:
#    Sue Yockey               E-mail: yockey@acm.org
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: entryfield.tcl,v 1.1.2.7 2009/01/03 17:30:31 wiede Exp $
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
    option [list -command command Command] -default {}
    option [list -fixed fixed Fixed] -default 0 -configuremethod configFixed
    option [list -focuscommand focusCommand Command] -default {}
    option [list -invalid invalid Command] -default {bell}
    option [list -pasting pasting Behaviour] -default 1 -configuremethod configPasting
    option [list -validate validate Command] -default {} -configuremethod configValidate

    delegate method * to entry except [list configure cget childsite]
    delegate option [list -textfont textFont Font] to entry as -font
    delegate option [list -background background Background] to entry as -highlightbackground
    delegate option [list -textbackground textBackground Background] to entry as -background

    constructor {args} {}

    private method _peek {char}
    private method _checkLength {}

    protected method _focusCommand {}
    protected method _keyPress {char sym state}
    protected method configChildsite {option value}
    protected method configPasting {option value}
    protected method configValidate {option value}
    protected method configFixed {option value}

    public method childsite {}
    public method clear {}

    protected proc numeric {char}
    protected proc integer {string}
    protected proc alphabetic {char}
    protected proc alphanumeric {char}
    protected proc hexadecimal {string}
    protected proc real {string}
} ; # end Entryfield

::itcl::body Entryfield::constructor {args} {
    setupcomponent entry using entry $itcl_interior.entry
    keepcomponentoption entry -borderwidth -cursor -exportselection \
            -foreground -highlightcolor \
	    -highlightthickness -insertbackground -insertborderwidth \
	    -insertofftime -insertontime -insertwidth -justify \
	    -relief -selectbackground -selectborderwidth \
	    -selectforeground -show -state -textvariable -width

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
# OPTION: -fixed
#
# Restrict entry to 0 (unlimited) chars.  The value is the maximum 
# number of chars the user may type into the field, regardles of 
# field width, i.e. the field width may be 20, but the user will 
# only be able to type -fixed number of characters into it (or 
# unlimited if -fixed = 0).
# ------------------------------------------------------------------
::itcl::body Entryfield::configFixed {option value} {
    if {[regexp {[^0-9]} $value] || ($value < 0)} {
	error "bad fixed option \"$value\", should be positive integer"
    }
    set itcl_options($option) $value
    return 1
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
        error "bad childsite option \"$value\": should be n, e, s, or w"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -validate
#
# Specify a command to be executed for the validation of Entryfields.
# ------------------------------------------------------------------
::itcl::body Entryfield::configValidate {option value} {
    switch $value {
    {} {
        set itcl_options($option) {}
      }
    numeric {
        set itcl_options($option) "::itcl::widgets::Entryfield::numeric %c"
      }
    integer {
        set itcl_options($option) "::itcl::widgets::Entryfield::integer %P"
      }
    hexadecimal {
        set itcl_options($option) "::itcl::widgets::Entryfield::hexadecimal %P"
      }
    real {
        set itcl_options($option) "::itcl::widgets::Entryfield::real %P"
      }
    alphabetic {
        set itcl_options($option) "::itcl::widgets::Entryfield::alphabetic %c"
      }
    alphanumeric {
        set itcl_options($option) "::itcl::widgets::Entryfield::alphanumeric %c"
      }
    default {
        set itcl_options($option) $value
      }
    }
}

::itcl::body Entryfield::childsite {} {
    return $efchildsite
}

::itcl::body Entryfield::clear {} {
    $entry delete 0 end
    $entry icursor 0
}

# ------------------------------------------------------------------
# PROCEDURE: numeric char
#
# The numeric procedure validates character input for a given 
# Entryfield to be numeric and returns the result.
# ------------------------------------------------------------------
::itcl::body Entryfield::numeric {char} {
    return [regexp {[0-9]} $char]
}

# ------------------------------------------------------------------
# PROCEDURE: integer string
#
# The integer procedure validates character input for a given 
# Entryfield to be integer and returns the result.
# ------------------------------------------------------------------
::itcl::body Entryfield::integer {string} {
    return [regexp {^[-+]?[0-9]*$} $string]
}

# ------------------------------------------------------------------
# PROCEDURE: alphabetic char
#
# The alphabetic procedure validates character input for a given 
# Entryfield to be alphabetic and returns the result.
# ------------------------------------------------------------------
::itcl::body Entryfield::alphabetic {char} {
    return [regexp -nocase {[a-z]} $char]
}

# ------------------------------------------------------------------
# PROCEDURE: alphanumeric char
#
# The alphanumeric procedure validates character input for a given 
# Entryfield to be alphanumeric and returns the result.
# ------------------------------------------------------------------
::itcl::body Entryfield::alphanumeric {char} {
    return [regexp -nocase {[0-9a-z]} $char]
}

# ------------------------------------------------------------------
# PROCEDURE: hexadecimal string
#
# The hexadecimal procedure validates character input for a given 
# Entryfield to be hexadecimal and returns the result.
# ------------------------------------------------------------------
::itcl::body Entryfield::hexadecimal {string} {
    return [regexp {^(0x)?[0-9a-fA-F]*$} $string]
}

# ------------------------------------------------------------------
# PROCEDURE: real string
#
# The real procedure validates character input for a given Entryfield
# to be real and returns the result.
# ------------------------------------------------------------------
::itcl::body Entryfield::real {string} {
    return [regexp {^[-+]?[0-9]*\.?[0-9]*([0-9]\.?[eE][-+]?[0-9]*)?$} $string]
}

} ; # end ::itcl::widgets

