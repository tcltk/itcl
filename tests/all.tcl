# all.tcl --
#
# This file contains a top-level script to run all of the Tcl
# tests.  Execute it by invoking "source all.test" when running tcltest
# in this directory.
#
# Copyright (c) 1998-2000 by Ajuba Solutions
# All rights reserved.

package prefer latest

package require Tcl 8.6
package require tcltest 2.2

if {"-testdir" ni $argv} {
    lappend argv -testdir [file dir [info script]]
}
tcltest::configure {*}$argv \
	-loadfile [file join [file dirname [info script]] helpers.tcl]
tcltest::runAllTests

return
