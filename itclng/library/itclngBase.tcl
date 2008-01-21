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

    namespace eval find {
	upvar __rootNamespace __rootNamespace

        namespace ensemble create
	namespace ensemble configure ${__rootNamespace}::find -map [list \
	    classes ${__rootNamespace}::find::classes \
	    objects ${__rootNamespace}::find::objects \
	    object ${__rootNamespace}::find::object \
	]
	proc classes {args} {
puts stderr "find classes called!$args!"
	}

	proc objects {args} {
puts stderr "find objects called!$args!"
	}

	proc object {args} {
puts stderr "find object called!$args!"
	}
    }

    namespace eval is {
	upvar __rootNamespace __rootNamespace

        namespace ensemble create
	namespace ensemble configure ${__rootNamespace}::is -map [list \
	    class ${__rootNamespace}::is::class \
	    object ${__rootNamespace}::is::object \
	]
	proc class {args} {
puts stderr "is class called!$args!"
	}

	proc object {args} {
puts stderr "is object called!$args!"
	}
    }

    proc delete {args} {
puts stderr "delete called!$args!"
    }
}

