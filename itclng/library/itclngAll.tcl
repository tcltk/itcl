set dir [file dirname [::info script]]
if {$dir eq "."} {
    set dir ./library
}
source [file join $dir itclngBase.tcl]
source [file join $dir itclngParse.tcl]
source [file join $dir itclngMember.tcl]
source [file join $dir itclngBuiltin.tcl]
source [file join $dir itclngInfo.tcl]
source [file join $dir itclngClass.tcl]
