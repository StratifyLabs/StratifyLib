#include "ux/Component.hpp"
#include "sys/Printer.hpp"
#include "ux/Scene.hpp"

using namespace ux;


Component & Component::enable(
      hal::Display & display
      ){

   m_reference_drawing_attributes.set_bitmap(display);
   m_display = &display;

   //local bitmap is a small section of the reference bitmap
   m_local_bitmap.allocate(
            m_reference_drawing_attributes.calculate_area_on_bitmap(),
            sgfx::Bitmap::BitsPerPixel(
               m_reference_drawing_attributes.bitmap().bits_per_pixel()
               )
            );

   //local attributes fill local bitmap
   m_local_drawing_attributes
         .set_area(DrawingArea(1000,1000))
         .set_bitmap(m_local_bitmap);

   return *this;
}


Component & Component::disable(){
   m_local_bitmap.free();
   return *this;
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
   if( m_display ){
      //use the palette if it is available

      if( scene()->scene_collection()->theme().set_display_palette(
               *m_display,
               m_theme_style,
               m_theme_state
               ) < 0 ){
         printf("--failed to set display palette\n");
      }

      if( m_display->set_window(
               m_reference_drawing_attributes.calculate_region_on_bitmap()
               ) < 0 ){

      }
      //using the attributes and bitmap to write the display device
      m_display->write(m_local_bitmap);

   }
}

const sgfx::Theme & Component::theme() const {
   return scene()->scene_collection()->theme();
}

void Component::apply_antialias_filter(const DrawingAttributes & attributes){
   if( is_antialias() ){
      attributes.bitmap().apply_antialias_filter(
               theme().antialias_filter(),
               attributes.bitmap().region()
               );
   }
}

void Component::apply_antialias_filter(const DrawingScaledAttributes & attributes){
   if( is_antialias() ){
      attributes.bitmap().apply_antialias_filter(
               theme().antialias_filter(),
               attributes.bitmap().region()
               );
   }
}
