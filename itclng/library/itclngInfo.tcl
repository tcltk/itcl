namespace eval ${::itcl::internal::infos::rootNamespace}::builtin::info {
    variable infoNS $::itcl::internal::infos::internalClassInfos
    variable __rootNamespace $::itcl::internal::infos::rootNamespace

    namespace ensemble create
    namespace ensemble configure ${__rootNamespace}::builtin::info -map [list \
	    args ${__rootNamespace}::builtin::info::args \
	    body ${__rootNamespace}::builtin::info::body \
	    class ${__rootNamespace}::builtin::info::class \
	    component ${__rootNamespace}::builtin::info::component \
	    function ${__rootNamespace}::builtin::info::function \
	    heritage ${__rootNamespace}::builtin::info::heritage \
	    inherit ${__rootNamespace}::builtin::info::inherit \
	    option ${__rootNamespace}::builtin::info::option \
	    variable ${__rootNamespace}::builtin::info::variable \
	    vars ${__rootNamespace}::builtin::info::vars \
	    exists ${__rootNamespace}::builtin::info::exists \
    ]

    proc GetInheritance {nsName className} {
# puts stderr "GetInheritance!$className!"
	set inh [list]
        set inh2 [dict get [set ${nsName}${className}::infos] inheritance]
# puts stderr "INH2!$inh2!"
        foreach className $inh2 {
	    lappend inh $className
            set inh2 [GetInheritance $nsName $className]
	    if {[llength $inh2] > 0} {
                set inh [concat $inh $inh2]
	    }
        }
# puts stderr "RET!$inh!"
        return $inh
    }

    proc args {args} {
	if {[llength $args] != 1} {
	    return -code error "wrong # args: should be \"info args function\""
	}
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
	namespace upvar ${::itcl::internal::infos::rootNamespace}::builtin::info infoNS infoNS

	if {![dict exists [set ${infoNS}${className}::infos] functions]} {
	    set err "no such class \"$className\"\n\
get info like this instead: \n\
  namespace eval className { info ", name, "... }"
	    return -code error -level 3 $err
	}
        set inh [dict get [set ${infoNS}${objectClassName}::infos] inheritance]
	set result [list]
	if {$inh ne ""} {
	    set inh [concat [list $objectClassName] $inh]
	} else {
	    set inh [list $objectClassName]
	}
	set arg0 [lindex $args 0]
	set funcName [::namespace tail $arg0]
	set prefix [::namespace qualifiers $arg0]
	if {[string length $prefix] > 0} {
	    if {$prefix ne "::"} {
	        set prefix ::$prefix
	    }
	}
	set lastClassName [lindex $inh end]
	switch $funcName {
	cget {
	    return "option"
	  }
	configure {
	    return {?-option? ?value -option value...?}
	  }
	isa {
	    return "class or object"
	  }
	}
	foreach className $inh {
	    if {[string length $prefix] > 0} {
		if {$prefix ne $className} {
		    continue
		}
	    }
            set functions [dict get [set ${infoNS}${className}::infos] functions]
            foreach {name info} $functions {
	        if {$name eq $funcName} {
# puts stderr "FUNC!$name!$info!"
	            set ret [dict get $functions $funcName arguments usage]
	            if {$ret eq ""} {
	                return "<undefined>"
	            }
                    return $ret
	        }
            }
        }
        return -code error "no such function \"$funcName\""
    }

    proc body {args} {
# puts stderr "info body called!$args!"
        if {[llength $args] != 1} {
	    return -code error "wrong # args: should be \"info body function\""
	}
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
	namespace upvar ${::itcl::internal::infos::rootNamespace}::builtin::info infoNS infoNS

	if {![dict exists [set ${infoNS}${className}::infos] functions]} {
	    set err "no such class \"$className\"\n\
get info like this instead: \n\
  namespace eval className { info ", name, "... }"
	    return -code error -level 3 $err
	}
        set inh [dict get [set ${infoNS}${objectClassName}::infos] inheritance]
	set result [list]
	if {$inh ne ""} {
	    set inh [concat [list $objectClassName] $inh]
	} else {
	    set inh [list $objectClassName]
	}
	set arg0 [lindex $args 0]
	set funcName [::namespace tail $arg0]
	set prefix [::namespace qualifiers $arg0]
	if {[string length $prefix] > 0} {
	    if {$prefix ne "::"} {
	        set prefix ::$prefix
	    }
	}
	set lastClassName [lindex $inh end]
	switch $funcName {
	cget {
	    return "@itcl-builtin-cget"
	  }
	configure {
	    return "@itcl-builtin-configure"
	  }
	isa {
	    return "@itcl-builtin-isa"
	  }
	}
	foreach className $inh {
	    if {[string length $prefix] > 0} {
		if {$prefix ne $className} {
		    continue
		}
	    }
            set functions [dict get [set ${infoNS}${className}::infos] functions]
            foreach {name info} $functions {
	        if {$name eq $funcName} {
# puts stderr "FUNC!$name!$info!"
	            set ret [dict get $functions $funcName body]
	            if {$ret eq ""} {
	                return "<undefined>"
	            }
                    return $ret
	        }
            }
        }
        return -code error "no such function \"$funcName\""
    }

    proc class {args} {
# puts stderr "info class called!$args![namespace current]![uplevel 1 namespace current]!"
	if {[llength $args] > 0} {
	    return -code error "wrong # args: should be \"info class\""
	}
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
# puts stderr "callContextInfo!$callContextInfo!"
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
	if {$objectClassName eq ""}  {
	    set className [set ::itcl::internal::infos::infoNamespace]
	}
	if {$className eq ""} {
	    set className [uplevel 1 namespace current]
	    namespace upvar ${::itcl::internal::infos::rootNamespace}::builtin::info infoNS infoNS
	    if {![::namespace exists ${infoNS}$className]} {
	        return -code error "$className is no class namespace"
	    } else {
                return [uplevel 1 namespace current]
	    }
	} else {
	    if {$objectClassName eq ""}  {
                return $className
	    } else {
                return $objectClassName
	    }
        }
    }

    proc component {args} {
# puts stderr "info component called!$args!"
    }

    proc function {args} {
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
# puts stderr "info function called!$args![namespace current]![uplevel 1 namespace current]!"
#puts stderr "className!$callContextInfo!"
#puts stderr "objectName!$objectName!className!$className!objectClassName!$objectClassName!namespaceName!$namespaceName!funcName!$funcName!"
	namespace upvar ${::itcl::internal::infos::rootNamespace}::builtin::info infoNS infoNS
	if {![dict exists [set ${infoNS}${objectClassName}::infos] functions]} {
	    set err "no such class \"$className\"
get info like this instead: 
  namespace eval className { info ", name, "... }"
	    return -code error -level 3 $err
	}
        set inh [dict get [set ${infoNS}${objectClassName}::infos] inheritance]
	set result [list]
	if {$inh ne ""} {
	    set inh [concat [list $objectClassName] $inh]
	} else {
	    set inh [list $objectClassName]
	}
	set found 0
        switch [llength $args] {
	0 {
# puts stderr "INH!$inh!"
	    foreach className $inh {
                set functions [dict get [set ${infoNS}${className}::infos] functions]
                foreach {name info} $functions {
		    lappend result ${className}::$name
                }
	    }
	    lappend result ${className}::cget
	    lappend result ${className}::configure
	    lappend result ${className}::isa
	    set found 1
	  }
        1 {
	    set arg0 [lindex $args 0]
	    set funcName [::namespace tail $arg0]
	    set prefix [::namespace qualifiers $arg0]
	    if {[string length $prefix] > 0} {
	        if {$prefix ne "::"} {
		    set prefix ::$prefix
		}
	    }
	    set lastClassName [lindex $inh end]
	    switch $funcName {
	    cget {
	        return [list public method ${lastClassName}::cget -option @itcl-builtin-cget]
	      }
	    configure {
	        return [list public method ${lastClassName}::configure {?-option? ?value -option value...?} @itcl-builtin-configure]
	      }
	    isa {
	        return [list public method ${lastClassName}::isa class or object @itcl-builtin-isa]
	      }
	    }
	    foreach className $inh {
		if {[string length $prefix] > 0} {
		    if {$prefix ne $className} {
		        continue
		    }
		}
                set functions [dict get [set ${infoNS}${className}::infos] functions]
                foreach {name info} $functions {
		    if {$name eq $funcName} {
# puts stderr "FUNC!$name!$info!"
			lappend result [dict get $info protection]
			set type [dict get $info type]
			lappend result $type
		        lappend result ${className}::$name
			set arguments [dict get $info arguments]
			set arglist [dict get $arguments definition]
			set body [dict get $info body]
			set state [dict get $info state]
			switch $state {
			COMPLETE {
			    lappend result $arglist
			    lappend result $body
			  }
			NO_BODY {
			    lappend result $arglist
			    lappend result "<undefined>"
			  }
			NO_ARGS {
			    lappend result "<undefined>"
			    lappend result "<undefined>"
			  }
			}
			set found 1
		        break
		    }
                }
	        if {$found} {
		    break
		}
	    }
	  }
        default {
	    set funcName [lindex $args 0]
	    set args [lrange $args 1 end]
	    foreach className $inh {
                set functions [dict get [set ${infoNS}${className}::infos] functions]
                foreach {name info} $functions {
		    if {$name eq $funcName} {
		        set type [dict get $info type]
			set state [dict get $info state]
			set arguments [dict get $info arguments]
			set arglist [dict get $arguments definition]
			set body [dict get $info body]
			switch $state {
			COMPLETE {
			  }
			NO_BODY {
			    set body "<undefined>"
			  }
			NO_ARGS {
			    set arglist "<undefined>"
			    set body "<undefined>"
			  }
			}
			set type [dict get $info type]
			foreach opt $args {
			    set shOpt [string range $opt 1 end] ; # strip off -
			    switch $opt {
			    -type {
			        lappend result $type
			      }
			    -name {
		                lappend result ${className}::$name
			      }
			    -protection {
			        lappend result [dict get $info protection]
			      }
			    -args {
			        lappend result $arglist
			      }
			    -body {
			        lappend result $body
			      }
			    default {
			        return -code error "bad option \"$opt\": must be -args, -body, -name, -protection, or -type"
			      }
			    }
			}
			set found 1
		        break
		    }
                }
	        if {$found} {
		    break
		}
	    }
	  }
	}
	if {!$found} {
	    return -code error "no such function: \"$funcName\""
	}
        if {[llength $result] == 1} {
	    set result [lindex $result 0]
	}
# puts stderr "RES!$result!"
        return $result
    }

    proc heritage {args} {
# puts stderr "info heritage called!$args!"
        if {[llength $args] != 0} {
	    return -code error "wrong # args: should be \"info heritage\""
	}
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
# puts stderr "callContextInfo!$callContextInfo!"
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
	namespace upvar ${::itcl::internal::infos::rootNamespace}::builtin::info infoNS infoNS
	if {$className eq "::itcl::class"} {
	    set className $objectClassName
	}
        set inh [GetInheritance ${infoNS} ${className}]
#        set inh [dict get [set ${infoNS}${className}::infos] inheritance]
# puts stderr "inh!$className!$inh!"
        return [concat [list $className] $inh]
    }

    proc inherit {args} {
# puts stderr "info inherit called!$args!"
        if {[llength $args] != 0} {
	    return -code error "wrong # args: should be \"info inherit\""
	}
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
# puts stderr "callContextInfo!$callContextInfo!"
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
	if {$className eq "::itcl::class"} {
	    set className $objectClassName
	}
	namespace upvar ${::itcl::internal::infos::rootNamespace}::builtin::info infoNS infoNS
        set inh2 [GetInheritance ${infoNS} ${className}]
        set inh [dict get [set ${infoNS}${className}::infos] inheritance]
# puts stderr "inh!$className!$inh!$inh2!"
        return $inh
    }

    proc variable {args} {
        set callContextInfo [uplevel 1 ${::itcl::internal::infos::internalCmds}::getCallContextInfo]
	set objectClassName ""
        foreach {objectName className objectClassName namespaceName funcName} $callContextInfo break
# puts stderr "info variable called!$args![namespace current]![uplevel 1 namespace current]!"
# puts stderr "callContextInfo!$callContextInfo!"
#puts stderr "objectName!$objectName!className!$className!objectClassName!$objectClassName!namespaceName!$namespaceName!funcName!$funcName!"
	namespace upvar ${::itcl::internal::infos::rootNamespace}::builtin::info infoNS infoNS
	if {$objectClassName eq ""}  {
	    set objectClassName [set ::itcl::internal::infos::infoNamespace]
	}
	if {![dict exists [set ${infoNS}${objectClassName}::infos] variables]} {
	    set err "no such class \"$className\"
get info like this instead: 
  namespace eval className { info ", name, "... }"
	    return -code error -level 3 $err
	}
	set result [list]
        set inh [list $objectClassName]
	set className $objectClassName
        set inh2 [GetInheritance ${infoNS} ${className}]
# puts stderr "INH2!$inh2!"
        set inh [concat $inh $inh2]
# puts stderr "INH!$inh![llength $args]!"
	set found 0
        switch [llength $args] {
	0 {
	    foreach className $inh {
                set variables [dict get [set ${infoNS}${className}::infos] variables]
                foreach {name info} $variables {
		    lappend result ${className}::$name
                }
	    }
	    lappend result ${objectClassName}::this
	    set found 1
	  }
        1 {
	    set arg0 [lindex $args 0]
	    set varName [::namespace tail $arg0]
	    set prefix [::namespace qualifiers $arg0]
	    if {[string length $prefix] > 0} {
	        if {$prefix ne "::"} {
		    set prefix ::$prefix
		}
	    }
	    if {$varName eq "this"} {
	        return [list protected variable ${objectClassName}::this $objectName $objectName]
	    }
	    foreach className $inh {
		if {[string length $prefix] > 0} {
		    if {$prefix ne $className} {
		        continue
		    }
		}
# puts stderr "INFOVAR!${className}!"
                set variables [dict get [set ${infoNS}${className}::infos] variables]
                foreach {name info} $variables {
		    if {$name eq $varName} {
set varNs ::itcl::internal::variables
			set protection [dict get $info protection]
			lappend result $protection
			set type [dict get $info type]
			lappend result $type
		        lappend result ${className}::$name
			set init [dict get $info init]
			set config [dict get $info config]
			set state [dict get $info state]
# puts stderr "VI!$name!$init!$config!$state!$type!"
			if {$type eq "common"} {
			    if {$protection eq "public"} {
			        if {[catch {
			            set instVarValue [set ${className}::$varName]
			        } msg]} {
			            set instVarValue "<undefined>"
			        }
			    } else {
append varNs $className
			        if {[catch {
			            set instVarValue [set ${varNs}::$varName]
			        } msg]} {
			            set instVarValue "<undefined>"
			        }
			    }
			} else {
                          if {$namespaceName eq ""} {
			      return -code error "cannot access object-specific info without an object context"
			  }
append varNs ${objectName}$className
			  if {[catch {
			    set instVarValue [set ${varNs}::$varName]
#			    set instVarValue [${::itcl::internal::infos::internalCmds}::getInstanceVarValue $name ""]
			  } msg]} {
puts stderr "MSG!$name!$msg!"
			    set instVarValue "<undefined>"
			  } 
			}
			switch $state {
			COMPLETE {
			    lappend result $init
			    lappend result $config
			  }
			NO_CONFIG {
			    lappend result $init
			    if {$protection eq "public"} {
			        if {$type ne "common"} {
			            lappend result {}
			        }
			    }
			  }
			NO_INIT {
			    lappend result "<undefined>"
			  }
			}
		        lappend result $instVarValue
			set found 1
		        break
		    }
                }
	        if {$found} {
		    break
		}
	    }
	  }
        default {
	    set varName [lindex $args 0]
	    set args [lrange $args 1 end]
	    foreach className $inh {
                set variables [dict get [set ${infoNS}${className}::infos] variables]
                foreach {name info} $variables {
		    if {$name eq $varName} {
set varNs ::itcl::internal::variables
		        set type [dict get $info type]
			set state [dict get $info state]
		        set protection [dict get $info protection]
			if {$type eq "common"} {
			    if {$protection eq "public"} {
			        if {[catch {
			            set instVarValue [set ${className}::$varName]
			        } msg]} {
			            set instVarValue "<undefined>"
			        }
			    } else {
append varNs $className
			        if {[catch {
			            set instVarValue [set ${varNs}::$varName]
			        } msg]} {
puts stderr "MSG!${className}::$varName!$msg!"
			            set instVarValue "<undefined>"
			        }
			    }
			} else {
                          if {$namespaceName eq ""} {
			      return -code error "cannot access object-specific info without an object context"
			  }
append varNs ${objectName}$className
			  if {[catch {
			    set instVarValue [set ${varNs}::$varName]
#			    set instVarValue [${::itcl::internal::infos::internalCmds}::getInstanceVarValue $name ""]
			  } msg]} {
puts stderr "MSG!$name!$msg!"
			    set instVarValue "<undefined>"
			  } 
			}
			switch $state {
			COMPLETE {
			    set init [dict get $info init]
			    set config [dict get $info config]
			  }
			NO_CONFIG {
			    set init [dict get $info init]
			    set config ""
			  }
			NO_INIT {
			    set init "<undefined>"
			    set config ""
			  }
			}
			set type [dict get $info type]
			foreach opt $args {
			    set shOpt [string range $opt 1 end] ; # strip off -
			    switch $opt {
			    -type {
			        lappend result $type
			      }
			    -name {
		                lappend result ${className}::$name
			      }
			    -protection {
			        lappend result $protection
			      }
			    -init {
			        lappend result $init
			      }
			    -config {
			        lappend result $config
			      }
			    -value {
			        lappend result $instVarValue
			      }
			    default {
			        return -code error "bad option \"$opt\": must be -config, -init, -name, -protection, -type, or -value"
			      }
			    }
			}
			set found 1
		        break
		    }
                }
	        if {$found} {
		    break
		}
	    }
	  }
	}
	if {!$found} {
	    return -code error "no such variable: \"$varName\""
	}
        if {[llength $result] == 1} {
	    set result [lindex $result 0]
	}
# puts stderr "RES!$result!"
        return $result
    }

    proc vars {args} {
# puts stderr "info vars called!$args!"
        set lst [uplevel 1 ::info vars]
#        set locals [uplevel 1 ::info locals]
        return $lst
    }

    proc exists {args} {
# puts stderr "info exists called!$args!"
        return [uplevel 1 ::info exists $args]
    }
}
