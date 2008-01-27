namespace eval ${::itcl::internal::infos::rootNamespace}::builtin {
    variable infoNS $::itcl::internal::infos::internalClassInfos
    variable __builtinRootNamespace ${::itcl::internal::infos::rootNamespace}::builtin

    proc configure {args} {
puts stderr "builtin configure called!$args!"
    }
    proc cget {args} {
puts stderr "builtin cget called!$args!"
    }
    proc create {className args} {
#puts stderr "BI!create!$className!$args!"
	if {[llength $args] == 0} {
	    # call without arguments: ignore it
	    return
	}
	set objectName [lindex $args 0]
	set args [lrange $args 1 end]
        set namespace [uplevel 1 namespace current]
	set myNamespace $namespace
        if {$myNamespace ne "::"} {
            set myNamespace ${myNamespace}::
        }
# check for class already exists here !!!
        set cmd [uplevel 1 ::info command ${myNamespace}$objectName]
#puts stderr "CMD!$namespace!$cmd!"
	if {[string length $cmd] > 0} {
            return -code error -level 2 "command \"$objectName\" already exists in namespace \"$namespace\""
        }
	set cmd [list uplevel 1 \
	        ${::itcl::internal::infos::internalCmds}::createObject \
	        $className $objectName {*}[list $args]]
	if {[catch {
	    set obj [eval $cmd]
        } errInfo]} {
            return -code error -level 2 -errorinfo $::errorInfo
	}
        return $obj

    }
    proc unknown {args} {
puts stderr "builtin unknown called!$args!"
    }
    proc objectunknown {args} {
        return -code error -level 2 "bad option \"[lindex $args 1]\": should be one of ...\n\
"
    }
}
