namespace eval ::itclng::parser {
    proc parseMember {type args} {
	::set protection [lindex [::info level -2] 0]
	switch $protection {
	public -
	protected -
	private {
	  }
	default {
	    return -code error -level 2 "need protection keyword"
	  }
	}
	if {[llength $args] < 1} {
	    ::set frameInfo [::info frame 4]
	    ::set line [dict get $frameInfo line]
	    return -code error -level 4 -line $line "usage: $protection $type <name> ...\n     while executing \"$protection $type\""
	}
	::set name [lindex $args 0]
	::set args [lrange $args 1 end]
	::set fullClassName $::itclng::internal::parseinfos::currFullClassName
	::set infoNs ::itclng::internal::classinfos${fullClassName}::infos
puts stderr "parseMember!$fullClassName!!$protection!$type!$name!$args!"
        ::itclng::member::${type} $infoNs $fullClassName $protection $name {*}$args
    }
    proc parseSpecialMember {type args} {
	if {[llength $args] < 1} {
	    return -code error -level 2 "usage: $type <name> ..."
	}
	::set name [lindex $args 0]
	::set args [lrange $args 1 end]
        puts stderr "parseSpecialMember!$type!$name!$args!"
    }
    proc private {args} {
        if {[llength $args] == 1} {
	    uplevel 0 [lindex $args 0]
	} else {
	    uplevel 0 $args
	}
    }
    proc protected {args} {
        if {[llength $args] == 1} {
	    uplevel 0 [lindex $args 0]
	} else {
	    uplevel 0 $args
	}
    }
    proc public {args} {
        if {[llength $args] == 1} {
	    uplevel 0 [lindex $args 0]
	} else {
	    uplevel 0 $args
	}
    }
    proc variable {args} {
        parseMember variable {*}$args
    }
    proc common {args} {
        parseMember common {*}$args
    }
    proc method {args} {
        parseMember method {*}$args
    }
    proc methodvariable {args} {
        parseMember methodvariable {*}$args
    }
    proc option {args} {
        parseMember option {*}$args
    }
    proc component {args} {
        parseMember component {*}$args
    }
    proc constructor {args} {
        parseSpecialMember constructor {*}$args
    }
    proc destructor {args} {
        parseSpecialMember destructor {*}$args
    }
    proc filter {args} {
        parseSpecialMember filter {*}$args
    }
    proc mixin {args} {
        parseSpecialMember mixin {*}$args
    }
    proc forward {args} {
        parseSpecialMember forward {*}$args
    }
    proc delegate {args} {
        parseSpecialMember delegate {*}$args
    }
    proc inherit {args} {
        parseSpecialMember inherit {*}$args
    }
    proc set {name args} {
	::set fullClassName $::itclng::internal::parseinfos::currFullClassName
        namespace eval $fullClassName [list ::set $name $args]
    }
    proc proc {args} {
        parseMember proc {*}$args
    }
}
