#
# Labeledframe
# ----------------------------------------------------------------------
# Implements a hull frame with a grooved relief, a label, and a
# frame childsite.
# 
#
# The frame childsite can be filled with any widget via a derived class
# or though the use of the childsite method.  This class was designed
# to be a general purpose base class for supporting the combination of
# a labeled frame and a childsite.  The options include the ability to
# position the label at configurable locations within the grooved relief
# of the hull frame, and control the display of the label.
#
#  To following demonstrates the different values which the "-labelpos"
#  option may be set to and the resulting layout of the label when
#  one executes the following command with "-labeltext" set to "LABEL":
#
#  example:
#   labeledframe .w -labeltext LABEL -labelpos <ne,n,nw,se,s,sw,en,e,es,wn,s,ws>
#
#      ne          n         nw         se          s         sw
#
#   *LABEL****  **LABEL**  ****LABEL*  **********  ********* **********
#   *        *  *       *  *        *  *        *  *       * *        *  
#   *        *  *       *  *        *  *        *  *       * *        *  
#   *        *  *       *  *        *  *        *  *       * *        *
#   **********  *********  **********  *LABEL****  **LABEL** ****LABEL*
#
#      en          e         es         wn          s         ws
#
#   **********  *********  *********  *********  *********  **********
#   *        *  *        * *       *  *        * *       *  *        *
#   L        *  *        * *       *  *        L *       *  *        *
#   A        *  L        * *       *  *        A *       L  *        L
#   B        *  A        * L       *  *        B *       A  *        A
#   E        *  B        * A       *  *        E *       B  *        B
#   L        *  E        * B       *  *        L *       E  *        E
#   *        *  L        * E       *  *        * *       L  *        L
#   *        *  *        * L       *  *        * *       *  *        *
#   **********  ********** *********  ********** *********  **********
#
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package Labeledframe
# written by:
#  AUTHOR: John A. Tucker               EMAIL: jatucker@spd.dsccc.com
#    Copyright (c) 1995 DSC Technologies Corporation
# ----------------------------------------------------------------------
#
#   @(#) $Id: labeledframe.tcl,v 1.1.2.1 2009/01/10 17:09:09 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Labeledframe.labelMargin    10      widgetDefault
option add *Labeledframe.labelFont     \
      "-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-*"  widgetDefault
option add *Labeledframe.labelPos       n       widgetDefault
option add *Labeledframe.borderWidth    2      widgetDefault
option add *Labeledframe.relief         groove widgetDefault


namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Labeledframe class.
# 
proc ::itcl::widgets::labeledframe {pathName args} {
    uplevel ::itcl::widgets::Labeledframe $pathName $args
}

::itcl::extendedclass Labeledframe {
    component itcl_hull
    component itcl_interior
    component childsite
    component label

    option [list -ipadx iPadX IPad] -default 0 -configuremethod configIpadx
    option [list -ipady iPadY IPad] -default 0 -configuremethod configIpady
    option [list -labelmargin labelMargin LabelMargin] -default 10 -configuremethod configLabelmargin
    option [list -labelpos labelPos LabelPos] -default n -configuremethod configLabelpos
  
    delegate option [list -background background Background] to itcl_hull as -highlightbackground
    delegate option [list -background background Background] to itcl_hull as -highlightcolor
    delegate option [list -labelbitmap labelBitmap Bitmap] to label as -bitmap
    delegate option [list -labelfont labelFont Font] to label as -font
    delegate option [list -labelimage labelImage Image] to label as -image
    delegate option [list -labeltext labelText Text] to label as -text
    delegate option [list -labelvariable labelVariable Variable] to label as -textvariable

    private variable _reposition ""  ;# non-null => _positionLabel pending
    private variable itk_hull ""

    private common _LAYOUT_TABLE 

    constructor {args} {}
    destructor {}

    private proc _initTable {}

    protected method _positionLabel {{when later}}
    protected method _collapseMargin {}
    protected method _setMarginThickness {value}
    protected method smt {value} { _setMarginThickness $value }
    protected method configIpadx {option value}
    protected method configIpady {option value}
    protected method configLabelmargin {option value}
    protected method configLabelpos {option value}

    public method childsite {}
}

