namespace eval ::itclng {
    namespace eval internal::parseinfos {}
    namespace eval internal::classinfos {}
    namespace ensemble create
    namespace ensemble configure ::itclng -map [list \
        class ::itclng::class \
	object ::itclng::object \
	body ::itclng::body \
    ]
    namespace eval class {
	variable infosNsName ::itclng::internal::parseinfos
        namespace ensemble create
	namespace ensemble configure ::itclng::class -map [list \
	    create ::itclng::class::create \
	]
        proc create {className args} {
	    variable infosNsName

	    if {$className eq ""} {
	        return -code error -level 2 "invalid classname \"\""
	    }
	    set nsName [uplevel 1 namespace current]
	    if {$nsName ne "::"} {
	        set nsName ${nsName}::
	    }
	    set fullClassName $nsName$className
	    set infoNs ::itclng::internal::classinfos$fullClassName
	    if {[::namespace exists $infoNs]} {
	        return -code error -level 2 "class \"$className\" already exists"
	    }
	    if {[::info comm $fullClassName] ne ""} {
	        return -code error -level 2 "command \"$className\" already exists"
	    }
	    set ${infosNsName}::currClassName $className
	    set ${infosNsName}::currFullClassName $fullClassName
	    set xx [::itclng::internal::commands createClass $fullClassName]
puts stderr "CLASS!$xx!"
	    namespace eval $infoNs {}
	    set infoNs ${infoNs}::infos
puts stderr "IS!$infoNs!"
	    set $infoNs [list]
	    dict set ${infoNs} functions [list]
puts stderr "IV!$infoNs![dict get [set ${infoNs}] functions]!"
	    dict set ${infoNs} variables [list]
	    dict set ${infoNs} options [list]
	    namespace eval ::itclng::parser {*}$args
	}
    }
    namespace eval object {
        namespace ensemble create
	namespace ensemble configure ::itclng::object -map [list \
	    add ::itclng::object::add \
	]
        namespace eval add {
	    namespace ensemble create
	    namespace ensemble configure ::itclng::object::add -map [list \
	        option ::itclng::object::add::option \
		filter ::itclng::object::add::filter \
	    ]
            proc option {args} {
	        puts stderr "::itclng::object:add::option called![info level 0]!$args!"
            }
            proc filter {args} {
	        puts stderr "::itclng::object:add::option called![info level 0]!$args!"
            }
	}
    }
}
