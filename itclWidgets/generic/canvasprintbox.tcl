#
# CanvasPrintBox v1.5
# ----------------------------------------------------------------------
# Implements a print box for printing the contents of a canvas widget
# to a printer or a file. It is possible to specify page orientation, the
# number of pages to print the image on and if the output should be
# stretched to fit the page.
# 
# CanvasPrintBox is a "super-widget" that can be used as an
# element in ones own GUIs. It is used to print the contents
# of a canvas (called the source hereafter) to a printer or a
# file. Possible settings include: portrait and landscape orientation
# of the output, stretching the output to fit the page while maintaining
# a proper aspect-ratio and posterizing to enlarge the output to fit on
# multiple pages. A stamp-sized copy of the source will be shown (called
# the stamp hereafter) at all times to reflect the effect of changing
# the settings will have on the output.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package CanvasPrintBox
# written by:
# AUTHOR: Tako Schotanus               EMAIL: Tako.Schotanus@bouw.tno.nl
#                Copyright (c) 1995  Tako Schotanus
# ----------------------------------------------------------------------
#
#   @(#) $Id: canvasprintbox.tcl,v 1.1.2.1 2009/01/10 14:47:58 wiede Exp $
# ======================================================================

#
# Default resources.
#
option add *Canvasprintbox.filename	"canvas.ps"	widgetDefault
option add *Canvasprintbox.hPageCnt	1		widgetDefault
option add *Canvasprintbox.orient	landscape	widgetDefault
option add *Canvasprintbox.output	printer		widgetDefault
option add *Canvasprintbox.pageSize	A4		widgetDefault
option add *Canvasprintbox.posterize	0		widgetDefault
option add *Canvasprintbox.printCmd	lpr		widgetDefault
option add *Canvasprintbox.printRegion	""		widgetDefault
option add *Canvasprintbox.vPageCnt	1		widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Canvasprintbox class.
# 
proc ::itcl::widgets::canvasprintbox {args} {
    uplevel ::itcl::widgets::Canvasprintbox $args
}

#<
#
# CanvasPrintBox is a "super-widget" that can be used as an
# element in ones own GUIs. It is used to print the contents
# of a canvas (called the source hereafter) to a printer or a
# file. Possible settings include: portrait and landscape orientation
# of the output, stretching the output to fit the page while maintaining
# a proper aspect-ratio and posterizing to enlarge the output to fit on
# multiple pages. A stamp-sized copy of the source will be shown (called
# the stamp hereafter) at all times to reflect the effect of changing
# the settings will have on the output.
#
#>
::itcl::extendedclass Canvasprintbox {
    component itcl_hull
    component itcl_interior
    component canvasframe
    component canvs
    component outputom
    component printerrb
    component printeref
    component filerb
    component fileef
    component propsframe
    component paperom
    component orientom
    component stretchcb
    component postercb
    component hpcnt
    component vpcnt
    component pages

    #
    # Holds the current state for all check- and radiobuttons.
    #
    option [list -filename filename FileName] -default "canvas.ps" -configuremethod configFilename
    option [list -hpagecnt hPageCnt PageCnt] -default 1 -configuremethod configHpagecnt
    option [list -orient orient Orient] -default "landscape" -configuremethod configOrient
    option [list -output output Output] -default "printer" -configuremethod configOutput
    option [list -pagesize pageSize PageSize] -default "A4" -configuremethod configPagesize
    option [list -posterize posterize Posterize] -default 0 -configuremethod configPosterize
    option [list -printcmd printCmd PrintCmd] -default "" -configuremethod configPrintcmd
    option [list -printregion printRegion PrintRegion] -default "" -configuremethod configPrintregion
    option [list -stretch stretch Stretch] -default 0 -configuremethod configStretch
    option [list -vpagecnt vPageCnt PageCnt] -default 1 -configuremethod configVpagecnt

    delegate option [list -labelfont labelFont Font] to printerrb as -font
    delegate option [list -labelfont labelFont Font] to filerb as -font
    delegate option [list -labelfont labelFont Font] to paperom as -font
    delegate option [list -labelfont labelFont Font] to stretchcb as -font
    delegate option [list -labelfont labelFont Font] to postercb as -font
    delegate option [list -labelfont labelFont Font] to vpcnt as -font

    #
    # Just holds the names of some widgets/objects. "cwin" is used to
    # determine if the object is fully constructed and initialized.
    #
    variable cwin ""
    variable canvw ""
    #
    # The canvas we want to print. 
    #
    variable canvas ""
    #
    # Boolean indicating if the attribute "orient" is set
    # to landscape or not.
    #
    variable rotate 1
    #
    # Holds the configure options that were used to create this object.
    #
    variable init_opts ""
    #
    # The following attributes hold a list of lines that are
    # currently drawn on the "stamp" to show how the page(s) is/are
    # oriented. The first holds the vertical dividing lines and the
    # second the horizontal ones.
    #
    variable hlines ""
    variable vlines ""
    #
    # Updating is set when the thumbnail is being drawn. Settings
    # this to 0 while drawing is still busy will terminate the
    # proces.
    # Restart_update can be set to 1 when the thumbnail is being
    # drawn to force a redraw.
    #
    variable _reposition ""
    variable _update_attr_id ""

    protected common _globVar

    constructor {args} {}
    destructor {}

    protected method configFilename {option value}
    protected method configHpagecnt {option value}
    protected method configOrient {option value}
    protected method configOutput {option value}
    protected method configPagesize {option value}
    protected method configPosterize {option value}
    protected method configPrintcmd {option value}
    protected method configPrintregion {option value}
    protected method configStretch {option value}
    protected method configVpagecnt {option value}

    public method getoutput {}
    public method print {}
    public method refresh {}
    public method setcanvas {canv}
    public method stop {}
    public method _calc_poster_size {}
    public method _calc_print_region {}
    public method _calc_print_scale {}
    public method _mapEventHandler {}
    public method _update_attr {{when later}}
    public method _update_canvas {{when later}}

    public proc ezPaperInfo {size {attr ""} {orient "portrait"} {window ""}}
}

