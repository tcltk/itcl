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
            set __rootNamespace $::itcl::internal::infos::rootNamespace

	    set forceFullNames 0
	    switch [llength $args] {
	    0 {
	        set pattern ""
	      }
            1 {
	        set pattern [lindex $args 0]
		if {[string match "*::*" $pattern]} {
	            set forceFullNames 0
	        }
	      }
	    default {
	        return -code error -level 2 "wrong # args should be find classes ?pattern?"
	      }
	    }
            set keys [namespace children ${__rootNamespace}::internal::classinfos]
            set keys [string map [list ${__rootNamespace}::internal::classinfos {}] $keys]
	    set result [list]
            foreach name $keys {
		switch $name {
	        ::itcl {
	            continue
	          }
	        default {
		    if {!$forceFullNames} {
		        set name [string trimleft $name :]
		    }
	            if {$pattern eq ""} {
		        lappend result $name
		    } else {
		        if {[string match ${pattern} $name]} {
		            lappend result $name
		        }
		    }
		  }
		}
	    }
            return $result
	}

	proc objects {args} {
            set __rootNamespace $::itcl::internal::infos::rootNamespace
	    set result [uplevel 1 ${__rootNamespace}::internal::commands::findObjects $args]
	    return $result
	}

	proc object {args} {
            set __rootNamespace $::itcl::internal::infos::rootNamespace

puts stderr "find object called!$args!"
	    return [uplevel 1 ${__rootNamespace}::internal::commands::findObject $args]
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
            set __rootNamespace $::itcl::internal::infos::rootNamespace
            return [uplevel 1 ${__rootNamespace}::internal::commands::isClass $args]
	}

	proc object {args} {
            set __rootNamespace $::itcl::internal::infos::rootNamespace
            return [uplevel 1 ${__rootNamespace}::internal::commands::isObject $args]
	}
    }

    namespace eval delete {
	upvar __rootNamespace __rootNamespace

        namespace ensemble create
	namespace ensemble configure ${__rootNamespace}::delete -map [list \
	    class ${__rootNamespace}::delete::class \
	    object ${__rootNamespace}::delete::object \
	]
        proc class {args} {
            set __rootNamespace $::itcl::internal::infos::rootNamespace
            return [uplevel 1 ${__rootNamespace}::internal::commands::deleteClass $args]
        }
        proc object {args} {
            set __rootNamespace $::itcl::internal::infos::rootNamespace
            return [uplevel 1 ${__rootNamespace}::internal::commands::deleteObject $args]
        }
    }
    proc code {args} {
        set __rootNamespace $::itcl::internal::infos::rootNamespace
        return [uplevel 1 ${__rootNamespace}::internal::commands::code $args]
    }
    proc scope {args} {
        set __rootNamespace $::itcl::internal::infos::rootNamespace
        return [uplevel 1 ${__rootNamespace}::internal::commands::scope $args]
    }
    proc body {args} {
        set __rootNamespace $::itcl::internal::infos::rootNamespace
#        return [uplevel 1 ${__rootNamespace}::internal::commands::body $args]
return {}
    }
    proc configbody {args} {
        set __rootNamespace $::itcl::internal::infos::rootNamespace
#        return [uplevel 1 ${__rootNamespace}::internal::commands::body $args]
return {}
    }
}

