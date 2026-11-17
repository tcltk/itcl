# helpers.tcl --
#
# This file contains helper scripts for all tests, like a mem-leak checker, etc.

# -loadfile overwrites -load, so restore it from ::env(TESTFLAGS):
if {[info exists ::env(TESTFLAGS)]} {
  array set testargs $::env(TESTFLAGS)
  if {[info exists ::testargs(-load)]} {
    eval $::testargs(-load)
  }
  unset testargs
}

package require itcl

if {[namespace which -command memory] ne "" && (
    ![info exists ::tcl::inl_mem_test] || $::tcl::inl_mem_test
  )
} {
  proc getbytes {} {lindex [split [memory info] \n] 3 3}
  proc leaktest {script {iterations 3}} {
      set end [getbytes]
      for {set i 0} {$i < $iterations} {incr i} {
	  uplevel 1 $script
	  set tmp $end
	  set end [getbytes]
      }
      return [expr {$end - $tmp}]
  }
  proc itcl_leaktest {testfile} {
    set leak [leaktest [string map [list \
      @test@ $testfile \
      @testargv@ [if {[info exists ::argv]} {list tcltest::configure {*}$::argv}]
    ] {
      interp create i
      load {} Itcl i
      i eval {set ::tcl::inl_mem_test 0}
      i eval {package require tcltest; @testargv@}
      i eval [list source -encoding utf-8 @test@]
      interp delete i
      # Process the window events of a deleted Tk application; until
      # then they hold memory, which would be counted as leaked.
      update
    }]]
    if {$leak} {
      puts "LEAKED: $leak bytes"
    }
  }
  itcl_leaktest [info script]
  # The test file may have loaded Tk in this interpreter (to set its tk
  # constraint) before calling loadTestedCommands. Destroy its main
  # window, or the application would wait in the Tk event loop when the
  # script ends.
  if {[info commands ::destroy] ne ""} {
    destroy .
  }
  return -code return
}