# ------------------------------------------------------------------
# CONSTRUCTOR
# ------------------------------------------------------------------
::itcl::body Canvasprintbox::constructor {args} {
    createhull frame $this -class [info class]
    set _globVar($this,output) printer
    set _globVar($this,printeref) ""
    set _globVar($this,fileef) "canvas.ps"
    set _globVar($this,hpc) 1
    set _globVar($this,vpc) 1
    set _globVar($this,postercb) 0
    set _globVar($this,stretchcb) 0

    setupcomponent canvasframe using frame $itcl_interior.f18 -bd 2
    setupcomponent canvs using canvas $canvasframe.c1 \
	    -bd 2 -relief sunken \
	    -scrollregion {0c 0c 10c 10c} \
	    -width 250
    pack $canvs -expand 1 -fill both
    setupcomponent outputom using ::itcl::widgets::Labeledframe \
            $itcl_interior.outputom \
	    -labelpos nw \
	    -labeltext "Output to"
    set cs [$outputom childsite]
    setupcomponent printerrb using radiobutton $cs.printerrb \
	    -text Printer \
	    -variable [itcl::scope _globVar($this,output)] \
	    -anchor w \
	    -justify left \
	    -value printer \
	    -command [itcl::code $this _update_attr]
    keepcomponentoption printerrb -background -cursor -textbackground -foreground
    setupcomponent printeref using ::itcl::widgets::entryfield $cs.printeref \
	    -labeltext "command:" \
	    -state normal \
	    -labelpos w \
	    -textvariable [itcl::scope _globVar($this,printeref)]
    setupcomponent filerb using radiobutton $cs.filerb \
	    -text File \
	    -justify left \
	    -anchor w \
	    -variable [itcl::scope _globVar($this,output)] \
	    -value file \
	    -command [itcl::code $this _update_attr]
    keepcomponentoption filerb -background -cursor -textbackground -foreground
    setupcomponent fileef using ::itcl::widgets::entryfield $cs.fileef \
	    -labeltext "filename:" \
	    -state disabled \
	    -labelpos w \
	    -textvariable [itcl::scope _globVar($this,fileef)]
    setupcomponent propsframe using ::itcl::widgets::Labeledframe \
            $itcl_interior.propsframe \
	    -labelpos nw \
	    -labeltext "Properties"
    set cs [$propsframe childsite]
    setupcomponent paperom using ::itcl::widgets::optionmenu $cs.paperom \
	    -labelpos w -cyclicon 1 \
	    -labeltext "Paper size:" \
	    -command [itcl::code $this refresh]
    keepcomponentoption paperom -background -cursor -textbackground -foreground
    uplevel 0 $paperom insert end [ezPaperInfo types]
    $paperom select A4
    setupcomponent orientom using ::itcl::widgets::radiobox \
            $itcl_interior.orientom \
	    -labeltext "Orientation" -command [itcl::code $this refresh]
    $orientom add landscape -text Landscape
    $orientom add portrait -text Portrait
    $orientom select 0
    setupcomponent stretchcb using checkbutton $cs.stretchcb \
		-relief flat \
		-text {Stretch to fit} \
		-justify left \
		-anchor w \
		-variable [itcl::scope _globVar($this,stretchcb)] \
		-command [itcl::code $this refresh]
    keepcomponentoption stretchcb -background -cursor -textbackground -foreground
    setupcomponent postercb using checkbutton $cs.postercb \
		-relief flat \
		-text Posterize \
		-justify left \
		-anchor w \
		-variable [itcl::scope _globVar($this,postercb)] \
		-command [itcl::code $this refresh]
    keepcomponentoption postercb -background -cursor -textbackground -foreground
    setupcomponent hpcnt using ::itcl::widgets::entryfield $cs.hpcnt \
		-labeltext on \
		-textvariable [itcl::scope _globVar($this,hpc)] \
		-validate integer -width 3 \
		-command [itcl::code $this refresh]
    setupcomponent vpcnt using ::itcl::widgets::entryfield $cs.vpcnt \
		-labeltext by \
		-textvariable [itcl::scope _globVar($this,vpc)] \
		-validate integer -width 3 \
		-command [itcl::code $this refresh]
    setupcomponent pages using label $cs.pages -text pages.
    keepcomponentoption pages -background -cursor -textbackground -foreground
    set init_opts $args
    grid $canvasframe -row 0 -column 0 -rowspan 4 -sticky nsew
    grid $propsframe  -row 0 -column 1 -sticky nsew
    grid $outputom    -row 1 -column 1 -sticky nsew
    grid $orientom    -row 2 -column 1 -sticky nsew
    grid columnconfigure $itcl_interior 0 -weight 1
    grid rowconfigure    $itcl_interior 3 -weight 1
    grid $printerrb -row 0 -column 0 -sticky nsw
    grid $printeref -row 0 -column 1 -sticky nsw
    grid $filerb    -row 1 -column 0 -sticky nsw
    grid $fileef    -row 1 -column 1 -sticky nsw
    ::itcl::widgets::Labeledwidget::alignlabels $printeref $fileef
    grid columnconfigure $outputom 1 -weight 1
    grid $paperom   -row 0 -column 0 -columnspan 2 -sticky nsw
    grid $stretchcb -row 1 -column 0 -sticky nsw
    grid $postercb  -row 2 -column 0 -sticky nsw
    grid $hpcnt     -row 2 -column 1 -sticky nsw
    grid $vpcnt     -row 2 -column 2 -sticky nsw
    grid $pages     -row 2 -column 3 -sticky nsw
    grid columnconfigure $propsframe 3 -weight 1
    uplevel 0 itcl_initoptions $args
    bind $pages <Map> +[itcl::code $this _mapEventHandler]
    bind $canvas <Configure> +[itcl::code $this refresh]
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

#<
# A list of four coordinates specifying which part of the canvas to print.
# An empty list means that the canvas' entire scrollregion should be
# printed. Any change to this attribute will automatically update the "stamp".
# Defaults to an empty list.
#>
::itcl::body Canvasprintbox::configPrintregion {option value} {
    if {($value ne "") && ([llength $value] != 4)} {
	error {bad option "printregion": should contain 4 coordinates}
    }
    set itcl_options($option) $value
    _update_canvas
}

#<
# Specifies where the postscript output should go: to the printer
# or to a file. Can take on the values "printer" or "file".
# The corresponding entry-widget will reflect the contents of
# either the printcmd attribute or the filename attribute.
#>
::itcl::body Canvasprintbox::configOutput {option value} {
    switch $value {
    file -
    printer {
	set _globVar($this,output) $value
      }
    default {
	error {bad output option \"$value\":\
		should be file or printer}
      }
    }
    set itcl_options($option) $value
    _update_attr
}

#<
# The command to execute when printing the postscript output.
# The command will get the postscript directed to its standard
# input. (Only when output is set to "printer")
#>
::itcl::body ::itcl::widgets::Canvasprintbox::configPrintcmd {option value} {
    set _globVar($this,printeref) $value
    set itcl_options($option) $value
    _update_attr
}

#<
# The file to write the postscript output to (Only when output
# is set to "file"). If posterizing is turned on and hpagecnt
# and/or vpagecnt is more than 1, x.y is appended to the filename
# where x is the horizontal page number and y the vertical page number.
#>
::itcl::body Canvasprintbox::configFilename {option value} {
    set _globVar($this,fileef) $value
    set itcl_options($option) $value
    _update_attr
}

#<
# The pagesize the printer supports. Changes to this attribute
# will be reflected immediately in the "stamp".
#>
::itcl::body Canvasprintbox::configPagesize {option value} {
    set opt [string tolower $value]
    set lst [string tolower [ezPaperInfo types]]
    if {[lsearch $lst $opt] == -1} {
	error "bad option \"pagesize\": should be one of: [ezPaperInfo types]"
    }
    $paperom select "*[string range $opt 1 end]"
    set itcl_options($option) $value
    _update_canvas
}

#<
# Determines the orientation of the output to the printer (or file).
# It can take the value "portrait" or "landscape" (default). Changes
# to this attribute will be reflected immediately in the "stamp".
#>
::itcl::body Canvasprintbox::configOrient {option value} {
    switch $itcl_options(-orient) {
    "portrait" -
    "landscape" {
	$orientom select $value
        set itcl_options($option) $value
	_update_canvas

      }
    default {
	error "bad orient option \"$value\":\
		should be portrait or landscape"
      }
    }
}

#<
# Determines if the output should be stretched to fill the
# page (as defined by the attribute pagesize) as large as
# possible. The aspect-ratio of the output will be retained
# and the output will never fall outside of the boundaries
# of the page.
#>
::itcl::body Canvasprintbox::configStretch {option value} {
    if {($value != 0) && ($value != 1)} {
	error {bad option "stretch": should be a boolean}
    }
    set _globVar($this,stretchcb) $value
    set itcl_options($option) $value
    _update_attr
}

#<
# Indicates if posterizing is turned on or not. Posterizing
# the output means that it is possible to distribute the
# output over more than one page. This way it is possible to
# print a canvas/region which is larger than the specified
# pagesize without stretching. If used in combination with
# stretching it can be used to "blow up" the contents of a
# canvas to as large as size as you want (See attributes:
# hpagecnt end vpagecnt). Any change to this attribute will
# automatically update the "stamp".
#>
::itcl::body Canvasprintbox::configPosterize {option value} {
    if {($value != "0") && ($value != "1")} {
	error "expected boolean but got \"$value\""
    }
    set _globVar($this,postercb) $value
    set itcl_options($option) $value
    _update_canvas
}

#<
# Is used in combination with "posterize" to determine over
# how many pages the output should be distributed. This
# attribute specifies how many pages should be used horizontaly.
# Any change to this attribute will automatically update the "stamp".
#>
::itcl::body Canvasprintbox::configHpagecnt {option value} {
    set _globVar($this,hpc) $value
    set itcl_options($option) $value
    _update_canvas
}

#<
# Is used in combination with "posterize" to determine over
# how many pages the output should be distributed. This
# attribute specifies how many pages should be used verticaly.
# Any change to this attribute will automatically update the "stamp".
#>
::itcl::body Canvasprintbox::configVpagecnt {option value} {
    set _globVar($this,vpc) $value
    set itcl_options($option) $value
    _update_canvas
}


# ---------------------------------------------------------------
# PUBLIC METHODS
#----------------------------------------------------------------

#<
# This is used to set the canvas that has to be printed.
# A stamp-sized copy will automatically be drawn to show how the
# output would look with the current settings.
#
# In:	canv - The canvas to be printed
# Out:	canvas (attrib) - Holds the canvas to be printed
#>	
::itcl::body Canvasprintbox::setcanvas {canv} {
    set canvas $canv
    _update_canvas
}

#<
# Returns the value of the -printercmd or -filename option
# depending on the current setting of -output.
#
# In:	itcl_options (attrib)
# Out:	The value of -printercmd or -filename
#>
::itcl::body Canvasprintbox::getoutput {} {
    switch $_globVar($this,output) {
    "file" {
	return $_globVar($this,fileef)
      }
    "printer" {
  	return $_globVar($this,printeref)
      }
    }
    return ""
}

#<
# Perfrom the actual printing of the canvas using the current settings of
# all the attributes.
#
# In:	itcl_options, rotate (attrib)
# Out:	A boolean indicating wether printing was successful
#>
::itcl::body Canvasprintbox::print {} {
    global env tcl_platform
    stop
    if {$itcl_options(-output) eq "file"} {
	set nm $_globVar($this,fileef)
	if {[string range $nm 0 1] eq "~/"} {
	    set nm "$env(HOME)/[string range $nm 2 end]"
	}
    } else {
	set nm "/tmp/xge[winfo id $canvas]"
    }
    set pr [_calc_print_region]
    set x1 [lindex $pr 0]
    set y1 [lindex $pr 1]
    set x2 [lindex $pr 2]
    set y2 [lindex $pr 3]
    set cx [expr {int(($x2 + $x1) / 2)}]
    set cy [expr {int(($y2 + $y1) / 2)}]
    if {!$itcl_options(-stretch)} {
	set ps [_calc_poster_size]
	set pshw [expr {int([lindex $ps 0] / 2)}]
	set pshh [expr {int([lindex $ps 1] / 2)}]
	set x [expr {$cx - $pshw}]
	set y [expr {$cy - $pshh}]
	set w [ezPaperInfo $itcl_options(-pagesize) pwidth $itcl_options(-orient) $cwin]
	set h [ezPaperInfo $itcl_options(-pagesize) pheight $itcl_options(-orient) $cwin]
    } else {
	set x $x1
	set y $y1
	set w [expr {($x2-$x1) / $_globVar($this,hpc)}]
	set h [expr {($y2-$y1) / $_globVar($this,vpc)}]
    }
    set i 0
    set px $x
    while {$i < $_globVar($this,hpc)} {
	set j 0
	set py $y
	while {$j < $_globVar($this,vpc)} {
	    set nm2 [expr {$_globVar($this,hpc) > 1 || $_globVar($this,vpc) > 1 ? "$nm$i.$j" : $nm}]
	    if {$itcl_options(-stretch)} {
		$canvas postscript \
		  -file $nm2 \
		  -rotate $rotate \
		  -x $px -y $py \
		  -width $w \
		  -height $h \
		  -pagex [ezPaperInfo $itcl_options(-pagesize) centerx] \
		  -pagey [ezPaperInfo $itcl_options(-pagesize) centery] \
		  -pagewidth [ezPaperInfo $itcl_options(-pagesize) pwidth $itcl_options(-orient)] \
		  -pageheight [ezPaperInfo $itcl_options(-pagesize) pheight $itcl_options(-orient)]
	    } else {
		$canvas postscript \
		  -file $nm2 \
		  -rotate $rotate \
		  -x $px -y $py \
		  -width $w \
		  -height $h \
		  -pagex [ezPaperInfo $itcl_options(-pagesize) centerx] \
		  -pagey [ezPaperInfo $itcl_options(-pagesize) centery]
	    }
	    if {$itcl_options(-output) == "printer"} {
		set cmd "$itcl_options(-printcmd) < $nm2"
		if {[catch {eval exec $cmd &}]} {
			return 0
		}
	    }
	    set py [expr {$py + $h}]
	    incr j
	}
	set px [expr {$px + $w}]
	incr i
    }
    return 1
}

#<
# Retrieves the current value for all edit fields and updates
# the stamp accordingly. Is useful for Apply-buttons.
#>
::itcl::body Canvasprintbox::refresh {} {
    stop
    _update_canvas
    return
}

#<
# Stops the drawing of the "stamp". I'm currently unable to detect
# when a Canvasprintbox gets withdrawn. It's therefore advised
# that you perform a stop before you do something like that.
#>
::itcl::body ::itcl::widgets::Canvasprintbox::stop {} {
    if {$_reposition ne ""} {
	after cancel $_reposition
	set _reposition ""
    }
    if {$_update_attr_id ne ""} {
	after cancel $_update_attr_id
	set _update_attr_id ""
    }
    return
}

# ---------------------------------------------------------------
# PROTECTED METHODS
#----------------------------------------------------------------

#
# Calculate the total size the output would be with the current
# settings for "pagesize" and "posterize" (and "hpagecnt" and
# "vpagecnt"). This size will be the size of the printable area,
# some space has been substracted to take into account that a
# page should have borders because most printers can't print on
# the very edge of the paper.
#
# In:	posterize, hpagecnt, vpagecnt, pagesize, orient (attrib)
# Out:	A list of two numbers indicating the width and the height
#	of the total paper area which will be used for printing
#	in pixels.
#
::itcl::body Canvasprintbox::_calc_poster_size {} {
    set tpw [expr {[ezPaperInfo $itcl_options(-pagesize) \
	pwidth $itcl_options(-orient) $cwin]*$_globVar($this,hpc)}]
    set tph [expr {[ezPaperInfo $itcl_options(-pagesize) \
	pheight $itcl_options(-orient) $cwin]*$_globVar($this,vpc)}]
    return "$tpw $tph"
}

#
# Determine which area of the "source" canvas will be printed.
# If "printregion" was set by the "user" this will be used and
# converted to pixel-coordinates. If the user didn't set it
# the bounding box that contains all canvas-items will be used
# instead.
#
# In:	printregion, canvas (attrib)
# Out:	Four floats specifying the region to be printed in
#	pixel-coordinates (topleft & bottomright).
#
::itcl::body Canvasprintbox::_calc_print_region {} {
    set printreg [expr {$itcl_options(-printregion) != "" 
	? $itcl_options(-printregion) : [$canvas bbox all]}]
    if {$printreg ne ""} {
	set prx1 [winfo fpixels $canvas [lindex $printreg 0]]
	set pry1 [winfo fpixels $canvas [lindex $printreg 1]]
	set prx2 [winfo fpixels $canvas [lindex $printreg 2]]
	set pry2 [winfo fpixels $canvas [lindex $printreg 3]]

	set res "$prx1 $pry1 $prx2 $pry2"
    } else {
	set res "0 0 0 0"
    }
    return $res
}

#
# Calculate the scaling factor needed if the output was
# to be stretched to fit exactly on the page (or pages).
# If stretching is turned off this will always return 1.0.
#
# In:	stretch (attrib)
# Out:	A float specifying the scaling factor.
#
::itcl::body Canvasprintbox::_calc_print_scale {} {
    if {$itcl_options(-stretch)} {
	set pr [_calc_print_region]
	set prw [expr {[lindex $pr 2] - [lindex $pr 0]}]
	set prh [expr {[lindex $pr 3] - [lindex $pr 1]}]
	set ps [_calc_poster_size]
	set psw [lindex $ps 0]
	set psh [lindex $ps 1]
	set sfx [expr {$psw / $prw}]
	set sfy [expr {$psh / $prh}]
	set sf [expr {$sfx < $sfy ? $sfx : $sfy}]
	return $sf
    } else {
	return 1.0
    }
}

#
# Schedule the thread that makes a copy of the "source"
# canvas to the "stamp".
#
# In:	cwin, canvas (attrib)
# Out:	-
#
::itcl::body Canvasprintbox::_update_canvas {{when later}} {
    if {($cwin ne "") || ($canvas ne "") || ([$canvas find all] eq "")} {
	return
    }
    if {$when eq "later"} {
	if {$_reposition eq ""} {
	    set _reposition [after idle [itcl::code $this _update_canvas now]]
	}
	return
    }
    _update_attr now
    #
    # Make a copy of the "source" canvas to the "stamp".
    #
    if {($_globVar($this,hpc) == [llength $vlines]) &&
            ($_globVar($this,vpc) == [llength $hlines])} {
	stop
	return
    }	
    $canvw delete all
    set width  [winfo width $canvw]
    set height [winfo height $canvw]
    set ps [_calc_poster_size]
    #
    # Calculate the scaling factor that would be needed to fit the
    # whole "source" into the "stamp". This takes into account the
    # total amount of "paper" that would be needed to print the
    # contents of the "source".
    #
    set xsf [expr {$width/[lindex $ps 0]}]
    set ysf [expr {$height/[lindex $ps 1]}]
    set sf [expr {$xsf < $ysf ? $xsf : $ysf}]
    set w [expr {[lindex $ps 0]*$sf}]
    set h [expr {[lindex $ps 1]*$sf}]
    set x1 [expr {($width-$w)/2}]
    set y1 [expr {($height-$h)/2}]
    set x2 [expr {$x1+$w}]
    set y2 [expr {$y1+$h}]
    set cx [expr {($x2+$x1)/ 2}]
    set cy [expr {($y2+$y1)/ 2}]
    set printreg [_calc_print_region]
    set prx1 [lindex $printreg 0]
    set pry1 [lindex $printreg 1]
    set prx2 [lindex $printreg 2]
    set pry2 [lindex $printreg 3]
    set prcx [expr {($prx2+$prx1)/2}]
    set prcy [expr {($pry2+$pry1)/2}]
    set psf [_calc_print_scale]
    #
    # Copy all items from the "real" canvas to the canvas
    # showing what we'll send to the printer. Bitmaps and
    # texts are not copied because they can't be scaled,
    # a rectangle will be created instead.
    #
    set tsf [expr {$sf * $psf}]
    set dx [expr {$cx-($prcx*$tsf)}]
    set dy [expr {$cy-($prcy*$tsf)}]
    $canvw create rectangle \
	[expr {$x1+0}] \
	[expr {$y1+0}] \
	[expr {$x2-0}] \
	[expr {$y2-0}] -fill white
    set items [eval "$canvas find overlapping $printreg"]
    set itemCount [llength $items]
    for {set cnt 0} {$cnt < $itemCount} {incr cnt} {
	#
	# Determine the item's type and coordinates
	#
	set i [lindex $items $cnt]
	set t [$canvas type $i]
	set crds [$canvas coords $i]
	# Ask for the item's configuration settings and strip
	# it to leave only a list of option names and values.
	#
	set cfg [$canvas itemconfigure $i]
	set cfg2 ""
	foreach c $cfg {
	    if {[llength $c] == 5} {
		lappend cfg2 [lindex $c 0] [lindex $c 4]
	    }
	}
	#
	# Handle texts and bitmaps differently: they will
	# be represented as rectangles.
	#
	if {($t eq "text") || ($t eq "bitmap") || ($t eq "window")} {
	    set t "rectangle"
	    set crds [$canvas bbox $i]
	    set cfg2 "-outline {} -fill gray"
	}
	#
	# Remove the arrows from a line item when the scale
	# factor drops below 1/3rd of the original size.
	# This to prevent the arrowheads from dominating the
	# display.
	#
	if {($t eq "line") && ($tsf < 0.33)} {
		lappend cfg2 -arrow none
	}
	#
	# Create a copy of the item on the "printing" canvas.
	#
	set i2 [eval "$canvw create $t $crds $cfg2"]
	$canvw scale $i2 0 0 $tsf $tsf
	$canvw move $i2 $dx $dy
	if {($cnt%25) == 0} {
	    update
	}
	if {$_reposition == ""} {
	    return
	}
    }
    set p $x1
    set i 1
    set vlines {}
    while {$i < $_globVar($this,hpc)} {
	set p [expr {$p + ($w/$_globVar($this,hpc))}]
	set l [$canvw create line $p $y1 $p $y2]
	lappend vlines $l
	incr i
    }
    set p $y1
    set i 1
    set vlines {}
    while {$i < $_globVar($this,vpc)} {
	set p [expr {$p + ($h/$_globVar($this,vpc))}]
	set l [$canvw create line $x1 $p $x2 $p]
	lappend vlines $l
	incr i
    }
    set _reposition ""
}

#
# Update the attributes to reflect changes made in the user-
# interface.
#
# In:	itcl_options (attrib) - the attributes to update
#	component (attrib) - the widgets
#	_globVar (common) - the global var holding the state
#		of all radiobuttons and checkboxes.
# Out:	-
#
::itcl::body Canvasprintbox::_update_attr {{when "later"}} {
    if {$when ne "now"} {
	if {$_update_attr_id eq ""} {
	    set _update_attr_id [after idle [itcl::code $this _update_attr now]]
	}
	return
    }
    set itcl_options(-printcmd)  $_globVar($this,printeref)
    set itcl_options(-filename)  $_globVar($this,fileef)
    set itcl_options(-output)    $_globVar($this,output)
    set itcl_options(-pagesize)  [string tolower [$paperom get]]
    set itcl_options(-stretch)   $_globVar($this,stretchcb)
    set itcl_options(-posterize) $_globVar($this,postercb)
    set itcl_options(-vpagecnt)  $_globVar($this,vpc)
    set itcl_options(-hpagecnt)  $_globVar($this,hpc)
    set itcl_options(-orient)    [$orientom get]
    set rotate [expr {$itcl_options(-orient) == "landscape"}]
    if {$_globVar($this,output) == "file"} {
	$fileef configure \
		-state normal -foreground $itcl_options(-foreground)
	$printeref configure \
		-state disabled -foreground $itcl_options(-disabledforeground)
    } else {
	$fileef configure \
		-state disabled -foreground $itcl_options(-disabledforeground)
	$printeref configure \
		-state normal -foreground $itcl_options(-foreground)
    }
    set fg [expr {$_globVar($this,postercb) \
	? $itcl_options(-foreground) : $itcl_options(-disabledforeground)}]
    $vpcnt configure -foreground $fg
    $hpcnt configure -foreground $fg
    $pages configure -foreground $fg
    #
    # Update dependencies among widgets. (For example: disabling
    # an entry-widget when its associated checkbox-button is used
    # to turn of the option (the entry's value is not needed
    # anymore and this should be reflected in the fact that it
    # isn't possible to change it anymore).
    #
    # former method:_update_widgets/_update_UI
    #
    set state [expr {$itcl_options(-posterize) ? "normal" : "disabled"}]
    $vpcnt configure -state $state
    $hpcnt configure -state $state
    $paperom select "*[string range $itcl_options(-pagesize) 1 end]"
    set _update_attr_id ""
}

#
# Gets called when the CanvasPrintBox-widget gets mapped.
#
::itcl::body Canvasprintbox::_mapEventHandler {} {
    set cwin $itcl_interior
    set canvw $icanvas
    if {$canvas ne ""} {
	setcanvas $canvas
    }
    _update_attr
}

#
# Destroy this object and its associated widgets.
#
::itcl::body Canvasprintbox::destructor {} {
    stop
}

#
# Hold the information about common paper sizes. A bit of a hack, but it
# should be possible to add your own if you take a look at it.
#
::itcl::body Canvasprintbox::ezPaperInfo {size {attr ""} {orient "portrait"} {window ""}} {
    set size [string tolower $size]
    set attr [string tolower $attr]
    set orient [string tolower $orient]
    case $size in {
    types {
	return "A5 A4 A3 A2 A1 Legal Letter"
      }
    a5 {
	set paper(x1) "1.0c"
	set paper(y1) "1.0c"
	set paper(x2) "13.85c"
	set paper(y2) "20.0c"
	set paper(pheight) "19.0c"
	set paper(pwidth) "12.85c"
	set paper(height) "21.0c"
	set paper(width) "14.85c"
	set paper(centerx) "7.425c"
	set paper(centery) "10.5c"
      }
    a4 {
	set paper(x1) "1.0c"
	set paper(y1) "1.0c"
	set paper(x2) "20.0c"
	set paper(y2) "28.7c"
	set paper(pheight) "27.7c"
	set paper(pwidth) "19.0c"
	set paper(height) "29.7c"
	set paper(width) "21.0c"
	set paper(centerx) "10.5c"
	set paper(centery) "14.85c"
      }
    a3 {
	set paper(x1) "1.0c"
	set paper(y1) "1.0c"
	set paper(x2) "28.7c"
	set paper(y2) "41.0c"
	set paper(pheight) "40.0c"
	set paper(pwidth) "27.7c"
	set paper(height) "42.0c"
	set paper(width) "29.7c"
	set paper(centerx) "14.85c"
	set paper(centery)  "21.0c"
      }
    a2 {
	set paper(x1) "1.0c"
	set paper(y1) "1.0c"
	set paper(x2) "41.0c"
	set paper(y2) "58.4c"
	set paper(pheight) "57.4c"
	set paper(pwidth) "40.0c"
	set paper(height) "59.4c"
	set paper(width) "42.0c"
	set paper(centerx) "21.0c"
	set paper(centery)  "29.7c"
      }
    a1 {
	set paper(x1) "1.0c"
	set paper(y1) "1.0c"
	set paper(x2) "58.4c"
	set paper(y2) "83.0c"
	set paper(pheight) "82.0c"
	set paper(pwidth) "57.4c"
	set paper(height) "84.0c"
	set paper(width) "59.4c"
	set paper(centerx) "29.7c"
	set paper(centery)  "42.0c"
      }
    legal {
	set paper(x1) "0.2i"
	set paper(y1) "0.2i"
	set paper(x2) "8.3i"
	set paper(y2) "13.8i"
	set paper(pheight) "13.6i"
	set paper(pwidth) "8.1i"
	set paper(height) "14.0i"
	set paper(width) "8.5i"
	set paper(centerx) "4.25i"
	set paper(centery) "7.0i"
      }
    letter {
	set paper(x1) "0.2i"
	set paper(y1) "0.2i"
	set paper(x2) "8.3i"
	set paper(y2) "10.8i"
	set paper(pheight) "10.6i"
	set paper(pwidth) "8.1i"
	set paper(height) "11.0i"
	set paper(width) "8.5i"
	set paper(centerx) "4.25i"
	set paper(centery) "5.5i"
      }
    default {
	error "ezPaperInfo: Unknown paper type ($type)"
      }
    }
    set inv(x1) "y1"
    set inv(x2) "y2"
    set inv(y1) "x1"
    set inv(y2) "x2"
    set inv(pwidth) "pheight"
    set inv(pheight) "pwidth"
    set inv(width) "height"
    set inv(height) "width"
    set inv(centerx) "centery"
    set inv(centery) "centerx"
    case $orient in {
    landscape {
	set res $paper($inv($attr))
      }
    portrait {
	set res $paper($attr)
      }
    default {
	error "ezPaperInfo: orientation should be\
		portrait or landscape (not $orient)"
      }
    }
    if {$window ne ""} {
	set res [winfo fpixels $window $res]
    }
    return $res
} 

} ; # end ::itcl::widgets
