#
# Datefield
# ----------------------------------------------------------------------
# Implements a date entry field with adjustable built-in intelligence
# levels.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the ::itcl::widgets package Datefield
# written by:
#    Mark L. Ulferts          E-mail: mulferts@austin.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: datefield.tcl,v 1.1.2.1 2009/01/10 17:54:47 wiede Exp $
# ======================================================================

#
# Use option database to override default resources of base classes.
#
option add *Datefield.justify center widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the datefield class.
# 
proc ::itcl::widgets::datefield {pathName args} {
    uplevel ::itcl::widgets::Datefield $pathName $args
}

# ------------------------------------------------------------------
#                               DATEFIELD
# ------------------------------------------------------------------
::itcl::extendedclass ::itcl::widgets::Datefield {
    inherit ::itcl::widgets::Labeledwidget 
    
    component entry
    protected component dfchildsite
    component date

    option [list -childsitepos childSitePos Position] -default e -configuremethod configChildsitepos
    option [list -command command Command] -default {} -configuremethod configCommand
    option [list -iq iq Iq] -default high -configuremethod configIq
    option [list -gmt gmt GMT] -default no -configuremethod configGmt
    option [list -int int DateFormat] -default no -configuremethod configInt

    delegate option [list -textfont textFont Font] to entry as -font
    delegate option [list -background background Background] to entry as -highlightbackground
   delegate option [list -textbackground textBackground Background] to entry as -background

    protected variable _cfield "month"
    protected variable _fields {month day year}
    
    constructor {args} {}

    protected method _backward {}
    protected method _focusIn {}
    protected method _forward {}
    protected method _keyPress {char sym state}
    protected method _lastDay {month year}
    protected method _moveField {direction}
    protected method _setField {field}
    protected method _whichField {}
    protected method configChildsitepos {otion value}
    protected method configCommand {otion value}
    protected method configIq {otion value}
    protected method configGmt {otion value}
    protected method configInt {otion value}

    public method get {{format "-string"}}
    public method isvalid {}
    public method show {{date now}}
}


# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Datefield::constructor {args} {
# FIXME    $win configure -borderwidth 0
    #   
    # Create an entry field for entering the date.
    #   
    setupcomponent date using entry $itcl_interior.date -width 10
    keepcomponentoption entry -borderwidth -cursor -exportselection \
        -foreground -highlightcolor -highlightthickness \
        -insertbackground -justify -relief -state
    #
    # Create the child site widget.
    #
    setupcomponent dfchildsite using frame $itcl_interior.dfchildsite
    set itcl_interior $dfchildsite
    #
    # Add datefield event bindings for focus in and keypress events.
    #
    bind $date <FocusIn> [itcl::code $this _focusIn]
    bind $date <KeyPress> [itcl::code $this _keyPress %A %K %s]
    #
    # Disable some mouse button event bindings:
    #   Button Motion
    #   Double-Clicks
    #   Triple-Clicks
    #   Button2
    #
    bind $date <Button1-Motion>  break
    bind $date <Button2-Motion>  break
    bind $date <Double-Button>   break
    bind $date <Triple-Button>   break
    bind $date <2>       break
    #
    # Initialize the widget based on the command line options.
    #
    uplevel 0 itcl_initoptions $args
    #
    # Initialize the date to the current date.
    #
    $date delete 0 end
    show now
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -childsitepos
#
# Specifies the position of the child site in the widget.  Valid
# locations are n, s, e, and w.
# ------------------------------------------------------------------
::itcl::body Datefield::configChildsitepos {option value} {
    set parent [winfo parent $date]
    switch $value {
    n {
        grid $dfchildsite -row 0 -column 0 -sticky ew
        grid $date -row 1 -column 0 -sticky nsew
        grid rowconfigure $parent 0 -weight 0
        grid rowconfigure $parent 1 -weight 1
        grid columnconfigure $parent 0 -weight 1
        grid columnconfigure $parent 1 -weight 0
      }
    e {
        grid $dfchildsite -row 0 -column 1 -sticky ns
        grid $date -row 0 -column 0 -sticky nsew
        grid rowconfigure $parent 0 -weight 1
        grid rowconfigure $parent 1 -weight 0
        grid columnconfigure $parent 0 -weight 1
        grid columnconfigure $parent 1 -weight 0
      }
    s {
        grid $dfchildsite -row 1 -column 0 -sticky ew
        grid $date -row 0 -column 0 -sticky nsew
        grid rowconfigure $parent 0 -weight 1
        grid rowconfigure $parent 1 -weight 0
        grid columnconfigure $parent 0 -weight 1
        grid columnconfigure $parent 1 -weight 0
      }
    w {
        grid $dfchildsite -row 0 -column 0 -sticky ns
        grid $date -row 0 -column 1 -sticky nsew
        grid rowconfigure $parent 0 -weight 1
        grid rowconfigure $parent 1 -weight 0
        grid columnconfigure $parent 0 -weight 0
        grid columnconfigure $parent 1 -weight 1
      }
    default {
        error "bad childsite option\
            \"$value\":\
            should be n, e, s, or w"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -command
#
# Command invoked upon detection of return key press event.
# ------------------------------------------------------------------
::itcl::body Datefield::configCommand {option value} {
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -iq
#
# Specifies the level of intelligence to be shown in the actions
# taken by the date field during the processing of keypress events.
# Valid settings include high, average, and low.  With a high iq,
# the date prevents the user from typing in an invalid date.  For 
# example, if the current date is 05/31/1997 and the user changes
# the month to 04, then the day will be instantly modified for them 
# to be 30.  In addition, leap years are fully taken into account.
# With average iq, the month is limited to the values of 01-12, but
# it is possible to type in an invalid day.  A setting of low iq
# instructs the widget to do no validity checking at all during
# date entry.  With both average and low iq levels, it is assumed
# that the validity will be determined at a later time using the
# date's isvalid command.
# ------------------------------------------------------------------
::itcl::body Datefield::configIq {option value} {
    switch $value {
    high -
    average -
    low {
      }
    default {
        error "bad iq option \"$value\":\
                   should be high, average or low"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
# OPTION: -int 
#
# Added by Mark Alston 2001/10/21
#
# Allows for the use of dates in "international" format: YYYY-MM-DD.
# It must be a boolean value.
# ------------------------------------------------------------------
::itcl::body Datefield::configInt {option value} { 
    switch $value {
    1 -
    yes -
    true -
    on {
        set _cfield "year"
        set _fields {year month day}
      }
    0 -
    no -
    false -
    off {
      }
    default {
        error "bad int option \"$value\": should be boolean"
      }
    }
    set itcl_options($option) $value
    show [get]
}

# ------------------------------------------------------------------
# OPTION: -gmt
#
# This option is used for GMT time.  Must be a boolean value.
# ------------------------------------------------------------------
::itcl::body Datefield::configGmt {option value} {
    switch $value {
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
      error "bad gmt option \"$itcl_options(-gmt)\": should be boolean"
      }
    }
    set itcl_options($option) $value
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# PUBLIC METHOD: get ?format?
#
# Return the current contents of the datefield in one of two formats
# string or as an integer clock value using the -string and -clicks
# options respectively.  The default is by string.  Reference the 
# clock command for more information on obtaining dates and their 
# formats.
# ------------------------------------------------------------------
::itcl::body Datefield::get {{format "-string"}} {
    set datestr [$date get]
    switch -- $format {
    "-string" {
        return $datestr
      }
    "-clicks" {
        return [clock scan $datestr]
      }
    default {
        error "bad format option \"$format\":\
                   should be -string or -clicks"
      }
    }
}

# ------------------------------------------------------------------
# PUBLIC METHOD: show date
#
# Changes the currently displayed date to be that of the date 
# argument.  The date may be specified either as a string or an
# integer clock value.  Reference the clock command for more 
# information on obtaining dates and their formats.
# ------------------------------------------------------------------
::itcl::body Datefield::show {{fdate "now"}} {
    $date delete 0 end
    if {$itcl_options(-int)} {
        set format {%Y-%m-%d}
    } else {
        set format {%m/%d/%Y}
    }
    if {$fdate == "now"} {
        set seconds [::clock seconds]
        $date insert end \
            [clock format $seconds -format "$format" -gmt $itcl_options(-gmt)]
    } elseif { $itcl_options(-iq) ne "low" } {
        if {[catch {::clock format $fdate}] == 0} {
            set seconds $fdate
        } elseif {[catch {set seconds [::clock scan $fdate -gmt \
                $itcl_options(-gmt)]}] != 0} {
            error "bad date: \"$date\", must be a valid date\
            string, clock clicks value or the keyword now"
        }
        $date insert end \
            [clock format $seconds -format "$format" -gmt $itcl_options(-gmt)]
    } else {
        # Note that it doesn't matter what -int is set to.
        $date insert end $fdate
    }
    if {$itcl_options(-int)} {
        _setField year
    } else {
        _setField month
    }
    return
}

# ------------------------------------------------------------------
# PUBLIC METHOD: isvalid
#
# Returns a boolean indication of the validity of the currently
# displayed date value.  For example, 3/3/1960 is valid whereas
# 02/29/1997 is invalid.
# ------------------------------------------------------------------
::itcl::body Datefield::isvalid {} {
    if {[catch {clock scan [$date get]}] != 0} {
        return 0
    } else {
        return 1
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _focusIn
#
# This method is bound to the <FocusIn> event.  It resets the 
# insert cursor and field settings to be back to their last known
# positions.
# ------------------------------------------------------------------
::itcl::body Datefield::_focusIn {} {
    _setField $_cfield
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _keyPress 
#
# This method is the workhorse of the class.  It is bound to the
# <KeyPress> event and controls the processing of all key strokes.
# ------------------------------------------------------------------
::itcl::body Datefield::_keyPress {char sym state} {
    #
    #  Determine which field we are in currently.  This is needed
    # since the user may have moved to this position via a mouse
    # selection and so it would not be in the position we last 
    # knew it to be.
    #
    _whichField 
    #
    # If we are using an international date the split char is "-" 
    # otherwise it is "/".
    #
    if {$itcl_options(-int)} {
        set split_char "-"
    } else {
        set split_char "/"
    }
    #
    # Set up a few basic variables we'll be needing throughout the
    # rest of the method such as the position of the insert cursor
    # and the currently displayed day, month, and year.
    #
    set icursor [$date index insert]
    set splist [split [$date get] "$split_char"]
    # A bunch of added variables to allow for the use of int dates
    if {$itcl_options(-int)} {
        set order {year month day}
        set year [lindex $splist 0]
        set month [lindex $splist 1]
        set day [lindex $splist 2]
        set year_start_pos 0
        set year_second_pos 1
        set year_third_pos 2
        set year_fourth_pos 3
        set year_end_pos 4
        set month_start_pos 5
        set month_second_pos 6
        set month_end_pos 7
        set day_start_pos 8
        set day_second_pos 9
        set day_end_pos 10
    } else {
        set order {month day year}
        set month [lindex $splist 0]
        set day [lindex $splist 1]
        set year [lindex $splist 2]
        set month_start_pos 0
        set month_second_pos 1
        set month_end_pos 2
        set day_start_pos 3
        set day_second_pos 4
        set day_end_pos 5
        set year_start_pos 6
        set year_second_pos 7
        set year_third_pos 8
        set year_fourth_pos 9
        set year_end_pos 10
    }
    # Process numeric keystrokes.  This involes a fair amount of 
    # processing with step one being to check and make sure we
    # aren't attempting to insert more that 10 characters.  If
    # so ring the bell and break.
    #
    if {[string match {[0-9]} $char]} {
        if {[$date index insert] == 10} {
            bell
            return -code break
        }
        #
        # If we are currently in the month field then we process the
        # number entered based on the cursor position.  If we are at
        # at the first position and our iq is low, then accept any 
        # input.
        #
        if {$_cfield eq "month"} {
            if {[$date index insert] == $month_start_pos} {
                if {$itcl_options(-iq) == "low"} {
                    $date delete $month_start_pos
                    $date insert $month_start_pos $char
                } else {
                    #
                    # Otherwise, we're slightly smarter.  If the number
                    # is less than two insert it at position zero.  If 
                    # this makes the month greater than twelve, set the 
                    # number at position one to zero which makes in 
                    # effect puts the month back in range.  
                    #
                    regsub {([0-9])([0-9])} $month "$char\\2" month2b
                    if {$char < 2} {
                        $date delete $month_start_pos
                        $date insert $month_start_pos $char
                        if {$month2b > 12} {
                            $date delete $month_second_pos
                            $date insert $month_second_pos 0
                            $date icursor $month_second_pos
                        } elseif {$month2b == "00"} {
                            $date delete $month_second_pos
                            $date insert $month_second_pos 1
                            $date icursor $month_second_pos
                        }
                        #
                        # Finally, if the number is greater than one we'll 
                        # assume that they really mean to be entering a zero
                        # followed by their number, do so for them, and 
                        # proceed to skip to the next field which is the 
                        # day field.
                        #
                    } else {
                        $date delete $month_start_pos $month_end_pos
                        $date insert $month_start_pos 0$char
                        _setField day
                    }
                }
                #
                # Else, we're at the second month position.  Again, if we aren't
                # too smart, let them enter anything.  Otherwise, if the 
                # number makes the month exceed twelve, set the month to
                # zero followed by their number to get it back into range.
                #
            } else {
                regsub {([0-9])([0-9])} $month "\\1$char" month2b
                if {$itcl_options(-iq) == "low"} {
                    $date delete $month_second_pos
                    $date insert $month_second_pos $char
                } else {
                    if {$month2b > 12} {
                        $date delete $month_start_pos $month_end_pos
                        $date insert $month_start_pos 0$char
                    } elseif {$month2b == "00"} {
                        bell
                        return -code break
                    } else {
                        $date delete $month_second_pos
                        $date insert $month_second_pos $char
                    }           
                }
                _setField day
            }
            # 
            # Now, the month processing is complete and if we're of a
            # high level of intelligence, then we'll make sure that the
            # current value for the day is valid for this month.  If
            # it is beyond the last day for this month, change it to
            # be the last day of the new month.
            #
            if {$itcl_options(-iq) == "high"} {
                set splist [split [$date get] "$split_char"]
                set month [lindex $splist [lsearch $order month]]
                if {$day > [set endday [_lastDay $month $year]]} {
                    set icursor [$date index insert]
                    $date delete $day_start_pos $day_end_pos
                    $date insert $day_start_pos $endday
                    $date icursor $icursor
                }
            }
            
            #
            # Finally, return with a code of break to stop any normal
            # processing in that we've done all that is necessary.
            #
            return -code break
        }
        #
        # This next block of code is for processing of the day field
        # which is quite similar is strategy to that of the month.
        #
        if {$_cfield eq "day"} {
            if {$itcl_options(-iq) eq "high"} {
                set endofMonth [_lastDay $month $year]
            } else {
                set endofMonth 31
            }
            #
            # If we are at the first cursor position for the day 
            # we are processing 
            # the first character of the day field.  If we have an iq 
            # of low accept any input.
            #
            if {[$date index insert] == $day_start_pos} {
                if {$itcl_options(-iq) eq "low"} {
                    $date delete $day_start_pos
                    $date insert $day_start_pos $char
                } else {
                    #
                    # If the day to be is double zero, then make the
                    # day be the first.
                    #
                    regsub {([0-9])([0-9])} $day "$char\\2" day2b
                    if {$day2b == "00"} {
                        $date delete $day_start_pos $day_end_pos
                        $date insert $day_start_pos 01
                        $date icursor $day_second_pos
                        #
                        # Otherwise, if the character is less than four 
                        # and the month is not Feburary, insert the number 
                        # and if this makes the day be beyond the valid 
                        # range for this month, than set to be back in 
                        # range.  
                        #
                    } elseif {($char < 4) && ($month != "02")} {
                        $date delete $day_start_pos
                        $date insert $day_start_pos $char
                        if {$day2b > $endofMonth} {
                            $date delete $day_second_pos
                            $date insert $day_second_pos 0
                            $date icursor $day_second_pos
                        } 
                        #
                        # For Feburary with a number to be entered of 
                        # less than three, make sure the number doesn't 
                        # make the day be greater than the correct range
                        # and if so adjust the input. 
                        #
                    } elseif {$char < 3} {
                        $date delete $day_start_pos
                        $date insert $day_start_pos $char
                        if {$day2b > $endofMonth} {
                            $date delete $day_start_pos $day_end_pos
                            $date insert $day_start_pos $endofMonth
                            $date icursor $day_second_pos
                        } 
                        #
                        # Finally, if the number is greater than three,
                        # set the day to be zero followed by the number 
                        # entered and proceed to the year field or end.
                        #
                    } else {
                        $date delete $day_start_pos $day_end_pos
                        $date insert $day_start_pos 0$char
                        $date icursor $day_end_pos
                        if {!$itcl_options(-int)} {
                            _setField year
                        }
                    }
                }
                #
                # Else, we're dealing with the second number in the day
                # field.  If we're not too bright accept anything, otherwise
                # if the day is beyond the range for this month or equal to
                # zero then ring the bell.
                #
            } else {
                regsub {([0-9])([0-9])} $day "\\1$char" day2b
                if {($itcl_options(-iq) != "low") && \
                    (($day2b > $endofMonth) || ($day2b == "00"))} {
                    bell
                } else {
                    $date delete $day_second_pos
                    $date insert $day_second_pos $char
                    $date icursor $day_end_pos
                    if {!$itcl_options(-int)} {
                        _setField year
                    }
                }
            }
    
            #
            # Return with a code of break to prevent normal processing. 
            #
            return -code break
        }
        #
        # This month and day we're tough, the code for the year is 
        # comparitively simple.  Accept any input and if we are really
        # sharp, then make sure the day is correct for the month
        # given the year.  In short, handle leap years.
        #
        if {$_cfield eq "year"} {
            if {$itcl_options(-iq) eq "low"} {
                $date delete $icursor
                $date insert $icursor $char
            } else {
                set prevdate [get]
                if {[$date index insert] == $year_start_pos} {
                    set yrdgt [lindex [split [lindex \
                         [split $prevdate "$split_char"] \
			 [lsearch $order year]] ""] 0]
                    if {$char != $yrdgt} {
                        if {$char == 1} {
                            $date delete $icursor $year_end_pos
                            $date insert $icursor 1999
                        } elseif {$char == 2} {
                            $date delete $icursor $year_end_pos
                            $date insert $icursor 2000
                        } else {
                            bell
                            return -code break
                        }
                    }
                    $date icursor $year_second_pos
                    return -code break
                }
                $date delete $icursor
                $date insert $icursor $char
                if {[catch {clock scan [get]}] != 0} {
                    $date delete $year_start_pos $year_end_pos
                    $date insert $year_start_pos \
                        [lindex [split $prevdate "$split_char"] \
			[lsearch $order year]]
                    $date icursor $icursor
                    bell
                    return -code break
                }
                if {$itcl_options(-iq) == "high"} {
                    set splist [split [$date get] "$split_char"]
                    set year [lindex $splist [lsearch $order year]]
                    if {$day > [set endday [_lastDay $month $year]]} {
                        set icursor [$date index insert]
                        $date delete $day_start_pos $day_end_pos
                        $date insert $day_start_pos $endday
                        $date icursor $icursor
                    }
                }
            }
            if {$itcl_options(-int)} {
                if {$icursor == $year_fourth_pos } {
                    _setField month
                }
            }
            return -code break
        }
        #
        # Process the plus and the up arrow keys.  They both yeild the same
        # effect, they increment the day by one.
        #
    } elseif {($sym eq "plus") || ($sym eq "Up")} {
        if {[catch {show [clock scan "1 day" -base [get -clicks]]}] != 0} {
            bell
        }
        return -code break
        #
        # Process the minus and the down arrow keys which decrement the day.
        #
    } elseif {($sym eq "minus") || ($sym eq "Down")} {
        if {[catch {show [clock scan "-1 day" -base [get -clicks]]}] != 0} {
            bell
        }
        return -code break
        #
        # A tab key moves the day/month/year (or year/month/day) field
        # forward by one unless
        # the current field is the last field.  In that case we'll let tab
        # do what is supposed to and pass the focus onto the next widget.
        #
    } elseif {($sym eq "Tab") && ($state == 0)} {
        if {$_cfield != "[lindex $order 2]"} {
            _moveField forward
            return -code break
        } else {
            _setField "[lindex $order 0]"
            return -code continue
        }
        #
        # A ctrl-tab key moves the day/month/year field backwards by one 
        # unless the current field is the the first field.  In that case we'll
        # let tab take the focus to a previous widget.
        #
    } elseif {($sym eq "Tab") && ($state == 4)} {
        if {$_cfield != "[lindex $order 0]"} {
            _moveField backward
            return -code break
        } else {
            set _cfield "[lindex $order 0]"
            return -code continue
        }
        #
        # A right arrow key moves the insert cursor to the right one.
        #
    } elseif {$sym eq "Right"} {
        _forward
        return -code break
        #
        # A left arrow, backspace, or delete key moves the insert cursor 
        # to the left one.  This is what you expect for the left arrow
        # and since the whole widget always operates in overstrike mode,
        # it makes the most sense for backspace and delete to do the same.
        #
    } elseif {$sym eq "Left" || $sym eq "BackSpace" || $sym eq "Delete"} {
        _backward
        return -code break
    } elseif {($sym eq "Control_L") || ($sym eq "Shift_L") || \
            ($sym eq "Control_R") || ($sym eq "Shift_R")} {
        return -code break
        #
        # A Return key invokes the optionally specified command option.
        #
    } elseif {$sym == "Return"} {
        uplevel #0 $itcl_options(-command)
        return -code break 
    } else {
        bell
        return -code break
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _setField field
#
# Internal method which adjusts the field to be that of the 
# argument, setting the insert cursor appropriately.
# ------------------------------------------------------------------
::itcl::body Datefield::_setField {field} {
    set _cfield $field
    if {$itcl_options(-int)} {
        set year_pos 2
        set month_pos 5
        set day_pos 8
    } else {
        set month_pos 0
        set day_pos 3
        set year_pos 8
    }
    switch $field {
    "month" {
        $date icursor $month_pos
      }
    "day" {
        $date icursor $day_pos
      }
    "year" {
        $date icursor $year_pos
      }
    default {
        error "bad field: \"$field\", must be month, day or year"
      }
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _moveField
#
# Internal method for moving the field forward or backward by one.
# ------------------------------------------------------------------
::itcl::body Datefield::_moveField {direction} {
    set index [lsearch $_fields $_cfield]
    if {$direction eq "forward"} {
        set newIndex [expr {$index + 1}]
    } else {
        set newIndex [expr {$index - 1}]
    }
    if {$newIndex == [llength $_fields]} {
        set newIndex 0
    }
    if {$newIndex < 0} {
        set newIndex [expr {[llength $_fields] - 1}]
    }
    _setField [lindex $_fields $newIndex]
    return
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _whichField
#
# Internal method which returns the current field that the cursor
# is currently within.
# ------------------------------------------------------------------
::itcl::body Datefield::_whichField {} {
    set icursor [$date index insert]
    if {$itcl_options(-int)} {
        switch $icursor {
        0 -
	1 -
	2 -
	3 {
            set _cfield "year"
          }
        5 -
	6 {
            set _cfield "month"
          }
        8 -
	9 {
            set _cfield "day"
          }
        }
    } else {
        switch $icursor {
        0 -
	1 {
            set _cfield "month"
          }
        3 -
	4 {
            set _cfield "day"
          }
        6 -
	7 -
	8 -
	9 {
            set _cfield "year"
          }
        }
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _forward
#
# Internal method which moves the cursor forward by one character
# jumping over the slashes and wrapping.
# ------------------------------------------------------------------
::itcl::body Datefield::_forward {} {
    set icursor [$date index insert]
    if {$itcl_options(-int)} {
        switch $icursor {
        3 {
            _setField month
          }
        6 {
            _setField day
          }
        9 -
	10 {
            _setField year
          }
        default {
            $date icursor [expr {$icursor + 1}]
          }
        }
    } else {
        switch $icursor {
        1 {
            _setField day
          }
        4 {
            _setField year
          }
        9 -
	10 {
            _setField month
          }
        default {
            $date icursor [expr {$icursor + 1}]
          }
        }
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _backward
#
# Internal method which moves the cursor backward by one character
# jumping over the slashes and wrapping.
# ------------------------------------------------------------------
::itcl::body Datefield::_backward {} {
    set icursor [$date index insert]
    if {$itcl_options(-int)} {
        switch $icursor {
        8 {
            _setField month
          }
        5 {
            _setField year
          }
        0 {
            _setField day
          }
        default {
            $date icursor [expr {$icursor -1}]
          }
        }
    } else {
        switch $icursor {
        6 {
            _setField day
          }
        3 {
            _setField month
          }
        0 {
            _setField year
          }
        default {
            $date icursor [expr {$icursor -1}]
          }
        }
    }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _lastDay month year
#
# Internal method which determines the last day of the month for
# the given month and year.  We start at 28 and go forward till
# we fail.  Crude but effective.
# ------------------------------------------------------------------
::itcl::body Datefield::_lastDay {month year} {
    set lastone 28
    for {set lastone 28} {$lastone < 32} {incr lastone} {
        set nextone [expr $lastone + 1]
        if {[catch {clock scan $month/$nextone/$year}] != 0} {
            return $lastone
        }
    }
}

} ; # end ::itcl::widgets
