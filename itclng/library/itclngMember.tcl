namespace eval ${::itcl::internal::infos::rootNamespace}::member {

    proc GetArgumentInfos {name arguments} {
	set minArgs 0
	set maxArgs 0
	set haveArgsArg 0
	set usage ""
	if {$arguments ne ""} {
	    set lgth [llength $arguments]
	    set argNo 0
	    set sep ""
	    set defaultArgs [list]
	    foreach arg $arguments {
		incr argNo
	        if {$arg eq ""} {
		    return -code error -level 4 "procedure \"$name\" has \
argument with no name"
		}
		incr maxArgs
	        set argumentName [lindex $arg 0]
	        switch [llength $arg] {
		1 {
		    set haveDefault 0
		  }
		2 {
		    set haveDefault 1
		    set defaultValue [lindex $arg 1]
		    lappend defaultArgs [expr {$argNo - 1}] $defaultValue
		  }
		default {
		    return -code error -level 4 "too many fields in argument specifier \"$arg\"
		  }
		}
		if {[string first "::" $argumentName] >= 0} {
		    return -code error -level 4 "bad argument name \
\"$argumentName\""
		}
		append usage $sep
		if {$argumentName eq "args"} {
		    if {$argNo == $lgth} {
		        set haveArgsArg 1
		    }
		}
		if {$haveArgsArg} {
		    append usage "?arg arg ...?"
		} else {
		    if {$haveDefault} {
		        append usage "?$argumentName?"
		    } else {
		        append usage $argumentName
		        incr minArgs
		    }
		}
	        set sep " "
	    }
	}
	set argInfos [list $minArgs $maxArgs $haveArgsArg $usage $defaultArgs]
        return $argInfos
    }

    proc methodOrProc {infoNs className type protection name args} {
puts stderr "methodOrProc!$name!$type!"
#puts stderr "ME!$infoNs!$className!$protection!$name![join $args !]!"
	if {[dict exists [set ${infoNs}] functions $name]} {
	    return -code error -level 4 "\"$name\" already defined in class \"$className\""
	}
	if {[string first "::" $name] >= 0} {
	    return -code error -level 4 "bad $type name \"$name\""
	}
	switch [llength $args] {
	0 {
	    set state NO_ARGS
	    set arguments ""
	    set body ""
	  }
	1 {
	    set state NO_BODY
	    set arguments [lindex $args 0]
	    set body ""
	  }
	2 {
	    set state COMPLETE
	    set arguments [lindex $args 0]
	    set body [lindex $args 1]
	  }
	default {
	    return -code error -level 4 "should be: <protection> $type <name> ?arguments? ?body?"
	  }
	}
	set minArgs 0
	set maxArgs 0
	set haveArgsArg 0
	set usage ""
	set defaultArgs [list]
	if {$state ne "NO_ARGS"} {
	    set argInfo [GetArgumentInfos $name $arguments]
	    foreach {minArgs maxArgs haveArgsArg usage defaultArgs} $argInfo break
	}
	set argumentInfo [list definition $arguments minargs $minArgs maxargs $maxArgs haveArgsArg $haveArgsArg usage $usage defaultargs $defaultArgs]
        dict lappend ${infoNs} functions $name [list protection $protection arguments $argumentInfo origArguments $argumentInfo body $body origBody $body state $state]
#puts stderr "DI2![dict get [set ${infoNs}] functions $name]!"
        ${::itcl::internal::infos::internalCmds} createClass$type $className $name
    }

    proc commonOrVariable {infoNs className type protection name args} {
#puts stderr "commonOrVariable!$name![llength $args]!"
#puts stderr "ME!$infoNs!$className!$protection!$name![join $args !]!"
	if {[dict exists [set ${infoNs}] variables $name]} {
	    return -code error -level 4 "\"$name\" already defined in class \"$className\""
	}
	if {[string first "::" $name] >= 0} {
	    return -code error -level 4 "bad $type name \"$name\""
	}
	switch [llength $args] {
	0 {
	    set state NO_INIT
	    set initValue ""
	    set configValue ""
	  }
	1 {
	    set state NO_CONFIG
	    set initValue [lindex $args 0]
	    set configValue ""
	  }
	2 {
	    if {($protection ne "public") || ($type eq "Common")} {
	        return -code error -level 4 "should be: <protection> $type <name> ?init?"
	    }
	    set state COMPLETE
	    set initValue [lindex $args 0]
	    set configValue [lindex $args 1]
	  }
	default {
	    set protectionUsage ""
	    if {$protection eq "public"} {
	         set protectionUsage " ?config?"
	    }
	    return -code error -level 4 "should be: <protection> $type <name> ?init?$protectionUsage"
	  }
	}
        dict lappend ${infoNs} variables $name [list protection $protection init $initValue config $configValue state $state]
#puts stderr "DI2![dict get [set ${infoNs}] variables $name]!"
        ${::itcl::internal::infos::internalCmds} createClass$type $className $name
    }

    proc method {infoNs className protection name args} {
        return [methodOrProc $infoNs $className Method $protection $name {*}$args]
    }

    proc common {infoNs className protection name args} {
        return [commonOrVariable $infoNs $className Common $protection $name {*}$args]
    }

    proc variable {infoNs className protection name args} {
        return [commonOrVariable $infoNs $className Variable $protection $name {*}$args]
    }

    proc constructor {infoNs className type protection name args} {
	if {[dict exists [set ${infoNs}] functions $name]} {
	    return -code error -level 4 "\"$name\" already defined in class \"$className\""
	}
	set initCode [list]
	set body [list]
        switch [llength $args] {
	1 {
	    set state NO_BODY
	    set arguments [lindex $args 0]
	  }
	2 {
	    set state COMPLETE
	    set arguments [lindex $args 0]
	    set body [lindex $args 1]
	  }
	3 {
	    set state COMPLETE
	    set arguments [lindex $args 0]
	    set initCode [lindex $args 1]
	    set body [lindex $args 2]
	  }
	default {
	    return -code error -level 3 "Wrong # args should be $name args ?init? body"
	  }
	}
	set minArgs 0
	set maxArgs 0
	set haveArgsArg 0
	set usage ""
	set defaultArgs [list]
        set argInfo [GetArgumentInfos $name $arguments]
	foreach {minArgs maxArgs haveArgsArg usage defaultArgs} $argInfo break
	set argumentInfo [list definition $arguments minargs $minArgs maxargs $maxArgs haveArgsArg $haveArgsArg usage $usage defaultargs $defaultArgs]
        dict lappend ${infoNs} functions $name [list protection $protection arguments $argumentInfo origArguments $argumentInfo body $body origBody $body state $state]
#puts stderr "DI2![dict get [set ${infoNs}] functions $name]!"
	if {$initCode ne ""} {
            dict lappend ${infoNs} functions __constructor_init [list protection $protection arguments $argumentInfo origArguments $argumentInfo body $initCode origBody $initCode state COMPLETE]
            ${::itcl::internal::infos::internalCmds} createClassConstructorInit $className __constructor_init
	}
        ${::itcl::internal::infos::internalCmds} createClassConstructor $className $name
    }

    proc destructor {infoNs className type protection name args} {
puts stderr "DES!$args!"
    }

    proc methodvariable {infoNs className protection name args} {
    }

    proc component {infoNs className protection name args} {
    }

    proc option {infoNs className protection name args} {
    }

    proc inherit {infoNs className args} {
	set nsName [uplevel 5 namespace current]
	if {$nsName ne "::"} {
	    set nsName ${nsName}::
	}
puts stderr "INHERIT!$nsName!$infoNs!$className!$args!"
	set inh [dict get [set $infoNs] inheritance]
        if {[llength $inh] > 0} {
	    return -code error -level 5 "inheritance \"[join $inh { }]\" already defined for class \"$className\""
	}
	set idx -1
	set superClasses [list]
	foreach clsName $args {
	    set fullClsName $clsName
	    if {![string match "::*" $clsName]} {
	        set fullClsName $nsName$clsName
	    }
	    incr idx
	    set idx2 [lsearch $args $clsName]
	    if {$idx != $idx2} {
	        return -code error -level 6 "class \"$className\" more than once in inherit list"
	    }
	    set idx2 [lsearch [lrange $args [expr {$idx+1}] end] $clsName]
	    if {$idx2 >= 0} {
	        return -code error -level 6 "class \"$clsName\" more than once in inherit list"
	    }
	    if {$clsName eq "$className"} {
	        return -code error -level 6 "class \"$className\" cannot inherit from itself"
	    }
	    if {![::namespace exists ${::itcl::internal::infos::internalClassInfos}$fullClsName]} {
	        return -code error -level 5 "cannot inherit from \"$clsName\""
	    }
	    lappend superClasses $fullClsName
	}
	dict set $infoNs inheritance $superClasses
        ${::itcl::internal::infos::internalCmds}::createClassInherit $className {*}$superClasses
	::oo::define $className superclass {*}$superClasses
    }

    proc proc {infoNs className protection name args} {
        return [methodOrProc $infoNs $className Proc $protection $name {*}$args]
    }
}
