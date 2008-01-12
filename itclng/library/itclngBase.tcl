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

puts stderr "CREATE!$className!"
	    set ${infosNsName}::currClassName $className
	    set nsName [uplevel 1 namespace current]
	    if {$nsName ne "::"} {
	        set nsName ${nsName}::
	    }
	    set fullClassName $nsName$className
	    set ${infosNsName}::currFullClassName $fullClassName
	    set myClassName [string trimleft $fullClassName :]
	    set infoNs ::itclng::internal::classinfos::${myClassName}
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
