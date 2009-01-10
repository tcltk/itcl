#
# CanvasPrintDialog v1.5
# ----------------------------------------------------------------------
# Implements a print dialog for printing the contents of a canvas widget
# to a printer or a file. It is possible to specify page orientation, the
# number of pages to print the image on and if the output should be
# stretched to fit the page. The CanvasPrintDialog is derived from the
# Dialog class and is composed of a CanvasPrintBox with attributes set to
# manipulate the dialog buttons.
# 
# Author: Arnulf P. Wiedemann
# Copyright (c) 2008 for the reimplemented version
#
# see file license.terms in the top directory
#
# ----------------------------------------------------------------------
# This code is derived/reimplemented from the iwidgets package CanvasPrintDialog
# written by:
# ----------------------------------------------------------------------
#  AUTHOR: Tako Schotanus              EMAIL: Tako.Schotanus@bouw.tno.nl
# ----------------------------------------------------------------------
#				   Copyright (c) 1995  Tako Schotanus
# ----------------------------------------------------------------------
#
#   @(#) $Id: canvasprintdialog.tcl,v 1.1.2.1 2009/01/10 18:46:00 wiede Exp $
# ======================================================================

#
# Option database default resources:
#
option add *Canvasprintdialog.filename "canvas.ps" widgetDefault
option add *Canvasprintdialog.hPageCnt 1 widgetDefault
option add *Canvasprintdialog.orient landscape widgetDefault
option add *Canvasprintdialog.output printer widgetDefault
option add *Canvasprintdialog.pageSize A4 widgetDefault
option add *Canvasprintdialog.posterize 0 widgetDefault
option add *Canvasprintdialog.printCmd lpr widgetDefault
option add *Canvasprintdialog.printRegion "" widgetDefault
option add *Canvasprintdialog.vPageCnt 1 widgetDefault
option add *Canvasprintdialog.title "Canvas Print Dialog" widgetDefault
option add *Canvasprintdialog.master "." widgetDefault

namespace eval ::itcl::widgets {

#
# Provide a lowercased access method for the Canvasprintdialog class.
# 
proc ::itcl::widgets::canvasprintdialog {args} {
	uplevel ::itcl::widgets::Canvasprintdialog $args
}

# ------------------------------------------------------------------
# CANVASPRINTDIALOG
# ------------------------------------------------------------------
::itcl::extendedclass Canvasprintdialog {
    inherit ::itcl::widgets::Dialog

    component cpb

    constructor {args} {}   
    destructor {}

    public method deactivate {args} {}
    public method getoutput {} {}
    public method setcanvas {canv} {}
    public method refresh {} {}
    public method print {} {}
}

# ------------------------------------------------------------------
# CONSTRUCTOR 
#
# Create new file selection dialog.
# ------------------------------------------------------------------
::itcl::body Canvasprintdialog::constructor {args} {
# FIXME $win configure -borderwidth 0
    # 
    # Instantiate a file selection box widget.
    #
    setupcomponent cpb using ::itcl::widgets::Canvasprintbox $itcl_interior.cpb
    keepcomponentoption cpb -background -cursor -foreground -modality 
    keepcomponentoption cpb -printregion -output -printcmd -filename -pagesize \
	         -orient -stretch -posterize -hpagecnt -vpagecnt
    pack $cpb -fill both -expand yes
    #
    # Hide the apply and help buttons.
    #
    buttonconfigure OK -text Print
    buttonconfigure Apply -command [itcl::code $this refresh] -text Refresh
    hide Help
    uplevel 0 itcl_initoptions $args
}   

# ------------------------------------------------------------------
# METHOD: deactivate
#
# Redefines method of dialog shell class. Stops the drawing of the
# thumbnail (when busy) upon deactivation of the dialog.
# ------------------------------------------------------------------
::itcl::body Canvasprintdialog::deactivate {args} {
    $cpb stop
    return [eval Shell::deactivate $args]
}

# ------------------------------------------------------------------
# METHOD: getoutput
#
# Thinwrapped method of canvas print box class.
# ------------------------------------------------------------------
::itcl::body ::itcl::widgets::Canvasprintdialog::getoutput {} {
    return [$cpb getoutput]
}

# ------------------------------------------------------------------
# METHOD: setcanvas
#
# Thinwrapped method of canvas print box class.
# ------------------------------------------------------------------
::itcl::body Canvasprintdialog::setcanvas {canv} {
    return [$cpb setcanvas $canv]
}

# ------------------------------------------------------------------
# METHOD: refresh
#
# Thinwrapped method of canvas print box class.
# ------------------------------------------------------------------
::itcl::body Canvasprintdialog::refresh {} {
    return [$cpb refresh]
}

# ------------------------------------------------------------------
# METHOD: print
#
# Thinwrapped method of canvas print box class.
# ------------------------------------------------------------------
::itcl::body Canvasprintdialog::print {} {
    return [$cpb print]
}

} ; # end ::itcl::widgets
