namespace eval ::itcl::internal::infos {
    variable rootNamespace ::itcl
    variable rootClassName ${rootNamespace}::class
#    variable rootClassName ${rootNamespace}::type
    variable internalCmds ${rootNamespace}::internal::commands
    variable internalVars ${rootNamespace}::internal::variables
    variable internalClassInfos ${rootNamespace}::internal::classinfos

    namespace eval $internalCmds {}
    namespace eval $internalVars {}
    namespace eval $internalClassInfos {}
}


