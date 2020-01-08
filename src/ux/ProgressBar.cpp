#include "ux/ProgressBar.hpp"
#include "ux/Rectangle.hpp"


using namespace sgfx;
using namespace ux;


void ProgressBar::draw_to_scale(const DrawingScaledAttributes & attributes){

   sg_size_t border = 0;
   border = m_border_thickness * attributes.height() / 200;

   sg_size_t progress_size =
         value() * (attributes.area().width() - border * 2) / maximum();


   attributes.bitmap() << Pen().set_color(color_border);

   attributes.bitmap().draw_rectangle(
            attributes.point(),
            attributes.area()
            );

   attributes.bitmap() << Pen().set_color(color_text);
   attributes.bitmap().draw_rectangle(
            attributes.point() + Point(border, border),
            Area(
               progress_size,
               attributes.area().height() - border*2
               )
            );

   apply_antialias_filter(attributes);

}

