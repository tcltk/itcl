namespace eval ::itclng::member {

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
puts stderr "methodOrProc!$name![llength $args]!"
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
	    return -code error -level 4 "should be: <protection> method <name> ?arguments? ?body?"
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
        ::itclng::internal::commands createClass$type $className $name
    }
    proc method {infoNs className protection name args} {
        return [methodOrProc $infoNs $className Method $protection $name {*}$args]
    }
    proc common {infoNs className protection name args} {
    }
    proc variable {infoNs className protection name args} {
    }
    proc methodvariable {infoNs className protection name args} {
    }
    proc component {infoNs className protection name args} {
    }
    proc option {infoNs className protection name args} {
    }
    proc proc {infoNs className protection name args} {
        return [methodOrProc $infoNs $className Proc $protection $name {*}$args]
    }
}
