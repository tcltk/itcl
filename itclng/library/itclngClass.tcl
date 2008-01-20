::oo::class create ${::itcl::internal::infos::rootClassName}
::oo::define ${::itcl::internal::infos::rootClassName} superclass ::oo::class
::oo::define ${::itcl::internal::infos::rootClassName} self.method create {className args} {
puts stderr "=== class!self!create!$className!"
    ::set infosNsName ${::itcl::internal::infos::rootNamespace}::internal::parseinfos
    ::set internalCmds $::itcl::internal::infos::internalCmds

    if {$className eq ""} {
        return -code error -level 2 "invalid classname \"\""
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
        return -code error -level 2 $errs
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
    return $fullClassName
}

::oo::define $::itcl::internal::infos::rootClassName self.export create
# need to create namespace by hand, as the class is not created fully as usual!
namespace eval $::itcl::internal::infos::rootClassName {}

# this time we create the class again so Itcl knows about it too!!
$::itcl::internal::infos::rootClassName create $::itcl::internal::infos::rootClassName {
    public proc info {args} {
puts stderr "class!info!" ;
	return [uplevel 1 ${::itcl::internal::infos::rootNamespace}::builtin::info {*}$args]
    }

    public method cget {args} {
puts stderr "class!cget!" ;
	return [uplevel 1 ${::itcl::internal::infos::internalCmds}::cget {*}$args]
    }

    public method configure {args} {
puts stderr "class!configure!" ;
	return [namespace eval ::arnulf::cl1 " ${::itcl::internal::infos::internalCmds}::configure $args"]
#	return [uplevel 2 ${::itcl::internal::infos::internalCmds}::configure {*}$args]
    }

    public proc create {args} {
puts stderr "+++class!create!" ;
	return [uplevel 1 ${::itcl::internal::infos::rootNamespace}::builtin::create [self] {*}$args]
    }
}
