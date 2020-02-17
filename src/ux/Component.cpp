/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.
#include "ux/Component.hpp"
#include "sys/Printer.hpp"
#include "ux/EventLoop.hpp"

using namespace sgfx;
using namespace ux;

Component::~Component(){
	set_visible(false);
}

void Component::set_visible(bool value){

	if( value && is_enabled() ){
		if( (m_is_visible == false) && display() ){

			m_reference_drawing_attributes.set_bitmap(*display());
			//local bitmap is a small section of the reference bitmap
			if( m_local_bitmap.allocate(
						m_reference_drawing_attributes.calculate_area_on_bitmap(),
						sgfx::Bitmap::BitsPerPixel(
							m_reference_drawing_attributes.bitmap().bits_per_pixel()
							)
						) < 0 ){
				return;
			}

			//local attributes fill local bitmap
			m_local_drawing_attributes
					.set_area(DrawingArea(1000,1000))
					.set_bitmap(m_local_bitmap);

			set_refresh_region(sgfx::Region());
			m_is_visible = true;

			redraw();
		}

	} else if( value == false ){
		if( m_is_visible ){
			m_is_visible = false;
			m_local_bitmap.free();
		}
	}
}


DrawingPoint Component::translate_point(const sgfx::Point & point){
	if( contains(point) == false ){
		return DrawingPoint(0,0);
	}

	sgfx::Point relative_point = point -
			m_reference_drawing_attributes.calculate_point_on_bitmap();

	sgfx::Area area = m_reference_drawing_attributes.calculate_area_on_bitmap();

	//now scale for width
	return DrawingPoint(
				1000 * relative_point.x() / area.width(),
				1000 * relative_point.y() / area.height()
				);
}


void Component::refresh_drawing(){
	if( display() && theme() && is_enabled() ){
		//use the palette if it is available

		if( theme()->set_display_palette(
					*display(),
					m_theme_style,
					m_theme_state
					) < 0 ){
			printf("--failed to set display palette\n");
		}

		Region window_region =
				Region(
					Point(m_reference_drawing_attributes.calculate_point_on_bitmap())
					+ m_refresh_region.point(),
					m_refresh_region.area()
					);

		display()->set_window(window_region);

#if 0
		sys::Printer p;
		p.open_object("draw " + name());
		p << window_region;
		p.close_object();
#endif

		display()->write(
					m_local_bitmap.create_reference(m_refresh_region)
					);

		m_is_refresh_drawing_pending = false;
	}
}

const sgfx::Theme * Component::theme() const {
	return event_loop()->theme();
}

const hal::Display* Component::display() const {
	return event_loop()->display();
}

hal::Display* Component::display(){
	return event_loop()->display();
}


void Component::erase(){
	if( is_enabled() ){
		Region window_region =
				Region(
					Point(m_reference_drawing_attributes.calculate_point_on_bitmap())
					+ m_refresh_region.point(),
					m_refresh_region.area()
					);

		if( theme()->set_display_palette(
					*display(),
					m_theme_style,
					m_theme_state
					) < 0 ){
			printf("--failed to set display palette\n");
		}

#if 0
		sys::Printer p;
		p.open_object("erase " + name());
		p << window_region;
		p.close_object();
#endif
		if( (window_region.width() * window_region.height()) > 0 ){
			display()->set_window(window_region);
			display()->clear();
		}
	}
}

void Component::apply_antialias_filter(const DrawingAttributes & attributes){
	if( is_enabled() ){
		if( is_antialias() ){
#if 0
			attributes.bitmap().apply_antialias_filter(
						theme().antialias_filter(),
						attributes.bitmap().region()
						);
#endif
		}
		set_refresh_drawing_pending();
	}
}

void Component::apply_antialias_filter(const DrawingScaledAttributes & attributes){
	if( is_antialias() ){
#if 0
		attributes.bitmap().apply_antialias_filter(
					theme().antialias_filter(),
					attributes.bitmap().region()
					);
#endif
	}
}
