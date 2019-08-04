/***
 * mast: MainWindow.cpp
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#include "MainWindow.h"

#include <wx/button.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include <common/dicom/volumes/DICOMVolumeLoader.h>
#include <common/io/files/DICOMDIRFile.h>
#include <common/io/files/VolumeChoiceFile.h>
#include <mast/gui/dialogs/DialogUtil.h>
#include <mast/gui/dialogs/VolumeChooserDialog.h>
#include <mast/gui/windows/PartitionWindow.h>
#include <mast/util/StringConversion.h>

namespace {

//#################### LOCAL CONSTANTS ####################
enum
{
	BUTTONID_BASE = wxID_HIGHEST,	// a dummy value which is never used: subsequent values are guaranteed to be higher than this
	BUTTONID_OPENDICOMDIR,
	BUTTONID_OPENVOLUMECHOICE,
	BUTTONID_OPENTESTVOLUME1,
	BUTTONID_OPENTESTVOLUME2,
	BUTTONID_EXIT,
};

enum
{
	MENUID_BASE = wxID_HIGHEST + 1000,	// a dummy value which is never used: subsequent values are guaranteed to be higher than this
	MENUID_FILE_EXIT,
	MENUID_FILE_OPENDICOMDIR,
	MENUID_FILE_OPENVOLUMECHOICE,
	MENUID_HELP_ABOUT,
};

}

namespace mp {

//#################### CONSTRUCTORS ####################
MainWindow::MainWindow(const std::string& title)
:	wxFrame(NULL, wxID_ANY, string_to_wxString(title))
{
	setup_menus();
	setup_gui();
}

//#################### PRIVATE METHODS ####################
void MainWindow::load_saved_volume_choice(const std::string& volumeChoiceFilename)
try
{
	check_file_exists(volumeChoiceFilename);
	DICOMVolumeChoice volumeChoice = VolumeChoiceFile::load(volumeChoiceFilename);
	load_volume(volumeChoice);
}
catch(std::exception& e)
{
	wxMessageBox(string_to_wxString(e.what()), wxT("Error"), wxOK|wxICON_ERROR|wxCENTRE, this);
}

void MainWindow::load_volume(const DICOMVolumeChoice& volumeChoice)
{
	DICOMDirectory_CPtr dicomdir = DICOMDIRFile::load(volumeChoice.dicomdirFilename);
	load_volume(dicomdir, volumeChoice);
}

void MainWindow::load_volume(const DICOMDirectory_CPtr& dicomdir, const DICOMVolumeChoice& volumeChoice)
{
	DICOMVolumeLoader_Ptr loader(new DICOMVolumeLoader(dicomdir, volumeChoice));
	if(execute_with_progress_dialog(loader, this, "Loading DICOM Volume"))
	{
		// Create a window for the user to interact with the new volume.
		std::string caption = "MAST - " + loader->volume_choice().description() + " - Untitled";
		PartitionWindow *partitionWindow = new PartitionWindow(this, caption, loader->volume(), loader->volume_choice());
		partitionWindow->Show(true);
	}
}

void MainWindow::setup_gui()
{
	wxFlexGridSizer *sizer = new wxFlexGridSizer(3, 3, 0, 0);
	SetSizer(sizer);

	// Top left, top, top right and middle left
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);
	for(int i=0; i<4; ++i) sizer->AddSpacer(50);

	// Middle
	wxPanel *panel = new wxPanel(this);
	panel->SetBackgroundColour(GetBackgroundColour());
	sizer->Add(panel);

	wxGridSizer *panelSizer = new wxGridSizer(0, 1, 10, 0);
	panel->SetSizer(panelSizer);

	wxSize buttonSize(300, 50);

	std::vector<wxString> buttonCaptions;
	buttonCaptions.push_back(wxT("Open DICOMDIR..."));
	buttonCaptions.push_back(wxT("Open Volume Choice..."));
	buttonCaptions.push_back(wxT("Open Test Volume 1"));
	buttonCaptions.push_back(wxT("Open Test Volume 2"));
	buttonCaptions.push_back(wxT("Exit"));

	for(size_t i=0, size=buttonCaptions.size(); i<size; ++i)
	{
		wxButton *button = new wxButton(panel, BUTTONID_BASE + 1 + i, buttonCaptions[i], wxDefaultPosition, buttonSize);
		wxFont font = button->GetFont();
		font.SetPointSize(14);
		button->SetFont(font);
		panelSizer->Add(button, 0, wxALIGN_CENTER_HORIZONTAL);
	}

	// Middle right, bottom left, bottom and bottom right
	sizer->AddGrowableCol(2);
	sizer->AddGrowableRow(2);
	for(int i=0; i<4; ++i) sizer->AddSpacer(50);

	sizer->Fit(this);
}

void MainWindow::setup_menus()
{
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(MENUID_FILE_OPENDICOMDIR, wxT("Open &DICOMDIR...\tCtrl+O"));
	fileMenu->Append(wxID_ANY, wxT("Open &Volume...\tCtrl+Shift+O"));
	fileMenu->Append(MENUID_FILE_OPENVOLUMECHOICE, wxT("Open Volume &Choice...\tCtrl+Alt+O"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MENUID_FILE_EXIT, wxT("&Exit\tAlt+F4"));

	wxMenu *toolsMenu = new wxMenu;
	toolsMenu->Append(wxID_ANY, wxT("&Anonymize Directory Tree...\tCtrl+A"));
	toolsMenu->Append(wxID_ANY, wxT("&Replace Volume Choice Section...\tCtrl+R"));

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(MENUID_HELP_ABOUT, wxT("&About...\tF2"));

	m_menuBar = new wxMenuBar;
	m_menuBar->Append(fileMenu, wxT("&File"));
	m_menuBar->Append(toolsMenu, wxT("&Tools"));
	m_menuBar->Append(helpMenu, wxT("&Help"));

	SetMenuBar(m_menuBar);
}

//#################### EVENT HANDLERS ####################

//~~~~~~~~~~~~~~~~~~~~ BUTTONS ~~~~~~~~~~~~~~~~~~~~
void MainWindow::OnButtonOpenTestVolume1(wxCommandEvent&)
{
	load_saved_volume_choice("../resources/testvolume1.vcf");
}

void MainWindow::OnButtonOpenTestVolume2(wxCommandEvent&)
{
	load_saved_volume_choice("../resources/testvolume2.vcf");
}

//~~~~~~~~~~~~~~~~~~~~ COMMON ~~~~~~~~~~~~~~~~~~~~
void MainWindow::OnCommonExit(wxCommandEvent&)
{
	Close();
}

void MainWindow::OnCommonOpenDICOMDIR(wxCommandEvent&)
{
	wxFileDialog_Ptr dialog = construct_open_dialog(this, "Open DICOMDIR", "DICOMDIR Files|DICOMDIR|All Files|*.*");
	if(dialog->ShowModal() == wxID_OK)
	{
		std::string path = wxString_to_string(dialog->GetPath());
		try
		{
			check_file_exists(path);

			// Display a volume chooser dialog to allow the user to choose which volume to load.
			VolumeChooserDialog dialog(path);
			dialog.ShowModal();

			if(dialog.volume_choice())
			{
				load_volume(dialog.dicomdir(), *dialog.volume_choice());
			}
		}
		catch(std::exception& e)
		{
			wxMessageBox(string_to_wxString(e.what()), wxT("Error"), wxOK|wxICON_ERROR|wxCENTRE, this);
		}
	}
}

void MainWindow::OnCommonOpenVolumeChoice(wxCommandEvent&)
{
	wxFileDialog_Ptr dialog = construct_open_dialog(this, "Open Volume Choice", "Volume Choice Files (*.vcf)|*.vcf|All Files|*.*");
	if(dialog->ShowModal() == wxID_OK)
	{
		std::string path = wxString_to_string(dialog->GetPath());
		load_saved_volume_choice(path);
	}
}

//~~~~~~~~~~~~~~~~~~~~ MENUS ~~~~~~~~~~~~~~~~~~~~
void MainWindow::OnMenuHelpAbout(wxCommandEvent&)
{
	std::ostringstream oss;
	oss << "MAST (the Millipede Automatic Segmentation Tool) is a program I am developing as part of "
		<< "my computing doctorate at Oxford University, under the watchful guidance and supervision "
		<< "of Dr Stephen Cameron and Dr Irina Voiculescu. I am extremely grateful to both of them for "
		<< "their help and support throughout.\n"
		<< '\n'
		<< "MAST is the successor application to FAST, the automatic segmentation tool which I wrote "
		<< "to accompany my centipede image processing library.\n"
		<< '\n'
		<< "- Stuart Golodetz, November 2009";
	wxMessageBox(string_to_wxString(oss.str()), wxT("About MAST"), wxOK|wxCENTRE, this);
}

//#################### EVENT TABLE ####################
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	//~~~~~~~~~~~~~~~~~~~~ BUTTONS ~~~~~~~~~~~~~~~~~~~~
	EVT_BUTTON(BUTTONID_OPENDICOMDIR, MainWindow::OnCommonOpenDICOMDIR)
	EVT_BUTTON(BUTTONID_OPENTESTVOLUME1, MainWindow::OnButtonOpenTestVolume1)
	EVT_BUTTON(BUTTONID_OPENTESTVOLUME2, MainWindow::OnButtonOpenTestVolume2)
	EVT_BUTTON(BUTTONID_OPENVOLUMECHOICE, MainWindow::OnCommonOpenVolumeChoice)
	EVT_BUTTON(BUTTONID_EXIT, MainWindow::OnCommonExit)

	//~~~~~~~~~~~~~~~~~~~~ MENUS ~~~~~~~~~~~~~~~~~~~~
	EVT_MENU(MENUID_FILE_EXIT, MainWindow::OnCommonExit)
	EVT_MENU(MENUID_FILE_OPENDICOMDIR, MainWindow::OnCommonOpenDICOMDIR)
	EVT_MENU(MENUID_FILE_OPENVOLUMECHOICE, MainWindow::OnCommonOpenVolumeChoice)
	EVT_MENU(MENUID_HELP_ABOUT, MainWindow::OnMenuHelpAbout)
END_EVENT_TABLE()

}