# -----------------------------------------------------------------------------
#                        CONSTRUCTOR
# -----------------------------------------------------------------------------
::itcl::body Labeledframe::constructor { args } {
    createhull frame $this -class [info class] -relief groove
    keepcomponentoption itcl_hull -background -cursor -relief -borderwidth
# FIXME   bind itk-delete-$itk_hull <Destroy> "itcl::delete object $this"
# FIXME  set tags [bindtags $itcl_hull]
# FIXME  bindtags $itcl_hull [linsert $tags 0 itk-delete-$itk_hull]
    #
    # Create the childsite frame window
    # _______
    # |_____|
    # |_|X|_|
    # |_____|
    #
    setupcomponent childsite using frame $itcl_interior.childsite -highlightthickness 0 -bd 0
    #
    # Create the label to be positioned within the grooved relief
    # of the hull frame.
    #
    setupcomponent label using label $itcl_interior.label -highlightthickness 0 -bd 0
    keepcomponentoption label -background -cursor -labelfont -foreground
# FIXME    ignore -highlightthickness -highlightcolor
    grid $childsite -row 1 -column 1 -sticky nsew
    grid columnconfigure $itcl_interior 1 -weight 1
    grid rowconfigure    $itcl_interior 1 -weight 1
    bind $label <Configure> +[itcl::code $this _positionLabel]
    #
    # Initialize the class array of layout configuration options.  Since
    # this is a one time only thing.
    #
    _initTable
    uplevel 0 itcl_initoptions $args
    # 
    # When idle, position the label.
    #
    _positionLabel
}

# -----------------------------------------------------------------------------
#                           DESTRUCTOR
# -----------------------------------------------------------------------------
::itcl::body Labeledframe::destructor {} {
    if {$_reposition ne ""} {
        after cancel $_reposition
    }
    if {[winfo exists $win]} {
        set tags [bindtags $win]
# FIXME        set i [lsearch $tags itk-delete-$itk_hull]
# FIXME        if {$i >= 0} {
# FIXME            bindtags $win [lreplace $tags $i $i]
# FIXME        }
        destroy $win
    }
}

# -----------------------------------------------------------------------------
#                             OPTIONS
# -----------------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -ipadx
#
# Specifies the width of the horizontal gap from the border to the
# the child site.
# ------------------------------------------------------------------
::itcl::body Labeledframe::configIpadx {option value} {
    grid configure $childsite -padx $value
    set itcl_options($option) $value
    _positionLabel
}

# ------------------------------------------------------------------
# OPTION: -ipady
#
# Specifies the width of the vertical gap from the border to the
# the child site.
# ------------------------------------------------------------------
::itcl::body Labeledframe::configIpady {option value} {
    grid configure $childsite -pady $value
    set itcl_options($option) $value
    _positionLabel
}

# -----------------------------------------------------------------------------
# OPTION: -labelmargin
#
# Set the margin of the most adjacent side of the label to the hull
# relief.
# ----------------------------------------------------------------------------
::itcl::body Labeledframe::configLabelmargin {option value} {
    set itcl_options($option) $value
    _positionLabel
}

# -----------------------------------------------------------------------------
# OPTION: -labelpos
#
# Set the position of the label within the relief of the hull frame
# widget.
# ----------------------------------------------------------------------------
::itcl::body Labeledframe::configLabelpos {option value} {
    set itcl_options($option) $value
    _positionLabel
}

