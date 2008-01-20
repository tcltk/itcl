namespace eval ${::itcl::internal::infos::rootNamespace}::builtin {
    variable infoNS $::itcl::internal::infos::internalClassInfos
    variable __builtinRootNamespace ${::itcl::internal::infos::rootNamespace}::builtin

    proc infox {args} {
        puts stderr "builtin info called!$args!"
    }
    proc configure {args} {
        puts stderr "builtin configure called!$args!"
    }
    proc cget {args} {
        puts stderr "builtin cget called!$args!"
    }
    proc create {className args} {
        puts stderr "builtin create called!$className!$args!"
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
	set cmd [list uplevel 1 ${::itcl::internal::infos::internalCmds}::createObject \
	        $className $className {*}[list $args]]
	if {[catch {
	    eval $cmd
        } obj errInfo]} {
            return -code error -level 2 -errorinfo $::errorInfo $obj
	}
        return $obj

    }
    proc unknown {args} {
        puts stderr "builtin unknown called!$args!"
    }
    proc objectunknown {args} {
        puts stderr "builtin objectunknown called!$args!"
    }
}
