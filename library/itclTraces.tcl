namespace eval ::itcl {
  namespace eval traces {

    proc onRename {obj oldName newName type} {
puts stderr "onRename called for: $obj $oldName $newName $type!"
    }

    proc onDelete {obj oldName newName type} {
puts stderr "onDelete called for: $obj $oldName $newName $type!"
    }
    
  }
}