# -----------------------------------------------------------------------------
#                            PROCS
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# PRIVATE PROC: _initTable
#
# Initializes the _LAYOUT_TABLE common variable of the Labeledframe
# class.  The initialization is performed in its own proc ( as opposed
# to in the class definition ) so that the initialization occurs only
# once.
#
# _LAYOUT_TABLE common array description:
#   Provides a table of the configuration option values
#   used to place the label widget within the grooved relief of the hull
#   frame for each of the 12 possible "-labelpos" values.
#
#   Each of the 12 rows is layed out as follows:
#     {"-relx" "-rely" <rowconfigure|columnconfigure> <row/column number>}
# -----------------------------------------------------------------------------
::itcl::body Labeledframe::_initTable {} {
    array set _LAYOUT_TABLE {
        nw-relx 0.0  nw-rely 0.0  nw-wrap 0 nw-conf rowconfigure    nw-num 0
        n-relx  0.5  n-rely  0.0  n-wrap  0 n-conf  rowconfigure    n-num  0
        ne-relx 1.0  ne-rely 0.0  ne-wrap 0 ne-conf rowconfigure    ne-num 0

        sw-relx 0.0  sw-rely 1.0  sw-wrap 0 sw-conf rowconfigure    sw-num 2
        s-relx  0.5  s-rely  1.0  s-wrap  0 s-conf  rowconfigure    s-num  2
        se-relx 1.0  se-rely 1.0  se-wrap 0 se-conf rowconfigure    se-num 2

        en-relx 1.0  en-rely 0.0  en-wrap 1 en-conf columnconfigure en-num 2
        e-relx  1.0  e-rely  0.5  e-wrap  1 e-conf  columnconfigure e-num  2
        es-relx 1.0  es-rely 1.0  es-wrap 1 es-conf columnconfigure es-num 2

        wn-relx 0.0  wn-rely 0.0  wn-wrap 1 wn-conf columnconfigure wn-num 0
        w-relx  0.0  w-rely  0.5  w-wrap  1 w-conf  columnconfigure w-num  0
        ws-relx 0.0  ws-rely 1.0  ws-wrap 1 ws-conf columnconfigure ws-num 0
    }
    #
    # Since this is a one time only thing, we'll redefine the proc to be empty
    # afterwards so it only happens once.
    #
    # NOTE: Be careful to use the "body" command, or the proc will get lost!
    #
# FIXME    ::itcl::body Labeledframe::_initTable {} {}
}

# -----------------------------------------------------------------------------
#                            METHODS
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# PUBLIC METHOD:: childsite
#
# -----------------------------------------------------------------------------
::itcl::body Labeledframe::childsite {} {
    return $childsite
}

