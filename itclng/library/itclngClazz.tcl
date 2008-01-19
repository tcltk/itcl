::itclng::class create ::itclng::clazz {
    public proc info {args} {
puts stderr "clazz!info!" ;
	return [uplevel 1 ::itclng::builtin::info {*}$args]
    }

    public method cget {args} {
puts stderr "clazz!cget!" ;
	return [uplevel 1 ::itclng::builtin::cget {*}$args]
    }

    public method configure {args} {
puts stderr "clazz!configure!" ;
	return [uplevel 1 ::itclng::builtin::configure {*}$args]
    }

    public proc create {args} {
puts stderr "+++clazz!create!" ;
	return [uplevel 1 ::itclng::builtin::create [self] {*}$args]
    }
}
::oo::define ::itclng::clazz superclass ::oo::class

