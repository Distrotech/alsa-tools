/*
 * Controller for Tascam US-X2Y
 *
 * Copyright (c) 2003 by Karsten Wiese <annabellesgarden@yahoo.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef Cus428State_h
#define Cus428State_h

#include "Cus428_ctls.h"

class Cus428State: public us428_lights, public Cus428_ctls{
 public:
	Cus428State(struct us428ctls_sharedmem* Pus428ctls_sharedmem)
		:us428ctls_sharedmem(Pus428ctls_sharedmem)
		{
			init_us428_lights();
		}
	enum eKnobs{
		eK_RECORD = 72,
		eK_PLAY = 73,
		eK_STOP,
		eK_InputMonitor = 80
	};
	void KnobChangedTo(eKnobs K, bool V);
	void SliderChangedTo(int S, unsigned char New);
 private:
	int LightSend();
	struct us428ctls_sharedmem* us428ctls_sharedmem;
};

extern Cus428State* OneState;

#endif