# -----------------------------------------------------------------------------
# PROTECTED METHOD: _positionLabel ?when?
#
# Places the label in the relief of the hull.  If "when" is "now", the
# change is applied immediately.  If it is "later" or it is not
# specified, then the change is applied later, when the application
# is idle.
# -----------------------------------------------------------------------------
::itcl::body Labeledframe::_positionLabel {{when later}} {
    if {$when eq "later"} {
        if {$_reposition eq ""} {
            set _reposition [after idle [itcl::code $this _positionLabel now]]
        }
        return
    } 
    set pos $itcl_options(-labelpos)
    #
    # If there is not an entry for the "relx" value associated with
    # the given "-labelpos" option value, then it invalid.
    #
    if {[catch {set relx $_LAYOUT_TABLE($pos-relx)}]} {
        error "bad labelpos option\"$itcl_options(-labelpos)\": should be\
                  nw, n, ne, sw, s, se, en, e, es, wn, w, or ws"
    }
    update idletasks
    $label configure -wraplength $_LAYOUT_TABLE($pos-wrap)
    set labelWidth [winfo reqwidth $label]
    set labelHeight [winfo reqheight $label]
    set borderwidth $itcl_options(-borderwidth)
    set margin $itcl_options(-labelmargin)
    switch $pos {
    nw {
        set labelThickness $labelHeight
        set minsize [expr {$labelThickness/2.0}]
        set xPos [expr {$minsize+$borderwidth+$margin}]
        set yPos -$minsize
      }
    n {
        set labelThickness $labelHeight
        set minsize [expr {$labelThickness/2.0}]
        set xPos [expr {-$labelWidth/2.0}]
        set yPos -$minsize
      }
    ne  {
        set labelThickness $labelHeight
        set minsize [expr {$labelThickness/2.0}]
        set xPos [expr {-($minsize+$borderwidth+$margin+$labelWidth)}]
        set yPos -$minsize
      }

    sw  {
        set labelThickness $labelHeight
        set minsize [expr {$labelThickness/2.0}]
        set xPos [expr {$minsize+$borderwidth+$margin}]
        set yPos -$minsize
      }
    s {
        set labelThickness $labelHeight
        set minsize [expr {$labelThickness/2.0}]
        set xPos [expr {-$labelWidth/2.0}]
        set yPos [expr {-$labelHeight/2.0}]
      }
    se {
        set labelThickness $labelHeight
        set minsize [expr {$labelThickness/2.0}]
        set xPos [expr {-($minsize+$borderwidth+$margin+$labelWidth)}]
        set yPos [expr {-$labelHeight/2.0}]
      }

    wn {
        set labelThickness $labelWidth
        set minsize [expr {$labelThickness/2.0}]
        set xPos -$minsize
        set yPos [expr {$minsize+$margin+$borderwidth}]
      }
    w {
        set labelThickness $labelWidth
        set minsize [expr {$labelThickness/2.0}]
        set xPos -$minsize
        set yPos [expr {-($labelHeight/2.0)}]
      }
    ws {
        set labelThickness $labelWidth
        set minsize [expr {$labelThickness/2.0}]
        set xPos -$minsize
        set yPos [expr {-($minsize+$borderwidth+$margin+$labelHeight)}]
      }

    en {
        set labelThickness $labelWidth
        set minsize [expr {$labelThickness/2.0}]
        set xPos -$minsize
        set yPos [expr {$minsize+$borderwidth+$margin}]
      }
    e {
        set labelThickness $labelWidth
        set minsize [expr {$labelThickness/2.0}]
        set xPos -$minsize
        set yPos [expr {-($labelHeight/2.0)}]
      }
    es {
        set labelThickness $labelWidth
        set minsize [expr {$labelThickness/2.0}]
        set xPos -$minsize
        set yPos [expr {-($minsize+$borderwidth+$margin+$labelHeight)}]
      }
    }
    _setMarginThickness $minsize
    place $label \
        -relx $_LAYOUT_TABLE($pos-relx) -x $xPos \
        -rely $_LAYOUT_TABLE($pos-rely) -y $yPos \
        -anchor nw
    set what $_LAYOUT_TABLE($pos-conf)
    set number $_LAYOUT_TABLE($pos-num)
    grid $what $itcl_interior $number -minsize $minsize
    set _reposition ""
}

# -----------------------------------------------------------------------------
# PROTECTED METHOD: _collapseMargin
#
# Resets the "-minsize" of all rows and columns of the hull's grid
# used to set the label margin to 0
# -----------------------------------------------------------------------------
::itcl::body Labeledframe::_collapseMargin {} {
    grid columnconfigure $itcl_interior 0 -minsize 0
    grid columnconfigure $itcl_interior 2 -minsize 0
    grid rowconfigure $itcl_interior 0 -minsize 0
    grid rowconfigure $itcl_interior 2 -minsize 0
}

# -----------------------------------------------------------------------------
# PROTECTED METHOD: _setMarginThickness
#
# Set the margin thickness ( i.e. the hidden "-highlightthickness"
# of the hull ) to the input value.
#
# The "-highlightthickness" option of the hull frame is not intended to be
# configured by users of this class, but does need to be configured to properly
# place the label whenever the label is configured.
#
# Therefore, since I can't find a better way at this time, I achieve this 
# configuration by: adding the "-highlightthickness" option back into
# the hull frame; configuring the "-highlightthickness" option to properly
# place the label;  and then remove the "-highlightthickness" option from the
# hull.
#
# This way the option is not visible or configurable without some hacking.
#
# -----------------------------------------------------------------------------
::itcl::body Labeledframe::_setMarginThickness {value} {
# FIXME    itcl_options add hull.highlightthickness
# FIXME    $win configure -highlightthickness $value
# FIXME    itcl_options remove hull.highlightthickness
}

} ; # end ::itcl::widgets
