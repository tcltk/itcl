::oo::class create ${::itcl::internal::infos::rootClassName}
::oo::define ${::itcl::internal::infos::rootClassName} superclass ::oo::class
::oo::define ${::itcl::internal::infos::rootClassName} self.method create {className args} {
#puts stderr "SELF.CREATE!$className!"
    ::set infosNsName ${::itcl::internal::infos::rootNamespace}::internal::parseinfos
    ::set internalCmds $::itcl::internal::infos::internalCmds

    if {$className eq ""} {
        return -code error "invalid class name \"\""
    }
    set nsName [uplevel 1 namespace current]
    if {$nsName ne "::"} {
        set nsName ${nsName}::
    }
    if {[string match "::*" $className]} {
        set fullClassName $className
    } else {
        set fullClassName $nsName$className
    }
    set infoNs ${::itcl::internal::infos::internalClassInfos}$fullClassName
    if {[::namespace exists $infoNs]} {
	if {$fullClassName ne "$::itcl::internal::infos::rootClassName"} {
            return -code error -level 2 "class \"$className\" already exists"
        }
    }
    if {[::info comm $fullClassName] ne ""} {
	if {$fullClassName ne "$::itcl::internal::infos::rootClassName"} {
            return -code error -level 2 "command \"$className\" already exists"
        }
    }
    set ${infosNsName}::currClassName $className
    set ${infosNsName}::currFullClassName $fullClassName
    if {$fullClassName eq "$::itcl::internal::infos::rootClassName"} {
        set fromClassName ::oo::class
    } else {
        set fromClassName $::itcl::internal::infos::rootClassName
    }
    $internalCmds createClass $fullClassName $fromClassName
    namespace eval $infoNs {}
    set infoNs ${infoNs}::infos
    set $infoNs [list]
    dict set ${infoNs} functions [list]
    dict set ${infoNs} variables [list]
    dict set ${infoNs} options [list]
    dict set ${infoNs} inheritance [list]
    set result 0
    if {[catch {
        namespace eval ${::itcl::internal::infos::rootNamespace}::parser {*}$args
    } errs]} {
        set result 1
    }
    $internalCmds createClassFinish $fullClassName $result
    if {$result} {
        return -code error -level 1 $errs
    }
    set inh [dict get [set $infoNs] inheritance]
    if {[llength $inh] == 0} {
        if {$fullClassName ne "$::itcl::internal::infos::rootClassName"} {
            ::oo::define $fullClassName superclass $::itcl::internal::infos::rootClassName
        }
    }
    ::oo::define $fullClassName self.method unknown {args} { 
        return [uplevel 1 ${::itcl::internal::infos::rootNamespace}::builtin::unknown {*}$args]
    }
    ::oo::define $fullClassName export unknown
#puts stderr "SELF.CREATE END!$fullClassName!"
    return $fullClassName
}

::oo::define $::itcl::internal::infos::rootClassName self.export create
# need to create namespace by hand, as the class is not created fully as usual!
namespace eval $::itcl::internal::infos::rootClassName {}

# this time we create the class again so Itcl knows about it too!!
$::itcl::internal::infos::rootClassName create $::itcl::internal::infos::rootClassName {
    protected common envNs ""
    public proc info {args} {
puts stderr "class!info!$args![uplevel 1 namespace current]!" ;
	if {[llength $args] == 0} {
	    return -code error "wrong # args: should be one of...
  info args procname
  info body procname
  info class
  info component ?name? ?-inherit? ?-value?
  info function ?name? ?-protection? ?-type? ?-name? ?-args? ?-body?
  info heritage
  info inherit
  info option ?name? ?-protection? ?-resource? ?-class? ?-name? ?-default? ?-cgetmethod? ?-configuremethod? ?-validatemethod? ?-value?
  info variable ?name? ?-protection? ?-type? ?-name? ?-init? ?-value? ?-config?
...and others described on the man page"
	}
set ::itcl::internal::infos::infoNamespace [uplevel 1 namespace current]
	return [uplevel 1 ${::itcl::internal::infos::rootNamespace}::builtin::info {*}$args]
    }

    public method cget {args} {
	return [uplevel 1 ${::itcl::internal::infos::internalCmds}::cget {*}$args]
    }

    public method configure {args} {
	set nsName [namespace current]
	set nsName [uplevel 1 namespace current]
	set contextInfo [${::itcl::internal::infos::internalCmds}::getContext]
	return [namespace eval $nsName " ${::itcl::internal::infos::internalCmds}::configure $args"]
    }

    public method isa {args} {
	set nsName [namespace current]
	set nsName [uplevel 1 namespace current]
	set contextInfo [${::itcl::internal::infos::internalCmds}::getContext]
	return [namespace eval $nsName " ${::itcl::internal::infos::internalCmds}::isa $args"]
    }

    public proc create {args} {
#puts stderr "CREATE![lindex $args 0]!"
	set result [uplevel 1 ${::itcl::internal::infos::rootNamespace}::builtin::create [self] {*}$args]
#puts stderr "CREATE END!$result!"
        return $result
    }
}
