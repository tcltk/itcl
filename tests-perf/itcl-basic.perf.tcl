#!/usr/bin/tclsh

# ------------------------------------------------------------------------
#
# itcl-basic.perf.tcl --
#
#  This file provides performance tests for comparison of basic itcl-speed.
#
# ------------------------------------------------------------------------
#
# Copyright (c) 2019 Serg G. Brester (aka sebres)
#
# See the file "license.terms" for information on usage and redistribution
# of this file.
#


if {![namespace exists ::tclTestPerf]} {
  source -encoding utf-8 [file join [file dirname [info library]] tests-perf test-performance.tcl]
}

namespace eval ::itclTestPerf-Basic {

namespace path {::tclTestPerf}


## test cases covering regression on class count (memory preserve/release):
proc test-cls-init {{reptime {3000 1000}}} {
  set reptime [_adjust_maxcount $reptime 1000]
  _test_run $reptime {
    setup {set i 0; set k 0}
    ## 1) create up-to 1000 classes (with 100 vars):
    {itcl::class timeClass[incr i] { for {set j 0} {$j<100} {incr j} { public variable d$j } }}
    ## 2) create up-to 1000 classes (with 100 vars):
    {itcl::class finiClass[incr k] { for {set j 0} {$j<100} {incr j} { public variable d$j } }}
    ## 2) delete up-to 1000 classes:
    {itcl::delete class finiClass$k; if {[incr k -1] <= 0} break}
    cleanup {while {$k > 0} {itcl::delete class finiClass$k; incr k -1}}
    ## 1) delete up-to 1000 classes:
    {itcl::delete class timeClass$i; if {[incr i -1] <= 0} break}
    cleanup {while {$i > 0} {itcl::delete class timeClass$i; incr i -1}}
  }
}

## test cases covering run-time dependency to variable count of class with nested
## namespaces and class inheritances...
## original itcl-resolver (due to completely rebuild) has the complexity ca. O(nn**2,2**vn) here,
## so the deeper a class/inheritance and expecially the more variables it has,
## the worse the performance of class creation or modification.

proc test-var-create {{reptime {3000 10000}}} {
  upvar maxv maxv
  foreach ns {{} ::test-itcl-ns1 ::test-itcl-ns1::test-itcl-ns2} {
    incr n
    if {$ns ne {}} { namespace eval $ns {} }
    _test_start $reptime
    foreach clsi {0 1 2} {
      if {$clsi} {
        set inh ${ns}::timeClass[expr {$clsi-1}]
      } else {
        set inh {}
      }
      set cls ${ns}::timeClass$clsi
      puts "== ${n}.$clsi) class : $cls == [expr {$inh ne "" ? "inherite $inh" : ""}]"
      if {[info command $cls] ne ""} {
        itcl::delete class $cls
      }
      itcl::class $cls [string map [list \$reptime [list $reptime] \$in_inh [list $inh] \$clsi $clsi] {
        set j 0
        set inh $in_inh
        if {$inh ne ""} {
          puts "% inherit $inh"
          ::tclTestPerf::_test_iter 2 [timerate {
            inherit $inh
          } 1 1]
        }
        puts "% declare vars ..."
        ::tclTestPerf::_test_iter 2 [timerate {
          public variable pub[incr j] 0
          protected variable pro$j 1
          private variable pri$j 2
          # 10K commons is too slow in Itcl original edition (time grows on each iter), so 1K enough:
          if {$j <= 1000} {
            public common com$j ""
          }
        } {*}$reptime]
        public method getv {vn} {set $vn}
        public method getpub1 {} {set pro1}
        public method getpro1 {} {set pro1}
        public method getpri1 {} {set pri1}
        public method getunknown {} {catch {set novarinthisclass}}
        # Itcl original edition may be too slow (time grows on each inheritance), so save real max-iters (<= 10K):
        uplevel [list set j $j]
      }]
      set maxv($clsi,$ns) $j
    }
  }
  _test_out_total
}

# access variable:
proc test-access {{reptime 1000}} {
  upvar maxv maxv
  _test_start $reptime
  foreach ns {{} ::test-itcl-ns1 ::test-itcl-ns1::test-itcl-ns2} {
    set reptm [_adjust_maxcount $reptime $maxv(0,$ns)]
    incr n
    set cls ${ns}::timeClass0
    puts "== ${n}) class : $cls =="
    set mp [list \
      \$cls $cls \$n $n \
      \$maxc0 [expr {min(1000,$maxv(0,$ns))}]
    ]
    _test_run $reptm [string map $mp {
      # $n) obj-var resolve/get
      setup {$cls o; set j 0}
      {o getv pub[incr j]}
      # $n) obj-var get (resolved)
      setup {set j 0}
      {o getv pub[incr j]}
      # $n) obj-var resolved
      setup {set j 0}
      {o getv pub1}
      # $n) obj-var in method compiled (public)
      {o getpub1}
      # $n) obj-var in method compiled (protected)
      {o getpro1}
      # $n) obj-var in method compiled (private)
      {o getpri1}
      # $n) obj-var in method unknown
      {o getunknown}
      cleanup {itcl::delete object o}

      # $n) obj-var resolve/cget
      setup {$cls o; set j 0}
      {o cget -pub[incr j]}
      # $n) obj-var cget (resolved):
      setup {set j 0}
      {o cget -pub[incr j]}

      # $n) obj-var cfg/cget
      {o configure -pub1}
      {o cget -pub1}

      # $n) cls-com resolve
      setup {set j 0}
      {o getv com[incr j]; if {$j >= $maxc0} {set j 0}}

      # $n) cls-com resolved
      {o getv com1}
      cleanup {itcl::delete object o}
    }]
  }
  _test_out_total
}

# ------------------------------------------------------------------------

# create/delete object:
proc test-obj-instance {{reptime 1000}} {
  _test_start $reptime
  set n 0
  foreach ns {{} ::test-itcl-ns1 ::test-itcl-ns1::test-itcl-ns2} {
    incr n
    set cls ${ns}::timeClass0
    puts "== ${n}) class : $cls =="
    _test_run $reptime [string map [list \$cls $cls \$n $n] {
      setup {set i 0}
      # $n) create :
      {$cls o[incr i]}
      # $n) delete:
      {itcl::delete object o$i; if {[incr i -1] <= 0} break}
      cleanup {while {$i > 0} {itcl::delete object o$i; incr i -1}}
      # $n) create + delete:
      {$cls o; itcl::delete object o}
    }]
  }
  _test_out_total
}

# ------------------------------------------------------------------------

proc test {{reptime 1000}} {
  set reptm $reptime
  lset reptm 0 [expr {[lindex $reptm 0] * 10}]
  if {[llength $reptm] == 1} {
    lappend reptm 10000
  }
  puts "==== initialization (preserve/release) ====\n"
  test-cls-init $reptm
  puts "==== class/var creation ====\n"
  test-var-create $reptm
  puts "==== var access ====\n"
  test-access $reptime
  puts "==== object instance ====\n"
  test-obj-instance $reptime

  puts \n**OK**
}

}; # end of ::tclTestPerf-Timer-Event

# ------------------------------------------------------------------------

# if calling direct:
if {[info exists ::argv0] && [file tail $::argv0] eq [file tail [info script]]} {
  array set in {-time 500 -lib {} -load {}}
  array set in $argv
  if {$in(-load) ne ""} {
    eval $in(-load)
  }
  if {![namespace exists ::itcl]} {
    if {$in(-lib) eq ""} {
      set in(-lib) "itcl412"
    }
    puts "testing with $in(-lib)"
    load $in(-lib) itcl
  }

  ::itclTestPerf-Basic::test $in(-time)
}
