namespace eval foo {
    namespace export *

    proc test {args} {
        return "test: $args"
    }
}

namespace import foo::*
namespace eval bar {
    namespace import ::foo::*
}

puts "ORIGINAL:"
puts "global => [test 1 2 3]"
puts "   bar => [namespace eval bar {test 1 2 3}]"

namespace eval foo {
    itcl::class test { }
}

puts "REPLACED:"
puts "global => [test 1 2 3]"
puts "   bar => [namespace eval bar {test 1 2 3}]"
