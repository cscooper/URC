/*
 *  Classifier.h - Computation of pathloss in CORNER.
 *  Copyright (C) 2012  C. S. Cooper, A. Mukunthan
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Contact Details: Cooper - andor734@gmail.com
 */

#pragma once

#include "VectorMath.h"
#include "UrcData.h"


namespace Urc {

	class UrcData;

	/*
	 * Name: Classifier
	 * Inherits: None
	 * Description: This calculates the CORNER state given a source and destination.
	 */
	class Classifier {

	public:

		/*
		 * Name: State
		 * Description: Enumerates the different states for CORNER
		 */
		enum State {
			LOS = 0,		// Direct Line of Sight
			NLOS1,			// Receiver just around the corner
			NLOS2,			// Receiver is on parallel street
			OutOfRange		// Receiver is out of range 
		};
		
		/*
		 * Name: Fading
		 * Description: Boolean to turn Fading on or off
		 */
		bool fading;

		// constructor/destructor
		Classifier();
		Classifier( UrcData::Classification c );
		virtual ~Classifier();

		/*
		 * Method: Real CalculatePathloss( VectorMath::Vector2D source, VectorMath::Vector2D destination );
		 * Description: Calculate the pathloss given the source and destination.
		 */
		VectorMath::Real CalculatePathloss( VectorMath::Vector2D source, VectorMath::Vector2D destination );

		UrcData::Classification GetClassification() { return mClassification; }
		int GetSourceLink() { return mSourceLink; }
		int GetDestinationLink() { return mDestinationLink; }

	protected:

		bool mPrecomputed;
		UrcData::Classification mClassification;
		unsigned int mSourceLink;
		unsigned int mDestinationLink;

		/*
		 * Method: void ComputeState( VectorMath::Vector2D source, VectorMath::Vector2D destination );
		 * Description: Gets the CORNER state given the source and destination.
		 */
		void ComputeState( VectorMath::Vector2D source, VectorMath::Vector2D destination );

	};


};








