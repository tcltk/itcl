package provide Itcl 4.0
set dir [file dirname [::info script]]
#puts stderr "DIR!$dir!"
load [file join $dir libItcl4.0.so] Itcl
if {$dir eq "."} {
    set dir library
}

namespace eval ::itcl {
    namespace eval variables {}

set itclClass [::oo::class create ::itcl::clazz]
::oo::define $itclClass superclass ::oo::class

#puts stderr "INFO SCRIPT!$dir!"
source [file join $dir itclClass.tcl]
source [file join $dir itclTraces.tcl]

}
