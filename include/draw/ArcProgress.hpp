/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

#ifndef SAPI_DRAW_ARCPROGRESS_HPP_
#define SAPI_DRAW_ARCPROGRESS_HPP_

#include "Progress.hpp"

namespace draw {

/*! \brief Progress Arc Class
 * \ingroup element
 * \details The Progress Class displays a progress arc on the screen.
 */
class ArcProgress : public Progress {
public:
	ArcProgress();

	/*! \details Draw a scaled version of the arc using the specified \a attr */
	void draw_to_scale(const DrawingScaledAttr & attr);

	void set_direction(s8 dir){
		m_direction = dir;
	}

	void set_offset(s16 offset){
		m_offset = offset;
	}


private:
	s16 m_offset;
	s8 m_direction;
};

}

#endif /* SAPI_DRAW_ARCPROGRESS_HPP_ */
