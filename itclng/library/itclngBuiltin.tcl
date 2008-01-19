namespace eval ::itclng::builtin {
    variable infoNS ::itclng::internal::classinfos

    proc info {args} {
        puts stderr "::itclng::builtin::info called!$args!"
    }
    proc configure {args} {
        puts stderr "::itclng::builtin::configure called!$args!"
    }
    proc cget {args} {
        puts stderr "::itclng::builtin::cget called!$args!"
    }
    proc create {className args} {
        puts stderr "::itclng::builtin::create called!$className!$args!"
        set namespace [uplevel 1 namespace current]
	set myNamespace $namespace
        if {$myNamespace ne "::"} {
            set myNamespace ${myNamespace}::
        }
# check for class already exists here !!!
        set cmd [uplevel 1 ::info command ${myNamespace}$className]
	if {[string length $cmd] > 0} {
            error "command \"$className\" already exists in namespace \"$namespace\""
        }
	set cmd [list uplevel 1 ::itclng::internal::commands::createObject \
	        $className $className {*}[list $args]]
	if {[catch {
	    eval $cmd
        } obj errInfo]} {
            return -code error -level 2 -errorinfo $::errorInfo $obj
	}
        return $obj

    }
    proc unknown {args} {
        puts stderr "::itclng::builtin::unknown called!$args!"
    }
}
