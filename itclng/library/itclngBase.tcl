namespace eval $::itcl::internal::infos::rootNamespace {
    variable __rootNamespace $::itcl::internal::infos::rootNamespace

    namespace eval internal::parseinfos {}
    namespace eval internal::classinfos {}
    namespace ensemble create
    namespace ensemble configure $__rootNamespace -map [list \
	object ${__rootNamespace}::object \
	body ${__rootNamespace}::body \
    ]
    namespace eval object {
	upvar __rootNamespace __rootNamespace

        namespace ensemble create
	namespace ensemble configure ${__rootNamespace}::object -map [list \
	    add ${__rootNamespace}::object::add \
	]
        namespace eval add {
	    upvar __rootNamespace __rootNamespace

	    namespace ensemble create
	    namespace ensemble configure ${__rootNamespace}::object::add -map [list \
	        option ${__rootNamespace}::object::add::option \
		filter ${__rootNamespace}::object::add::filter \
	    ]
            proc option {args} {
	        puts stderr "${::itcl::internal::infos::rootNamespace}::object:add::option called![info level 0]!$args!"
            }
            proc filter {args} {
	        puts stderr "${::itcl::internal::infos::rootNamespace}::object:add::option called![info level 0]!$args!"
            }
	}
    }
}

