/*
 *  Fading.h - Calculates Rayleigh or Rician fading for CORNER based on the situation
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
 *  		     Mukunthan - lexxonnet@gmail.com
 */


#pragma once

#include "Singleton.h"
#include <vector>
#include <pthread.h>
using namespace std;

namespace Urc {

	/* class Fading
	 * This is a class which reads in two randomly sampled gaussian component lists from a file and uses them to calculate Rayleigh and Rician fading
	 * The format of these files is identical to the component files used in Qualnet
	 */
	class Fading : public Singleton<Fading> {
	public:
		typedef vector<double> GaussianList;
		
		Fading(const char* componentsFile, int seed);
		~Fading();
		double CalculateFading(int classification, double kFactor = 0);
	protected:
		pthread_mutex_t mListMutex;
		GaussianList mComponents1;
		GaussianList mComponents2;
		int mSamplingRate;
		int mBaseDopplerFrequency;
		int mNumGaussianComponents;
		int mCurrentIndex;
		void SetArrayIdx();

	};

};
