/***
 * millipede: PartitionCanvas.cpp
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#include "PartitionCanvas.h"

#include <common/math/NumericUtil.h>
#include <mast/gui/overlays/PartitionOverlayManager.h>
#include "PartitionCamera.h"

namespace mp {

//#################### CONSTRUCTORS ####################
PartitionCanvas::PartitionCanvas(wxWindow *parent, wxGLContext *context, int *attribList, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:	BaseCanvas(parent, context, attribList, id, pos, size, style)
{}

//#################### PRIVATE METHODS ####################
void PartitionCanvas::render_overlays(double left, double top, double right, double bottom) const
{
	if(overlay_manager())
	{
		overlay_manager()->render_partition_overlays(left, top, right, bottom);
	}
}

Greyscale8SliceTextureSet_CPtr PartitionCanvas::texture_set_to_display() const
{
	if(camera())
	{
		int layer = camera()->slice_location().layer;
		return partition_texture_set(layer);
	}
	else return Greyscale8SliceTextureSet_CPtr();
}

//#################### EVENT HANDLERS ####################

//~~~~~~~~~~~~~~~~~~~~ MOUSE ~~~~~~~~~~~~~~~~~~~~
void PartitionCanvas::OnLeftDown(wxMouseEvent& e)
{
	if(!model()->volume_ipf()) return;

	itk::Vector<double,2> p_Pixels;
	p_Pixels[0] = e.GetX(), p_Pixels[1] = e.GetY();

	// Exit if the click is not within the image bounds.
	itk::Vector<double,2> tl_Pixels, br_Pixels;
	calculate_image_bounds(tl_Pixels, br_Pixels);
	for(int i=0; i<2; ++i)
	{
		if(p_Pixels[i] < tl_Pixels[i] || p_Pixels[i] > br_Pixels[i])
		{
			return;
		}
	}

	// Determine the node being clicked.
	itk::Vector<double,3> p_Coords = pixels_to_3d_coords(p_Pixels);
	int layerIndex = camera()->slice_location().layer;
	itk::Index<3> position;
	for(int i=0; i<3; ++i) position[i] = NumericUtil::round_to_nearest<int>(p_Coords[i]);
	PFNodeID id = model()->volume_ipf()->node_of(layerIndex, position);

	if(e.ShiftDown())
	{
		// Toggle the node in the selection.
		model()->selection()->toggle_node(id);
	}
	else
	{
		// Replace the current selection with the clicked node.
		model()->selection()->replace_with_node(id);
	}
}

//#################### EVENT TABLE ####################
BEGIN_EVENT_TABLE(PartitionCanvas, BaseCanvas)
	//~~~~~~~~~~~~~~~~~~~~ MOUSE ~~~~~~~~~~~~~~~~~~~~
	EVT_LEFT_DOWN(PartitionCanvas::OnLeftDown)
END_EVENT_TABLE()

}
