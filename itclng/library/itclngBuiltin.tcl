namespace eval ${::itcl::internal::infos::rootNamespace}::builtin {
    variable infoNS $::itcl::internal::infos::internalClassInfos
    variable __builtinRootNamespace ${::itcl::internal::infos::rootNamespace}::builtin

    proc configure {args} {
puts stderr "builtin configure called!$args!"
    }
    proc cget {args} {
puts stderr "builtin cget called!$args!"
    }
    proc chain {args} {
puts stderr "builtin chain called!$args!${::itcl::internal::infos::internalCmds}!"
        return [${::itcl::internal::infos::internalCmds}::chain $args]
    }
    proc isa {args} {
puts stderr "builtin isa called!$args!"
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
        } errInfo errInfo2]} {
#            return -code error -level 1 $errInfo
            return -code error -level 2 -errorinfo $::errorInfo $errInfo
	}
        return $obj

    }
    proc unknown {args} {
puts stderr "builtin unknown called!$args!"
    }
    proc objectunknown {args} {
puts stderr "builtin objectunknown called!$args!"
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
puts stderr "COI!$callContextInfo!"
        return -code error -level 2 "objectunknown bad option \"[lindex $args 1]\": should be one of ...\n\
"
    }
}
