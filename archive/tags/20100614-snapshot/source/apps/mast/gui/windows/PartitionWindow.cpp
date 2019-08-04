/***
 * millipede: PartitionWindow.cpp
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#include "PartitionWindow.h"

#include <wx/menu.h>
#include <wx/sizer.h>

#include <mast/gui/components/partitionview/PartitionView.h>
#include <mast/util/StringConversion.h>

namespace mp {

//#################### CONSTRUCTORS ####################
PartitionWindow::PartitionWindow(wxWindow *parent, const std::string& title, const DICOMVolume_Ptr& volume, const DICOMVolumeChoice& volumeChoice,
								 wxGLContext *context)
:	wxFrame(parent, -1, string_to_wxString(title))
{
	setup_menus();
	setup_gui(volume, volumeChoice, context);
}

//#################### PUBLIC METHODS ####################
wxGLContext *PartitionWindow::get_context() const
{
	return m_view->get_context();
}

//#################### PRIVATE METHODS ####################
void PartitionWindow::setup_gui(const DICOMVolume_Ptr& volume, const DICOMVolumeChoice& volumeChoice, wxGLContext *context)
{
	SetBackgroundColour(wxColour(240,240,240));

	wxGridSizer *sizer = new wxGridSizer(1, 1, 0, 0);
	SetSizer(sizer);

	Show();

	m_view = new PartitionView(this, volume, volumeChoice, context);
	sizer->Add(m_view);

	sizer->Fit(this);
}

void PartitionWindow::setup_menus()
{
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_ANY, wxT("&Save\tCtrl+S"));
	fileMenu->Append(wxID_ANY, wxT("Save &As..."));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_ANY, wxT("E&xit\tAlt+F4"));

	wxMenu *actionsMenu = new wxMenu;
	actionsMenu->Append(wxID_ANY, wxT("&Undo\tCtrl+Z"));
	actionsMenu->Append(wxID_ANY, wxT("&Redo\tCtrl+Y"));
	actionsMenu->AppendSeparator();
	actionsMenu->Append(wxID_ANY, wxT("&Clear History"));

	wxMenu *navigationMenu = new wxMenu;
	navigationMenu->Append(wxID_ANY, wxT("&Next Slice"));
	navigationMenu->Append(wxID_ANY, wxT("&Previous Slice"));
	navigationMenu->AppendSeparator();
	navigationMenu->Append(wxID_ANY, wxT("N&ext Layer"));
	navigationMenu->Append(wxID_ANY, wxT("P&revious Layer"));

	wxMenu *selectionMenu = new wxMenu;
	selectionMenu->Append(wxID_ANY, wxT("&Select Nodes By ID..."));
	selectionMenu->AppendSeparator();
	selectionMenu->Append(wxID_ANY, wxT("&Clear Selection"));

	wxMenu *segmentationMenu = new wxMenu;
	segmentationMenu->Append(wxID_ANY, wxT("Segment CT &Volume..."));
	segmentationMenu->AppendSeparator();
	segmentationMenu->Append(wxID_ANY, wxT("&Clone Current Layer"));
	segmentationMenu->Append(wxID_ANY, wxT("&Delete Current Layer"));
	segmentationMenu->Append(wxID_ANY, wxT("&Merge Selected Nodes"));
	wxMenu *splitNodeMenu = new wxMenu;
	segmentationMenu->AppendSubMenu(splitNodeMenu, wxT("&Split Node"));
		splitNodeMenu->Append(wxID_ANY, wxT("Set &Node"));
		splitNodeMenu->Append(wxID_ANY, wxT("&Add Subgroup"));
		splitNodeMenu->Append(wxID_ANY, wxT("&Manage Subgroups..."));
		splitNodeMenu->Append(wxID_ANY, wxT("&Finalize Split"));
		splitNodeMenu->AppendSeparator();
		splitNodeMenu->Append(wxID_ANY, wxT("&Start Again"));
	segmentationMenu->AppendSeparator();
	segmentationMenu->Append(wxID_ANY, wxT("&Unzip Selected Node..."));
	segmentationMenu->AppendSeparator();
	wxMenu *switchParentMenu = new wxMenu;
	segmentationMenu->AppendSubMenu(switchParentMenu, wxT("Switch &Parent"));
		switchParentMenu->Append(wxID_ANY, wxT("Set &Child"));
		switchParentMenu->Append(wxID_ANY, wxT("Set New &Parent"));
		switchParentMenu->AppendSeparator();
		switchParentMenu->Append(wxID_ANY, wxT("&Start Again"));

	wxMenu *featureMenu = new wxMenu;
	wxMenu *autoMarkMenu = new wxMenu;
	featureMenu->AppendSubMenu(autoMarkMenu, wxT("&Automatically Mark"));
		autoMarkMenu->Append(wxID_ANY, wxT("Using &Default Identifier"));
		autoMarkMenu->Append(wxID_ANY, wxT("Using &Script..."));
	wxMenu *manuMarkMenu = new wxMenu;
	featureMenu->AppendSubMenu(manuMarkMenu, wxT("&Manually Mark"));
		manuMarkMenu->Append(wxID_ANY, wxT("&Kidney"));
		// TODO: Other features (possibly via iterating over the feature ID enumeration)
	featureMenu->AppendSeparator();
	wxMenu *selectMarkedMenu = new wxMenu;
	featureMenu->AppendSubMenu(selectMarkedMenu, wxT("&Select Marked"));
		selectMarkedMenu->Append(wxID_ANY, wxT("&Kidney"));
		// TODO: Other features (possibly via iterating over the feature ID enumeration)

	wxMenu *toolsMenu = new wxMenu;
	toolsMenu->Append(wxID_ANY, wxT("&Visualize in 3D..."));

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(wxID_ANY, wxT("&Contents..."));

	m_menuBar = new wxMenuBar;
	m_menuBar->Append(fileMenu, wxT("&File"));
	m_menuBar->Append(actionsMenu, wxT("&Actions"));
	m_menuBar->Append(navigationMenu, wxT("&Navigation"));
	m_menuBar->Append(selectionMenu, wxT("S&election"));
	m_menuBar->Append(segmentationMenu, wxT("&Segmentation"));
	m_menuBar->Append(featureMenu, wxT("Feature &Identification"));
	m_menuBar->Append(toolsMenu, wxT("&Tools"));
	m_menuBar->Append(helpMenu, wxT("&Help"));

	SetMenuBar(m_menuBar);
}

}
