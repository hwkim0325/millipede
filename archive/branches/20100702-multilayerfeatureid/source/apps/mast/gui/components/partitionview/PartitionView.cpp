/***
 * millipede: PartitionView.cpp
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#include "PartitionView.h"

#include <wx/button.h>
#include <wx/numdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <common/dicom/volumes/DICOMVolume.h>
#include <common/featureid/multilayer/MultilayerNodeScorer_Kidney.h>
#include <common/partitionforests/images/MosaicImageCreator.h>
#include <common/segmentation/DICOMLowestLayersBuilder.h>
#include <common/segmentation/VolumeIPFBuilder.h>
#include <common/slices/SliceTextureSetFiller.h>
#include <mast/gui/dialogs/DialogUtil.h>
#include <mast/gui/dialogs/SegmentDICOMVolumeDialog.h>
#include <mast/gui/overlays/IPFMultiFeatureSelectionOverlay.h>
#include <mast/gui/overlays/IPFSelectionOverlay.h>
#include <mast/gui/overlays/PartitionOverlayManager.h>
#include <mast/util/StringConversion.h>
#include "DICOMCanvas.h"
#include "PartitionCanvas.h"
using namespace mp;

namespace {

//#################### LOCAL CONSTANTS ####################
enum
{
	ID_BASE = wxID_HIGHEST + 1000,	// a dummy value which is never used: subsequent values are guaranteed to be higher than this
	BUTTONID_SEGMENT_VOLUME,
	BUTTONID_VIEW_XY,
	BUTTONID_VIEW_XZ,
	BUTTONID_VIEW_YZ,
	BUTTONID_VISUALIZE_IN_3D,
	SLIDERID_X,
	SLIDERID_Y,
	SLIDERID_Z,
	SLIDERID_LAYER,
	SLIDERID_ZOOM,
};

}

namespace mp {

//#################### LISTENERS ####################
struct PartitionView::CameraListener : PartitionCamera::Listener
{
	PartitionView *base;

	explicit CameraListener(PartitionView *base_)
	:	base(base_)
	{}

	void slice_location_changed(bool sliceChanged, bool layerChanged)
	{
		base->update_sliders();
		if(sliceChanged || layerChanged) base->recreate_overlays();
		base->refresh_canvases();
	}

	void slice_orientation_changed()
	{
		base->recreate_overlays();
		base->refresh_canvases();
	}

	void zoom_level_changed()
	{
		base->update_sliders();
		base->refresh_canvases();
	}
};

struct PartitionView::MultiFeatureSelectionListener : VolumeIPFMultiFeatureSelection<DICOMImageLeafLayer,DICOMImageBranchLayer,AbdominalFeature::Enum>::Listener
{
	PartitionView *base;

	explicit MultiFeatureSelectionListener(PartitionView *base_)
	:	base(base_)
	{}

	void multi_feature_selection_changed(int commandDepth)
	{
		if(commandDepth == 0)
		{
			base->recreate_multi_feature_selection_overlay();
			base->refresh_canvases();
		}
	}
};

struct PartitionView::SelectionListener : VolumeIPFSelection<DICOMImageLeafLayer,DICOMImageBranchLayer>::Listener
{
	PartitionView *base;

	explicit SelectionListener(PartitionView *base_)
	:	base(base_)
	{}

	void selection_changed(int commandDepth)
	{
		if(commandDepth == 0)
		{
			base->recreate_selection_overlay();
			base->refresh_canvases();
		}
	}
};

//#################### CONSTRUCTORS ####################
PartitionView::PartitionView(wxWindow *parent, const DICOMVolume_Ptr& volume, const DICOMVolumeChoice& volumeChoice,
							 const ICommandManager_Ptr& commandManager, wxGLContext *context)
:	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(100,100)),
	m_camera(new PartitionCamera(
		SliceLocation((volumeChoice.maxX - volumeChoice.minX)/2, (volumeChoice.maxY - volumeChoice.minY)/2, (volumeChoice.maxZ - volumeChoice.minZ)/2, 0),
		ORIENT_XY,
		volume->size()
	)),
	m_commandManager(commandManager),
	m_model(new PartitionModelT(volume)),
	m_overlayManager(new PartitionOverlayManager),
	m_volumeChoice(volumeChoice)
{
	m_camera->add_shared_listener(boost::shared_ptr<PartitionCamera::Listener>(new CameraListener(this)));
	m_model->set_command_manager(commandManager);

	calculate_canvas_size();
	setup_gui(context);

	m_dicomCanvas->setup(this);
	m_partitionCanvas->setup(this);

	fit_image_to_view();
	create_dicom_textures();
	create_overlays();
}

//#################### PUBLIC METHODS ####################
const PartitionCamera_Ptr& PartitionView::camera()
{
	return m_camera;
}

PartitionCamera_CPtr PartitionView::camera() const
{
	return m_camera;
}

void PartitionView::fit_image_to_view()
{
	m_dicomCanvas->fit_image_to_canvas();
}

wxGLContext *PartitionView::get_context() const
{
	return m_dicomCanvas->GetContext();
}

void PartitionView::goto_slice()
{
	wxSlider *slider = NULL;
	switch(m_camera->slice_orientation())
	{
		case ORIENT_XY:	slider = m_zSlider; break;
		case ORIENT_XZ:	slider = m_ySlider; break;
		case ORIENT_YZ:	slider = m_xSlider; break;
	}
	assert(slider != NULL);

	SliceLocation loc = m_camera->slice_location();
	long minValue = slider->GetMin(), maxValue = slider->GetMax();
	long curValue = minValue + loc[m_camera->slice_orientation()];
	long newValue = wxGetNumberFromUser(wxT(""), wxT("Slice Number:"), wxT("Goto Slice"), curValue, minValue, maxValue, this);
	if(newValue != -1)
	{
		loc[m_camera->slice_orientation()] = newValue - minValue;
		m_camera->set_slice_location(loc);
	}
}

void PartitionView::iterate_multilayer_node_scorer()
{
	// FIXME: The menu item should be disabled if it's not possible to iterate.
	if(m_multilayerNodeScorerOverlay)
	{
		m_multilayerNodeScorerOverlay->iterate();
		m_multilayerNodeScorerOverlay->recreate_texture(m_camera->slice_location(), m_camera->slice_orientation());
		refresh_canvases();
	}
}

const PartitionView::PartitionModel_Ptr& PartitionView::model()
{
	return m_model;
}

PartitionView::PartitionModel_CPtr PartitionView::model() const
{
	return m_model;
}

void PartitionView::reset_multilayer_node_scorer()
{
	// FIXME: The menu item should be disabled if it's not possible to reset.
	if(m_multilayerNodeScorerOverlay)
	{
		m_multilayerNodeScorerOverlay->reset();
		m_multilayerNodeScorerOverlay->recreate_texture(m_camera->slice_location(), m_camera->slice_orientation());
		refresh_canvases();
	}
}

void PartitionView::segment_volume()
{
	typedef boost::shared_ptr<VolumeIPF<DICOMImageLeafLayer,DICOMImageBranchLayer> > VolumeIPF_Ptr;
	VolumeIPF_Ptr volumeIPF;

	Job_Ptr job;

	// Display a segment volume dialog to allow the user to choose how the segmentation process should work.
	SegmentDICOMVolumeDialog dialog(this, m_model->dicom_volume()->size(), m_volumeChoice.windowSettings);
	dialog.ShowModal();
	if(dialog.segmentation_options())
	{
		typedef VolumeIPFBuilder<DICOMLowestLayersBuilder> DICOMVolumeIPFBuilder;
		job.reset(new DICOMVolumeIPFBuilder(m_model->dicom_volume(), *dialog.segmentation_options(), volumeIPF));
	}

	// If the user cancelled the segment volume dialog, exit.
	if(!job) return;

	// Actually segment the volume. If the segmentation finishes successfully, set up the textures, overlays, listeners, etc.
	if(execute_with_progress_dialog(job, this, "Segmenting Volume"))
	{
		m_model->set_volume_ipf(volumeIPF);
		create_partition_textures();
		recreate_overlays();
		refresh_canvases();

		m_model->multi_feature_selection()->add_shared_listener(boost::shared_ptr<MultiFeatureSelectionListener>(new MultiFeatureSelectionListener(this)));
		m_model->selection()->add_shared_listener(boost::shared_ptr<SelectionListener>(new SelectionListener(this)));
	}
}

void PartitionView::zoom_to_fit()
{
	m_dicomCanvas->zoom_to_fit();
}

//#################### PRIVATE METHODS ####################
void PartitionView::calculate_canvas_size()
{
	// We want our canvases to be at least 512x512, but beyond that their size should be dictated by the sizes
	// of the images. We want to be able to show the images in axial (X-Y), sagittal (Y-Z) and coronal (X-Z)
	// orientations, which dictates which dimensions we need to take into account for the canvas sizes.

	itk::Size<3> volumeSize = m_model->dicom_volume()->size();
	m_canvasWidth = std::max<int>(512, std::max(volumeSize[0], volumeSize[1]));
	m_canvasHeight = std::max<int>(512, std::max(volumeSize[1], volumeSize[2]));
}

void PartitionView::create_dicom_textures()
{
	m_dicomTextureSet.reset(new SliceTextureSet);

	DICOMVolume::WindowedImagePointer windowedImage = m_model->dicom_volume()->windowed_image(m_volumeChoice.windowSettings);
	Job_Ptr job = fill_dicom_textures_job(m_camera->slice_orientation(), windowedImage);
	execute_with_progress_dialog(job, this, "Creating DICOM Texture Set", false);
}

void PartitionView::create_overlays()
{
	m_overlayManager->clear_overlays();
	m_multilayerNodeScorerOverlay = multilayer_node_scorer_overlay();
	m_overlayManager->insert_overlay_at_top("MultilayerNodeScorer", m_multilayerNodeScorerOverlay);
	m_overlayManager->insert_overlay_at_top("IPFMultiFeatureSelection", multi_feature_selection_overlay());
	m_overlayManager->insert_overlay_at_top("IPFSelection", selection_overlay());
}

void PartitionView::create_partition_textures()
{
	typedef VolumeIPF<DICOMImageLeafLayer,DICOMImageBranchLayer> CTVolumeIPF;
	typedef boost::shared_ptr<const CTVolumeIPF> CTVolumeIPF_CPtr;

	CTVolumeIPF_CPtr volumeIPF = m_model->volume_ipf();
	if(!volumeIPF) return;
	int highestLayer = volumeIPF->highest_layer();

	m_partitionTextureSets = std::vector<SliceTextureSet_Ptr>(highestLayer);
	for(int layer=1; layer<=highestLayer; ++layer) m_partitionTextureSets[layer-1].reset(new SliceTextureSet);

	Job_Ptr job = fill_partition_textures_job(m_camera->slice_orientation());
	execute_with_progress_dialog(job, this, "Creating Partition Texture Sets", false);

	m_layerSlider->SetRange(1, highestLayer);
	m_camera->set_highest_layer(highestLayer);
	SliceLocation loc = m_camera->slice_location();
	m_camera->set_slice_location(SliceLocation(loc.x, loc.y, loc.z, (1+highestLayer)/2));
}

SliceTextureSet_CPtr PartitionView::dicom_texture_set() const
{
	return m_dicomTextureSet;
}

Job_Ptr PartitionView::fill_dicom_textures_job(SliceOrientation ori, const itk::Image<unsigned char,3>::Pointer& windowedImage) const
{
	SliceTextureSetFiller<unsigned char> *job = new SliceTextureSetFiller<unsigned char>(ori, m_model->dicom_volume()->size(), m_dicomTextureSet);
	job->set_volume_image(windowedImage);
	return Job_Ptr(job);
}

Job_Ptr PartitionView::fill_partition_textures_job(SliceOrientation ori) const
{
	typedef VolumeIPF<DICOMImageLeafLayer,DICOMImageBranchLayer> VolumeIPFT;
	typedef boost::shared_ptr<const VolumeIPFT> VolumeIPF_CPtr;

	VolumeIPF_CPtr volumeIPF = m_model->volume_ipf();
	int highestLayer = volumeIPF->highest_layer();

	CompositeJob_Ptr job(new CompositeJob);
	for(int layer=1; layer<=highestLayer; ++layer)
	{
		typedef MosaicImageCreator<DICOMImageLeafLayer,DICOMImageBranchLayer> MIC;
		typedef SliceTextureSetFiller<unsigned char> TSF;

		MIC *mosaicImageCreator = new MIC(volumeIPF, layer, ori, true);
		TSF *textureSetFiller = new TSF(ori, volumeIPF->volume_size(), m_partitionTextureSets[layer-1]);
		textureSetFiller->set_volume_image_hook(mosaicImageCreator->get_mosaic_image_hook());

		job->add_subjob(mosaicImageCreator);
		job->add_subjob(textureSetFiller);
	}
	return job;
}

void PartitionView::fill_textures(SliceOrientation ori)
{
	CompositeJob_Ptr job(new CompositeJob);

	if(!m_dicomTextureSet->has_textures(ori))
	{
		DICOMVolume::WindowedImagePointer windowedImage = m_model->dicom_volume()->windowed_image(m_volumeChoice.windowSettings);
		job->add_subjob(fill_dicom_textures_job(ori, windowedImage));
	}

	if(!m_partitionTextureSets.empty() && !m_partitionTextureSets[0]->has_textures(ori))
	{
		job->add_subjob(fill_partition_textures_job(ori));
	}

	if(!job->empty()) execute_with_progress_dialog(job, this, "Creating Textures", false);
}

PartitionOverlay *PartitionView::multi_feature_selection_overlay() const
{
	PartitionModelT::VolumeIPFMultiFeatureSelection_CPtr multiFeatureSelection = m_model->multi_feature_selection();
	if(multiFeatureSelection)
	{
		SliceLocation loc = m_camera->slice_location();
		SliceOrientation ori = m_camera->slice_orientation();
		Map<AbdominalFeature::Enum,RGBA32> colourMap;
		colourMap.set(AbdominalFeature::KIDNEY, ITKImageUtil::make_rgba32(255,255,0,100));
		colourMap.set(AbdominalFeature::LIVER, ITKImageUtil::make_rgba32(128,0,128,100));
		return new IPFMultiFeatureSelectionOverlay(multiFeatureSelection, loc, ori, colourMap);
	}
	else return NULL;
}

PartitionView::MultilayerNodeScorerOverlay_Ptr PartitionView::multilayer_node_scorer_overlay() const
{
	typedef MultilayerNodeScorer<LeafLayer,BranchLayer> MultilayerNodeScorerT;
	typedef boost::shared_ptr<MultilayerNodeScorerT> MultilayerNodeScorer_Ptr;
	typedef MultilayerNodeScorer_Kidney<LeafLayer,BranchLayer> MultilayerNodeScorer_KidneyT;

	PartitionModelT::VolumeIPF_CPtr volumeIPF = m_model->volume_ipf();
	if(volumeIPF)
	{
		SliceLocation loc = m_camera->slice_location();
		SliceOrientation ori = m_camera->slice_orientation();
		MultilayerNodeScorer_Ptr scorer(new MultilayerNodeScorer_KidneyT(volumeIPF));
		return MultilayerNodeScorerOverlay_Ptr(new MultilayerNodeScorerOverlayT(scorer, volumeIPF, loc, ori));
	}
	else return MultilayerNodeScorerOverlay_Ptr();
}

PartitionOverlayManager_CPtr PartitionView::overlay_manager() const
{
	return m_overlayManager;
}

SliceTextureSet_CPtr PartitionView::partition_texture_set(int layer) const
{
	int n = layer - 1;
	if(0 <= n && n < static_cast<int>(m_partitionTextureSets.size())) return m_partitionTextureSets[n];
	else return SliceTextureSet_CPtr();
}

void PartitionView::recreate_multi_feature_selection_overlay()
{
	m_overlayManager->replace_overlay("IPFMultiFeatureSelection", multi_feature_selection_overlay());
}

void PartitionView::recreate_multilayer_node_scorer_overlay()
{
	m_multilayerNodeScorerOverlay = multilayer_node_scorer_overlay();
	m_overlayManager->replace_overlay("MultilayerNodeScorer", m_multilayerNodeScorerOverlay);
}

void PartitionView::recreate_overlays()
{
	recreate_multi_feature_selection_overlay();
	recreate_multilayer_node_scorer_overlay();
	recreate_selection_overlay();
}

void PartitionView::recreate_selection_overlay()
{
	m_overlayManager->replace_overlay("IPFSelection", selection_overlay());
}

void PartitionView::refresh_canvases()
{
	m_dicomCanvas->Refresh();
	m_partitionCanvas->Refresh();
}

PartitionOverlay *PartitionView::selection_overlay() const
{
	PartitionModelT::VolumeIPFSelection_CPtr selection = m_model->selection();
	if(selection)
	{
		SliceLocation loc = m_camera->slice_location();
		SliceOrientation ori = m_camera->slice_orientation();
		return new IPFSelectionOverlay(selection, loc, ori);
	}
	else return NULL;
}

void PartitionView::setup_gui(wxGLContext *context)
{
	SetBackgroundColour(wxColour(240,240,240));

	wxFlexGridSizer *sizer = new wxFlexGridSizer(3, 3, 5, 5);
	SetSizer(sizer);

	int attribList[] =
	{
		WX_GL_RGBA,
		WX_GL_DEPTH_SIZE,
		16,
		WX_GL_DOUBLEBUFFER,
		0
	};

	// Top left
	sizer->Add(new wxStaticText(this, wxID_ANY, ""));

	// Top middle
	sizer->Add(new wxStaticText(this, wxID_ANY, ""));

	// Top right
	m_segmentVolumeButton = new wxButton(this, BUTTONID_SEGMENT_VOLUME, wxT("Segment Volume..."));
	sizer->Add(m_segmentVolumeButton, 0, wxALIGN_CENTER_HORIZONTAL);

	// Middle left
	wxPanel *middleLeft = new wxPanel(this);
	wxBoxSizer *middleLeftSizer = new wxBoxSizer(wxVERTICAL);
	middleLeft->SetSizer(middleLeftSizer);
		m_dicomCanvas = new DICOMCanvas(middleLeft, context, attribList, wxID_ANY, wxDefaultPosition, wxSize(m_canvasWidth, m_canvasHeight));
		middleLeftSizer->Add(m_dicomCanvas);

		wxPanel *middleLeftBottom = new wxPanel(middleLeft);
		wxFlexGridSizer *middleLeftBottomSizer = new wxFlexGridSizer(0, 2, 0, 0);
		middleLeftBottom->SetSizer(middleLeftBottomSizer);
			wxStaticText *xText = new wxStaticText(middleLeftBottom, wxID_ANY, wxT("X: "));
			middleLeftBottomSizer->Add(xText, 0, wxALIGN_CENTER_VERTICAL);
			m_xSlider = new wxSlider(middleLeftBottom, SLIDERID_X, m_volumeChoice.minX + m_camera->slice_location().x, m_volumeChoice.minX, m_volumeChoice.maxX, wxDefaultPosition, wxSize(100,50), wxHORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS|wxSL_TOP);
			middleLeftBottomSizer->Add(m_xSlider, 0, wxALIGN_CENTER);

			wxStaticText *yText = new wxStaticText(middleLeftBottom, wxID_ANY, wxT("Y: "));
			middleLeftBottomSizer->Add(yText, 0, wxALIGN_CENTER_VERTICAL);
			m_ySlider = new wxSlider(middleLeftBottom, SLIDERID_Y, m_volumeChoice.minY + m_camera->slice_location().y, m_volumeChoice.minY, m_volumeChoice.maxY, wxDefaultPosition, wxSize(100,50), wxHORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS|wxSL_TOP);
			middleLeftBottomSizer->Add(m_ySlider, 0, wxALIGN_CENTER);

			wxStaticText *zText = new wxStaticText(middleLeftBottom, wxID_ANY, wxT("Z: "));
			middleLeftBottomSizer->Add(zText, 0, wxALIGN_CENTER_VERTICAL);
			m_zSlider = new wxSlider(middleLeftBottom, SLIDERID_Z, m_volumeChoice.minZ+1 + m_camera->slice_location().z, m_volumeChoice.minZ+1, m_volumeChoice.maxZ+1, wxDefaultPosition, wxSize(100,50), wxHORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS|wxSL_TOP);
			middleLeftBottomSizer->Add(m_zSlider, 0, wxALIGN_CENTER);

			wxStaticText *zoomText = new wxStaticText(middleLeftBottom, wxID_ANY, wxT("Zoom: "));
			middleLeftBottomSizer->Add(zoomText, 0, wxALIGN_CENTER_VERTICAL);
			m_zoomSlider = new wxSlider(middleLeftBottom, SLIDERID_ZOOM, m_camera->zoom_level(), m_camera->min_zoom_level(), m_camera->max_zoom_level(), wxDefaultPosition, wxSize(100,50), wxHORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS|wxSL_TOP);
			middleLeftBottomSizer->Add(m_zoomSlider, 0, wxALIGN_CENTER);
		middleLeftSizer->Add(middleLeftBottom, 0, wxALIGN_CENTER_HORIZONTAL);
	sizer->Add(middleLeft);

	// Middle
	wxPanel *middle = new wxPanel(this);
	wxBoxSizer *middleSizer = new wxBoxSizer(wxVERTICAL);
	middle->SetSizer(middleSizer);
		wxButton *viewXYButton = new wxButton(middle, BUTTONID_VIEW_XY, wxT("View X-Y (usually Axial)"));
		middleSizer->Add(viewXYButton, 0, wxALIGN_CENTER_HORIZONTAL);

		wxButton *viewXZButton = new wxButton(middle, BUTTONID_VIEW_XZ, wxT("View X-Z (usually Coronal)"));
		middleSizer->Add(viewXZButton, 0, wxALIGN_CENTER_HORIZONTAL);

		wxButton *viewYZButton = new wxButton(middle, BUTTONID_VIEW_YZ, wxT("View Y-Z (usually Sagittal)"));
		middleSizer->Add(viewYZButton, 0, wxALIGN_CENTER_HORIZONTAL);

		middleSizer->AddSpacer(10);

		wxButton *visualizeIn3DButton = new wxButton(middle, BUTTONID_VISUALIZE_IN_3D, wxT("Visualize in 3D..."));
		middleSizer->Add(visualizeIn3DButton, 0, wxALIGN_CENTER_HORIZONTAL);
	sizer->Add(middle);

	// Middle right
	wxPanel *middleRight = new wxPanel(this);
	wxBoxSizer *middleRightSizer = new wxBoxSizer(wxVERTICAL);
	middleRight->SetSizer(middleRightSizer);
		m_partitionCanvas = new PartitionCanvas(middleRight, get_context(), attribList, wxID_ANY, wxDefaultPosition, wxSize(m_canvasWidth, m_canvasHeight));
		middleRightSizer->Add(m_partitionCanvas);

		wxPanel *middleRightBottom = new wxPanel(middleRight);
		wxFlexGridSizer *middleRightBottomSizer = new wxFlexGridSizer(0, 2, 0, 0);
		middleRightBottom->SetSizer(middleRightBottomSizer);
			wxStaticText *layerText = new wxStaticText(middleRightBottom, wxID_ANY, wxT("Layer: "));
			middleRightBottomSizer->Add(layerText, 0, wxALIGN_CENTER_VERTICAL);
			m_layerSlider = new wxSlider(middleRightBottom, SLIDERID_LAYER, 0, 0, 1, wxDefaultPosition, wxSize(100,50), wxHORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS|wxSL_TOP);
			middleRightBottomSizer->Add(m_layerSlider, 0, wxALIGN_CENTER);
		middleRightSizer->Add(middleRightBottom, 0, wxALIGN_CENTER_HORIZONTAL);
	sizer->Add(middleRight);

	sizer->Fit(this);
}

void PartitionView::update_sliders()
{
	SliceLocation loc = m_camera->slice_location();
	m_xSlider->SetValue(m_xSlider->GetMin() + loc.x);
	m_ySlider->SetValue(m_ySlider->GetMin() + loc.y);
	m_zSlider->SetValue(m_zSlider->GetMin() + loc.z);
	m_layerSlider->SetValue(loc.layer);
	m_zoomSlider->SetValue(m_camera->zoom_level());
}

//#################### EVENT HANDLERS ####################

//~~~~~~~~~~~~~~~~~~~~ BUTTONS ~~~~~~~~~~~~~~~~~~~~
void PartitionView::OnButtonSegmentVolume(wxCommandEvent&)
{
	segment_volume();
}

void PartitionView::OnButtonViewXY(wxCommandEvent&)
{
	fill_textures(ORIENT_XY);
	m_camera->set_slice_orientation(ORIENT_XY);
	zoom_to_fit();
}

void PartitionView::OnButtonViewXZ(wxCommandEvent&)
{
	fill_textures(ORIENT_XZ);
	m_camera->set_slice_orientation(ORIENT_XZ);
	zoom_to_fit();
}

void PartitionView::OnButtonViewYZ(wxCommandEvent&)
{
	fill_textures(ORIENT_YZ);
	m_camera->set_slice_orientation(ORIENT_YZ);
	zoom_to_fit();
}

//~~~~~~~~~~~~~~~~~~~~ SLIDERS ~~~~~~~~~~~~~~~~~~~~
void PartitionView::OnSliderX(wxScrollEvent&)
{
	SliceLocation loc = m_camera->slice_location();
	m_camera->set_slice_location(SliceLocation(m_xSlider->GetValue() - m_xSlider->GetMin(), loc.y, loc.z, loc.layer));
}

void PartitionView::OnSliderY(wxScrollEvent&)
{
	SliceLocation loc = m_camera->slice_location();
	m_camera->set_slice_location(SliceLocation(loc.x, m_ySlider->GetValue() - m_ySlider->GetMin(), loc.z, loc.layer));
}

void PartitionView::OnSliderZ(wxScrollEvent&)
{
	SliceLocation loc = m_camera->slice_location();
	m_camera->set_slice_location(SliceLocation(loc.x, loc.y, m_zSlider->GetValue() - m_zSlider->GetMin(), loc.layer));
}

void PartitionView::OnSliderLayer(wxScrollEvent&)
{
	SliceLocation loc = m_camera->slice_location();
	m_camera->set_slice_location(SliceLocation(loc.x, loc.y, loc.z, m_layerSlider->GetValue()));
}

void PartitionView::OnSliderZoom(wxScrollEvent&)
{
	m_camera->set_zoom_level(m_zoomSlider->GetValue());
}

//~~~~~~~~~~~~~~~~~~~~ UI UPDATES ~~~~~~~~~~~~~~~~~~~~
void PartitionView::OnUpdateSliderLayer(wxUpdateUIEvent& e)
{
	e.Enable(partition_texture_set(1).get() != NULL);
}

//#################### EVENT TABLE ####################
BEGIN_EVENT_TABLE(PartitionView, wxPanel)
	//~~~~~~~~~~~~~~~~~~~~ BUTTONS ~~~~~~~~~~~~~~~~~~~~
	EVT_BUTTON(BUTTONID_SEGMENT_VOLUME, PartitionView::OnButtonSegmentVolume)
	EVT_BUTTON(BUTTONID_VIEW_XY, PartitionView::OnButtonViewXY)
	EVT_BUTTON(BUTTONID_VIEW_XZ, PartitionView::OnButtonViewXZ)
	EVT_BUTTON(BUTTONID_VIEW_YZ, PartitionView::OnButtonViewYZ)

	//~~~~~~~~~~~~~~~~~~~~ SLIDERS ~~~~~~~~~~~~~~~~~~~~
	EVT_COMMAND_SCROLL(SLIDERID_X, PartitionView::OnSliderX)
	EVT_COMMAND_SCROLL(SLIDERID_Y, PartitionView::OnSliderY)
	EVT_COMMAND_SCROLL(SLIDERID_Z, PartitionView::OnSliderZ)
	EVT_COMMAND_SCROLL(SLIDERID_LAYER, PartitionView::OnSliderLayer)
	EVT_COMMAND_SCROLL(SLIDERID_ZOOM, PartitionView::OnSliderZoom)

	//~~~~~~~~~~~~~~~~~~~~ UI UPDATES ~~~~~~~~~~~~~~~~~~~~
	EVT_UPDATE_UI(SLIDERID_LAYER, PartitionView::OnUpdateSliderLayer)
END_EVENT_TABLE()

}
