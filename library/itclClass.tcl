proc ::tryHandleClass {myObj mySelf m args} {
#if {[catch [list 
return [::itcl::parser::handleClass $myObj $mySelf $m {*}[list $args]]
#] msg errInfo]} {
#puts stderr "111!$msg!"
#puts stderr "EI2!$msg!$errInfo![dict get $errInfo -errorinfo]!"
#puts stderr "EI2!$msg![dict get $errInfo -errorinfo]!"
#return -code error -errorInfo [dict get $errInfo -errorinfo]
#    return -code error $msg
#} else {
#puts stderr "222!$msg!"
#    return $msg
#}
#puts stderr "333"
}
# ========================== class method unknown ==========================

::oo::define $itclClass method unknown {m args} {
#puts stderr "----CLASS UNKNOWN!$m!args!$args!"
    set mySelf [::oo::Helpers::self]
#puts stderr "mySelf!$mySelf![uplevel 1 namespace current]![::itcl::is class $mySelf]!"
    if {[::itcl::is class $mySelf]} {
        set namespace [uplevel 1 namespace current]
        set my_namespace $namespace
        if {$my_namespace ne "::"} {
            set my_namespace ${my_namespace}::
        }
        set my_class [::itcl::find classes ${my_namespace}$m]
        if {[string length $my_class] > 0} {
            # class already exists, it is a redefinition, so delete old class first
puts stderr "::itcl::delete class $my_class"
	    ::itcl::delete class $my_class
        }
        set cmd [uplevel 1 ::info command ${my_namespace}$m]
#puts stderr "CMD!$cmd!${my_namespace}$m!"
        if {[string length $cmd] > 0} {
            error "command \"$m\" already exists in namespace \"$namespace\""
        }
#puts stderr "++++++++ $mySelf make object $m"
    } 
    set myns [uplevel namespace current]
#puts stderr "itcl::class unknown $myns $m"
    if {$myns ne "::"} {
       set myns ${myns}::
    }
    set myObj [lindex [::info level 0] 0]
    set cmd [list uplevel 1 ::itcl::parser::handleClass $myObj $mySelf $m {*}[list $args]]
    if {[catch {
        eval $cmd
    } obj errInfo]} {
	return -code error -errorinfo $::errorInfo $obj
    }
#    set obj [uplevel 1 ::tryHandleClass $myObj $mySelf $m {*}[list $args]]
    set body [list {set obj [lindex [::info level 0] 0]}]
    set part {return [::itcl::methodset::objectUnknownCommand [self]}
    append part " ${myns}::$obj "
    append part {{*}$args]}
    lappend body $part
#puts stderr "BODY!$myns!$body!"
    catch {
        # just in case object has been destructed during construction
    ::oo::define ${myns}::$obj method unknown {args} "[join $body \n]"
    ::oo::define ${myns}::$obj export unknown
    }
    return $obj
}
::oo::define $itclClass export unknown
